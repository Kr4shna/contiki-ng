/*
 * apteen-client.c : sensor node with APTEEN-style reporting.
 *
 * Reporting rules (APTEEN = TEEN thresholds + periodic count time):
 *   - Hard threshold (HT): only report when the sensed value >= HT.
 *   - Soft threshold (ST): and only if it changed by >= ST since the
 *     last value this node transmitted.
 *   - Count time (TC): if nothing was transmitted for TC seconds,
 *     report anyway (this is the periodic part that makes it APTEEN).
 *
 * APTEEN_MODE 1 : APTEEN-style reporting (default)
 * APTEEN_MODE 0 : baseline, report every sample (LEACH-style periodic)
 *
 * NOTE: this implements APTEEN's reporting mechanism only, not its
 * cluster formation. Energy is estimated with the first-order radio
 * model used in the WSN literature, counted per transmitted packet.
 */
#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "Sensor"
#define LOG_LEVEL LOG_LEVEL_INFO

#ifndef APTEEN_MODE
#define APTEEN_MODE 1
#endif

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

/* ---- Sensing and APTEEN parameters (values in tenths of a degree C) ---- */
#define SAMPLE_INTERVAL  (2 * CLOCK_SECOND)
#define HARD_THRESHOLD   300   /* 30.0 C */
#define SOFT_THRESHOLD   10    /* 1.0 C  */
#define COUNT_TIME       (60 * CLOCK_SECOND)
#define STATS_EVERY      15    /* print stats every N samples */

/* ---- First-order radio energy model (picojoules) ---- */
#define E_ELEC_PJ_PER_BIT   50000UL  /* 50 nJ/bit  */
#define EPS_FS_PJ_PER_BIT_M2 10UL    /* 10 pJ/bit/m^2 */
#define LINK_DISTANCE_M     30UL     /* assumed hop distance */
#define PKT_OVERHEAD_BYTES  25UL     /* approx. 802.15.4 + 6LoWPAN/UDP header */

static struct simple_udp_connection udp_conn;

PROCESS(apteen_client_process, "APTEEN sensor");
AUTOSTART_PROCESSES(&apteen_client_process);
/*---------------------------------------------------------------------------*/
/* Simulated temperature: bounded random walk with occasional hot spells. */
static int
read_sensor(void)
{
  static int temp = 280;
  static int hot_left = 0;

  temp += (int)(random_rand() % 11) - 5;        /* step -0.5 .. +0.5 C */
  if(hot_left == 0 && (random_rand() % 60) == 0) {
    hot_left = 8;                                /* event: heat spike */
  }
  if(hot_left > 0) {
    temp += 6;
    hot_left--;
  } else if(temp > 290) {
    temp -= 3;                                   /* cool back down */
  }
  if(temp < 200) temp = 200;
  if(temp > 420) temp = 420;
  return temp;
}
/*---------------------------------------------------------------------------*/
static uint64_t
tx_energy_pj(uint32_t payload_len)
{
  uint64_t bits = (uint64_t)(payload_len + PKT_OVERHEAD_BYTES) * 8;
  uint64_t per_bit = E_ELEC_PJ_PER_BIT
    + EPS_FS_PJ_PER_BIT_M2 * LINK_DISTANCE_M * LINK_DISTANCE_M;
  return bits * per_bit;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(apteen_client_process, ev, data)
{
  static struct etimer sample_timer;
  static char str[40];
  static uip_ipaddr_t dest_ipaddr;
  static uint32_t samples = 0, sent = 0, suppressed = 0;
  static uint64_t energy_pj = 0;
  static int last_sent_val = -10000;
  static clock_time_t last_tx_time = 0;
  int val, diff;
  int should_send;

  PROCESS_BEGIN();

  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, NULL);
  last_tx_time = clock_time();
  etimer_set(&sample_timer, random_rand() % SAMPLE_INTERVAL + 1);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sample_timer));

    val = read_sensor();
    samples++;

#if APTEEN_MODE
    diff = val - last_sent_val;
    if(diff < 0) diff = -diff;
    should_send = 0;
    if(val >= HARD_THRESHOLD && diff >= SOFT_THRESHOLD) {
      should_send = 1;                                   /* TEEN rule */
    } else if((clock_time() - last_tx_time) >= COUNT_TIME) {
      should_send = 1;                                   /* APTEEN count time */
    }
#else
    (void)diff;
    (void)last_sent_val;
    (void)last_tx_time;
    should_send = 1;                                     /* baseline */
#endif

    if(should_send) {
      if(NETSTACK_ROUTING.node_is_reachable() &&
         NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
        snprintf(str, sizeof(str), "T=%d.%d #%" PRIu32,
                 val / 10, val % 10, sent);
        simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
        energy_pj += tx_energy_pj(strlen(str));
        sent++;
        last_sent_val = val;
        last_tx_time = clock_time();
      } else {
        LOG_INFO("Not reachable yet\n");
      }
    } else {
      suppressed++;
    }

    if(samples % STATS_EVERY == 0) {
      LOG_INFO("STATS mode=%d samples=%" PRIu32 " sent=%" PRIu32
               " suppressed=%" PRIu32 " tx_energy_uJ=%" PRIu32 "\n",
               APTEEN_MODE, samples, sent, suppressed,
               (uint32_t)(energy_pj / 1000000ULL));
    }

    etimer_set(&sample_timer, SAMPLE_INTERVAL);
  }

  PROCESS_END();
}
