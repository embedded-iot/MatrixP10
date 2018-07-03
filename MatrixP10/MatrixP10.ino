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
  
  // ConnectAP();
  StartWebServer();
  char *s1;
  // ConvertStringToArrayChar("1234567890", true);
  s1 = ConvertStringToArrayChar("a á à ạ ả ã", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("d đ", true);
  s1 = ConvertStringToArrayChar("d đ", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("a á à ạ ả ã", true);
  s1 = ConvertStringToArrayChar("a á à ạ ả ã", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("â ấ ầ ậ ẩ ẫ", true);
  s1 = ConvertStringToArrayChar("â ấ ầ ậ ẩ ẫ", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("ă ắ ằ ặ ẳ ẵ", true);
  s1 = ConvertStringToArrayChar("ă ắ ằ ặ ẳ ẵ", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("e é è ẹ ẻ ẽ", true);
  s1 = ConvertStringToArrayChar("e é è ẹ ẻ ẽ", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("ê ế ề ệ ể ễ", true);
  s1 = ConvertStringToArrayChar("ê ế ề ệ ể ễ", false);
  ShowArray(s1);

  // ConvertStringToArrayChar("i í ì ị ỉ ĩ", true);
  s1 = ConvertStringToArrayChar("i í ì ị ỉ ĩ", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("o ó ò ọ ỏ õ", true);
  s1 = ConvertStringToArrayChar("o ó ò ọ ỏ õ", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("ô ố ồ ộ ổ ỗ", true);
  s1 = ConvertStringToArrayChar("ô ố ồ ộ ổ ỗ", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("ơ ớ ờ ợ ở ỡ", true);
  s1 = ConvertStringToArrayChar("ơ ớ ờ ợ ở ỡ", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("u ú ù ụ ủ ũ", true);
  s1 = ConvertStringToArrayChar("u ú ù ụ ủ ũ", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("ư ứ ừ ự ử ữ", true);
  s1 = ConvertStringToArrayChar("ư ứ ừ ự ử ữ", false);
  ShowArray(s1);
  // ConvertStringToArrayChar("y ý ỳ ỵ ỷ ỹ", true);
  s1 = ConvertStringToArrayChar("y ý ỳ ỵ ỷ ỹ", false);
  ShowArray(s1);
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
    // ConvertString(s1);
    
  }
}

void ConnectAP(){
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
}
void StartWebServer() {
  server.on("/", web);
  server.begin();
  Serial.println("HTTP server started");
}


char* ConvertStringToArrayChar(String x, bool display) {
  int length = x.length();
  char *resultConvert = new char[length + 1]; // or
  int len = 0;
  Serial.println(x);
  char *y = new char[length + 1]; // or
  strcpy(y, x.c_str());
  if (display) {
    while (*y) {
      if (*y == ' ') {
        Serial.println("  ");
      } else {
        Serial.print((int)*y);
        Serial.print(" ");
      }
      *y++;
    }
  }
  else {
    for (int i = 0; i < length; i++) {
      char c = 0;
      if (y[i] < 128) {
        // Serial.print(y[i]);
        resultConvert[len++] = y[i];
        
      } else if (y[i] < 225) {
        String tg = "_";
        if (y[i] == 195 && i < length - 1) {
          if (y[i+1] == 161) { tg = "á"; c = 1; }
          else if (y[i+1] == 160) { tg = "à"; c = 2; }
          else if (y[i+1] == 163) { tg = "ã"; c = 3; }

          else if (y[i+1] == 162) { tg = "â"; c = 4; }

          else if (y[i+1] == 169) { tg = "é"; c = 5; }
          else if (y[i+1] == 168) { tg = "è"; c = 6; }

          else if (y[i+1] == 170) { tg = "ê"; c = 7; }

          else if (y[i+1] == 173) { tg = "í"; c = 8; }
          else if (y[i+1] == 172) { tg = "ì"; c = 9; }

          else if (y[i+1] == 179) { tg = "ó"; c = 10; }
          else if (y[i+1] == 178) { tg = "ò"; c = 11; }
          else if (y[i+1] == 181) { tg = "õ"; c = 12; }

          else if (y[i+1] == 180) { tg = "ô"; c = 13; }

          else if (y[i+1] == 186) { tg = "ú"; c = 14; }
          else if (y[i+1] == 185) { tg = "ù"; c = 15; }

          else if (y[i+1] == 189) { tg = "ý"; c = 16; }

        } else if (y[i] == 196 && i < length - 1) {
          if (y[i+1] == 131) { tg = "ă"; c = 17; }

          else if (y[i+1] == 169) { tg = "ĩ"; c = 18; }

          else if (y[i+1] == 145) { tg = "đ"; c = 19; }
          
        } else if (y[i] == 197 && i < length - 1) {
          if (y[i+1] == 169) { tg = "ũ"; c = 20; }

          // else if (y[i+1] == 169) { tg = "ĩ"; c = 1; }
          
        } else if (y[i] == 198 && i < length - 1) {
          if (y[i+1] == 161) { tg = "ơ"; c = 21; }

          else if (y[i+1] == 176) { tg = "ư"; c = 22; }
        }
        resultConvert[len++] = 128 + c;
        // Serial.print(tg);
        i = i + 1;
      } else {
        String tg = "=";
        if (y[i] == 225 && i < length - 1) {
          if (y[i+1] == 186 && i + 1 < length - 1) {
            if (y[i+2] == 161) { tg = "ạ"; c = 23; }
            else if (y[i+2] == 163) { tg = "ả"; c = 24; }

            else if (y[i+2] == 165) { tg = "ấ"; c = 25; }
            else if (y[i+2] == 167) { tg = "ầ"; c = 26; }
            else if (y[i+2] == 173) { tg = "ậ"; c = 27; }
            else if (y[i+2] == 169) { tg = "ẩ"; c = 28; }
            else if (y[i+2] == 171) { tg = "ẫ"; c = 29; }

            else if (y[i+2] == 175) { tg = "ắ"; c = 30; }
            else if (y[i+2] == 177) { tg = "ằ"; c = 31; }
            else if (y[i+2] == 183) { tg = "ặ"; c = 32; }
            else if (y[i+2] == 179) { tg = "ẳ"; c = 33; }
            else if (y[i+2] == 181) { tg = "ẵ"; c = 34; }

            else if (y[i+2] == 185) { tg = "ẹ"; c = 35; }
            else if (y[i+2] == 187) { tg = "ẻ"; c = 36; }
            else if (y[i+2] == 189) { tg = "ẽ"; c = 37; }
            else if (y[i+2] == 191) { tg = "ế"; c = 38; }

          } else if (y[i+1] == 187 && i + 1 < length - 1) {
            
            if (y[i+2] == 129) { tg = "ề"; c = 39; }
            else if (y[i+2] == 135) { tg = "ệ"; c = 40; }
            else if (y[i+2] == 131) { tg = "ể"; c = 41; }
            else if (y[i+2] == 133) { tg = "ễ"; c = 42; }

            else if (y[i+2] == 139) { tg = "ị"; c = 43; }
            else if (y[i+2] == 137) { tg = "ỉ"; c = 44; }

            else if (y[i+2] == 141) { tg = "ọ"; c = 45; }
            else if (y[i+2] == 143) { tg = "ỏ"; c = 46; }

            else if (y[i+2] == 145) { tg = "ố"; c = 47; }
            else if (y[i+2] == 147) { tg = "ồ"; c = 48; }
            else if (y[i+2] == 153) { tg = "ộ"; c = 49; }
            else if (y[i+2] == 149) { tg = "ổ"; c = 50; }
            else if (y[i+2] == 151) { tg = "ỗ"; c = 51; }

            else if (y[i+2] == 155) { tg = "ớ"; c = 52; }
            else if (y[i+2] == 157) { tg = "ờ"; c = 53; }
            else if (y[i+2] == 163) { tg = "ợ"; c = 54; }
            else if (y[i+2] == 159) { tg = "ở"; c = 55; }
            else if (y[i+2] == 161) { tg = "ỡ"; c = 56; }

            else if (y[i+2] == 165) { tg = "ụ"; c = 57; }
            else if (y[i+2] == 167) { tg = "ủ"; c = 58; }

            else if (y[i+2] == 169) { tg = "ứ"; c = 59; }
            else if (y[i+2] == 171) { tg = "ừ"; c = 60; }
            else if (y[i+2] == 177) { tg = "ự"; c = 61; }
            else if (y[i+2] == 173) { tg = "ử"; c = 62; }
            else if (y[i+2] == 175) { tg = "ữ"; c = 63; }

            else if (y[i+2] == 179) { tg = "ỳ"; c = 64; }
            else if (y[i+2] == 181) { tg = "ỵ"; c = 65; }
            else if (y[i+2] == 183) { tg = "ỷ"; c = 66; }
            else if (y[i+2] == 185) { tg = "ỹ"; c = 67; }

          }
          resultConvert[len++] = 128 + c;       
        }
        // Serial.print(tg);
        i = i + 2;
      }
    }
  }
  resultConvert[len] = '\0';
  return resultConvert;
}
void ShowArray(char* arr) {
  while (*arr) {
    Serial.print((int)*arr++);
    Serial.print(" "); 
  }
  Serial.println("");
}
void print(String s) {
  Serial.print(s);
}
void println(String s) {
  Serial.println(s);
}


