# APTEEN-style threshold reporting in Cooja (Contiki-NG)

Compile-tested against current Contiki-NG (native target, no errors).

## 1. Install the project
Copy this whole folder into your Contiki-NG examples directory:

    cp -r ~/Downloads/apteen-demo ~/dev/contiki-ng/examples/apteen-demo

(adjust the Downloads path to where the folder actually is)

## 2. Run experiment A: APTEEN-style
1. `cd ~/dev/contiki-ng/tools/cooja && ./gradlew run`
2. File -> New simulation (keep defaults, radio medium UDGM) -> Create
3. Motes -> Add motes -> Create new mote type -> Cooja mote
   - Browse to `examples/apteen-demo/apteen-server.c` -> Compile -> Create -> add 1 mote
4. Repeat for `examples/apteen-demo/apteen-client.c` -> add 4 motes
5. Drag motes in the Network window so every client is within about 40 m of the server
6. Open Tools -> Mote output (or Log Listener); filter on `STATS`
7. Start the simulation and let it run about 10 simulated minutes
   (the RPL network needs 1-2 minutes to form; "Not reachable yet" at the start is normal)
8. Note the last `STATS` line per client: samples / sent / suppressed / tx_energy_uJ

## 3. Run experiment B: baseline
Make a new simulation with the same layout, but use `baseline-client.c`
for the 4 client motes. Run the same duration and note the STATS lines.

## 4. Compare
Same samples, far fewer `sent` and much lower `tx_energy_uJ` in experiment A.
That is the energy saving from hard threshold + soft threshold + count time.

## Parameters (top of apteen-client.c)
HARD_THRESHOLD 300 (30.0 C), SOFT_THRESHOLD 10 (1.0 C), COUNT_TIME 60 s,
SAMPLE_INTERVAL 2 s. Change them and rerun to show sensitivity.

## Honest scope note for the presentation
This implements APTEEN's reporting mechanism (HT, ST, count time), not its
cluster formation. Energy is an estimate from the first-order radio model
(50 nJ/bit electronics, 10 pJ/bit/m^2 amplifier, 30 m hop), counted per
transmitted packet.
