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
  unsigned char plaintext[AES_128_BLOCK_SIZE] = "this is a test 1";
  // char* str = "This is a test";

  // for (int i = 0; i < 16; i++) {
  //   printf("%d ", (char)plaintext[i]);
  // }
  // printf("\n");
  printf("plaintext: %s\n", plaintext);

  // 128 bit key
  // user defined key
  // int key_length = 16;
  uint8_t key[AES_128_KEY_LENGTH] = {5, 0, 7, 6, 9, 9, 6, 2, 9, 1, 3, 8, 6, 8, 4, 0};

  // cc2420_aes_set_key(key, 0);
  // cc2420_aes_cipher(plaintext, N, 0);

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

  // print decrypted
  // for (int i = 0; i < 16; i++) {
  // printf("%c ", (char)plaintext[i]);
  // }
  // printf("\n");

  cc2420_off();
  
  PROCESS_END();
}