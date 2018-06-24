#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiServer.h>

const char* ssid = "TTQ";
const char* password = "0987654321";

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(2000);
  String s = "a á";
  long tg = 'ả';
   Serial.println(tg);
  Serial.println(s.c_str());
  // prepare GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Print the IP address
  Serial.println(WiFi.localIP());
  server.on("/", web);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void web() {
  GiaTriThamSo();
  String html = Title();
  html += webContent();
  server.send ( 200, "text/html",html);
}
String Title(){
  String html = "<html>\
  <head>\
  <meta charset=\"utf-8\">\
  <title>Config</title>\
  <style>\
    * {margin:0;padding:0}\
    body {width: 600px;height: auto;border: red 3px solid; margin: 0 auto; box-sizing: border-box}\
    .head1{ display: flex; height: 50px;border-bottom: red 3px solid;}\
    .head1 h1{margin: auto;}\
    table, th, td { border: 1px solid black;border-collapse: collapse;}\
    tr{ height: 40px;text-align: center;font-size: 20px;}\
    input { height: 25px;text-align: center;}\
    button {height: 25px;width: 100px;margin: 5px;}\
    button:hover {background: #ccc; font-weight: bold; cursor: pointer;}\
    .subtitle {text-align: left;font-weight: bold;}\
    .content {padding: 10px 20px;}\
    .left , .right { width: 50%; float: left;text-align: left;line-height: 25px;padding: 5px 0;}\
    .left {text-align: right}\
    .listBtn {text-align: center}\
    a {text-decoration: none;}\
    table {width: 100%;}\
    .column {width: 50%;text-align: center;}\
    .noboder {border: none;}\
    .card-rf {background: yellow;color: red;font-size: 90px;text-align: center;}\
    .tr-active {background: #0095ff !important;}\
  </style>\
  </head>";
  return html;
}
String webContent(){
  String content = "<body>\
    <div class=\"head1\">\
      <h1>Login Config</h1>\
    </div>\
    <div class=\"content\">\
      <form action=\"\" method=\"get\">\
        <div class=\"left\">Name Access Point </div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Tên wifi\" name=\"txtNameAP\" required></div>\
        <div class=\"left\">Password Port TCP</div>\
        <div class=\"right\">: <input class=\"input\" placeholder=\"Cổng TCP\" name=\"txtPassPortTCP\"></div>\
        <div class=\"listBtn\">\
      <button type=\"submit\">Login</button></div>\
      </form>\
    </div>\
  </body>\
  </html>";
  return content;
}
void GiaTriThamSo()
{
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
    Serial.println(s1);
    ConvertString(s1);
    
  }
}

void ConvertString(String s1) {
  Serial.println('a');
  Serial.println('á');
  Serial.println('à');
  Serial.println('ạ');
  Serial.println('ả');
}