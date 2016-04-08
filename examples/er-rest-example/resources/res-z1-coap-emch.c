#include "contiki.h"

#if PLATFORM_HAS_BATTERY

#include <string.h>
#include "rest-engine.h"

//--- Libs for E-MCH-APP ----
#include "dev/battery-sensor.h"
#include "dev/i2cmaster.h"
#include "dev/tmp102.h"
//---End Libs for E-MCH-APP ---

 static int32_t mid = 0;  // MessageID
 static int32_t upt = 0;  // UpTime
 static int32_t clk = 0;  // ClockTime
 static uint8_t tem = 0;  // Temperature
 static uint8_t bat = 0;  // Battery

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple getter example. Returns the reading from light sensor with a simple etag */
RESOURCE(res_z1_coap_emch,
         "title=\"Sensor\";rt=\"status\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  //----- Get Data Instance -------
  tmp102_init();  // Init Sensor
  ++mid;  // MessageID
  upt = clock_seconds();  // UpTime
  clk = clock_time(); // ClockTime
  tem = tmp102_read_temp_x100();  // Temperature
  bat = battery_sensor.value(0);  // Battery
//----- End Get Data -------

  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);

    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
  //  MessageID, UpTime, ClockTime, Temperature, Battery  //<-- This
  // "%lu,%lu,%lu,%u,%u", mid,upt,clk,tem,bat
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,"%lu,%lu,%lu,%u,%u", mid,upt,clk,tem,bat);

    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));

}
#endif /* PLATFORM_HAS_BATTERY */
