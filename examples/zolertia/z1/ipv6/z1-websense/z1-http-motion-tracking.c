/*
 * Copyright (c) 2011, Zolertia(TM) is a trademark by Advancare,SL
 * All rights reserved.
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         Motion Detector with Zolertia Z1
 * \author
 *         Kaleem Ullah    <MSCS14059@ITU.EDU.PK>
 */
/*---------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include "contiki.h"
#include "httpd-simple.h"
#include "webserver-nogui.h"
#include "dev/cc2420/cc2420.h"
#include "dev/adxl345.h"

static struct etimer et;

// set the sensor value get interval
#define ACCM_READ_INTERVAL    CLOCK_SECOND/50

// define Status
static char *WALKING = "WALKING";
static char *STANDING = "STANDING";
static char *RUNNING= "RUNNING";
static char *FALLING = "FALLING";
static char *STATUS_PT = NULL;
static char last;

// declare the pridiction function.
void predict();
/*---------------------------------------------------------------------------*/
// Perform on Acceleration
/*---------------------------------------------------------------------------*/
PROCESS(accel_process, "Test Accel process");
AUTOSTART_PROCESSES(&accel_process);
/*---------------------------------------------------------------------------*/
#define HISTORY 16

/*---------------------------------------------------------------------------*/

#define sampleNo 81
static int16_t x,y,z;
static int sample[3][sampleNo];
static int pos;

/* Only one single request at time */
static char buf[256];
static int blen;
#define ADD(...) do { \
  blen = snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__); \
} while(0)
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
  static
PT_THREAD(send_values(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
  ADD("%s", STATUS_PT); //Motion Status (Values displayed on Web Browser)
  SEND_STRING(&s->sout, buf);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
  httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return send_values;
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(accel_process, ev, data) {
  PROCESS_BEGIN();
  {

    cc2420_set_txpower(31);
    process_start(&webserver_nogui_process, NULL);

    /* Start and setup the accelerometer with default values, eg no interrupts enabled. */
    accm_init();

    /* Set what strikes the corresponding interrupts. Several interrupts per pin is
      possible. For the eight possible interrupts, see adxl345.h and adxl345 datasheet. */
    accm_set_irq(ADXL345_INT_FREEFALL, ADXL345_INT_TAP + ADXL345_INT_DOUBLETAP);

    while (1) {
      x = accm_read_axis(X_AXIS);
      y = accm_read_axis(Y_AXIS);
      z = accm_read_axis(Z_AXIS);
      //printf("x: %d y: %d z: %d\n", x, y, z);
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
    }
  }
  PROCESS_END();
}

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
  if(result1<result2 && result1<result3 && result1<result4){last=*STATUS_PT;STATUS_PT=STANDING;if(last!=*STATUS_PT)printf("standing\n");}
  if(result2<result1 && result2<result3 && result2<result4){last=*STATUS_PT;STATUS_PT=WALKING;if(last!=*STATUS_PT)printf("walking\n");}
  if(result3<result1 && result3<result2 && result3<result4){last=*STATUS_PT;STATUS_PT=RUNNING;if(last!=*STATUS_PT)printf("running\n");}
  if(result4<result1 && result4<result2 && result4<result3){
    last=*STATUS_PT;STATUS_PT=FALLING;if(last!=*STATUS_PT)printf("falling\n");
  }
}
/*---------------------------------------------------------------------------*/