/*
 * \file
 *         CoAP Application layer protocol Motion Detector with Zolertia Z1 example.
 * \author
 *         Kaleem Ullah    <MSCS14059@ITU.EDU.PK>
 *
 * \Short Description:
 *
 *    Track Motion eg. Walking, Standing, Falling, Running, Battery Sensing and Temperature monitoring
 * This application uses Zolertia adxl345 sensor. and CoAP Protocol at Application layer
 * The sensor produced x, y, z axis values upone actuation.
 * Observe resources and on status change it notify the subscriber
 *
 * then using these values a predict(); function output state of the sensor node.
 * Send Battery values and Temperature.
 *
 */
#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include <cc2420-radio.h>

#include <string.h>
#include "er-coap.h"



static struct etimer et;

/*---------------------------------------------------------------------------*/
extern resource_t res_z1_coap_rtgs_obs_rssi;

PROCESS_NAME(rssi_lqi_process);  
PROCESS_NAME(er_example_server);  

PROCESS(er_example_server, "RTGS Server");
PROCESS(rssi_lqi_process, "RSSI LQI");

AUTOSTART_PROCESSES(&er_example_server, &rssi_lqi_process);
PROCESS_THREAD(er_example_server, ev, data)
{
  PROCESS_BEGIN();
	set_cc2420_txpower(0);
	set_cc2420_channel(0);

  PROCESS_PAUSE();
  /* Initialize the REST engine. */
  rest_init_engine();

  rest_activate_resource(&res_z1_coap_rtgs_obs_rssi, "obs/rssi");

  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();

  }  /* while (1) */

  PROCESS_END();
}

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);

PERIODIC_RESOURCE(res_z1_coap_rtgs_obs_rssi,
                  "title=\"rTGS\";obs",
                  res_get_handler,
                  NULL,
                  NULL,
                  NULL,
                  5 * CLOCK_SECOND,
                  res_periodic_handler);


static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  REST.set_header_max_age(response, res_z1_coap_rtgs_obs_rssi.periodic->period / CLOCK_SECOND);
  REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "RSSI: %d LQI: %d\n", cc2420_last_rssi, cc2420_last_correlation));

}
void notify() {
    //printf("FinalStatus: %s\n", STATUS_PT);
    REST.notify_subscribers(&res_z1_coap_rtgs_obs_rssi);
}
static void
res_periodic_handler()
{
    // do periodic task here.
    // this method is called each after 5 seconds
}

PROCESS_THREAD(rssi_lqi_process, ev, data){
  PROCESS_BEGIN();

  while(1){
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }

  PROCESS_END();
}