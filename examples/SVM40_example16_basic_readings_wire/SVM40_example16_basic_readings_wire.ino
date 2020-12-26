/*
 *  Version 1.0.1 / December 2020 / paulvha
 *  
 *   Example shows how to read basic values from the SVM40 as Plotter
 *   
 *   after compile and upload
 *   
 *   CNTRL + SHIFT + L to see the plotter
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

#include "svm40.h"

// create constructor
SVM40 svm40;

void setup() {

  Serial.begin(115200);

  SVM40_COMMS.begin();
  
  // Initialize SVM40 library
  if (! svm40.begin(&SVM40_COMMS))
    Errorloop((char *) "Could not set Wire communication channel.");

  // check for SVM40 connection
  if (! svm40.probe()) Errorloop((char *) "could not probe / connect with SVM40.");

  // reset SVM40 connection
  if (! svm40.reset()) Errorloop((char *) "could not reset.");

  // set Temperature 
  svm40.SetTempCelsius(SetTemp);
   
  // start measurement
  if (! svm40.start()) Errorloop((char *) "Could NOT start measurement");
}

void loop() {
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
          Serial.println(F("Error during reading values"));
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


  Serial.print(v.VOC_index);
  Serial.print(F(","));
  Serial.print(v.raw_voc_ticks/1000);   // to fit the plotter area
  Serial.print(F(","));
  Serial.print(v.humidity);
  Serial.print(F(","));
  Serial.println(v.temperature);
  return(true);
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
