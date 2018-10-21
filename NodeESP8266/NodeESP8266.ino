/***********************************************************************************
 *
 * GPIO PIN 
 * RESET 3  GPIO4
 * LED   2  GPIO3
 * 
 * 
 * Matrix P10
 * CLK_PIN		14   GPIO14 or SCK
 * DATA_PIN	  13   GPIO13 or MOSI
 * CS_PIN		  15   GPIO15      // or SS
 * NOE_PIN    5    GPIO5
 * A_PIN      4    GPIO4
 * B_PIN      12   GPIO12
 * Note: SPIDMD(byte panelsWide, byte panelsHigh, byte pin_noe, byte pin_a, byte pin_b, byte pin_sck);
 **********************************************************************************/

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
// #include <WiFi.h>
#include "eeprom.h"
#include "spiffs.h"
#include <ArduinoJson.h>

#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD2.h>        //
#include <fonts/Font_1.h>

#include <FS.h>
#include <Ticker.h>
Ticker secondTick;


// #define DEBUG_ESP_HTTP_SERVER
//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 1
//SPIDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, 5, 4, 12, 15);  // DMD controls the entire displaySPIDMD
//SPIDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, 16, 4, 12, 0);  // DMD controls the entire displaySPIDMD
SPIDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, 16, 5, 12, 4);  // DMD controls the entire displaySPIDMD

ESP8266WebServer server(80);
// WiFiServer   ;


#define RESET 0
#define LED 2
#define DIRECTION_PIN 3
#define WIFI_PIN A0
#define DEBUGGING true
#define MODE_STATION false
#define SPIFFS_MODE true

#define DataFile "/data.json"


IPAddress staticIP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);


const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

#define ADDR 0
#define ADDR_APSSID ADDR
#define ADDR_APPASS (ADDR_APSSID+20)
#define ADDR_PASS_LOGIN (ADDR_APPASS + 20)
#define ADDR_JSON_MESSAGE (ADDR_PASS_LOGIN + 20)
#define ADDR_TIME_NEXT_MSG 504 // 4 byte
#define ADDR_LIGHT_MATRIX 506 // 1 byte


#define NAME_DEFAULT "MBELL"
#define PASS_DEFAULT "1234567890"
//#define STA_SSID_DEFAULT "Gear"
//#define STA_PASS_DEFAULT "Quan1995"
#define STA_SSID_DEFAULT "TTQ"
#define STA_PASS_DEFAULT "0987654321"
#define AP_SSID_DEFAULT NAME_DEFAULT
#define AP_PASS_DEFAULT "1234567890"

#define PASS_LOGIN_DEFAULT ""

#define TIME_LIMIT_RESET 3000
#define TIME_TIME_NEXT_MSG_DEFAULT 60000

#define LIGHT_MATRIX_DEFAULT 255

bool flagClear = false;
bool isReconnectAP = false;

bool isLogin = false;
String staSSID, staPASS;
String apSSID, apPASS;
String SoftIP, LocalIP;
String passLogin;
byte lightMatrix;
int idWebSite = 0;
long timeLogout = 120000;
long timeNextMessage = 60000;
long t, tNext, tMotion;

// StaticJsonBuffer<1024> JSONBuffer; //Memory pool
DynamicJsonBuffer JSONBuffer;
JsonObject& parsed = JSONBuffer.createObject();
// JsonArray& parsed["arr"] = JSONBuffer.createArray();
#define MAX_LENGTH_JSON_MESSAGE (512 - ADDR_JSON_MESSAGE)
String JSONMessage;
int currentIndex, showCurrentIndex;

#define FONT_SIZE 1
String Fonts[FONT_SIZE] = {"Font_1"};
String startMsg = "I ready!";

String currentMotion = "stop";
long repeatMotion;
long baudMotion;
int showCurrentMessage = 0;
int lengthMessageActive ;

volatile int watchdogCount = 0;
void ISRwatchdog() {
  watchdogCount++;
  // Serial.println("Ticker!" + String(watchdogCount));
  if ( watchdogCount >= 2 ) {
    // Only print to serial when debugging
    Serial.println("The dog bites!");
    //     ESP.reset();
    // ESP.restart();
    ESP.reset();
  }
}
bool wifi = true;
String pinDirection = "all";
bool digitalDirection;

char* nameMessage;
int marginTop;
int marginLeft;

void setup()
{
  delay(2000);
  Serial.begin(115200);
  Serial.println();
  BeginEEPROM();
  if (SPIFFS.begin())
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
  show("Start:" + String(ADDR_JSON_MESSAGE) + " Lenght:" + String(512 - ADDR_JSON_MESSAGE));
  Serial.println(ESP.getCpuFreqMHz());
  GPIO();
  idWebSite = 0;
  isLogin = false;
  if (EEPROM.read(511) == EEPROM.read(0) || !isExistsFile(DataFile) || flagClear) {
    Serial.println("ConfigDefault!");
    ClearEEPROM();
    ConfigDefault();
    WriteConfig();
  }
  ReadConfig();
#if MODE_STATION
  WiFi.mode(WIFI_AP_STA);
  ConfigNetwork();
  ConnectWifi(STA_SSID_DEFAULT, STA_PASS_DEFAULT, 15000);
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    show("Set WIFI_AP");
  }
#else
  if (analogRead(WIFI_PIN) > 500) {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    AccessPoint();
  } else {
    WiFi.mode(WIFI_OFF);
    show("WIFI_OFF");
  }
#endif
  StartServer();

  // String JSONMessage = "{'arr':[{status:true,name:'nguyen ',font:'Font 2',light:40,motion:'left',baud:400},{status:true,name:'nguyen  quan',font:'Font 2',light:40,motion:'left',baud:400},{status:true,name:'nguyensd  quan',font:'Font 2',light:40,motion:'left',baud:400},{status:true,name:'nguyen 4234234 quan',font:'Font 2',light:40,motion:'left',baud:400}]}";
  // String JSONMessage = "[{'status':'true','name':'nguyen van quan','font':'Font 2','light':'40','motion':'left','baud':'400'}]";
  // char JSONMessage[] = "[{\"status\":\"true\",\"name\":\"nguyen van quan\",\"font\":\"Font 2\",\"light\":\"40\",\"motion\":\"left\",\"baud\":\"400\"},{\"status\":\"true\",\"name\":\"nguyen van quan\",\"font\":\"Font 2\",\"light\":\"40\",\"motion\":\"left\",\"baud\":\"400\"},{\"status\":\"true\",\"name\":\"nguyen van quan\",\"font\":\"Font 2\",\"light\":\"40\",\"motion\":\"left\",\"baud\":\"400\"}]";
  show("arr size:" + String(parsed["arr"].size()));
  String json1 = "";
  parsed["arr"].printTo(json1);
  show(json1);
  dmd.setBrightness(lightMatrix);
  dmd.begin();
  //  clear/init the DMD pixels held in RAM
  dmd.clearScreen();   //true is normal (all pixels off), false is negative (all pixels on)
  dmd.selectFont(Font_1);
  nameMessage = dmd.ConvertStringToArrayChar(startMsg, false);
  dmd.drawString(0,0, nameMessage, GRAPHICS_ON);
  refreshShowMessage();
  //  secondTick.attach(5, ISRwatchdog);
  //  ESP.wdtDisable();
  //  ESP.wdtEnable(WDTO_8S);
  //  delay(20000);
  //  if (wifi) {
  //    show("WIFI_OFF");
  //    wifi = false;
  ////    WiFi.softAPdisconnect(false);
  //    WiFi.enableAP(false);
  //  }
  show("End Setup()");

}
long tCheckClient;
int listClient;
int tWatchDog;
long countRepeat;
long tRepeat = 0;
long strLengthMessage = 0;

void loop()
{

  //  if (millis() - tWatchDog > 1000) {
  //    watchdogCount = 0;
  //    tWatchDog = millis();
  //  }

  dnsServer.processNextRequest();

  #if MODE_STATION
    server.handleClient();
    return;
  #endif

  if (listClient > 0) {
    server.handleClient();
  }

  if (millis() - tCheckClient > 2000) {
    if (listClient != WiFi.softAPgetStationNum()) {
      listClient = WiFi.softAPgetStationNum();
      show("listClient" + String(listClient));
      if (listClient > 0) {
        dmd.end();
        show("dmd.end");
      } else {
        dmd.begin();
        show("dmd.begin");
        refreshShowMessage();
      }
    }
    tCheckClient = millis();
  }
  if (millis() - t > timeLogout) {
    isLogin = false;
    t = millis();
  }

  if (listClient == 0 && lengthMessageActive > 0) {
    if (millis() - tMotion > baudMotion) {
      countRepeat++;
      if (lengthMessageActive > 1 && countRepeat > (currentMotion.equals("blink") ? (2 * repeatMotion) : (repeatMotion * strLengthMessage))) {
        showMatrix();
        countRepeat = 1;
      }else {
        onMotion();
      }
      show("countRepeat:" + String(countRepeat));
      tMotion = millis();

    }
  }
  if (digitalRead(RESET) == LOW)
  {
    long t = TIME_LIMIT_RESET/100;
    while (digitalRead(RESET)==LOW && t-- >= 0){
      delay(100);
    }
    if (t < 0){
      show("RESET MODE");
      while (digitalRead(RESET) == LOW){delay(100);}
      ClearEEPROM();
      setup();
    }
  }
  // delay(1);
}

void show(String s)
{
  #ifdef DEBUGGING
    Serial.println(s);
  #endif
}
void onMotion() {
  if (currentMotion.equals("left")) {
    dmd.marqueeScrollX(1);
  } else if (currentMotion.equals("right")) {
    dmd.marqueeScrollX(-1);
  } else if (currentMotion.equals("up")) {
    dmd.marqueeScrollY(1);
  } else if (currentMotion.equals("down")) {
    dmd.marqueeScrollY(-1);
  } else if (currentMotion.equals("blink")) {
    if (countRepeat % 2 == 1) {
      dmd.drawString(marginLeft, marginTop, nameMessage, GRAPHICS_ON);
    } else {
      // dmd.drawString(marginLeft, marginTop, "                 ", GRAPHICS_ON);
      dmd.clearScreen();
    }
  }
}
void showMatrix() {
  int lengthMessage = getLengthMessage();
  if (lengthMessage > 0) {
    JsonArray& arrayMessageActive = listMessageActiveStatus();
    lengthMessageActive = arrayMessageActive.size();
    // show("MessageActiveStatus: " + String(lengthMessageActive));
    if (lengthMessageActive > 0) {
      String stg = "";
      arrayMessageActive.printTo(stg);
      // show(stg);
      stg = "";
      show("Get showCurrentIndex : " + String(showCurrentIndex));
      JsonObject& objectMessage = arrayMessageActive[showCurrentIndex];
      // show("objectMessage:");
      (arrayMessageActive[showCurrentIndex]).printTo(stg);
      // show(stg);
      showCurrentIndex++;
      matrixSeting(objectMessage);
      if (showCurrentIndex >= lengthMessageActive) {
        showCurrentIndex = 0;
      }
    } else {
      nameMessage = dmd.ConvertStringToArrayChar(startMsg, false);
      dmd.drawString(0,0, nameMessage, GRAPHICS_ON);
      show("Show default message!");

    }
  }
}
void refreshShowMessage() {
  show("refreshShowMessage");
  showCurrentIndex = 0;
  tNext = millis();
  showMatrix();
}
void matrixSeting(JsonObject& message) {
  show("matrixSeting: ");
  String strMessage = "";
  message.printTo(strMessage);
  show(strMessage);
  if (strMessage.length() <= 10) {
    show("strMessage.length() <= 10");
    refreshShowMessage();
    setup();
  }
  String strName = message["name"];
  if (strName.length() > 0) {
    dmd.clearScreen(); // No clear screen when transfer next message.
    String font = message["font"];
    nameMessage = dmd.ConvertStringToArrayChar(strName, false);
    strLengthMessage = dmd.stringWidth(string2char(strName), (uint8_t*)string2char(font));
    show("stringWidth : " + String(strLengthMessage));
    marginTop = message["top"];
    marginLeft = message["left"];
    dmd.drawString(marginLeft, marginTop, nameMessage, GRAPHICS_ON);
    if (font.equals("Font_1")) {
      dmd.selectFont(Font_1);
    }
    int light = message["light"];
//    dmd.setBrightness(light);
    repeatMotion = message["repeat"];
    baudMotion = message["baud"];
    String motion = message["motion"];
    currentMotion = motion;
  }
}
char* string2char(String command){
  char *szBuffer = new char[command.length()+1];
     strcpy(szBuffer,command.c_str( ));
  return szBuffer;
}

void blinkLed(int numberRepeat, long tdelay) {
  int i=0;
  while (i++ < numberRepeat) {
    digitalWrite(LED,HIGH);delay(tdelay);
    digitalWrite(LED,LOW);delay(tdelay);
  }
}
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}
bool handleFileRead(String path) {
  show("handleFileRead: " + path);
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return false;
  }
  if (path.endsWith("/")) {
    path += "index.html";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  Serial.println(server.hostHeader());
  if (server.hostHeader() != "192.168.4.1") {
    Serial.print("Request redirected to captive portal");
    server.sendHeader("Location", String("http://192.168.4.1/"), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}
void GPIO()
{
  show("GPIO");
  pinMode(LED,OUTPUT);
  digitalWrite(LED, HIGH);
  pinMode(RESET,INPUT_PULLUP);
  pinMode(DIRECTION_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(DIRECTION_PIN), handleInterruptVT, CHANGE);
  setDirection(digitalRead(DIRECTION_PIN));
}

bool flagInterrupt;
//This program get executed when interrupt is occures i.e.change of input state
void handleInterruptVT() {
  if (flagInterrupt)
    return;
  flagInterrupt = true;
  if (digitalRead(DIRECTION_PIN) != digitalDirection) {
     setDirection(digitalRead(DIRECTION_PIN));
     refreshShowMessage();
  }

  flagInterrupt = false;
}

void setDirection(bool dir) {
  digitalDirection = dir;
  if (dir) {
      pinDirection = "prev";
  } else {
    pinDirection = "next";
  }
  show("pinDirection:" + pinDirection);
}
void delayMs(int x)   {
  for(int i=0; i<=x; i++) {
    delayMicroseconds(1000);
  }
}

int ListenIdRF()
{
  int Di = 0;
  // if (digitalRead(VT) == HIGH)
  // {
  //   while (digitalRead(VT) == HIGH) {
  //     delayMs(10);
  //   }
  // }
  return Di;
}

void initialJson(String strJson) {
  show("initialJson :");
  // show(strJson);
  JSONBuffer.clear();
  JsonObject& parsed1 = JSONBuffer.parseObject(strJson);   //Parse message
  if (!parsed1.success()) {      //Check for errors in parsing
    show("Parsing failed");
//    ClearEEPROM();
//    ConfigDefault();
//    WriteConfig();
  }
  parsed["arr"] = parsed1["arr"];
}
void DeleteMessage(int index) {
  JsonArray& array1 = parsed["arr"];
  show("Delete item: " + String(index));
  if (index >= 0) {
    array1.remove(index);
    show("True!");
    return;
  }
  show("False!");
}
void UpdateMessage(int index, String field, String value) {
  JsonObject &object = parsed["arr"][index];
  object[field] = value;
}
void UpdateMessage(int index, String field, bool value) {
  JsonObject &object = parsed["arr"][index];
  object[field] = value;
}
int getLengthMessage() {
  return parsed["arr"].size();
}
JsonObject& getMessageByIndex(int index) {
  JsonObject& tg = JSONBuffer.createObject();
  if (index < getLengthMessage()) {
    return parsed["arr"][index];
  }
  return tg;
}
JsonArray& listMessageActiveStatus() {
  // JSONBuffer.clear();
  // DynamicJsonBuffer JSONBuffer1;
  // StaticJsonBuffer<1024> JSONBuffer1;
  JsonArray& tg = JSONBuffer.createArray();
  int len = parsed["arr"].size();

  for (int i = 0; i < len; i++) {
    JsonObject& item = parsed["arr"][i];
    bool status = item["status"];
    String dir = item["dir"];
    if (!!status && (pinDirection.equals(dir) || dir.equals("all")) ) {
      tg.add(item);
    }
  }
  return tg;
}
void ConfigDefault()
{
  isLogin = false;
  apSSID = AP_SSID_DEFAULT;
  apPASS = AP_PASS_DEFAULT;
  passLogin =  PASS_LOGIN_DEFAULT;
  parsed["arr"] = JSONBuffer.createArray();
  timeNextMessage = TIME_TIME_NEXT_MSG_DEFAULT;
  lightMatrix = LIGHT_MATRIX_DEFAULT;
  // JSONMessage = "{'arr':[{status:true,name:'nguyen ',font:'Font 2',light:40,motion:'left',baud:400},{status:true,name:'nguyen  quan',font:'Font 2',light:40,motion:'left',baud:400},{status:true,name:'nguyensd  quan',font:'Font 2',light:40,motion:'left',baud:400},{status:true,name:'nguyen 4234234 quan',font:'Font 2',light:40,motion:'left',baud:400}]}";
  show("Config Default");
}
void WriteConfig()
{
  SaveStringToEEPROM(apSSID, ADDR_APSSID);
  SaveStringToEEPROM(apPASS, ADDR_APPASS);
  SaveStringToEEPROM(passLogin, ADDR_PASS_LOGIN);
  String JSONMessage = "";
  parsed.printTo(JSONMessage);

  #if SPIFFS_MODE
    writeFile(DataFile, "w", JSONMessage);
  #else 
    SaveStringToEEPROM(JSONMessage, ADDR_JSON_MESSAGE);
  #endif
  
  SaveLongToEEPROM(timeNextMessage, ADDR_TIME_NEXT_MSG);
  SaveCharToEEPROM(lightMatrix, ADDR_LIGHT_MATRIX);
  show("Write Config");
//  getJSONFromEEPROM();
}
void ReadConfig()
{
  apSSID = ReadStringFromEEPROM(ADDR_APSSID);
  apPASS = ReadStringFromEEPROM(ADDR_APPASS);
  passLogin = ReadStringFromEEPROM(ADDR_PASS_LOGIN);
  timeNextMessage = ReadLongFromEEPROM(ADDR_TIME_NEXT_MSG);
  lightMatrix = ReadCharFromEEPROM(ADDR_LIGHT_MATRIX);
  show("Read Config");
  show("Access Point: \n" + apSSID + "\n" + apPASS);
  show("Pass login: \n" + passLogin);
  show("Time next message: \n" + String(timeNextMessage));
  show("Light matrix:\n" + String(lightMatrix));
  getJSONFromEEPROM();
}

void getJSONFromEEPROM() {
  String JSONMessage = "";
  #if SPIFFS_MODE 
    JSONMessage = readFile(DataFile);
  #else 
    JSONMessage = ReadStringFromEEPROM(ADDR_JSON_MESSAGE);
  #endif
  show("Json Message: \n" + String(JSONMessage.length()));
  show(JSONMessage);
  initialJson(JSONMessage);
}

void WiFiOn() {
  WiFi.mode(WIFI_AP);
  AccessPoint();
}


void WiFiOff() {
    //Serial.println("diconnecting client and wifi");
    //client.disconnect();
      WiFi.forceSleepBegin();
  WiFi.mode(WIFI_OFF);
//  WiFi.softAPdisconnect(true);
  delay(1);
}
void AccessPoint()
{
  show("Access Point Config");
  //WiFi.disconnect();
  delay(1000);
  // Wait for connection
  show( WiFi.softAP(apSSID.c_str(), apPASS.c_str()) ? "Ready" : "Failed!");
  IPAddress myIP = WiFi.softAPIP();
  show("AP IP address: ");
  SoftIP = "" + (String)myIP[0] + "." + (String)myIP[1] + "." + (String)myIP[2] + "." + (String)myIP[3];
  show(SoftIP);
}

void ConnectWifi(String ssid, String password, long timeOut)
{
  show("Connect to other Access Point");
  // delay(1000);
  int count = timeOut / 500;
  show("Connecting");
  show(ssid + "-" + password);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED && --count > 0) {
    delay(500);
    Serial.print(".");
  }
  if (count > 0) {
    show("Connected");
    IPAddress myIP = WiFi.localIP();
    String LocalIP = "" + (String)myIP[0] + "." + (String)myIP[1] + "." + (String)myIP[2] + "." + (String)myIP[3];
    show("Local IP :");
    show(LocalIP);
  } else {
    show("Disconnect");
  }
}
void ConfigNetwork(){
 // Fire up wifi station
  show("Station configuration ... ");
  WiFi.config(staticIP, gateway, subnet);
}
void StartServer()
{
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/login", login);
  server.on("/isLogin", checkLogin);
  server.on("/logout", logout);
  server.on("/setting", websetting);
  server.on("/matrixSetting", matrixSetting);
  server.on("/createMessage", createMessage);
  server.on("/listMessage", webListMessage);
  server.on("/getSettings", getSettings);
  server.on("/restart", restartDevice);
  server.on("/verifyDelete", verifyDeleteMessage);
  server.on("/updateStatus", updateStatus);
  //  server.onNotFound(handleNotFound);
  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  server.begin();
  show("HTTP server started");
  
}

void getSettings() {
  String strfonts = "[";
  for (int i = 0; i < FONT_SIZE; i++) {
    strfonts += "\"" + Fonts[i] + "\"";
    if (i < FONT_SIZE - 1) {
      strfonts += ",";
    }
  }
  strfonts += "]";
  String strDirections = "[{\"name\":\"Chiều đi\", \"value\": \"next\"},{\"name\":\"Chiều về\", \"value\":\"prev\"},{\"name\":\"Tất cả\", \"value\":\"all\"}]";
  String motions = "[{\"name\":\"Không\", \"value\":\"stop\"},{\"name\":\"Trái qua phải\", \"value\":\"left\"},{\"name\":\"Phải qua trái\", \"value\":\"right\"},{\"name\":\"Trên xuống dưới\", \"value\":\"up\"},{\"name\":\"Dưới lên trên\", \"value\":\"down\"},{\"name\":\"Nhấp nháy\", \"value\":\"blink\"}]";
  String json = "{\"txtFonts\":" + strfonts + ",\
                  \"txtMotions\":" + motions + ",\
                  \"txtDirections\":" + strDirections + ",\
                  \"txtMinBaud\":" + String(50) + ",\
                  \"txtMaxBaud\":" + String(1000) + "}";
  show(json);
  server.send(200, "application/json", json);
}
void login() {
  // // dmd.end();
  GiaTriThamSo();
  String json = "{\"isLogin\":" + String(isLogin ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

void checkLogin() {
  String json = "{\"isLogin\":" + String(isLogin ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

void logout() {
  isLogin = false;
  checkLogin();
  show("Logout!");
}

void websetting() {
  GiaTriThamSo();
  String json = "{\"txtAPName\":\"" + apSSID + "\",\
                \"txtAPPass\":\"" + apPASS + "\",\
                \"txtPassLogin\":\"" + passLogin + "\"}";
  server.send(200, "application/json", json);
}
void matrixSetting() {
  GiaTriThamSo();
  String json = "{\"txtDisplayAcross\":" + String(DISPLAYS_ACROSS) + ",\
                \"txtDisplayDown\":" + DISPLAYS_DOWN + ",\
                \"txtLightMessage\":" + lightMatrix + ",\
                \"txtMinLight\":" + String(2) + ",\
                \"txtMaxLight\":" + String(255) + "}";
  server.send(200, "application/json", json);
}

void createMessage() {
  int lengthMessage = parsed["arr"].size();
  String json = "";
  if (server.method() == HTTP_GET) {
    json = "{\"txtIndexMessage\":" + String(lengthMessage) + ",\
              \"chboxStatusMessage\":false,\
              \"txtNameMessage\": \"New message\",\
              \"txtFontMessage\":\"" + Fonts[0] + "\",\
              \"chboxMotionMessage\":\"stop\",\
              \"txtDirectionMessage\": \"prev\",\
              \"txtMarginTopMessage\":" + String(0) + ",\
              \"txtMarginLeftMessage\":" + String(0) + ",\
              \"txtRepeatMessage\":" + String(1) + ",\
              \"txtBaudMessage\":" + String(200) + "}";
  } else {
    GiaTriThamSo();
    int newLengthMessage = parsed["arr"].size();
    json = "{\"btnSaveMessage\":" + String(newLengthMessage > lengthMessage ? "true" : "false") + "}";
  }
  server.send(200, "application/json", json);
}
void webListMessage() {
  String json = "";
  parsed["arr"].printTo(json);
  server.send(200, "application/json", json);
}
void restartDevice() {
  GiaTriThamSo();
  String json = "{\"restartDevice\": true}";
  server.send(200, "application/json", json);
}
void verifyDeleteMessage() {
  int lengthMessage = parsed["arr"].size();
  GiaTriThamSo();
  int newLengthMessage = parsed["arr"].size();
  String json = "{\"btnDeleleMessage\":" + String(newLengthMessage < lengthMessage ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}
void updateStatus() {
  GiaTriThamSo();
  String json = "{\"btnSaveList\": true}";
  server.send(200, "application/json", json);
}
void GiaTriThamSo()
{
  t = millis();
  String message = "";
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  String UserName, PassWord;

  int lengthMessage = parsed["arr"].size();
  JsonObject &object = JSONBuffer.createObject();
  bool isbtnSaveSetting = false, isbtnSaveMessage = false, istxtVerifyDelete = false, isbtnSaveList = false;
  for (uint8_t i = 0; i < server.args(); i++) {

    String Name = server.argName(i);
    String Value = String(server.arg(i)) ;
    String s1 = Name + ": " + Value;
    show("--" + s1);
    if (isLogin == true) {
      if (Name.indexOf("txtLogout") >= 0) {
        isLogin = false;
        show("Logout");
      }
      else if (Name.indexOf("txtBack") >= 0) {
        idWebSite = 1;
        show("Back");
      }
      else if (Name.indexOf("txtAPName") >= 0) {
        if (Value != staSSID && Value != apSSID && Value.length() > 0) {
          apSSID = Value;
          show("Set apSSID : " + Value);
        }
      }
      else if (Name.indexOf("txtAPPass") >= 0) {
        if (Value != apPASS) {
          if (Value.length() >= 8) { // Length password >= 8
            apPASS =  Value ;
            show("Set apPASS : " + apPASS);
          } else {
            show("txtAPPass is invalid (Value.length() >= 8 && Value != apPASS)");
          }
        }
      }
      else if (Name.indexOf("txtPassLogin") >= 0) {
        if (Value != passLogin) {
          passLogin = Value;
          show("Set passLogin : " + Value);
        }
      }
      else if (Name.indexOf("txtTimeNextMsg") >= 0){
        if (Value.equals(String(timeNextMessage)) == false) {
          timeNextMessage = atol(Value.c_str());
          show("Set timeNextMessage : " + timeNextMessage);
        }
      }
      else if (Name.indexOf("txtRestart") >= 0){
        idWebSite = 2;
        show("Verify restart");
        show(Value);
      }

      else if (Name.indexOf("txtIndexMessage") >= 0)
      {
        currentIndex = (Value.toInt()) & 0xff;
        show("txtIndexMessage: " + String(currentIndex));
      }
      else if (Name.indexOf("txtAddMessage") >= 0)
      {
        if ( Value.indexOf("true") >= 0 ) {
          int length = parsed["arr"].size();
          currentIndex = length;
          show("txtAddMessage: " + Value);
        }
      }
      else if (Name.indexOf("txtVerifyDelete") >= 0)
      {
        if ( Value.indexOf("true") >= 0 ) {
          istxtVerifyDelete = true;
        }
      }
      else if (Name.indexOf("updatechboxStatus") >= 0)
      {
        bool status = false;
        if ( Value.indexOf("true") >= 0 ) {
          status = true;
        }
        show("Set chboxStatus " + String(currentIndex) + " :" + Value);
        UpdateMessage(currentIndex, "status", status);
      }
      else if (Name.indexOf("chboxStatusMessage") >= 0) {
        object["status"] = ( Value.equals("true") ? true : false);
        show("Set chboxStatusMessage : " + Value);
      }
      else if (Name.indexOf("txtNameMessage") >= 0) {
        object["name"] = Value;
        show("Set txtNameMessage : " + Value);
      }
      else if (Name.indexOf("txtFontMessage") >= 0) {
        object["font"] = Value;
        show("Set txtFontMessage : " + Value);
      }
      else if (Name.indexOf("txtDirectionMessage") >= 0) {
        object["dir"] = Value;
        show("Set txtDirectionMessage : " + Value);
      }
      else if (Name.indexOf("txtLightMessage") >= 0) {
        if (Value.equals(String(lightMatrix)) == false) {
          lightMatrix = Value.toInt();
          show("Set lightMatrix : " + String(lightMatrix));
        }
      }
      else if (Name.indexOf("chboxMotionMessage") >= 0) {
        object["motion"] = Value;
        show("Set chboxMotionMessage : " + Value);
      }
      else if (Name.indexOf("txtRepeatMessage") >= 0) {
        object["repeat"] = Value.toInt();
        show("Set txtRepeatMessage : " + Value);
      }
      else if (Name.indexOf("txtMarginTopMessage") >= 0) {
        object["top"] = Value.toInt();
        show("Set txtMarginTopMessage : " + Value);
      }
      else if (Name.indexOf("txtMarginLeftMessage") >= 0) {
        object["left"] = Value.toInt();
        show("Set txtMarginLeftMessage : " + Value);
      }
      else if (Name.indexOf("txtBaudMessage") >= 0) {
        object["baud"] = Value.toInt();
        show("Set txtBaudMessage : " + Value);
      }
      else if (Name.indexOf("btnSaveMessage") >= 0) {
        isbtnSaveMessage = true;
      }
      else if (Name.indexOf("btnSaveSetting") >= 0)
      {
        isbtnSaveSetting = true;
      }
      else if (Name.indexOf("txtVerifyRestart") >= 0)
      {
        if ( Value.indexOf("true") >= 0 ) {
          show("txtVerifyRestart:" + Value);
          show("Restart Device");
          setup();
        }
      }
      else if (Name.indexOf("btnSaveList") >= 0) {
        if ( Value.indexOf("true") >= 0 ) {
          isbtnSaveList = true;
        }
      }

    } else {
      if (Name.indexOf("txtUsername") >= 0) {
        UserName =  Value ;
        show("Set UserName : " + UserName);
      }
      else if (Name.indexOf("txtPassword") >= 0) {
        PassWord =  Value ;
        show("Set Password : " + PassWord);
      }

      if (UserName.equals(apSSID) && PassWord.equals(passLogin)) {
        isLogin = true;
        show("Login == true");
      } else {
        isLogin = false;
      }
    }
    Name = "";
    Value = "";
  }
  if (isLogin == false) {
    idWebSite = 0;
  }
  if (isbtnSaveSetting) {
    dmd.setBrightness(lightMatrix);
    WriteConfig();
    show("Save config");
  } else if (isbtnSaveMessage) {
    show("Set btnSaveMessage: true");
    String strJson = "";
    //     object.printTo(strJson);
    //     show("object:" + strJson);
    JsonArray& array1 = parsed["arr"];
    bool isEdit = currentIndex < lengthMessage;
    if (isEdit) {
      JsonObject& object1 = parsed["arr"][currentIndex];
      array1[currentIndex] = object;
      show("Edit message");
    } else {
      if (array1.add(object)) {
        show("Add message success!");
      } else {
        show("Add message fail!");
      }
      parsed["arr"].printTo(strJson);
      show("arr:" + strJson);
    }
    //    show("object:" + strJson);
    //    parsed.printTo(strJson);
    // show(strJson);
    #if !SPIFFS_MODE
      if (strJson.length() > MAX_LENGTH_JSON_MESSAGE) {
        array1.remove(array1.size() - 1); // Out EEPROM => revert array1.add(object);
        show("Out EEPROM");
      }
    #endif
      
    WriteConfig();
    show("Save config");
    refreshShowMessage();
  } else if (istxtVerifyDelete) {
    show("txtVerifyDelete: true");
    DeleteMessage(currentIndex);
    WriteConfig();
    show("Save config");
  } else if (isbtnSaveList) {
    show("Save List Message to EEPROM!");
    WriteConfig();
    refreshShowMessage();
  }

}
void handleNotFound() {
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
}
