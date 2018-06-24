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
  ConvertStringToArrayChar("d đ", true);
  ConvertStringToArrayChar("d đ", false);

  ConvertStringToArrayChar("a á à ạ ả ã", true);
  ConvertStringToArrayChar("a á à ạ ả ã", false);

  ConvertStringToArrayChar("â ấ ầ ậ ẩ ẫ", true);
  ConvertStringToArrayChar("â ấ ầ ậ ẩ ẫ", false);

  ConvertStringToArrayChar("ă ắ ằ ặ ẳ ẵ", true);
  ConvertStringToArrayChar("ă ắ ằ ặ ẳ ẵ", false);

  ConvertStringToArrayChar("e é è ẹ ẻ ẽ", true);
  ConvertStringToArrayChar("e é è ẹ ẻ ẽ", false);
  
  ConvertStringToArrayChar("ê ế ề ệ ể ễ", true);
  ConvertStringToArrayChar("ê ế ề ệ ể ễ", false);


  ConvertStringToArrayChar("i í ì ị ỉ ĩ", true);
  ConvertStringToArrayChar("i í ì ị ỉ ĩ", false);

  ConvertStringToArrayChar("o ó ò ọ ỏ õ", true);
  ConvertStringToArrayChar("o ó ò ọ ỏ õ", false);

  ConvertStringToArrayChar("ô ố ồ ộ ổ ỗ", true);
  ConvertStringToArrayChar("ô ố ồ ộ ổ ỗ", false);

  ConvertStringToArrayChar("ơ ớ ờ ợ ở ỡ", true);
  ConvertStringToArrayChar("ơ ớ ờ ợ ở ỡ", false);

  ConvertStringToArrayChar("u ú ù ụ ủ ũ", true);
  ConvertStringToArrayChar("u ú ù ụ ủ ũ", false);
  
  ConvertStringToArrayChar("ư ứ ừ ự ử ữ", true);
  ConvertStringToArrayChar("ư ứ ừ ự ử ữ", false);

  ConvertStringToArrayChar("y ý ỳ ỵ ỷ ỹ", true);
  ConvertStringToArrayChar("y ý ỳ ỵ ỷ ỹ", false);

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


void ConvertStringToArrayChar(String x, bool display) {
  int length = x.length();
  if (display) {
    Serial.println("\n" + x);
    Serial.println("length :" + String(length));
  }
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
    Serial.println("");
    for (int i = 0; i < length; i++) {
      if (y[i] < 128) {
        Serial.print(y[i]);
      } else if (y[i] < 225) {
        String tg = "_";
        if (y[i] == 195 && i < length - 1) {
          if (y[i+1] == 161) tg = "á";
          else if (y[i+1] == 160) tg = "à";
          else if (y[i+1] == 163) tg = "ã";

          else if (y[i+1] == 162) tg = "â";

          else if (y[i+1] == 169) tg = "é";
          else if (y[i+1] == 168) tg = "è";

          else if (y[i+1] == 170) tg = "ê";

          else if (y[i+1] == 173) tg = "í";
          else if (y[i+1] == 172) tg = "ì";

          else if (y[i+1] == 179) tg = "ó";
          else if (y[i+1] == 178) tg = "ò";
          else if (y[i+1] == 181) tg = "õ";

          else if (y[i+1] == 180) tg = "ô";

          else if (y[i+1] == 186) tg = "ú";
          else if (y[i+1] == 185) tg = "ù";

          else if (y[i+1] == 189) tg = "ý";

        } else if (y[i] == 196 && i < length - 1) {
          if (y[i+1] == 131) tg = "ă";

          else if (y[i+1] == 169) tg = "ĩ";

          else if (y[i+1] == 145) tg = "đ";
          
        } else if (y[i] == 197 && i < length - 1) {
          if (y[i+1] == 169) tg = "ũ";

          // else if (y[i+1] == 169) tg = "ĩ";
          
        } else if (y[i] == 198 && i < length - 1) {
          if (y[i+1] == 161) tg = "ơ";

          else if (y[i+1] == 176) tg = "ư";
        }

        Serial.print(tg);
        i = i + 1;
      } else {
        String tg = "=";
        if (y[i] == 225 && i < length - 1) {
          if (y[i+1] == 186 && i + 1 < length - 1) {
            if (y[i+2] == 161) tg = "ạ";
            else if (y[i+2] == 163) tg = "ả";

            else if (y[i+2] == 165) tg = "ấ";
            else if (y[i+2] == 167) tg = "ầ";
            else if (y[i+2] == 173) tg = "ậ";
            else if (y[i+2] == 169) tg = "ẩ";
            else if (y[i+2] == 171) tg = "ẫ";

            else if (y[i+2] == 175) tg = "ắ";
            else if (y[i+2] == 177) tg = "ằ";
            else if (y[i+2] == 183) tg = "ặ";
            else if (y[i+2] == 179) tg = "ẳ";
            else if (y[i+2] == 181) tg = "ẵ";

            else if (y[i+2] == 185) tg = "ẹ";
            else if (y[i+2] == 187) tg = "ẻ";
            else if (y[i+2] == 189) tg = "ẽ";
            else if (y[i+2] == 191) tg = "ế";

          } else if (y[i+1] == 187 && i + 1 < length - 1) {
            
            if (y[i+2] == 129) tg = "ề";
            else if (y[i+2] == 135) tg = "ệ";
            else if (y[i+2] == 131) tg = "ể";
            else if (y[i+2] == 133) tg = "ễ";

            else if (y[i+2] == 139) tg = "ị";
            else if (y[i+2] == 137) tg = "ỉ";

            else if (y[i+2] == 141) tg = "ọ";
            else if (y[i+2] == 143) tg = "ỏ";

            else if (y[i+2] == 145) tg = "ố";
            else if (y[i+2] == 147) tg = "ồ";
            else if (y[i+2] == 153) tg = "ộ";
            else if (y[i+2] == 149) tg = "ổ";
            else if (y[i+2] == 151) tg = "ỗ";

            else if (y[i+2] == 155) tg = "ớ";
            else if (y[i+2] == 157) tg = "ờ";
            else if (y[i+2] == 163) tg = "ợ";
            else if (y[i+2] == 159) tg = "ở";
            else if (y[i+2] == 161) tg = "ỡ";

            else if (y[i+2] == 165) tg = "ụ";
            else if (y[i+2] == 167) tg = "ủ";

            else if (y[i+2] == 169) tg = "ứ";
            else if (y[i+2] == 171) tg = "ừ";
            else if (y[i+2] == 177) tg = "ự";
            else if (y[i+2] == 173) tg = "ử";
            else if (y[i+2] == 175) tg = "ữ";

            else if (y[i+2] == 179) tg = "ỳ";
            else if (y[i+2] == 181) tg = "ỵ";
            else if (y[i+2] == 183) tg = "ỷ";
            else if (y[i+2] == 185) tg = "ỹ";

          }       
        }
        Serial.print(tg);
        i = i + 2;
      }
    }
  }
}


