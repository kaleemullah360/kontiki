/*
 */

#include "contiki.h"
#include <stdio.h>		/* For printf() */
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-debug.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

#include "contiki-net.h"
#include "erbium.h"
#include "er-coap-13.h"
#include <string.h>
#include "dev/adxl345.h"

#define ACCM_READ_INTERVAL    CLOCK_SECOND
#define ALARM_DURATION	      CLOCK_SECOND << 1

/*---------------------------------------------------------------------------*/

static int alarm = 0;

void resource1_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {
	uint8_t length;
	uint8_t method = REST.get_method_type(request);
	if(method & METHOD_GET) {
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON); 
		REST.set_header_etag(response, (uint8_t *) &length, 1);
		REST.set_response_status(response, REST.status.OK);
		leds_on(LEDS_BLUE);
	} else
		REST.set_response_status(response, REST.status.BAD_REQUEST);
}

void resource1_event_handler(resource_t *r) {
	static int event_counter;
	uint8_t length;
	coap_packet_t notification[1];
	coap_init_message(notification, COAP_TYPE_NON, REST.status.OK, 0);
	char buf[32];
	sprintf(buf, "{\"alarm\": %s}", alarm? "true" : "false");
	length = strlen(buf);
	coap_set_payload(notification, buf, length);
	REST.set_header_content_type(notification, REST.type.APPLICATION_JSON);
	REST.notify_subscribers(r, event_counter++, notification);
}

EVENT_RESOURCE(resource1, METHOD_GET, "alarm", "title=\"alarm resource\";rt=\"Text\";obs");
PROCESS(coap_server_process, "CoAP server process with observable event resource");
PROCESS(alarmon_process, "Turns led on after a while");
PROCESS(alarmoff_process, "Turns led off after a while");
AUTOSTART_PROCESSES(&coap_server_process, &alarmon_process, &alarmoff_process);

/*---------------------------------------------------------------------------*/

static process_event_t ledoff_event;
static process_event_t alarm_event;

void accelerometer_callback(uint8_t reg) {
    process_post(&alarmon_process, alarm_event, NULL);
}

PROCESS_THREAD(coap_server_process, ev, data)
{

  PROCESS_BEGIN();
  static uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;
  static struct etimer timer;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF); 

  rest_init_engine();
  rest_activate_event_resource(&resource_resource1);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE 
       || state == ADDR_PREFERRED)) {
            uip_debug_ipaddr_print(
               &uip_ds6_if.addr_list[i].ipaddr);
            printf("\n");
    }
  }

  ledoff_event = process_alloc_event();
  alarm_event = process_alloc_event();

  accm_init();

  /* Register the callback functions for each interrupt */
  ACCM_REGISTER_INT1_CB(accelerometer_callback);
  ACCM_REGISTER_INT2_CB(accelerometer_callback);

  /* Set what strikes the corresponding interrupts. Several interrupts per pin is 
     possible. For the eight possible interrupts, see adxl345.h and adxl345 datasheet. */
  accm_write_reg(ADXL345_THRESH_ACT, 0x01);
  accm_write_reg(ADXL345_ACT_INACT_CTL, 0xFF);
  accm_set_irq(ADXL345_INT_ACTIVITY | ADXL345_INT_FREEFALL | ADXL345_INT_TAP | ADXL345_INT_DOUBLETAP, ADXL345_INT_DISABLE);
  printf("CoAP event observable server started\n");
  etimer_set(&timer, CLOCK_SECOND >> 1);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_restart(&timer);
    leds_toggle(LEDS_GREEN);
  }
  PROCESS_END();
}


PROCESS_THREAD(alarmon_process, ev, data) {
  PROCESS_BEGIN();
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev == alarm_event);
    if(!alarm) {
      leds_on(LEDS_RED);
      alarm = 1;
      process_post(&alarmoff_process, ledoff_event, NULL);
      printf("[%u] activity detected -- \n", ((uint16_t) clock_time())/128);
      resource1_event_handler(&resource_resource1);
    } else 
      printf("*** alarm already notified\n");
  }
  PROCESS_END();
}

PROCESS_THREAD(alarmoff_process, ev, data) {
  static struct etimer ledETimer;
  PROCESS_BEGIN();
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev == ledoff_event);
    etimer_set(&ledETimer, ALARM_DURATION);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&ledETimer));
    leds_off(LEDS_RED);
    alarm = 0;
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
