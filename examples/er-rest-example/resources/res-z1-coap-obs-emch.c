
#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"

//--- Libs for E-MCH-APP ----
#include "dev/battery-sensor.h"
#include "dev/i2cmaster.h"
#include "dev/tmp102.h"
//---End Libs for E-MCH-APP ---


static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);

PERIODIC_RESOURCE(res_z1_coap_obs_emch,
  "title=\"Periodic demo\";obs",
  res_get_handler,
  NULL,
  NULL,
  NULL,
  5 * CLOCK_SECOND,
  res_periodic_handler);

/*
 * Use local resource state that is accessed by res_get_handler() and altered by res_periodic_handler() or PUT or POST.
 */
 
 static int32_t mid = 0;  // MessageID
 static int32_t upt = 0;  // UpTime
 static int32_t clk = 0;  // ClockTime
 static uint8_t tem = 0;  // Temperature
 static uint8_t bat = 0;  // Battery

 static uint8_t ttem = 0;  // temporary Temperature
 static void
 res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
 {
  /*
   * For minimal complexity, request query and options should be ignored for GET on observable resources.
   * Otherwise the requests must be stored with the observer list and passed by REST.notify_subscribers().
   * This would be a TODO in the corresponding files in contiki/apps/erbium/!
   */

   REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
   REST.set_header_max_age(response, res_z1_coap_obs_emch.periodic->period / CLOCK_SECOND);
  //  MessageID, UpTime, ClockTime, Temperature, Battery  //<-- This
  // "%lu,%lu,%lu,%u,%u", mid,upt,clk,tem,bat
   REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%lu,%lu,%lu,%u,%u", mid,upt,clk,tem,bat));

  /* The REST.subscription_handler() will be called for observable resources by the REST framework. */
 }
/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
 static void
 res_periodic_handler()
 {	
//----- Get Data Instance -------
  tmp102_init();  // Init Sensor
  ++mid;  // MessageID
  upt = clock_seconds();  // UpTime
  clk = clock_time(); // ClockTime
  tem = tmp102_read_temp_x100();  // Temperature
  bat = battery_sensor.value(0);  // Battery
//----- End Get Data -------

 /* Do a periodic task here, e.g., sampling a sensor. */
  /* Usually a condition is defined under with subscribers are notified, e.g., large enough delta in sensor reading. */
  if(ttem != bat) {
    ttem = bat;   // update the temporary valruable with fresh value
    /* Notify the registered observers which will trigger the res_get_handler to create the response. */
    REST.notify_subscribers(&res_z1_coap_obs_emch);
  }
}
