/*
 * Copyright (c) 2011, Zolertia(TM) is a trademark of Advancare,SL
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
 *
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         Testing the internal MSP430 battery sensor on the Zolertia Z1 Platform.
 * \author
 *         Enric M. Calvo <ecalvo@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/battery-sensor.h"
#include <stdio.h>
#include <cc2420.h>
#include "net/packetbuf.h"

#include <string.h>


#include "contiki-lib.h"
#include "sys/compower.h"
#include <powertrace.h>
 struct powertrace_sniff_stats {
 	struct powertrace_sniff_stats *next;
 	unsigned long num_input, num_output;
 	unsigned long input_txtime, input_rxtime;
 	unsigned long output_txtime, output_rxtime;
#if NETSTACK_CONF_WITH_IPV6
  uint16_t proto; /* includes proto + possibly flags */
#endif
 	uint16_t channel;
 	unsigned long last_input_txtime, last_input_rxtime;
 	unsigned long last_output_txtime, last_output_rxtime;
 };

#define INPUT  1
#define OUTPUT 0

#define MAX_NUM_STATS  16

 MEMB(stats_memb, struct powertrace_sniff_stats, MAX_NUM_STATS);
 unsigned long time, all_time, radio, all_radio;

 void
 powertrace_print(char *str)
 {
 	static unsigned long last_cpu, last_lpm, last_transmit, last_listen;
 	static unsigned long last_idle_transmit, last_idle_listen;

 	unsigned long cpu, lpm, transmit, listen;
 	unsigned long all_cpu, all_lpm, all_transmit, all_listen;
 	unsigned long idle_transmit, idle_listen;
 	unsigned long all_idle_transmit, all_idle_listen;

 	static unsigned long seqno;

 	energest_flush();

 	all_cpu = energest_type_time(ENERGEST_TYPE_CPU);
 	all_lpm = energest_type_time(ENERGEST_TYPE_LPM);
 	all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
 	all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
 	all_idle_transmit = compower_idle_activity.transmit;
 	all_idle_listen = compower_idle_activity.listen;

 	cpu = all_cpu - last_cpu;
 	lpm = all_lpm - last_lpm;
 	transmit = all_transmit - last_transmit;
 	listen = all_listen - last_listen;
 	idle_transmit = compower_idle_activity.transmit - last_idle_transmit;
 	idle_listen = compower_idle_activity.listen - last_idle_listen;

 	last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
 	last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
 	last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
 	last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);
 	last_idle_listen = compower_idle_activity.listen;
 	last_idle_transmit = compower_idle_activity.transmit;

 	radio = transmit + listen;
 	time = cpu + lpm;
 	all_time = all_cpu + all_lpm;
 	all_radio = energest_type_time(ENERGEST_TYPE_LISTEN) +
 	energest_type_time(ENERGEST_TYPE_TRANSMIT);

 	printf("%s,%lu,%d.%d,%lu,%lu,%lu,%lu %lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%d.%02d%%,%d.%02d%%,%d.%02d%%,%d.%02d%%,%d.%02d%%,%d.%02d%%\n",
 		str,
 		clock_time(), linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1], seqno,
 		all_cpu, all_lpm, all_transmit, all_listen, all_idle_transmit, all_idle_listen,
 		cpu, lpm, transmit, listen, idle_transmit, idle_listen,
 		(int)((100L * (all_transmit + all_listen)) / all_time),
 		(int)((10000L * (all_transmit + all_listen) / all_time) - (100L * (all_transmit + all_listen) / all_time) * 100),
 		(int)((100L * (transmit + listen)) / time),
 		(int)((10000L * (transmit + listen) / time) - (100L * (transmit + listen) / time) * 100),
 		(int)((100L * all_transmit) / all_time),
 		(int)((10000L * all_transmit) / all_time - (100L * all_transmit / all_time) * 100),
 		(int)((100L * transmit) / time),
 		(int)((10000L * transmit) / time - (100L * transmit / time) * 100),
 		(int)((100L * all_listen) / all_time),
 		(int)((10000L * all_listen) / all_time - (100L * all_listen / all_time) * 100),
 		(int)((100L * listen) / time),
 		(int)((10000L * listen) / time - (100L * listen / time) * 100));
 	seqno++;
 }

 int t1 =0;
 int t2 =0;

/*---------------------------------------------------------------------------*/
 float
 floor(float x)
 {
 	if(x >= 0.0f) {
 		return (float)((int)x);
 	} else {
 		return (float)((int)x - 1);
 	}
 }
/*---------------------------------------------------------------------------*/
 PROCESS(test_battery_process, "Sensor Energy & Multiple Test");
 AUTOSTART_PROCESSES(&test_battery_process);
/*---------------------------------------------------------------------------*/
 PROCESS_THREAD(test_battery_process, ev, data)
 {

 	PROCESS_BEGIN();
 	//powertrace_start(CLOCK_SECOND * 5);
 	SENSORS_ACTIVATE(battery_sensor);

 	while(1) {
 		powertrace_print("powertrace results");

 		t1 = clock_time();
 		uint16_t bateria = battery_sensor.value(0);
 		float mv = (bateria * 2.500 * 2) / 4096;
 		printf("Battery: %i (%ld.%03d mV)\n", bateria, (long)mv,
 			(unsigned)((mv - floor(mv)) * 1000));
 		t2 = clock_time();
 		printf("Time Difference: %u\n",( (t2-t1) * 1000 )/( 2*128 ));
 		printf("get tx power: %u\n", cc2420_get_txpower());
 		printf("tstamp=0x%0x\n",packetbuf_attr(PACKETBUF_ATTR_TIMESTAMP));
 	}

 	SENSORS_DEACTIVATE(battery_sensor);

 	PROCESS_END();
 }
/*---------------------------------------------------------------------------*/