#include "Arduino.h"
#include <FS.h>
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
    f.println(content);
    f.close();  //Close file
    return true;
  }
  return false;
}
bool removeFile(String path) {
  if (SPIFFS.remove(path)) {
    return true;
  }
  return false;
}