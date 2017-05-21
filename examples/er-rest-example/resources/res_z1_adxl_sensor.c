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
* then using these values a predict_adxl(); function output state of the sensor node.
*/

#include "contiki.h"

#if PLATFORM_HAS_ADXLSENSOR

#include <string.h>
#include "rest-engine.h"

#include "dev/adxl345.h"
#include <math.h>

/*--------------------------- Motion Tracking --------------------------*/
#define sampleNo 81
static int16_t x,y,z;
static int sample[3][sampleNo];
static int pos;


// define Status
static char *WALKING = "WALKING";
static char *STANDING = "STANDING";
static char *RUNNING= "RUNNING";
static char *FALLING = "FALLING";
static char *STATUS_PT = NULL;
static char last;

// declare the pridiction function.
void predict_adxl();

/*-------------------------End Motion Tracking --------------------------*/

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* Get Method Example. Returns the reading from Adxl345 sensors. */
RESOURCE(res_z1_adxl_sensor,
 "title=\"Motion Tracking\";rt=\"Adxl345\"",
 res_get_handler,
 NULL,
 NULL,
 NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{


  x = accm_read_axis(X_AXIS);
  y = accm_read_axis(Y_AXIS);
  z = accm_read_axis(Z_AXIS);
  //printf("x: %d y: %d z: %d\n", x, y, z);
  sample[0][pos]=(int)x;
  sample[1][pos]=(int)y;
  sample[2][pos]=(int)z;
  predict_adxl();
  if(pos==(sampleNo-1))predict_adxl();
  pos=(pos+1)%sampleNo;

  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "Staus: %s", STATUS_PT); //40
    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
  } 
  } //End Resources get handler

/*------------------------- Movement Prediction Function -----------------------*/
/**
 * TODO: Needs improvements
 * this function get the Sensor X, Y, Z Coordinate values and predicts different states
 */
 
 void predict_adxl(){

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
  //printf("This is data from predict_adxl:%d,%d\n",(int)std,(int)mean);
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

#endif /* PLATFORM_HAS_ADXLSENSOR */
