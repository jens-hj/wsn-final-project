// AES encryption module
#undef AES_128_CONF
#define AES_128_CONF cc2420_aes_128_driver // hardware AES
// #define AES_128_CONF aes_128_driver // software AES
// #define RADIO radio_driver

// Energest energy estimation module
#define ENERGEST_CONF_ON 1
#define SIMPLE_ENERGEST_CONF_PERIOD (10 * CLOCK_SECOND)

// If all flags, full energy will be measured
// If specific time and energy flag, nothing will be measured
#define SINGLE_MOTE_TESTING
#define MEASURE_ENERGY_FULL

// #define MEASURE_TIME_SENSOR
#define MEASURE_ENERGY_SENSOR

// #define ENCRYPT_WITH_AES
// #define MEASURE_TIME_AES
// #define MEASURE_ENERGY_AES

// #define MEASURE_TIME_TX
// #define MEASURE_ENERGY_TX