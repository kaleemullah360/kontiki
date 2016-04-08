#include "contiki.h"

#if PLATFORM_HAS_BATTERY

#include <string.h>
#include "rest-engine.h"

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

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple getter example. Returns the reading from light sensor with a simple etag */
RESOURCE(res_z1_coap_rtgs,
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
  upt = clock_seconds();  // UpTime
  clk = clock_time(); // ClockTime
  tem = stmp();  // Temperature
  bat_v = sbat_v(); // Get Battery in Volts
  bat_mv = sbat_mv(bat_v);  //Get Battery in MilliVolts
//----- End Get Data -------

  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);

    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  //  MessageID, UpTime, ClockTime, Temperature, Battery  //<-- This
  // "%lu,%lu,%lu,%ld.%03d,%ld.%03d", mid,upt,clk,tem.tem,bat.bat
  //  snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%lu,%lu,%lu,%ld.%03d,%ld.%03d", mid,upt,clk,(long)tem,(unsigned)((tem - floor(tem)) * 1000),(long)bat_mv,(unsigned)((bat_mv - floor(bat_mv)) * 1000));
  // "%lu,%lu,%lu,%ld.%03d,%ld.%03d", mid,upt,clk,tem,bat
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%lu,%lu,%lu,%ld,%03d", mid,upt,clk,(long)tem,(unsigned)((bat_mv - floor(bat_mv)) * 1000));

    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));

}
#endif /* PLATFORM_HAS_BATTERY */
