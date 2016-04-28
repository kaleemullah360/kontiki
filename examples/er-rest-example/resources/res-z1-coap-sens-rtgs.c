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
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include "contiki.h"

#if PLATFORM_HAS_BATTERY

#include <string.h>
#include "rest-engine.h"
//--- Libs for e-MCH-APp ----

#include "dev/battery-sensor.h"
#include "dev/i2cmaster.h"
#include "dev/tmp102.h"

//---End Libs for e-MCH-APp ---

//--- Variable Declaration for e-MCH-APp ----

 static int32_t mid = 0;  // MessageID
 static int32_t upt = 0;  // UpTime
 static int32_t clk = 0;  // ClockTime

  // temperature function variables 
 static int16_t tempint;
 static uint16_t tempfrac;
 static int16_t raw;
 static uint16_t absraw;
 static int16_t sign;
 static char minus = ' ';

  // Battery function variables 
 static uint16_t bat_v = 0;
 static float bat_mv = 0; 
 static char *status_str = "1";
//---End Variable Declaration e-MCH-APp ---

//--- Function Deffinitions for e-MCH-APp ----

// function to return floor of float value
 float floorsens(float x){
  if(x >= 0.0f){ // check the value of x is +eve
    return (float)((int) x);
  }else{ // if value of x is -eve
    // x = -2.2
    // -3.2 = (-2.2) - 1
    // -3  = (int)(-3.2)
    //return -3.0 = (float)(-3)
    return(float) ((int) x - 1);   
  } //end if-else

} //end floor function

static void get_sensor_time(){
  upt = clock_seconds();  // UpTime
  clk = clock_time(); // ClockTime
}

static void get_sensor_temperature(){
  tmp102_init();  // Init Sensor
  sign = 1;
  raw = tmp102_read_temp_x100(); // tmp102_read_temp_raw();
  absraw = raw;
    if(raw < 0) {   // Perform 2C's if sensor returned negative data
      absraw = (raw ^ 0xFFFF) + 1;
    sign = -1;
  }
  tempint = (absraw >> 8) * sign;
    tempfrac = ((absraw >> 4) % 16) * 625;  // Info in 1/10000 of degree
    minus = ((tempint == 0) & (sign == -1)) ? '-' : ' ';
    //printf("Temp = %c%d.%04d\n", minus, tempint, tempfrac);
  }

  static void get_sensor_battery(){
  // Activate Temperature and Battery Sensors  
    SENSORS_ACTIVATE(battery_sensor);
  // prints as fast as possible (with no delay) the battery level.
    bat_v = battery_sensor.value(0);
  // When working with the ADC you need to convert the ADC integers in milliVolts. 
  // This is done with the following formula:
    bat_mv = (bat_v * 2.500 * 2) / 4096;
  //printf("Battery Analog Data Value: %i , milli Volt= (%ld.%03d mV)\n", bat_v, (long) bat_mv, (unsigned) ((bat_mv - floorsens(bat_mv)) * 1000));
  }

//---End Function Deffinitions e-MCH-APp ---

 static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple getter example. Returns the reading from light sensor with a simple etag */
 RESOURCE(res_z1_coap_sens_rtgs,
 	"title=\"Sensor\";rt=\"status\"",
 	res_get_handler,
 	NULL,
 	NULL,
 	NULL);

 static void
 res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
 {
 	
 //----- Get Data Instance -------
++mid;  // MessageID
get_sensor_temperature();
get_sensor_time();
get_sensor_battery();
//----- End Get Data -------

 	unsigned int accept = -1;
 	REST.get_header_accept(request, &accept);

 	if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
 		REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
 		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,  "%lu,%lu,%lu,%c%d.%04d,%ld.%03d,%s", mid, upt, clk, minus,tempint,tempfrac, (long) bat_mv, (unsigned) ((bat_mv - floorsens(bat_mv)) * 1000),status_str);

 		REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
 	}
 }
#endif /* PLATFORM_HAS_BATTERY */
