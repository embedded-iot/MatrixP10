#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiServer.h>

#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD2.h>        //
#include <fonts/Font_1.h>
//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 1
//SPIDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN, 5, 4, 12, 15);  // DMD controls the entire display
SPIDMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);
// DMD_TextBox box(dmd, 0, 0, 64, 16);
DMD_TextBox box(dmd);  

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
//  // ConvertStringToArrayChar("1234567890", true);
//  s1 = ConvertStringToArrayChar("aáàạảã", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("d đ", true);
//  s1 = ConvertStringToArrayChar("dđ", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("a á à ạ ả ã", true);
//  s1 = ConvertStringToArrayChar("aáàạảã", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("â ấ ầ ậ ẩ ẫ", true);
//  s1 = ConvertStringToArrayChar("âấầậẩẫ", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("ă ắ ằ ặ ẳ ẵ", true);
//  s1 = ConvertStringToArrayChar("ăắằặẳẵ", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("e é è ẹ ẻ ẽ", true);
//  s1 = ConvertStringToArrayChar("eéèẹẻẽ", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("ê ế ề ệ ể ễ", true);
//  s1 = ConvertStringToArrayChar("êếềệểễ", false);
//  ShowArray(s1);
//
//  // ConvertStringToArrayChar("i í ì ị ỉ ĩ", true);
//  s1 = ConvertStringToArrayChar("iíìịỉĩ", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("o ó ò ọ ỏ õ", true);
//  s1 = ConvertStringToArrayChar("oóòọỏõ", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("ô ố ồ ộ ổ ỗ", true);
//  s1 = ConvertStringToArrayChar("ôốồộổỗ", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("ơ ớ ờ ợ ở ỡ", true);
//  s1 = ConvertStringToArrayChar("ơớờợởỡ", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("u ú ù ụ ủ ũ", true);
//  s1 = ConvertStringToArrayChar("uúùụủũ", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("ư ứ ừ ự ử ữ", true);
//  s1 = ConvertStringToArrayChar("ưứừựửữ", false);
//  ShowArray(s1);
//  // ConvertStringToArrayChar("y ý ỳ ỵ ỷ ỹ", true);
//  s1 = ConvertStringToArrayChar("yýỳỵỷỹ", false);
//  ShowArray(s1);
  ConvertStringToArrayChar("A Á À Ạ Ả Ã", true);
  s1 = ConvertStringToArrayChar("AÁÀẠẢÃ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("Ă Ắ Ằ Ặ Ẳ Ẵ", true);
  s1 = ConvertStringToArrayChar("ĂẮẰẶẲẴ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("Â Ấ Ầ Ậ Ẩ Ẫ", true);
  s1 = ConvertStringToArrayChar("ÂẤẦẬẨẪ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("D Đ", true);
  s1 = ConvertStringToArrayChar("DĐ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("E É È Ẹ Ẻ Ẽ", true);
  s1 = ConvertStringToArrayChar("EÉÈẸẺẼ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("Ê Ế Ề Ệ Ể Ễ", true);
  s1 = ConvertStringToArrayChar("ÊẾỀỆỂỄ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("I Í Ì Ị Ỉ Ĩ", true);
  s1 = ConvertStringToArrayChar("IÍÌỊỈĨ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("O Ó Ò Ọ Ỏ Õ", true);
  s1 = ConvertStringToArrayChar("OÓÒỌỎÕ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("Ô Ố Ồ Ộ Ổ Ỗ", true);
  s1 = ConvertStringToArrayChar("ÔỐỒỘỔỖ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("Ơ Ớ Ờ Ợ Ở Ỡ", true);
  s1 = ConvertStringToArrayChar("ƠỚỜỢỞỠ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("U Ú Ù Ụ Ủ Ũ", true);
  s1 = ConvertStringToArrayChar("UÚÙỤỦŨ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("Ư Ứ Ừ Ự Ử Ữ", true);
  s1 = ConvertStringToArrayChar("ƯỨỪỰỬỮ", false);
  ShowArray(s1);
  ConvertStringToArrayChar("Y Ý Ỳ Ỵ Ỷ Ỹ", true);
  s1 = ConvertStringToArrayChar("YÝỲỴỶỸ", false);
  ShowArray(s1);
  s1 = dmd.ConvertStringToArrayChar("YÝỲỴỶỸ", false);
  s1 = dmd.ConvertStringToArrayChar("NGUYỄN VĂN QUÂN", false);

  dmd.setBrightness(1);
  dmd.selectFont(Font_1);
  dmd.begin();
  //  clear/init the DMD pixels held in RAM
  dmd.clearScreen();   //true is normal (all pixels off), false is negative (all pixels on)
  // dmd.drawString(0, 0, F("NGUYEN VAN QUAN"));
}

void loop() {
//  server.handleClient();
 testString();
//  testScroll();
  // scroll1();
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
  if (display) {
    Serial.println("\n" + x);
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
    // println("");
    for (int i = 0; i < length; i++) {
      char c = 0;
      if (y[i] < 128) {
        // Serial.print(y[i]);
        resultConvert[len++] = y[i];
        
      } else if (y[i] < 225) {
        String tg = "_";
        if (y[i] == 195 && i < length - 1) {
          if (y[i+1] == 161) { tg = "á"; c = 128; }
          else if (y[i+1] == 160) { tg = "à"; c = 129; }
          else if (y[i+1] == 163) { tg = "ã"; c = 132; }

          else if (y[i+1] == 162) { tg = "â"; c = 139; }

          else if (y[i+1] == 169) { tg = "é"; c = 146; }
          else if (y[i+1] == 168) { tg = "è"; c = 147; }

          else if (y[i+1] == 170) { tg = "ê"; c = 151; }

          else if (y[i+1] == 173) { tg = "í"; c = 157; }
          else if (y[i+1] == 172) { tg = "ì"; c = 158; }

          else if (y[i+1] == 179) { tg = "ó"; c = 162; }
          else if (y[i+1] == 178) { tg = "ò"; c = 163; }
          else if (y[i+1] == 181) { tg = "õ"; c = 166; }

          else if (y[i+1] == 180) { tg = "ô"; c = 167; }

          else if (y[i+1] == 186) { tg = "ú"; c = 179; }
          else if (y[i+1] == 185) { tg = "ù"; c = 180; }

          else if (y[i+1] == 189) { tg = "ý"; c = 190; }

          else if (y[i+1] == 129) { tg = "Á"; c = 195; }
          else if (y[i+1] == 128) { tg = "À"; c = 196; }
          else if (y[i+1] == 131) { tg = "Ã"; c = 199; }

          else if (y[i+1] == 130) { tg = "Â"; c = 206; }

          else if (y[i+1] == 137) { tg = "É"; c = 213; }
          else if (y[i+1] == 136) { tg = "È"; c = 214; }


          else if (y[i+1] == 138) { tg = "Ê"; c = 218; }

          else if (y[i+1] == 141) { tg = "Í"; c = 224; }
          else if (y[i+1] == 140) { tg = "Ì"; c = 225; }

          else if (y[i+1] == 147) { tg = "Ó"; c = 229; }
          else if (y[i+1] == 146) { tg = "Ò"; c = 230; }
          else if (y[i+1] == 149) { tg = "Õ"; c = 233; }

          else if (y[i+1] == 148) { tg = "Ô"; c = 234; }

          else if (y[i+1] == 154) { tg = "Ú"; c = 246; }
          else if (y[i+1] == 153) { tg = "Ù"; c = 247; }

          else if (y[i+1] == 157) { tg = "Ý"; c = 258 - 255; resultConvert[len++] = 255;}


        } else if (y[i] == 196 && i < length - 1) {
          if (y[i+1] == 131) { tg = "ă"; c = 133; }

          else if (y[i+1] == 169) { tg = "ĩ"; c = 161; }

          else if (y[i+1] == 145) { tg = "đ"; c = 145; }

          else if (y[i+1] == 130) { tg = "Ă"; c = 200; }

          else if (y[i+1] == 144) { tg = "Đ"; c = 212; }

          else if (y[i+1] == 168) { tg = "Ĩ"; c = 228; }


        } else if (y[i] == 197 && i < length - 1) {
          if (y[i+1] == 169) { tg = "ũ"; c = 183; }

          else if (y[i+1] == 168) { tg = "Ũ"; c = 250; }
          
        } else if (y[i] == 198 && i < length - 1) {
          if (y[i+1] == 161) { tg = "ơ"; c = 173; }

          else if (y[i+1] == 176) { tg = "ư"; c = 184; }

          else if (y[i+1] == 160) { tg = "Ơ"; c = 240; }

          else if (y[i+1] == 175) { tg = "Ư"; c = 251; }

        }
        resultConvert[len++] = c;
        // Serial.print(tg);
        i = i + 1;
      } else {
        String tg = "=";
        if (y[i] == 225 && i < length - 1) {
          if (y[i+1] == 186 && i + 1 < length - 1) {
            if (y[i+2] == 161) { tg = "ạ"; c = 130; }
            else if (y[i+2] == 163) { tg = "ả"; c = 131; }

            else if (y[i+2] == 165) { tg = "ấ"; c = 140; }
            else if (y[i+2] == 167) { tg = "ầ"; c = 141; }
            else if (y[i+2] == 173) { tg = "ậ"; c = 142; }
            else if (y[i+2] == 169) { tg = "ẩ"; c = 143; }
            else if (y[i+2] == 171) { tg = "ẫ"; c = 144; }

            else if (y[i+2] == 175) { tg = "ắ"; c = 134; }
            else if (y[i+2] == 177) { tg = "ằ"; c = 135; }
            else if (y[i+2] == 183) { tg = "ặ"; c = 136; }
            else if (y[i+2] == 179) { tg = "ẳ"; c = 137; }
            else if (y[i+2] == 181) { tg = "ẵ"; c = 138; }

            else if (y[i+2] == 185) { tg = "ẹ"; c = 148; }
            else if (y[i+2] == 187) { tg = "ẻ"; c = 149; }
            else if (y[i+2] == 189) { tg = "ẽ"; c = 150; }
            else if (y[i+2] == 191) { tg = "ế"; c = 152; }

            else if (y[i+2] == 160) { tg = "Ạ"; c = 197; }
            else if (y[i+2] == 162) { tg = "Ả"; c = 198; }

            else if (y[i+2] == 174) { tg = "Ắ"; c = 201; }
            else if (y[i+2] == 176) { tg = "Ằ"; c = 202; }
            else if (y[i+2] == 182) { tg = "Ặ"; c = 203; }
            else if (y[i+2] == 178) { tg = "Ẳ"; c = 204; }
            else if (y[i+2] == 180) { tg = "Ẵ"; c = 205; }

            else if (y[i+2] == 164) { tg = "Ấ"; c = 207; }
            else if (y[i+2] == 166) { tg = "Ầ"; c = 208; }
            else if (y[i+2] == 172) { tg = "Ậ"; c = 209; }
            else if (y[i+2] == 168) { tg = "Ẩ"; c = 210; }
            else if (y[i+2] == 170) { tg = "Ẫ"; c = 211; }

            else if (y[i+2] == 184) { tg = "Ẹ"; c = 215; }
            else if (y[i+2] == 186) { tg = "Ẻ"; c = 216; }
            else if (y[i+2] == 188) { tg = "Ẽ"; c = 217; }

            else if (y[i+2] == 190) { tg = "Ế"; c = 219; }



          } else if (y[i+1] == 187 && i + 1 < length - 1) {
            
            if (y[i+2] == 129) { tg = "ề"; c = 153; }
            else if (y[i+2] == 135) { tg = "ệ"; c = 154; }
            else if (y[i+2] == 131) { tg = "ể"; c = 155; }
            else if (y[i+2] == 133) { tg = "ễ"; c = 156; }

            else if (y[i+2] == 139) { tg = "ị"; c = 159; }
            else if (y[i+2] == 137) { tg = "ỉ"; c = 160; }

            else if (y[i+2] == 141) { tg = "ọ"; c = 164; }
            else if (y[i+2] == 143) { tg = "ỏ"; c = 165; }

            else if (y[i+2] == 145) { tg = "ố"; c = 168; }
            else if (y[i+2] == 147) { tg = "ồ"; c = 169; }
            else if (y[i+2] == 153) { tg = "ộ"; c = 170; }
            else if (y[i+2] == 149) { tg = "ổ"; c = 171; }
            else if (y[i+2] == 151) { tg = "ỗ"; c = 172; }

            else if (y[i+2] == 155) { tg = "ớ"; c = 174; }
            else if (y[i+2] == 157) { tg = "ờ"; c = 175; }
            else if (y[i+2] == 163) { tg = "ợ"; c = 176; }
            else if (y[i+2] == 159) { tg = "ở"; c = 177; }
            else if (y[i+2] == 161) { tg = "ỡ"; c = 178; }

            else if (y[i+2] == 165) { tg = "ụ"; c = 181; }
            else if (y[i+2] == 167) { tg = "ủ"; c = 182; }

            else if (y[i+2] == 169) { tg = "ứ"; c = 185; }
            else if (y[i+2] == 171) { tg = "ừ"; c = 186; }
            else if (y[i+2] == 177) { tg = "ự"; c = 187; }
            else if (y[i+2] == 173) { tg = "ử"; c = 188; }
            else if (y[i+2] == 175) { tg = "ữ"; c = 189; }

            else if (y[i+2] == 179) { tg = "ỳ"; c = 191; }
            else if (y[i+2] == 181) { tg = "ỵ"; c = 192; }
            else if (y[i+2] == 183) { tg = "ỷ"; c = 193; }
            else if (y[i+2] == 185) { tg = "ỹ"; c = 194; }

            else if (y[i+2] == 128) { tg = "Ề"; c = 220; }
            else if (y[i+2] == 134) { tg = "Ệ"; c = 221; }
            else if (y[i+2] == 130) { tg = "Ể"; c = 222; }
            else if (y[i+2] == 132) { tg = "Ễ"; c = 223; }

            else if (y[i+2] == 138) { tg = "Ị"; c = 226; }
            else if (y[i+2] == 136) { tg = "Ỉ"; c = 227; }

            else if (y[i+2] == 140) { tg = "Ọ"; c = 231; }
            else if (y[i+2] == 142) { tg = "Ỏ"; c = 232; }

            else if (y[i+2] == 144) { tg = "Ố"; c = 235; }
            else if (y[i+2] == 146) { tg = "Ồ"; c = 236; }
            else if (y[i+2] == 152) { tg = "Ộ"; c = 237; }
            else if (y[i+2] == 148) { tg = "Ổ"; c = 238; }
            else if (y[i+2] == 150) { tg = "Ỗ"; c = 239; }

            else if (y[i+2] == 154) { tg = "Ớ"; c = 241; }
            else if (y[i+2] == 156) { tg = "Ờ"; c = 242; }
            else if (y[i+2] == 162) { tg = "Ợ"; c = 243; }
            else if (y[i+2] == 158) { tg = "Ở"; c = 244; }
            else if (y[i+2] == 160) { tg = "Ỡ"; c = 245; }

            else if (y[i+2] == 164) { tg = "Ụ"; c = 248; }
            else if (y[i+2] == 166) { tg = "Ủ"; c = 249; }

            else if (y[i+2] == 168) { tg = "Ứ"; c = 252; }
            else if (y[i+2] == 170) { tg = "Ừ"; c = 253; }
            else if (y[i+2] == 176) { tg = "Ự"; c = 254; }
            else if (y[i+2] == 172) { tg = "Ử"; c = 256 - 255; resultConvert[len++] = 255;}
            else if (y[i+2] == 174) { tg = "Ữ"; c = 257 - 255; resultConvert[len++] = 255;}

            else if (y[i+2] == 178) { tg = "Ỳ"; c = 259 - 255; resultConvert[len++] = 255;}
            else if (y[i+2] == 180) { tg = "Ỵ"; c = 260 - 255; resultConvert[len++] = 255;}
            else if (y[i+2] == 182) { tg = "Ỷ"; c = 261 - 255; resultConvert[len++] = 255;}
            else if (y[i+2] == 184) { tg = "Ỹ"; c = 262 - 255; resultConvert[len++] = 255;}



          }
          resultConvert[len++] = c;       
        }
        // Serial.print(tg);
        i = i + 2;
      }
    }
    // show("\nlen:" + String(len) + "\n");
  }
  resultConvert[len] = '\0';
  return resultConvert;
}
void ShowArray(char* arr) {
  Serial.println("");
  while (*arr) {
    Serial.print((int)*arr++);
    Serial.print(" "); 
  }
}
void print(String s) {
  Serial.print(s);
}
void println(String s) {
  Serial.println(s);
}

void testString() {
  dmd.selectFont(Font_1);
  const char *MSG ; 
  // const String arrayMsg[50] = {"aáàạảã", "ăắằặẳẵ", "âấầậẩẫ", "dđ", "eéèẹẻẽ", "êếềệểễ", "iíìịỉĩ", "oóòọỏõ","ôốồộổỗ", "ơớờợởỡ", "uúùụủũ","ưứừựửữ","yýỳỵỷỹ","abcde","fghij","klmno","pqrst","uvwxyz","01234","56789","ABCDE", "FGHIJ", "KLMNO","KLMNO", "PQRST", "UVWXYZ"};
  // MSG = ConvertStringToArrayChar("aáàạảã", false);
  // MSG = ConvertStringToArrayChar("ăắằặẳẵ", false);
  // MSG = ConvertStringToArrayChar("âấầậẩẫ", false);
  // MSG = ConvertStringToArrayChar("dđ", false);
  // MSG = ConvertStringToArrayChar("eéèẹẻẽ", false);
  // MSG = ConvertStringToArrayChar("êếềệểễ", false);
  // MSG = ConvertStringToArrayChar("iíìịỉĩ", false);
  // MSG = ConvertStringToArrayChar("oóòọỏõ", false);
  // MSG = ConvertStringToArrayChar("ôốồộổỗ", false);
  // MSG = ConvertStringToArrayChar("ơớờợởỡ", false);
  // MSG = ConvertStringToArrayChar("uúùụủũ", false);
  // MSG = ConvertStringToArrayChar("ưứừựửữ", false);
  // MSG = ConvertStringToArrayChar("yýỳỵỷỹ", false);
  // MSG = ConvertStringToArrayChar("abcde", false);
  // MSG = ConvertStringToArrayChar("fghij", false);
  // MSG = ConvertStringToArrayChar("klmno", false);
  // MSG = ConvertStringToArrayChar("pqrst", false);
  // MSG = ConvertStringToArrayChar("uvwxyz", false);
  // MSG = ConvertStringToArrayChar("01234", false);
  // MSG = ConvertStringToArrayChar("56789", false);
  // MSG = ConvertStringToArrayChar("ABCDE", false);
  // MSG = ConvertStringToArrayChar("FGHIJ", false);
  // MSG = ConvertStringToArrayChar("KLMNO", false);
  // MSG = ConvertStringToArrayChar("PQRST", false);
  // MSG = ConvertStringToArrayChar("UVWXYZ", false);


  // MSG = ConvertStringToArrayChar("AÁÀẠẢÃ", false);
  // MSG = ConvertStringToArrayChar("ĂẮẰẶẲẴ", false);
  // MSG = ConvertStringToArrayChar("ẮẰẶẲẴ", false);
  // MSG = ConvertStringToArrayChar("ÂẤẦẬẨẪ", false);
  // MSG = ConvertStringToArrayChar("DĐ", false);
  // MSG = ConvertStringToArrayChar("EÉÈẸẺẼ", false);
  // MSG = ConvertStringToArrayChar("ÊẾỀỆỂỄ", false);
  // MSG = ConvertStringToArrayChar("IÍÌỊỈĨ", false);
  // MSG = ConvertStringToArrayChar("OÓÒỌỎÕ", false);
  // MSG = ConvertStringToArrayChar("ÔỐỒỘỔỖ", false);
  // MSG = ConvertStringToArrayChar("ỐỒỘỔỖ", false);
  // MSG = ConvertStringToArrayChar("ƠỚỜỢỞỠ", false);
  // MSG = ConvertStringToArrayChar("ỚỜỢỞỠ", false);
  // MSG = dmd.ConvertStringToArrayChar("UÚÙỤỦŨ", false);
  // MSG = dmd.ConvertStringToArrayChar("ƯỨỪỰỬỮ", false);
  // MSG = ConvertStringToArrayChar("ỰỬỮ", false);
  MSG = dmd.ConvertStringToArrayChar("YÝỲỴỶỸ", false);
//  for (int i = 0; i < 26; i++) {
//    dmd.fillScreen(false);
//    MSG = ConvertStringToArrayChar(arrayMsg[i], false);
//    dmd.drawString(0,0, MSG, GRAPHICS_ON); 
//    delay(2000);
//  }
    dmd.fillScreen(false);
    dmd.drawString(0,0, MSG, GRAPHICS_ON); 
    delay(2000);
}
DMD_TextBox box1(dmd);  // "box" provides a text box to automatically write to/scroll the display
void testScroll() {
  const char *next = ConvertStringToArrayChar("NGUYỄN VĂN QUÂN", false);
  while(*next) {
    Serial.print(*next);
    box1.print(*next);
    delay(500);
    next++;
  }
}
int phase = 0; // 0-3, 'phase' value determines direction
void scroll1() {
  int steps = random(48); // Each time we scroll a random distance
  for(int i = 0; i < steps; i++) {
    // Do a different type of scroll, depending on which phase we are in
    switch(0) {
      case 0:
       dmd.marqueeScrollX(1); break;
      case 1:
       dmd.marqueeScrollX(-1); break;
      case 2:
       dmd.marqueeScrollY(1); break;
      case 3:
       dmd.marqueeScrollY(-1); break;
    }
    delay(100);
  }

  // Move to the next phase
  phase = (phase + 1) % 4;
}

void scrollTest() {
}
