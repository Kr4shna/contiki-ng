/*
 * apteen-server.c : sink / RPL root for the APTEEN-style demo.
 * Receives sensor reports and counts them. No echo reply, so the
 * traffic (and energy) comparison between the two clients stays clean.
 */
#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdint.h>
#include <inttypes.h>

#include "sys/log.h"
#define LOG_MODULE "Sink"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;
static uint32_t rx_total = 0;

PROCESS(apteen_server_process, "APTEEN sink");
AUTOSTART_PROCESSES(&apteen_server_process);
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
  rx_total++;
  LOG_INFO("RX #%" PRIu32 " '%.*s' from ", rx_total, datalen, (char *)data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(apteen_server_process, ev, data)
{
  PROCESS_BEGIN();

  NETSTACK_ROUTING.root_start();
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}
