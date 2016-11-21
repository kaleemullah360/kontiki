/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 *      Example resource
 * \author
 *      Kaleem Ullah <kaleemullah360@live.com>
 *      Kaleem Ullah <mscs14059@itu.edu.pk>
 */

#include "contiki.h"

#if PLATFORM_HAS_BATTERY

#include <string.h>
#include "rest-engine.h"

// Powertracing
#include "powertrace-z1.h"
char *powertrace_result();
//char *pow_str = "";

//--- Variable Declaration for e-MCH-APp ----

 static int32_t mid = 0;  // MessageID
 static int32_t upt = 0;  // UpTime

//---End Variable Declaration e-MCH-APp ---

//--- Function Deffinitions for e-MCH-APp ----

static void get_sensor_time(){
  upt = clock_seconds();  // UpTime
}

//---End Function Deffinitions e-MCH-APp ---

 static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple getter example. Returns the reading from light sensor with a simple etag */
 RESOURCE(res_z1_coap_emch_pow,
 	"title=\"Sensor\";rt=\"status\"",
 	res_get_handler,
 	NULL,
 	NULL,
 	NULL);

 static void
 res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
 {
 	
 	unsigned int accept = -1;
 	REST.get_header_accept(request, &accept);

 	if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
 		// power trace start here
 		powertrace_start(CLOCK_SECOND * 1);

 		REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
 		//----- Get Data Instance -------
		get_sensor_time();
		//pow_str = powertrace_result();
    // id, MessageID, UpTime, ClockTime, Temperature, Battery, Protocol, RTT, PowTrace, created_at
    // db, MessageNo, RunTime,  RSSI,         LQI,      N/A,   Protocol, RTT, PowTrace,   db
snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%lu,%lu,%d,%d,0,%s", mid++, upt, cc2420_last_rssi, cc2420_last_correlation, powertrace_result());
	    printf("Message %lu Sent on: %lu \n", mid, upt);
	    printf("Ticks per second: %u\n", RTIMER_SECOND);
	    //----- End Get Data -------
 		REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
 		// power trace ends here
 		powertrace_stop();
 	}
 }
#endif /* PLATFORM_HAS_BATTERY */
