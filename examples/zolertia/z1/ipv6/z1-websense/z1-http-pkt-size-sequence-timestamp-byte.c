/*
 * Copyright (c) 2011, Zolertia(TM) is a trademark by Advancare,SL
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
/*---------------------------------------------------------------------------*/
/**
 * \file
 *          er-coap-server-confirmable-pkt-size-sequence-timestamp-byte
 * \Description
 *          
 *      Performnace evaluation of Zolertia Mote over CoAP protocol 
 * by just transmitting a message containing Temperature as payload a Message Sequence number
 * along with Message time (uptime). 
 *      performamnce is measured with changing 
 * payload size of 10 Bytes, 20 Bytes, 30 Bytes and 45 Bytes over 100 CoAP requests foreach payload size.
 * 
 * \author
 *          Kaleem Ullah    <MSCS14059@ITU.EDU.PK>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "httpd-simple.h"
#include "webserver-nogui.h"
#include "dev/temperature-sensor.h"
#include "dev/sht11/sht11-sensor.h"
#include "dev/battery-sensor.h"
#include "cc2420.h"
#include "dev/leds.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*/
float
floor(float x)
{
  if(x >= 0.0f) {
    return (float)((int)x);
  } else { return (float)((int)x - 1);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS(web_sense_process, "Simplest HTTP Server -I'm here to give ya few byte.");
AUTOSTART_PROCESSES(&web_sense_process);
/*---------------------------------------------------------------------------*/
#define HISTORY 16

/*---------------------------------------------------------------------------*/


 int sequence_numer = 1;
/*------------------- Get the Mote Temperature ------------------------------*/
static int get_temp(void){   // <--- This 
  return ((sht11_sensor.value(SHT11_SENSOR_TEMP) / 10) - 396) / 10;}
static int get_sequence(void){   // <--- This 
  return sequence_numer++; }
static int get_uptime(void){   // <--- This 
  return clock_seconds(); }

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Only one single request at time */
static char buf[256];
static int blen;
#define ADD(...) do { \
    blen = snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__); \
} while(0)
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_values(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);


//10 byte:
  ADD("%u,%u,%u", get_sequence(), get_uptime(), get_temp()); //10

//20 byte:
//  ADD("SN %u UPT %u PD %u", get_sequence(), get_uptime(), get_temp()); //20

//30 byte:
//  ADD("Sequence %u UpT %u Data %u", get_sequence(), get_uptime(), get_temp()); //30

//40 byte:
//  ADD("SequenceNunmber %u UTime %u Payload %u", get_sequence(), get_uptime(), get_temp()); //40

//50 byte:
//  ADD("SequenceNunmber %u UpTime %u Payload PayloadData %u", get_sequence(), get_uptime(), get_temp()); //50


  ADD("");
  SEND_STRING(&s->sout, buf);

  PSOCK_END(&s->sout);
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
  cc2420_set_txpower(31);

  process_start(&webserver_nogui_process, NULL);

  etimer_set(&timer, CLOCK_SECOND * 2);


  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
