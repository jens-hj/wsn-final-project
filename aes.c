#include "project-conf.h"
#include "contiki.h"
// #include "cc2420-aes.h"
#include "lib/aes-128.h"
#include "dev/radio.h"
#include "cc2420.h"
#include <stdio.h>

PROCESS(aes_process, "aes process");
AUTOSTART_PROCESSES(&aes_process);

PROCESS_THREAD(aes_process, ev, data) {
  PROCESS_BEGIN();
  cc2420_init();
  cc2420_on();

  // sequence to be encrypted
  uint8_t vals[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  // char* str = "This is a test";

  // 128 bit key
  // user defined key
  // int key_length = 16;
  uint8_t key[16] = {5, 0, 7, 6, 9, 9, 6, 2, 9, 1, 3, 8, 6, 8, 4, 0};

  // cc2420_aes_set_key(key, 0);
  // cc2420_aes_cipher(vals, N, 0);

  AES_128.set_key(key);
  AES_128.encrypt(vals);

  for (int i = 0; i < 16; i++) {
    printf("%d ", vals[i]);
  }
  printf("\n");

  // decrypt
  AES_128.encrypt(vals);

  // print decrypted
  for (int i = 0; i < 16; i++) {
    printf("%d ", vals[i]);
  }
  printf("\n");

  cc2420_off();
  
  PROCESS_END();
}