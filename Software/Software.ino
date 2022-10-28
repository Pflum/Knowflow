/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi
 
 */

#include <SPI.h>
#include <UIPEthernet.h>
#include <EEPROM.h>
#include <DS18B20.h>
#include <DFRobot_PH.h>
#include <DFRobot_EC10.h>
#include <DFRobot_ORP_PRO.h>
#include <Arduino.h>

#define PH_PIN A2
#define EC_PIN A1
#define ORP_PIN A3
#define DO_PIN A4
#define TEMP_PIN 5
byte mac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
IPAddress ip(192, 168, 50, 200);
long int orpOffset = -482;
//Single-point calibration Mode=0
//Two-point calibration Mode=1
#define TWO_POINT_CALIBRATION 0
//Single point calibration needs to be filled CAL1_V and CAL1_T
#define CAL1_V (131) //mv
#define CAL1_T (25)   //℃
//Two-point calibration needs to be filled CAL2_V and CAL2_T
//CAL1 High temperature point, CAL2 Low temperature point
#define CAL2_V (1300) //mv
#define CAL2_T (15)   //℃

DS18B20 ds(TEMP_PIN);
EthernetServer server(80);
float doVoltage,orpVoltage,phVoltage,ecVoltage,phValue,ecValue,orpValue,doValue;

char orpString[10],tempString[6],phString[10],ecString[10],doString[10];
double tempValue;
int count = 0, dotemp, orpCalibrationValue, orpenterCalibrationFlag = 0, orpCalibrationFinish = 0, doCalibrationValue, doenterCalibrationFlag = 0, doCalibrationFinish = 0;

DFRobot_PH ph;
DFRobot_EC10 ec;
DFRobot_ORP_PRO orp(0);
const uint16_t DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410};

void setup() {
  Ethernet.init(10);  // Most Arduino shields

  Serial.begin(115200);
  Serial.println(F("Ethernet WebServer Example"));
  Ethernet.begin(mac, ip);
  ph.begin();
  ec.begin();
  orp.setCalibration(orpOffset);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  
  server.begin();
  Serial.print(F("server is at "));
  Serial.println(Ethernet.localIP());
}

void loop() {
  int tracker = 1;
  char json[90];
  // Temp
  tempValue = ds.getTempC();
  dtostrf(tempValue, 5, 2, tempString);
  // pH
  phVoltage = analogRead(PH_PIN)/1024.0*5000;
  phValue = ph.readPH(phVoltage,tempValue);
  dtostrf(phValue, 4, 2, phString);
  // EC
  ecVoltage = analogRead(EC_PIN)/1024.0*5000;
  ecValue = ec.readEC(ecVoltage,tempValue);
  dtostrf(ecValue, 3, 1, ecString);
  // ORP
  orpVoltage = analogRead(ORP_PIN)/1024.0*5000;
  orpValue = orp.getORP(orpVoltage);
  dtostrf(orpValue, 3, 1, orpString);
  // DO
  doVoltage = analogRead(DO_PIN)/1024.0*5000;
  doValue = readDO(doVoltage,tempValue);
  dtostrf(doValue, 4, 2, doString);
  
  sprintf(json, "{\"trackerId\":%d,\"temp\": %s,\"ec\": %s,\"ph\": %s,\"orp\": %s,\"do\": %s }", tracker, tempString, ecString, phString, orpString, doString);
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c == '\n' && currentLineIsBlank) {
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: application/json"));
          client.println(F("Connection: close"));  // the connection will be closed after completion of the response
          client.println(F("Refresh: 5"));  // refresh the page automatically every 5 sec
          client.println();
          client.println(json);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
  }
  if(count == 0) {
    Serial.println(json);
    count = 5;
  }
  count--;
  char cmd[10];
  if(readSerial(cmd)){
    strupr(cmd);
    Serial.println(cmd);
    if(strstr(cmd,"PH")){
      ph.calibration(phVoltage,tempValue,cmd);       //PH calibration process by Serail CMD
    }
    if(strstr(cmd,"EC")){
      ec.calibration(ecVoltage,tempValue,cmd);       //EC calibration process by Serail CMD
    }
    if(strstr(cmd,"ORP")){
      if(strstr(cmd,"ENTERORP")){
        Serial.println();
        Serial.println(F(">>>Enter ORP Calibration Mode<<<"));
        Serial.println(F(">>>Please disconnect probe and press the botton<<<"));
        Serial.println();
        orpenterCalibrationFlag = 1;
      }
      else if(strstr(cmd,"EXITORP") && orpenterCalibrationFlag){
        if(orpCalibrationFinish){
          orp.setCalibration(orpCalibrationValue);
          Serial.print(F(">>>Calibration Successful"));
        } else {
          Serial.print(F(">>>Calibration Failed"));
        }
        Serial.println(F(", Exit ORP Calibration Mode<<<"));
        Serial.print(orpCalibrationValue);
        Serial.println(F(" in die Variable orpOffset schreiben und neu flashen sonst ist diese Eistellung nach einem Stromausfall weg!")); 
        Serial.println();
        orpenterCalibrationFlag = 0;
        orpCalibrationFinish  = 0;
      }
      else if(strstr(cmd,"CALORP")){
        orpCalibrationValue = orp.calibrate(analogRead(ORP_PIN)/1024.0*5000);
        orpCalibrationFinish  = 1;
        Serial.println();
        Serial.println(F(">>>Send EXITORP to Save and Exit<<<")); 
        Serial.println();
      }
    }
    //#######################################
    
    if(strstr(cmd,"DO")){
      if(strstr(cmd,"ENTERDO")){
        Serial.println();
        Serial.println(F(">>>Enter DO Calibration Mode<<<"));
        Serial.println(F(">>>Please put the probe into the buffer solution<<<"));
        Serial.println();
        doenterCalibrationFlag = 1;
      }
      else if(strstr(cmd,"EXITDO") && doenterCalibrationFlag){
        if(doCalibrationFinish){
          orp.setCalibration(doCalibrationValue);
          Serial.print(F(">>>Calibration Successful"));
        } else {
          Serial.print(F(">>>Calibration Failed"));
        }
        Serial.println(F(", Exit DO Calibration Mode<<<"));
        Serial.print(doCalibrationValue);
        Serial.print("mV ");
        Serial.print(dotemp);
        Serial.print("C");
        Serial.println(F(" in die Variablen schreiben und neu flashen sonst ist diese Eistellung nach einem Stromausfall weg!")); 
        Serial.println();
        doenterCalibrationFlag = 0;
        doCalibrationFinish  = 0;
      }
      else if(strstr(cmd,"CALDO")){
        doCalibrationValue = analogRead(DO_PIN)/1024.0*5000;
        dotemp = ds.getTempC();
        doCalibrationFinish  = 1;
        Serial.println();
        Serial.println(F(">>>Send EXITDO to Save and Exit<<<")); 
        Serial.println();
      }
    }
    
    //########################################
  }
}


int i = 0;
bool readSerial(char result[]){
    while(Serial.available() > 0){
        char inChar = Serial.read();
        if(inChar == '\n'){
             result[i] = '\0';
             Serial.flush();
             i=0;
             return true;
        }
        if(inChar != '\r'){
             result[i] = inChar;
             i++;
        }
        delay(1);
    }
    return false;
}

int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c)
{
  if (TWO_POINT_CALIBRATION == 0){
    uint16_t V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature_c - (uint32_t)CAL1_T * 35;
    return (voltage_mv * DO_Table[temperature_c] / V_saturation);
  }
  else {
    uint16_t V_saturation = (int16_t)((int8_t)temperature_c - CAL2_T) * ((uint16_t)CAL1_V - CAL2_V) / ((uint8_t)CAL1_T - CAL2_T) + CAL2_V;
    return (voltage_mv * DO_Table[temperature_c] / V_saturation);
  }
}
