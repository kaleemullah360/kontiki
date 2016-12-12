
[Source](http://zolertia.sourceforge.net/wiki/index.php/Z1 "Permalink to Z1 - Zolertia")

# Z1 - Zolertia

Recent technology advances in areas like computing, networking, sensing and wireless communication, now enable a new paradigm where objects and environments can interact in a seamlessly way to monitor and control environments. The time for smart objects in smart environments has come.

Discover our new flagship WSN mote, **the Z1**.

  
![File:Z1.png][1]

##  Ultra-Low Power MCU and 2.4GHz Transceiver

Z1 is equipped with a second generation MSP430F2617 low power microcontroller, which features a powerful 16-bit RISC CPU @16MHz clock speed, built-in clock factory calibration, 8KB RAM and a 92KB Flash memory. Also includes the well known CC2420 transceiver, IEEE 802.15.4 compliant, which operates at 2.4GHz with an effective data rate of 250Kbps. Z1 hardware selection guarantees the maximum efficience and robustness with low energy cost.

##  2 x Digital Built-in sensors

: Z1 comes with built-in digital sensors ready to work: a digital programmable accelerometer (ADXL345) and a digital temperature sensor (TMP102) are on the main board.

##  Up to 4 External Sensors

: Z1 is compatible with [ Phidgets][2] and many other analog and digital sensors. Stay tuned for an upcoming release of the new sensor interface developed in Zolertia Labs: [ Ziglets][3]. Enhancing the way the world can be seen, in just one device.

##  USB Programming Ready

: Z1 do not require any external hardware to be programmed, built-in full USB capability allows quick developing of WSN applications and rapid integration with multiple systems.

##  Flexible Powering

: The board can be powered in many ways: Battery Pack (2xAA or 2xAAA), Coin Cell (up to 3.6V), USB Powered, Directly Connected through two wires comming from a power source. USB VCC and GND pins are available on the digital buses expansion port. You can connect to this pins any power of source from 4V to 5.25V and it will be regulated to 5V and 3V.

##  Z1 Starter Platform

: More to come with our new [ Z1 Starter Platform][4], which features additional leds, buzzer, 3 and 5V regulated out and many other features.

In this section you will find a brief overview of the components inside Z1. The complete Z1 documentation can be seen online or be downloaded to be reviewed later in the links below:

For more detailed information of the design, see the schematics in the [Hardware Schematics][5] section.

###  MSP430F2617 MCU from Texas Instruments

Features

![][6]

![][7]

MSP430F2617 Pin out

:

* Low Supply Voltage Range, 1.8 V to 3.6 V
* Ultra-Low Power Consumption:
    * Active Mode: 365 µA at 1 MHz, 2.2 V
    * Standby Mode (VLO): 0.5 µA
    * Off Mode (RAM Retention): 0.1 µA
* Wake-Up From Standby Mode in Less Than 1 µs
* 16-Bit RISC Architecture, 62.5-ns Instruction Cycle Time
* Three-Channel Internal DMA
* 12-Bit Analog-to-Digital (A/D) Converter With Internal Reference,
* Sample-and-Hold, and Autoscan Feature
* Dual 12-Bit Digital-to-Analog (D/A)
* 16-Bit Timer_A With Three Capture/Compare Registers
* 16-Bit Timer_B With Seven Capture/Compare-With-Shadow Registers
* On-Chip Comparator
* Four Universal Serial Communication Interfaces (USCIs)
    * USCI_A0 and USCI_A1
        * Enhanced UART Supporting Auto-Baudrate Detection
        * IrDA Encoder and Decoder
        * Synchronous SPI
    * USCI_B0 and USCI_B1

The installation of the MSP430 toolchain and environment depends of the target operative system, for detailed information please refer to the installations guides for TinyOS and Contiki.

  
Documentation:

###  CC2420 Transceiver from Texas Instruments / Chipcon

![][8]

![][7]

CC2420 features

The CC2420 is a true single-chip 2.4 GHz IEEE 802.15.4 compliant RF transceiver designed for low-power and low-voltage wireless applications. CC2420 includes a digital direct sequence spread spectrum baseband modem providing a spreading gain of 9 dB and an effective data rate of 250 kbps.

The CC2420 is a low-cost, highly integrated solution for robust wireless communication in the 2.4 GHz unlicensed ISM band. It complies with worldwide regulations covered by ETSI EN 300 328 and EN 300 440 class 2 (Europe), FCC CFR47 Part 15 (US) and ARIB STD-T66 (Japan).

The CC2420 provides extensive hardware support for packet handling, data buffering, burst transmissions, data encryption, data authentication, clear channel assessment, link quality indication and packet timing information.

Proper drivers for both TinyOS and Contiki are available as a part of its Operative system.

  
Documentation:

###  CP2102 USB-to-UART Bridge Controller from SiLabs

The CP2102 is a highly-integrated USB-to-UART Bridge Controller providing a simple solution for updating RS-232 designs to USB using a minimum of components and PCB space. The CP2102 includes a USB 2.0 full-speed function controller, USB transceiver, oscillator, EEPROM, and asynchronous serial data bus (UART) with full modem control signals in a compact 5 x 5 mm QFN-28 package. No other external USB components are required.

CP2102 is connected to msp430 through USCIA0 used as UART.

Drivers for Linux systems are available as a part of the distributions releases, CP2102 is recognized without problem on any modern distribution. If you have problems with CP2102 (for example for embedded computers or other architectures) Silabs provides drivers for linux (see below). For Windows and Mac distributions drivers are also available:

  
Documentation:

###  Built-in Sensors

![][9]

![][7]

Orientation vs. output for ADXL345 sensor

The ADXL345 is a small, thin, low power, 3-axis accelerometer with high resolution (13-bit) measurement at up to ±16 g. Digital output data is formatted as 16-bit twos complement and is accessible through either a SPI (3- or 4-wire) or I2C digital interface.

The TMP102 is ideal for extended temperature measurement in a variety of communication, computer, consumer, environmental, industrial, and instrumentation applications. The device is specified for operation over a temperature range of -40°C to +125°C.

Please refer to the following links for more information and documentation:

* [ADXL345][10] 3-axis programmable digital accelerometer from Analog Devices
* [TMP102][11] Thermometer from Texas Instruments

###  Supported External Sensors

Z1 features a full range of pins available for connecting many kinds of sensors in a really easy way. Pinout is divided in 3 areas (each at one border of the board): Analog sensors, digital buses and GPIO. The Z1 mote provides out-of-the box support for all the phidgets [[1]][12] when powered at 5V and most of them when powered at 3V. Check the list of [Phidgets][2] to see which of them are compatible.

A full list of sensors known to work with Z1 can be found [Here][13]

###  Antennae

Z1 motes comes with an integrated ceramic antena from Yageo/Phycomp connected to the CC2420 throught the C62 capacitor.

Optionally, an external antenna can be connected via a u.FL connector. _Note that this option requires to move the output capacitor C62 towards the uFL connector, which is a task that you can do if you have high soldering skills._

Documentation:

Since the Z1 platform is under every circumstance an aggregation of integrated circuits whose main core is a T.I. microcontroller, the Z1 module can be programmed in C from scratch. Note, however, that in order to help you take advantage of the radio transceiver and on-board sensors without the hassle of writing your own device drives, we recommend you to use any of the supported OS described below.

##  Embedded Operating Systems

Currently Z1 supports:

Please refer to the links above for installation instructions and examples.

[1]: http://zolertia.sourceforge.net/wiki/images/a/a3/Z1.png
[2]: /wiki/index.php/Mainpage:allsensors#Phidgets "Mainpage:allsensors"
[3]: /wiki/index.php/Mainpage:allsensors#Ziglets "Mainpage:allsensors"
[4]: /wiki/index.php/Mainpage:z1sp "Mainpage:z1sp"
[5]: /wiki/index.php/Hardware_Schematics "Hardware Schematics"
[6]: http://zolertia.sourceforge.net/wiki/images/thumb/3/37/Msp430_pinout.png/350px-Msp430_pinout.png
[7]: http://zolertia.sourceforge.net/wiki/skins/common/images/magnify-clip.png
[8]: http://zolertia.sourceforge.net/wiki/images/thumb/9/91/CC2420_features.png/300px-CC2420_features.png
[9]: http://zolertia.sourceforge.net/wiki/images/thumb/6/6c/ADXL345_axis.png/280px-ADXL345_axis.png
[10]: /wiki/index.php/Z1_Sensors#ADXL345_Accelerometer "Z1 Sensors"
[11]: /wiki/index.php/Z1_Sensors#TMP102_Temperature "Z1 Sensors"
[12]: http://www.phidgets.com "http://www.phidgets.com"
[13]: /wiki/index.php/Z1_Sensors "Z1 Sensors"