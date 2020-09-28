#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "T4_V13.h"
#include "WiFi.h"
#include <Button2.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <Ticker.h>
#include <driver/uart.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>


WiFiMulti WiFiMulti;

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
  pBtns[0].setLongClickHandler([](Button2 &b) {
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

    delay(6000);
    digitalWrite(TFT_BL, !r);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_1, LOW);
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

void drawScreenLayout(){
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.drawLine(0, 20, 239, 20, TFT_ORANGE); // Top divider
  tft.drawLine(0, 309, 239, 309, TFT_ORANGE); // Bottom divider
}

void setup() {
  Serial.begin(115200);
  Barcode.begin(115200, SERIAL_8N1, 33, 26);

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

  drawScreenLayout();
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("WiFi Connected, IP:", tft.width() / 2, tft.height() / 2);
  tft.drawString(WiFi.localIP().toString(), tft.width() / 2, (tft.height() / 2)+10);

  delay(3000);

  //spisd_test();
  buzzer_test();
  button_init();

  btnscanT.attach_ms(30, button_loop);
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
                    Serial.println(payload);
                    drawScreenLayout();
                    tft.setTextColor(TFT_GREEN, TFT_BLACK);
                    tft.setTextDatum(TL_DATUM);
                    if(payload.length()<=40){
                      tft.drawString(payload, 0, tft.height() / 2);
                    }
                    else{
                      uint8_t line = 0;
                      uint8_t stringlen = 0;
                      for(uint8_t x = 0; x < payload.length(); x += 40){
                        if(payload.length() - x < 40){
                          stringlen = payload.length();
                        }
                        else{
                          stringlen = 40 * (x + 1);
                        }
                        tft.drawString(payload.substring(40*line, stringlen), 0, (tft.height() / 2)+(8*line));
                        line++;
                      }
                    }
                  }
                } else {
                  Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
                  drawScreenLayout();
                  tft.setTextColor(TFT_GREEN, TFT_BLACK);
                  tft.setTextDatum(MC_DATUM);
                  tft.drawString("Network Error", tft.width() / 2, tft.height() / 2);
                }
                https.end();
              } else {
                Serial.printf("[HTTPS] Unable to connect\n");
                drawScreenLayout();
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
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("Network Error", tft.width() / 2, tft.height() / 2);
          }
        }
        else{
          drawScreenLayout();
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setTextDatum(MC_DATUM);
          tft.drawString("Coin had no address", tft.width() / 2, tft.height() / 2);
        }
      }
    }
    else{
      drawScreenLayout();
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Could not read Coin", tft.width() / 2, tft.height() / 2);
    }
  }
}