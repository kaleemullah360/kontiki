/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
* MQTT Server Mote App Extension By Kaleem Ullah MSCS14059@ITU.EDU.PK
*/

/*
 * \file
 *         CoAP Application layer protocol Motion Detector with Zolertia Z1 example.
 * \author
 *         Kaleem Ullah    <MSCS14059@ITU.EDU.PK>
 *
 * \Short Description:
 *
 *    Track Motion eg. Walking, Standing, Falling, Running, Battery Sensing and Temperature monitoring
 * This application uses Zolertia adxl345 sensor. and CoAP Protocol at Application layer
 * The sensor produced x, y, z axis values upone actuation.
 * Observe resources and on status change it notify the subscriber
 *
 * then using these values a predict(); function output state of the sensor node.
 * Send Battery values and Temperature.
 *
 */

/*---------------------------------------------------------------------------*/
#include "contiki-conf.h"
#include "rpl/rpl-private.h"
#include "mqtt.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "lib/sensors.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include <string.h>
#include <cc2420-radio.h>
#include <mqtt-conf.h>
//------- pgediction custom libs ------
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

  static char *STANDING   = "STANDING";  //STANDING
  static char *WALKING    = "WALKING";   //WALKING
  static char *RUNNING    = "RUNNING";   //RUNNING
  static char *FALLING    = "FALLING";   //FALLING
  static char *STATUS_PT  =  NULL;       //Nothing
  static char last;

// declare/define the pridiction function.
void predict();

#define HISTORY 16
#define sampleNo 41
// Why I'm using int16_t ? (finally I discovered, I don't know)
/* c Type   |stdint.h Type|Bits|Signed| Range           |
 signed char| int16_t     | 16 |Signed|-32,768 .. 32,767| */

static int16_t x,y,z;
static int16_t sample[3][sampleNo];
static int16_t pos;

static struct etimer et;

//-------End prediction functions ------

/*---------------------------------------------------------------------------*/
/*
 * Publish to a local MQTT broker (e.g. mosquitto) running on the host
 */
 static const char *broker_ip = MQTT_Z1_BROKER_IP_ADDR;
 #define DEFAULT_ORG_ID "rTGS-APp"
/*---------------------------------------------------------------------------*/
/*
 * A timeout used when waiting for something to happen (e.g. to connect or to
 * disconnect)
 */
#define STATE_MACHINE_PERIODIC     (CLOCK_SECOND >> 1)
/*---------------------------------------------------------------------------*/
/* Provide visible feedback via LEDS during various states */
/* When connecting to broker */
#define CONNECTING_LED_DURATION    (CLOCK_SECOND >> 2)

/* Each time we try to publish */
#define PUBLISH_LED_ON_DURATION    (CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
/* Connections and reconnections */
#define RETRY_FOREVER              0xFF
#define RECONNECT_INTERVAL         (CLOCK_SECOND * 2)

/*
 * Number of times to try reconnecting to the broker.
 * Can be a limited number (e.g. 3, 10 etc) or can be set to RETRY_FOREVER
 */
#define RECONNECT_ATTEMPTS         RETRY_FOREVER
#define CONNECTION_STABLE_TIME     (CLOCK_SECOND * 5)
 static struct timer connection_life;
 static uint8_t connect_attempt;
/*---------------------------------------------------------------------------*/
/* Various states */
 static uint8_t state;
#define STATE_INIT            0
#define STATE_REGISTERED      1
#define STATE_CONNECTING      2
#define STATE_CONNECTED       3
#define STATE_PUBLISHING      4
#define STATE_DISCONNECTED    5
#define STATE_NEWCONFIG       6
#define STATE_CONFIG_ERROR 0xFE
#define STATE_ERROR        0xFF
/*---------------------------------------------------------------------------*/
#define CONFIG_ORG_ID_LEN        32
#define CONFIG_TYPE_ID_LEN       32
#define CONFIG_AUTH_TOKEN_LEN    32
#define CONFIG_EVENT_TYPE_ID_LEN 32
#define CONFIG_CMD_TYPE_LEN       8
#define CONFIG_IP_ADDR_STR_LEN   64
/*---------------------------------------------------------------------------*/
#define RSSI_MEASURE_INTERVAL_MAX 86400 /* secs: 1 day */
#define RSSI_MEASURE_INTERVAL_MIN     5 /* secs */
#define PUBLISH_INTERVAL_MAX      86400 /* secs: 1 day */
#define PUBLISH_INTERVAL_MIN          5 /* secs */
/*---------------------------------------------------------------------------*/
/* A timeout used when waiting to connect to a network */
#define NET_CONNECT_PERIODIC        (CLOCK_SECOND >> 2)
#define NO_NET_LED_DURATION         (NET_CONNECT_PERIODIC >> 1)
/*---------------------------------------------------------------------------*/
/* Take a sensor reading on button press */
#define PUBLISH_TRIGGER &button_sensor

/* Payload length of ICMPv6 echo requests used to measure RSSI with def rt */
#define ECHO_REQ_PAYLOAD_LEN   20
/*---------------------------------------------------------------------------*/
 PROCESS_NAME(motion_tracking_process);  
 PROCESS_NAME(mqtt_z1_client_process);  
 AUTOSTART_PROCESSES(&mqtt_z1_client_process, &motion_tracking_process);
/*---------------------------------------------------------------------------*/
/**
 * \brief Data structure declaration for the MQTT client configuration
 */
 typedef struct mqtt_client_config {
  char org_id[CONFIG_ORG_ID_LEN];
  char type_id[CONFIG_TYPE_ID_LEN];
  char auth_token[CONFIG_AUTH_TOKEN_LEN];
  char event_type_id[CONFIG_EVENT_TYPE_ID_LEN];
  char broker_ip[CONFIG_IP_ADDR_STR_LEN];
  char cmd_type[CONFIG_CMD_TYPE_LEN];
  clock_time_t pub_interval;
  int def_rt_ping_interval;
  uint16_t broker_port;
} mqtt_client_config_t;
/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE    32
/*---------------------------------------------------------------------------*/
/*
 * Buffers for Client ID and Topic.
 * Make sure they are large enough to hold the entire respective string
 *
 * d:quickstart:status:EUI64 is 32 bytes long
 * iot-2/evt/status/fmt/json is 25 bytes
 * We also need space for the null termination
 */
#define BUFFER_SIZE 64
 static char client_id[BUFFER_SIZE];
 static char pub_topic[BUFFER_SIZE];
 static char sub_topic[BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
/*
 * The main MQTT buffers.
 * We will need to increase if we start publishing more data.
 */
#define APP_BUFFER_SIZE 256
 static struct mqtt_connection conn;
 static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
#define QUICKSTART "quickstart"
/*---------------------------------------------------------------------------*/
 static struct mqtt_message *msg_ptr = 0;
 static struct etimer publish_periodic_timer;
 static struct ctimer ct;
 static char *buf_ptr;
/*---------------------------------------------------------------------------*/
/* Parent RSSI functionality */
 static struct uip_icmp6_echo_reply_notification echo_reply_notification;
 static struct etimer echo_request_timer;
 //static int def_rt_rssi = 0;
/*---------------------------------------------------------------------------*/
 static mqtt_client_config_t conf;
/*---------------------------------------------------------------------------*/
PROCESS(motion_tracking_process, "Motion Tracker");
 PROCESS(mqtt_z1_client_process, "eMCH-APp");
/*---------------------------------------------------------------------------*/
 int
 ipaddr_sprintf(char *buf, uint8_t buf_len, const uip_ipaddr_t *addr)
 {
  uint16_t a;
  uint8_t len = 0;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        len += snprintf(&buf[len], buf_len - len, "::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        len += snprintf(&buf[len], buf_len - len, ":");
      }
      len += snprintf(&buf[len], buf_len - len, "%x", a);
    }
  }

  return len;
}
/*---------------------------------------------------------------------------*/
static void
echo_reply_handler(uip_ipaddr_t *source, uint8_t ttl, uint8_t *data,
 uint16_t datalen)
{ /*
  if(uip_ip6addr_cmp(source, uip_ds6_defrt_choose())) {
    def_rt_rssi = sicslowpan_get_last_rssi();
  } */
}
/*---------------------------------------------------------------------------*/
static void
publish_led_off(void *d)
{
  //leds_off(STATUS_LED);
}
/*---------------------------------------------------------------------------*/
static void
pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
  uint16_t chunk_len)
{
  DBG("Pub Handler: topic='%s' (len=%u), chunk_len=%u\n", topic, topic_len,
    chunk_len);

  /* If we don't like the length, ignore 
  if(topic_len != 23 || chunk_len != 1) {
    printf("Incorrect topic or chunk len. Ignored topic= %d chunk= %d\n", topic_len, chunk_len);
    return;
  }

   //If the format != json, ignore 
  
  if(strncmp(&topic[topic_len - 4], "json", 4) != 0) {
    printf("Incorrect format\n");
  }

  if(strncmp(&topic[10], "leds", 4) == 0) {
    if(chunk[0] == '1') {
      leds_on(LEDS_RED);
    } else if(chunk[0] == '0') {
      leds_off(LEDS_RED);
    }
    return;
  }
  */
}
/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
    case MQTT_EVENT_CONNECTED: {
      DBG("APP - Application has a MQTT connection\n");
      timer_set(&connection_life, CONNECTION_STABLE_TIME);
      state = STATE_CONNECTED;
      break;
    }
    case MQTT_EVENT_DISCONNECTED: {
      DBG("APP - MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));

      state = STATE_DISCONNECTED;
      process_poll(&mqtt_z1_client_process);
      break;
    }
    case MQTT_EVENT_PUBLISH: {
      msg_ptr = data;

    /* Implement first_flag in publish message? */
      if(msg_ptr->first_chunk) {
        msg_ptr->first_chunk = 0;
        DBG("APP - Application received a publish on topic '%s'. Payload "
          "size is %i bytes. Content:\n\n",
          msg_ptr->topic, msg_ptr->payload_length);
      }

      pub_handler(msg_ptr->topic, strlen(msg_ptr->topic), msg_ptr->payload_chunk,
        msg_ptr->payload_length);
      break;
    }
    case MQTT_EVENT_SUBACK: {
      DBG("APP - Application is subscribed to topic successfully\n");
      break;
    }
    case MQTT_EVENT_UNSUBACK: {
      DBG("APP - Application is unsubscribed to topic successfully\n");
      break;
    }
    case MQTT_EVENT_PUBACK: {
      DBG("APP - Publishing complete.\n");
      break;
    }
    default:
    DBG("APP - Application got a unhandled MQTT event: %i\n", event);
    break;
  }
}
/*---------------------------------------------------------------------------*/
static int
construct_pub_topic(void)
{
  int len = snprintf(pub_topic, BUFFER_SIZE, "iot-2/evt/%s/fmt/json",		// <---- Set Topic
   conf.event_type_id);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    printf("Pub Topic: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
construct_sub_topic(void)
{
  int len = snprintf(sub_topic, BUFFER_SIZE, "iot-2/evt/%s/fmt/json",		// <---- Set Topic
   conf.cmd_type);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    printf("Sub Topic: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
construct_client_id(void)
{
  int len = snprintf(client_id, BUFFER_SIZE, "d:%s:%s:%02x%02x%02x%02x%02x%02x",
   conf.org_id, conf.type_id,
   linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
   linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
   linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  /* len < 0: Error. Len >= BUFFER_SIZE: Buffer too small */
  if(len < 0 || len >= BUFFER_SIZE) {
    printf("Client ID: %d, Buffer %d\n", len, BUFFER_SIZE);
    return 0;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
update_config(void)
{
  if(construct_client_id() == 0) {
    /* Fatal error. Client ID larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  if(construct_sub_topic() == 0) {
    /* Fatal error. Topic larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }

  if(construct_pub_topic() == 0) {
    /* Fatal error. Topic larger than the buffer */
    state = STATE_CONFIG_ERROR;
    return;
  }


  state = STATE_INIT;

  /*
   * Schedule next timer event ASAP
   *
   * If we entered an error state then we won't do anything when it fires.
   *
   * Since the error at this stage is a config error, we will only exit this
   * error state if we get a new config.
   */
   etimer_set(&publish_periodic_timer, 0);

   return;
 }
/*---------------------------------------------------------------------------*/
 static int
 init_config()
 {
  /* Populate configuration with default values */
  memset(&conf, 0, sizeof(mqtt_client_config_t));

  memcpy(conf.org_id, DEFAULT_ORG_ID, strlen(DEFAULT_ORG_ID));
  memcpy(conf.type_id, DEFAULT_TYPE_ID, strlen(DEFAULT_TYPE_ID));
  memcpy(conf.auth_token, DEFAULT_AUTH_TOKEN, strlen(DEFAULT_AUTH_TOKEN));
  memcpy(conf.event_type_id, DEFAULT_EVENT_TYPE_ID,
   strlen(DEFAULT_EVENT_TYPE_ID));
  memcpy(conf.broker_ip, broker_ip, strlen(broker_ip));
  memcpy(conf.cmd_type, DEFAULT_SUBSCRIBE_CMD_TYPE, 1);

  conf.broker_port = DEFAULT_BROKER_PORT;
  conf.pub_interval = DEFAULT_PUBLISH_INTERVAL;
  conf.def_rt_ping_interval = DEFAULT_RSSI_MEAS_INTERVAL;

  return 1;
}
/*---------------------------------------------------------------------------*/
static void
subscribe(void)
{
  /* Publish MQTT topic in IBM quickstart format */
  mqtt_status_t status;

  status = mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS);	// <------ Set QoS

  DBG("APP - Subscribing!\n");
  if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
    DBG("APP - Tried to subscribe but command queue was full!\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
publish(void)
{

  /* Publish MQTT topic in IBM quickstart format */
int len;
int remaining = APP_BUFFER_SIZE;
  //int16_t value;

buf_ptr = app_buffer;
//  MessageID, UpTime, ClockTime, Temperature, Battery, Status  //<-- This
len = snprintf(buf_ptr, remaining,"%d,%lu,1,%d,%ld.%03d,%s", mid++ ,clock_seconds(), temp, (long)mv,(unsigned)((mv - floor_bat(mv)) * 1000), STATUS_PT);

if(len < 0 || len >= remaining) {
  printf("Buffer too short. Have %d, need %d + \\0\n", remaining, len);
  return;
}
  /* Put our Default route's string representation in a buffer */
char def_rt_str[64];
memset(def_rt_str, 0, sizeof(def_rt_str));
ipaddr_sprintf(def_rt_str, sizeof(def_rt_str), uip_ds6_defrt_choose());

mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,
               strlen(app_buffer), MQTT_QOS, MQTT_MESSAGE_STATE);	// <------ Set QoS

DBG("APP - Publish!\n");
}
/*---------------------------------------------------------------------------*/
static void
connect_to_broker(void)
{
  /* Connect to MQTT server */
  mqtt_connect(&conn, conf.broker_ip, conf.broker_port,
   conf.pub_interval * 3);

  state = STATE_CONNECTING;
}
/*---------------------------------------------------------------------------*/
static void
ping_parent(void)
{
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL) {
    return;
  }

  uip_icmp6_send(uip_ds6_defrt_choose(), ICMP6_ECHO_REQUEST, 0,
   ECHO_REQ_PAYLOAD_LEN);
}

//	viola ! these are actions to be fired on each event. 
//	i.e set status 1, print walking, turn on blue LED and off other LEDs when WALKING is fired.
void standing(){publish(); printf("standing\n"); leds_on(LEDS_BLUE);  leds_off(LEDS_RED); leds_off(LEDS_GREEN); }
void walking() {publish(); printf("walking\n");  leds_on(LEDS_GREEN); leds_off(LEDS_RED); leds_off(LEDS_BLUE);  }
void running() {publish(); printf("running\n");  leds_on(LEDS_GREEN); leds_on(LEDS_RED);  leds_off(LEDS_BLUE);  }
void falling() {publish(); printf("falling\n");  leds_on(LEDS_RED);   leds_off(LEDS_BLUE);leds_off(LEDS_GREEN); }
/*---------------------------------------------------------------------------*/
  
/* accelerometer free fall detection callback */
void accm_ff_cb(uint8_t reg){
#define ANNOYING_ALWAYS_THERE_ANYWAY_OUTPUT 0
  if(reg && ADXL345_INT_FREEFALL){last=*STATUS_PT;STATUS_PT=FALLING;if(last!=*STATUS_PT){ falling(); }}
}

static void
state_machine(void)
{
  switch(state) {
    case STATE_INIT:
    /* If we have just been configured register MQTT connection */
    mqtt_register(&conn, &mqtt_z1_client_process, client_id, mqtt_event,
      MAX_TCP_SEGMENT_SIZE);

    /*
     * If we are not using the quickstart service (thus we are an IBM
     * registered device), we need to provide user name and password
     */
     if(strncasecmp(conf.org_id, QUICKSTART, strlen(conf.org_id)) != 0) {
      if(strlen(conf.auth_token) == 0) {
        printf("User name set, but empty auth token\n");
        state = STATE_ERROR;
        break;
      } else {
        mqtt_set_username_password(&conn, "SuperUser",
         conf.auth_token);
      }
    }

    /* _register() will set auto_reconnect. We don't want that. */
    conn.auto_reconnect = 0;
    connect_attempt = 1;

    state = STATE_REGISTERED;
    DBG("Init\n");
    /* Continue */
    case STATE_REGISTERED:
    if(uip_ds6_get_global(ADDR_PREFERRED) != NULL) {
      /* Registered and with a public IP. Connect */
      DBG("Registered. Connect attempt %u\n", connect_attempt);
      ping_parent();
      connect_to_broker();
    } else {
      ctimer_set(&ct, NO_NET_LED_DURATION, publish_led_off, NULL);
    }
    etimer_set(&publish_periodic_timer, NET_CONNECT_PERIODIC);
    return;
    break;
    case STATE_CONNECTING:
    ctimer_set(&ct, CONNECTING_LED_DURATION, publish_led_off, NULL);
    /* Not connected yet. Wait */
    DBG("Connecting (%u)\n", connect_attempt);
    break;
    case STATE_CONNECTED:
    /* Don't subscribe unless we are a registered device */
    if(strncasecmp(conf.org_id, QUICKSTART, strlen(conf.org_id)) == 0) {
      DBG("Using 'quickstart': Skipping subscribe\n");
      state = STATE_PUBLISHING;
    }
    /* Continue */
    case STATE_PUBLISHING:
    /* If the timer expired, the connection is stable. */
    if(timer_expired(&connection_life)) {
      /*
       * Intentionally using 0 here instead of 1: We want RECONNECT_ATTEMPTS
       * attempts if we disconnect after a successful connect
       */
       connect_attempt = 0;
     }

     if(mqtt_ready(&conn) && conn.out_buffer_sent) {
      /* Connected. Publish */
      if(state == STATE_CONNECTED) {
        subscribe();
        state = STATE_PUBLISHING;
      } else {
        ctimer_set(&ct, PUBLISH_LED_ON_DURATION, publish_led_off, NULL);
  /*      if(ttem != bat_mv) {
    ttem = bat_mv;   // update the temporary valruable with fresh value
     publish();  // <-- This
   } */

   }
   etimer_set(&publish_periodic_timer, conf.pub_interval);

   DBG("Publishing\n");
      /* Return here so we don't end up rescheduling the timer */
   return;
 } else {
      /*
       * Our publish timer fired, but some MQTT packet is already in flight
       * (either not sent at all, or sent but not fully ACKd).
       *
       * This can mean that we have lost connectivity to our broker or that
       * simply there is some network delay. In both cases, we refuse to
       * trigger a new message and we wait for TCP to either ACK the entire
       * packet after retries, or to timeout and notify us.
       */
       DBG("Publishing... (MQTT state=%d, q=%u)\n", conn.state,
        conn.out_queue_full);
     }
     break;
     case STATE_DISCONNECTED:
     DBG("Disconnected\n");
     if(connect_attempt < RECONNECT_ATTEMPTS ||
       RECONNECT_ATTEMPTS == RETRY_FOREVER) {
      /* Disconnect and backoff */
      clock_time_t interval;
    mqtt_disconnect(&conn);
    connect_attempt++;

    interval = connect_attempt < 3 ? RECONNECT_INTERVAL << connect_attempt :
    RECONNECT_INTERVAL << 3;

    DBG("Disconnected. Attempt %u in %lu ticks\n", connect_attempt, interval);

    etimer_set(&publish_periodic_timer, interval);

    state = STATE_REGISTERED;
    return;
  } else {
      /* Max reconnect attempts reached. Enter error state */
    state = STATE_ERROR;
    DBG("Aborting connection after %u attempts\n", connect_attempt - 1);
  }
  break;
  case STATE_CONFIG_ERROR:
    /* Idle away. The only way out is a new config */
  printf("Bad configuration.\n");
  return;
  case STATE_ERROR:
  default:
    /*
     * 'default' should never happen.
     *
     * If we enter here it's because of some error. Stop timers. The only thing
     * that can bring us out is a new config event
     */
     printf("Default case: State=0x%02x\n", state);
     return;
   }

  /* If we didn't return so far, reschedule ourselves */
   etimer_set(&publish_periodic_timer, STATE_MACHINE_PERIODIC);
 }
/*---------------------------------------------------------------------------*/

 PROCESS_THREAD(mqtt_z1_client_process, ev, data)
 {

  PROCESS_BEGIN();

  set_cc2420_txpower(0);
  set_cc2420_channel(0);
  printf("eMCH-APp\n");

  if(init_config() != 1) {
    PROCESS_EXIT();
  }

  update_config();
  //def_rt_rssi = 0x8000000;
  //def_rt_rssi = 0x8000;
  uip_icmp6_echo_reply_callback_add(&echo_reply_notification,
    echo_reply_handler);
  etimer_set(&echo_request_timer, conf.def_rt_ping_interval);

  /* Main loop */
  while(1) {

    PROCESS_YIELD();

    if(ev == sensors_event && data == PUBLISH_TRIGGER) {
      if(state == STATE_ERROR) {
        connect_attempt = 1;
        state = STATE_REGISTERED;
      }
    }

    if((ev == PROCESS_EVENT_TIMER && data == &publish_periodic_timer) ||
     ev == PROCESS_EVENT_POLL ||
     (ev == sensors_event && data == PUBLISH_TRIGGER)) {
      state_machine();
  }

  if(ev == PROCESS_EVENT_TIMER && data == &echo_request_timer) {
    ping_parent();
    etimer_set(&echo_request_timer, conf.def_rt_ping_interval);
  }
}

PROCESS_END();
}

PROCESS_THREAD(motion_tracking_process, ev, data){
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(battery_sensor);
  tmp102_init();
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
    //printf("FinalStatus: %s\n", STATUS_PT);

  }
  printf("Motion Tracking Terminated\n");
  SENSORS_DEACTIVATE(battery_sensor);
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
