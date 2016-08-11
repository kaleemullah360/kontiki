# RADIO MOD

## CC2420 Radio Configurations Library.


### Problem:

1. Tired of setting custom Channel and Transmission ranges in number of nodes ?. 
2. Often forget to change Channel or Transmission range in one of node in network and wonder why it is not functioning ?
3. want to set Channel and Transmission ranges in single step for whole network ?, also want to set a quit different configurations for one ore more nodes ?

### Solution:
include the library in your application and set the desired channel and TX power for your network thats it!.

## Abstract
The RADIO MOD library uses very small code footprints. Very light weight and usefull when creating multi hops or mesh network.
This library enhance the cc2420 radio methods. it uses `cc2420_set_txpower(radioChannel_tx_power);`, `cc2420_set_channel(radioChannel);`.

## Setup:

include the library in your application `Makefile`

	```sh
	APPS += config-radio
	```

call the methods with approperiate settings.

Change th channel or TX power of all the nodes wich includes this library.

In file

	~/Contiki/app/config-radio/cc2420-radio.c

Change the values of follwing variables.

	uint8_t radioChannel = 26;  // default channel
	uint8_t radioChannel_tx_power = 31; // default power

	set_cc2420_txpower(0);	// setting zero will use predefined configurations in 
	set_cc2420_channel(0);

## Author
Kaleem Ullah <mscs14059@itu.edu.pk>

Kaleem Ullah <kaleemullah360@live.com>

## Usage
###Case 1:	
zero is passed in power/channel function i.e set_cc2420_txpower(0); will use predefined channel number in cc2420-radio lib

###Case 2:
a custom value is passed in power/channel function i.e set_cc2420_txpower(13); will use channel 13.

###Case 3:
not using both/eithere of power/channel setting function will set Contiki OS default configurations i.e channel 26, tx power 31.

For more information, see the Contiki website:

[http://contiki-os.org](http://contiki-os.org)
