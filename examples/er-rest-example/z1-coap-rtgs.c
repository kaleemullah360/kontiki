#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"

//------- prediction custom libs ------
#include "dev/adxl345.h"
#include <math.h>
//------- End prediction custom libs ------

//------- prediction functions ------
// set the sensor reading value interval
#define ACCM_READ_INTERVAL    CLOCK_SECOND/50

  char *STANDING   = "ST";  //STANDING
  char *WALKING    = "WA";   //WALKING
  char *RUNNING    = "RU";   //RUNNING
  char *FALLING    = "FA";   //FALLING
  char *STATUS_PT  =  NULL;       //Nothing
  char last;

// declare/define the pridiction function.
void predict();
//	viola ! these are actions to be fired on each event. 
//	i.e set status 1, print walking, turn on blue LED and off other LEDs when WALKING is fired.
void standing(){ printf("standing\n"); }
void walking() { printf("walking\n");  }
void running() { printf("running\n");  }
void falling() { printf("falling\n");  }
#define HISTORY 16
#define sampleNo 72
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


/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */

extern resource_t res_z1_coap_obs_rtgs;
extern resource_t res_z1_coap_sens_rtgs;

PROCESS(er_example_server, "RTGS Server");
PROCESS(motion_tracking_process, "Motion Tracker");
AUTOSTART_PROCESSES(&motion_tracking_process, &er_example_server);
PROCESS_THREAD(er_example_server, ev, data)
{
  PROCESS_BEGIN();

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
  float mag[sampleNo-1],sum=0,max=0,std=0,mean=0,tmp=0;
  float result1,result2,result3,result4;
  int i;
  for(i=1;i<sampleNo;i++){
    mag[i-1]=sqrt(pow(sample[0][i]-sample[0][i-1],2)+pow(sample[1][i]-sample[1][i-1],2)+pow(sample[2][i]-sample[2][i-1],2));
    sum=sum+mag[i-1];
    if(mag[i-1]>max)max=mag[i-1];
  }

  mean =(sum/sampleNo);
  for(i=0; i<sampleNo-1; i++){ tmp = tmp + pow( ( 1 - mean ), 2); }
  std=sqrt(tmp/71);
  //printf("This is data from predict:%d,%d\n",(int)std,(int)mean);
  if(std>1000)return;
  float std1=3, mean1=2.2, max1=0.7;
  float std2=30, mean2=37, max2=5.74;
  float std3=68, mean3=83, max3=22;
  float std4=96, mean4=34, max4=42;
  float a1=1,a2=1,a3=0;
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
