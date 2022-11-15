#include "project-conf.h"
#include "contiki.h"
// #include "cc2420-aes.h"
#include "lib/aes-128.h"
#include "dev/radio.h"
#include "cc2420.h"
#include "sys/energest.h"
#include <stdio.h>

PROCESS(aes_process, "aes process");
AUTOSTART_PROCESSES(&aes_process);

PROCESS_THREAD(aes_process, ev, data) {
  PROCESS_BEGIN();
  energest_init();

  cc2420_init();
  cc2420_on();

  // sequence to be encrypted
  unsigned char plaintext[AES_128_BLOCK_SIZE] = "this is a test 1";
  printf("plaintext: %s\n", plaintext);

  // 128 bit key
  uint8_t key[AES_128_KEY_LENGTH] = {5, 0, 7, 6, 9, 9, 6, 2, 9, 1, 3, 8, 6, 8, 4, 0};

  AES_128.set_key(key);
  AES_128.encrypt(plaintext);
  printf("encrypted: %s\n", plaintext);

  printf("encrypted: ");
  for (int i = 0; i < AES_128_BLOCK_SIZE; i++) {
  printf("%02x ", plaintext[i]);
  }
  printf("\n");

  // decrypt
  AES_128.encrypt(plaintext);
  printf("decrypted: %s\n", plaintext);

  cc2420_off();
  // print the ticks per second for energest and statistics
  // slide 31-32, lecture 2
  printf("Energest ticks per second: %d", ENERGEST_SECOND);
  energest_flush();
  
  PROCESS_END();
}