/*
 * Copyright (c) 2014, Nimbus Centre for Embedded Systems Research, Cork Institute of Technology.
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
 *      SHT11 Sensor Resource
 *
 *      This is a simple GET resource that returns the temperature in Celsius
 *      and the humidity reading from the SHT11.
 * \author
 *      Pablo Corbalan <paul.corbalan@gmail.com>
 */

#include "contiki.h"

#if PLATFORM_HAS_ZSTEMP

#include <string.h>
#include "rest-engine.h"
#include "dev/sht11/sht11-sensor.h"

 static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
 int sequence_numer = 1;
   /*-------------------------------Z1 Get Temperature only-------------*/
  static int get_temp(void){
      return ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10; }
  static int get_sequence(void){   // <--- This 
      return sequence_numer++; }
  static int get_uptime(void){   // <--- This 
      return clock_seconds(); }
   /*--------------------------------------------------------------------*/

/* Get Method Example. Returns the reading from temperature and humidity sensors. */
  RESOURCE(res_zstemp_sequence_timestamp,
   "title=\"Temperature and Humidity\";rt=\"Sht11\"",
   res_get_handler,
   NULL,
   NULL,
   NULL);

  static void
  res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
  {
  /* Temperature in Celsius (t in 14 bits resolution at 3 Volts)
   * T = -39.60 + 0.01*t
   */

  /* Relative Humidity in percent (h in 12 bits resolution)
   * RH = -4 + 0.0405*h - 2.8e-6*(h*h)
   */
 // uint16_t rh = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);

   unsigned int accept = -1;
   REST.get_header_accept(request, &accept);

   if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    //iot-2/evt/status/fmt/json ,"SequenceNo:":"97","UpTime":124,"Data:":"65497"  // <-- This
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "SequenceNunmber %u UTime %u Payload %u", get_sequence(), get_uptime(), get_temp()); //40


    /*
10 byte:
  "%u,%u,%u", get_sequence(), get_uptime(), get_temp()

20 byte:
  "SN %u UPT %u PD %u", get_sequence(), get_uptime(), get_temp()

30 byte:
  "Sequence %u UpT %u Data %u", get_sequence(), get_uptime(), get_temp()

40 byte:
"SequenceNunmber %u UTime %u Payload %u", get_sequence(), get_uptime(), get_temp()

50 byte:
  "SequenceNunmber %u UpTime %u Payload PayloadData %u", get_sequence(), get_uptime(), get_temp()
*/

    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_XML) {
    REST.set_header_content_type(response, REST.type.APPLICATION_XML);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "<Temperature =\"%u\"", get_temp());

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_JSON) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'Sht11':{'Temperature':%u}}", get_temp());

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
    REST.set_response_payload(response, msg, strlen(msg));
  }
}
#endif /* PLATFORM_HAS_ZSTEMP */

/*
#if PLATFORM_HAS_ZSTEMP_SEQUENCE_TIMESTAMP
#include "dev/sht11/sht11-sensor.h"
extern resource_t res_zstemp_sequence_timestamp; // <--- This
#endif

#if PLATFORM_HAS_ZSTEMP   // <--- This
  leds_on(LEDS_GREEN);
  rest_activate_resource(&res_zstemp_sequence_timestamp, "zolertia/sensor/temperature");  
  SENSORS_ACTIVATE(sht11_sensor);
  leds_off(LEDS_GREEN);  
#endif

*/