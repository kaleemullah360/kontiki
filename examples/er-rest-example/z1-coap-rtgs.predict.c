#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"
#include <cc2420-radio.h>

#include <string.h>
#include "er-coap.h"

//------- prediction custom libs ------
#include "dev/adxl345.h"
#include <math.h>
#include "dev/leds.h"
//------- End prediction custom libs ------

//--- Libs for rTGS-APp ----
#include "dev/battery-sensor.h"
#include "dev/i2cmaster.h"
#include "dev/tmp102.h"
//---End Libs for rTGS-APp ---

//--- Function Deffinitions for rTGS-APp ----
uint8_t mid = 0;
int16_t temp;
uint16_t bateria = 0;
float mv = 0.0;

float
floor_bat(float x)
{
  if(x >= 0.0f) {
    return (float)((int)x);
  } else {
    return (float)((int)x - 1);
  }
}
//---End Function Deffinitions rTGS-APp ---

//------- prediction functions ------
// set the sensor reading value interval
#define ACCM_READ_INTERVAL    CLOCK_SECOND/50

// if you want "STANDING" instead of "1" just set it to 1
#define CHAR_STR_STATUS 1 
#if CHAR_STR_STATUS
  static char *STANDING   = "STANDING";  //STANDING
  static char *WALKING    = "WALKING";   //WALKING
  static char *RUNNING    = "RUNNING";   //RUNNING
  static char *FALLING    = "FALLING";   //FALLING
  static char *STATUS_PT  =  NULL;       //Nothing
  static char last;
#else
  static char *STANDING   = "1"; //STANDING
  static char *WALKING    = "2"; //WALKING
  static char *RUNNING    = "3"; //RUNNING
  static char *FALLING    = "4"; //FALLING
  static char *STATUS_PT  = NULL;//Nothing
  static char last;
#endif

// declare/define the pridiction function.
void predict();
void notify();
//  viola ! these are actions to be fired on each event. 
//  i.e set status 1, print walking, turn on blue LED and off other LEDs when WALKING is fired.
void standing(){ notify(); leds_on(LEDS_BLUE);  leds_off(LEDS_RED); leds_off(LEDS_GREEN); }
void walking() { notify(); leds_on(LEDS_GREEN); leds_off(LEDS_RED); leds_off(LEDS_BLUE);  }
void running() { notify(); leds_on(LEDS_GREEN); leds_on(LEDS_RED);  leds_off(LEDS_BLUE);  }
void falling() { notify(); leds_on(LEDS_RED);   leds_off(LEDS_BLUE);leds_off(LEDS_GREEN); }

#define HISTORY 16
#define sampleNo 41
// Why I'm using int16_t ? (finally I discovered, I don't know)
/* c Type   |stdint.h Type|Bits|Signed| Range           |
 signed char| int16_t     | 16 |Signed|-32,768 .. 32,767| */

static int16_t x,y,z;
static int16_t sample[3][sampleNo];
static int16_t pos;

static struct etimer et;

/* accelerometer free fall detection callback */
void accm_ff_cb(uint8_t reg){
#define ANNOYING_ALWAYS_THERE_ANYWAY_OUTPUT 0
  if(reg && ADXL345_INT_FREEFALL){last=*STATUS_PT;STATUS_PT=FALLING;if(last!=*STATUS_PT){ falling(); }}
}

//-------End prediction functions ------
/*---------------------------------------------------------------------------*/


extern resource_t res_z1_coap_rtgs_obs_moves;

PROCESS_NAME(motion_tracking_process);  
PROCESS_NAME(er_example_server);  

PROCESS(er_example_server, "RTGS Server");
PROCESS(motion_tracking_process, "Motion Tracker");

AUTOSTART_PROCESSES(&er_example_server, &motion_tracking_process);
PROCESS_THREAD(er_example_server, ev, data)
{
  PROCESS_BEGIN();
	set_cc2420_txpower(0);
	set_cc2420_channel(0);
	SENSORS_ACTIVATE(battery_sensor);
	tmp102_init();
  PROCESS_PAUSE();
  /* Initialize the REST engine. */
  rest_init_engine();

  rest_activate_resource(&res_z1_coap_rtgs_obs_moves, "obs/moves");

  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();

  }  /* while (1) */

	SENSORS_DEACTIVATE(battery_sensor);
  PROCESS_END();
}

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);

PERIODIC_RESOURCE(res_z1_coap_rtgs_obs_moves,
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
  REST.set_header_max_age(response, res_z1_coap_rtgs_obs_moves.periodic->period / CLOCK_SECOND);
	//  MessageID, UpTime, ClockTime, Temperature, Battery, Status, RTT  //<-- This
  REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "%d,%lu,1,%d,%ld.%03d,%s", mid++ ,clock_seconds(), temp, (long)mv,(unsigned)((mv - floor_bat(mv)) * 1000), STATUS_PT));

}
void notify() {
    printf("FinalStatus: %s\n", STATUS_PT);
    REST.notify_subscribers(&res_z1_coap_rtgs_obs_moves);
}
static void
res_periodic_handler()
{

}

PROCESS_THREAD(motion_tracking_process, ev, data){
  PROCESS_BEGIN();

  //----------- Init ADXL Sensor ------------
  /* Start and setup the accelerometer with default values, eg no interrupts enabled. */
  accm_init();

  /* Set what strikes the corresponding interrupts. Several interrupts per pin is
     possible. For the eight possible interrupts, see adxl345.h and adxl345 datasheet. */
  accm_set_irq(ADXL345_INT_FREEFALL, ADXL345_INT_TAP + ADXL345_INT_DOUBLETAP);
  ACCM_REGISTER_INT1_CB(accm_ff_cb);
  //-----------End Init ADXL Sensor ------------

  printf("Motion Tracking Started\n");

  while(1){
    //----- Get Data Instance -------
		temp = tmp102_read_temp_x100()/100;
    bateria = battery_sensor.value(0);
    mv = (bateria * 2.500 * 2) / 4096;
    //----- End Get Data -------
    //------------ Prediction (read values) ------------------
    x = accm_read_axis(X_AXIS);
    y = accm_read_axis(Y_AXIS);
    z = accm_read_axis(Z_AXIS);

    //printf("%d : %d : %d \n", x, y, z);
    sample[0][pos]=(int16_t)x;
    sample[1][pos]=(int16_t)y;
    sample[2][pos]=(int16_t)z;

    if(pos==(sampleNo-1))predict();
    pos=(pos+1)%sampleNo;
    etimer_set(&et, ACCM_READ_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    if(STATUS_PT==FALLING){
      etimer_set(&et, 15);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));}
    //------------ End Prediction (read values) ------------------
    //printf("FinalStatus: %s\n", status_str);

  }
  printf("Motion Tracking Terminated\n");

  PROCESS_END();
}

/*--- - Human Body Posture Detection and Prediction ---------------*/
void predict(){
  double mag[sampleNo-1],sum=0,max=0,std=0,mean=0,tmp=0;
  double result1,result2,result3,result4;
  int i;
  for(i=1;i<sampleNo;i++){
    mag[i-1]=sqrt(pow(sample[0][i]-sample[0][i-1],2)+pow(sample[1][i]-sample[1][i-1],2)+pow(sample[2][i]-sample[2][i-1],2));
    sum=sum+mag[i-1];
    if(mag[i-1]>max)max=mag[i-1];
  }

  mean =(sum/sampleNo);
  for(i=0; i<sampleNo-1; i++){ tmp = tmp + pow( ( 1 - mean ), 2); }
  std=sqrt(tmp/40);
  //printf("This is data from predict:%d,%d\n",(int)std,(int)mean);
  if(std>1000)return;
  double std1=3, mean1=2.2, max1=0.7;
  double std2=30, mean2=37, max2=5.74;
  double std3=68, mean3=83, max3=22;
  double std4=96, mean4=34, max4=42;
  double a1=1,a2=1,a3=0;
  result1 = sqrt(a1*pow((std-std1),2)+a2*pow((mean-mean1),2)+a3*pow((max-max1),2));
  result2 = sqrt(a1*pow((std-std2),2)+a2*pow((mean-mean2),2)+a3*pow((max-max2),2));
  result3 = sqrt(a1*pow((std-std3),2)+a2*pow((mean-mean3),2)+a3*pow((max-max3),2));
  result4 = sqrt(a1*pow((std-std4),2)+a2*pow((mean-mean4),2)+a3*pow((max-max4),2));
  //printf("This is four results:%d,%d,%d,%d\n",(int)result1,(int)result2,(int)result3,(int)result4);
  if(result1<result2 && result1<result3 && result1<result4){ last=*STATUS_PT;STATUS_PT=STANDING; if(last!=*STATUS_PT){ standing();} }
  if(result2<result1 && result2<result3 && result2<result4){ last=*STATUS_PT;STATUS_PT=WALKING;  if(last!=*STATUS_PT){ walking(); } }
  if(result3<result1 && result3<result2 && result3<result4){ last=*STATUS_PT;STATUS_PT=RUNNING;  if(last!=*STATUS_PT){ running(); } }
}
/*---- End Human Body Posture Detection and Prediction ---------------*/
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
