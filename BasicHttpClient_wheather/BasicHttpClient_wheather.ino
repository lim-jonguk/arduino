#include <LiquidCrystal_I2C.h>

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>
#define USE_SERIAL Serial

WiFiMulti wifiMulti;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 16, 2);

int tones[8] = {261, 293, 329, 349, 392, 440, 494, 523};
int R = 2;
int G = 0;
int B = 4;
void setup() {

  USE_SERIAL.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  wifiMulti.addAP("KT_GiGA_8AA4", "1baf5kk305");
  xTaskCreate(task1, "task1", 2048, NULL, 1, NULL);
}
String temp;//온도
String wfEn;//날씨
String AirPoll;
String timehttp = "http://worldtimeapi.org/api/timezone/Asia/Seoul";
void loop() { //API에서 정보를 받아오고,원하는 정보만 Parsing
  lcd.clear();

  // wait for WiFi connection
  if ((wifiMulti.run() == WL_CONNECTED)) {
    airPollution();
    weather();
    getTime();
    
  }
  vTaskDelay(30000 / portTICK_PERIOD_MS);
}

void task1(void *parameter) //날씨를 LED로 표현
{
  while (1) {
    if(AirPoll.toInt() == 0){
      digitalWrite(R, HIGH);
      digitalWrite(G, HIGH);
      digitalWrite(B, HIGH);
    }
    else if (AirPoll.toInt() <= 30)
    {
      digitalWrite(R, LOW);
      digitalWrite(G, LOW);
      digitalWrite(B, HIGH);
    }
    else if (AirPoll.toInt() <= 80)
    {
      digitalWrite(R, LOW);
      digitalWrite(G, HIGH);
      digitalWrite(B, LOW);
    }
    else if (AirPoll.toInt() <= 150)
    {
      digitalWrite(R, HIGH);
      digitalWrite(G, HIGH);
      digitalWrite(B, LOW);
    } else {
      digitalWrite(R, HIGH);
      digitalWrite(G, LOW);
      digitalWrite(B, LOW);
    }

  }
}
void getTime() {
  int endtag_index = 0;
  String tag_str;
  String content_str;
  HTTPClient http;


  http.begin("http://worldtimeapi.org/api/timezone/Asia/Seoul"); //HTTP


  int httpCode = http.GET();


  if (httpCode > 0) {

    if (httpCode == HTTP_CODE_OK) { //링크에 있는 정보를 다 불러왔을때
      String payload = http.getString(); //payload 변수에 링크에 표현되는 정보를 넣어라

      tag_str = "datetime"; // abcd.substring(1,4) = bc
      content_str = payload.substring(payload.indexOf(tag_str) + (tag_str.length()) + 3, payload.indexOf(tag_str) + (tag_str.length()) + 13);

      USE_SERIAL.println(content_str);
      lcd.setCursor(0, 0);
      lcd.print(content_str);

      content_str = payload.substring(payload.indexOf(tag_str) + (tag_str.length()) + 14, payload.indexOf(tag_str) + (tag_str.length()) + 19);

      USE_SERIAL.println(content_str);
      lcd.setCursor(11, 0);
      lcd.print(content_str);

    }
  } else {
    USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

}
void weather() {
  int endtag_index = 0;
  String tag_str;
  String content_str;
  HTTPClient http;

  //USE_SERIAL.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin("http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=4146551000"); //HTTP

  //USE_SERIAL.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    //USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) { //링크에 있는 정보를 다 불러왔을때
      String payload = http.getString(); //payload 변수에 링크에 표현되는 정보를 넣어라
      //USE_SERIAL.println(payload);
      endtag_index = payload.indexOf("</data>"); //indexOf ()안에 있는 내용의 위치(index)를 찾아라.
      //USE_SERIAL.println(payload_index);

      if (endtag_index > 0) // </data> 가 있으면,
      {
        tag_str = "<data seq=\"0\">"; // abcd.substring(1,4) = bc
        content_str = payload.substring(payload.indexOf(tag_str) + (tag_str.length()) , endtag_index);
        //USE_SERIAL.println(content_str);

        endtag_index = content_str.indexOf("</wfEn>");
        if (endtag_index > 0)
        {
          tag_str = "<wfEn>";
          wfEn = content_str.substring(content_str.indexOf(tag_str) + tag_str.length(), endtag_index);
          USE_SERIAL.println(wfEn);
          lcd.setCursor(0, 1);
          lcd.print("weather: " + wfEn );
        }

        endtag_index = content_str.indexOf("</temp>");
        if (endtag_index > 0)
        {
          tag_str = "<temp>";
          temp = content_str.substring(content_str.indexOf(tag_str) + tag_str.length(), endtag_index);
          USE_SERIAL.println(temp);
        }
      }
    }
  } else {
    USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

}
void airPollution() {
  int endtag_index = 0;
  String tag_str;
  String content_str;
  HTTPClient http;

  //USE_SERIAL.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin("http://apis.data.go.kr/B552584/ArpltnInforInqireSvc/getMsrstnAcctoRltmMesureDnsty?serviceKey=pj6EkAcEiNpHyTGfsVDkVyetmmh4eDf0X5RgJWUjDWjyxZJDHUBOgxvElUgIEiJZpv%2BDKg2s2Ye8SuEd9MgOwQ%3D%3D&returnType=xml&numOfRows=100&pageNo=1&stationName=%EA%B8%B0%ED%9D%A5&dataTerm=DAILY&ver=1.0"); //HTTP

  //USE_SERIAL.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    //USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) { //링크에 있는 정보를 다 불러왔을때
      String payload = http.getString(); //payload 변수에 링크에 표현되는 정보를 넣어라
      //USE_SERIAL.println(payload);
      endtag_index = payload.indexOf("</item>"); //indexOf ()안에 있는 내용의 위치(index)를 찾아라.
      //USE_SERIAL.println(payload_index);

      if (endtag_index > 0) // </data> 가 있으면,
      {
        tag_str = "<item>"; // abcd.substring(1,4) = bc
        content_str = payload.substring(payload.indexOf(tag_str) + (tag_str.length()) , endtag_index);
        //USE_SERIAL.println(content_str);

        endtag_index = content_str.indexOf("</pm10Value>");
        if (endtag_index > 0)
        {
          tag_str = "<pm10Value>";
          AirPoll = content_str.substring(content_str.indexOf(tag_str) + tag_str.length(), endtag_index);
          USE_SERIAL.println(AirPoll);
        }

      }
    }
  } else {
    USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

}
