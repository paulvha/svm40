/**
 * SVM40 Library file
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
 *********************************************************************
 */

#include "svm40.h"

#if !defined INCLUDE_I2C && !defined INCLUDE_UART
#error you must enable either I2C or UART communication
#endif

/**
 * @brief constructor and initialize variables
 */
SVM40::SVM40(void) {
  _Send_BUF_Length = 0;
  _Receive_BUF_Length = 0;
  _SVM40_Debug = false;
  _SelectTemp = true;          // default to celsius
  _FW_major = 0;               // Firmware level unknown
}

/**
 * @brief Print debug message if enabled
 *
 */

static char prfbuf[256];
void SVM40::DebugPrintf(const char *pcFmt, ...) {
    va_list pArgs;

    if (_SVM40_Debug){

        va_start(pArgs, pcFmt);
        vsprintf(prfbuf, pcFmt, pArgs);
        va_end(pArgs);

        if (_SVM40_Debug_Serial == STANDARD)
            SVM40_DEBUGSERIAL.print(prfbuf);

#ifdef SVM40_DEBUGSERIAL_SODAQ
        else if (_SVM40_Debug_Serial == SODAQ)
            SVM40_DEBUGSERIAL_SODAQ.print(prfbuf);
#endif
    }
}

/**
 * @brief Manual assigment of the serial communication port
 *
 * @param serialPort: serial communication port to use
 *
 * User must have preformed the serialPort.begin(115200) in the sketch.
 */
bool SVM40::begin(Stream &serialPort) {
#if defined INCLUDE_UART
    _Sensor_Comms = SERIAL_COMMS;
    _serial  = &serialPort; // Grab which port the user wants us to use
    return true;
#else
    DebugPrintf("UART communication not enabled\n");
    return(false);
#endif // INCLUDE_UART
}

/**
 * @brief Manual assigment of the serial communication port
 *
 * @param serialPort: serial communication port to use
 *
 * User must have preform the serialPort.begin(115200) in the sketch.
 */
bool SVM40::begin(Stream *serialPort) {
#if defined INCLUDE_UART
    _Sensor_Comms = SERIAL_COMMS;
    _serial  = serialPort; // Grab which port the user wants us to use
    return true;
#else
    DebugPrintf("UART communication not enabled\n");
    return(false);
#endif // INCLUDE_UART
}

/**
 * @brief Manual assigment I2C communication port
 *
 * @param port : I2C communication channel to be used
 * The sensor supports I2C “standard-mode” with a maximum clock frequency of 100 kHz
 * User must have preform the wirePort.begin() in the sketch.
 */
bool SVM40::begin(TwoWire *wirePort) {
#if defined INCLUDE_I2C
    _Sensor_Comms = I2C_COMMS;
    _i2cPort = wirePort;            // Grab which port the user wants us to use
    _i2cPort->setClock(100000);     // Apollo3 is default 400K (although stated differently in 2.0.1)
    return true;
#else
    DebugPrintf("I2C communication not enabled\n");
    return(false);
#endif // INCLUDE_I2C
}

/**
 * @brief check if SVM40 sensor is available (read version)
 *
 * Return:
 *   true on success else false
 */
bool SVM40::probe() {

    SVM40_version v;

    if (GetVersion(&v) == ERR_OK) return(true);

    return(false);
}

/**
 * @brief  Enable or disable the printing of sent/response HEX values.
 *
 * @param act : level of debug to set
 *  0 : no debug message
 *  1 : sending and receiving data
 *  2 : 1 +  protocol progress
 *
 * @param SelectDebugSerial:
 *  STANDARD to Serial (default)
 *  SODAQ to SerialUSB
 */
void SVM40::EnableDebugging(uint8_t act, debug_serial SelectDebugSerial) {
    _SVM40_Debug = act;
    _SVM40_Debug_Serial = SelectDebugSerial;
}

/**
 * @brief Read version info
 * @param : pointer to structure to store
 *
 * @return :
 *  ERR_OK = ok else error
 */
uint8_t SVM40::GetVersion(SVM40_version *v) {
    uint8_t ret, offset;
    memset(v, 0x0, sizeof(struct SVM40_version));

#if defined INCLUDE_I2C

    if (_Sensor_Comms == I2C_COMMS) {

        I2C_fill_buffer(SVM40_I2C_GET_VERSION);

        ret = I2C_RequestFromSVM(8,false);

        offset = 0;
    }
    else
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {
        // fill buffer to send
        if ( ! SHDLC_fill_buffer(SVM40_SHDLC_NO_BASE_VALUE,SVM40_SHDLC_GET_VERSION) ) return(ERR_PARAMETER);

        ret = SHDLC_ReadFromSerial();

        offset = 5;
    }
#else
    {}
#endif // INCLUDE_UART

    v->major = _Receive_BUF[offset + 0];
    v->minor = _Receive_BUF[offset + 1];
    v->debug = _Receive_BUF[offset + 2];
    v->HW_major = _Receive_BUF[offset + 3];
    v->HW_minor = _Receive_BUF[offset + 4];
    v->SHDLC_major = _Receive_BUF[offset + 5];
    v->SHDLC_minor = _Receive_BUF[offset + 6];
    v->DRV_major = DRIVER_MAJOR;
    v->DRV_minor = DRIVER_MINOR;

    // needed in Temperature Offset
    _FW_major = v->major;
    _FW_minor = v->minor;

    return(ret);
}

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
uint8_t SVM40::GetSystemUpTime(uint32_t *val) {
    uint8_t ret;
    uint8_t offset;

#if defined INCLUDE_I2C
    if (_Sensor_Comms == I2C_COMMS) {

       // opcode for I2C is not known (yet)
       *val = 0;
       return(ERR_OK);

       //I2C_fill_buffer(SVM40_I2C_SYSTEM_UPTIME);
       //ret = I2C_RequestFromSVM(4);

       offset = 0;
    }
    else
#endif // INCLUDE_I2C
#if defined INCLUDE_UART
    {
        offset = 5;

        // fill buffer to send
        if (SHDLC_fill_buffer(SVM40_SHDLC_NO_BASE_VALUE, SVM40_SHDLC_SYSTEM_UPTIME) != true) return(ERR_PARAMETER);

        ret = SHDLC_ReadFromSerial();

        if (ret != ERR_OK) return (ret);

        /// buffer : hdr addr cmd state length data....data crc hdr
        ///           0    1   2    3     4     5
        // check length
        if (_Receive_BUF[4] != 0x4){
            DebugPrintf("%d Not enough bytes for all values\n", _Receive_BUF[4]);
            return(ERR_DATALENGTH);
        }
    }

#else
    {}
#endif // INCLUDE_UART

    *val = (((uint32_t)_Receive_BUF[offset] << 24) | ((uint32_t)_Receive_BUF[offset+1] << 16) | \
    ((uint32_t)_Receive_BUF[offset+2] << 8) | ((uint32_t)_Receive_BUF[offset+3] << 0));

    return(ERR_OK);
}

/**
 * @brief : read VOC algorithm state from the sensor and store in array
 * @param : pointer to array to store ( 8 bytes !!)
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
uint8_t SVM40::GetVocState(uint8_t *p) {
    uint8_t ret, offset, i;

#if defined INCLUDE_I2C
    if (_Sensor_Comms == I2C_COMMS) {

        offset = 0;

        I2C_fill_buffer(SVM40_I2C_GET_VOC_STATE);
        ret = I2C_RequestFromSVM(8);
        if (ret != ERR_OK) return (ret);
    }
    else
#endif // INCLUDE_I2C
#if defined INCLUDE_UART
    {
        offset = 5;

        // fill buffer to send
        if (SHDLC_fill_buffer(SVM40_SHDLC_BASELINE_STATE, SVM40_SHDLC_GET_VOC_STATE) != true) return(ERR_PARAMETER);

        ret = SHDLC_ReadFromSerial();

        if (ret != ERR_OK) return (ret);

        /// buffer : hdr addr cmd state length data....data crc hdr
        ///           0    1   2    3     4     5
        // check length
        if (_Receive_BUF[4] != 8){
            DebugPrintf("%d Not enough bytes for all values\n", _Receive_BUF[4]);
            return(ERR_DATALENGTH);
        }
    }
#else
    {}
#endif // INCLUDE_UART

    for(i = 0; i < 8; i++)  p[i] = _Receive_BUF[offset +i];

    return(ERR_OK);
}

/**
 * @brief : Gets the currently parameters of the VOC algorithm
 * @param : pointer to store the parameters
 *
 * - voc_index_offset (int) -
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
uint8_t SVM40::GetVocTuningParameters(struct svm_algopar *p) {
    uint8_t ret, offset, i;

    // measurement started already?
    if ( !_started ) {
        if ( ! start() ) return(ERR_CMDSTATE);
    }

#if defined INCLUDE_I2C
    if (_Sensor_Comms == I2C_COMMS) {

        offset = 0;

        I2C_fill_buffer(SVM40_I2C_GET_VOC_TUNING);
        ret = I2C_RequestFromSVM(8);
        if (ret != ERR_OK) return (ret);
    }
    else
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {
        offset = 5;

        // fill buffer to send

        if (SHDLC_fill_buffer(SVM40_SHDLC_BASELINE_ALG, SVM40_SHDLC_GET_VOC_TUNING) != true) return(ERR_PARAMETER);
        ret = SHDLC_ReadFromSerial();

        if (ret != ERR_OK) return (ret);

        /// buffer : hdr addr cmd state length data....data crc hdr
        ///           0    1   2    3     4     5
        // check length
        if (_Receive_BUF[4] != 0x8){
            DebugPrintf("%d Not enough bytes for all values\n", _Receive_BUF[4]);
            return(ERR_DATALENGTH);
        }
    }
#else
    {}
#endif // INCLUDE_UART

    p->voc_index_offset = byte_to_uint16(offset);
    p->learning_time_hours = byte_to_uint16(offset+2);
    p->gating_max_duration_minutes = byte_to_uint16(offset+4);
    p->std_initial = byte_to_uint16(offset+6);

    return(ERR_OK);
}

/**
 * @brief : Set VOC Tuning parameter
 * @param : pointer to the structure containing the values
 *
 * @return
 *  ERR_OK = ok else error
 */
uint8_t SVM40::SetVocTuningParameters(struct svm_algopar *p) {
    uint8_t ret;
    bool restart = _started;
    uint8_t data[8];

    // measurement started already?
    if ( _started ) {
        if ( ! stop() ) return(ERR_CMDSTATE);
    }

    data[0] = p->voc_index_offset >> 8;
    data[1] = p->voc_index_offset & 0xff;
    data[2] = p->learning_time_hours >> 8;
    data[3] = p->learning_time_hours & 0xff;
    data[4] = p->gating_max_duration_minutes >> 8;
    data[5] = p->gating_max_duration_minutes & 0xff;
    data[6] = p->std_initial >> 8;
    data[7] = p->std_initial & 0xff;

#if defined INCLUDE_I2C
    if (_Sensor_Comms == I2C_COMMS) {

        I2C_fill_buffer(SVM40_I2C_SET_VOC_TUNING, 8, data);
        ret = I2C_SendToSVM();
    }
    else
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {
        // fill buffer to send
        if (SHDLC_fill_buffer(SVM40_SHDLC_BASELINE_ALG, SVM40_SHDLC_SET_VOC_TUNING, 8, data) != true) return(ERR_PARAMETER);

        SHDLC_SendToSerial();

        ret = SHDLC_ReadFromSerial();
    }
#else
    {}
#endif // INCLUDE_UART

    // measurement restart ?
    if ( restart ) {
        if ( ! start() ) return(ERR_CMDSTATE);
    }

    return(ret);
}

/**
 * @brief : Read Temperature offset
 * @param : pointer to store temperature offset value in degrees celsius
 *
 *  Temperature offset which is used for the RHT measurements.
 *  Firmware versions prior to 2.0 will return a float value (4 bytes).
 *  For firmware version >= 2.0 an int16 value (2 bytes) is returned.
 *  Float temperature values are in degrees celsius with no scaling.
 *  Integer temperature values are in degrees celsius with a scaling of 200.
 *
 * @return :
 *  ERR_OK = ok else error
 */
uint8_t SVM40::GetTemperatureOffset(int16_t *val) {
    uint8_t ret, offset,len;

    if (_FW_major == 0) {
        if (!probe()) return(ERR_PARAMETER);
    }

    // Firmware level 1 is sending float (4 bytes)
    if (_FW_major == 1) len = 4;
    else len = 2;

#if defined INCLUDE_I2C

    if (_Sensor_Comms == I2C_COMMS) {

        offset = 0;

        I2C_fill_buffer(SVM40_I2C_GET_TEMP_OFFSET);
        ret = I2C_RequestFromSVM(len);
    }
    else
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {
        offset = 5;

        // fill buffer to send
        if (SHDLC_fill_buffer(SVM40_SHDLC_BASELINE_ALG, SVM40_SHDLC_GET_TEMP_OFFSET) != true) return(ERR_PARAMETER);

        ret = SHDLC_ReadFromSerial();

        if (ret != ERR_OK) return (ret);

        /// buffer : hdr addr cmd state length data....data crc hdr
        ///           0    1   2    3     4     5
    }
#else
    {}
#endif // INCLUDE_UART

    // did we get a float (FW version 1.x)
    if (len == 4 ){
        float a = byte_to_float(offset);
        Serial.println(a);
        *val = (uint16_t) a;
    }
    else {    //(FW version > 1.x)
        *val = byte_to_uint16(offset) / 200;
    }

    return(ERR_OK);
}

/**
 * @brief : set Temperature offset
 * @param : temperature offset value in degrees celsius.
 *
 *  Temperature offset in degrees celsius. Accepted data formats are either a float
 *  value (4 bytes) or an int16 value (2 bytes). Float temperature values are in degrees
 *  celsius with no scaling. Integer temperature values are in degrees celsius with a scaling of 200.
 *
 * @return
 *  ERR_OK = ok
 *  else error
 */
uint8_t SVM40::SetTemperatureOffset(int16_t val) {
    uint8_t len, ret;
    uint8_t data[4];
    uint16_t v;
    bool restart = _started;

    //  can only be done in idle mode
    if ( _started ) {
        if ( ! stop() ) return(ERR_CMDSTATE);
    }

    if (_FW_major == 0) {
        if (!probe()) return(ERR_PARAMETER);
    }

    // Firmware level 1 is expecting float (4 bytes)
    if (_FW_major == 1){
        float a = (float) val;
        float_to_byte(data, a);
        len = 4;
    }
    else {
        v = val * 200;       // scaling 200
        data[0] = v >> 8;    // msb first
        data[1] = v & 0xff;
        len = 2;
    }

#if defined INCLUDE_I2C
    if (_Sensor_Comms == I2C_COMMS) {

        I2C_fill_buffer(SVM40_I2C_SET_TEMP_OFFSET, len, data);
        ret = I2C_SendToSVM();
    }
    else
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {
        // fill buffer to send
        if (SHDLC_fill_buffer(SVM40_SHDLC_BASELINE_ALG, SVM40_SHDLC_SET_TEMP_OFFSET, len, data) != true) return(ERR_PARAMETER);

        SHDLC_SendToSerial();

        // check response
        ret = SHDLC_ReadFromSerial();
    }
#else
    {}
#endif // INCLUDE_UART

    // measurement restart ?
    if ( restart ) {
        if ( ! start() ) return(ERR_CMDSTATE);
    }

    return(ret);
}

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
 * @return
 *  ERR_OK = ok else error
 */
uint8_t SVM40::SetVocState(uint8_t *p) {
    uint8_t ret;
    bool restart = _started;

    //  can only be done in idle mode
    if ( _started ) {
        if ( ! stop() ) return(ERR_CMDSTATE);
    }

#if defined INCLUDE_I2C
    if (_Sensor_Comms == I2C_COMMS) {

        I2C_fill_buffer(SVM40_I2C_SET_VOC_STATE, p , 8);
        ret = I2C_SendToSVM();
    }
    else
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {
        // fill buffer to send
        if (SHDLC_fill_buffer(SVM40_SHDLC_BASELINE_STATE, SVM40_SHDLC_GET_VOC_STATE, 8, p) != true) return(ERR_PARAMETER);
        ret = SHDLC_SendToSerial();
    }
#else
    {}
#endif // INCLUDE_UART

    // measurement restart ?
    if ( restart ) {
        if ( ! start() ) return(ERR_CMDSTATE);
    }

    return(ret);
}

/**
 * @brief : read all values from the sensor and store in structure
 * @param : pointer to structure to store
 *
 * NO NEED TO CALL MORE THEN ONCE PER SECOND
 *
 * @return :
 *  ERR_OK = ok else error
 */
uint8_t SVM40::GetValues(struct svm40_values *v) {
    uint8_t ret;
    uint8_t offset;

    // measurement started already?
    if ( !_started ) {
        if ( ! start() ) return(ERR_CMDSTATE);
    }

#if defined INCLUDE_I2C
    if (_Sensor_Comms == I2C_COMMS) {

        offset = 0;

        I2C_fill_buffer(SVM40_I2C_READ_RESULTS_INT_R);

        ret = I2C_RequestFromSVM(12);
    }
    else
#endif // INCLUDE_I2C
#if defined INCLUDE_UART
    {
        offset = 5;

        // fill buffer to send
        if (SHDLC_fill_buffer(SVM40_SHDLC_READ_BASE, SVM40_SHDLC_READ_RESULTS_INT_RAW) != true) return(ERR_PARAMETER);

        ret = SHDLC_ReadFromSerial();

        if (ret != ERR_OK) return (ret);

        /// buffer : hdr addr cmd state length data....data crc hdr
        ///           0    1   2    3     4     5
        // check length
        if (_Receive_BUF[4] != 0xC){
            DebugPrintf("%d Not enough bytes for all values\n", _Receive_BUF[4]);
            return(ERR_DATALENGTH);
        }
    }
#else
    {}
#endif // INCLUDE_UART

    memset(v,0x0,sizeof(struct svm40_values));

    // get data
    v->VOC_index = byte_to_uint16(offset) / 10;
    v->humidity = ((float) byte_to_uint16(offset+2)) / 100;
    v->temperature = ((float) byte_to_uint16(offset+4)) / 200;
    v->raw_voc_ticks = byte_to_uint16(offset+6);
    v->raw_humidity = ((float) byte_to_uint16(offset+8)) / 100;
    v->raw_temperature = ((float) byte_to_uint16(offset+10)) / 200;
    v->Celsius = _SelectTemp;

    // perform some calculations
    calc_HeatIndex(v);
    calc_absolute_humidity(v);
    calc_dewpoint(v);

    // report temperatures in Fahrenheit if requested
    if (! _SelectTemp) {
        v->temperature = (v->temperature * 1.8) + 32;
        v->raw_temperature = (v->raw_temperature * 1.8) + 32;
        v->heat_index = (v->heat_index * 1.8) + 32;
        v->dew_point =(v->dew_point * 1.8) + 32;
    }

    return(ERR_OK);
}

/**
 * @brief : Set temperature to obtain
 * @param act : true is Celsius, false is Fahrenheit
 *
 */
void SVM40::SetTempCelsius(bool act) {
    _SelectTemp = act;
}

/**
 * @brief : Stores all algorithm parameters to the non-volatile memory.
 *
 * @return
 *  ERR_OK = ok else error
 */
uint8_t SVM40::StoreNvData() {
    uint8_t ret;

#if defined INCLUDE_I2C
    if (_Sensor_Comms == I2C_COMMS) {

        I2C_fill_buffer(SVM40_I2C_STORE_NVRAM);
        ret = I2C_SendToSVM();
    }
    else
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {
        // fill buffer to send
        if (SHDLC_fill_buffer(SVM40_SHDLC_BASELINE_ALG, SVM40_SHDLC_STORE_NVRAM) != true) return(ERR_PARAMETER);

        SHDLC_SendToSerial();

        ret = SHDLC_ReadFromSerial();
    }
#else
    {}
#endif // INCLUDE_UART

    return(ret);
}

/**
 * @brief : Instruct SVM40
 * @param type : type of instruction
 *
 * @return :
 *   true on success else false
 */
bool SVM40::Instruct(uint8_t type){

    uint8_t ret;

    if(type == SVM40_SHDLC_STOP_MEASURE && !_started) return(true);

#if defined INCLUDE_I2C

    if (_Sensor_Comms == I2C_COMMS) {

        if (type == SVM40_SHDLC_START_MEASURE)
            I2C_fill_buffer(SVM40_I2C_START_MEASURE);
        else if(type == SVM40_SHDLC_STOP_MEASURE)
            I2C_fill_buffer(SVM40_I2C_STOP_MEASURE);
        else if(type == SVM40_SHDLC_RESET)
            I2C_fill_buffer(SVM40_I2C_RESET);
        else
            return(false);

        ret = I2C_SendToSVM();
    }
    else // if serial communication
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {    // fill buffer to send
        if (type == SVM40_SHDLC_START_MEASURE){
            if (SHDLC_fill_buffer(SVM40_SHDLC_START_BASE,type) != true) return(false);
        }
        else {
            if (SHDLC_fill_buffer(SVM40_SHDLC_NO_BASE_VALUE,type) != true) return(false);
        }
        ret = SHDLC_ReadFromSerial();
    }
#else
    {}
#endif // INCLUDE_UART

    if (ret == ERR_OK){

        if (type == SVM40_SHDLC_START_MEASURE) {
            _started = true;
            delay(1000);
        }
        else if (type == SVM40_SHDLC_STOP_MEASURE)
            _started = false;

        else if (type == SVM40_SHDLC_RESET){
            _started = false;

#if defined INCLUDE_I2C
            if (_Sensor_Comms == I2C_COMMS) {
                _i2cPort->begin();       // some I2C channels need a reset
            }
#endif
            delay(2000);
        }

        return(true);
    }

    DebugPrintf("instruction failed\n");
    return(false);
}

/**
 * @brief General Read device info
 *
 * @param type:
 *  Product Name  : SVM40_SHDLC_DEVICE_PRODUCT_NAME
 *  Product Type  : SVM40_SHDLC_DEVICE_PRODUCT_TYPE
 *  Serial Number : SVM40_SHDLC_DEVICE_SERIAL
 *
 * @param ser     : buffer to hold the read result
 * @param len     : length of the buffer
 *
 * @return :
 *  ERR_OK = ok else error
 */
uint8_t SVM40::Get_Device_info(uint8_t type, char *ser, uint8_t len) {
    uint8_t ret,i, offset;

#if defined INCLUDE_I2C

    if (_Sensor_Comms == I2C_COMMS) {

        // Serial or article code
        if (type == SVM40_SHDLC_DEVICE_SERIAL) {
            I2C_fill_buffer(SVM40_I2C_GET_ID);

            // true = check zero termination
            ret =  I2C_RequestFromSVM(24,true);
        }

        else {
            sprintf(ser,"Not Supported");
            return(ERR_OK);
        }

        offset = 0;
    }
    else
#endif // INCLUDE_I2C

#if defined INCLUDE_UART
    {
        // fill buffer to send
        if ( ! SHDLC_fill_buffer(SVM40_SHDLC_GET_DEVICE_INFO,type) ) return(ERR_PARAMETER);

        ret = SHDLC_ReadFromSerial();

        offset = 5;
    }
#else
    {}
#endif // INCLUDE_UART

    if (ret != ERR_OK) return(ret);

    // get data
    for (i = 0; i < len ; i++) {
        ser[i] = _Receive_BUF[i+offset];
        if (ser[i] == 0x0) break;
    }

    return(ret);
}

////////////////////////////////////////////////////////////////
// CALCULATIONS FOR SVM40                                     //
////////////////////////////////////////////////////////////////
/**
 * @brief : calculate the absolute humidity from relative humidity [%RH *1000]
 * and temperature [mC]
 */
void SVM40::calc_absolute_humidity(struct svm40_values *v) {

    float Temp = v->temperature;
    float Hum =  v->humidity;

    if (Hum == 0) return;

    v->absolute_hum = (6.112 * pow(2.71828,((17.67 * Temp)/(Temp + 243.5))) * Hum * 2.1674) / (273.15 + Temp);
}

/**
 * Convert relative humidity [%RH] and temperature [C] to
 * absolute humidity [g/m^3] that can be used as input for
 * the SetHumidity() call.
 *
 * Absolute humidity refers to the amount of water contained in a parcel
 * of air and is commonly measured in grams of water per kg of dry air.
 *
 * source: datasheet
 * The 2 data bytes represent humidity values as a fixed-point 8.8bit
 * number with a minimum value of 0x0001 (=1/256 g/m 3 ) and a maximum
 * value of 0xFFFF (255 g/m 3 + 255/256 g/m 3 ).
 * For instance, sending a value of 0x0F80 corresponds to a humidity
 * value of 15.50 g/m3 (15 g/m3 + 128/256 g/m 3 ).
 */
uint16_t SVM40::ConvAbsolute(float AbsoluteHumidity) {
    uint16_t val;
    int val1;

    // get top 8 bits (MSB)
    val = (uint16_t) AbsoluteHumidity;

    // get bottom 8 bits (LSB)
    val1 = (AbsoluteHumidity - (float) val) * 100;

    val = val << 8 | val1;
    // printf("val 0x%x, val1 %d / 0x%x\n", val,val1, val1);

    return(val);
}

/**
 * @brief calculate Heat Index
 *
 * The heat index, also known as the apparent temperature, is what the
 * temperature feels like to the human body when relative humidity is
 * combined with the air temperature. This has important considerations
 * for the human body's comfort. When the body gets too hot,
 * it begins to perspire or sweat to cool itself off.
 *
 * Using both Rothfusz and Steadman's equations
 *  http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
 */
void SVM40::calc_HeatIndex(struct svm40_values *v) {

  float hi, temperature;

  /* Celsius turn to Fahrenheit */
  temperature = (v->temperature * 1.8) + 32;

  float percentHumidity = v->humidity;

  /* calculate */
  hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

  if (hi > 79) {
    hi = -42.379 +
             2.04901523 * temperature +
            10.14333127 * percentHumidity +
            -0.22475541 * temperature*percentHumidity +
            -0.00683783 * pow(temperature, 2) +
            -0.05481717 * pow(percentHumidity, 2) +
             0.00122874 * pow(temperature, 2) * percentHumidity +
             0.00085282 * temperature*pow(percentHumidity, 2) +
            -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

    if((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
      hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

    else if((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
      hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
  }

  /* convert Fahrenheit to Celsius */
  v->heat_index = (hi - 32) * 0.55555;
}

/**
 * @brief calculate dew point
 *
 *
 * The dew point is the temperature the air needs to be cooled to (at
 * constant pressure) in order to achieve a relative humidity (RH) of 100%.
 * At this point the air cannot hold more water in the gas form
 *
 * using the Augst-Roche-Magnus Approximation.
 *
 */
void SVM40::calc_dewpoint(struct svm40_values *v) {

    float H;

    float temp = v->temperature;
    float hum =  v->humidity;

    /* calculate */
    H = log(hum/100) + ((17.625 * temp) / (243.12 + temp));
    v->dew_point = 243.04 * H / (17.625 - H);
}

/////////////////////////////////////////////////////////////////
//  ROUTINES TO COMMUNICATE WITH SVM40                         //
/////////////////////////////////////////////////////////////////

/**
 * @brief : translate 2 bytes to uint16
 * @param x : offset in _Receive_BUF
 *
 * assumed is MSB first, LSB second byte
 *
 * @return : uint16_t number
 */
uint16_t SVM40::byte_to_uint16(int x) {
    uint16_t val;
    val =  _Receive_BUF[x] << 8 | _Receive_BUF[x+1];
    return val;
}

/**
 * @brief : translate 4 bytes to float IEEE754
 * @param x : offset in _Receive_BUF
 *
 * return : float number
 */
float SVM40::byte_to_float(int x) {
    ByteToFloat conv;

    for (byte i = 0; i < 4; i++){
      conv.array[3-i] = _Receive_BUF[x+i]; //or conv.array[i] = _Receive_BUF[x+i]; depending on endianness
    }

    return conv.value;
}

/**
 * @brief : translate float to 4 bytes IEEE754
 * @param data : place to store 4 bytes
 * @param x : float value to parse
 */
void SVM40::float_to_byte(uint8_t *data, float x) {
    ByteToFloat conv;

    conv.value = x;

    for (byte i = 0; i < 4; i++){
      data[i] = conv.array[3-i]; //or data[i] = conv.array[i]; depending on endianness
    }
}

/*******************************************************************
 *  UART ROUTINES
 *******************************************************************/
#if defined INCLUDE_UART

/**
 * @brief check and perform byte stuffing
 * @param b   : byte to send
 * @param off : current pointer in _Send_BUF
 *
 * Will store the value in _Send_BUF
 * @return : the new offset position
 */
int SVM40::SHDLC_ByteStuff(uint8_t b, int off) {
    uint8_t  x = 0;

    switch(b){
        case 0x11: {x = 0x31; break;}
        case 0x13: {x = 0x33; break;}
        case 0x7d: {x = 0x5d; break;}
        case 0x7e: {x = 0x5e; break;}
    }

    if (x == 0) _Send_BUF[off++] = b;
    else
    {
        _Send_BUF[off++] = 0x7D;
        _Send_BUF[off++] = x;
    }

    return(off);
}

/**
 * @brief : unstuff bytes
 * @param : received Stuff byte
 *
 * @return
 *  byte to use or zero if error
 */
uint8_t SVM40::SHDLC_ByteUnStuff(uint8_t b) {
    switch(b){
        case 0x31: return(0x11);
        case 0x33: return(0x13);
        case 0x5d: return(0x7d);
        case 0x5e: return(0x7e);

        default:
            if ( _SVM40_Debug > 1)
                DebugPrintf("Incorrect byte Unstuffing. Got: 0x%02X\n",b);
            return(0);
    }
}

/**
 * @brief : create the SHDLC buffer to send

 * @param lead: some commands are a combination of base + extension
 * @param par: data to add for some commands
 * @param len: length of data to sent
 *
 * @return:
 *  true OK
 *  false ERROR
 */
bool SVM40::SHDLC_fill_buffer(uint8_t lead, uint8_t command, uint8_t len, uint8_t *par) {
    memset(_Send_BUF,0x0,sizeof(_Send_BUF));
    _Send_BUF_Length = 0;

    int i = 0;
    uint8_t tmp;

    _Send_BUF[i++] = SHDLC_IND;
    _Send_BUF[i++] = 0x0;               // SHDLC address SVM40 is zero

    if(lead != SVM40_SHDLC_NO_BASE_VALUE){

        _Send_BUF[i++] = lead;
        _Send_BUF[i++] = 1;     // length
        _Send_BUF[i++] = command & 0xff;

        // check for additional data to be added
        if (lead == SVM40_SHDLC_BASELINE_STATE && command == SVM40_SHDLC_SET_VOC_STATE){
            for(tmp=0; tmp < len ; tmp++) _Send_BUF[i++] = par[tmp];
            _Send_BUF[3] = len + 1;
        }

        else if (lead == SVM40_SHDLC_BASELINE_ALG && command == SVM40_SHDLC_SET_TEMP_OFFSET) {
            for(tmp=0; tmp < len ; tmp++) _Send_BUF[i++] = par[tmp];
            _Send_BUF[3] = len + 1;
        }
    }
    else {
        _Send_BUF[i++] = command;
        _Send_BUF[i++] = 0;     // length
    }

    // add CRC and check for byte stuffing
    tmp = SHDLC_calc_CRC(_Send_BUF, 1, i);
    i = SHDLC_ByteStuff(tmp, i);

    _Send_BUF[i] = SHDLC_IND;
    _Send_BUF_Length = ++i;

    // set for delay  (add V2)
    // time is set wider than datasheet to be sure
    _RespDelay = RX_DELAY_MS;

    switch(command) {
        case SVM40_SHDLC_STORE_NVRAM:
            _RespDelay = 750;
            break;

        case SVM40_SHDLC_RESET:
            _RespDelay = 200;
            break;
    }

    return(true);
}

/**
 * @brief calculate SHDLC CRC
 * @param buf   : buffer to calculated
 * @param first : first data byte to include
 * @param last  : last data byte to include
 *
 * @return : calculated CRC
 */
uint8_t SVM40::SHDLC_calc_CRC(uint8_t *buf, uint8_t first, uint8_t last) {
    uint8_t i;
    uint32_t ret = 0;

    for (i = first; i <= last ; i++)   ret += buf[i];
    return(~(ret & 0xff));
}

/**
 * @brief send a filled buffer to the SVM40 over serial
 *
 * @return
 *   Err_OK is OK  else error
 */
uint8_t SVM40::SHDLC_SendToSerial() {
    uint8_t i;

    if (_Send_BUF_Length == 0) return(ERR_DATALENGTH);

    if (_SVM40_Debug){
        DebugPrintf("Sending: ");
        for(i = 0; i < _Send_BUF_Length; i++)
            DebugPrintf(" 0x%02X", _Send_BUF[i]);
        DebugPrintf("\n");
    }

    for (i = 0 ; i <_Send_BUF_Length; i++)
        _serial->write(_Send_BUF[i]);

    // indicate that command has been sent
    _Send_BUF_Length = 0;

    // wait
    delay(_RespDelay);

    return(ERR_OK);
}

/**
 * @brief send command, read response & check for errors
 *
 * @return :
 *  Ok = ERR_OK else Error code
 */
uint8_t SVM40::SHDLC_ReadFromSerial() {
    uint8_t ret;
    _serial->flush();

    // write to serial
    // Neglect if there is nothing to send first. This
    // could also be a read status from earlier command
    SHDLC_SendToSerial();

    // read serial
    ret = SHDLC_SerialToBuffer();
    if (ret != ERR_OK) return(ret);

    /**
     * check CRC.
     * CRC MIGHT have been byte stuffed as well but that is handled
     * in SerialtoBuffer !
     * buffer : hdr addr cmd state length data....data crc hdr
     *           0    1   2    3     4     5       -2   -1  -0
     */

    ret = SHDLC_calc_CRC(_Receive_BUF, 1,_Receive_BUF_Length-2);
    if (_Receive_BUF[_Receive_BUF_Length-1] != ret)
    {
        DebugPrintf("CRC error. expected 0x%02X, got 0x%02X\n",_Receive_BUF[_Receive_BUF_Length-1], ret);
        return(ERR_PROTOCOL);
    }

    // check status
    SHDLC_State(_Receive_BUF[3]);

    return(_Receive_BUF[3]);
}

/**
 * @brief  Check status and display error message
 * @param  err : state byte from device
 *
 */
void SVM40::SHDLC_State(uint8_t state)
{
    if (state == SVM40_ERR_OK) return;

    // remove top bit to get real code
    state = state & 0x7f;

    switch(state) {

        case SVM40_ERR_DATA:
            DebugPrintf("0x%x: Wrong data length for this command\n", state);
            break;
        case SVM40_ERR_UCMD:
            DebugPrintf("0x%x: Unknown command\n", state);
            break;
        case SVM40_ERR_PERM:
            DebugPrintf("0x%x: No access right for command\n", state);
            break;
        case SVM40_ERR_PAR:
            DebugPrintf("0x%x: Illegal command parameter or parameter out of allowed range\n", state);
            break;
        case SVM40_ERR_RANGE:
            DebugPrintf("0x%x: Internal function argument out of range\n", state);
            break;
        case SVM40_ERR_STAT:
            DebugPrintf("0x%x: Command not allowed in current state\n", state);
            break;
        default:
            DebugPrintf("0x%x: unknown state\n", state);
            break;
    }
}

/**
 * @brief  read bytes into the receive buffer and perform byte (un)stuffing
 *
 * @return
 *   Err_OK is OK  else error
 */
uint8_t SVM40::SHDLC_SerialToBuffer() {
    uint32_t startTime;
    bool  byte_stuff = false;
    uint8_t i;

    startTime = millis();
    i = 0;

    // read until last 0x7E
    while (true)
    {
        // prevent deadlock
        if (millis() - startTime > TIME_OUT)
        {
            if ( _SVM40_Debug > 1)
                DebugPrintf("TimeOut during reading byte %d\n", i);
            return(ERR_TIMEOUT);
        }

        if (_serial->available())
        {
            _Receive_BUF[i] = _serial->read();

            // check for good header
            if (i == 0) {

                if (_Receive_BUF[i] != SHDLC_IND){
                    if ( _SVM40_Debug > 1)
                        DebugPrintf("Incorrect Header. Expected 0x7E got 0x02X\n", _Receive_BUF[i]);
                    return(ERR_PROTOCOL);
                }
            }
            else {

                // detect Byte stuffing
                if (_Receive_BUF[i] == 0x7D ) {
                    i--;                // remove stuffing byte
                    byte_stuff = true;
                }

                // handle byte stuffing
                else if (byte_stuff) {
                    _Receive_BUF[i] = SHDLC_ByteUnStuff(_Receive_BUF[i]);
                    byte_stuff = false;
                }

                // check last byte received
                else if (_Receive_BUF[i] == SHDLC_IND) {

                    _Receive_BUF_Length = i;

                    if (_SVM40_Debug){
                       DebugPrintf("Received: ");
                       for(i = 0; i < _Receive_BUF_Length+1; i++) DebugPrintf("0x%02X ",_Receive_BUF[i]);
                       DebugPrintf("length: %d\n\n",_Receive_BUF_Length);
                    }

                    /* if a board can not handle 115K you get uncontrolled input
                     * that can result in short / wrong messages
                     */
                    if (_Receive_BUF_Length < 3) return(ERR_PROTOCOL);

                    return(ERR_OK);
                }
            }

            i++;

            if(i > MAXRECVBUFLENGTH)
            {
                DebugPrintf("\nReceive buffer full\n");
                return(ERR_PROTOCOL);
            }
        }
    }
}

#endif  // INCLUDE_UART

/************************************************************
 * I2C routines
 *************************************************************/
#if defined INCLUDE_I2C
/**
 * @brief :      Fill buffer to send over I2C communication
 * @param cmd:   I2C commmand for device
 * @param param: additional parameters to add
 * @param len :  length of parameters to add (zero if none)
 *
 */
void SVM40::I2C_fill_buffer(uint16_t cmd,  uint8_t len, uint8_t *param) {
    uint8_t     i = 0, j, c;

    // add command
    _Send_BUF[i++] = cmd >> 8 & 0xff;   //0 MSB
    _Send_BUF[i++] = cmd & 0xff;        //1 LSB

    // additional parameters to add?
    if (len != 0) {

        for (j = 0, c = 0 ; j < len; j++) {

            // add bytes
            _Send_BUF[i++] = param[j];

            // add CRC after each 2 bytes
            if(++c == 2){
                _Send_BUF[i] = I2C_calc_CRC(&_Send_BUF[i - 2]);
                i++;
                c = 0;
            }
        }
    }

    _Send_BUF_Length = i;

    // set for delay  (add V2)
    // time is set wider than datasheet to be sure
    _RespDelay = RX_DELAY_MS;

    switch(cmd) {
        case SVM40_I2C_STORE_NVRAM:
            _RespDelay = 750;
            break;

        case SVM40_I2C_RESET:
            _RespDelay = 200;
            break;
    }
}

/**
 * @brief : send a prepared command (with PrepsendBuffer())
 *
 * @return :
 * Ok ERR_OK else error
 */
uint8_t SVM40::I2C_SendToSVM() {

    if (_Send_BUF_Length == 0) return(ERR_DATALENGTH);

    if (_SVM40_Debug) {
        DebugPrintf("Sending ");
        for(byte i = 0; i < _Send_BUF_Length; i++)
            DebugPrintf("0x%02X ", _Send_BUF[i]);
        DebugPrintf("\n");
    }

    _i2cPort->beginTransmission(SVM40_I2C_ADDRESS);
    _i2cPort->write(_Send_BUF, _Send_BUF_Length);

    if ( _i2cPort->endTransmission() != 0) return ERR_PROTOCOL;

    _Send_BUF_Length = 0;

    // give time to act on request
    delay(_RespDelay);

    return(ERR_OK);
}

/**
 * @brief : sent command/request and read from SVM40 sensor
 * @param cnt: number of data bytes to get
 *
 * @return :
 * OK   ERR_OK else error
 */
uint8_t SVM40::I2C_RequestFromSVM(uint8_t cnt, bool chk_zero) {
    uint8_t ret;

    // sent Request
    ret = I2C_SendToSVM();
    if (ret != ERR_OK) {
        DebugPrintf("Can not sent request\n");
        return(ret);
    }

    // read from Sensor
    ret = I2C_ReadFromSVM(cnt, chk_zero);

    if (ret != ERR_OK) {
        DebugPrintf("Error during reading. Errorcode: 0x%02X\n", ret);
    }

    if (_SVM40_Debug){
       DebugPrintf("I2C Received: ");
       for(byte i = 0; i < _Receive_BUF_Length; i++)
                    DebugPrintf("0x%02X ",_Receive_BUF[i]);
       DebugPrintf("length: %d\n\n",_Receive_BUF_Length);
    }

    return(ret);
}

/**
 * @brief       : receive from Sensor
 * @param count :    number of data bytes to expect
 * @param chk_zero : check for zero termination ( Serial and product code)
 *  false : expect and read all the data bytes
 *  true  : expect NULL termination and count is MAXIMUM data bytes
 *
 * @return :
 * OK   ERR_OK else error
 */
uint8_t SVM40::I2C_ReadFromSVM(uint8_t count, bool chk_zero) {
    uint8_t data[3];
    uint8_t i, j;

    j = i = _Receive_BUF_Length = 0;

    // 2 data bytes  + crc
    _i2cPort->requestFrom((uint8_t) SVM40_I2C_ADDRESS, uint8_t (count / 2 * 3));

    while (_i2cPort->available()) {

        data[i++] = _i2cPort->read();

        if ( _SVM40_Debug > 1) DebugPrintf("data 0x%02X\n", data[i-1]);

        // 2 bytes data, 1 CRC
        if( i == 3) {

            if (data[2] != I2C_calc_CRC(&data[0])){
                DebugPrintf("I2C CRC error: got 0x%02X, calculated 0x%02X\n",data[2] & 0xff,I2C_calc_CRC(&data[0]) &0xff);
                return(ERR_PROTOCOL);
            }

            _Receive_BUF[_Receive_BUF_Length++] = data[0];
            _Receive_BUF[_Receive_BUF_Length++] = data[1];

            i = 0;

            // check for zero termination (Serial and product code)
            if (chk_zero) {

                if (data[0] == 0 && data[1] == 0) {

                    // flush any bytes pending (added as the Apollo 2.0.1 was NOT clearing Wire rxBuffer)
                    // Logged as an issue and expect this could be removed in the future
                    while (_i2cPort->available()) _i2cPort->read();
                    return(ERR_OK);
                }
            }

            if (_Receive_BUF_Length >= count) break;
        }
    }

    if (i != 0) {
        DebugPrintf("Error: Data counter %d\n",i);
        while (j < i) _Receive_BUF[_Receive_BUF_Length++] = data[j++];
    }

    if (_Receive_BUF_Length == 0) {
        DebugPrintf("Error: Received NO bytes\n");
        return(ERR_PROTOCOL);
    }

    if (_Receive_BUF_Length == count) return(ERR_OK);

    DebugPrintf("Error: Expected bytes : %d, Received bytes %d\n", count,_Receive_BUF_Length);

    return(ERR_DATALENGTH);
}

/**
 * @brief : calculate CRC for I2c comms
 * @param data : 2 databytes to calculate the CRC from
 *
 * Source : datasheet SVM40
 *
 * @return CRC
 */
uint8_t SVM40::I2C_calc_CRC(uint8_t data[2]) {
    uint8_t crc = 0xFF;
    for(int i = 0; i < 2; i++) {
        crc ^= data[i];
        for(uint8_t bit = 8; bit > 0; --bit) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ 0x31u;
            } else {
                crc = (crc << 1);
            }
        }
    }

    return crc;
}
#endif // INCLUDE_I2C
