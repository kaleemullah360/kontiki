/*
* \file
*         Motion Detector with Zolertia Z1 using CoAP Application layer protocol.
          Erbium (Er) REST Engine example.
* \author
*         Kaleem Ullah    <MSCS14059@ITU.EDU.PK>
*
* \Short Description:
* 
*    Track Motion eg. Walking, Standing, Falling, Running
* this application uses Zolertia adxl345 sensor and CoAP protocol on Application layer.
* The sensor produced x, y, z axis values upone actuation.
* then using these values a predict(); function output state of the sensor node.
*/

/*
   <<<<<<<<<<<<<< Steps to perform: >>>>>>>>>>>>>>>>

1.  Add the following line in file " ~/contiki/platform/z1/platform-conf.h "
[ #define PLATFORM_HAS_ZSADXL_THB 1 ]

2.  Copy the following file into " ~/contiki/examples/er-rest-example/resources "
  [ res-z1-coap-rtgs.c ]

3.  Downlaod and install following Addon for firefox
" https://addons.mozilla.org/en-US/firefox/addon/copper-270430/ "

4. Compile/Burn Application then connect-border-router
  [ coap-motion-tracker.c ]

5.  Launch the Firefox Browser and Hit the following URL for simulating the browser to automatically get status use the below function
    " coap://[aaaa::c30c:0:0:2]:5683/sensor/tracker "
*/

/*  
    <<<<<<<<<<<<<< Function to simulate Browser. >>>>>>>>>>>>>>>>

    index = 0;  // message offset
    limit = 1000; // total messages to get
    miliseconds = 2000; //message request delay in milliseconds
    window.setInterval(function () {
      if (index <= limit) {
        var timeStampMilliSeconds = 0;  // set Time
        msg_id = $('#packet_header_tid').getAttribute('label'); // CoAP Received Message ID
        if (msg_id != '') {
          timeStampMilliSeconds = Math.floor(Date.now()); // if message is received and has an ID
        }
        pay_load = $('#info_payload').getAttribute('label');  // get the payload size
        temp = $('#packet_payload').value;    // get payload value
        $('#toolbar_get').click();  // click the GET button after getting previouse values
        // print all the data to console
        console.log('RespTime(mSec) ' + timeStampMilliSeconds + ' Message Number ' + index + ' Data ' + temp + ' Message ID ' + msg_id + ' ' + pay_load);
      } else {
        return; // if all messages received then terminate
      }
      index++;  // increment by 1 (one)
    }, miliseconds);  // message request interval

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include "dev/adxl345.h"

#if PLATFORM_HAS_BUTTON
#include "dev/button-sensor.h"
#endif

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
 extern resource_t
 res_mirror,
 res_chunks,
 res_separate,
 res_event,
 res_sub,
 res_b1_sep_b2;

#if PLATFORM_HAS_ZSADXL_THB
extern resource_t res_z1_coap_rtgs; // <--- This
extern resource_t res_z1_coap_obs_rtgs; // <--- This
#endif


PROCESS(er_example_server, "Erbium Example Server");
AUTOSTART_PROCESSES(&er_example_server);

PROCESS_THREAD(er_example_server, ev, data)
{
  PROCESS_BEGIN();


/* Start and setup the accelerometer with default values, eg no interrupts enabled. */
  accm_init();

  /* Set what strikes the corresponding interrupts. Several interrupts per pin is
    possible. For the eight possible interrupts, see adxl345.h and adxl345 datasheet. */
  accm_set_irq(ADXL345_INT_FREEFALL, ADXL345_INT_TAP + ADXL345_INT_DOUBLETAP);


  PROCESS_PAUSE();

  PRINTF("Starting Erbium Example Server\n");

#ifdef RF_CHANNEL
  PRINTF("RF channel: %u\n", RF_CHANNEL);
#endif
#ifdef IEEE802154_PANID
  PRINTF("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif

  PRINTF("uIP buffer: %u\n", UIP_BUFSIZE);
  PRINTF("LL header: %u\n", UIP_LLH_LEN);
  PRINTF("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
  PRINTF("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);

  /* Initialize the REST engine. */
  rest_init_engine();

  /*
   * Bind the resources to their Uri-Path.
   * WARNING: Activating twice only means alternate path, not two instances!
   * All static variables are the same for each URI path.
   */

#if PLATFORM_HAS_ZSADXL_THB   // <--- This
   rest_activate_resource(&res_z1_coap_rtgs, "sensor/monitor");
   rest_activate_resource(&res_z1_coap_obs_rtgs, "sensor/obsmonitor");
#endif

  /* Define application-specific events here. */
   while(1) {
    PROCESS_WAIT_EVENT();
    #if PLATFORM_HAS_BUTTON
    if(ev == sensors_event && data == &button_sensor) {
      PRINTF("*******BUTTON*******\n");

      /* Call the event_handler for this application-specific event. */
      res_event.trigger();

      /* Also call the separate response example handler. */
      res_separate.resume();
    }
#endif /* PLATFORM_HAS_BUTTON */
  }                             /* while (1) */

    PROCESS_END();
  }
