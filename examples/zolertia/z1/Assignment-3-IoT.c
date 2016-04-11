
#include "contiki.h"  //contiki header library 

// temperature sensor header
#include "dev/i2cmaster.h"
#include "dev/tmp102.h"


// battery level driver 
// It activates the sensor and prints as fast as possible (with no delay) the battery level.
#include "dev/battery-sensor.h" 

#include "stdio.h"  // c standard i/o lib

// defines for temperature sensor function
#if 1
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#if 0
#define PRINTFDEBUG(...) printf(__VA_ARGS__)
#else
#define PRINTFDEBUG(...)
#endif


#define TMP102_READ_INTERVAL (CLOCK_SECOND/2)


//================================================================
//      << Battery & Temperature Sensor Status in Z1 mote >>
//      
//      There is one analog sensor in the Z1, 
//      and it provides the battery level expressed in milliVolts.
//      also it has temperature sensor which can record temperature
//      Developed by Kaleem Ullah MSCS14059
//      Special Thanks to: Antonio Lignan [alignan] & Valentin [vsaw]
//=================================================================

// function to return floor of float value
float floor(float x){
  if(x >= 0.0f){ // check the value of x is +eve
    return (float)((int) x);
  }else{ // if value of x is -eve
    // x = -2.2
    // -3.2 = (-2.2) - 1
    // -3  = (int)(-3.2)
    //return -3.0 = (float)(-3)
    return(float) ((int) x - 1);   
  } //end if-else

} //end floor function

// cretae single process for both sensors
PROCESS(test_battery_and_temperature_process, "Battery Sensor And Temperature Test");
AUTOSTART_PROCESSES(&test_battery_and_temperature_process);


// create a thread for the function
PROCESS_THREAD(test_battery_and_temperature_process, ev, data)
{
  // start thread process
  PROCESS_BEGIN();
  
 // temperature function variables 
  int16_t tempint;
  uint16_t tempfrac;
  int16_t raw;
  uint16_t absraw;
  int16_t sign;
  char minus = ' ';

  tmp102_init();

  

  // switch on battery sensor
  SENSORS_ACTIVATE(battery_sensor);
  // loop will continue for ever
  while(1) {
   // if uncomments will not let the battery level function to be execute
   //  etimer_set(&et, TMP102_READ_INTERVAL);
   // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    sign = 1;

    // prints as fast as possible (with no delay) the battery level.
    uint16_t bateria = battery_sensor.value(0);
    // When working with the ADC you need to convert the ADC integers in milliVolts. 
    // This is done with the following formula:
    float mv = (bateria * 2.500 * 2) / 4096;
    printf("Battery Analog Data Value: %i , milli Volt= (%ld.%03d mV)\n", bateria, (long) mv, (unsigned) ((mv - floor(mv)) * 1000));
    
  
    // Temperature Function
    PRINTFDEBUG("Reading Temp...\n");
    raw = tmp102_read_temp_x100(); // tmp102_read_temp_raw();
    absraw = raw;
    if(raw < 0) {   // Perform 2C's if sensor returned negative data
      absraw = (raw ^ 0xFFFF) + 1;
      sign = -1;
    }
    tempint = (absraw >> 8) * sign;
    tempfrac = ((absraw >> 4) % 16) * 625;  // Info in 1/10000 of degree
    minus = ((tempint == 0) & (sign == -1)) ? '-' : ' ';
    PRINTF("Temp = %c%d.%04d\n", minus, tempint, tempfrac);


          }
  // switch off battery sensor
  SENSORS_DEACTIVATE(battery_sensor);
  
  // end thread process
  PROCESS_END();
}

