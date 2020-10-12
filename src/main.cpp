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
#include <Regexp.h>

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


WiFiMulti WiFimulti;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "coin.polyb.io", 3600);
bool isNTPSet = false;

const char* root_ca = \
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
Ticker redrawHFT;
Ticker connectNTP;

String headerTitle = "";

bool setPowerBoostKeepOn(int en) {
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en)
    Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  else
    Wire.write(0x35); // 0x37 is default reg value
  return Wire.endTransmission() == 0;
}

void shutdownSystem(){
  bool isOk = setPowerBoostKeepOn(1);
    
  if (!isOk) {
    log_e("Shutting down error, Wire %s", Wire.getErrorText(Wire.lastError()));
  } else {
    log_d("Shutting down peripherals");
  }
  
  // Sleep nfc module
  nfc.shutDown(false, true);
  
  // Send command to sleep mode the barcode reader
  const uint8_t activate[]  = {0x7E, 0x00, 0x08, 0x01, 0x00, 0xD9, 0xA5, 0xAB, 0xCD};
  Barcode.write(activate, 9);
  delay(10);

  while(Barcode.available()){
    Serial.print(Barcode.read(), HEX);
  }

  delay(1000);

  log_d("Shutting down esp32");
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_2, LOW);
  esp_deep_sleep_start();
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
      log_d("btn: %u press", pBtns[i].getAttachPin());
      button_handle(pBtns[i].getAttachPin());
    }
  }
}

void button_init() {
  const uint8_t args = sizeof(g_btns) / sizeof(g_btns[0]);
  pBtns = new Button2[args];
  for (int i = 0; i < args; ++i) {
    pBtns[i] = Button2(g_btns[i]);
    pBtns[i].setPressedHandler(button_callback);
  }
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
      tft.drawString(F("SDCard MOUNT FAIL"), tft.width() / 2,
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

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setCursor(0, 0);

  tft.println("Listing directory:" + String(dirname));

  File root = fs.open(dirname);
  if (!root) {
    tft.println(F("- failed to open directory"));
    return;
  }
  if (!root.isDirectory()) {
    tft.println(F(" - not a directory"));
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      tft.print(F("  DIR : "));
      tft.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      tft.print(F("  FILE: "));
      tft.print(file.name());
      tft.print(F("  SIZE: "));
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
  const uint8_t activate[] = {0x7E, 0x00, 0x08, 0x01, 0x00, 0x02, 0x01, 0xAB, 0xCD};
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
    log_d("Found an ISO14443A card");
    log_d("  UID Length: %d bytes", uidLength);
    log_d("  UID Value: %02X:%02X:%02X:%02X:%02X:%02X:%02X", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
    
    if (uidLength == 7)
    {
      if(activateBarcodeScan(1000)){
        log_d("Activated scan");
      }
      else{
        log_w("Couldn't activate scan");
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

void drawHeader(){
  tft.setTextFont(1);
  tft.setTextColor(TFT_ORANGE);
  tft.fillRect(0, 0, tft.width(), 10, TFT_BLACK);
  tft.drawFastHLine(0, 10, tft.width(), TFT_ORANGE); // Top divider
  if(isNTPSet){
    tft.setTextDatum(TL_DATUM);
    if(timeClient.getHours() < 10){
      tft.drawNumber(0, 0, 0);
      tft.drawNumber(timeClient.getHours(), 7, 0);
    }
    else{
      tft.drawNumber(timeClient.getHours(), 0, 0);
    }
    tft.drawString(F(":"), 13, 0);
    if(timeClient.getMinutes() < 10){
      tft.drawNumber(0, 18, 0);
      tft.drawNumber(timeClient.getMinutes(), 25, 0);
    }
    else{
      tft.drawNumber(timeClient.getMinutes(), 18, 0);
    }
  }

  tft.setTextDatum(TC_DATUM);
  if(headerTitle == ""){
    tft.drawString(WiFi.localIP().toString(), (tft.width()/2)-20, 0);
  }
  else{
    tft.drawString(headerTitle, (tft.width()/2)-20, 0);
  }

  // Wifi icon
  long wifiStrength = WiFi.RSSI();
  tft.setTextDatum(TR_DATUM);
  if(wifiStrength != 0){
    tft.drawString(String(wifiStrength), tft.width()-16, 0);
  }
  else{
    tft.drawString(F("X"), tft.width()-16, 0);
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
}

void drawFooter(String left, String middle, String right){
  tft.fillRect(0, 310, tft.width(), 10, TFT_BLACK);
  tft.drawLine(0, 309, 239, 309, TFT_ORANGE); // Bottom divider
  tft.setTextDatum(BL_DATUM);
  tft.drawString(left, 0, 320);
  tft.setTextDatum(BC_DATUM);
  tft.drawString(middle, tft.width()/2, 320);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(right, tft.width(), 320);
}

bool tryToConnectNTP(){
  if(!isNTPSet){
    if(timeClient.update()){
      isNTPSet = true;
      return true;
    }
  }
  return false;
}

void tryToConnectNTPTickerCall(){
  if(headerTitle == "" && !isNTPSet){
    log_d("Attempting ticker ntp connect:");
    if(tryToConnectNTP()){
      log_d("Ticker ntp connected");
      drawHeader();
    }
    else{
      log_d("Ticker ntp connect failed");
    }
  }
}

void backgroundHeaderFooterDraw(){
  log_d("Running background header redraw");
  drawHeader();
}

void mintNewCoin(){
  headerTitle = "Mint Fresh Coin";
  drawHeader();
  drawFooter("", "", "");
  clearScreen();
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(F("Please present coin"), tft.width() / 2, tft.height() / 2);

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    log_d("Found an ISO14443A card");
    log_d("  UID Length: %d bytes", uidLength);
    log_d("  UID Value: %02X:%02X:%02X:%02X:%02X:%02X:%02X", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
    
    if (uidLength == 7){
      clearScreen();
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString(F("Coin found, checking compatibility"), tft.width() / 2, tft.height() / 2);

      if(!nfc.ntag2xx_IsLocked() && !nfc.ntag2xx_IsPassworded(NTAG213)){
        clearScreen();
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(F("Coin compatable"), tft.width() / 2, tft.height() / 2);
        tft.drawString(F("Contacting PolyCoin Servers"), tft.width() / 2, (tft.height() / 2) + 10);

        // Request coin ID from /coin POST endpoint
        WiFiClientSecure *client = new WiFiClientSecure;
        if(client) {
          client -> setCACert(root_ca);
          {
            // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
            HTTPClient https;
            log_d("[HTTPS] begin...\n");
            if (https.begin(*client, "https://coin.polyb.io/coins")) {  // HTTPS
              log_d("[HTTPS] POST...\n");
              // start connection and send HTTP header
              String body = "{\"key\":\"887dee7b-15a5-40fd-ae00-b852e99c3d49\"}";
              https.addHeader("Content-Type", "application/json");
              const char * headerKeys[] = {"location"} ;
              const size_t numberOfHeaders = 1;
              https.collectHeaders(headerKeys, numberOfHeaders);
              int httpCode = https.POST(body);
              // httpCode will be negative on error
              if (httpCode > 0) {
                // HTTP header has been send and Server response header has been handled
                log_d("[HTTPS] POST... code: %d\n", httpCode);
                // file found at server
                if (httpCode == HTTP_CODE_SEE_OTHER) {
                  String location = https.header("location");
                  log_d("Location of new coin is: %s", location.c_str());
                  https.end();

                  clearScreen();
                  tft.setTextColor(TFT_GREEN, TFT_BLACK);
                  tft.setTextDatum(MC_DATUM);
                  tft.drawString(F("Connected to server"), tft.width() / 2, tft.height() / 2);
                  tft.drawString(F("Initialising coin, please wait"), tft.width() / 2, (tft.height() / 2) + 10);

                  // Write new coin address to coin and verify
                  uint8_t uriIdentifier2 = 0;
                  char buffer2[60];
                  if(nfc.ntag2xx_ReadNDEFString(&uriIdentifier2, buffer2, 60)){
                    // We have an existing record, erase
                    log_d("Tag has existing record, trying to remove it");
                    if(nfc.ntag2xx_EraseUserData()){
                      // We erased the tag ready for writing
                      log_d("Successfully erased tag");
                    }
                    else{
                      log_e("Could not erase tag");
                    }
                  }

                  String coinLoc = "coin.polyb.io" + location;
                  char *cstr = new char[coinLoc.length() + 1];
                  strcpy(cstr, coinLoc.c_str());
                  if(nfc.ntag2xx_WriteNDEFURI(NDEF_URIPREFIX_HTTPS, cstr)){
                    log_d("URL Written to coin");
                    clearScreen();
                    tft.setTextColor(TFT_GREEN, TFT_BLACK);
                    tft.setTextDatum(MC_DATUM);
                    tft.drawString(F("Coin Initalised"), tft.width() / 2, tft.height() / 2);
                    tft.drawString(F("Finalising coin with PolyCoin Server"), tft.width() / 2, (tft.height() / 2) + 10);
                    tft.drawString(F("Please wait"), tft.width() / 2, (tft.height() / 2) + 20);

                    // Post coin serial to claim coin
                    log_d("[HTTPS] begin...\n");
                    String coinClaimLoc = "https://coin.polyb.io" + location + "/claim";
                    log_d("%s", coinClaimLoc.c_str());
                    if (https.begin(*client, coinClaimLoc)) {  // HTTPS
                      log_d("[HTTPS] POST...\n");
                      // start connection and send HTTP header
                      char buf[15];
                      memset(buf, 0, 15);
                      sprintf(buf, "%02X%02X%02X%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
                      String body = "{\"key\":\"887dee7b-15a5-40fd-ae00-b852e99c3d49\",\"serial\":\"" + String(buf) + "\"}";
                      https.addHeader("Content-Type", "application/json");
                      int httpCode = https.POST(body);
                      // httpCode will be negative on error
                      if (httpCode > 0) {
                        // HTTP header has been send and Server response header has been handled
                        log_d("[HTTPS] POST... code: %d\n", httpCode);
                        // file found at server
                        if (httpCode == HTTP_CODE_NO_CONTENT) {
                          clearScreen();
                          tft.setTextColor(TFT_GREEN, TFT_BLACK);
                          tft.setTextDatum(MC_DATUM);
                          tft.drawString(F("PolyCoin Finalised"), tft.width() / 2, tft.height() / 2);
                          tft.drawString(F("Locking Coin"), tft.width() / 2, (tft.height() / 2) + 10);

                          // Lock coin
                          // if(nfc.ntag2xx_Lock()){ // Disable locking for the moment
                          if(true){
                            clearScreen();
                            tft.setTextColor(TFT_GREEN, TFT_BLACK);
                            tft.setTextDatum(MC_DATUM);
                            tft.drawString(F("Coin Minted and Locked"), tft.width() / 2, tft.height() / 2);
                            tft.drawString(F("Please remove coin"), tft.width() / 2, (tft.height() / 2) + 10);
                          }
                          else{
                            log_e("Error could not lock coin");
                            clearScreen();
                            tft.setTextColor(TFT_GREEN, TFT_BLACK);
                            tft.setTextDatum(MC_DATUM);
                            tft.drawString(F("Could not lock coin"), tft.width() / 2, tft.height() / 2);
                            tft.drawString(F("Dispose of coin and try again"), tft.width() / 2, (tft.height() / 2) + 10);
                          }
                        }
                        else{
                          log_e("Error httpcode not OK");
                          clearScreen();
                          tft.setTextColor(TFT_GREEN, TFT_BLACK);
                          tft.setTextDatum(MC_DATUM);
                          tft.drawString(F("Error finalising with PolyCoin"), tft.width() / 2, tft.height() / 2);
                          tft.drawString(F("Dispose of coin and try again"), tft.width() / 2, (tft.height() / 2) + 10);
                        }
                      }
                      else{
                        log_e("Error httpcode error");
                        clearScreen();
                        tft.setTextColor(TFT_GREEN, TFT_BLACK);
                        tft.setTextDatum(MC_DATUM);
                        tft.drawString(F("Error finalising with PolyCoin"), tft.width() / 2, tft.height() / 2);
                        tft.drawString(F("Dispose of coin and try again"), tft.width() / 2, (tft.height() / 2) + 10);
                      }
                    }
                    else{
                      log_e("Error opening client for /claim POST");
                      clearScreen();
                      tft.setTextColor(TFT_GREEN, TFT_BLACK);
                      tft.setTextDatum(MC_DATUM);
                      tft.drawString(F("Error finalising with PolyCoin"), tft.width() / 2, tft.height() / 2);
                      tft.drawString(F("Dispose of coin and try again"), tft.width() / 2, (tft.height() / 2) + 10);
                    }
                  }
                  else{
                    log_e("Error writing ndef record to coin");
                    clearScreen();
                    tft.setTextColor(TFT_GREEN, TFT_BLACK);
                    tft.setTextDatum(MC_DATUM);
                    tft.drawString(F("Error writing to coin"), tft.width() / 2, tft.height() / 2);
                    tft.drawString(F("Dispose of coin and try again"), tft.width() / 2, (tft.height() / 2) + 10);
                  }
                }
              } else {
                log_e("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
                clearScreen();
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.setTextDatum(MC_DATUM);
                tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
              }
              https.end();
            } else {
              log_w("[HTTPS] Unable to connect\n");
              clearScreen();
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.setTextDatum(MC_DATUM);
              tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
            }
            // End extra scoping block
          }
          delete client;
        }
      }
      else{
        clearScreen();
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(F("Coin not compatable for minting"), tft.width() / 2, tft.height() / 2);
      }
    }
  }
  else{
    clearScreen();
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(F("Could not read Coin"), tft.width() / 2, tft.height() / 2);
  }
}

void readCoinAPIData(){
  headerTitle = "Read Coin Details";
  drawHeader();
  drawFooter("", "", "");
  clearScreen();
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(F("Please present coin"), tft.width() / 2, tft.height() / 2);

  uint8_t success = 0;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  state = 0;
  drawFooter("", "CANCEL", "");
  while(!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 500)){
    if(state != 0){
      clearScreen();
      tft.printf("Read Coin Canceled\n\nPress Exit to return");
      return;
    }
  }

  drawFooter("", "", "");
  success = 1;

  if (success) {
    // Display some basic information about the card
    log_d("Found an ISO14443A card");
    log_d("  UID Length: %d bytes", uidLength);
    log_d("  UID Value: %02X:%02X:%02X:%02X:%02X:%02X:%02X", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
    
    if (uidLength == 7)
    {
      clearScreen();
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString(F("Coin read, accessing data"), tft.width() / 2, tft.height() / 2);

      uint8_t uriIdentifier = 0;
      char buffer[60];
      memset(buffer, 0, 60);
      if(nfc.ntag2xx_ReadNDEFString(&uriIdentifier, buffer, 60)){
        // Check the ndef record is a valid url
        MatchState ms;
        ms.Target(buffer);
        const char * regex = "^coin.polyb.io/coins/[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]%-[0-9a-f][0-9a-f][0-9a-f][0-9a-f]" \
        "%-4[0-9a-f][0-9a-f][0-9a-f]%-[89ab][0-9a-f][0-9a-f][0-9a-f]%-[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]$";
        bool result = ms.Match(regex);
        log_d("The string %s a a valid coin id url", result?"was":"was not");

        if(result){
          WiFiClientSecure *client = new WiFiClientSecure;
          if(client) {
            client -> setCACert(root_ca);
            {
              // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
              HTTPClient https;
              log_d("[HTTPS] begin...\n");
              if (https.begin(*client, String("https://") + String(buffer))) {  // HTTPS
                log_d("[HTTPS] GET...\n");
                // start connection and send HTTP header
                int httpCode = https.GET();
                // httpCode will be negative on error
                if (httpCode > 0) {
                  // HTTP header has been send and Server response header has been handled
                  log_d("[HTTPS] GET... code: %d\n", httpCode);
                  // file found at server
                  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                    String payload = https.getString();
                    log_d("Payload: %s", payload.c_str());
                    SpiRamJsonDocument doc(JSON_OBJECT_SIZE(6) + 90);
                    DeserializationError error = deserializeJson(doc, payload);
                    // Test if parsing succeeds.
                    if (error) {
                      log_e("deserializeJson() failed: %s", error.c_str());
                      return;
                    }

                    const char* coin = doc["coin"];
                    long reserved = doc["reserved"];
                    long claimed = doc["claimed"];
                    long modified = doc["modified"];
                    float value = doc["value"];
                    float escrow = doc["escrow"];

                    const char* coinDetails[78];
                    memset(coinDetails, 0, 78);

                    clearScreen();

                    tft.printf("Coin ID:\n  %s\n", coin);
                    tft.printf("\nUID Value:\n  %02X:%02X:%02X:%02X:%02X:%02X:%02X\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
                    tft.printf("\nCurrent Value:\n  %f\n", value);
                    tft.printf("\nEscrow Value:\n  %f\n", escrow);
                    if(reserved != 0){
                      tft.printf("\nReserved on:\n  %02d:%02d:%02d %02d/%02d/%02d\n", hour(reserved),
                      minute(reserved), second(reserved), day(reserved), month(reserved), year(reserved));
                    }
                    if(claimed != 0){
                      tft.printf("\nClaimed on:\n  %02d:%02d:%02d %02d/%02d/%02d\n", hour(claimed),
                      minute(claimed), second(claimed), day(claimed), month(claimed), year(claimed));
                    }
                    if(modified != 0){
                      tft.printf("\nLast modified:\n  %02d:%02d:%02d %02d/%02d/%02d\n", hour(modified),
                      minute(modified), second(modified), day(modified), month(modified), year(modified));
                    }
                  }
                } else {
                  log_e("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
                  clearScreen();
                  tft.setTextColor(TFT_GREEN, TFT_BLACK);
                  tft.setTextDatum(MC_DATUM);
                  tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
                }
                https.end();
              } else {
                log_w("[HTTPS] Unable to connect\n");
                clearScreen();
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.setTextDatum(MC_DATUM);
                tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
              }
              // End extra scoping block
            }
            delete client;
          } else {
            log_e("Unable to create client");
            clearScreen();
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.setTextDatum(MC_DATUM);
            tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
          }
        }
        else{
          log_e("Bad coin url");
          clearScreen();
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextDatum(MC_DATUM);
          tft.drawString(F("Invalid coin details"), tft.width() / 2, tft.height() / 2);
        }
      }
      else{
        clearScreen();
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(F("Coin had no address"), tft.width() / 2, tft.height() / 2);
      }
    }
  }
  else{
    clearScreen();
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(F("Could not read Coin"), tft.width() / 2, tft.height() / 2);
  }
}

void sweepQRCode(){
  headerTitle = "Read Escrow QRCode";
  drawHeader();
  clearScreen();
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(F("Please Scan Transaction QRCode"), tft.width() / 2, tft.height() / 2);
  if(activateBarcodeScan(1000)){
    // Reader active, read for code over serial with 6 second timeout
    Barcode.setTimeout(6000);
    String barcodeString = Barcode.readStringUntil('\r');
    log_d("Barcode string was: %s with a length of %d", (char *)barcodeString.c_str(), sizeof((char *)barcodeString.c_str()));

    // If read, check its a valid uuid4
    MatchState ms;
    ms.Target((char *)barcodeString.c_str());
    const char * regex = "^[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]%-[0-9a-f][0-9a-f][0-9a-f][0-9a-f]" \
    "%-4[0-9a-f][0-9a-f][0-9a-f]%-[89ab][0-9a-f][0-9a-f][0-9a-f]%-[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]$";
    bool result = ms.Match(regex);
    log_d("The string %s a uuid4", result?"was":"was not");

    // If it is, try to get its details over https
    if(result){
      clearScreen();
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString(F("Transaction code read"), tft.width() / 2, tft.height() / 2);
      tft.drawString(F("Contacting PolyCoin for details"), tft.width() / 2, (tft.height() / 2) + 10);

      // If it exists, show details on screen and allow for either canceling or scanning a tag
      WiFiClientSecure *client = new WiFiClientSecure;
        if(client) {
          client -> setCACert(root_ca);
          {
            // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
            HTTPClient https;
            log_d("[HTTPS] begin...\n");
            if (https.begin(*client, String("https://coin.polyb.io/transactions/escrow/") + barcodeString)) {  // HTTPS
              log_d("[HTTPS] GET...\n");
              // start connection and send HTTP header
              int httpCode = https.GET();
              // httpCode will be negative on error
              if (httpCode > 0) {
                // HTTP header has been send and Server response header has been handled
                log_d("[HTTPS] GET... code: %d\n", httpCode);
                // file found at server
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                  String payload = https.getString();
                  log_d("Payload: %s", payload.c_str());

                  https.end();

                  SpiRamJsonDocument doc(JSON_OBJECT_SIZE(7) + 250);
                  DeserializationError error = deserializeJson(doc, payload);
                  // Test if parsing succeeds.
                  if (error) {
                    log_e("deserializeJson() failed: %s", error.c_str());
                    return;
                  }

                  const char* transaction = doc["transaction"];
                  const char* origin = doc["origin"];
                  float value = doc["value"];
                  long created = doc["created"];
                  const char* destination = doc["destination"];
                  const char* resolvedTransaction = doc["resolvedTransaction"];
                  long claimed = doc["claimed"];

                  clearScreen();

                  tft.printf("Escrow Transaction ID:\n  %s\n", transaction);
                  tft.printf("\nTransaction Value:\n  %f\n", value);
                  tft.printf("\nOrigin Coin ID:\n  %s\n", origin);
                  tft.printf("\nCreated on:\n  %02d:%02d:%02d %02d/%02d/%02d\n", hour(created),
                    minute(created), second(created), day(created), month(created), year(created));

                  if(claimed == 0){
                    tft.printf("\n\nEscrow Transaction has not been claimed!\nPresent Coin to claim the value");

                    // If scanning a tag, try to post claim the transaction to the tag
                    uint8_t success = 0;
                    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
                    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

                    state = 0;
                    drawFooter("", "CANCEL", "");
                    while(!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 500)){
                      if(state != 0){
                        clearScreen();
                        tft.printf("Escrow Sweep Canceled\n\nPress Exit to return");
                        return;
                      }
                    }

                    drawFooter("", "", "");
                    success = 1;
                    
                    if (success) {
                      log_d("RFID Coin found, read ID...");
                      clearScreen();
                      tft.setTextColor(TFT_GREEN, TFT_BLACK);
                      tft.setTextDatum(MC_DATUM);
                      tft.drawString(F("Coin found, reading ID"), tft.width() / 2, tft.height() / 2);
                      // Display some basic information about the card
                      log_d("Found an ISO14443A card");
                      log_d("  UID Length: %d bytes", uidLength);
                      log_d("  UID Value: %02X:%02X:%02X:%02X:%02X:%02X:%02X", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
                      
                      if (uidLength == 7)
                      {
                        uint8_t uriIdentifier = 0;
                        char buffer[60];
                        memset(buffer, 0, 60);
                        if(nfc.ntag2xx_ReadNDEFString(&uriIdentifier, buffer, 60)){
                          log_d("NDEF string read");
                          // Check the ndef record is a valid url
                          MatchState ms;
                          ms.Target(buffer);
                          const char * regex = "^coin.polyb.io/coins/[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]%-[0-9a-f][0-9a-f][0-9a-f][0-9a-f]" \
                          "%-4[0-9a-f][0-9a-f][0-9a-f]%-[89ab][0-9a-f][0-9a-f][0-9a-f]%-[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]$";
                          bool result = ms.Match(regex);
                          log_d("The string %s a a valid coin id url", result?"was":"was not");

                          if(result){
                            clearScreen();
                            tft.setTextColor(TFT_GREEN, TFT_BLACK);
                            tft.setTextDatum(MC_DATUM);
                            tft.drawString(F("Accessing coin data from PolyCoin"), tft.width() / 2, tft.height() / 2);

                            log_d("[HTTPS] begin...\n");
                            if (https.begin(*client, String("https://") + String(buffer))) {  // HTTPS
                              log_d("[HTTPS] GET...\n");
                              // start connection and send HTTP header
                              int httpCode = https.GET();
                              // httpCode will be negative on error
                              if (httpCode > 0) {
                                // HTTP header has been send and Server response header has been handled
                                log_d("[HTTPS] GET... code: %d\n", httpCode);
                                // file found at server
                                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                                  String payload = https.getString();
                                  log_d("Payload: %s", payload.c_str());

                                  https.end();

                                  SpiRamJsonDocument doc(JSON_OBJECT_SIZE(6) + 90);
                                  DeserializationError error = deserializeJson(doc, payload);
                                  // Test if parsing succeeds.
                                  if (error) {
                                    log_e("deserializeJson() failed: %s", error.c_str());
                                    return;
                                  }

                                  const char* coin = doc["coin"];
                                  long reserved = doc["reserved"];
                                  long claimed = doc["claimed"];
                                  long modified = doc["modified"];
                                  float value = doc["value"];
                                  // float escrow = doc["escrow"];

                                  clearScreen();

                                  tft.printf("Coin ID:\n  %s\n", coin);
                                  tft.printf("\nUID Value:\n  %02X:%02X:%02X:%02X:%02X:%02X:%02X\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
                                  tft.printf("\nCurrent Value:\n  %f\n", value);
                                  if(modified != 0){
                                    tft.printf("\nLast modified:\n  %02d:%02d:%02d %02d/%02d/%02d\n", hour(modified),
                                    minute(modified), second(modified), day(modified), month(modified), year(modified));
                                  }
                                  if(reserved != 0){
                                    tft.printf("\nReserved on:\n  %02d:%02d:%02d %02d/%02d/%02d\n", hour(reserved),
                                    minute(reserved), second(reserved), day(reserved), month(reserved), year(reserved));
                                  }
                                  if(claimed != 0){
                                    tft.printf("\nClaimed on:\n  %02d:%02d:%02d %02d/%02d/%02d\n", hour(claimed),
                                    minute(claimed), second(claimed), day(claimed), month(claimed), year(claimed));
                                    tft.printf("\n\nValid coin!\nNow attempting to claim transaction\nPlease wait...");

                                    // Attempt to claim the escrow to the presented coin
                                    log_d("[HTTPS] begin...\n");
                                    String escrowTransactionClaimLoc = "https://coin.polyb.io/transactions/escrow/" + String(transaction) + "/claim";
                                    log_d("%s", escrowTransactionClaimLoc.c_str());
                                    if (https.begin(*client, escrowTransactionClaimLoc)) {  // HTTPS
                                      log_d("[HTTPS] POST...\n");
                                      // start connection and send HTTP header
                                      String body = "{\"key\":\"887dee7b-15a5-40fd-ae00-b852e99c3d49\",\"destination\":\"" + String(coin) + "\"}";
                                      https.addHeader("Content-Type", "application/json");
                                      const char * headerKeys[] = {"location"} ;
                                      const size_t numberOfHeaders = 1;
                                      https.collectHeaders(headerKeys, numberOfHeaders);
                                      int httpCode = https.POST(body);
                                      // httpCode will be negative on error
                                      if (httpCode > 0) {
                                        // HTTP header has been send and Server response header has been handled
                                        log_d("[HTTPS] POST... code: %d\n", httpCode);
                                        // file found at server
                                        if (httpCode == HTTP_CODE_SEE_OTHER) {
                                          String location = https.header("location");
                                          https.end();

                                          clearScreen();
                                          tft.setTextColor(TFT_GREEN, TFT_BLACK);
                                          tft.setTextDatum(MC_DATUM);
                                          tft.drawString(F("Escrow Transaction claimed to coin"), tft.width() / 2, tft.height() / 2);
                                          tft.drawString(F("Obtaining resulting transaction"), tft.width() / 2, (tft.height() / 2) + 10);

                                          // GET resulting transaction details
                                          String tranactionLoc = "https://coin.polyb.io" + location;
                                          log_d("[HTTPS] begin...\n");
                                          if (https.begin(*client, tranactionLoc)) {  // HTTPS
                                            log_d("[HTTPS] GET...\n");
                                            // start connection and send HTTP header
                                            int httpCode = https.GET();
                                            // httpCode will be negative on error
                                            if (httpCode > 0) {
                                              // HTTP header has been send and Server response header has been handled
                                              log_d("[HTTPS] GET... code: %d\n", httpCode);
                                              // file found at server
                                              if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                                                String payload = https.getString();
                                                log_d("Payload: %s", payload.c_str());
                                                SpiRamJsonDocument doc(JSON_OBJECT_SIZE(5) + 180);
                                                DeserializationError error = deserializeJson(doc, payload);
                                                // Test if parsing succeeds.
                                                if (error) {
                                                  log_e("deserializeJson() failed: %s", error.c_str());
                                                  return;
                                                }

                                                const char* transaction = doc["transaction"];
                                                const char* origin = doc["origin"];
                                                const char* destination = doc["destination"];
                                                float value = doc["value"];
                                                long created = doc["created"];

                                                clearScreen();

                                                tft.printf("Escrow Transaction Successfully Claimed!\nResulting finalised transaction details:\n\nTransaction ID:\n  %s\n", transaction);
                                                tft.printf("\nOrigin Coin ID:\n  %s\n", origin);
                                                tft.printf("\nDestination Coin ID:\n  %s\n", destination);
                                                tft.printf("\nTransaction Value:\n  %f\n", value);
                                                tft.printf("\nCreated on:\n  %02d:%02d:%02d %02d/%02d/%02d\n", hour(created),
                                                minute(created), second(created), day(created), month(created), year(created));
                                                tft.printf("\n\nDisconnecting from PolyCoin.");
                                              }
                                            } else {
                                              log_e("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
                                              clearScreen();
                                              tft.setTextColor(TFT_GREEN, TFT_BLACK);
                                              tft.setTextDatum(MC_DATUM);
                                              tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
                                            }
                                            https.end();
                                          } else {
                                            log_w("[HTTPS] Unable to connect\n");
                                            clearScreen();
                                            tft.setTextColor(TFT_GREEN, TFT_BLACK);
                                            tft.setTextDatum(MC_DATUM);
                                            tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
                                          }
                                        }
                                        else{
                                          log_e("Error httpcode not OK");
                                          clearScreen();
                                          tft.setTextColor(TFT_GREEN, TFT_BLACK);
                                          tft.setTextDatum(MC_DATUM);
                                          tft.drawString(F("Could not claim escrow transaction"), tft.width() / 2, tft.height() / 2);
                                          tft.drawString(F("Please try again"), tft.width() / 2, (tft.height() / 2) + 10);
                                        }
                                      }
                                      else{
                                        log_e("Error httpcode error");
                                        clearScreen();
                                        tft.setTextColor(TFT_GREEN, TFT_BLACK);
                                        tft.setTextDatum(MC_DATUM);
                                        tft.drawString(F("Could not claim escrow transaction"), tft.width() / 2, tft.height() / 2);
                                        tft.drawString(F("Please try again"), tft.width() / 2, (tft.height() / 2) + 10);
                                      }
                                    }
                                    else{
                                      log_e("Error opening client for /claim POST");
                                      clearScreen();
                                      tft.setTextColor(TFT_GREEN, TFT_BLACK);
                                      tft.setTextDatum(MC_DATUM);
                                      tft.drawString(F("Error finalising with PolyCoin"), tft.width() / 2, tft.height() / 2);
                                      tft.drawString(F("Please try again"), tft.width() / 2, (tft.height() / 2) + 10);
                                    }
                                  }
                                  else{
                                    tft.printf("\n\nCoin not claimed\nTry again with another coin");
                                  }
                                }
                              } else {
                                log_e("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
                                clearScreen();
                                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                                tft.setTextDatum(MC_DATUM);
                                tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
                              }
                              https.end();
                            } else {
                              log_w("[HTTPS] Unable to connect\n");
                              clearScreen();
                              tft.setTextColor(TFT_GREEN, TFT_BLACK);
                              tft.setTextDatum(MC_DATUM);
                              tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
                            }
                          }
                          else{
                            log_d("Non matching ndef record");
                            clearScreen();
                            tft.setTextColor(TFT_GREEN, TFT_BLACK);
                            tft.setTextDatum(MC_DATUM);
                            tft.drawString(F("Not a valid coin"), tft.width() / 2, tft.height() / 2);
                          }
                        }
                        else{
                          log_d("No ndef record");
                          clearScreen();
                          tft.setTextColor(TFT_GREEN, TFT_BLACK);
                          tft.setTextDatum(MC_DATUM);
                          tft.drawString(F("Coin had no ID"), tft.width() / 2, tft.height() / 2);
                        }
                      }
                      else{
                        log_d("Non matching rfid tag");
                        clearScreen();
                        tft.setTextColor(TFT_GREEN, TFT_BLACK);
                        tft.setTextDatum(MC_DATUM);
                        tft.drawString(F("Not valid coin"), tft.width() / 2, tft.height() / 2);
                      }
                    }
                    else{
                      log_d("RFID Coin not found");
                      clearScreen();
                      tft.setTextColor(TFT_GREEN, TFT_BLACK);
                      tft.setTextDatum(MC_DATUM);
                      tft.drawString(F("Coin not found"), tft.width() / 2, tft.height() / 2);
                    }
                  }
                  else{
                    tft.printf("\nEscrow Transaction Claimed on:\n  %02d:%02d:%02d %02d/%02d/%02d\n", hour(claimed),
                    minute(claimed), second(claimed), day(claimed), month(claimed), year(claimed));
                    tft.printf("\nWith Transaction ID:\n  %s\n", resolvedTransaction);
                    tft.printf("\nTo Coin ID:\n  %s\n", destination);
                  }
                }
              } else {
                log_e("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
                clearScreen();
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.setTextDatum(MC_DATUM);
                tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
              }
              https.end();
            } else {
              log_w("[HTTPS] Unable to connect\n");
              clearScreen();
              tft.setTextColor(TFT_GREEN, TFT_BLACK);
              tft.setTextDatum(MC_DATUM);
              tft.drawString(F("Network Error"), tft.width() / 2, tft.height() / 2);
            }
            // End extra scoping block
          }
          delete client;
        }
    }
    else{
      clearScreen();
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString(F("No Valid Code Read"), tft.width() / 2, tft.height() / 2);
    }
  }
  else{
    // We didnt activate the reader!
    log_e("Could not activate the barcode reader");
    clearScreen();
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(F("Could not activate reader"), tft.width() / 2, tft.height() / 2);
    tft.drawString(F("Contact support"), tft.width() / 2, (tft.height() / 2) + 10);
  }
}

void mainMenu(){
  drawHeader();
  drawFooter("       UP", "SELECT", "DOWN      ");
  clearScreen();
  // ASCII Logo
  tft.print(" _____      _       _     _\n");
  tft.print("|  __ \\    | |     | |   (_)\n");
  tft.print("| |__) |__ | |_   _| |__  _ _   _ ___\n");
  tft.print("|  ___/ _ \\| | | | | '_ \\| | | | / __|\n");
  tft.print("| |  | (_) | | |_| | |_) | | |_| \\__ \\\n");
  tft.print("|_|   \\___/|_|\\__, |_.__/|_|\\__,_|___/\n");
  tft.print("               __/ |\n");
  tft.print("              |___/\n");
  tft.print("      ______ _       _            _\n");
  tft.print("      | ___ (_)     | |          | |\n");
  tft.print("      | |_/ /_  ___ | |_ ___  ___| |__\n");
  tft.print("      | ___ \\ |/ _ \\| __/ _ \\/ __| '_ \\\n");
  tft.print("      | |_/ / | (_) | ||  __/ (__| | | |\n");
  tft.print("      \\____/|_|\\___/ \\__\\___|\\___|_| |_|\n");

  // Draw list
  uint8_t selection = 0;
  uint8_t active = 1;
  while(!selection){
    tft.setCursor(0, 170);
    tft.fillRect(0, 170, tft.width(), 130, TFT_BLACK);

    if(active == 1){
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.print("* Read Coin Data");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("\n\nSweep QRCode Transaction\n\nMint New Coin\n\nShutdown");
    }
    else if(active == 2){
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("Read Coin Data");
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.print("\n\n* Sweep QRCode Transaction");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("\n\nMint New Coin\n\nShutdown");
    }
    else if(active == 3){
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("Read Coin Data\n\nSweep QRCode Transaction");
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.print("\n\n* Mint New Coin");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("\n\nShutdown");
    }
    else if(active == 4){
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("Read Coin Data\n\nSweep QRCode Transaction\n\nMint New Coin");
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.print("\n\n* Shutdown");
    }

    state = 0;
    while(!state){
      delay(20);
    }
    if(state == 1){
      // Up button
      active--;
      if(active < 1){
        active = 4;
      }
    }
    else if(state == 3){
      // Down button
      active++;
      if(active > 4){
        active = 1;
      }
    }
    else if(state == 2){
      selection = active;
    }
    state = 0;
  }

  if(selection == 1){
    readCoinAPIData();
  }
  else if(selection == 2){
    sweepQRCode();
  }
  else if(selection == 3){
    mintNewCoin();
  }
  else if(selection == 4){
    shutdownSystem();
  }

  drawFooter("", "EXIT", "");
  state = 0;
  while(state != 2){
    state = 0;
    delay(10);
  }
  state = 0;
  headerTitle = "";
  drawHeader();
}

void setup() {
  Serial.begin(115200);
  Barcode.begin(115200, SERIAL_8N1, 33, 26);

  // Send command to exit sleep mode on Barcode scanner
  const uint8_t activate[] = {0x7E, 0x00, 0x08, 0x01, 0x00, 0xD9, 0x00, 0xAB, 0xCD};
  Barcode.write(activate, 9);
  delay(10);

  while(Barcode.available()){
    Serial.print(Barcode.read(), HEX);
  }

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

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    log_e("Didn't find PN53x board");
    drawHeader();
    clearScreen();
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(F("ERROR: UNABLE TO FIND RFID READER"), tft.width() / 2, tft.height() / 2);
    tft.drawString(F("SHUTTINGDOWN IN 5 SECONDS"), tft.width() / 2, tft.height() / 2+10);
    delay(5000);
    shutdownSystem();
  }
  // Got ok data, print it out!
  log_d("Found chip PN5%x", (versiondata>>24) & 0xFF); 
  log_d("Firmware ver: %d.%d", (versiondata>>16) & 0xFF, (versiondata>>8) & 0xFF);
  
  // configure board to read RFID tags
  nfc.SAMConfig();

  printIP5306Settings();

  drawHeader();
  drawFooter(F(""), F(""), F(""));
  clearScreen();
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(F("BOOTING, PLEASE WAIT"), tft.width() / 2, tft.height() / 2);

  // SET UP WIFI AND STUFF FOR RESTING REST API
  WiFimulti.addAP("Bifrost", "lolbutts");
  log_d("WiFi Connecting..."); 
  while(WiFimulti.run() != WL_CONNECTED) {
    log_d(".");
    delay(500);
  }
  log_d("WiFi connected, IP address: %s", WiFi.localIP().toString().c_str());

  timeClient.begin();
  if(!tryToConnectNTP()){
    log_w("Could not connect to NTP server, waiting 3 seconds then trying again.");
    delay(3000);
    if(!tryToConnectNTP()){
      log_w("Could not connect a second time, aborting");
    }
    else{
      log_d("NTP Updated");
      isNTPSet = true;
    }
  }
  else{
    log_d("NTP Updated");
    isNTPSet = true;
  }

  drawHeader();
  clearScreen();
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(F("WiFi Connected, IP:"), tft.width() / 2, tft.height() / 2);
  tft.drawString(WiFi.localIP().toString(), tft.width() / 2, (tft.height() / 2)+10);

  delay(3000);

  //spisd_test();
  buzzer_test();
  button_init();

  btnscanT.attach_ms(30, button_loop);
  redrawHFT.attach(60, backgroundHeaderFooterDraw);
  connectNTP.attach(61, tryToConnectNTPTickerCall);

  log_d("Total heap: %d", ESP.getHeapSize());
  log_d("Free heap: %d", ESP.getFreeHeap());
  log_d("Total PSRAM: %d", ESP.getPsramSize());
  log_d("Free PSRAM: %d", ESP.getFreePsram());
  log_d("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
  log_d("Flash chip size: %d", ESP.getFlashChipSize());
}

void loop() {
  mainMenu();
  log_d("Minimum heap so far: %d", ESP.getMinFreeHeap());
}