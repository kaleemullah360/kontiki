/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */
/*
 * \file
 *         Evaluation of HTTP Protocols at Application layer.
 * \author
 *         Kaleem Ullah    <MSCS14059@ITU.EDU.PK>
 *
 * \Short Description:
 *
 *    e-MCH-APp (Evaluation MQTT, CoAP, HTTP protocol) using Node.Js server.
 * this application uses Zolertia z1 mote. on each request, 
 * power traces are save in data base while we also sent uptime along with battery & temperature, 
 * while request we also compute RTT.
 *
 */

#include "contiki.h"
#include "httpd-simple.h"
#include <stdio.h>
#include <cc2420-radio.h>

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
PROCESS(web_sense_process, "e-MCH-APp Power Tracing");
PROCESS(webserver_nogui_process, "e-MCH server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
AUTOSTART_PROCESSES(&web_sense_process,&webserver_nogui_process);


/*---------------------------------------------------------------------------*/
/* Only one single request at time */
static char buf[256];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)

static
PT_THREAD(send_values(struct httpd_state *s))
{
powertrace_start(CLOCK_SECOND * 1);
//----- Get Data Instance -------
get_sensor_time();
PSOCK_BEGIN(&s->sout);
blen = 0;
// variables
ADD("%lu,%lu,0,0,0,%s", mid++, upt, powertrace_result());
printf("Message %lu Sent on: %lu \n", mid, upt);
printf("Ticks per second: %u\n", RTIMER_SECOND);
//----- End Get Data -------
SEND_STRING(&s->sout, buf);
PSOCK_END(&s->sout);
powertrace_stop();
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return send_values;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(web_sense_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();
	set_cc2420_txpower(0);
	set_cc2420_channel(0);
  enable_rdc(1);
  

  etimer_set(&timer, CLOCK_SECOND * 2);


  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
