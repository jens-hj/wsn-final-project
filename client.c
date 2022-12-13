#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "dev/radio.h"
#include "sys/energest.h"
#include "dev/radio.h"
#include "cc2420.h"
#include "dev/light-sensor.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO
#if defined MEASURE_ENERGY_FULL || defined MEASURE_ENERGY_SENSOR || defined MEASURE_ENERGY_AES || defined MEASURE_ENERGY_TX
#include "energest_utility.h"
#endif

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL	(CLOCK_SECOND / 4)

#ifdef ENCRYPT_WITH_AES
#include "lib/aes-128.h"
#else
#define AES_128_BLOCK_SIZE 16
#define AES_128_KEY_LENGTH 16
#endif

static struct simple_udp_connection udp_conn;
static uint32_t rx_count = 0;

PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);

static bool check_definitions() {
#if defined MEASURE_ENERGY_FULL && (defined MEASURE_TIME_SENSOR || defined MEASURE_ENERGY_SENSOR || defined MEASURE_TIME_AES || defined MEASURE_ENERGY_AES || defined MEASURE_TIME_TX || defined MEASURE_ENERGY_TX)
  LOG_INFO("Can't measure time or energy for sensor, aes or transmission and measure full energy simultaneously.\n");
  return false;
#endif

#if defined MEASURE_TIME_SENSOR && defined MEASURE_ENERGY_SENSOR
  LOG_INFO("Can't measure energy and time for sensor simultaneously.\n");
  return false;
#endif

#if defined MEASURE_TIME_AES && defined MEASURE_ENERGY_AES
  LOG_INFO("Can't measure energy and time for aes simultaneously.\n");
  return false;
#endif
#if defined MEASURE_TIME_AES && !defined ENCRYPT_WITH_AES 
  LOG_INFO("Can't measure time for aes if not encrypting with aes.\n");
  return false;
#endif
#if defined MEASURE_ENERGY_AES && !defined ENCRYPT_WITH_AES
  LOG_INFO("Can't measure energy for aes if not encrypting with aes.\n");
  return false;
#endif

#if defined MEASURE_TIME_TX && defined MEASURE_ENERGY_TX
  LOG_INFO("Can't measure energy and time for transmission simultaneously.\n");
  return false;
#endif

  return true;
}

static void udp_rx_callback(struct simple_udp_connection *c,
                            const uip_ipaddr_t *sender_addr,
                            uint16_t sender_port,
                            const uip_ipaddr_t *receiver_addr,
                            uint16_t receiver_port,
                            const uint8_t *data,
                            uint16_t datalen) {
  // LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");
  rx_count++;
}

PROCESS_THREAD(udp_client_process, ev, data) {

  static struct etimer periodic_timer;
  uip_ipaddr_t dest_ipaddr;
  static uint32_t tx_count;
  static uint32_t missed_tx_count;

  uint8_t light_data[AES_128_BLOCK_SIZE];
  static struct etimer timer_sensor;
  // static struct rtimer timer_sensor;

  PROCESS_BEGIN();

  assert(check_definitions());

  // SETUP
  
#ifdef MEASURE_ENERGY_FULL
  custom_energest_init();
#endif

  rtimer_init();

  // print the ticks per second for energest and statistics
  LOG_INFO("Energest ticks per second: %u\n", ENERGEST_SECOND);
  LOG_INFO("RTimer ticks per second: %u\n", RTIMER_SECOND);
  
  etimer_set(&timer_sensor, 1); // Collecting sensor data every 1 tick 1/128 seconds
  // rtimer_set(&timer_sensor, 1);
  etimer_set(&periodic_timer, SEND_INTERVAL);

#ifdef ENCRYPT_WITH_AES
  uint8_t key[AES_128_KEY_LENGTH] = {5, 0, 7, 6, 9, 9, 6, 2, 9, 1, 3, 8, 6, 8, 4, 0};
  AES_128.set_key(key);
#endif

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

#ifdef SINGLE_MOTE_TESTING
  NETSTACK_ROUTING.root_start();
#endif

  while(1) {
#ifndef SINGLE_MOTE_TESTING
    NETSTACK_RADIO.on(); // TURNING ON RADIO
#endif
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
#ifndef SINGLE_MOTE_TESTING
      NETSTACK_RADIO.off(); // TURNING OFF RADIO
#endif

#ifdef MEASURE_ENERGY_FULL
      custom_energest_init();
#endif

      /* Print statistics every 10th TX */
      if(tx_count % 10 == 0) {
        LOG_INFO("Tx/Rx/MissedTx: %" PRIu32 "/%" PRIu32 "/%" PRIu32 "\n",
                 tx_count, rx_count, missed_tx_count);
      }

#if defined MEASURE_TIME_SENSOR && !defined MEASURE_ENERGY_SENSOR && !defined MEASURE_ENERGY_FULL
      static int start_time_sensor;
      start_time_sensor = RTIMER_NOW();
#elif defined MEASURE_ENERGY_SENSOR && !defined MEASURE_ENERGY_FULL && !defined MEASURE_TIME_SENSOR
      custom_energest_init();
#endif
      SENSORS_ACTIVATE(light_sensor); // ACTIVATING LIGHT SENSOR
      static int i;
      for (i = 0; i < AES_128_BLOCK_SIZE; i++) {
        light_data[i] = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
        RTIMER_BUSYWAIT(2);
      }
      SENSORS_DEACTIVATE(light_sensor); // DEACTIVATING LIGHT SENSOR
#if defined MEASURE_TIME_SENSOR && !defined MEASURE_ENERGY_SENSOR && !defined MEASURE_ENERGY_FULL
      int end_time_sensor = RTIMER_NOW();
      int delta_time_sensor = end_time_sensor - start_time_sensor;
      delta_time_sensor = delta_time_sensor < 0 ? delta_time_sensor * -1 : delta_time_sensor;
      LOG_INFO("Sensor time: %d\n", delta_time_sensor);
#elif defined MEASURE_ENERGY_SENSOR && !defined MEASURE_ENERGY_FULL && !defined MEASURE_TIME_SENSOR
      LOG_INFO("Sensor energy:\n");
      custom_energest_step();
#endif
      // LOG_INFO("Light Sensor Data: ");
      // LOG_INFO_("\"");
      // for (int j = 0; j < AES_128_BLOCK_SIZE; j++) {
      //   LOG_INFO_("%02x ", light_data[j]);
      // }
      // LOG_INFO_("\"\n");

      // LOG_INFO("Before");
      // for (int i = 0; i < AES_128_BLOCK_SIZE; i++) {
      //   // LOG_INFO_(" %02x", light_data[i]);
      //   LOG_INFO_(" %d", light_data[i]);
      // }
      // LOG_INFO_("\n");

#if defined MEASURE_TIME_AES && defined ENCRYPT_WITH_AES && !defined MEASURE_ENERGY_AES && !defined MEASURE_ENERGY_FULL 
//#if defined MEASURE_TIME_AES
      static int start_time_aes;
      start_time_aes = RTIMER_NOW();
#elif defined MEASURE_ENERGY_AES && !defined MEASURE_ENERGY_FULL && !defined MEASURE_TIME_AES
      custom_energest_init();
#endif

#ifdef ENCRYPT_WITH_AES
      AES_128.encrypt(light_data);
#endif

#if defined MEASURE_TIME_AES && defined ENCRYPT_WITH_AES && !defined MEASURE_ENERGY_AES && !defined MEASURE_ENERGY_FULL
      int end_time_aes = RTIMER_NOW();
      int delta_time_aes = end_time_aes - start_time_aes;
      delta_time_aes = delta_time_aes < 0 ? delta_time_aes * -1 : delta_time_aes;
      LOG_INFO("AES time: %d\n", delta_time_aes);
#elif defined MEASURE_ENERGY_AES && !defined MEASURE_ENERGY_FULL && !defined MEASURE_TIME_AES
      LOG_INFO("AES energy:\n");
      custom_energest_step();
#endif

      LOG_INFO("Sending request %"PRIu32" to ", tx_count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      
      LOG_INFO("with");
      LOG_INFO_("\"");
      for (int i = 0; i < AES_128_BLOCK_SIZE; i++) {
        LOG_INFO_(" %02x", light_data[i]);
        // LOG_INFO_(" %d", light_data[i]);
      }
      LOG_INFO_("\"\n");

#if defined MEASURE_TIME_TX && !defined MEASURE_ENERGY_TX && !defined MEASURE_ENERGY_FULL
      static int start_time_tx;
      start_time_tx = RTIMER_NOW();
#elif defined MEASURE_ENERGY_TX && !defined MEASURE_ENERGY_FULL && !defined MEASURE_TIME_TX
      custom_energest_init();
#endif

      NETSTACK_RADIO.on(); // TURNING ON RADIO
      simple_udp_sendto(&udp_conn, light_data, sizeof(light_data)/sizeof(light_data[0]), &dest_ipaddr);
      NETSTACK_RADIO.off(); // TURNING OFF RADIO

#if defined MEASURE_TIME_TX && !defined MEASURE_ENERGY_TX && !defined MEASURE_ENERGY_FULL
      int end_time_tx = RTIMER_NOW();
      int delta_time_tx = end_time_tx - start_time_tx;
      delta_time_tx = delta_time_tx < 0 ? delta_time_tx * -1 : delta_time_tx;
      LOG_INFO("TX time: %d\n", delta_time_tx);
#elif defined MEASURE_ENERGY_TX && !defined MEASURE_ENERGY_FULL && !defined MEASURE_TIME_TX
      LOG_INFO("TX energy:\n");
      custom_energest_step();
#endif

#ifdef MEASURE_ENERGY_FULL
      custom_energest_step();
#endif

      tx_count++;
    } else {
      LOG_INFO("Not reachable yet\n");
      if(tx_count > 0) {
        missed_tx_count++;
      }
    }
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
