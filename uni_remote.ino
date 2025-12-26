#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Preferences.h>

// ================= HARDWARE CONFIG =================
#define IR_SEND_PIN 4   // IR LED Anode (+)
// Connect IR LED Cathode (-) to GND via 220ohm resistor

const char* ssid = "Universal-Remote";
const char* pass = "12345678";

// ================= OBJECTS =================
AsyncWebServer server(80);
IRsend irsend(IR_SEND_PIN);
Preferences prefs;

// ================= STATE =================
String currentRoom = "living";
String currentBrand = "samsung";
bool scanning = false;
int scanIndex = 0;
unsigned long lastScanTime = 0;

// List of supported brands for the "Auto Scan" feature
const char* brands[] = {
  "samsung", "lg", "sony", "panasonic", "philips", 
  "sharp", "toshiba", "vizio", "tcl", "hisense", 
  "jvc", "sanyo", "rc5", "nec"
};
const int numBrands = 14;

// ================= IR SENDING LOGIC =================
void sendCode(String brand, String cmd) {
  uint64_t code = 0;
  int bits = 32;

  // --- SAMSUNG ---
  if (brand == "samsung") {
    if (cmd == "power") code = 0xE0E040BF; else if (cmd == "volup") code = 0xE0E0E01F;
    else if (cmd == "voldown") code = 0xE0E0D02F; else if (cmd == "mute") code = 0xE0E0F00F;
    else if (cmd == "source") code = 0xE0E0807F; else if (cmd == "menu") code = 0xE0E058A7;
    else if (cmd == "up") code = 0xE0E006F9; else if (cmd == "down") code = 0xE0E08679;
    else if (cmd == "left") code = 0xE0E0A659; else if (cmd == "right") code = 0xE0E046B9;
    else if (cmd == "ok") code = 0xE0E016E9; else if (cmd == "back") code = 0xE0E01AE5;
    else if (cmd == "chup") code = 0xE0E048B7; else if (cmd == "chdown") code = 0xE0E008F7;
    else if (cmd == "exit") code = 0xE0E0B44B; else if (cmd == "info") code = 0xE0E0F807;
    // Numbers
    else if (cmd == "1") code = 0xE0E020DF; else if (cmd == "2") code = 0xE0E0A05F;
    else if (cmd == "3") code = 0xE0E0609F; else if (cmd == "4") code = 0xE0E010EF;
    else if (cmd == "5") code = 0xE0E0906F; else if (cmd == "6") code = 0xE0E050AF;
    else if (cmd == "7") code = 0xE0E030CF; else if (cmd == "8") code = 0xE0E0B04F;
    else if (cmd == "9") code = 0xE0E0708F; else if (cmd == "0") code = 0xE0E08877;
    
    if (code) irsend.sendSAMSUNG(code, 32);
  }
  
  // --- LG ---
  else if (brand == "lg") {
    if (cmd == "power") code = 0x20DF10EF; else if (cmd == "volup") code = 0x20DF40BF;
    else if (cmd == "voldown") code = 0x20DFC03F; else if (cmd == "mute") code = 0x20DF906F;
    else if (cmd == "ok") code = 0x20DF22DD; else if (cmd == "menu") code = 0x20DFC23D;
    else if (cmd == "up") code = 0x20DF02FD; else if (cmd == "down") code = 0x20DF827D;
    else if (cmd == "left") code = 0x20DFE01F; else if (cmd == "right") code = 0x20DF609F;
    if (code) irsend.sendNEC(code, 32); 
  }

  // --- SONY (12-bit legacy) ---
  else if (brand == "sony") {
    if (cmd == "power") code = 0xA90; else if (cmd == "volup") code = 0x490;
    else if (cmd == "voldown") code = 0xC90; else if (cmd == "mute") code = 0x290;
    else if (cmd == "chup") code = 0x090; else if (cmd == "chdown") code = 0x890;
    if (code) for(int i=0; i<3; i++) irsend.sendSony(code, 12);
  }

  // --- NEC GENERIC (TCL, VIZIO, HISENSE) ---
  else if (brand == "nec" || brand == "tcl" || brand == "vizio" || brand == "hisense") {
    if (cmd == "power") code = 0x20DF10EF; 
    // Note: Vizio/TCL codes vary wildly. NEC standard is a safe bet for many.
    if (code) irsend.sendNEC(code, 32);
  }

  // --- PANASONIC ---
  else if (brand == "panasonic") {
    if (cmd == "power") code = 0x0100BCBD;
    if (code) irsend.sendPanasonic(0x4004, code);
  }

  // --- PHILIPS / RC5 (Old CRT) ---
  else if (brand == "philips" || brand == "rc5") {
    if (cmd == "power") code = 0xC; else if (cmd == "volup") code = 0x10;
    else if (cmd == "voldown") code = 0x11; else if (cmd == "mute") code = 0xD;
    if (code) irsend.sendRC5(code, 12);
  }

  // --- SHARP ---
  else if (brand == "sharp") {
    // Sharp legacy
    if (cmd == "power") irsend.sendSharp(0x4004, 0x1, 15);
  }

  Serial.println("Sent " + cmd + " for " + brand);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  irsend.begin();
  
  prefs.begin("remote", false); // Namespace "remote"
  
  WiFi.softAP(ssid, pass);
  Serial.println("IP: " + WiFi.softAPIP().toString());

  // --- API: Connectivity ---
  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *r){ r->send(200); });

  // --- API: Room Logic ---
  server.on("/setRoom", HTTP_POST, [](AsyncWebServerRequest *r){
    if (r->hasParam("room", true)) {
      currentRoom = r->getParam("room", true)->value();
      currentBrand = prefs.getString(currentRoom.c_str(), "samsung");
      r->send(200, "application/json", "{\"brand\":\"" + currentBrand + "\"}");
    }
  });

  server.on("/saveBrand", HTTP_POST, [](AsyncWebServerRequest *r){
    if (r->hasParam("brand", true)) {
      currentBrand = r->getParam("brand", true)->value();
      prefs.putString(currentRoom.c_str(), currentBrand);
      r->send(200);
    }
  });

  // --- API: Command ---
  server.on("/cmd", HTTP_POST, [](AsyncWebServerRequest *r){
    if (r->hasParam("key", true)) {
      sendCode(currentBrand, r->getParam("key", true)->value());
      r->send(200);
    }
  });

  // --- API: Auto Scan Trigger ---
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *r){
    if (r->hasParam("action")) {
      String action = r->getParam("action")->value();
      if (action == "start") {
        scanning = true;
        scanIndex = 0;
        Serial.println("--- SCAN START ---");
      } else {
        scanning = false;
        // Save the brand that was last active
        if (scanIndex > 0) {
            String foundBrand = brands[scanIndex - 1]; // Previous index was the one that worked
            currentBrand = foundBrand;
            prefs.putString(currentRoom.c_str(), currentBrand);
            Serial.println("Scan saved brand: " + currentBrand);
        }
      }
    }
    r->send(200);
  });

  server.begin();
}

// ================= LOOP =================
void loop() {
  // AUTO SCAN LOGIC
  if (scanning) {
    if (millis() - lastScanTime > 2500) { // Wait 2.5 seconds between tries
      lastScanTime = millis();
      
      if (scanIndex < numBrands) {
        String tryBrand = brands[scanIndex];
        Serial.print("Scanning: "); Serial.println(tryBrand);
        sendCode(tryBrand, "power");
        scanIndex++;
      } else {
        // Reset or Stop if we ran out of brands
        scanIndex = 0; 
      }
    }
  }
}