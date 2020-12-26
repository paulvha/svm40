/*
 *   Version 1.0.1 / December 2020 / paulvha
 *  
 *   Example shows how to apply the Algorithm changes.
 *   
 *  - voc_index_offset (int) -
 *    VOC index representing typical (average) conditions. The default
 *    value is 100.
 *  - learning_time_hours (int) -
 *    Time constant of long-term estimator in hours. Past events will
 *     be forgotten after about twice the learning time. The default
 *     value is 12 hours.
 *   - gating_max_duration_minutes (int) -
 *     Maximum duration of gating in minutes (freeze of estimator during
 *     high VOC index signal). Zero disables the gating. The default
 *     value is 180 minutes.
 *   - std_initial (int) -
 *     Initial estimate for standard deviation. Lower value boosts
 *     events during initial learning period, but may result in larger
 *     device-to-device variations. The default value is 50.
 *   
 *  The update will happen in RAM, but you can set UpdateNvRAM-parameter below to store the 
 *  Temperature offset in non-volatile memory. It will then continue to be used after a next 
 *  power-up  / reset
 *
 *  ..........................................................
 *  Successfully tested on ATMEGA2560, Due
 *  Used Serial1.
 *  SVM40 pin          ATMEGA
 *  1 VCC --- RED    --- 5V
 *  2 GND --- BLACK  --- GND
 *  3 TX  --- GREEN  --- RX1
 *  4 RX  --- YELLOW --- TX1
 *  5 Select-  BLUE          (NOT CONNECTED)
 *  6 nc      PURPLE         (NOT CONNECTED)
 *
 *  Also tested on Serial2 and Serial3 successfully
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
#define SVM40_COMMS Serial1

/////////////////////////////////////////////////////////////
// define whether or not to update the VOC parameters in 
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

struct svm_algopar v;

void setup() {

 Serial.begin(115200);

  serialTrigger((char *) "SVM40-Example5: Get & Set Algorithm parameters. !! Press <enter> to start");

  // set driver debug level
  svm40.EnableDebugging(DEBUG);

  Serial.println(F("Trying to connect."));

  // Initialize communication port
  // FOR ESP32 SERIAL1 CHANGE THIS TO : E.G. RX-PIN 25, TX-PIN 26
  // SVM40_COMMS.begin(115200, SERIAL_8N1, 25, 26);

  SVM40_COMMS.begin(115200);
  
  // Initialize SVM40 library
  if (! svm40.begin(&SVM40_COMMS))
    Errorloop((char *) "Could not set serial communication channel.");

  // check for SVM40 connection
  if (! svm40.probe()) Errorloop((char *) "could not probe / connect with SVM40.");
  else  Serial.println(F("Detected SVM40."));

  // reset SVM40 connection
  if (! svm40.reset()) Errorloop((char *) "could not reset.");
  
  // read device info
  GetDeviceInfo();
 
  // start measurement
  if (svm40.start()) Serial.println(F("Measurement started"));
  else Errorloop((char *) "Could NOT start measurement");
}

void loop() {
  serialTrigger((char *) "Hit <enter> to READ Algorithm parameters.");
  int i;
   
  if (read_algorithm() == false){
    Errorloop((char *) "failed");
  }

  serialTrigger((char *) "Hit <enter> to WRITE Algorithm parameters.");
  
  if ( write_algorithm() == false){
    Errorloop((char *) "failed");
  }
  
  Serial.println("Done\n");
}

// read algorithm
bool read_algorithm()
{
   uint8_t ret = svm40.GetVocTuningParameters(&v);
   
   if(ret != ERR_OK) {
    Serial.println(F("error during reading\n"));
    return(false);
   }
  Serial.print("voc_index_offset\t");
  Serial.println(v.voc_index_offset);
  Serial.println("\tVOC index representing typical (average) conditions. The default value is 100.");

  Serial.print("learning_time_hours\t");
  Serial.println(v.learning_time_hours);
  Serial.println("\tTime constant of long-term estimator in hours. Past events will be forgotten");
  Serial.println("\tafter about twice the learning time. The default value is 12 hours.");

  Serial.print("gating_max_duration_minutes\t");
  Serial.println(v.gating_max_duration_minutes);
  Serial.println("\tMaximum duration of gating in minutes (freeze of estimator during high VOC");
  Serial.println("\tindex signal). Zero disables the gating. The default value is 180 minutes.");

  Serial.print("std_initial\t");
  Serial.println(v.std_initial);
  Serial.println("\tInitial estimate for standard deviation. Lower value boosts events during initial");
  Serial.println("\tlearning period, but may result in larger device-to-device variations.");
  Serial.println("\tThe default value is 50.");

  return(true);
}

bool write_algorithm()
{
  return(true);
  uint8_t ret;

   // update the value you want to change
  //v.voc_index_offset=
  //v.learning_time_hours=
  //v.gating_max_duration_minutes=
  //v.std_initial=
  
  Serial.print("VOC algorithm parameters  to write: "); 
  Serial.print("voc_index_offset\t");
  Serial.println(v.voc_index_offset);

  Serial.print("learning_time_hours\t");
  Serial.println(v.learning_time_hours);

  Serial.print("gating_max_duration_minutes\t");
  Serial.println(v.gating_max_duration_minutes);
  
  Serial.print("std_initial\t");
  Serial.println(v.std_initial);
    
  ret = svm40.SetVocTuningParameters(&v);

   // if error
  if(ret != ERR_OK) {
    Serial.println(F("Error during setting offset"));
    return(false);
  }

  Serial.println("Algorithm parameters have been updated\n");
  
  if (! UpdateNvRAM) return(true);

  Serial.println("Updating non-volatile memory\n");
  
  // you can make the change permanent and store in Nov Ram
  ret = svm40.StoreNvData();
  
  if(ret != ERR_OK) {
    Serial.println(F("Error during setting to non-volatile memory"));
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
void serialTrigger(char *mess)
{
  Serial.println();

  while (!Serial.available()) {
    Serial.println(mess);
    delay(2000);
  }

  while (Serial.available())
    Serial.read();
}
