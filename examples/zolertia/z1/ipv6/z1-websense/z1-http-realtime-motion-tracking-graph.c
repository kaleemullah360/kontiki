/*
 * Copyright (c) 2011, Zolertia(TM) is a trademark by Advancare,SL
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         Battery and Temperature IPv6 Demo for Zolertia Z1
 * \author
 *         Niclas Finne    <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Joel Hoglund    <joel@sics.se>
 *         Enric M. Calvo  <ecalvo@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "httpd-simple.h"
#include "webserver-nogui.h"
#include "dev/temperature-sensor.h"
#include "dev/battery-sensor.h"
#include "cc2420.h"
#include "dev/leds.h"
#include <stdio.h>
#include "dev/sht25.h"
#include "sys/ctimer.h"
#include "dev/adxl345.h"

static struct etimer et;

// set the sensor value get interval
#define ACCM_READ_INTERVAL    CLOCK_SECOND/50

// define Status
static char *WALKING = "1"; //walking
static char *STANDING = "2"; // standing
static char *RUNNING= "3"; // running
static char *FALLING = "4"; // falling
static char *STATUS_PT = NULL;
static char last;

// declare the pridiction function.
void predict();
double sqrt();


/*---------------------------------------------------------------------------*/
float
floor(float x)
{
  if(x >= 0.0f) {
    return (float)((int)x);
  } else { return (float)((int)x - 1);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS(web_sense_process, "Sense Web Demo");
AUTOSTART_PROCESSES(&web_sense_process);
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#define sampleNo 81
static int16_t x,y,z;
static int sample[3][sampleNo];
static int pos;

#define HISTORY 16
static int temperature[HISTORY];
static int battery1[HISTORY];
static int sensors_pos;
/*---------------------------------------------------------------------------*/
static int
get_battery(void)
{
  return battery_sensor.value(0);
}
/*---------------------------------------------------------------------------*/
static int
get_temp(void)
{
  return temperature_sensor.value(0);
}
static float
get_mybatt(void)
{
  return (float)((get_battery() * 2.500 * 2) / 4096);
}
static float
get_mytemp(void)
{
  return (float)(((get_temp() * 2.500) / 4096) - 0.986) * 282;
}

static float get_myhumi(void){
return (float)(sht25.value(SHT25_VAL_HUM));
}
/*---------------------------------------------------------------------------*/
/* Only one single request at time */
static char buf[256];
static int blen;
#define ADD(...) do { \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__); \
} while(0)
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_values(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

//clock_seconds();
    blen = 0;
    float mybatt = get_mybatt();
    float mytemp = get_mytemp();
    float myhumi = get_myhumi();
    
    ADD("cb([");
    
    // Temperature
    ADD("{\"x\":%lu,\"y\":%ld.%03d},",clock_seconds(),(long)mytemp,(unsigned)((mytemp - floor(mytemp)) * 1000));

    // Humidity
    ADD("{\"x\":%lu,\"y\":%ld.%03d},",clock_seconds(),(long)myhumi,(unsigned)((myhumi - floor(myhumi)) * 1000));

    // Battery
    ADD("{\"x\":%lu,\"y\":%ld.%03d},",clock_seconds(),(long)mybatt,(unsigned)((mybatt - floor(mybatt)) * 1000));
    
    //Movement Status 
    ADD("{\"x\":%lu,\"y\":%s}",clock_seconds(),STATUS_PT);
    
    ADD("]);");
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
PROCESS_THREAD(web_sense_process, ev, data)
{
  static struct etimer timer;
  PROCESS_BEGIN();
  cc2420_set_txpower(31);


    /* Start and setup the accelerometer with default values, eg no interrupts enabled. */
    accm_init();

    /* Set what strikes the corresponding interrupts. Several interrupts per pin is
      possible. For the eight possible interrupts, see adxl345.h and adxl345 datasheet. */
    accm_set_irq(ADXL345_INT_FREEFALL, ADXL345_INT_TAP + ADXL345_INT_DOUBLETAP);

  sensors_pos = 0;
  process_start(&webserver_nogui_process, NULL);

  etimer_set(&timer, CLOCK_SECOND * 2);
  SENSORS_ACTIVATE(battery_sensor);
  SENSORS_ACTIVATE(temperature_sensor);

  while(1) {

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
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      if(STATUS_PT==FALLING){
          etimer_set(&timer, 15);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));}

    battery1[sensors_pos] = get_mybatt() * 1000;
    temperature[sensors_pos] = get_mytemp();
    sensors_pos = (sensors_pos + 1) % HISTORY;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
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
  if(result1<result2 && result1<result3 && result1<result4){last=*STATUS_PT;STATUS_PT=STANDING;if(last!=*STATUS_PT)printf("2\n");}// standing
  if(result2<result1 && result2<result3 && result2<result4){last=*STATUS_PT;STATUS_PT=WALKING;if(last!=*STATUS_PT)printf("1\n");}//walking
  if(result3<result1 && result3<result2 && result3<result4){last=*STATUS_PT;STATUS_PT=RUNNING;if(last!=*STATUS_PT)printf("3\n");}// running
  if(result4<result1 && result4<result2 && result4<result3){
    last=*STATUS_PT;STATUS_PT=FALLING;if(last!=*STATUS_PT)printf("4\n"); // falling
  }
}
/*---------------------------------------------------------------------------*/