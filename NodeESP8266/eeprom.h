#include "Arduino.h"
#include <EEPROM.h>
void BeginEEPROM() {
  EEPROM.begin(512);
}
void ClearEEPROM()
{
  // write a 255 to all 512 bytes of the EEPROM
  for (int i = 0; i < 512; i++)
    EEPROM.write(i, 255);
  EEPROM.commit();
}
/*
 * Function Save String To EEPROM
 * Parameter : +data    : String Data
 *             +address : address in EEPROM
 * Return: None.
 */
void SaveStringToEEPROM(String data,int address)
{
  int len = data.length();
  if (len >= 255) {
    byte byte1 = 0xFF;
    byte byte2 = len - byte1;
    EEPROM.write(address, byte1);
    address++;
    EEPROM.write(address, byte2);
    // Serial.println("Save Byte1 :" + String(byte1));
    // Serial.println("Save Byte2 :" + String(byte2));
  } else {
    EEPROM.write(address, len);
    // Serial.println("Save Byte1 :" + String(len));
  } 
  for (int i=1;i<=len;i++)
    EEPROM.write(address+i,data.charAt(i-1));
  EEPROM.commit();
}
/*
 * Function Read String From EEPROM
 * Parameter : +address : address in EEPROM
 * Return: String.
 */
String ReadStringFromEEPROM(int address)
{
  String s="";
  int len=(int)EEPROM.read(address);
  if (len == 255) {
    address = address + 1;
    int byte2 = (int)EEPROM.read(address);
    len = len + byte2;
  }
  // Serial.println("Read length :" + String(len));
  for (int i=1;i<=len;i++)
    s+=(char)EEPROM.read(address+i);
  return s;
}


/*
 * Function Save Int To EEPROM
 * Parameter : +data    : int Data
 *             +address : address in EEPROM
 * Return: None.
 */
void SaveCharToEEPROM(char data,int address) {
  EEPROM.write(address,data); 
  EEPROM.commit();
}

/*
 * Function Read Int From EEPROM
 * Parameter : +address : address in EEPROM
 * Return: int.
 */
int ReadCharFromEEPROM(char address) {
  return EEPROM.read(address);
}

/*
 * Function Save Long To EEPROM
 * Parameter : +data    : int Data
 *             +address : address in EEPROM
 * Return: None.
 */
void SaveLongToEEPROM(long data,int address) {
  char byte1 = (data >> 24) & 0xFF;
  char byte2 = (data >> 16) & 0xFF;
  char byte3 = (data >> 8) & 0xFF;
  char byte4 = data & 0xFF;
  // Serial.println("Write Long:");
  // Serial.println("Byte1:" + String(byte1, HEX));
  // Serial.println("Byte2:" + String(byte2, HEX));
  // Serial.println("Byte3:" + String(byte3, HEX));
  // Serial.println("Byte4:" + String(byte4, HEX));
  EEPROM.write(address, byte1); 
  EEPROM.write(address+1, byte2); 
  EEPROM.write(address+2, byte3); 
  EEPROM.write(address+3, byte4); 
  EEPROM.commit();
}

/*
 * Function Read Long From EEPROM
 * Parameter : +address : address in EEPROM
 * Return: int.
 */
long ReadLongFromEEPROM(long address) {
  long byte1 = EEPROM.read(address);
  long byte2 = EEPROM.read(address + 1);
  long byte3 = EEPROM.read(address + 2);
  long byte4 = EEPROM.read(address + 3);
  // Serial.println("Read Long:");
  // Serial.println("Byte1:" + String(byte1, HEX));
  // Serial.println("Byte2:" + String(byte2, HEX));
  // Serial.println("Byte3:" + String(byte3, HEX));
  // Serial.println("Byte4:" + String(byte4, HEX));
  long result = ((byte1 << 24) & 0xFFFFFFFF)  + ((byte2 << 16 ) & 0xFFFFFF)  + ((byte3 << 8) & 0xFFFF)  + (byte4 & 0xFF);
  // Serial.println("Read Long:" + String(result));
  return result;
}