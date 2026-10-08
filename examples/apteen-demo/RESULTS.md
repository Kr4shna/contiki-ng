# Results: APTEEN-style vs. baseline (10 simulated minutes)

Setup: Cooja, UDGM radio (50 m range), 1 sink (node 1) + 4 sensor clients
(nodes 2-5) placed 30 m from the sink, random seed 123456, sample every 2 s.
Raw logs are in `results/apteen/` and `results/baseline/`.

## Totals (all 4 clients)

| Metric                     | APTEEN-style | Baseline | Reduction |
|----------------------------|-------------:|---------:|----------:|
| Reports received at sink   |          100 |    1 166 |     91.4% |
| Estimated TX energy (µJ)   |        1 631 |   19 126 |     91.5% |
| Energy per sample (µJ)     |         1.36 |    16.35 |     91.7% |

## Per node (last STATS line)

| Node | APTEEN sent / samples | APTEEN energy (µJ) | Baseline sent / samples | Baseline energy (µJ) |
|-----:|----------------------:|-------------------:|------------------------:|---------------------:|
| 2    |  17 / 300             |  276               | 277 / 285               | 4 654                |
| 3    |  32 / 300             |  523               | 292 / 300               | 4 909                |
| 4    |  18 / 300             |  292               | 291 / 300               | 4 892                |
| 5    |  33 / 300             |  540               | 278 / 285               | 4 671                |

## How to read this

- Both modes take the same number of samples; APTEEN suppresses about 92%
  of them because the value is below the hard threshold (30.0 C) or has not
  changed by the soft threshold (1.0 C). The count time (60 s) still forces
  a periodic report, so the sink never loses track of a node.
- Nodes 3 and 5 sent about twice as many reports as nodes 2 and 4 because
  their simulated temperature crossed 30 C more often (more heat spikes).
- Baseline "sent" is slightly below "samples" because the first few
  samples happen before RPL has formed ("Not reachable yet").
- Some baseline rows show 285 samples because STATS is printed every 15
  samples and the run ended just before the next one. That is also why
  the sink count (1 166) is a little higher than the sum of the last STATS
  lines.
- Energy is an estimate from the first-order radio model (transmit side
  only), not a measurement.
