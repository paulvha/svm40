/*
 *  Version 1.0 / December 2020 / paulvha
 *  
 *  Example shows how to apply and what the impact is of temperature offset with I2C.
 *   
 *  One can increase the temperature reading (and decrease humidity) with a positive offset
 *  with a negative offset the temperature reading will decrease (and humidity will increase)
 *   
 *  This can be used in case environmental circumstances have a big impact on the reading.
 *  
 *  The update will happen in RAM, but you can set UpdateNvRAM-parameter below to store the 
 *  Temperature offset in non-volatile memory. It will then continue to be used after a next 
 *  power-up  / reset
 *
 ********************************************************************************
 *  HARDWARE CONNECTION
 *  ..........................................................
 *  Successfully tested on ATMEGA2560, Due
 *  
 *  SVM40 pin          ATMEGA
 *  1 VCC --- RED    --- 5V
 *  2 GND --- BLACK  --- GND
 *  3 SDA  --- GREEN  -- SDA
 *  4 SCL  --- YELLOW -- SCL
 *  5 Select-  BLUE ---- GND  (select I2C, BEFORE applying power)
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
// define Temperature to obtain 
// (true = Celsius / false = Fahrenheit)
/////////////////////////////////////////////////////////////
#define SetTemp true

/////////////////////////////////////////////////////////////
// define whether or not to update the temperature offset in 
// non-volatile memory
// (true = update / false = do NOT update)
/////////////////////////////////////////////////////////////
#define UpdateNvRAM false

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

int16_t TempOffset;

void setup() {

 Serial.begin(115200);

  serialTrigger((char *) "SVM40-Example14: Get & Set Temperature offset over I2C. !! press <enter> to start");

  // set driver debug level
  svm40.EnableDebugging(DEBUG);

  Serial.println(F("Trying to connect."));

  SVM40_COMMS.begin();
  
  // Initialize SVM40 library
  if (! svm40.begin(&SVM40_COMMS))
    Errorloop((char *) "Could not set Wire communication channel.", 0);

  // check for SVM40 connection
  if (! svm40.probe()) Errorloop((char *) "could not probe / connect with SVM40.", 0);
  else  Serial.println(F("Detected SVM40."));

  // reset SVM40 connection
  if (! svm40.reset()) Errorloop((char *) "could not reset.", 0);
  
  // read device info
  GetDeviceInfo();

  // set Temperature standard to obtain
  svm40.SetTempCelsius(SetTemp);
   
  // start measurement
  if (svm40.start()) Serial.println(F("Measurement started"));
  else Errorloop((char *) "Could NOT start measurement", 0);
}

void loop() {
  serialTrigger((char *) "Hit <enter> to READ Temperature offset.");
  int i;
   
  read_temp_offset();
   
  for(i = 0; i < 2; i++){
    read_temp();
    delay(1000);
  }

  serialTrigger((char *) "Hit <enter> to WRITE Temperature offset.");
  
  write_temp_offset();

 for(i = 0; i < 2; i++){
    read_temp();
    delay(1000);
  }

  Serial.println("Done\n");

}

// read temperature
void read_temp()
{
   struct svm40_values v;
   uint8_t ret = svm40.GetValues(&v);
   
   if(ret != ERR_OK) {
    Serial.println("error during reading\n");
    return;
   }
   
  Serial.print(v.humidity);
  Serial.print(F("\t\t"));
  Serial.print(v.temperature);
  if(SetTemp) Serial.println(F("*C"));
  else Serial.println(F("*F"));
}

// read the current temperature offset
// zero means no offset applied.

bool read_temp_offset()
{
  uint8_t ret = svm40.GetTemperatureOffset(&TempOffset);
  
  // data might not have been ready
  if (ret == ERR_DATALENGTH) {
        ErrtoMess((char *) "Error during reading values: ",ret);
        return(false);
  }
  // if  error
  else if(ret != ERR_OK) {
    ErrtoMess((char *) "Error during reading values: ",ret);
    return(false);
  }
  
  Serial.print("TempOffset: "); 
  Serial.println(TempOffset);
 
  return(true);
}

// set a new offset manually
bool write_temp_offset()
{
  // here is just example for the loop.
  // the offset can be positive (the temperature reading will increase)
  // the offset can be negative (the temperature reading will decrease)
  // the change is in degree celsius.
  
  TempOffset = 1;
  
  Serial.print("TempOffset to write: "); 
  Serial.println(TempOffset);
    
  uint8_t ret = svm40.SetTemperatureOffset(TempOffset);

  // if  error
  if(ret != ERR_OK) {
      ErrtoMess((char *) "Error during reading values: ",ret);
      return(false);
  }

  Serial.println("Temperature offset has been updated\n");

  if (! UpdateNvRAM) return(true);

  Serial.println("Updating non-volatile memory\n");
  
  // you can make the change permanent and store in Nov Ram
  ret = svm40.StoreNvData();
  
  if(ret != ERR_OK) {
      ErrtoMess((char *) "Error during setting to non-volatile memory: ",ret);
      return(false);
  }

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
    ErrtoMess((char *) "could not get serial number", ret);

  // try to get product name
  ret = svm40.GetProductName(buf, 32);
  if (ret == ERR_OK)  {
    Serial.print(F("Product name  : "));

    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    ErrtoMess((char *) "could not get product name.", ret);

  // try to get product Type
  ret = svm40.GetProductType(buf, 32);
  if (ret == ERR_OK)  {
    Serial.print(F("Product Type  : "));

    if(strlen(buf) > 0)  Serial.println(buf);
    else Serial.println(F("not available"));
  }
  else
    ErrtoMess((char *) "could not get product name.", ret);

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
 *  @param r : error code
 *
 *  if r is zero, it will only display the message
 */
void Errorloop(char *mess, uint8_t r)
{
  if (r) ErrtoMess(mess, r);
  else Serial.println(mess);
  Serial.println(F("Program on hold"));
  for(;;) delay(100000);
}

/**
 *  @brief : display error message
 *  @param mess : message to display
 *  @param r : error code
 *
 */
void ErrtoMess(char *mess, uint8_t r)
{
  char buf[80];

  Serial.print(mess);

  //svm40.GetErrDescription(r, buf, 80);
  Serial.println(buf);
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
