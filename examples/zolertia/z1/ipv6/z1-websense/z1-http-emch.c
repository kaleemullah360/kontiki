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
 *         Battery and Temperature IPv6 Demo for Zolertia Z1
 * \author
 *         Niclas Finne    <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Joel Hoglund    <joel@sics.se>
 *         Enric M. Calvo  <ecalvo@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "httpd-simple.h"
#include "webserver-nogui.h"
#include "cc2420.h"
#include "dev/leds.h"
#include <stdio.h>
//--- Libs for E-MCH-APP ----
#include "dev/temperature-sensor.h"
#include "dev/battery-sensor.h"
//---End Libs for E-MCH-APP ---
//--- Function & Variable for E-MCH-APP ----
static int32_t mid = 0;  // MessageID
static int32_t upt = 0;  // UpTime
static int32_t clk = 0;  // ClockTime
static float tem = 0;  // Temperature
static uint16_t bat_v = 0; // Battery in Volts
static float bat_mv = 0; // Battery in MilliVolts

float floor(float x){
  if(x >= 0.0f) {
    return (float)((int)x);
  } else {
    return (float)((int)x - 1);
  }
}
static float stmp(void){
  return (float)(((temperature_sensor.value(0) * 2.500) / 4096) - 0.986) * 282;
}
static int sbat_v(void){
  return battery_sensor.value(0);
}
static float sbat_mv(bat_v){
  return (bat_v * 2.500 * 2) / 4096;
}
//---End Function & Variable for E-MCH-APP ---
/*---------------------------------------------------------------------------*/

PROCESS(web_sense_process, "Sense Web Demo");
AUTOSTART_PROCESSES(&web_sense_process);
/*---------------------------------------------------------------------------*/
/* Only one single request at time */
static int sensors_pos;
static char buf[256];
static int blen;
#define ADD(...) do { \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__); \
} while(0)
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_values(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
    blen = 0;
//----- Get Data Instance -------
  ++mid;  // MessageID
  upt = clock_seconds();  // UpTime
  clk = clock_time(); // ClockTime
  tem = stmp();  // Temperature
  bat_v = sbat_v(); // Get Battery in Volts
  bat_mv = sbat_mv(bat_v);  //Get Battery in MilliVolts
//----- End Get Data -------
  ADD(" ");
  //  MessageID, UpTime, ClockTime, Temperature, Battery  //<-- This
  // "%lu,%lu,%lu,%ld.%03d,%ld.%03d", mid,upt,clk,tem.tem,bat.bat
  //ADD("%lu,%lu,%lu,%ld.%03d,%ld.%03d", mid,upt,clk,(long)tem,(unsigned)((tem - floor(tem)) * 1000),(long)bat_mv,(unsigned)((bat_mv - floor(bat_mv)) * 1000));
  // "%lu,%lu,%lu,%ld.%03d,%ld.%03d", mid,upt,clk,tem,bat
  ADD("%lu,%lu,%lu,%ld,%03d", mid,upt,clk,(long)tem,(unsigned)((bat_mv - floor(bat_mv)) * 1000));
  ADD(" ");

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

  sensors_pos = 0;
  process_start(&webserver_nogui_process, NULL);

  etimer_set(&timer, CLOCK_SECOND * 2);

  // Activate Temperature and Battery Sensors  
  SENSORS_ACTIVATE(battery_sensor);
  SENSORS_ACTIVATE(temperature_sensor);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
