/***********************************************************************************
 *
 * GPIO PIN 
 * RESET 4  GPIO4
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
#include "eeprom.h"


ESP8266WebServer server(80);

#define RESET 3 
#define LED 2
#define DEBUGGING

#define ADDR 0
#define ADDR_STASSID (ADDR)
#define ADDR_STAPASS (ADDR_STASSID+20)
#define ADDR_STAIP (ADDR_STAPASS+20)
#define ADDR_STAGATEWAY (ADDR_STAIP+20)
#define ADDR_STASUBNET (ADDR_STAGATEWAY+20)

#define ADDR_APSSID (ADDR_STASUBNET+20)
#define ADDR_APPASS (ADDR_APSSID+20)
#define ADDR_APIP (ADDR_APPASS+20)
#define ADDR_APGATEWAY (ADDR_APIP+20)
#define ADDR_APSUBNET (ADDR_APGATEWAY+20)

#define NAME_DEFAULT "MBELL"
#define STA_SSID_DEFAULT "TTQ"
#define STA_PASS_DEFAULT "0987654321"
#define AP_SSID_DEFAULT NAME_DEFAULT
#define AP_PASS_DEFAULT "04081984"

#define TIME_LIMIT_RESET 3000

bool flagClear = false;
bool isReconnectAP = false;

bool isLogin = false;
String staSSID, staPASS;
String apSSID, apPASS;
String SoftIP, LocalIP;
int idWebSite = 0;
long timeLogout = 120000;
long t = 0;

void setup()
{
  delay(500);
  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(512);
  GPIO();
  idWebSite = 0;
  isLogin = false;
  if (EEPROM.read(500) == EEPROM.read(0) || flagClear) {
    ClearEEPROM();
    ConfigDefault();
    WriteConfig();
  }
  ReadConfig();
  WiFi.mode(WIFI_AP_STA);
  delay(2000);
  ConnectWifi(15000); 
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    show("Set WIFI_AP");
  }
  AccessPoint();
  StartServer();
  show("End Setup()");
}

void loop()
{
  server.handleClient();
  if (millis() - t > timeLogout) {
    isLogin = false;
    t = millis();
  }
  if (digitalRead(RESET) == LOW)
  {
    long t = TIME_LIMIT_RESET/100;
    while (digitalRead(RESET)==LOW && t-- >= 0){
      delay(100);
    }
    if (t < 0){
      show("RESET DEFAULT CONFIG");
      ConfigDefault();
      WriteConfig();
      setup();
    }
  }
  delay(10);
}

void show(String s)
{
  #ifdef DEBUGGING 
    Serial.println(s);
  #endif
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
void GPIO()
{
  show("GPIO");
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
  pinMode(RESET,INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(VT), handleInterruptVT, RISING); 
}

int flagInterrupt;
//This program get executed when interrupt is occures i.e.change of input state
void handleInterruptVT() { 
  if (flagInterrupt) 
    return;
  flagInterrupt = true;
  // To do:

  flagInterrupt = false;
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

void ConfigDefault()
{
  isLogin = false;
  staSSID = STA_SSID_DEFAULT;
  staPASS = STA_PASS_DEFAULT;
  apSSID = AP_SSID_DEFAULT;
  apPASS = AP_PASS_DEFAULT;
  show("Config Default");
}
void WriteConfig()
{
  SaveStringToEEPROM(staSSID, ADDR_STASSID);
  SaveStringToEEPROM(staPASS, ADDR_STAPASS);
  SaveStringToEEPROM(apSSID, ADDR_APSSID);
  SaveStringToEEPROM(apPASS, ADDR_APPASS);

  show("Write Config");
}
void ReadConfig()
{
  staSSID = ReadStringFromEEPROM(ADDR_STASSID);
  staPASS = ReadStringFromEEPROM(ADDR_STAPASS);
  apSSID = ReadStringFromEEPROM(ADDR_APSSID);
  apPASS = ReadStringFromEEPROM(ADDR_APPASS);
  show("Read Config");
  show("Station: \n" + staSSID + "\n" + staPASS);
  show("Access Point: \n" + apSSID + "\n" + apPASS);
}

void AccessPoint()
{
  show("Access Point Config");
  //WiFi.disconnect();
  delay(1000);
  // Wait for connection
  show( WiFi.softAP(apSSID.c_str(),apPASS.c_str()) ? "Ready" : "Failed!");
  IPAddress myIP = WiFi.softAPIP();
  show("AP IP address: ");
  SoftIP = ""+(String)myIP[0] + "." + (String)myIP[1] + "." +(String)myIP[2] + "." +(String)myIP[3];
  show(SoftIP);
}

void ConnectWifi(long timeOut)
{
  show("Connect to other Access Point");
  // delay(1000);
  int count = timeOut / 500;
  show("Connecting");
  show(staSSID);
  show(staPASS);
  WiFi.begin(staSSID.c_str(),staPASS.c_str());
  while (WiFi.status() != WL_CONNECTED && --count > 0) {
    delay(500);
    Serial.print(".");
  }
  if (count > 0){
    show("Connected");
    IPAddress myIP = WiFi.localIP();
    String LocalIP = ""+(String)myIP[0] + "." + (String)myIP[1] + "." +(String)myIP[2] + "." +(String)myIP[3];
    show("Local IP :"); 
    show(LocalIP);
  }else {
    show("Disconnect");
  }
}

void StartServer()
{
  server.on("/", webConfig);
  server.on("/rfconfig", webRFConfig);
  server.on("/home", webViewHome);
  server.onNotFound(handleNotFound);
  server.begin();
  show("HTTP server started");
}

void webConfig() {
  GiaTriThamSo();
  String html = Title();
  if (idWebSite == 0) {
    html += ContentLogin();
  }
  else if (idWebSite == 1) {
    html += ContentConfig();
  }
  else if (idWebSite == 2) {
    html += ContentVerifyRestart();
  }else html += ContentLogin();
  server.send ( 200, "text/html",html);
}
void webRFConfig() {
  String html = Title();
  html += ChannelRFConfig();
  server.send ( 200, "text/html",html);
}

void webViewHome() {
  String html = Title();
  // html += webView();
  server.send ( 200, "text/html",html);
}

String Title(){
  String html = "<html>\
  <head>\
  <meta charset=\"utf-8\">\
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
  <title>Config</title>\
  <style>\
    * {margin:0;padding:0}\
    body {width: 100%; height: auto;border: red 3px solid; margin: 0 auto; box-sizing: border-box}\
    form {font-size: 12px;}\
    .head1{ display: flex; height: 50px;border-bottom: red 3px solid;}\
    .head1 h1{margin: auto;}\
    table, th, td { border: 1px solid black;border-collapse: collapse;}\
    tr{ height: 40px;text-align: center;font-size: 20px;}\
    .input, input { height: 25px;text-align: center;width: 90%;}\
    input[type=\"radio\"] {width: auto;}\
    button {height: 25px;min-width: 100px;margin: 5px;}\
    button:hover {background: #ccc; font-weight: bold; cursor: pointer;}\
    .subtitle {text-align: left;font-weight: bold;}\
    .content {padding: 10px 20px;}\
    .left , .right { width: 50%; float: left;text-align: left;line-height: 25px;padding: 5px 0; vertical-align: top;}\
    .left {text-align: right}\
    .listBtn {width: 100%; display: inline-block; text-align: center}\
    a {text-decoration: none;}\
    table {width: 100%;}\
    .column {width: 50%;text-align: center;}\
    .column3 {width: 33.3%;text-align: center;}\
    .noboder {border: none;}\
    .card-rf {background: yellow;color: red;font-size: 90px;text-align: center;}\
    .align-left {text-align: left;}\
    .small-table .row {height: auto;}\
    .tr-active {background: #0095ff !important;}\
    .important {color: red;}\
    .row-block {display: inline-block; width: 100%;}\
    @media only screen and (min-width: 768px) {\
      body {width: 600px;}\
      form {font-size: 18px;}\
      }\
  </style>\
  </head>";
  return html;
}
String ContentVerifyRestart() {
  String content = "<body>\
    <div class=\"head1\">\
      <h1>Xác thực</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
      <div class=\"subtitle\">Bạn có muốn khởi động lại thiết bị?</div>\
      <div class=\"listBtn\">\
        <button type=\"submit\" name=\"txtVerifyRestart\" value=\"false\">Không</button>\
        <button type=\"submit\" name=\"txtVerifyRestart\" value=\"true\">Đồng ý</button>\
      <div>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}
String ContentLogin(){
  String content = "<body>\
    <div class=\"head1\">\
      <h1>Đăng nhập</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        <div class=\"row-block\"><div class=\"row-block\"><div class=\"left\">Tài khoản</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Name wifi\" name=\"txtNameAP\" value=\""+apSSID+"\" required></div></div>\
        <div class=\"row-block\"><div class=\"row-block\"><div class=\"left\">Mật khẩu</div>\
        <div class=\"right\">: <input class=\"input\" type=\"password\" placeholder=\"Password\" name=\"txtPassPortTCP\" value=\"\" required></div></div>\
        <div class=\"listBtn\">\
        <a href=\"/home\" target=\"_blank\">Trang chủ!</a>\
      <button type=\"submit\">Đăng nhập</button></div>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}
String ContentConfig(){
  String content = "<body>\
    <div class=\"head1\">\
      <h1>Cài đặt thiết bị</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        <div class=\"subtitle\">Chế độ Access Point (Phát ra wifi)</div>\
        <div class=\"row-block\"><div class=\"left\">Tên wifi</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Name wifi\" maxlength=\"15\" name=\"txtAPName\" value=\""+apSSID+"\" required></div></div>\
        <div class=\"row-block\"><div class=\"left\">Mật khẩu</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Password wifi\" maxlength=\"15\" name=\"txtAPPass\" value=\""+apPASS+"\"></div></div>\
        <hr>\
        <div class=\"listBtn\">\
          <button type=\"submit\"><a href=\"?txtRefresh=true\">Làm mới</a></button>\
          <button type=\"submit\" name=\"btnSave\" value=\"true\">Lưu</button>\
          <button type=\"submit\"><a href=\"/?txtRestart=true\">Khởi động</a></button>\
          <button type=\"submit\"><a href=\"/?txtLogout=true\">Đăng xuất</a></button>\
        </div>\
        <hr>\
        <a href=\"/rfconfig\">Mã hóa tên!</a>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}

String ChannelRFConfig(){
  GiaTriThamSo();
  String content = "<body>\
    <div class=\"head1\">\
    <h1>Cài đặt RF</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        <table><tr class=\"row\"><th>Vị trí</th><th>Tên hiển thị</th></tr>"+ SendTRRFConfig() +"</table>\
        <br><hr>\
        <div class=\"listBtn\">\
          <button type=\"submit\"><a href=\"/rfconfig?\">Làm mới</a></button>\
          <button type=\"submit\">Lưu lại</button>\
          <button type=\"submit\"><a href=\"/?txtLogout=true\">Đăng xuất</a></button>\
          <button type=\"submit\"><a href=\"/?txtBack=true\">Trang trước</a></button>\
        </div>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}
/*
String webView(){
  String content = "<body>\
    <div class=\"head1\">\
    <h1>Trang chủ</h1>\
    </div>\
    <div class=\"card-rf " + isTrActive(0) + "\">"+ getData(bufferRF[0]) +"</div>\
    <div class=\"content\">\
    <form action=\"\" method=\"get\">\
      <table>\
      <tr class=\"row\"><th>Vị trí</th><th>Tên gọi</th></tr>"+ SendTRViewHome() +"\
      </table>\
      <br><hr>\
      <div class=\"listBtn\">\
      <button type=\"submit\"><a href=\"/\">Đăng nhập</a></button>\
      </div>\
    </form>\
    <script type=\"text/javascript\">\
      setInterval(function() {\
      window.location.reload();\
      }, 2000);\
    </script>\
    </div>\
  </body>\
  </html>";
  return content;
}*/
String SendTRRFConfig()
{
  String s="";
  // for (int i=0; i< (isButtonHandle ? RFCHANNEL - 1: RFCHANNEL) ;i++) {
  //   String id = (i < 10 ? "0" + String(i) : String(i));
  //   s += "<tr class=\"row\"><td class=\"column\">"+ id +"</td><td class=\"column\"><input type=\"text\" class=\"input noboder\" maxlength=\"6\" placeholder=\"Tên bàn\" name=\"txtChannelRF"+id+"\" value=\""+ channelRF[i] +"\" required></td></tr>";
  // }
  //show(s);
  return s;
}
void GiaTriThamSo()
{
  t = millis();
  String message="";
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  String UserName, PassWord;
  for (uint8_t i=0; i<server.args(); i++){
     
    String Name=server.argName(i); 
    String Value=String( server.arg(i)) ;
    String s1=Name+ ": " +Value;
    //show(s1);
    if (isLogin == true) {
      if (Name.indexOf("txtLogout") >= 0){
        isLogin = false;
        show("Logout");
      }
      else if (Name.indexOf("txtBack") >= 0){
        idWebSite = 1;
        show("Back");
      }
      else if (Name.indexOf("txtAPName") >= 0){
        if (Value != staSSID && Value != apSSID && Value.length() > 0){
          show("Set apSSID : " + Value);
        }
      }
      else if (Name.indexOf("txtAPPass") >= 0){
        if (Value != apPASS){
          if (Value.length() >= 8) { // Length password >= 8 
            apPASS =  Value ;
            show("Set apPASS : " + apPASS);
          } else {
            show("txtAPPass is invalid (Value.length() >= 8 && Value != apPASS)");
          }
          
        } 
      }
      else if (Name.indexOf("txtRestart") >= 0){
        idWebSite = 2;
        show("Verify restart");
        show(Value);
      }
      else if (Name.indexOf("btnSave") >= 0)
      {
        WriteConfig();
        show("Save config");
      }
      else if (Name.indexOf("txtVerifyRestart") >= 0)
      {
        if ( Value.indexOf("true") >=0 ) {
          setup();
          show("Restart Device");
          idWebSite = 0;
        }
        else idWebSite = 1;
      }
    }else {
      if (Name.indexOf("txtNameAP") >= 0)
        UserName =  Value ;
      else if (Name.indexOf("txtPassPortTCP") >= 0)
        PassWord =  Value ;

      if (UserName.equals(apSSID) && PassWord.equals(String(apSSID))){
        isLogin = true;
        idWebSite = 1;
        show("Login == true");
      }else {
        idWebSite = 0;
        isLogin = false;
      }
    }/*
    if (Name.indexOf("txtChannelRF") >=0){
      int i1 = Name.indexOf("RF");
      if (i1 > 0){
        String strId = Name.substring(i1 + 2,i1 + 4);
        int id = strId.toInt();
        if (Value != channelRF[id]) {
          channelRF[id] = Value;
          WriteConfig();
          show("Save config");
        }
      }
    }
    */
    Name = "";
    Value = "";
  }
  if (isLogin == false)
    idWebSite = 0;
}
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
