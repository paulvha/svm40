/*
 *  Version 1.0.1 / December 2020 / paulvha
 *  
 *  Example shows how to read basic values from the SVM40 using I2C communication.
 *  This differs from example1 as it will display raw numbers as well as system uptime.
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
// define Temperature to use 
// (true = Celsius / false = Fahrenheit)
/////////////////////////////////////////////////////////////
#define SetTemp true

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

void setup() {

 Serial.begin(115200);

  serialTrigger((char *) "SVM40-Example12: Basic + raw reading I2C. press <enter> to start");

  // set driver debug level
  svm40.EnableDebugging(DEBUG);

  Serial.println(F("Trying to connect."));

  SVM40_COMMS.begin();
  
  // Initialize SVM40 library
  if (! svm40.begin(&SVM40_COMMS))
    Errorloop((char *) "Could not set Wire communication channel.");

  // check for SVM40 connection
  if (! svm40.probe()) Errorloop((char *) "could not probe / connect with SVM40.");
  else  Serial.println(F("Detected SVM40."));

  // reset SVM40 connection
  if (! svm40.reset()) Errorloop((char *) "could not reset.");
  
  // read device info
  GetDeviceInfo();

  // set Temperature 
  svm40.SetTempCelsius(SetTemp);
   
  // start measurement
  if (svm40.start()) Serial.println(F("Measurement started"));
  else Errorloop((char *) "Could NOT start measurement");

  serialTrigger((char *) "Hit <enter> to continue reading.");
}

void loop() {

  // put your main code here, to run repeatedly:
  read_all();
  delay(3000);
}

/**
 * @brief : read and display all values
 */
bool read_all()
{
  static bool header = true;
  uint8_t ret, error_cnt = 0;
  struct svm40_values v;

  // loop to get data
  do {

    ret = svm40.GetValues(&v);

    // data might not have been ready
    if (ret == ERR_DATALENGTH){

        if (error_cnt++ > 3) {
          Serial.println(F( "Error during reading values"));
          return(false);
        }
        delay(1000);
    }

    // if other error
    else if(ret != ERR_OK) {
      Serial.println(F("Error during reading values"));
      return(false);
    }

  } while (ret != ERR_OK);

  // only print header first time
  if (header) {
    
    Serial.println(F("Voc_index    Humidity      Temperature       Raw-voc     Raw-humidity     Raw-temperature    System uptime"));
    Serial.print(F("Points\t\tRH%"));
    if (v.Celsius) Serial.print(F("\t\t*C\t\t"));
    else Serial.print(F("\t\t*F\t\t"));
    Serial.print(F("ticks\t\tRH%"));
    if (v.Celsius) Serial.println(F("\t\t*C\t\tSec"));
    else Serial.println(F("\t\t*F\t\tSec"));
    
    header = false;
  }

  Serial.print(v.VOC_index);
  Serial.print(F("\t\t"));
  Serial.print(v.humidity);
  Serial.print(F("\t\t"));
  Serial.print(v.temperature);
  Serial.print(F("\t\t"));
  Serial.print(v.raw_voc_ticks);
  Serial.print(F("\t\t"));
  Serial.print(v.raw_humidity);
  Serial.print(F("\t\t"));
  Serial.print(v.raw_temperature);

  // try to get system uptime in seconds
  uint32_t t;
  ret = svm40.GetSystemUpTime(&t);
  
  if (ret == ERR_OK)  {
    Serial.print(F("\t\t"));
    Serial.println(t);
  }
  else {
    Serial.println(F("\n\ncould not get system time."));
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
