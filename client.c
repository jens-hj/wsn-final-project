#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include "dev/radio.h"
#include "sys/energest.h"
// #include "simple-energest.h"
#include "lib/aes-128.h"
#include "dev/radio.h"
#include "cc2420.h"
#include "dev/light-sensor.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_INTERVAL	(1 * CLOCK_SECOND)

#define AES_128_BLOCK_SIZE 16
#define AES_128_KEY_LENGTH 16

static struct simple_udp_connection udp_conn;
static uint32_t rx_count = 0;

/*---------------------------------------------------------------------------*/
// os/services/simple-energest.c
static uint64_t last_tx, last_rx, last_time, last_cpu, last_lpm, last_deep_lpm;
void
my_energest_init(void)
{
  energest_flush();
  last_time = ENERGEST_GET_TOTAL_TIME();
  last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  last_deep_lpm = energest_type_time(ENERGEST_TYPE_DEEP_LPM);
  last_tx = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  last_rx = energest_type_time(ENERGEST_TYPE_LISTEN);
}
/*---------------------------------------------------------------------------*/
// os/services/simple-energest.c
static uint64_t
to_permil(uint64_t delta_metric, uint64_t delta_time)
{
  return (1000ul * delta_metric) / delta_time;
}
/*---------------------------------------------------------------------------*/
// os/services/simple-energest.c
static void
log_energest(const char *name, uint64_t delta, uint64_t delta_time)
{
  LOG_INFO("%-12s: %10"PRIu64"/%10"PRIu64" (%"PRIu64" permil)\n",
           name, delta, delta_time, to_permil(delta, delta_time));
}
/*---------------------------------------------------------------------------*/
// os/services/simple-energest.c
static void
simple_energest_step(void)
{
  static unsigned count = 0;
  uint64_t curr_tx, curr_rx, curr_time, curr_cpu, curr_lpm, curr_deep_lpm;
  uint64_t delta_time;

  energest_flush();

  curr_time = ENERGEST_GET_TOTAL_TIME();
  curr_cpu = energest_type_time(ENERGEST_TYPE_CPU);
  curr_lpm = energest_type_time(ENERGEST_TYPE_LPM);
  curr_deep_lpm = energest_type_time(ENERGEST_TYPE_DEEP_LPM);
  curr_tx = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  curr_rx = energest_type_time(ENERGEST_TYPE_LISTEN);

  delta_time = curr_time - last_time > 1 ? curr_time - last_time : 1;

  LOG_INFO("--- Period summary #%u (%"PRIu64" seconds)\n",
           count++, delta_time / ENERGEST_SECOND);
  LOG_INFO("Total time  : %10"PRIu64"\n", delta_time);
  log_energest("CPU", curr_cpu - last_cpu, delta_time);
  log_energest("LPM", curr_lpm - last_lpm, delta_time);
  log_energest("Deep LPM", curr_deep_lpm - last_deep_lpm, delta_time);
  log_energest("Radio Tx", curr_tx - last_tx, delta_time);
  log_energest("Radio Rx", curr_rx - last_rx, delta_time);
  log_energest("Radio total", curr_tx - last_tx + curr_rx - last_rx,
               delta_time);

  last_time = curr_time;
  last_cpu = curr_cpu;
  last_lpm = curr_lpm;
  last_deep_lpm = curr_deep_lpm;
  last_tx = curr_tx;
  last_rx = curr_rx;
}

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");
  rx_count++;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data) {

  static struct etimer periodic_timer;
  uip_ipaddr_t dest_ipaddr;
  static uint32_t tx_count;
  static uint32_t missed_tx_count;

  // uint8_t key[AES_128_KEY_LENGTH] = {5, 0, 7, 6, 9, 9, 6, 2, 9, 1, 3, 8, 6, 8, 4, 0};
  uint8_t light_data[AES_128_BLOCK_SIZE];
  static struct etimer timer;

  PROCESS_BEGIN();

  // SETUP
  // energest_flush();
  // energest_init();
  my_energest_init();
  // clock_init();
  cc2420_init();
  cc2420_on(); // TURNING CC2420 ON

  // print the ticks per second for energest and statistics
  LOG_INFO("Energest ticks per second: %u\n", ENERGEST_SECOND);
  etimer_set(&timer, CLOCK_SECOND * 0.1);

  // AES_128.set_key(key);

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);

  // TEST
  NETSTACK_ROUTING.root_start();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() &&
        NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {

      /* Print statistics every 10th TX */
      if(tx_count % 10 == 0) {
        LOG_INFO("Tx/Rx/MissedTx: %" PRIu32 "/%" PRIu32 "/%" PRIu32 "\n",
                 tx_count, rx_count, missed_tx_count);
        // ENERGEST EVERY 10 SEND
        // slide 31-32, lecture 2
        simple_energest_step();
        // LOG_INFO("%llu\n", ENERGEST_GET_TOTAL_TIME());
      }
      
      
      SENSORS_ACTIVATE(light_sensor); // ACTIVATING LIGHT SENSOR
      static int i;
      for (i = 0; i < AES_128_BLOCK_SIZE; i++) {
        light_data[i] = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
        etimer_reset(&timer);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      }
      // LOG_INFO("Light Sensor Data: ");
      // for (int j = 0; j < AES_128_BLOCK_SIZE; j++) {
      //   LOG_INFO_("%d ", light_data[j]);
      // }
      // LOG_INFO_("\n");
      SENSORS_DEACTIVATE(light_sensor); // DEACTIVATING LIGHT SENSOR


      /* LOG_INFO("Before");
      for (int i = 0; i < AES_128_BLOCK_SIZE; i++) {
        // LOG_INFO_(" %02x", light_data[i]);
        LOG_INFO_(" %d", light_data[i]);
      }
      LOG_INFO_("\n"); */

      // int start_time_E = ENERGEST_GET_TOTAL_TIME();
      // LOG_INFO("start_time_E: %d\n", start_time_E);
      int start_time = RTIMER_NOW();
      // LOG_INFO("start_time: %d\n", start_time);
      // AES_128.encrypt(light_data);

      /* Send to DAG root */
      LOG_INFO("Sending request %"PRIu32" to ", tx_count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      
      /* LOG_INFO("with");
      for (int i = 0; i < AES_128_BLOCK_SIZE; i++) {
        // LOG_INFO_(" %02x", light_data[i]);
        LOG_INFO_(" %d", light_data[i]);
      }
      LOG_INFO_("\n"); */

      NETSTACK_RADIO.on(); // TURNING ON RADIO
      simple_udp_sendto(&udp_conn, light_data, sizeof(light_data)/sizeof(light_data[0]), &dest_ipaddr);
      NETSTACK_RADIO.off(); // TURNING OFF RADIO

      int end_time = RTIMER_NOW();
      // LOG_INFO("end_time: %d\n", end_time);
      // int end_time_E = ENERGEST_GET_TOTAL_TIME();
      // LOG_INFO("end_time_E: %d\n", end_time_E);
      // int delta_time_E = end_time_E - start_time_E;
      // LOG_INFO("Transmission time Energest: %d\n", delta_time_E);
      int delta_time = end_time - start_time;
      LOG_INFO("Transmission time: %d\n", delta_time);

      tx_count++;
    } else {
      LOG_INFO("Not reachable yet\n");
      if(tx_count > 0) {
        missed_tx_count++;
      }
    }
    etimer_reset(&periodic_timer);
  }

  cc2420_off(); // TURNING CC2420 OFF

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
