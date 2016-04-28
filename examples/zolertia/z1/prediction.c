/*
* \file
*         Motion Detector with Zolertia Z1.
          Erbium (Er) REST Engine example.
* \author
*         Kaleem Ullah    <MSCS14059@ITU.EDU.PK>
*
* \Short Description:
*
*    Track Motion eg. Walking, Standing, Falling, Running
* this application uses Zolertia adxl345 sensor.
* The sensor produced x, y, z axis values upone actuation.
* then using these values a predict(); function output state of the sensor node.
*/

#include "contiki.h"
#include <stdio.h>

//------- prediction custom libs ------
#include "dev/adxl345.h"
#include <math.h>
#include "dev/leds.h"
//------- End prediction custom libs ------

//------- prediction functions ------
// set the sensor value get interval
#define ACCM_READ_INTERVAL    CLOCK_SECOND/50

// define Status
static char *STANDING = "1"; //STANDING
static char *WALKING = "2";   //WALKING
static char *RUNNING= "3";    //RUNNING
static char *FALLING = "4";   //FALLING
static char *STATUS_PT = NULL;
static char *status_str = "1"; //STANDING
static char last;

// declare the pridiction function.
void predict();

#define HISTORY 16

/*---------------------------------------------------------------------------*/

#define sampleNo 81
static int16_t x,y,z;
static int sample[3][sampleNo];
static int pos;

static struct etimer et;

void print_int(uint16_t reg){
#define ANNOYING_ALWAYS_THERE_ANYWAY_OUTPUT 0
  if(reg && ADXL345_INT_FREEFALL) {last=*STATUS_PT;STATUS_PT=FALLING;if(last!=*STATUS_PT){status_str = "4"; printf("falling\n");leds_toggle(LEDS_RED);}}
}

/* accelerometer free fall detection callback */
void accm_ff_cb(uint8_t reg){
  print_int(reg);
}

//-------End prediction functions ------
/*---------------------------------------------------------------------------*/

PROCESS(motion_tracking_process, "Motion Tracker");

AUTOSTART_PROCESSES(&motion_tracking_process);

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

  int counter = 0;
  while(counter < 200){
//------------ Prediction (read values) ------------------
    x = accm_read_axis(X_AXIS);
    y = accm_read_axis(Y_AXIS);
    z = accm_read_axis(Z_AXIS);

    sample[0][pos]=(int)x;
    sample[1][pos]=(int)y;
    sample[2][pos]=(int)z;

      if(pos==(sampleNo-1))predict();
      pos=(pos+1)%sampleNo;
            etimer_set(&et, ACCM_READ_INTERVAL);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      if(STATUS_PT==FALLING){
          etimer_set(&et, 15);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));}
//------------ End Prediction (read values) ------------------
  printf("FinalStatus: %s\n", status_str);
 
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
  for(i=0;i<sampleNo-1;i++){
    tmp = tmp + pow( ( 1 - mean ), 2);
  }
  std=sqrt(tmp/80);
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
  if(result1<result2 && result1<result3 && result1<result4){last=*STATUS_PT;STATUS_PT=STANDING;if(last!=*STATUS_PT){status_str = "1"; printf("standing\n");leds_toggle(LEDS_BLUE);}}
  if(result2<result1 && result2<result3 && result2<result4){last=*STATUS_PT;STATUS_PT=WALKING;if(last!=*STATUS_PT){status_str = "2"; printf("walking\n");leds_toggle(LEDS_GREEN);}}
  if(result3<result1 && result3<result2 && result3<result4){last=*STATUS_PT;STATUS_PT=RUNNING;if(last!=*STATUS_PT){status_str = "3"; printf("running\n");leds_toggle(LEDS_GREEN);}}
}
/*---- End Human Body Posture Detection and Prediction ---------------*/