// 1. Install 3.06 of MrFaptastic's library (seems to be an issue with 3.07)
// 2. Install fastLED (this sketch depends on it)
// 3. Open the "AuroraDemo" from the matrix library demo.
// 4. Get that running on your matrix (you may need to change pins etc)
// 5. Apply whatever changes you made to this sketch
// 6. Update the Wifi details


// --- Added by Brian
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "SSID";
const char* password = "password";

// End of Add ---

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

/*--------------------- MATRIX GPIO CONFIG  -------------------------*/
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 19 // Changed from library default
#define C_PIN 5
#define D_PIN 17
#define E_PIN -1
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16


/*--------------------- MATRIX PANEL CONFIG -------------------------*/
#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another
 
/*
//Another way of creating config structure
//Custom pin mapping for all pins
HUB75_I2S_CFG::i2s_pins _pins={R1, G1, BL1, R2, G2, BL2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};
HUB75_I2S_CFG mxconfig(
						64,   // width
						64,   // height
						 4,   // chain length
					 _pins,   // pin mapping
  HUB75_I2S_CFG::FM6126A      // driver chip
);

*/
MatrixPanel_I2S_DMA *dma_display = nullptr;

// Module configuration
HUB75_I2S_CFG mxconfig(
	PANEL_RES_X,   // module width
	PANEL_RES_Y,   // module height
	PANEL_CHAIN    // Chain length
);


//mxconfig.gpio.e = 18; // Assign a pin if you have a 64x64 panel
//mxconfig.clkphase = false; // Change this if you have issues with ghosting.
//mxconfig.driver = HUB75_I2S_CFG::FM6126A; // Change this according to your pane.



#include <FastLED.h>

#define MATRIX_HEIGHT 64 // --- Added by Brian (Matrix specific)

#include "Effects.h"
Effects effects;

#include "Drawable.h"
#include "Playlist.h"
//#include "Geometry.h"

#include "Patterns.h"
Patterns patterns;

/* -------------------------- Some variables -------------------------- */
unsigned long fps = 0, fps_timer; // fps (this is NOT a matrix refresh rate!)
unsigned int default_fps = 30, pattern_fps = 30;  // default fps limit (this is not a matrix refresh counter!)
unsigned long ms_animation_max_duration = 20000;  // 20 seconds
unsigned long last_frame=0, ms_previous=0;

// --- Added by Brian 
WebServer server(80);

const int led = 2;

void handleRoot() {
  digitalWrite(led, 1);
  String temp = "hello from esp32! Current Millis=" + String(millis());
  server.send(200, "text/plain", temp);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

// End of add --- 

void setup()
{
 /************** SERIAL **************/
  Serial.begin(115200);
  delay(250);

// --- Added by Brian
  /************** WIFI **************/

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // End of add --- 
  
 /************** DISPLAY **************/
  Serial.println("...Starting Display");
  // --- Added by Brian (This is matrix specific, so don't need to add if yours is working)
  mxconfig.gpio.e = 18; // Assign a pin if you have a 64x64 panel
  mxconfig.clkphase = false; // Change this if you have issues with ghosting.
  // End of add --- 
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90); //0-255

  dma_display->fillScreenRGB888(128,0,0);
  delay(1000);
  dma_display->fillScreenRGB888(0,0,128);
  delay(1000);
  dma_display->clearScreen();  
  delay(1000);  
  Serial.println("**************** Starting Aurora Effects Demo ****************");


   // setup the effects generator
  effects.Setup();

  delay(500);
  Serial.println("Effects being loaded: ");
  listPatterns();
 

  patterns.moveRandom(1); // start from a random pattern

  Serial.print("Starting with pattern: ");
  Serial.println(patterns.getCurrentPatternName());
  patterns.start();
  ms_previous = millis();
  fps_timer = millis();
}

void loop()
{
    // menu.run(mainMenuItems, mainMenuItemCount);  
  server.handleClient(); // --- Added by Brian
  if ( (millis() - ms_previous) > ms_animation_max_duration ) 
  {
       patterns.stop();      
       patterns.moveRandom(1);
       //patterns.move(1);
       patterns.start();  
       
       Serial.print("Changing pattern to:  ");
       Serial.println(patterns.getCurrentPatternName());
        
       ms_previous = millis();

       // Select a random palette as well
       //effects.RandomPalette();
    }
 
    if ( 1000 / pattern_fps + last_frame < millis()){
      last_frame = millis();
      pattern_fps = patterns.drawFrame();
      if (!pattern_fps)
        pattern_fps = default_fps;

      ++fps;
    }

    if (fps_timer + 1000 < millis()){
       Serial.printf_P(PSTR("Effect fps: %ld\n"), fps);
       fps_timer = millis();
       fps = 0;
    }
       
}


void listPatterns() {
  patterns.listPatterns();
}
