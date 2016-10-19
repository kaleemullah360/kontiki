/*
 * Copyright (c) 2011-2012, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *
 *  cc2420-radio.c
 *
 * \author
 *
 *	Kaleem Ullah <mscs14059@itu.edu.pk>
 *	Kaleem Ullah <kaleemullah360@live.com>
 *
 * \Usage
 *
 *	Case 1:	zero is passed in power/channel function i.e set_cc2420_txpower(0); will use predefined channel number in cc2420-radio lib.
 *	Case 2:	a custom value is passed in power/channel function i.e set_cc2420_txpower(13); will use channel 13.
 *	Case 3:	not using both/eithere of power/channel setting function will set Contiki OS default configurations i.e channel 26, tx power 31.
 */

/**
*          ,
*         `$b
*    .ss,  $$:         .,d$
*    `$$P,d$P'    .,md$P"'
*     ,$$$$$bmmd$$$P^'
*   .d$$$$$$$$$$P'
*   $$^' `"^$$$'    
*   $:     ,$$:       
*   `b     :$$        
*          $$:        
*          $$ 
*        .d$$                KuSu
*
* https://cruise.eecs.uottawa.ca/umpleonline/
* wow, what's that ?
*/

/* -------- Set Radio Powers --------------- */

#include <cc2420-radio.h>

/*|Power (dBm)|PA_LEVEL|Power (mW)|
* |0          |  31    |1.0000    |
* |-0.0914    |  30    |0.9792    |
* |-25.0000   |  3     |0.0032    |
* |-28.6970   |  2     |0.0013    |
* |-32.9840   |  1     |0.0005    |
* |-37.9170   |  0     |0.0002    |
*/ 
uint8_t radioChannel = 26;  // default channel
uint8_t rdc_channel_check_rate = 4;	// default in MHz
char *rdc_driver = "nullrdc_driver";	// default
char *mac_driver = "nullmac_driver";
//uint8_t radioChannel_tx_power = 0; // custom power
uint8_t radioChannel_tx_power = 31; // default power

void set_cc2420_channel(custom_radioChannel){
	/* channel */
	if(!custom_radioChannel == 0){
		radioChannel = custom_radioChannel;
	}
	cc2420_set_channel(radioChannel);
	printf("CC2420 Radio channel %d\n", cc2420_get_channel());
}


void set_cc2420_txpower(custom_radioChannel_tx_power){
	/* tx power */
	if(!custom_radioChannel_tx_power == 0){
		radioChannel_tx_power = custom_radioChannel_tx_power;
	}
	cc2420_set_txpower(radioChannel_tx_power);
	printf("CC2420 Radio TX power %d\n", cc2420_get_txpower());
}

/*
The Contiki RDC drivers are called:
 contikimac_driver
 cxmac_driver
 nullrdc_driver
*/

void enable_rdc(rdcStatus){

	print_netstack();

	if (rdcStatus == 1){
/*
	#undef NETSTACK_CONF_RDC
	#define NETSTACK_CONF_RDC	nullrdc_driver
	#undef NETSTACK_CONF_MAC
	#define NETSTACK_CONF_MAC	nullmac_driver
		
	#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
	#undef NETSTACK_CONF_RDC
	#undef NETSTACK_CONF_MAC

	#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE rdc_channel_check_rate
	#define NETSTACK_CONF_RDC *rdc_driver
	#define NETSTACK_CONF_MAC *mac_driver
	*/
	printf("CC2420 Radio Channel check rate [ %d Hz ]\n", NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE);
	printf("CC2420 Radio RDC Driver [ %s ]\n", (char*)rdc_driver);
	printf("CC2420 Radio MAC Driver [ %s ]\n", (char*)mac_driver);
	}else{
		printf("No duty cycling enabled, RDC status [%d] \n", rdcStatus);
	}
}

void set_cc2420_rdcmac(custom_rdc_channel_check_rate, custom_rdc_driver, custom_mac_driver){
	printf("%d, %s, %s\n", custom_rdc_channel_check_rate, (char*)custom_rdc_driver, (char*)custom_mac_driver);
	/* first remove existing settings from z1 platform configuration */

	#undef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
	#undef NETSTACK_CONF_RDC
	#undef NETSTACK_CONF_MAC
	
	char *driver="oh its a test driver";
	printf("Pointer Address= %p\n", &driver);
	
	printf("%d\n", sizeof(sprintf(driver, "Pointer value= %s\n", driver)));

	if(custom_rdc_channel_check_rate != 0){
		rdc_channel_check_rate = custom_rdc_channel_check_rate;
	}if(sizeof(custom_rdc_driver) > 2){
		rdc_driver = (char*)custom_rdc_driver;
	}if(sizeof(custom_mac_driver) > 2){
		mac_driver = (char*)custom_mac_driver;
	}

	/* set new default settings for duty cycling */
	#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE rdc_channel_check_rate
	#define NETSTACK_CONF_RDC *rdc_driver
	#define NETSTACK_CONF_MAC *mac_driver

	printf("CC2420 Radio Channel check rate %d Hz\n", NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE);
	printf("CC2420 Radio RDC Driver %s\n", (char*)rdc_driver);
	printf("CC2420 Radio MAC Driver %s\n", (char*)mac_driver);
}

void print_netstack(void) {
	/* Initialize communication stack */
  	netstack_init();
	printf("%s %s, channel check rate %lu Hz, radio channel %u\n",
	   NETSTACK_MAC.name, NETSTACK_RDC.name,
	   CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1:
		       NETSTACK_RDC.channel_check_interval()),
	   RF_CHANNEL);
}
/* -------- End Set Radio Powers ------------ */
