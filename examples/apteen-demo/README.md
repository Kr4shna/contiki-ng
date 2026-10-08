# APTEEN-style threshold reporting in Cooja (Contiki-NG)

Compile-tested against current Contiki-NG (native and cooja targets, no warnings).

## Demo (ready-made simulations)
Two saved simulations are included: `apteen-sim.csc` (experiment A) and
`baseline-sim.csc` (experiment B). Both use the same layout: sink (node 1)
in the middle, 4 sensor clients (nodes 2-5) 30 m away.

1. `cd ~/dev/contiki-ng/tools/cooja && ./gradlew run`
2. File -> Open simulation -> `examples/apteen-demo/apteen-sim.csc`
   (Cooja compiles the motes; you should see 5 nodes)
3. Press Start. Watch the `STATS` lines in the Log Listener window.
   Increase speed in Simulation control to finish faster.
4. After 10 simulated minutes the simulation stops by itself and the
   Script editor window at the bottom prints a SUMMARY with totals.
5. Repeat with `baseline-sim.csc` and compare the two summaries.

"Not reachable yet" during the first 1-2 minutes is normal (RPL forming).

Expected result (see RESULTS.md): APTEEN sends about 91% fewer reports and
uses about 91% less estimated TX energy than the baseline.

To rerun both experiments without the GUI (results go to `results/`):

    cd ~/dev/contiki-ng
    for s in apteen baseline; do ./tools/cooja/gradlew -q -p tools/cooja run --args="--no-gui --contiki=$PWD --logdir=$PWD/examples/apteen-demo/results/$s $PWD/examples/apteen-demo/$s-sim.csc"; done

## Building a simulation by hand (alternative)
1. File -> New simulation (radio medium UDGM) -> Create
2. Motes -> Add motes -> Create new mote type -> Cooja mote ->
   `apteen-server.c` -> Compile -> Create -> add 1 mote
3. Same for `apteen-client.c` (or `baseline-client.c`) -> add 4 motes
4. Place every client within about 40 m of the server, open
   Tools -> Log Listener, filter on `STATS`, and run about 10 minutes.

## Parameters (top of apteen-client.c)
HARD_THRESHOLD 300 (30.0 C), SOFT_THRESHOLD 10 (1.0 C), COUNT_TIME 60 s,
SAMPLE_INTERVAL 2 s. Change them and rerun to show sensitivity.

## Honest scope note for the presentation
This implements APTEEN's reporting mechanism (HT, ST, count time), not its
cluster formation. Energy is an estimate from the first-order radio model
(50 nJ/bit electronics, 10 pJ/bit/m^2 amplifier, 30 m hop), counted per
transmitted packet.
