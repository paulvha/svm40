/*
 * Version 1.0.1 / December 2020 / paulvha
 * 
 * This example tries to read the VOC algorithm state and write them back.
 */
 //////////////////////////////////////////////////////////////////////////
 // GIVEN THE FIRMWARE LEVEL 1.1. ON MY SVM40 THIS HAS NOT BEEN TESTED !!!!
 //////////////////////////////////////////////////////////////////////////
/* 
 *   READING :
 *   Gets the current VOC algorithm state. Retrieved values can be used to set
 *   the VOC algorithm state to resume operation after a short interruption,
 *   skipping initial learning phase. This command is only available during
 *   measurement mode.
 *
 *   .. note:: This feature can only be used after at least 3 hours of
 *             continuous operation.
 *             
 *   WRITING
 *   Set previously retrieved VOC algorithm state to resume operation after a
 *   short interruption, skipping initial learning phase. This command is only
 *   available in idle mode.
 *    
 *    . note:: This feature should not be used after interruptions of more than
 *              10 minutes.
 *             
 ********************************************************************************
 *  HARDWARE CONNECTION
 *  ..........................................................
 *  Successfully tested on ATMEGA2560, Due
 *  
 *  SVM40 pin          ATMEGA
 *  1 VCC ---  RED   --- 5V
 *  2 GND ---  BLACK --- GND
 *  3 SDA ---  GREEN  -- SDA
 *  4 SCL ---  YELLOW -- SCL
 *  5 Select-- BLUE ---- GND  (select I2C, BEFORE applying power)
 *  6 nc      PURPLE         (NOT CONNECTED)
 *
 *  
 *  Open the serial monitor at 115200 baud
 *
 *  ================================ Disclaimer ======================================
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  ===================================================================================
 *  NO support, delivered as is, have fun, good luck !!
 */
 
/////////////////////////////////////////////////////////////
// define serial communication channel to use for SVM40
/////////////////////////////////////////////////////////////
#define SVM40_COMMS Wire

/////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request sending and receiving
 * 2 : request sending and receiving + show protocol errors */
 //////////////////////////////////////////////////////////////
#define DEBUG 0

#include "svm40.h"

// create constructor
SVM40 svm40;

uint8_t voc_state[8];

void setup() {

 Serial.begin(115200);

  serialTrigger((char *) "SVM40-Example13: Get & Set VOC_state with I2C. press <enter> to start");

  // set driver debug level
  svm40.EnableDebugging(DEBUG);

  Serial.println(F("Trying to connect."));

  SVM40_COMMS.begin();
  
  // Initialize SVM40 library
  if (! svm40.begin(&SVM40_COMMS))
    Errorloop((char *) "Could not set Wire communication channel.");

  // check for SVM40 connection
  if (! svm40.probe()) Errorloop((char *) "could not probe / connect with SVM40.");
  else Serial.println(F("Detected SVM40."));

  // reset SVM40 connection
  if (! svm40.reset()) Errorloop((char *) "could not reset.");
  
  // read device info
  GetDeviceInfo();

  serialTrigger((char *) "Hit <enter> to READ voc_state.");

  read_vocstate();

  serialTrigger((char *) "Hit <enter> to WRITE voc_state.");
  
  write_vocstate();

  Serial.println("Done\n");
}

void loop() {

}

// read the VOC state table for quick restore 
bool read_vocstate()
{
  uint8_t ret = svm40.GetVocState(voc_state);
  
  // data might not have been ready
  if (ret == ERR_DATALENGTH) {
    Serial.println(F("Error during reading values"));
    return(false);
  }
  
  // if error
  else if(ret != ERR_OK) {
    Serial.println(F("Error during reading values"));
    return(false);
  }
  
  Serial.print(F("Voc_state values: ")); 
  
  for(int i = 0; i < 8; i++){
    Serial.print("0x");
    if (voc_state[i] < 0) Serial.print("0");
    Serial.print(voc_state[i],HEX);
  }
  Serial.println();

  return(true);
}

bool write_vocstate()
{
  Serial.print(F("Voc_state values to write: ")); 
  
  for(int i = 0; i < 8; i++){
    Serial.print("0x");
    if (voc_state[i] < 0) Serial.print("0");
    Serial.print(voc_state[i],HEX);
  }
  Serial.println();
    
  uint8_t ret = svm40.SetVocState(voc_state);

  // if error
  if(ret != ERR_OK) {
      Serial.println(F("Error during setting values"));
      return(false);
  }

  Serial.println(F("VOC values have been updated\n"));
  return(true);
}

/**
 * @brief : read and display device info
 */
void GetDeviceInfo()
{
  char buf[32];
  uint8_t ret;
  SVM40_version v;

  //try to read serial number
  ret = svm40.GetSerialNumber(buf, 32);
  if (ret == ERR_OK) {
    Serial.print(F("Serial number : "));
    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    Serial.println(F("could not get serial number"));

  // try to get product name
  ret = svm40.GetProductName(buf, 32);
  if (ret == ERR_OK)  {
    Serial.print(F("Product name  : "));

    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    Serial.println(F("could not get product name."));

  // try to get product Type
  ret = svm40.GetProductType(buf, 32);
  if (ret == ERR_OK)  {
    Serial.print(F("Product Type  : "));

    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    Serial.println(F("could not get product name."));

  // try to get version info
  ret = svm40.GetVersion(&v);
  if (ret != ERR_OK) {
    Serial.println(F("Can not read version info"));
    return;
  }

  Serial.print(F("Firmware level: "));  Serial.print(v.major);
  Serial.print(".");  Serial.print(v.minor); Serial.print("\t");  
  if (v.debug) Serial.println("debug enabled");
  else Serial.println("debug disabled");
  
  Serial.print(F("Hardware level: "));  Serial.print(v.HW_major);
  Serial.print(".");  Serial.println(v.HW_minor);
  
  Serial.print(F("SHDLC protocol: ")); Serial.print(v.SHDLC_major);
  Serial.print(".");  Serial.println(v.SHDLC_minor);

  Serial.print(F("Library level : "));  Serial.print(v.DRV_major);
  Serial.print(".");  Serial.println(v.DRV_minor);
}


/**
 *  @brief : continued loop after fatal error
 *  @param mess : message to display
 */
void Errorloop(char *mess)
{
  Serial.println(mess);
  Serial.println(F("Program on hold"));
  for(;;) delay(100000);
}

/**
 * serialTrigger prints repeated message, then waits for enter
 * to come in from the serial port.
 */
void serialTrigger(char * mess)
{
  Serial.println();

  while (!Serial.available()) {
    Serial.println(mess);
    delay(2000);
  }

  while (Serial.available())
    Serial.read();
}
