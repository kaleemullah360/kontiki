/*
 * Copyright (c) 2014, Nimbus Centre for Embedded Systems Research, Cork Institute of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/*
* \file   Resource for:
*           Motion Detector with Zolertia Z1 using CoAP Application layer protocol.
* \author
*         Kaleem Ullah    <MSCS14059@ITU.EDU.PK>
*
* \Short Description:
* 
*    Track Motion eg. Walking, Standing, Falling, Running
* this application uses Zolertia adxl345 sensor and CoAP protocol on Application layer.
* The sensor produced x, y, z axis values upone actuation.
* then using these values a obspredict(); function output state of the sensor node.
*/

#include <string.h>
#include "rest-engine.h"
#include "er-coap.h"

#if PLATFORM_HAS_ZSADXL_THB

//------- custom libs ------
#include "dev/adxl345.h"
#include <math.h>
#include "dev/leds.h"

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
void obspredict();

#define HISTORY 16

/*---------------------------------------------------------------------------*/

#define sampleNo 81
static int16_t x,y,z;
static int sample[3][sampleNo];
static int pos;

static struct etimer et;

/* Start and setup the accelerometer with default values, eg no interrupts enabled. */

/* Set what strikes the corresponding interrupts. Several interrupts per pin is
   possible. For the eight possible interrupts, see adxl345.h and adxl345 datasheet. */

 
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);

PERIODIC_RESOURCE(res_z1_coap_obs_rtgs,
                  "title=\"RTGS\";obs",
                  res_get_handler,
                  NULL,
                  NULL,
                  NULL,
                  1 * CLOCK_SECOND,
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
  REST.set_response_payload(response, buffer, snprintf((char *)buffer, preferred_size, "VERY LONG EVENT %s", STATUS_PT));

  /* The REST.subscription_handler() will be called for observable resources by the REST framework. */
}
/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
static void
res_periodic_handler()
{
 printf( "EVENT %s\n", STATUS_PT);
 x = accm_read_axis(X_AXIS);
 y = accm_read_axis(Y_AXIS);
 z = accm_read_axis(Z_AXIS);

 sample[0][pos]=(int)x;
 sample[1][pos]=(int)y;
 sample[2][pos]=(int)z;

 if(pos==(sampleNo-1))obspredict();
 	pos=(pos+1)%sampleNo;


if(STATUS_PT==FALLING){
    etimer_set(&et, 15);
}

void print_int(uint16_t reg){
#define ANNOYING_ALWAYS_THERE_ANYWAY_OUTPUT 0
  if(reg && ADXL345_INT_FREEFALL){
  	last=*STATUS_PT; STATUS_PT=FALLING;
  	if(last!=*STATUS_PT){
  		printf("falling\n");leds_toggle(LEDS_RED);
  		REST.notify_subscribers(&res_z1_coap_obs_rtgs);
  	}}
}

/* accelerometer free fall detection callback */
void accm_ff_cb(uint8_t reg){
  print_int(reg);
}

}

/*--- - Human Body Posture Detection and obsPrediction ---------------*/
void obspredict(){
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
  //printf("This is data from obspredict:%d,%d\n",(int)std,(int)mean);
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
  printf("This is four results:%d,%d,%d,%d\n",(int)result1,(int)result2,(int)result3,(int)result4);
  if(result1<result2 && result1<result3 && result1<result4){last=*STATUS_PT;STATUS_PT=STANDING;if(last!=*STATUS_PT){printf("standing\n");leds_toggle(LEDS_BLUE);REST.notify_subscribers(&res_z1_coap_obs_rtgs);}}
  if(result2<result1 && result2<result3 && result2<result4){last=*STATUS_PT;STATUS_PT=WALKING;if(last!=*STATUS_PT){printf("walking\n");leds_toggle(LEDS_GREEN);REST.notify_subscribers(&res_z1_coap_obs_rtgs);}}
  if(result3<result1 && result3<result2 && result3<result4){last=*STATUS_PT;STATUS_PT=RUNNING;if(last!=*STATUS_PT){printf("running\n");leds_toggle(LEDS_GREEN);REST.notify_subscribers(&res_z1_coap_obs_rtgs);}}
}
/*---- End Human Body Posture Detection and obsPrediction ---------------*/

#endif /* PLATFORM_HAS_ZSADXL_THB */
