#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "dev/adxl345.h"


/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */

extern resource_t res_z1_coap_obs_rtgs;
extern resource_t res_z1_coap_sens_rtgs;

PROCESS(er_example_server, "RTGS Server");
AUTOSTART_PROCESSES(&er_example_server);
PROCESS_THREAD(er_example_server, ev, data)
{
  PROCESS_BEGIN();

  //----------- Init ADXL Sensor ------------
  /* Start and setup the accelerometer with default values, eg no interrupts enabled. */
  accm_init();

  /* Set what strikes the corresponding interrupts. Several interrupts per pin is
     possible. For the eight possible interrupts, see adxl345.h and adxl345 datasheet. */
  accm_set_irq(ADXL345_INT_FREEFALL, ADXL345_INT_TAP + ADXL345_INT_DOUBLETAP);
  //-----------End Init ADXL Sensor ------------
  PROCESS_PAUSE();


  /* Initialize the REST engine. */
  rest_init_engine();

  rest_activate_resource(&res_z1_coap_obs_rtgs, "obs/mote");
  rest_activate_resource(&res_z1_coap_sens_rtgs, "sens/mote");
  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();

  }  /* while (1) */

  PROCESS_END();
}
