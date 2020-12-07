# Sensirion SVM40

## ===========================================================

A program to set instructions and get information from an SVM40. It has been
tested to run either UART or I2C communication on MEGA2560 and DUE.

<br> A detailed description of the options and findings are in SVM40.odt

## Getting Started
he SVM40 brand new (December 2020) and given that I have worked on the SVM30 and
other Sensirion sensors with a lot of pleasure, was looking forward to get my hands on it.

Unlike the SVM30, where one could address the SGP30 and SHTC1 directly with I2C,
the SVM40 is following the SPS30 approach with having a single MCU in between.
While this adds the possibility to connect either with I2C or Serial,
it does seem to limit to use the full potential of both sensors.

### 2 months after launch!!
There is NO datasheet available and this moment. I see parts of sample code still
being changed. My Firmware, on the brand new SVM40 bought last week,
is 1.1 while 2.0 is already available.
<br> I think this start with SVM40 is really an extremely bad performance of Sensirion.

### It looks the sensor is rushed to market while not being ready.
### Personally I would NOT advice for someone to obtain and SVM40 now. !

I hope they catch up soon as this is not my previous experience with Sensirion.
I will update the library and documentation accordingly.

## Software installation
Obtain the zip and install like any other

## Program usage
### Program options
Please see the description in the top of the sketch and read the documentation (odt)

## Versioning

### version 1.0 / December 2020
 * Initial version ATmega

## Author
 * Paul van Haastrecht (paulvha@hotmail.com)

## License
This project is licensed under the GNU GENERAL PUBLIC LICENSE 3.0

## Acknowledgments
Sample code on github from Sensirion

