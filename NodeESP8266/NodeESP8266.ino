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
#define ADDR_LIGHT_MATRIX 510 // 1 byte


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
  if (EEPROM.read(511) == EEPROM.read(0) || flagClear) {
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
  } else if (currentMotion.equals("bottom")) {
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
    //     dmd.clearScreen(); // No clear screen when transfer next message.
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
String readFile(String fileName) {
  // we could open the file
  File f = SPIFFS.open(fileName, "r");
  String content = "";
  if (f) {
    while (f.available()) {
      content += f.readStringUntil('\n');
    }
    f.close();
  }
  return content;
}
bool writeFile(String fileName, String modeFile, String content) {
  File f = SPIFFS.open(fileName.c_str(), modeFile.c_str());
  if (f) {
    //Write data to file
    Serial.println("Open file!");
    f.println(content);
    f.close();  //Close file
    return true;
  }
  return false;
}
void removeFile(String path) {
  if (SPIFFS.remove(path)) {
    show("Remove file "+ path);
  } else {
    show("Not exists " + path);
  }

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
  lightMatrix = ReadCharFromEEPROM(LIGHT_MATRIX_DEFAULT);
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
                \"txtMinLight\":" + String(1) + ",\
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
//String Title(){
//  String html = "<html>\
//  <head>\
//  <meta charset=\"utf-8\">\
//  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
//  <title>Config</title>\
//  <style>\
//    * {margin:0;padding:0; box-sizing: border-box;}\
//    body {font-size: 12px;width: 100%; height: 100%;border: red 3px solid; margin: 0 auto; box-sizing: border-box}\
//    .head1{ display: flex; height: 50px;border-bottom: red 3px solid;}\
//    .head1 h1{margin: auto;}\
//    tr{ height: 40px;text-align: center;font-size: 20px;}\
//    .input, input { height: 25px;text-align: center;width: 90%;}\
//    input[type=\"radio\"] {width: auto;}\
//    button {height: 25px;min-width: 100px;margin: 5px;}\
//    button:hover {background: #ccc; font-weight: bold; cursor: pointer;}\
//    .subtitle {text-align: left;font-weight: bold;}\
//    .content {padding: 10px 20px;}\
//    .left , .right { width: 50%; float: left;text-align: left;line-height: 25px;padding: 5px; vertical-align: top;}\
//    .left {text-align: right}\
//    .listBtn {width: 100%; display: inline-block; text-align: center}\
//    a {text-decoration: none;}\
//    .align-left {text-align: left;}\
//    .row-block {display: inline-block; width: 100%;}\
//    .slider {width: 100%;}\
//    .slidecontainer {width: 90%;display: inline-block;vertical-align: top;}\
//    .display-none {display: none;}\    
//    .display-contents {display: contents;}\
//    @media only screen and (min-width: 768px) {\
//      body {width: 600px;font-size: 16px;}\
//      .item-message {width: 50% !important; padding: 10px !important;}\
//      }\
//    textarea {padding: 5px 10px;width: 90%;}\
//    .label { vertical-align: top;}\
//    .item-message {width: 100%; display: inline-block; padding: 5px;}\
//    .status-message {display: inline-block; vertical-align: top;}\
//    .title-message {display: inline-block; line-height: 24px; font-weight: bold; cursor: pointer;}\
//  </style>\
//  <script>\
//    function goState(url) {\
//      console.log(url);\
//      window.location.href = url;\
//    };\
//  </script>\
//  </head>";
//  return html;
//}
//String ContentVerifyRestart() {
//  String content = "<body>\
//    <div class=\"head1\">\
//      <h1>Xác thực</h1>\
//    </div>\
//    <div class=\"content\">\
//      <form action=\"\" method=\"get\">\
//      <div class=\"subtitle\">Bạn có muốn khởi động lại thiết bị?</div>\
//      <div class=\"listBtn\">\
//        <button type=\"button\" onclick=\"goState('/?txtVerifyRestart = true')\" >Đồng ý</button>\
//        <button type=\"button\" onclick=\"goState('/setting')\">Không</button>\
//      <div>\
//      </form>\
//    </div>\
//  </body>\
//  </html>";
//  return content;
//}
//String ContentVerifyDelete() {
//  String content = "<body>\
//    <div class=\"head1\">\
//      <h1>Xác thực</h1>\
//    </div>\
//    <div class=\"content\">\
//      <form action=\"\" method=\"get\">\
//      <div class=\"subtitle\">Bạn có muốn xóa thông điệp \"" + String(currentIndex) + "\" hay không?</div>\
//      <div class=\"listBtn\">\
//        <button type=\"button\" onclick=\"goState('/listMessage?txtVerifyDelete=true')\" >Đồng ý</button>\
//        <button type=\"button\" onclick=\"goState('/settingMessage?txtIndexMessage =" + String(currentIndex) + "')\">Không</button>\
//      <div>\
//      </form>\
//    </div>\
//  </body>\
//  </html>";
//  return content;
//}
//String ContentLogin(){
//  String content = "<body>\
//    <div class=\"head1\">\
//      <h1>Đăng nhập</h1>\
//    </div>\
//    <div class=\"content\">\
//      <form action=\"\" method=\"get\">\
//        <div class=\"row-block\"><div class=\"row-block\"><div class=\"left\">Tài khoản :</div>\
//        <div class=\"right\"><input class=\"input\" placeholder=\"Name wifi\" name=\"txtUsername\" value=\""+apSSID+"\" required></div></div>\
//        <div class=\"row-block\"><div class=\"row-block\"><div class=\"left\">Mật khẩu :</div>\
//        <div class=\"right\"><input class=\"input\" type=\"password\" placeholder=\"Password\" name=\"txtPassword\"></div></div>\
//        <div class=\"listBtn\">\
//        <div class=\"left\"></div>\
//        <div class=\"right\"><button type=\"submit\">Đăng nhập</button></div>\
//        </div>\
//      </form>\
//      <script>\
//        if (window.location.pathname != '/') {\
//          goState('/');\
//        }\
//      </script>\
//    </div>\
//  </body>\
//  </html>";
//  return content;
//}
//String ContentConfig(){
//  GiaTriThamSo();
//  String content = "<body>\
//    <div class=\"head1\">\
//      <h1>Cài đặt chung</h1>\
//    </div>\
//    <div class=\"content\">\
//      <form action=\"\" method=\"get\">\
//        <div class=\"subtitle\">Chế độ Access Point (Phát ra wifi)</div>\
//        <div class=\"row-block\"><div class=\"left\">Tên wifi</div>\
//        <div class=\"right\">: <input class=\"input\" placeholder=\"Name wifi\" maxlength=\"15\" name=\"txtAPName\" value=\""+apSSID+"\" required></div></div>\
//        <div class=\"row-block\"><div class=\"left\">Mật khẩu</div>\
//        <div class=\"right\">: <input class=\"input\" placeholder=\"Password wifi\" maxlength=\"15\" name=\"txtAPPass\" value=\""+apPASS+"\"></div></div>\
//        <div class=\"subtitle\">Tài khoản đăng nhập</div>\
//        <div class=\"row-block\"><div class=\"left\">Tên tài khoản</div>\
//        <div class=\"right\">: <input class=\"input\" placeholder=\"Name wifi\" maxlength=\"15\" name=\"txtAPName\" value=\""+apSSID+"\" disabled required></div></div>\
//        <div class=\"row-block\"><div class=\"left\">Mật khẩu</div>\
//        <div class=\"right\">: <input class=\"input\" placeholder=\"Password wifi\" maxlength=\"15\" name=\"txtPassLogin\" value=\"" + passLogin + "\"></div></div>\
//        <div class=\"subtitle\">Cài đặt bảng LED</div>\
//        <div class=\"row-block\"><div class=\"left\">Số bảng chiều ngang</div>\
//        <div class=\"right\">: <input class=\"input\" placeholder=\"Password wifi\" disabled value=\"" + DISPLAYS_ACROSS + "\"></div></div>\
//        <div class=\"row-block\"><div class=\"left\">Số bảng chiều dọc</div>\
//        <div class=\"right\">: <input class=\"input\" placeholder=\"Password wifi\" disabled value=\"" + DISPLAYS_DOWN + "\"></div></div>\
//        <div class=\"row-block\"><div class=\"left\">Cường độ sáng</div>\
//        <div class=\"right\">: <div class=\"slidecontainer\"><input type=\"range\" name=\"txtLightMessage\" min=\"1\" max=\"255\" value=\"" + String(lightMatrix) + "\" class=\"slider\" id=\"rangeLight\"><br/>(<span id=\"txtRangeLight\"></span>)</div></div>\
//        </div>\
//        <hr>\
//        <div class=\"listBtn\">\
//          <button type=\"button\" onclick=\"goState('/listMessage')\">Trở về</button>\
//          <button type=\"submit\" name=\"btnSaveSetting\" value=\"true\">Lưu cài đặt</button>\
//          <button type=\"button\" onclick=\"goState('/restart')\">Khởi động lại</button>\
//        </div>\
//        <script type=\"text/javascript\">\
//        var slider = document.getElementById(\"rangeLight\");\
//        var output = document.getElementById(\"txtRangeLight\");\
//        output.innerHTML = slider.value;\
//        slider.oninput = function() {\
//          output.innerHTML = this.value;\
//        };\
//      </script>\
//      </form>\
//    </div>\
//  </body>\
//  </html>";
//  return content;
//}
//String configMessage(){
//  GiaTriThamSo();
//  int lengthMessage = parsed["arr"].size();
//  bool isEdit = currentIndex < lengthMessage;
//  const char * name = "Noi dung moi";
//  const char * font = Fonts[0].c_str();
//  bool status = false;
//  int light = 1;
//  const char * motion = "stop";
//  long minBaud = 50, maxBaud = 2000;
//  long baud = minBaud;
//  long minRepeat = 1, maxRepeat = 5000;
//  long repeat = minRepeat;
//
//  if (isEdit) {
//    JsonObject& item = parsed["arr"][currentIndex];  //Implicit cast
//    name = item["name"];
//    status = (bool)item["status"] == true ? true : false;
//    font = item["font"];
//    light = item["light"];
//    motion = item["motion"];
//    repeat = item["repeat"];
//    baud = item["baud"];
//  }
//
//  String content = "<body>\
//    <div class=\"head1\">\
//    <h1>" + String(isEdit ? "Cài đặt thông điệp" : "Thông điệp mới") +"</h1>\
//    </div>\
//    <div class=\"content\">\
//      <form action=\"\" method=\"get\">\
//        <div class=\"matrix-block\"><marquee id=\"myMarquee\">Hi There!</marquee><span id=\"txtMessage\"></span></div>\
//        <div class=\"row-block\"><div class=\"left\">Trạng thái :</div>\
//        <div class=\"right\"><input type=\"radio\" name=\"chboxStatusMessage\" value=\"true\" " + String(status ?  "checked" : "") + "><span class=\"label\">Có</span>\
//        <input type=\"radio\" name=\"chboxStatusMessage\" value=\"false\" " + String(!status ?  "checked" : "") +"><span class=\"label\">Không</span></div></div>\
//        <div class=\"row-block\"><div class=\"left\">Nội dung thông điệp :</div>\
//        <div class=\"right\"><textarea rows='2' id=\"idNameMessage\" name=\"txtNameMessage\" placeholder='Message' required oninput=\"onChangeText()\">" + (strlen(name) > 0 ? name : "") + "</textarea></div>\
//        <div class=\"row-block\"><div class=\"left\">Font hiển thị :</div>\
//        <div class=\"right\">" + dropdownFonts(font) + "</div></div>\
//        <div class=\"row-block\"><div class=\"left\">Hiệu ứng chuyển động :</div>\
//        <div class=\"right\">" + dropdownMotions(motion) + "</div></div>\
//        <div class=\"row-block\"><div class=\"left\">Tốc độ chuyển động :</div>\
//        <div class=\"right\"><div class=\"slidecontainer\"><input type=\"range\" name=\"txtBaudMessage\" min=\"" +String(minBaud)+ "\" max=\""+ String(maxBaud) +"\" value=\"" + String(baud) + "\" class=\"slider\" id=\"rangeLight\"><br/>(<span id=\"txtRangeLight\"></span>)</div></div>\
//        <div class=\"row-block\"><div class=\"left\">Số lần lặp lại hiệu ứng :</div>\
//        <div class=\"right\"><input type=\"number\" name=\"txtRepeatMessage\" min=\"" + String(minRepeat) + "\" max=\"" + String(maxRepeat) + "\" value=\"" + String(repeat) + "\"></div>\
//        <div class=\"row-block\"><div class=\"left\">Tọa độ hiển thị :</div>\
//        <div class=\"right\">\
//        <div><input type=\"number\" class=\"display-contents\" id=\"txtXMessage\" name=\"txtXMessage1\" value=\"0\">\
//        <input type=\"number\" class=\"display-contents\" id=\"txtYMessage\" name=\"txtYMessage1\" value=\"0\"></div>\
//        <div class=\"row-arrow\"><div class=\"arrow arrow-top\" onclick=\"btnXClickUpDown('txtYMessage', -1)\"></div></div>\
//        <div class=\"row-arrow\"><div class=\"arrow arrow-left\" onclick=\"btnXClickUpDown('txtXMessage', -1)\"></div><div class=\"arrow arrow-right\" onclick=\"btnXClickUpDown('txtXMessage', 1)\"></div></div>\
//        <div class=\"row-arrow\"><div class=\"arrow arrow-bottom\" onclick=\"btnXClickUpDown('txtYMessage', 1)\"></div></div>\
//        </div></div>\
//        <br><hr>\
//        <div class=\"listBtn\">\
//          <button type=\"button\" onclick=\"goState('/listMessage')\">Trở về</button>\
//          <button type=\"submit\" name=\"btnSaveMessage\" value=\"true\">Lưu lại</button>\
//          <button " + (isEdit ? "" : "style='display:none;'") +" type=\"button\" onclick=\"goState('/verifyDelete?txtIndexMessage =" + String(currentIndex) + "')\">Xóa</button>\
//        </div>\
//      </form>\
//      <style type=\"text/css\">\
//      .row-arrow {text-align: center; width: 90%;}\
//      .arrow {display: inline-block; cursor: pointer;width:0px;height:0px;border-bottom:10px solid transparent;border-top:10px solid transparent;border-left:15px solid #2f2f2f;}\
//      .arrow:hover {border-left:15px solid red;}\
//      .arrow-top {transform: rotate(-90deg);}\
//      .arrow-left {margin-right: 32px;transform: rotate(-180deg);}\
//      .arrow-bottom {transform: rotate(90deg);}\
//      @media only screen and (min-width: 768px) {\
//        .arrow {border-bottom:20px solid transparent;border-top:20px solid transparent;border-left:30px solid #2f2f2f;}\
//        .arrow:hover {border-left:30px solid red;}\
//        .arrow-left {margin-right: 54px;}\
//      }\
//      .matrix-block {height:75px; border: 1px solid red;}\
//      #myMarquee, #txtMessage {height: 100%; font-size: 40px;}\
//      </style>\
//      <script>\
//        var slider = document.getElementById(\"rangeLight\");\
//        var output = document.getElementById(\"txtRangeLight\");\
//        var eleMsg = document.getElementById(\"idNameMessage\");\
//        var eMar = document.getElementById(\"myMarquee\");\
//        var eMsg = document.getElementById(\"txtMessage\");\
//        output.innerHTML = slider.value;\
//        slider.oninput = function() {\
//          output.innerHTML = this.value;\
//          eMar.scrollDelay = this.value;\
//          reMar();\
//        };\
//        function reMar(){eMar.stop();eMar.start();}\
//        function showMarquee(flag){\
//          if (flag) {eMar.style.display=\"block\";eMsg.style.display=\"none\";}\
//          else {eMar.style.display=\"none\";eMsg.style.display=\"block\";}\
//        }\
//        function btnXClickUpDown(id,count) {\
//          var eleByID = document.getElementById(id);\
//          eleByID.value = parseInt(eleByID.value) + count;\
//        };\
//        function onChangeText() {\
//          eMar.innerHTML = eleMsg.value;\
//          eMsg.innerHTML =  eleMsg.value;\
//        };\
//        function setMotion(motion) {\
//          if (motion == \"stop\") {\
//            showMarquee(0);\
//          } else {\
//          eMar.direction = motion;\
//          showMarquee(1);}\
//          reMar();\
//        }\
//        function onMotion(elem) {\
//          var motion = elem.options[elem.selectedIndex].value;\
//          setMotion(motion);\
//        };\
//        function init() {\
//          eMar.innerHTML = eleMsg.value;\
//          eMsg.innerHTML = eleMsg.value;\
//          setMotion('" + motion + "');\
//        };\
//        init();\
//      </script>\
//    </div>\
//  </body>\
//  </html>";
//  return content;
//}
//String contentListMessage(){
//  GiaTriThamSo();
//  String content = "<body>\
//    <div class=\"head1\">\
//    <h1>Danh sách thông điệp</h1>\
//    </div>\
//    <div class=\"content\">\
//      <form action=\"\" method=\"get\">\
//        " + SendListMessage() + "\
//        <br>\
//        <hr>\
//        <div class=\"listBtn\">\
//          <button type=\"button\" onclick=\"goState('/settingMessage?txtAddMessage=true')\">Thêm thông điệp</button>\
//          <button type=\"button\" " + (parsed["arr"].size() <= 0 ? "style='display:none;'" : " ") +" onclick=\"saveMessage()\">Lưu lại</button>\
//          <button type=\"button\" onclick=\"goState('/setting')\">Cài đặt chung</button>\
//        </div>\
//      </form>\
//       <style type=\"text/css\">\
//        input[type=checkbox]{height: 0;width: 0;visibility: hidden;}\
//        label {	cursor: pointer;	text-indent: -9999px;	width: 50px;	height: 24px;	background: grey;	display: block;	border-radius: 22px;	position: relative;}\
//        label:after {	content: '';	position: absolute;	top: 2px;	left: 2px;	width: 20px;	height: 20px;	background: #fff;	border-radius: 20px;	transition: 0.3s;}\
//        input:checked + label {	background: #4cda64;}\
//        input:checked + label:after {left: calc(100% - 2px);transform:translateX(-100%);}\
//      </style>\
//      <script type=\"text/javascript\">\
//        var configMessage = function(index) {\
//          var url = \"/settingMessage?txtIndexMessage=\" + index;\
//          window.location.href = url;\
//        };\
//        var saveMessage = function() {\
//          var listCheckbox= document.getElementsByTagName('input');\
//          var param = '/listMessage?';\
//          for(var i = 0; i < listCheckbox.length; i++) {\
//            if(listCheckbox[i].type=='checkbox') {\
//              if (listCheckbox[i].checked == true) {\
//                param += 'chboxStatus'+ i + '=true';\
//              } else param += 'chboxStatus'+ i + '=false';\
//              param += '&';\
//            }\
//          }\
//          param += 'btnSaveList=true';\
//          window.location.href = param;\
//        };\
//      </script>\
//    </div>\
//  </body>\
//  </html>";
//  return content;
//}
//String dropdownFonts(String font) {
//  String s ="";
//  s += "<select class=\"input\" name=\"txtFontMessage\">";
//  for (int i = 0; i< 4; i++) {
//    s += "<option value=\"" + Fonts[i] + "\" " + ((font == Fonts[i]) ? "selected" : "") + ">" + (Fonts[i]) + "</option>";
//  }
//  s += "</select>";
//  return s;
//}
//
//String dropdownMotions(String motion) {
//
//  String motions = "[{\"name\":\"Không\", \"value\":\"stop\"},{\"name\":\"Trái qua phải\", \"value\":\"left\"},{\"name\":\"Phải qua trái\", \"value\":\"right\"},{\"name\":\"Trên xuống dưới\", \"value\":\"up\"},{\"name\":\"Dưới lên trên\", \"value\":\"down\"}]";
//  String s ="";
//  s += "<select class=\"input\" onchange=\"onMotion(this);\" name=\"chboxMotionMessage\">";
//  s += "<option value=\"stop\"" + String(motion.equals("stop") ? "selected" : "") + ">Không</option>";
//  s += "<option value=\"left\"" + String(motion.equals("left") ? "selected" : "") + ">Trái qua phải</option>";
//  s += "<option value=\"right\"" + String(motion.equals("right") ? "selected" : "") + ">Phải qua trái</option>";
//  s += "<option value=\"up\"" + String(motion.equals("up") ? "selected" : "") + ">Trên xuống dưới</option>";
//  s += "<option value=\"down\"" + String(motion.equals("down") ? "selected" : "") + ">Dưới lên trên</option>";
//  s += "</select>";
//  return s;
//}
//String SendListMessage()
//{
//  String s="";
//  String json1 = "";
//  int length = parsed["arr"].size();
//  show("SendListMessage length:" + String(length));
//  // for (int i = 0; i < length; i++) {
//  //   JsonObject& itemOb = parsed["arr"][i];
//  //   json1 = "";
//  //   itemOb.printTo(json1);
//  //   show(json1);
//  // }
//  for (int i = 0; i < length ;i++ ) {
//    String id = String(i);
//    JsonObject& item = parsed["arr"][i];  //Implicit cast
//    const char * name = item["name"];
//    // Serial.println(status == true ? "true" : "false");
//    String status = (bool)item["status"] ==  true ? " checked " : "" ;
//    s += "<div class=\"item-message\">\
//    <div class=\"status-message\"><input type=\"checkbox\" name=\"chboxStatus" + id + "\"" + status + "id=\"switch" + id + "\" /><label for=\"switch" + id + "\"></label></div>\
//    <div class=\"title-message\" onclick=\"configMessage(" + id +")\">" + name + "</div>\
//    </div>";
//  }
//  //show(s);
//  return s;
//}
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
        currentIndex = Value.toInt();
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
          show("Set lightMatrix : " + Value);
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
