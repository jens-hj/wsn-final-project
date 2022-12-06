// AES encryption module
#undef AES_128_CONF
// #define AES_128_CONF cc2420_aes_128_driver // hardware AES
#define AES_128_CONF aes_128_driver // software AES
// #define RADIO radio_driver

// Energest energy estimation module
#define ENERGEST_CONF_ON 1
#define SIMPLE_ENERGEST_CONF_PERIOD (10 * CLOCK_SECOND)