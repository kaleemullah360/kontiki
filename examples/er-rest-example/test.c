#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"

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
static int value = 10;

void
get_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	/* Populat the buffer with the response payload*/
	char message[20];
	int length = 20;

	sprintf(message, "VALUE:%03u", value);
	length = strlen(message);
	memcpy(buffer, message, length);

	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); 
	REST.set_header_etag(response, (uint8_t *) &length, 1);
	REST.set_response_payload(response, buffer, length);
}

void
post_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){


  int new_value, len;
  const char *val = NULL;
     
  len=REST.get_post_variable(request, "value", &val);
     
  if( len > 0 ){
     new_value = atoi(val);	
     value = new_value;
     REST.set_response_status(response, REST.status.CREATED);
  } else {
     REST.set_response_status(response, REST.status.BAD_REQUEST);
  }
}

RESOURCE(resource_example, "title=\"Resource\";rt=\"Text\"", get_handler, post_handler, NULL, NULL);

PROCESS(motion_tracking_process, "Motion Tracker");

PROCESS(server, "CoAP Server");
AUTOSTART_PROCESSES(&server, &motion_tracking_process);
PROCESS_THREAD(server, ev, data){
	PROCESS_BEGIN();
	rest_init_engine();
	rest_activate_resource(&resource_example, "example");
	while(1) {
   		 PROCESS_WAIT_EVENT();
	}
	PROCESS_END();
}



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
