/*
 * Stops the simulation after 10 simulated minutes and prints a summary
 * of the last STATS line from every client plus totals.
 */
var last = {};
var rx = 0;

function finish() {
  var t = { samples: 0, sent: 0, suppressed: 0, energy: 0 };
  log.log("\n===== SUMMARY after " + (time / 1000000) + " simulated seconds =====\n");
  for (var node in last) {
    var m = last[node].match(/mode=(\d+) samples=(\d+) sent=(\d+) suppressed=(\d+) tx_energy_uJ=(\d+)/);
    if (!m) continue;
    log.log("node " + node + ": samples=" + m[2] + " sent=" + m[3] +
            " suppressed=" + m[4] + " tx_energy_uJ=" + m[5] + "\n");
    t.samples += parseInt(m[2]); t.sent += parseInt(m[3]);
    t.suppressed += parseInt(m[4]); t.energy += parseInt(m[5]);
  }
  log.log("TOTAL: samples=" + t.samples + " sent=" + t.sent +
          " suppressed=" + t.suppressed + " tx_energy_uJ=" + t.energy + "\n");
  log.log("sink received " + rx + " reports\n");
  log.testOK();
}

var END_US = 600 * 1000000;  /* 10 simulated minutes */
TIMEOUT(700000);               /* safety net only */

while (true) {
  YIELD();
  if (time >= END_US) {
    finish();
    break;
  }
  if (msg.indexOf("STATS") >= 0) {
    last[id] = msg;
  } else if (id == 1 && msg.indexOf("RX #") >= 0) {
    rx++;
  }
}
