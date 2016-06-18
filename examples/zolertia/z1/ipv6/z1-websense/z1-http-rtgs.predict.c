/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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

/**
 * \file
 *         Light and temperatur sensor web demo
 * \author
 *         Niclas Finne    <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Joel Hoglund    <joel@sics.se>
 */

#include "contiki.h"
#include "httpd-simple.h"
#include <stdio.h>
#include <cc2420-radio.h>

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
// set the sensor value get interval
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
//  viola ! these are actions to be fired on each event. 
//  i.e set status 1, print walking, turn on blue LED and off other LEDs when WALKING is fired.
void standing(){ leds_on(LEDS_BLUE);  leds_off(LEDS_RED); leds_off(LEDS_GREEN); }
void walking() { leds_on(LEDS_GREEN); leds_off(LEDS_RED); leds_off(LEDS_BLUE);  }
void running() { leds_on(LEDS_GREEN); leds_on(LEDS_RED);  leds_off(LEDS_BLUE);  }
void falling() { leds_on(LEDS_RED);   leds_off(LEDS_BLUE);leds_off(LEDS_GREEN); }

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
PROCESS(web_sense_process, "rTGS");
PROCESS(webserver_nogui_process, "rTGS server");
PROCESS(motion_tracking_process, "Motion Tracker");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

	set_cc2420_txpower(0);
	set_cc2420_channel(0);
	SENSORS_ACTIVATE(battery_sensor);
	tmp102_init();
  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

	SENSORS_DEACTIVATE(battery_sensor);
  PROCESS_END();
}
AUTOSTART_PROCESSES(&web_sense_process,&webserver_nogui_process,&motion_tracking_process);


/*---------------------------------------------------------------------------*/
/* Only one single request at time */
static char buf[256];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)

static
PT_THREAD(send_values(struct httpd_state *s))
{
PSOCK_BEGIN(&s->sout);
blen = 0;

//  MessageID, UpTime, ClockTime, Temperature, Battery, Status, RTT  //<-- This
ADD("\n");
ADD("%d,%lu,1,%d,%ld.%03d,%s", mid++ ,clock_seconds(), temp, (long)mv,(unsigned)((mv - floor_bat(mv)) * 1000), STATUS_PT);

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
  etimer_set(&timer, CLOCK_SECOND * 2);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

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

  printf("FinalStatus: %s\n", STATUS_PT);
//------------ End Prediction (read values) ------------------ 
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
