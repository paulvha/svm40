# Sensirion SVM40

## ===========================================================

A program to set instructions and get information from an SVM40. It has been
tested to run either UART or I2C communication on MEGA2560 and DUE.

<br> A detailed description of the options and findings are in SVM40.odt

## NEWS UPDATE

In 25 December 2020 datasheet for the SVM40 have bee provided. The library
has been updated accordingly to Version 2.0

## Getting Started
The SVM40 brand new (December 2020) and given that I have worked on the SVM30 and
other Sensirion sensors with a lot of pleasure, was looking forward to get my hands on it.

Unlike the SVM30, where one could address the SGP30 and SHTC1 directly with I2C,
the SVM40 is following the SPS30 approach with having a single MCU in between.
While this adds the possibility to connect either with I2C or Serial,
it does seem to limit to use the full potential of both sensors.

While this is apperently NOT a product that is planned to be sold independently,
I expected that Sensirion would have provided the bear minimum information. If it is
to demonstrate the SGP40, then make all it's functions available through the
MCU interface and be clear how to use the SVM40.

## Software installation
Obtain the zip and install like any other.

## Program usage
### Program options
Please see the description in the top of the sketch and read the documentation (odt)

## Versioning
### version 2.1 / october 2023
 * Added update with testing on UNO-R4 Wifi with using I2C
 * When using Wire, pull-up resistors to SDA and SCL need to be applied for it to work as they are NOT populated on the UNO-R4.
 * When using QWIIC/Wire1 on UNO-R4 WIFI, there is NO need for pull resistors as the are on the board.
 * Special attention for connecting Serial1 on the UNO-R4 WIFI with level shifter! (see document chapter 2.2.1)

### Version 2.0 / December 2020
 * updated the code and documentation based on the SVM40 datasheets
 * update to examples

### Version 1.0.1 / December 2020
 * update the product positioning and verbatim

### version 1.0 / December 2020
 * Initial version ATmega

## Author
 * Paul van Haastrecht (paulvha@hotmail.com)

## License
This project is licensed under the GNU GENERAL PUBLIC LICENSE 3.0

## Acknowledgments
Sample code on github and Datasheet from Sensirion

