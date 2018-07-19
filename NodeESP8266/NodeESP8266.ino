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
#include <ArduinoJson.h>


ESP8266WebServer server(80);

#define RESET 3 
#define LED 2
#define DEBUGGING

#define ADDR 0
#define ADDR_APSSID ADDR
#define ADDR_APPASS (ADDR_APSSID+20)
#define ADDR_PASS_LOGIN (ADDR_APPASS + 20)

#define NAME_DEFAULT "MBELL"
#define PASS_DEFAULT "1234567890"
#define STA_SSID_DEFAULT "TTQ"
#define STA_PASS_DEFAULT "0987654321"
#define AP_SSID_DEFAULT NAME_DEFAULT
#define AP_PASS_DEFAULT PASS_DEFAULT

#define PASS_LOGIN_DEFAULT ""

#define TIME_LIMIT_RESET 3000

bool flagClear = false;
bool isReconnectAP = false;

bool isLogin = false;
String staSSID, staPASS;
String apSSID, apPASS;
String SoftIP, LocalIP;
String passLogin;
int idWebSite = 0;
long timeLogout = 120000;
long t = 0;

StaticJsonBuffer<300> JSONBuffer; //Memory pool
JsonObject& parsed = JSONBuffer.createObject();

void setup()
{
  delay(500);
  Serial.begin(115200);
  Serial.println();
  BeginEEPROM();
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
  ConnectWifi(STA_SSID_DEFAULT, STA_PASS_DEFAULT, 15000); 
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    show("Set WIFI_AP");
  }
  AccessPoint();
  StartServer();
  show("End Setup()");
  String JSONMessage = "{'arr': [{id: 1, name: 'nguyen', status: true},{id: 2, name: 'Quan', status: false}]}";
  initialJson(JSONMessage);
  show("arr size:" + String(parsed["arr"].size()));
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

void initialJson(String strJson) {
  JsonObject& parsed1 = JSONBuffer.parseObject(strJson);   //Parse message
  if (!parsed1.success()) {      //Check for errors in parsing
    show("Parsing failed");
  }
  parsed["arr"] = parsed1["arr"];
}

void ConfigDefault()
{
  isLogin = false;
  apSSID = AP_SSID_DEFAULT;
  apPASS = AP_PASS_DEFAULT;
  passLogin =  PASS_LOGIN_DEFAULT;
  show("Config Default");
}
void WriteConfig()
{
  SaveStringToEEPROM(apSSID, ADDR_APSSID);
  SaveStringToEEPROM(apPASS, ADDR_APPASS);
  SaveStringToEEPROM(passLogin, ADDR_PASS_LOGIN);
  show("Write Config");
}
void ReadConfig()
{
  apSSID = ReadStringFromEEPROM(ADDR_APSSID);
  apPASS = ReadStringFromEEPROM(ADDR_APPASS);
  passLogin = ReadStringFromEEPROM(ADDR_PASS_LOGIN);
  show("Read Config");
  show("Access Point: \n" + apSSID + "\n" + apPASS);
  show("Pass login: \n" + passLogin);
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

void ConnectWifi(String ssid, String password, long timeOut)
{
  show("Connect to other Access Point");
  // delay(1000);
  int count = timeOut / 500;
  show("Connecting");
  show(ssid + "-" + password);
  WiFi.begin(ssid.c_str(),password.c_str());
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
  server.on("/", login);
  server.on("/setting", websetting);
  server.on("/editMessage", webConfigMessage);
  server.on("/listMessage", webListMessage);
  server.on("/restart", restartDevice);
  server.onNotFound(handleNotFound);
  server.begin();
  show("HTTP server started");
}

void login() {
  GiaTriThamSo();
  String html = Title();
  if (isLogin == false) {
    html += ContentLogin();
  }
  else {
    html += contentListMessage();
  }
  // else if (idWebSite == 2) {
  //   html += ContentVerifyRestart();
  // }else html += ContentLogin();
  server.send ( 200, "text/html",html);
}

void websetting() {
  String html = Title();
  if (isLogin == false) {
    html += ContentLogin();
  } else {
    html += ContentConfig();
  }
  server.send ( 200, "text/html",html);
}
void webConfigMessage() {
  String html = Title();
  html += configMessage();
  server.send ( 200, "text/html",html);
}
void webListMessage() {
  String html = Title();
  if (isLogin == false) {
    html += ContentLogin();
  } else {
    html += contentListMessage();
  }
  server.send ( 200, "text/html",html);
}
void restartDevice() {
  String html = Title();
  if (isLogin == false) {
    html += ContentLogin();
  } else {
    html += ContentVerifyRestart();
  }
  server.send ( 200, "text/html",html);
}
String Title(){
  String html = "<html>\
  <head>\
  <meta charset=\"utf-8\">\
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
  <title>Config</title>\
  <style>\
    * {margin:0;padding:0; box-sizing: border-box;}\
    body {font-size: 12px;width: 100%; height: auto;border: red 3px solid; margin: 0 auto; box-sizing: border-box}\
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
    .left , .right { width: 50%; float: left;text-align: left;line-height: 25px;padding: 5px; vertical-align: top;}\
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
      body {width: 600px;font-size: 16;}\
      .item-message {width: 50% !important; padding: 10px !important;}\
      }\
    textarea {padding: 5px 10px;width: 90%;}\
    .label { vertical-align: top;}\
    .item-message {width: 100%; display: inline-block; padding: 5px;}\
    .status-message {display: inline-block; vertical-align: top;}\
    .title-message {display: inline-block; line-height: 24px; font-weight: bold; cursor: pointer;}\
  </style>\
  <script>\
    function goState(url) {\
      console.log(url);\
      window.location.href = url;\
    };\
  </script>\
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
        <button type=\"submit\" name=\"txtVerifyRestart\" value=\"false\">Đồng ý</button>\
        <button type=\"button\" onclick=\"goState('/setting')\">Không</button>\
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
        <div class=\"row-block\"><div class=\"row-block\"><div class=\"left\">Tài khoản :</div>\
        <div class=\"right\"><input class=\"input\" placeholder=\"Name wifi\" name=\"txtUsername\" value=\""+apSSID+"\" required></div></div>\
        <div class=\"row-block\"><div class=\"row-block\"><div class=\"left\">Mật khẩu :</div>\
        <div class=\"right\"><input class=\"input\" type=\"password\" placeholder=\"Password\" name=\"txtPassword\"></div></div>\
        <div class=\"listBtn\">\
        <div class=\"left\"></div>\
        <div class=\"right\"><button type=\"submit\">Đăng nhập</button></div>\
        </div>\
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
        <div class=\"subtitle\">Tài khoản đăng nhập</div>\
        <div class=\"row-block\"><div class=\"left\">Tên tài khoản</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Name wifi\" maxlength=\"15\" name=\"txtAPName\" value=\""+apSSID+"\" disabled required></div></div>\
        <div class=\"row-block\"><div class=\"left\">Mật khẩu</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Password wifi\" maxlength=\"15\" name=\"txtPassLogin\" value=\"" + passLogin + "\"></div></div>\
        <hr>\
        <div class=\"listBtn\">\
          <button type=\"button\" onclick=\"goState('/listMessage')\">Trở về</button>\
          <button type=\"button\" onclick=\"goState('/?txtLogout=true')\">Đăng xuất</button>\
          <button type=\"submit\" name=\"btnSave\" value=\"true\">Lưu</button>\
          <button type=\"button\" onclick=\"goState('/restart')\">Khởi động</button>\
        </div>\
        <hr>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}
bool isStatus = true;
String configMessage(){
  GiaTriThamSo();

  String content = "<body>\
    <div class=\"head1\">\
    <h1>Cài đặt thông điệp</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        <div class=\"row-block\"><div class=\"left\">Trạng thái :</div>\
        <div class=\"right\"><input type=\"radio\" name=\"chboxStatus\" " + String(isStatus ?  "checked" : "") + "><span class=\"label\">Có</span>\
        <input type=\"radio\" name=\"chboxStatus\" " + String(!isStatus ?  "checked" : "") +"><span class=\"label\">Không</span></div></div>\
        <div class=\"row-block\"><div class=\"left\">Nội dung thông điệp :</div>\
        <div class=\"right\"><textarea rows='2' placeholder='Message'></textarea></div>\
        <div class=\"row-block\"><div class=\"left\">Font hiển thị :</div>\
        <div class=\"right\">" + dropdownFonts() + "</div>\
        <div class=\"row-block\"><div class=\"left\">Cường độ sáng :</div>\
        <div class=\"right\"><div class=\"slidecontainer\"><input type=\"range\" min=\"1\" max=\"100\" value=\"50\" class=\"slider\" id=\"rangeLight\"><br/>(<span id=\"txtRangeLight\"></span>)</div></div>\
        <div class=\"row-block\"><div class=\"left\">Chuyển động :</div>\
        <div class=\"right\">\
        <input type=\"radio\" name=\"chboxStatus\" checked><span class=\"label\">Không</span><br/>\
        <input type=\"radio\" name=\"chboxStatus\" checked><span class=\"label\">Trái qua phải</span><br/>\
        <input type=\"radio\" name=\"chboxStatus\" checked><span class=\"label\">Phải qua trái</span><br/>\
        <input type=\"radio\" name=\"chboxStatus\" checked><span class=\"label\">Trên xuống dưới</span><br/>\
        <input type=\"radio\" name=\"chboxStatus\" checked><span class=\"label\">Dưới lên trên</span><br/>\
        </div>\
        <div class=\"row-block\"><div class=\"left\">Tốc độ chuyển động (Nếu có) :</div>\
        <div class=\"right\"><input class=\"input\" type=\"number\" value=\"100\" name=\"quantity\" min=\"100\" max=\"2000\" oninput=\"if(value.length>4)value=2000;if(value.length == 0)value=100\"></div></div>\
        <br><hr>\
        <div class=\"listBtn\">\
          <button type=\"button\" onclick=\"goState('/listMessage')\">Trở về</button>\
          <button type=\"button\" onclick=\"goState('/?txtLogout=true')\">Đăng xuất</button>\
          <button type=\"submit\" >Lưu lại</button>\
        </div>\
      </form>\
      <script type=\"text/javascript\">\
        var slider = document.getElementById(\"rangeLight\");\
        var output = document.getElementById(\"txtRangeLight\");\
        output.innerHTML = slider.value;\
        slider.oninput = function() {\
          output.innerHTML = this.value;\
        }\
      </script>\
    </div>\
  </body>\
  </html>";
  return content;
}
String contentListMessage(){
  GiaTriThamSo();
  String content = "<body>\
    <div class=\"head1\">\
    <h1>Danh sách thông điệp</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        " + SendListMessage() + "\
        <br><hr>\
        <div class=\"listBtn\">\
          <button type=\"button\" onclick=\"goState('/?txtLogout=true')\">Đăng xuất</button>\
          <button type=\"button\" onclick=\"goState('/setting')\">Cài đặt</button>\
          <button type=\"button\" onclick=\"saveMessage()\">Lưu lại</button>\
          <button type=\"submit\"><a href=\"/?txtAddMessage=true\">Thêm mới</a></button>\
        </div>\
      </form>\
       <style type=\"text/css\">\
        input[type=checkbox]{height: 0;width: 0;visibility: hidden;}\
        label {	cursor: pointer;	text-indent: -9999px;	width: 50px;	height: 24px;	background: grey;	display: block;	border-radius: 22px;	position: relative;}\
        label:after {	content: '';	position: absolute;	top: 2px;	left: 2px;	width: 20px;	height: 20px;	background: #fff;	border-radius: 20px;	transition: 0.3s;}\
        input:checked + label {	background: #4cda64;}\
        input:checked + label:after {left: calc(100% - 2px);transform:translateX(-100%);}\
      </style>\
      <script type=\"text/javascript\">\
        var configMessage = function(index) {\
          var url = \"/editMessage?message=\" + index;\
          window.location.href = url;\
        };\
        var saveMessage = function() {\
          var listCheckbox= document.getElementsByTagName('input');\
          var param = '/listMessage?';\
          for(var i = 0; i < listCheckbox.length; i++) {\
            if(listCheckbox[i].type=='checkbox') {\
              if (listCheckbox[i].checked == true) {\
                param += 'chboxStatus'+ i + '=true';\
              } else param += 'chboxStatus'+ i + '=false';\
              param += '&';\
            }\
          }\
          param += 'btnSaveList=true';\
          console.log(param);\
          window.location.href = param;\
        };\
      </script>\
    </div>\
  </body>\
  </html>";
  return content;
}
String dropdownFonts() {
  String modelsInventer[4] = {"Font 1", "Font 2", "Font 3", "Font 4"};
  String selectedInventer = modelsInventer[1];
  String s ="";
  s += "<select class=\"input\" name=\"txtSelectedInventer\">";
  for (int i = 0; i< 4; i++) {
    s += "<option value=\"" + modelsInventer[i] + "\" " + ((selectedInventer == modelsInventer[i]) ? "selected" : "") + ">" + (modelsInventer[i]) + "</option>";
  }
  s += "</select>";
  return s;
}
String SendListMessage()
{
  String s="";
  int length = parsed["arr"].size();
  for (int i = 0; i < length ;i++ ) {
    String id = String(i);
    JsonObject& item = parsed["arr"][i];  //Implicit cast
    const char * name = item["name"];
    // Serial.println(status == true ? "true" : "false");
    String status = (bool)item["status"] ==  true ? " checked " : "" ;
    s += "<div class=\"item-message\">\
    <div class=\"status-message\"><input type=\"checkbox\" name=\"chboxStatus" + id + "\"" + status + "id=\"switch" + id + "\" /><label for=\"switch" + id + "\"></label></div>\
    <div class=\"title-message\" onclick=\"configMessage(" + id +")\">" + name + "</div>\
    </div>";
  }
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
    show(s1);
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
      else if (Name.indexOf("txtPassLogin") >= 0){
        if (Value != passLogin){
          passLogin = Value;
          show("Set passLogin : " + Value);
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
        }
      }
    }else {
      if (Name.indexOf("txtUsername") >= 0) {
        UserName =  Value ;
        show("Set UserName : " + UserName);
      }
      else if (Name.indexOf("txtPassword") >= 0) {
        PassWord =  Value ;
        show("Set Password : " + PassWord);
      }

      if (Name.indexOf("txtPassword") >= 0 && UserName.equals(apSSID) && PassWord.equals(passLogin)){
        isLogin = true;
        show("Login == true");
      } else {
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
