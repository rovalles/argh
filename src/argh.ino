#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
//#include <WiFi.h>

#include <Wire.h>
#include <SPI.h>
#include "SH1106.h"
#include "SH1106Ui.h"

// Pin definitions for I2C
#define OLED_SDA    D2  // pin 14

#define OLED_RESET  D1   // RESET
#define OLED_DC     D2   // Data/Command
#define OLED_CS     D8   // Chip select

#include "images.h"

// Uncomment one of the following based on OLED type
SH1106 display(true, OLED_RESET, OLED_DC, OLED_CS); // FOR SPI
//SH1106   display(OLED_ADDR, OLED_SDA, OLED_SDC);    // For I2C
SH1106Ui ui     ( &display );

WiFiClient client;

void connectToService();
void wifi();

const char* host = "http://rodolfoovalles.com";
const int httpsPort = 80;

bool msOverlay(SH1106 *display, SH1106UiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, String(millis()));
  return true;
}

bool drawFrame1(SH1106 *display, SH1106UiState* state, int x, int y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "find Magic!");
  return false;
}

// how many frames are there?
int frameCount = 4;
// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
bool (*frames[])(SH1106 *display, SH1106UiState* state, int x, int y) = { drawFrame1 };

bool (*overlays[])(SH1106 *display, SH1106UiState* state)             = { msOverlay };
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
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //exit after config instead of connecting
  //wifiManager.setBreakAfterConfig(true);

  //reset settings - for testing
  //wifiManager.resetSettings();


  //tries to connect to last known settings
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP" with password "password"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("NodeMcu-Esp8266", "password")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
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

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Inital UI takes care of initalising the display too.
  ui.init();

  display.flipScreenVertically();


  // Use WiFiClientSecure class to create TLS connection
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(request);
  Serial.println("==========");
  Serial.println("closing connection");
}

void loop() {
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
  // put your main code here, to run repeatedly:
}
