/**
 * SVM40 Library Header file
 *
 * Copyright (c) December 2020, Paul van Haastrecht
 *
 * SVM40 is a sensor from Sensirion AG.
 * All rights reserved.
 *
 * Development environment specifics:
 * Arduino IDE 1.8.13
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************************************
 * Version 1.0 / December 2020 / paulvha
 * - Initial version
 *
 * Version 2.0 / December 2020 / paulvha
 * - updates based on SVM40 interface descriptions December 2020
 * - added UART state check
 *
 * Version 2.1 / october 2023 / paulvha
 *  - fixed setVocState in I2C
 *
 *********************************************************************
 */
#ifndef SVM40_H
#define SVM40_H

#include "Arduino.h"            // Needed for Stream
#include "printf.h"             // for debug
#include "Wire.h"               // for I2c

/**
 * library version levels
 */
#define DRIVER_MAJOR 2
#define DRIVER_MINOR 0

/**
 * To EXCLUDE I2C communication, maybe for resource reasons,
 * comment out the line below.
 */
#define INCLUDE_I2C   1

/**
 * To EXCLUDE the serial communication, maybe for resource reasons
 * as your board does not have a seperate serial, comment out the line below
 */
#define INCLUDE_UART 1

/**
 * select debug serial
 */
#define SVM40_DEBUGSERIAL Serial            // default

#if defined(ARDUINO_SODAQ_AUTONOMO) || defined(ARDUINO_SODAQ_SARA) || defined(ARDUINO_SODAQ_SFF)
#define SVM40_DEBUGSERIAL_SODAQ SerialUSB
#endif

enum debug_serial {
    STANDARD = 0,                           // default
#ifdef SVM40_DEBUGSERIAL_SODAQ
    SODAQ = 1
#endif
};

// structure to return measurement values
struct svm40_values
{
    float      humidity;           // Compensated ambient humidity in % RH
    float      temperature;        // Compensated ambient temperature in degrees celsius
    uint16_t   VOC_index;          // VOC algorithm output with a scaling value

    uint16_t   raw_voc_ticks;      // Raw VOC output ticks as read from the SGP sensor
    float      raw_humidity;       // Uncompensated raw humidity in % RH as read from the SHT40
    float      raw_temperature;    // Uncompensated raw temperature in degrees celsius as read from the SHT40
    bool       Celsius;            // if true : temperatures are reported in Celsius

    float      heat_index;         // calculated heat index
    float      dew_point;          // calculated dew point
    float      absolute_hum;       // calculated absolute humidity in g/m3.
};

// VOC parameters
struct svm_algopar {

    int16_t   voc_index_offset;    // VOC index representing typical (average) conditions. The default
                                   // value is 100.

    int16_t   learning_time_hours; // Time constant of long-term estimator in hours. Past events will
                                   // be forgotten after about twice the learning time. The default
                                   // value is 12 hours.

    int16_t   gating_max_duration_minutes; // Maximum duration of gating in minutes (freeze of estimator during
                                   // high VOC index signal). Zero disables the gating. The default
                                   // value is 180 minutes.

    int16_t   std_initial;         // Initial estimate for standard deviation. Lower value boosts
                                   // events during initial learning period, but may result in larger
                                   // device-to-device variations. The default value is 50.
};


// SVM40 device version information
struct SVM40_version {
    uint8_t major;                  // Firmware level
    uint8_t minor;
    uint8_t debug;                  // bool !!! debug state of firmware
    uint8_t HW_major;
    uint8_t HW_minor;
    uint8_t SHDLC_major;
    uint8_t SHDLC_minor;
    uint8_t DRV_major;              // library major
    uint8_t DRV_minor;
};

/* internal library error codes */
#define ERR_OK          0x00
#define ERR_DATALENGTH  0X01
#define ERR_UNKNOWNCMD  0x02
#define ERR_ACCESSRIGHT 0x03
#define ERR_PARAMETER   0x04
#define ERR_OUTOFRANGE  0x28
#define ERR_CMDSTATE    0x43
#define ERR_TIMEOUT     0x50
#define ERR_PROTOCOL    0x51

// wait times (mS) after sending command to sensor
#define RX_DELAY_MS     100                 // wait between write and read
#define MAXRECVBUFLENGTH 50

// I2C / WIRE
#define SVM40_I2C_ADDRESS       0x6A            // I2C address

#define SVM40_I2C_RESET              0xD304     // .Reset SVM40
#define SVM40_I2C_START_MEASURE      0x0010     // .Starts continuous measurement in polling mode.
#define SVM40_I2C_STOP_MEASURE       0x0104     // .Stop the measurement mode and returns to the idle mode.
#define SVM40_I2C_GET_ID             0xD033     // read ID, 39 bytes including CRC
#define SVM40_I2C_GET_VERSION        0xD100     // .Get the version of the device firmware, hardware and SHDLC protocol. (12 bytes)
#define SVM40_I2C_READ_RESULTS_INT   0x03a6     // .SVM40 command to get the the new measurement results as integers. (9 bytes)
#define SVM40_I2C_READ_RESULTS_INT_R 0x03B0     // .SVM40 command to get the the new measurement results as integers with raw (18 bytes)
#define SVM40_I2C_GET_TEMP_OFFSET    0x6014     // .Gets the T-Offset for the temperature compensation of the RHT algorithm.(3)
#define SVM40_I2C_SET_TEMP_OFFSET    0x6014     // .Sets the T-Offset for the temperature compensation of the RHT algorithm.
#define SVM40_I2C_GET_VOC_STATE      0x6181     // .Gets the current VOC algorithm state.
#define SVM40_I2C_SET_VOC_STATE      0x6181     // .Sets the VOC algorithm state. This command is only available in idle mode.
#define SVM40_I2C_GET_VOC_TUNING     0x6083     // .Gets the currently set parameters for customizing the VOC algorithm
#define SVM40_I2C_SET_VOC_TUNING     0x6083     // .Gets the currently set parameters for customizing the VOC algorithm
#define SVM40_I2C_STORE_NVRAM        0X6002     // .Stores all algorithm parameters to the non-volatile memory

// #define SVM40_I2C_PRODUCT_TYPE               // ?unknown
// #define SVM40_I2C_PRODUCT_NAME               // ?unknown
// #define SVM40_I2C_SYSTEM_UPTIME 0x9304       // ?unknown


// SERIAL
#define SVM40_SHDLC_START_BASE     0X00         //.
#define SVM40_SHDLC_START_MEASURE    0X00       //+

#define SVM40_SHDLC_STOP_MEASURE     0X01       //.
#define SVM40_SHDLC_RESET            0XD3       //.
#define SVM40_SHDLC_GET_VERSION      0XD1       //. 7 BYTES
#define SVM40_SHDLC_SYSTEM_UPTIME    0X93       // 4 BYTES

#define SVM40_SHDLC_READ_BASE        0X03       //.
#define SVM40_SHDLC_READ_RESULTS_INT     0X0A   //+
#define SVM40_SHDLC_READ_RESULTS_INT_RAW 0X0B   //+

#define SVM40_SHDLC_BASELINE_ALG     0X60       //.
#define SVM40_SHDLC_GET_TEMP_OFFSET     0X01    //+
#define SVM40_SHDLC_GET_VOC_TUNING      0X08    //+
#define SVM40_SHDLC_SET_TEMP_OFFSET     0X81    //+
#define SVM40_SHDLC_SET_VOC_TUNING      0X88    //+
#define SVM40_SHDLC_STORE_NVRAM         0X80    //+

#define SVM40_SHDLC_BASELINE_STATE   0X61       //.
#define SVM40_SHDLC_GET_VOC_STATE     0X08      //+  8 X uint8_t
#define SVM40_SHDLC_SET_VOC_STATE     0X88      //+

#define SVM40_SHDLC_GET_DEVICE_INFO  0XD0       // strings
#define SVM40_SHDLC_DEVICE_PRODUCT_TYPE 0X00    //+
#define SVM40_SHDLC_DEVICE_PRODUCT_NAME 0X01    //+
#define SVM40_SHDLC_DEVICE_SERIAL       0X03    //+

#define SVM40_SHDLC_NO_BASE_VALUE   0Xff

#define SHDLC_IND   0x7e                        // header & trailer
#define TIME_OUT    5000                        // timeout to prevent deadlock read


/* state byte data (only in SERIAL)
 * SOURCE : SVM40 UART interface description December 2020
 * added version 2.0
 * b7          b6 - b0
 * 1 = error   error code
 */
#define SVM40_ERR_OK    0X0
#define SVM40_ERR_DATA  0X1
#define SVM40_ERR_UCMD  0X2
#define SVM40_ERR_PERM  0X3
#define SVM40_ERR_PAR   0X4
#define SVM40_ERR_RANGE 0X28
#define SVM40_ERR_STAT  0X43

/* needed for conversion float IEE754 */
typedef union {
    byte array[4];
    float value;
} ByteToFloat;

/* needed for conversion timing */
typedef union {
    byte array[4];
    uint32_t value;
} ByteToU32;

/**
 *  The communication can be :
 *
 *   I2C_COMMS     use I2C communication
 *   SERIAL        use serial
 *   NONE          No port defined
 */
enum svm40_comms_port {
    I2C_COMMS = 0,
    SERIAL_COMMS = 1,
    NONE = 3
};

/***************************************************************/

class SVM40
{
  public:

    SVM40(void);

    /**
    * @brief  Enable or disable the printing of sent/response HEX values.
    *
    * @param act : level of debug to set
    *  0 : no debug message
    *  1 : sending and receiving data
    *  2 : 1 +  protocol progress
    *
    * @param SelectDebugSerial : select Serial port (see top of SVM40.h)
    *  This will allow to select a different port than Serial for debug
    *  messages. As real example an SODAQ NB board is using SerialUSB.
    */
    void EnableDebugging(uint8_t act, debug_serial SelectDebugSerial = STANDARD);

    /**
     * @brief Manual assigment of the serial communication port
     * @param serialPort: serial communication port to use
     *
     * User must have preformed the serialPort.begin(115200) in the sketch.
     *
     * @return :
     *   true on success else false
     */
    bool begin(Stream *serialPort);
    bool begin(Stream &serialPort);

    /**
     * @brief Manual assigment I2C communication port
     * @param port : I2C communication channel to be used
     *
     * User must have performed the wirePort.begin() in the sketch.
     * The sensor supports I 2 C “standard-mode” with a maximum clock frequency of 100 kHz
     * @return :
     *   true on success else false
     */
    bool begin(TwoWire *wirePort);

    /**
     * @brief check if SVM40 sensors are available (read ID)
     *
     * @return :
     *   true on success else false
     */
    bool probe();

    /**
     * @brief : Perform SVM40 instructions
     * @return :
     *   true on success else false
     */
    bool reset() {return(Instruct(SVM40_SHDLC_RESET));}
    bool start() {return(Instruct(SVM40_SHDLC_START_MEASURE));}
    bool stop()  {return(Instruct(SVM40_SHDLC_STOP_MEASURE));}

    /**
     * @brief : retrieve software/hardware version information from the SVM40
     * @param : pointer to structure to store
     *
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t GetVersion(SVM40_version *v);

    /**
     * @brief : The time since the last power-on or device reset in seconds.
     * @param : pointer to store uptime
     *
     * This value is reset after each start so a call like this would
     * only make sense if running for a longer time.
     *
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t GetSystemUpTime(uint32_t *val);

    /**
     * @brief : retrieve device information from the SVM40
     * @param ser: buffer store info
     * @param len: Max data to store in buffer
     *
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t GetSerialNumber(char *ser, uint8_t len) {return(Get_Device_info( SVM40_SHDLC_DEVICE_SERIAL, ser, len));}
    uint8_t GetProductName(char *ser, uint8_t len)  {return(Get_Device_info(SVM40_SHDLC_DEVICE_PRODUCT_NAME, ser, len));}
    uint8_t GetProductType(char *ser, uint8_t len)  {return(Get_Device_info(SVM40_SHDLC_DEVICE_PRODUCT_TYPE, ser, len));}

    /**
     * @brief : read all values from the sensor and store in structure
     * @param : pointer to structure to store
     *
     * NO NEED TO CALL MORE THEN ONCE PER SECOND as data only changes 1 seconds
     *
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t GetValues(struct svm40_values *v);

    /**
     * @brief : read VOC algorithm state from the sensor and store in array
     * @param : pointer to array to store
     *
     * Gets the current VOC algorithm state. Retrieved values can be used to set
     * the VOC algorithm state to resume operation after a short interruption,
     * skipping initial learning phase. This command is only available during
     * measurement mode.
     *
     *   .. note:: This feature can only be used after at least 3 hours of
     *             continuous operation.
     *
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t GetVocState(uint8_t *p);

    /**
     * @brief : write VOC algorithm state to the sensor
     * @param : pointer to array with data to restore
     *
     * Set previously retrieved VOC algorithm state to resume operation after a
     * short interruption, skipping initial learning phase. This command is only
     * available in idle mode.
     *
     *    .. note:: This feature should not be used after interruptions of more than
     *              10 minutes.
     *
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t SetVocState(uint8_t *p);

    /**
     * @brief : Read Temperature offset
     * @param : pointer to store temperature offset value in degrees celsius
     *
     *  Temperature offset which is used for the RHT measurements.
     *  Firmware versions prior to 2.0 will return a float value (4 bytes).
     *  For firmware version >= 2.0 an int16 value (2 bytes) is returned.
     *  Float temperature values are in degrees celsius with no scaling.
     *  Integer temperature values are in degrees celsius.
     *
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t GetTemperatureOffset(int16_t *val);

    /**
     * @brief : Set Temperature offset
     * @param : Temperature offset value are in degrees celsius.
     *
     *  Temperature offset in degrees celsius. Integer temperature values are in degrees celsius
     *
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t SetTemperatureOffset(int16_t val);

    /**
     * @brief : Gets the currently tuning parameters for the VOC algorithm
     * @param : pointer to store tuning parameters
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
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t GetVocTuningParameters(struct svm_algopar *p);

    /**
     * @brief : Set tuning parameters for the VOC algorithm
     * @param : pointer to the structure containing the values
     *
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t SetVocTuningParameters(struct svm_algopar *p);

    /**
     * @brief : Stores all algorithm parameters to the non-volatile memory.
     *
     * @return :
     *  ERR_OK = ok else error
     */
    uint8_t StoreNvData();

    /**
     * @brief : Set temperature values to return in Getvalues().
     * @param act :
     *  true set to Celsius,
     *  false is Fahrenheit
     */
    void SetTempCelsius(bool act);


  private:

    /** shared variables */
    uint8_t _Receive_BUF[MAXRECVBUFLENGTH]; // buffers
    uint8_t _Send_BUF[20];
    uint8_t _Receive_BUF_Length;
    uint8_t _Send_BUF_Length;

    svm40_comms_port _Sensor_Comms;     // sensor comms port to use
    debug_serial  _SVM40_Debug_Serial;  // serial debug-port to use
    bool          _started;             // indicate the measurement has started
    bool          _SelectTemp;          // select temperature (true = celsius)
    uint8_t       _SVM40_Debug;         // program debug level
    uint8_t       _FW_major;            // firmware level
    uint8_t       _FW_minor;            // firmware level
    unsigned long _RespDelay;           // delay after sending command

    /** supporting routines */
    void     DebugPrintf(const char *pcFmt, ...);
    uint16_t byte_to_uint16(int x);
    float    byte_to_float(int x);
    void     float_to_byte(uint8_t *data, float x);
    uint16_t ConvAbsolute(float AbsoluteHumidity);
    void     calc_absolute_humidity(struct svm40_values *v);
    void     calc_HeatIndex(struct svm40_values *v);
    void     calc_dewpoint(struct svm40_values *v);
    bool     Instruct(uint8_t type);
    uint8_t  Get_Device_info(uint8_t type, char *ser, uint8_t len);

#if defined INCLUDE_UART

    // calls
    uint8_t SHDLC_ReadFromSerial();
    uint8_t SHDLC_SerialToBuffer();
    uint8_t SHDLC_SendToSerial();
    bool    SHDLC_fill_buffer(uint8_t lead, uint8_t command, uint8_t len = 0, uint8_t *par = NULL);
    uint8_t SHDLC_calc_CRC(uint8_t * buf, uint8_t first, uint8_t last);
    int     SHDLC_ByteStuff(uint8_t b, int off);
    uint8_t SHDLC_ByteUnStuff(uint8_t b);
    void    SHDLC_State(uint8_t state);

    // variables
    Stream *_serial;            // serial port to use

#endif // INCLUDE_UART

#if defined INCLUDE_I2C

    // calls
    void    I2C_fill_buffer(uint16_t cmd, uint8_t len = 0,  uint8_t *param = NULL);
    uint8_t I2C_RequestFromSVM(uint8_t count, bool chk_zero = false);
    uint8_t I2C_ReadFromSVM(uint8_t cnt, bool chk_zero);
    uint8_t I2C_SendToSVM();
    uint8_t I2C_calc_CRC(uint8_t data[2]);

    // variables
    TwoWire *_i2cPort;      // holds the I2C port
#endif // INCLUDE_I2C
};

#endif /* SVM40_H */
