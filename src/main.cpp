#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "T4_V13.h"
#include "IP5306.h"
#include "WiFi.h"
#include <Button2.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <Ticker.h>
#include <driver/uart.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <SimpleTime.h>

// Arduino Json Setup
struct SpiRamAllocator {
  void* allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }
  void deallocate(void* pointer) {
    heap_caps_free(pointer);
  }
};
using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;


WiFiMulti WiFiMulti;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const char* root_ca PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIGEzCCA/ugAwIBAgIQfVtRJrR2uhHbdBYLvFMNpzANBgkqhkiG9w0BAQwFADCB\n" \
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTgx\n" \
"MTAyMDAwMDAwWhcNMzAxMjMxMjM1OTU5WjCBjzELMAkGA1UEBhMCR0IxGzAZBgNV\n" \
"BAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEYMBYGA1UE\n" \
"ChMPU2VjdGlnbyBMaW1pdGVkMTcwNQYDVQQDEy5TZWN0aWdvIFJTQSBEb21haW4g\n" \
"VmFsaWRhdGlvbiBTZWN1cmUgU2VydmVyIENBMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
"AQ8AMIIBCgKCAQEA1nMz1tc8INAA0hdFuNY+B6I/x0HuMjDJsGz99J/LEpgPLT+N\n" \
"TQEMgg8Xf2Iu6bhIefsWg06t1zIlk7cHv7lQP6lMw0Aq6Tn/2YHKHxYyQdqAJrkj\n" \
"eocgHuP/IJo8lURvh3UGkEC0MpMWCRAIIz7S3YcPb11RFGoKacVPAXJpz9OTTG0E\n" \
"oKMbgn6xmrntxZ7FN3ifmgg0+1YuWMQJDgZkW7w33PGfKGioVrCSo1yfu4iYCBsk\n" \
"Haswha6vsC6eep3BwEIc4gLw6uBK0u+QDrTBQBbwb4VCSmT3pDCg/r8uoydajotY\n" \
"uK3DGReEY+1vVv2Dy2A0xHS+5p3b4eTlygxfFQIDAQABo4IBbjCCAWowHwYDVR0j\n" \
"BBgwFoAUU3m/WqorSs9UgOHYm8Cd8rIDZsswHQYDVR0OBBYEFI2MXsRUrYrhd+mb\n" \
"+ZsF4bgBjWHhMA4GA1UdDwEB/wQEAwIBhjASBgNVHRMBAf8ECDAGAQH/AgEAMB0G\n" \
"A1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjAbBgNVHSAEFDASMAYGBFUdIAAw\n" \
"CAYGZ4EMAQIBMFAGA1UdHwRJMEcwRaBDoEGGP2h0dHA6Ly9jcmwudXNlcnRydXN0\n" \
"LmNvbS9VU0VSVHJ1c3RSU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNybDB2Bggr\n" \
"BgEFBQcBAQRqMGgwPwYIKwYBBQUHMAKGM2h0dHA6Ly9jcnQudXNlcnRydXN0LmNv\n" \
"bS9VU0VSVHJ1c3RSU0FBZGRUcnVzdENBLmNydDAlBggrBgEFBQcwAYYZaHR0cDov\n" \
"L29jc3AudXNlcnRydXN0LmNvbTANBgkqhkiG9w0BAQwFAAOCAgEAMr9hvQ5Iw0/H\n" \
"ukdN+Jx4GQHcEx2Ab/zDcLRSmjEzmldS+zGea6TvVKqJjUAXaPgREHzSyrHxVYbH\n" \
"7rM2kYb2OVG/Rr8PoLq0935JxCo2F57kaDl6r5ROVm+yezu/Coa9zcV3HAO4OLGi\n" \
"H19+24rcRki2aArPsrW04jTkZ6k4Zgle0rj8nSg6F0AnwnJOKf0hPHzPE/uWLMUx\n" \
"RP0T7dWbqWlod3zu4f+k+TY4CFM5ooQ0nBnzvg6s1SQ36yOoeNDT5++SR2RiOSLv\n" \
"xvcRviKFxmZEJCaOEDKNyJOuB56DPi/Z+fVGjmO+wea03KbNIaiGCpXZLoUmGv38\n" \
"sbZXQm2V0TP2ORQGgkE49Y9Y3IBbpNV9lXj9p5v//cWoaasm56ekBYdbqbe4oyAL\n" \
"l6lFhd2zi+WJN44pDfwGF/Y4QA5C5BIG+3vzxhFoYt/jmPQT2BVPi7Fp2RBgvGQq\n" \
"6jG35LWjOhSbJuMLe/0CjraZwTiXWTb2qHSihrZe68Zk6s+go/lunrotEbaGmAhY\n" \
"LcmsJWTyXnW0OMGuf1pGg+pRyrbxmRE1a6Vqe8YAsOf4vmSyrcjC8azjUeqkk+B5\n" \
"yOGBQMkKW+ESPMFgKuOXwIlCypTPRpgSabuY0MLTDXJLR27lk8QyKGOHQ+SwMj4K\n" \
"00u/I5sUKUErmgQfky3xxzlIPK1aEn8=\n" \
"-----END CERTIFICATE-----\n";

/**
 * 19 X SPEAKER_PWD
 * 26 Y Serial2 TX
 * 32 X TFT_RS
 * 34 X IP5306 INT
 * 4  X TFT_BL
 * 0? BTN 4, is that the boot key? Y PN532 RST
 * 33 Y Serial2 RX
 * 35 ? ADC IN, IS THIS USED?
 * 
 * 36 Y PN532 IRQ
 * 39 X BTN 3
**/

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (36)
#define PN532_RESET (0)  // Not connected by default on the NFC Shield
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

HardwareSerial Barcode(2);

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
SPIClass sdSPI(VSPI);
#define IP5306_ADDR 0X75
#define IP5306_REG_SYS_CTL0 0x00

uint8_t state = 0;
Button2 *pBtns = nullptr;
uint8_t g_btns[] = BUTTONS_MAP;
char buff[512];
Ticker btnscanT;

bool setPowerBoostKeepOn(int en) {
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en)
    Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  else
    Wire.write(0x35); // 0x37 is default reg value
  return Wire.endTransmission() == 0;
}

void button_handle(uint8_t gpio) {
  switch (gpio) {
    case BUTTON_1: {
      state = 1;
    } break;
  
    case BUTTON_2: {
      state = 2;
    } break;
  
    case BUTTON_3: {
      state = 3;
    } break;
    default:
      break;
  }
}

void button_callback(Button2 &b) {
  for (int i = 0; i < sizeof(g_btns) / sizeof(g_btns[0]); ++i) {
    if (pBtns[i] == b) {
      Serial.printf("btn: %u press\n", pBtns[i].getAttachPin());
      button_handle(pBtns[i].getAttachPin());
    }
  }
}

void button_init() {
  uint8_t args = sizeof(g_btns) / sizeof(g_btns[0]);
  pBtns = new Button2[args];
  for (int i = 0; i < args; ++i) {
    pBtns[i] = Button2(g_btns[i]);
    pBtns[i].setPressedHandler(button_callback);
  }
  pBtns[2].setLongClickHandler([](Button2 &b) {
    int x = tft.width() / 2;
    int y = tft.height() / 2 - 30;
    int r = digitalRead(TFT_BL);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.fillScreen(TFT_BLACK);
    tft.drawString(r ? "Backlight OFF" : "Backlight ON", x, y);
    tft.drawString("IP5306 KeepOn ", x - 20, y + 30);

    bool isOk = setPowerBoostKeepOn(1);
    tft.setTextColor(isOk ? TFT_GREEN : TFT_RED, TFT_BLACK);
    tft.drawString(isOk ? "PASS" : "FAIL", x + 50, y + 30);
    if (!isOk) {
      char *str = Wire.getErrorText(Wire.lastError());
      String err = "Wire " + String(str);
      tft.drawString(err, x + 50, y + 60);
      y += 60;
    } else {
      y += 30;
    }
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Press again to wake up", x - 20, y + 30);
    nfc.shutDown(false, true);
    
    // Send command to sleep mode
    uint8_t activate[] PROGMEM = {0x7E, 0x00, 0x08, 0x01, 0x00, 0xD9, 0xA5, 0xAB, 0xCD};
    Barcode.write(activate, 9);
    delay(10);

    while(Barcode.available()){
      Serial.print(Barcode.read(), HEX);
    }

    delay(6000);
    // digitalWrite(TFT_BL, !r);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_3, LOW);
    esp_deep_sleep_start();
  });
}

void button_loop() {
  for (int i = 0; i < sizeof(g_btns) / sizeof(g_btns[0]); ++i) {
    pBtns[i].loop();
  }
}

void spisd_test() {
  if (SD_CS > 0) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, sdSPI)) {
      tft.setTextFont(2);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("SDCard MOUNT FAIL", tft.width() / 2,
                      tft.height() / 2);
    } else {
      uint32_t cardSize = SD.cardSize() / (1024 * 1024);
      String str = "SDCard Size: " + String(cardSize) + "MB";
      tft.setTextFont(2);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawString(str, tft.width() / 2, tft.height() / 2);
    }
    delay(2000);
  }
}

void playSound(void) {
  if (SPEAKER_OUT > 0) {

    if (SPEAKER_PWD > 0) {
      digitalWrite(SPEAKER_PWD, HIGH);
      delay(200);
    }

    ledcWriteTone(CHANNEL_0, 1000);
    delay(250);
    ledcWriteTone(CHANNEL_0, 1500);
    delay(250);
    ledcWriteTone(CHANNEL_0, 2000);
    delay(250);
    ledcWriteTone(CHANNEL_0, 0);

    if (SPEAKER_PWD > 0) {
      delay(200);
      digitalWrite(SPEAKER_PWD, LOW);
    }
  }
}

void buzzer_test() {
  if (SPEAKER_OUT > 0) {
    if (SPEAKER_PWD > 0) {
      pinMode(SPEAKER_PWD, OUTPUT);
    }
    ledcSetup(CHANNEL_0, 1000, 8);
    ledcAttachPin(SPEAKER_OUT, CHANNEL_0);
  }
}

void wifi_scan() {
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(2);
  tft.setTextSize(1);

  tft.drawString("Scan Network", tft.width() / 2, tft.height() / 2);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int16_t n = WiFi.scanNetworks();
  tft.fillScreen(TFT_BLACK);
  if (n == 0) {
    tft.drawString("no networks found", tft.width() / 2, tft.height() / 2);
  } else {
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(0, 0);
    Serial.printf("Fount %d net\n", n);
    for (int i = 0; i < n; ++i) {
      sprintf(buff, "%2d %s (%d)", i + 1, WiFi.SSID(i).c_str(),
              WiFi.RSSI(i));
      tft.println(buff);
    }
  }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setCursor(0, 0);

  tft.println("Listing directory:" + String(dirname));

  File root = fs.open(dirname);
  if (!root) {
    tft.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    tft.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      tft.print("  DIR : ");
      tft.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      tft.print("  FILE: ");
      tft.print(file.name());
      tft.print("  SIZE: ");
      tft.println(file.size());
    }
    file = root.openNextFile();
  }
}

bool activateBarcodeScan(long timeout){
  long timestamp = millis();
  
  // Flush serial input
  while(Barcode.available()){
    Barcode.read();
  }

  // Send command to start scanning
  uint8_t activate[] PROGMEM = {0x7E, 0x00, 0x08, 0x01, 0x00, 0x02, 0x01, 0xAB, 0xCD};
  Barcode.write(activate, 9);

  // Check we get back the OK response, or timeout elapses
  // Wait for serial available but return if timeout reached before response
  // TODO - MAKE THIS A NICER ITTERATIVE LOOP
  while(!Barcode.available()){
    if(millis() > (timestamp + timeout)){
      return 0;
    }
  }
  // We have serial, itterate over it to check it matches the OK response
  if(Barcode.read() != 0x02){
    // Unexpected data
    return 0;
  }

  while(!Barcode.available()){
    if(millis() > (timestamp + timeout)){
      return 0;
    }
  }
  if(Barcode.read() != 0x00){
    // Unexpected data
    return 0;
  }

  while(!Barcode.available()){
    if(millis() > (timestamp + timeout)){
      return 0;
    }
  }
  if(Barcode.read() != 0x00){
    // Unexpected data
    return 0;
  }

  while(!Barcode.available()){
    if(millis() > (timestamp + timeout)){
      return 0;
    }
  }
  if(Barcode.read() != 0x01){
    // Unexpected data
    return 0;
  }

  while(!Barcode.available()){
    if(millis() > (timestamp + timeout)){
      return 0;
    }
  }
  if(Barcode.read() != 0x00){
    // Unexpected data
    return 0;
  }

  while(!Barcode.available()){
    if(millis() > (timestamp + timeout)){
      return 0;
    }
  }
  if(Barcode.read() != 0x33){
    // Unexpected data
    return 0;
  }

  while(!Barcode.available()){
    if(millis() > (timestamp + timeout)){
      return 0;
    }
  }
  if(Barcode.read() != 0x31){
    // Unexpected data
    return 0;
  }


  return 1;
}

bool scanForTag(){
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    if (uidLength == 7)
    {
      if(activateBarcodeScan(1000)){
        Serial.println("Activated scan");
      }
      else{
        Serial.println("Couldn't activate scan");
      }
      delay(1000);
    }
  }

  return 1;
}

void clearScreen(){
  tft.fillRect(0, 11, tft.width(), 308-11, TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(0,15);
}

void drawScreenLayout(){
  tft.setTextFont(1);
  tft.setTextColor(TFT_ORANGE);
  // tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, tft.width(), 10, TFT_BLACK);
  tft.drawFastHLine(0, 10, tft.width(), TFT_ORANGE); // Top divider
  if(WiFi.status() == WL_CONNECTED){
    tft.setTextDatum(TL_DATUM);
    // tft.drawString(timeClient.getFormattedTime(), 0, 0);
    tft.drawNumber(timeClient.getHours(), 0, 0);
    tft.drawString(":", 12, 0);
    tft.drawNumber(timeClient.getMinutes(), 18, 0);
  }

  // Wifi icon
  long wifiStrength = WiFi.RSSI();
  tft.setTextDatum(TR_DATUM);
  if(wifiStrength != 0){
    tft.drawString(String(wifiStrength), tft.width()-16, 0);
  }
  else{
    tft.drawString("X", tft.width()-16, 0);
  }
  if(wifiStrength == 0){
    // No connection
    tft.drawRect(tft.width() - 13, 8-2, 2, 2, TFT_DARKGREY);
    tft.drawRect(tft.width() - 10, 8-4, 2, 4, TFT_DARKGREY);
    tft.drawRect(tft.width() - 7, 8-6, 2, 6, TFT_DARKGREY);
    tft.drawRect(tft.width() - 4, 8-8, 2, 8, TFT_DARKGREY);
    tft.drawLine(tft.width() - 13, 8, tft.width() - 2, 0, TFT_RED);
    tft.drawLine(tft.width() - 14, 8, tft.width() - 3, 0, TFT_RED);
    tft.drawLine(tft.width() - 13, 0, tft.width() - 2, 8, TFT_RED);
    tft.drawLine(tft.width() - 14, 0, tft.width() - 3, 8, TFT_RED);
  }
  else{
    if(wifiStrength > -100){
      // One bar
      tft.drawRect(tft.width() - 13, 8-2, 2, 2, TFT_ORANGE);
    }
    if(wifiStrength > -75){
      // Two bars
      tft.drawRect(tft.width() - 10, 8-4, 2, 4, TFT_ORANGE);
    }
    if(wifiStrength > -50){
      // Three bars
      tft.drawRect(tft.width() - 7, 8-6, 2, 6, TFT_ORANGE);
    }
    if(wifiStrength > -30){
      // Four (full) bars
      tft.drawRect(tft.width() - 4, 8-8, 2, 8, TFT_ORANGE);
    }
  }

  // Battery icon
  uint8_t xpos = tft.width() - 17 - (6*3) - 5;

  // Draw battery box
  tft.drawFastVLine(xpos, 7-5, 4, TFT_ORANGE);
  tft.drawPixel(xpos-1, 7-1, TFT_ORANGE);
  tft.drawPixel(xpos-1, 7-6, TFT_ORANGE);
  tft.drawFastHLine(xpos-17, 7, 16, TFT_ORANGE);
  tft.drawFastHLine(xpos-17, 7-7, 16, TFT_ORANGE);
  tft.drawFastVLine(xpos-17, 7-6, 6, TFT_ORANGE);
  // Fill box depending on charge
  uint8_t batteryLevel = IP5306_LEDS2PCT(IP5306_GetLevelLeds());
  if(batteryLevel == 25){
    tft.fillRect(xpos-16, 7-6, 4, 6, TFT_ORANGE);
  }
  else if(batteryLevel == 50){
    tft.fillRect(xpos-16, 7-6, 8, 6, TFT_ORANGE);
  }
  else if(batteryLevel == 75){
    tft.fillRect(xpos-16, 7-6, 12, 6, TFT_ORANGE);
  }
  else if(batteryLevel == 100){
    tft.fillRect(xpos-16, 7-6, 15, 6, TFT_ORANGE);
    tft.drawFastVLine(xpos-1, 7-5, 4, TFT_ORANGE);
  }
  // Add bolt if currently charging
  if(!IP5306_GetBatteryFull() && IP5306_GetPowerSource()){
    tft.drawLine(xpos-7, 7-2, xpos-9, 7-4, TFT_GREEN);
    tft.drawLine(xpos-10, 7-3, xpos-11, 7-4, TFT_GREEN);
    tft.drawLine(xpos-12, 7-3, xpos-14, 7-5, TFT_GREEN);
  }
  // Add plug if docked
  if(IP5306_GetPowerSource()){
    tft.fillRect(xpos-22, 7-7, 4, 2, TFT_ORANGE);
    tft.fillRect(xpos-22, 7-1, 4, 2, TFT_ORANGE);
    tft.fillRect(xpos-22, 7-5, 2, 4, TFT_ORANGE);
    tft.fillRect(xpos-27, 7-4, 5, 2, TFT_ORANGE);
  }
  


  // ASCII Logo
  // tft.drawString(PROGMEM " _____      _       _     _", 0, 20);
  // tft.drawString(PROGMEM "|  __ \\    | |     | |   (_)", 0, 20+8);
  // tft.drawString(PROGMEM "| |__) |__ | |_   _| |__  _ _   _ ___", 0, 20+(8*2));
  // tft.drawString(PROGMEM "|  ___/ _ \\| | | | | '_ \\| | | | / __|", 0, 20+(8*3));
  // tft.drawString(PROGMEM "| |  | (_) | | |_| | |_) | | |_| \\__ \\", 0, 20+(8*4));
  // tft.drawString(PROGMEM "|_|   \\___/|_|\\__, |_.__/|_|\\__,_|___/", 0, 20+(8*5));
  // tft.drawString(PROGMEM "               __/ |                  ", 0, 20+(8*6));
  // tft.drawString(PROGMEM "              |___/                   ", 0, 20+(8*7));
  // tft.drawString(PROGMEM "______ _       _            _", 35, 84);
  // tft.drawString(PROGMEM "| ___ (_)     | |          | |", 35, 84+8);
  // tft.drawString(PROGMEM "| |_/ /_  ___ | |_ ___  ___| |__", 35, 84+(8*2));
  // tft.drawString(PROGMEM "| ___ \\ |/ _ \\| __/ _ \\/ __| '_ \\", 35, 84+(8*3));
  // tft.drawString(PROGMEM "| |_/ / | (_) | ||  __/ (__| | | |", 35, 84+(8*4));
  // tft.drawString(PROGMEM "\\____/|_|\\___/ \\__\\___|\\___|_| |_|", 35, 84+(8*5));

  tft.fillRect(0, 310, tft.width(), 10, TFT_BLACK);
  tft.drawLine(0, 309, 239, 309, TFT_ORANGE); // Bottom divider
  tft.setTextDatum(BL_DATUM);
  tft.drawString("TEST BUZZER", 0, 320);
  tft.setTextDatum(BC_DATUM);
  tft.drawString("READ RFID", tft.width()/2, 320);
  tft.setTextDatum(BR_DATUM);
  tft.drawString("TRY GET REQUEST", tft.width(), 320);
}

void setup() {
  Serial.begin(115200);
  Barcode.begin(115200, SERIAL_8N1, 33, 26);

  // Send command to exit sleep mode on Barcode scanner
  uint8_t activate[] PROGMEM = {0x7E, 0x00, 0x08, 0x01, 0x00, 0xD9, 0x00, 0xAB, 0xCD};
  Barcode.write(activate, 9);
  delay(10);

  while(Barcode.available()){
    Serial.print(Barcode.read(), HEX);
  }

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);

  if (TFT_BL > 0) {
    // configure LED PWM functionalitites
    ledcSetup(5, 500, 8);
    // attach the channel to the GPIO to be controlled
    ledcAttachPin(TFT_BL, 5);
    // Set brightness 0 - 255
    ledcWrite(5, 25);
  }

  printIP5306Settings();

  drawScreenLayout();
  clearScreen();
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("BOOTING, PLEASE WAIT", tft.width() / 2, tft.height() / 2);

  // SET UP WIFI AND STUFF FOR RESTING REST API
  WiFiMulti.addAP("Bifrost", "lolbutts");
  Serial.print("WiFi Connecting..."); 
  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(3600);
  timeClient.update();

  drawScreenLayout();
  clearScreen();
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("WiFi Connected, IP:", tft.width() / 2, tft.height() / 2);
  tft.drawString(WiFi.localIP().toString(), tft.width() / 2, (tft.height() / 2)+10);

  delay(3000);

  //spisd_test();
  buzzer_test();
  button_init();

  btnscanT.attach_ms(30, button_loop);

  log_d("Total heap: %d", ESP.getHeapSize());
  log_d("Free heap: %d", ESP.getFreeHeap());
  log_d("Total PSRAM: %d", ESP.getPsramSize());
  log_d("Free PSRAM: %d", ESP.getFreePsram());
  log_d("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
}

void loop() {
  // **********************************************************************************************
  if(state == 1){
    state = 0;
    // wifi_scan();
    drawScreenLayout();
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Read RFID", tft.width() / 2, tft.height() / 2);
    printIP5306Stats();
    activateBarcodeScan(1000);
    // scanForTag();
    // playSound();
  }
  else if(state == 2){
    state = 0;
    drawScreenLayout();
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Buzzer Test", tft.width() / 2, tft.height() / 2);
    playSound();
  }
  else if(state == 3){
    state = 0;

    drawScreenLayout();
    clearScreen();
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Please present coin", tft.width() / 2, tft.height() / 2);

    uint8_t success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    
    if (success) {
      // Display some basic information about the card
      Serial.println("Found an ISO14443A card");
      Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("  UID Value: ");
      nfc.PrintHex(uid, uidLength);
      Serial.println("");
      
      if (uidLength == 7)
      {
        drawScreenLayout();
        clearScreen();
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Coin read, accessing data", tft.width() / 2, tft.height() / 2);

        uint8_t uriIdentifier = 0;
        char buffer[60];
        memset(buffer, 0, 60);
        if(nfc.ntag2xx_ReadNDEFString(&uriIdentifier, buffer, 60)){
          WiFiClientSecure *client = new WiFiClientSecure;
          if(client) {
            client -> setCACert(root_ca);
            {
              // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
              HTTPClient https;
              Serial.print("[HTTPS] begin...\n");
              if (https.begin(*client, String("https://") + String(buffer))) {  // HTTPS
                Serial.print("[HTTPS] GET...\n");
                // start connection and send HTTP header
                int httpCode = https.GET();
                // httpCode will be negative on error
                if (httpCode > 0) {
                  // HTTP header has been send and Server response header has been handled
                  Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
                  // file found at server
                  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                    String payload = https.getString();
                    log_d("Payload: %s",payload);
                    drawScreenLayout();
                    clearScreen();
                    SpiRamJsonDocument doc(JSON_OBJECT_SIZE(5) + 90);
                    DeserializationError error = deserializeJson(doc, payload);
                    // Test if parsing succeeds.
                    if (error) {
                      Serial.print(F("deserializeJson() failed: "));
                      Serial.println(error.c_str());
                      return;
                    }

                    const char* coin = doc["coin"];
                    long reserved = doc["reserved"];
                    long claimed = doc["claimed"];

                    const char* coinDetails[78];
                    memset(coinDetails, 0, 78);

                    tft.println(F("\n\nCoin ID:"));
                    tft.println(coin);
                    tft.println(F("\nReserved on:"));
                    tft.print(hour(reserved));
                    tft.print(F(":"));
                    tft.print(minute(reserved));
                    tft.print(F(":"));
                    tft.print(second(reserved));
                    tft.print(F("  "));
                    tft.print(day(reserved));
                    tft.print(F("/"));
                    tft.print(month(reserved));
                    tft.print(F("/"));
                    tft.print(year(reserved));
                    tft.println(F("\n\nClaimed on:"));
                    tft.print(hour(claimed));
                    tft.print(F(":"));
                    tft.print(minute(claimed));
                    tft.print(F(":"));
                    tft.print(second(claimed));
                    tft.print(F("  "));
                    tft.print(day(claimed));
                    tft.print(F("/"));
                    tft.print(month(claimed));
                    tft.print(F("/"));
                    tft.print(year(claimed));

                  }
                } else {
                  Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
                  drawScreenLayout();
                  clearScreen();
                  tft.setTextColor(TFT_GREEN, TFT_BLACK);
                  tft.setTextDatum(MC_DATUM);
                  tft.drawString("Network Error", tft.width() / 2, tft.height() / 2);
                }
                https.end();
              } else {
                Serial.printf("[HTTPS] Unable to connect\n");
                drawScreenLayout();
                clearScreen();
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.setTextDatum(MC_DATUM);
                tft.drawString("Network Error", tft.width() / 2, tft.height() / 2);
              }
              // End extra scoping block
            }
            delete client;
          } else {
            Serial.println("Unable to create client");
            drawScreenLayout();
            clearScreen();
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("Network Error", tft.width() / 2, tft.height() / 2);
          }
        }
        else{
          drawScreenLayout();
          clearScreen();
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextDatum(MC_DATUM);
          tft.drawString("Coin had no address", tft.width() / 2, tft.height() / 2);
        }
      }
    }
    else{
      drawScreenLayout();
      clearScreen();
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Could not read Coin", tft.width() / 2, tft.height() / 2);
    }
  }
}