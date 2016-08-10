The Contiki Operating System
============================

# CC2420 Radio Configurations Library.

## Abstract

## Author

	Kaleem Ullah <mscs14059@itu.edu.pk>
	Kaleem Ullah <kaleemullah360@live.com>

## Usage
	Case 1:	zero is passed in power/channel function i.e set_cc2420_txpower(0); will use predefined channel number in cc2420-radio lib.
	Case 2:	a custom value is passed in power/channel function i.e set_cc2420_txpower(13); will use channel 13.
	Case 3:	not using both/eithere of power/channel setting function will set Contiki OS default configurations i.e channel 26, tx power 31.

For more information, see the Contiki website:

[http://contiki-os.org](http://contiki-os.org)
