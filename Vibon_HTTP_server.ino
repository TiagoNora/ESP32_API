#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>


#include "DHT.h"

#define DHTPIN 14

#define DHTTYPE DHT22  

char jsonOutput[128];

const int TRIG_PIN = 26;
const int ECHO_PIN = 27;

unsigned long t_eco;
int N = 10;

// Set your access point network credentials
const char* ssid = "ESP32_VIBON";
const char* password = "123";

const char* PARAM_DIST = "dist";
const char* PARAM_N = "n";
const char* PARAM_ALPHA = "alpha";
const char* PARAM_BETA = "beta";
DHT dht(DHTPIN, DHTTYPE);


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

float readTemp() {
  float t = dht.readTemperature();
  Serial.println(t);
  return t;
}

float readHumi() {
   float h = dht.readHumidity();
   Serial.println(h);
   return h;
}

float readHeatIndex() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  float hic = dht.computeHeatIndex(t, h, false);
  Serial.println(hic);
  return hic;
}

float calibration(int n){
  /* mean echo time, time_avg (us) */
    float time_avg = 0.0;
    N=n;
    for ( int n = 0; n < N; n++) {      // N measurements
      /* ultrasonic pulse */
      digitalWrite( TRIG_PIN, HIGH );     // SONAR trigger
      delayMicroseconds( 10 );            // wait 10 us
      digitalWrite( TRIG_PIN, LOW );      // end of pulse
      t_eco = pulseIn( ECHO_PIN, HIGH );  // echo time, us
      
      time_avg += float(t_eco);           // accumulate...
      delay(50);                          // wait 50 ms ?
    }
    
    time_avg /= float(N);                   // mean echo time, us
    time_avg /= 1000;  
    N=10;

    return time_avg;
}

float hooke(float alpha, float beta){
  float dist_avg=alpha*(float)calibration(10)+beta;
  return dist_avg;
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println();
  
  pinMode( TRIG_PIN, OUTPUT );
  pinMode (ECHO_PIN, INPUT );
  
  pinMode(14, OUTPUT);
  dht.begin();
  
  // Setting the ESP as an access point
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/plain", String(readTemp()).c_str());
    

    StaticJsonDocument<200> doc;
     String str=String(readTemp());
    doc["temperature"] = str;
    serializeJson(doc,jsonOutput);
    
     request->send_P(200, "application/json", String(jsonOutput).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/plain", String(readHumi()).c_str());
    StaticJsonDocument<200> doc;
     String str=String(readHumi());
    doc["humidity"] = str;
    serializeJson(doc,jsonOutput);
    
     request->send_P(200, "application/json", String(jsonOutput).c_str());
  });
  server.on("/heatIndex", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send_P(200, "text/plain", String(readHeatIndex()).c_str());
    StaticJsonDocument<200> doc;
     String str=String(readHeatIndex());
    doc["heatIndex"] = str;
    serializeJson(doc,jsonOutput);
    
     request->send_P(200, "application/json", String(jsonOutput).c_str());
  });
  server.on("/calibration", HTTP_GET, [](AsyncWebServerRequest *request){

    String dist_mm="DEFAULT";
    String n="10";
    String inputMessage;

    if (request->hasParam(PARAM_DIST)) {
      inputMessage = request->getParam(PARAM_DIST)->value();
      //dist_mm = PARAM_DIST;
      dist_mm = inputMessage;
    }
    if (request->hasParam(PARAM_N)) {
      inputMessage = request->getParam(PARAM_N)->value();
      //n = PARAM_N;
      n = inputMessage;
    }
    Serial.println(inputMessage);
     Serial.println("dist: ");
    Serial.println(dist_mm);
    Serial.println("n: ");
    Serial.println(n);
    //request->send_P(200, "text/plain", String(calibration(n.toInt())).c_str());
    StaticJsonDocument<200> doc;
     String str=String(calibration(n.toInt()));
    doc["calibration"] = str;
    serializeJson(doc,jsonOutput);
    
    
     request->send_P(200, "application/json", String(jsonOutput).c_str());
  });
  
  server.on("/hooke", HTTP_GET, [](AsyncWebServerRequest *request){
    String alpha="172.65";
    String beta="0.5";
    String inputMessage;

    if (request->hasParam(PARAM_ALPHA)) {
      inputMessage = request->getParam(PARAM_ALPHA)->value();
      //alpha = PARAM_ALPHA;
      alpha = inputMessage;
    }
    if (request->hasParam(PARAM_BETA)) {
      inputMessage = request->getParam(PARAM_BETA)->value();
      //beta = PARAM_BETA;
      beta = inputMessage;
    }
    Serial.println(inputMessage);
    Serial.println("alpha: ");
    Serial.println(alpha);
    Serial.println("beta: ");
    Serial.println(beta);
    //request->send_P(200, "text/plain", String(hooke(alpha.toFloat(),beta.toFloat())).c_str());
    StaticJsonDocument<200> doc;
     String str=String(hooke(alpha.toFloat(),beta.toFloat()));
    doc["hooke"] = str;
    serializeJson(doc,jsonOutput);
    
    
     request->send_P(200, "application/json", String(jsonOutput).c_str());
  });
  
  // Start server
  server.begin();
}
 
void loop(){
  
}
