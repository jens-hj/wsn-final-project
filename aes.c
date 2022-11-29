#include "project-conf.h"
#include "contiki.h"
#include "lib/aes-128.h"
#include "dev/radio.h"
#include "cc2420.h"
#include "sys/energest.h"
#include "sys/log.h"
#include <stdio.h>

#define LOG_MODULE "AES"
#define LOG_LEVEL LOG_LEVEL_INFO

PROCESS(aes_process, "aes process");
AUTOSTART_PROCESSES(&aes_process);

PROCESS_THREAD(aes_process, ev, data) {
  PROCESS_BEGIN();
  energest_init();
  energest_flush();

  cc2420_init();
  cc2420_on();

  // sequence to be encrypted
  unsigned char plaintext[AES_128_BLOCK_SIZE] = "this is a test 1";
  LOG_INFO("plaintext: %s", plaintext);

  // 128 bit key
  uint8_t key[AES_128_KEY_LENGTH] = {5, 0, 7, 6, 9, 9, 6, 2, 9, 1, 3, 8, 6, 8, 4, 0};

  AES_128.set_key(key);
  AES_128.encrypt(plaintext);
  LOG_INFO("encrypted: %s\n", plaintext);

  LOG_INFO("encrypted: ");
  for (int i = 0; i < AES_128_BLOCK_SIZE; i++) {
    LOG_INFO_("%02x", plaintext[i]);
  }
  LOG_INFO_("\n");

  // decrypt
  AES_128.encrypt(plaintext);
  LOG_INFO("decrypted: %s\n", plaintext);

  cc2420_off();
  // print the ticks per second for energest and statistics
  // slide 31-32, lecture 2
  LOG_INFO("Energest ticks per second: %u\n", ENERGEST_SECOND);
  energest_flush();

  // print the energest values separately
  unsigned long int cpu_time = energest_type_time(ENERGEST_TYPE_CPU);
  unsigned long int lpm_time = energest_type_time(ENERGEST_TYPE_LPM);
  unsigned long int deep_lpm_time = energest_type_time(ENERGEST_TYPE_DEEP_LPM);
  unsigned long int rx_time = energest_type_time(ENERGEST_TYPE_LISTEN);
  unsigned long int tx_time = energest_type_time(ENERGEST_TYPE_TRANSMIT);
  uint64_t total_time = ENERGEST_GET_TOTAL_TIME();
  // unsigned long int current_time = energest_type_time(ENERGEST_TYPE_MAX);
  
  // print
  LOG_INFO("ENERGEST: CPU time: %lu\n", cpu_time);
  LOG_INFO("LPM time: %lu\n", lpm_time);
  LOG_INFO("Deep LPM time: %lu\n", deep_lpm_time);
  LOG_INFO("RX time: %lu\n", rx_time);
  LOG_INFO("TX time: %lu\n", tx_time);
  LOG_INFO("Total time: %llu\n", total_time);
  // printf("Current time: %lu", current_time);
  
  PROCESS_END();
}