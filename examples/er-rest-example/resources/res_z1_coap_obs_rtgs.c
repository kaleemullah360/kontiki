
#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"

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
static uint8_t ttem = 0;  // temporary Temperature

float obsfloor(float x){
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

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);

PERIODIC_RESOURCE(res_z1_coap_obs_rtgs,
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

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /*
   * For minimal complexity, request query and options should be ignored for GET on observable resources.
   * Otherwise the requests must be stored with the observer list and passed by REST.notify_subscribers().
   * This would be a TODO in the corresponding files in contiki/apps/erbium/!
   */

   REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
   REST.set_header_max_age(response, res_z1_coap_obs_rtgs.periodic->period / CLOCK_SECOND);
  //  MessageID, UpTime, ClockTime, Temperature, Battery  //<-- This
  // "%lu,%lu,%lu,%ld.%03d,%ld.%03d", mid,upt,clk,tem.tem,bat.bat
  //  snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%lu,%lu,%lu,%ld.%03d,%ld.%03d", mid,upt,clk,(long)tem,(unsigned)((tem - obsfloor(tem)) * 1000),(long)bat_mv,(unsigned)((bat_mv - obsfloor(bat_mv)) * 1000));
  // "%lu,%lu,%lu,%ld.%03d,%ld.%03d", mid,upt,clk,tem,bat
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%lu,%lu,%lu,%ld,%03d", mid,upt,clk,(long)tem,(unsigned)((bat_mv - obsfloor(bat_mv)) * 1000));

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
  ++mid;  // MessageID
  upt = clock_seconds();  // UpTime
  clk = clock_time(); // ClockTime
  tem = stmp();  // Temperature
  bat_v = sbat_v(); // Get Battery in Volts
  bat_mv = sbat_mv(bat_v);  //Get Battery in MilliVolts
//----- End Get Data -------

 /* Do a periodic task here, e.g., sampling a sensor. */
  /* Usually a condition is defined under with subscribers are notified, e.g., large enough delta in sensor reading. */
  if(ttem != bat_v) {
    ttem = bat_v;   // update the temporary valruable with fresh value
    /* Notify the registered observers which will trigger the res_get_handler to create the response. */
    REST.notify_subscribers(&res_z1_coap_obs_rtgs);
  }
}
