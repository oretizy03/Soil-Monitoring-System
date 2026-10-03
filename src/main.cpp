// SMART_SOIL_MONITOR.ino
// Current readings (OLED + web)
// Last 20 readings
// 24-hour rolling Soil Condition Trend table
// Live updating current hour averages

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define MOISTURE_PIN 34
#define PH_PIN 35

const int AIR_VALUE = 2448;
const int WATER_VALUE = 841;

const int ADC_PH7 = 3265;
const int ADC_PH25 = 3790;

const char* AP_NAME = "SMART_SOIL_MONITOR";
WebServer server(80);

DNSServer dnsServer;
const byte DNS_PORT = 53;

unsigned long baseEpoch = 0;  
unsigned long baseMillis = 0; 
bool timeSynced = false;
float currentTemp = 0;
float currentPH = 0;
int currentMoisture = 0;
String currentMoistureStatus = "";
String currentPHStatus = "";

const int HISTORY_SIZE = 20;
float tempHistory[HISTORY_SIZE];
float phHistory[HISTORY_SIZE];
int moistureHistory[HISTORY_SIZE];
int historyIndex = 0;
int historyCount = 0;
unsigned long lastLog = 0;

const int TREND_SIZE = 24;

struct TrendRow {
  float temp;
  float moisture;
  float ph;
  String status;
  unsigned long epoch;
};

TrendRow trend[TREND_SIZE];
int trendCount = 0;
int trendStart = 0;
float hourTempSum = 0;
float hourMoistureSum = 0;
float hourPhSum = 0;
unsigned long hourSamples = 0;
unsigned long hourStartMillis = 0;
const unsigned long HOUR_DURATION = 3600000UL;

String getSoilStatus(float moisture)
{
  if (moisture < 30) return "Dry";
  if (moisture < 70) return "Good";
  return "Wet";
}

unsigned long currentEpoch()
{
  if(!timeSynced) return 0;
  return baseEpoch + (millis()-baseMillis)/1000UL;
}

String formatEpoch(unsigned long epoch, bool withDate)
{
  if(!timeSynced || epoch==0) return "--:--";
  time_t rawtime = (time_t)epoch;
  struct tm *info = gmtime(&rawtime);   
  char buf[24];
  if(withDate) strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", info);
  else strftime(buf, sizeof(buf), "%H:%M", info);
  return String(buf);
}

String predictIrrigation(float pH, float moisture, float temp)
{
    if (moisture <= 24.98)
    {
        if (temp <= 29.91)
        {
            if (moisture <= 11.35)
            {
                return "Medium";   
            }
            else
            {
                return "Medium";  
            }
        }
        else
        {
            if (temp <= 41.62)
            {
                return "Medium";   
            }
            else
            {
                return "High";    
            }
        }
    }
    else
    {
        if (temp <= 30.16)
        {
            if (pH <= 4.96)
            {
                return "Low";    
            }
            else
            {
                return "Low";    
            }
        }
        else
        {
            if (moisture <= 58.57)
            {
                return "Low";    
            }
            else
            {
                return "Low";     
            }
        }
    }
}

void finalizeHour()
{
  if (hourSamples == 0) return;

  TrendRow row;

  row.temp = hourTempSum / hourSamples;
  row.moisture = hourMoistureSum / hourSamples;
  row.ph = hourPhSum / hourSamples;
  row.status = getSoilStatus(row.moisture);
  row.epoch = currentEpoch();

  int pos;
  if (trendCount < TREND_SIZE)
  {
    pos = (trendStart + trendCount) % TREND_SIZE;
    trendCount++;
  }
  else
  {
    pos = trendStart;
    trendStart = (trendStart + 1) % TREND_SIZE;
  }

  trend[pos] = row;

  hourTempSum = 0;
  hourMoistureSum = 0;
  hourPhSum = 0;
  hourSamples = 0;
}

void handleRoot()
{
  String html;

  html += "<html><head>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<style>";
  html += "body{font-family:Arial;}";
  html += "table{border-collapse:collapse;}";
  html += "th,td{border:1px solid black;padding:6px;text-align:center;white-space:nowrap;}";
  html += ".table-wrap{overflow-x:auto;max-width:100%;}";
  html += "</style>";
  html += "</head><body>";
  
  html += "<script>";
  html += "var off=new Date().getTimezoneOffset()*60000;";
  html += "var t=Math.floor((Date.now()-off)/1000);";
  html += "fetch('/synctime?t='+t);";
  html += "</script>";

  html += "<h1>Smart Soil Monitoring System</h1>";

  unsigned long bootEpoch = timeSynced ? (baseEpoch - baseMillis/1000UL) : 0;
  html += "<p><b>Device Started:</b> " + formatEpoch(bootEpoch,true);
  if(!timeSynced) html += " (waiting for a device to sync time)";
  html += "</p>";

  html += "<p><b>NOTE:</b> The <b>Current Readings Table</b> leads the <b>Last 20 Readings Table</b> by 1.5 seconds due to processing delays.";

  html += "<h2>Current Readings</h2>";
  html += "<table>";
  html += "<tr><th>Parameter</th><th>Value</th></tr>";
  html += "<tr><td>Temperature</td><td>" + String(currentTemp,1) + " C</td></tr>";
  html += "<tr><td>Moisture</td><td>" + String(currentMoisture) + "% (" + currentMoistureStatus + ")</td></tr>";
  html += "<tr><td>pH</td><td>" + String(currentPH,2) + " (" + currentPHStatus + ")</td></tr>";
  html += "</table><br>";

  // ---- Last 20 Readings, now HORIZONTAL: one row per parameter, one column per reading ----
  html += "<h2>Last 20 Readings</h2>";
  html += "<div class='table-wrap'><table>";

  html += "<tr><th>Reading #</th>";
  for(int i=0;i<historyCount;i++){
    html += "<th>" + String(i+1) + "</th>";
  }
  html += "</tr>";

  html += "<tr><td>Temp (C)</td>";
  for(int i=0;i<historyCount;i++){
    int idx=(historyIndex-1-i+HISTORY_SIZE)%HISTORY_SIZE;
    html += "<td>" + String(tempHistory[idx],1) + "</td>";
  }
  html += "</tr>";

  html += "<tr><td>Moisture (%)</td>";
  for(int i=0;i<historyCount;i++){
    int idx=(historyIndex-1-i+HISTORY_SIZE)%HISTORY_SIZE;
    html += "<td>" + String(moistureHistory[idx]) + "</td>";
  }
  html += "</tr>";

  html += "<tr><td>pH</td>";
  for(int i=0;i<historyCount;i++){
    int idx=(historyIndex-1-i+HISTORY_SIZE)%HISTORY_SIZE;
    html += "<td>" + String(phHistory[idx],2) + "</td>";
  }
  html += "</tr>";

  html += "</table></div><br>";

  html += "<h2>Soil Condition Trend (24 Hours)</h2><table>";
  html += "<tr><th>Time</th><th>Status</th><th>Avg Moisture</th><th>Avg pH</th><th>Avg Temp</th></tr>";

  for(int i=0;i<trendCount;i++){
    int idx=(trendStart+i)%TREND_SIZE;
    html += "<tr><td>" + formatEpoch(trend[idx].epoch,true);
    html += "</td><td>"+trend[idx].status+"</td><td>"+String(trend[idx].moisture,1)+"%</td><td>"+String(trend[idx].ph,2)+"</td><td>"+String(trend[idx].temp,1)+" C</td></tr>";
  }

  if(hourSamples>0){
    float liveTemp=hourTempSum/hourSamples;
    float liveMoist=hourMoistureSum/hourSamples;
    float livePh=hourPhSum/hourSamples;

    html += "<tr><td>LIVE</td><td>"+getSoilStatus(liveMoist)+"</td><td>"+String(liveMoist,1)+"%</td><td>"+String(livePh,2)+"</td><td>"+String(liveTemp,1)+" C</td></tr>";
  }

  html += "</table></body></html>";

  server.send(200,"text/html",html);
}

void handleSyncTime()
{
  if(server.hasArg("t")){
    baseEpoch = strtoul(server.arg("t").c_str(), NULL, 10);
    baseMillis = millis();
    timeSynced = true;
  }
  server.send(200,"text/plain","ok");
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(21,22);

  if(!display.begin(OLED_ADDR, true)){
    Serial.println("SH1106 OLED not found at 0x3C - check wiring/address");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE); 
  display.setCursor(0,0);
  display.println("Booting...");
  display.display();

  sensors.begin();
  sensors.setResolution(10);
  analogReadResolution(12);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_NAME);

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/",handleRoot);
  server.on("/generate_204",handleRoot);          
  server.on("/gen_204",handleRoot);                
  server.on("/hotspot-detect.html",handleRoot);    
  server.on("/ncsi.txt",handleRoot);               
  server.on("/connecttest.txt",handleRoot);        
  server.on("/synctime",handleSyncTime);
  server.onNotFound(handleRoot);
  server.begin();
  hourStartMillis = millis();
}

void loop()
{
  dnsServer.processNextRequest();
  server.handleClient();
  sensors.requestTemperatures();
  float temperature=sensors.getTempCByIndex(0);
  int moistureADC=analogRead(MOISTURE_PIN);
  int moisturePercent=map(moistureADC,AIR_VALUE,WATER_VALUE,0,100);
  moisturePercent=constrain(moisturePercent,0,100);
  String moistureStatus=getSoilStatus(moisturePercent);

  long total=0;
  const int PH_SAMPLES = 10;
  for(int i=0;i<PH_SAMPLES;i++){
    total+=analogRead(PH_PIN);
    delay(5);
  }

  int phADC=total/PH_SAMPLES;

  float pH=7.0-((float)(phADC-ADC_PH7)*4.5)/(ADC_PH25-ADC_PH7);
  pH=constrain(pH,0.0,14.0);

  String phStatus;
  if(pH<6.5) phStatus="Acidic";
  else if(pH<=7.5) phStatus="Neutral";
  else phStatus="Alkaline";

  currentTemp=temperature;
  currentPH=pH;
  currentMoisture=moisturePercent;
  currentMoistureStatus=moistureStatus;
  currentPHStatus=phStatus;

  hourTempSum += temperature;
  hourMoistureSum += moisturePercent;
  hourPhSum += pH;
  hourSamples++;

  if(millis()-hourStartMillis>=HOUR_DURATION){
    finalizeHour();
    hourStartMillis=millis();
  }

  if(millis()-lastLog>=5000){
    tempHistory[historyIndex]=temperature;
    phHistory[historyIndex]=pH;
    moistureHistory[historyIndex]=moisturePercent;

    historyIndex=(historyIndex+1)%HISTORY_SIZE;
    if(historyCount<HISTORY_SIZE) historyCount++;

    lastLog=millis();
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  display.println("SMART SOIL MONITOR");
  display.drawLine(0,10,SCREEN_WIDTH,10,SH110X_WHITE);

  display.setCursor(0,16);
  display.print("Temp: "); display.print(temperature,1); display.println(" C");

  display.setCursor(0,30);
  display.print("Moist: "); display.print(moisturePercent); display.print("% "); display.println(moistureStatus);

  display.setCursor(0,44);
  display.print("pH: "); display.print(pH,2); display.print(" "); display.println(phStatus);

  display.display();

 
  delay(200);
}
