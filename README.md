# Elastic Sketch — Network Telemetry Engine

A memory-efficient network flow monitoring system built in Pure C.
Designed to handle high-speed traffic streams where traditional approaches
run out of memory or lose accuracy.

---

## 🚦 Problem

Modern networks generate traffic faster than traditional systems can monitor.

- **Hash Map (Exact Tracking)** — Perfect accuracy and fast lookups, but memory grows as **O(N)** with the number of flows, making it difficult to scale.

- **Count-Min Sketch (Approximate Tracking)** — Uses a fixed amount of memory and remains fast, but can suffer from high estimation errors due to hash collisions.

- **Elastic Sketch (This Project)** — Maintains a fixed memory footprint of **~152 KB**, achieves **~99% accuracy**, and processes up to **236M packets/sec** by combining exact and approximate flow tracking.
---

## Architecture
Two layers work together in a single pipeline:

Two layers work together in a single pipeline.

### Layer 1 — Heavy Guardian

Every packet is hashed into a bucket where one of three actions occurs:

- Same flow → increment its counter
- Different flow → decrease the reward value
- Reward reaches zero → evict the weaker flow and install the new one

This mechanism is called **Vote-and-Demote**. Frequently occurring flows continue receiving votes and stay in memory, while less important flows are gradually removed.

### Layer 2 — Count-Min Sketch

Flows removed from Heavy Guardian are forwarded to the Count-Min Sketch.

- Flow is hashed into 4 counter arrays
- Corresponding counters are incremented
- Queries return the minimum counter value

Using the minimum counter helps reduce overestimation caused by hash collisions.

**Result:** Important flows receive near-exact tracking while less significant flows are estimated efficiently within a fixed memory budget.
---

## 📊 Benchmark Results

```text
====================================
 ELASTIC SKETCH - NETWORK TELEMETRY
====================================
Total Packets Processed : 1,000,000
Time Elapsed            : 0.004 sec
Throughput              : ~236,568,806 pkt/s

Precision               : 100.0%
Recall                  : 98.8%

Mean Relative Error     : 22.15%  [ flows seen >= 10 packets ]
  Heavy flows  (> 500)  :  1.21%
  Mid flows    (>  50)  : 11.84%
  Light flows  (<= 50)  : 27.42%

Memory Footprint:
  Heavy Guardian        : 2048 buckets  (~24 KB)
  Count-Min Sketch      : 4 × 8192      (~128 KB)
  Total                 : ~152 KB
====================================
```

---

## 🛠️ Tech Stack

- **Language** — Pure C no external libraries
- **Hashing** — MurmurHash3 for efficient and uniform bucket distribution
- **Traffic Simulation** — Zipfian Distribution (α = 1.5) to mimic real internet patterns
- **Build** — GCC with O2 optimisation
- **Timing** — POSIX `clock_gettime` at nanosecond precision

---

##  📁 Project Structure

```
Elastic_Sketch_Netwrok_Telemetry/
├── src/
│   ├── heavy_guardian.c/h    ← Vote-and-Demote heavy hitter tracking
│   ├── count_min_sketch.c/h  ← Probabilistic light flow estimation
│   ├── elastic_sketch.c/h    ← Dual-pipeline coordinator
│   ├── traffic_gen.c/h       ← Zipfian synthetic traffic generator
│   └── main.c                ← Benchmarking, metrics, output
├── tests/
│   └── benchmark.c           ← Multi-config comparison suite
├── Makefile
└── README.md
```

---

##  🚀  Build & Run

### Prerequisites
- GCC (Linux/Mac built-in · Windows: [MinGW-w64](https://winlibs.com))

### Build & Run
```bash
# Main program
gcc -Wall -O2 -std=c99 -o elastic_sketch \
  src/heavy_guardian.c src/count_min_sketch.c \
  src/elastic_sketch.c src/traffic_gen.c src/main.c -lm
./elastic_sketch

# Benchmark suite
gcc -Wall -O2 -std=c99 -o benchmark \
  src/heavy_guardian.c src/count_min_sketch.c \
  src/elastic_sketch.c src/traffic_gen.c tests/benchmark.c -lm
./benchmark
```

### Windows (PowerShell)
```powershell
gcc -Wall -O2 -std=c99 -o elastic_sketch.exe src/heavy_guardian.c src/count_min_sketch.c src/elastic_sketch.c src/traffic_gen.c src/main.c -lm
.\elastic_sketch.exe
```

---

## 🧠 Key Concepts

- **Heavy Guardian** — Keeps track of the most active network flows with high accuracy.
- **Count-Min Sketch** — Estimates frequencies using fixed memory.
- **Vote-and-Demote** — Important flows stay in memory while weaker flows are gradually replaced.
- **Zipfian Traffic** — Models real-world traffic where a small number of flows dominate.
- **O(1) Processing** — Every packet updates a fixed number of counters regardless of traffic volume.

---

