# Sensirion SVM40

## ===========================================================

A program to set instructions and get information from an SVM40. It has been
tested to run either UART or I2C communication on MEGA2560 and DUE.

<br> A detailed description of the options and findings are in SVM40.odt

## NEWS UPDATE

In contact with Sensirion I got the following reaction:

*About SVM40 opposed to SVM30, SVM40 is currently not a product we plan to sell independently of the evaluation kit. This might change if we
find a customer guaranteeing high enough volumes. At that point there will be a datasheet.
Currently SVM40 is considered an Evaluation Kit and can sever customers interest in the SGP40 as reference design.
I agree that some additional information for those wanting to experiment on their own would be useful.*

For sure Sensirion should have done a much better job in communicating and clearly
positioning the SVM40. With this knowledge I have re-read a number of webpages
(both from Sensirion and others)and some should be updated to reflect this much better.
Actually some are totally confusing.

That said I have updated my verbatims and will continue to follow to see
whether there is any update available and update the library accordingly.

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

### Version 1.0.1 / December 2020
 * update the product positioning and verbatim

### version 1.0 / December 2020
 * Initial version ATmega

## Author
 * Paul van Haastrecht (paulvha@hotmail.com)

## License
This project is licensed under the GNU GENERAL PUBLIC LICENSE 3.0

## Acknowledgments
Sample code on github from Sensirion

