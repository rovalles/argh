# 1 "/var/folders/zv/1hqhxh0n6m374cwzysmdn6zc0000gn/T/tmpeqXy0_"
#include <Arduino.h>
# 1 "/Users/bdcoe/Projects/Arduino/argh/src/argh.ino"
#include <FS.h>

#include <ESP8266WiFi.h>


#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>


#include <Wire.h>
#include <SPI.h>
#include "SH1106.h"
#include "SH1106Ui.h"


#define OLED_SDA D2

#define OLED_RESET D1
#define OLED_DC D2
#define OLED_CS D8

#include "images.h"


SH1106 display(true, OLED_RESET, OLED_DC, OLED_CS);

SH1106Ui ui ( &display );

WiFiClient client;

void connectToService();
void wifi();

const char* host = "http://rodolfoovalles.com";
const int httpsPort = 80;
bool msOverlay(SH1106 *display, SH1106UiState* state);
bool drawFrame1(SH1106 *display, SH1106UiState* state, int x, int y);
void connectToService();
String makeRequest();
void wifi();
void setup();
void loop();
#line 38 "/Users/bdcoe/Projects/Arduino/argh/src/argh.ino"
bool msOverlay(SH1106 *display, SH1106UiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, String(millis()));
  return true;
}

bool drawFrame1(SH1106 *display, SH1106UiState* state, int x, int y) {



  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "find Magic!");
  return false;
}


int frameCount = 4;


bool (*frames[])(SH1106 *display, SH1106UiState* state, int x, int y) = { drawFrame1 };

bool (*overlays[])(SH1106 *display, SH1106UiState* state) = { msOverlay };
int overlaysCount = 1;

void connectToService(){

  Serial.print("connecting to ");
  Serial.println(host);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

}

String makeRequest(){
  String url = "/projects/weather/";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");



  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  String line = client.readStringUntil('\n');
  if (line.startsWith("weather:")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }

  return line;
}

void wifi(){


  WiFiManager wifiManager;
# 123 "/Users/bdcoe/Projects/Arduino/argh/src/argh.ino"
  if (!wifiManager.autoConnect("NodeMcu-Esp8266", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }


  Serial.println("connected...yeey :)");


  Serial.println("local ip");
  Serial.println(WiFi.localIP());

}


void setup() {
  Serial.begin(115200);
  Serial.println();
  wifi();
  connectToService();
  String request = makeRequest();

  ui.setTargetFPS(30);

  ui.setActiveSymbole(activeSymbole);
  ui.setInactiveSymbole(inactiveSymbole);



  ui.setIndicatorPosition(BOTTOM);


  ui.setIndicatorDirection(LEFT_RIGHT);



  ui.setFrameAnimation(SLIDE_LEFT);


  ui.setFrames(frames, frameCount);


  ui.setOverlays(overlays, overlaysCount);


  ui.init();

  display.flipScreenVertically();



  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(request);
  Serial.println("==========");
  Serial.println("closing connection");
}

void loop() {
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {



    delay(remainingTimeBudget);
  }

}