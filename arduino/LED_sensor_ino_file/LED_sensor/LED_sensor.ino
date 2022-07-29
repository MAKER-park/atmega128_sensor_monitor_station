//insert 관련
//http://cloud.park-cloud.co19.kr/project/insert.php?temp=45&hum=25&pm1=150&pm2=200&pm3=20 다음과 같은 주소 형식으로 데이터를 보낼 예정

//status 관련
//INSERT INTO `project_status` (`status`, `R`, `G`, `B`) VALUES ('0', '255', '255', '255');
//UPDATE `project_status` SET `status`=1,`R`=255,`G`=250,`B`=250 WHERE 1
//http://cloud.park-cloud.co19.kr/project/update_status.php?status=0&R=200&G=100&B=20

//view 관련
//http://cloud.park-cloud.co19.kr/project/view.php
//http://cloud.park-cloud.co19.kr/project/view_status.php

//----------------------------web---------------------------------
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid     = "printer_room_asus";
const char* password = "134625wo";
String host = "http://cloud.park-cloud.co19.kr/project";
String sta, R, G, B;
int count = 0;
int mode_number = 0;

WiFiServer server(80);
WiFiClient client;
HTTPClient http;
//----------------------------web---------------------------------
//----------------------------LED---------------------------------
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIN        2 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 13 // Popular NeoPixel ring size

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//----------------------------LED---------------------------------
//----------------------------dust sensor---------------------------------
#include <pms.h>
Pmsx003 pms(14, 16); //미세먼지 센서

int pm1, pm2, pm3;
//----------------------------dust sensor---------------------------------
//----------------------------dht---------------------------------
#include "DHT.h"
#define DHTPIN 5 
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  int h = 0;
  // Read temperature as Celsius (the default)
  int t = 0;
//----------------------------dht---------------------------------

void setup(void) {
  dht.begin();
  
  Serial.begin(115200);
  while (!Serial) {};
  Serial.println("Pmsx003");
  
  pms.begin();
  pms.waitForData(Pmsx003::wakeupTime);
  pms.write(Pmsx003::cmdModeActive);

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Server started");
}

auto lastRead = millis();

void loop(void) {
  const auto n = Pmsx003::Reserved;
  Pmsx003::pmsData data[n];
  Pmsx003::PmsStatus status = pms.read(data, n);



  switch (status) {
    case Pmsx003::OK:
      {
        auto newRead = millis();
        //Serial.print("Wait time ");
        //Serial.println(newRead - lastRead);
        if (newRead - lastRead > 10000) { //10초마다 센서 값 갱신 및 DB 업데이트
          lastRead = newRead;
          // For loop starts from 3
          // Skip the first three data (PM1dot0CF1, PM2dot5CF1, PM10CF1)
          for (size_t i = Pmsx003::PM1dot0; i < n; ++i) {
            if (i == 3) {
              pm1 = data[i];
            } else if (i == 4) {
              pm2 = data[i];
            } else if (i == 5) {
              pm3 = data[i];
            }
          }
          Serial.print("pm1.0 : ");
          Serial.println(pm1);
          Serial.print("pm2.5 : ");
          Serial.println(pm2);
          Serial.print("pm10 : ");
          Serial.println(pm3);
          //db 읽기 테스트
          read_status();
          mode_number = sta.toInt();
          Serial.print("mode number : ");
          Serial.println(mode_number);
          if(mode_number == 0){//무드등 모드가 아닐때 db 업데이트 진행
            h = int(dht.readHumidity());
            t = int(dht.readTemperature());
            insert_db();
          }
          break;
        }
      }
    case Pmsx003::noData:
      break;
    default:
      Serial.println(Pmsx003::errorMsg[status]);
  };
  if (mode_number == 0) {
    //Serial.println("status = false");//무드등 모드가 아닐때
    if (pm2 < 35) {
      colorWipe(pixels.Color(  0,   0,   150), 100);    // blue
    } else if (pm2 >= 35 && pm2 < 75 ) {
      colorWipe(pixels.Color(  150,   150,   0), 100);    // yellow
    } else {
      colorWipe(pixels.Color(  150,   0,   0), 100);    // red
    }
  }else{
    //Serial.println("status = true");
    int r= R.toInt();
    int g= G.toInt();
    int b= B.toInt();
    Serial.println("------------------------------ color -------------");
    Serial.println(r);
    Serial.println(g);
    Serial.println(b);
    colorWipe(pixels.Color(  r,   g,   b), 100);    // red
  }
}
