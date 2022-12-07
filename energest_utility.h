/*---------------------------------------------------------------------------*/
// os/services/simple-energest.c
static uint64_t last_tx, last_rx, last_time, last_cpu, last_lpm, last_deep_lpm;
void
custom_energest_init(void)
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
custom_energest_step(void)
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