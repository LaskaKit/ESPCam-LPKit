/*
 * Test code for LaskaKit ESP-S3-CAM module + LaskaKit ESPD-3.5" 320x480, ILI9488 display
 * SERVER part
 *
 * Board:   LaskaKit ESP-S3-CAM                                            https://www.laskakit.cz/laskakit-espink-esp32-e-paper-pcb-antenna/
 * Board with Display: LaskaKit ESPD-35 ESP32 3.5 TFT ILI9488 Touch        https://www.laskakit.cz/laskakit-espd-35-esp32-3-5-tft-ili9488-touch
 * 
 * How to steps for display:
 * 1. Copy file Setup300_ILI9488_ESPD-3_5.h from https://github.com/LaskaKit/ESPD-35/tree/main/SW to Arduino/libraries/TFT_eSPI/User_Setups/
 * 2. in Arduino/libraries/TFT_eSPI/User_Setup_Select.h
      a. comment: #include <User_Setup.h>
      b. add: #include <User_Setups/Setup300_ILI9488_ESPD-3_5.h>  // Setup file for LaskaKit ESPD-3.5" 320x480, ILI9488
 *
 * used code form ESP32_CAM_WEBSOCKET_CLIENT: https://github.com/0015/ThatProject/blob/master/ESP32CAM_Projects/ESP32_CAM_WIFI_CONNECT/ESP32_CAM_WEBSOCKET_CLIENT/ESP32_CAM_WEBSOCKET_CLIENT.ino
 * Email:podpora@laskakit.cz
 * Web:laskakit.cz
*/

#include <SPI.h>
#include <ArduinoWebsockets.h>
#include <WiFi.h>

#include <TJpg_Decoder.h>
#include <TFT_eSPI.h>

const char* ssid = "LaskaKit-WEB-CAM";
const char* password = "laskakit";

using namespace websockets;
WebsocketsServer server;
WebsocketsClient client;

TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
   // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // This might work instead if you adapt the sketch to use the Adafruit_GFX library
  // tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);


    pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);

  
  tft.begin();
  tft.setRotation(3);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.fillScreen(TFT_RED);
  tft.setSwapBytes(true); // We need to swap the colour bytes (endianess)

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);
  
  Serial.println();
  Serial.println("Setting AP...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP Address : ");
  Serial.println(IP);

  server.listen(8888);
}

void loop() {
  if(server.poll()){
      client = server.accept();
    }

    if(client.available()){
      client.poll();

      WebsocketsMessage msg = client.readBlocking();

      uint32_t t = millis();

      // Get the width and height in pixels of the jpeg if you wish
      uint16_t w = 0, h = 0;
      TJpgDec.getJpgSize(&w, &h, (const uint8_t*)msg.c_str(), msg.length());
      Serial.print("Width = "); Serial.print(w); Serial.print(", height = "); Serial.println(h);
    
      // Draw the image, top left at 0,0
      TJpgDec.drawJpg(0, 0, (const uint8_t*)msg.c_str(), msg.length());
    
      // How much time did rendering take (ESP8266 80MHz 271ms, 160MHz 157ms, ESP32 SPI 120ms, 8bit parallel 105ms
      t = millis() - t;
      Serial.print(t); Serial.println(" ms");
    }  
}