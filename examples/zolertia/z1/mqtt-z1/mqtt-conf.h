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
 *         MQTT Protocol configurations
 * \author
 *         Kaleem Ullah <mscs14059@itu.edu.pk>
 *         Kaleem Ullah <kaleemullah360@live.com>
 *
 * \Description
 *			This configuration file is usefull for tweaking the Protocol. include the protocol in your project and set the desired values in this file. you're good to go.
 */

/**
 * MQTT Quality of Services
 * --------------------------------------------
 * QoS  | Parameter       | Description       |
 *---------------------------------------------
 * QoS0 |MQTT_QOS_LEVEL_0 | Fire & Forget     |
 * QoS1 |MQTT_QOS_LEVEL_1 | Fire atleast ONCE |
 * QoS2 |MQTT_QOS_LEVEL_2 | Fire exactly ONCE |
 *---------------------------------------------
 */
/*---------------------------------------------------------------------------*/
#ifndef MQTT_CONF_H_
#define MQTT_CONF_H_

/*---------------------------------------------------------------------------*/

/* User configuration */
#define TIME_INTERVAL_SECONDS 1	// 1 Seconds
#define MILLISECONDS_CONSTANT 1		// 500 milliseconds = 1 second / 2

#define MQTT_QOS 						MQTT_QOS_LEVEL_0
#define	MQTT_MESSAGE_STATE				MQTT_RETAIN_OFF
	
/*---------------------------------------------------------------------------*/
	
/* Default configuration values */	
#define DEFAULT_TYPE_ID             	"cc2420"
#define DEFAULT_AUTH_TOKEN          	"F1R3W1R3"
#define DEFAULT_EVENT_TYPE_ID       	"status"
#define DEFAULT_SUBSCRIBE_CMD_TYPE  	"+"
#define DEFAULT_BROKER_PORT         	1883
#define DEFAULT_PUBLISH_INTERVAL    	(TIME_INTERVAL_SECONDS * (CLOCK_SECOND/MILLISECONDS_CONSTANT))
#define DEFAULT_KEEP_ALIVE_TIMER    	60
#define DEFAULT_RSSI_MEAS_INTERVAL  	(CLOCK_SECOND * 30)

/*---------------------------------------------------------------------------*/
#endif /* MQTT_CONF_H_ */
/*---------------------------------------------------------------------------*/
/** @} */
