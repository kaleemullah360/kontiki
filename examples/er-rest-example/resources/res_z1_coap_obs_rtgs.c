
#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"
//--- Libs for rTGS ----

#include "dev/battery-sensor.h"
#include "dev/i2cmaster.h"
#include "dev/tmp102.h"

//---End Libs for rTGS ---

//--- Variable Declaration for rTGS ----

 static int32_t mid = 0;  // MessageID
 static int32_t upt = 0;  // UpTime
 static int32_t clk = 0;  // ClockTime

  // temperature function variables 
 static int16_t tempint;
 static uint16_t tempfrac;
 static int16_t raw;
 static uint16_t absraw;
 static int16_t sign;
 static char minus = ' ';

  // Battery function variables 
 static uint16_t bat_v = 0;
 static float bat_mv = 0; 

//---End Variable Declaration rTGS ---

//--- Function Deffinitions for rTGS ----

// function to return obs_floor of float value
 float obs_floor(float x){
  if(x >= 0.0f){ // check the value of x is +eve
    return (float)((int) x);
  }else{ // if value of x is -eve
    // x = -2.2
    // -3.2 = (-2.2) - 1
    // -3  = (int)(-3.2)
    //return -3.0 = (float)(-3)
    return(float) ((int) x - 1);   
  } //end if-else

} //end obs_floor function

static void get_sensor_time(){
  upt = clock_seconds();  // UpTime
  clk = clock_time(); // ClockTime
}

static void get_sensor_temperature(){
  tmp102_init();  // Init Sensor
  sign = 1;
  raw = tmp102_read_temp_x100(); // tmp102_read_temp_raw();
  absraw = raw;
    if(raw < 0) {   // Perform 2C's if sensor returned negative data
      absraw = (raw ^ 0xFFFF) + 1;
    sign = -1;
  }
  tempint = (absraw >> 8) * sign;
    tempfrac = ((absraw >> 4) % 16) * 625;  // Info in 1/10000 of degree
    minus = ((tempint == 0) & (sign == -1)) ? '-' : ' ';
    //printf("Temp = %c%d.%04d\n", minus, tempint, tempfrac);
  }

  static void get_sensor_battery(){
  // Activate Temperature and Battery Sensors  
    SENSORS_ACTIVATE(battery_sensor);
  // prints as fast as possible (with no delay) the battery level.
    bat_v = battery_sensor.value(0);
  // When working with the ADC you need to convert the ADC integers in milliVolts. 
  // This is done with the following formula:
    bat_mv = (bat_v * 2.500 * 2) / 4096;
  //printf("Battery Analog Data Value: %i , milli Volt= (%ld.%03d mV)\n", bat_v, (long) bat_mv, (unsigned) ((bat_mv - obs_floor(bat_mv)) * 1000));
  }

//---End Function Deffinitions rTGS ---

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
   REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%lu,%lu,%lu,%c%d.%04d,%ld.%03d", mid, upt, clk, minus,tempint,tempfrac, (long) bat_mv, (unsigned) ((bat_mv - obs_floor(bat_mv)) * 1000)));

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
get_sensor_temperature();
get_sensor_time();
get_sensor_battery();
//----- End Get Data -------

 /* Do a periodic task here, e.g., sampling a sensor. */
  /* Usually a condition is defined under with subscribers are notified, e.g., large enough delta in sensor reading. */
if(1) {
    // update the temporary valruable with fresh value
    /* Notify the registered observers which will trigger the res_get_handler to create the response. */
  REST.notify_subscribers(&res_z1_coap_obs_rtgs);
}
}
