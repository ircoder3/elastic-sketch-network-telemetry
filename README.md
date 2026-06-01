# Elastic Sketch — Network Telemetry Engine

A memory-efficient network flow monitoring system built in Pure C.
Designed to handle high-speed traffic streams where traditional approaches
run out of memory or lose accuracy.

---

## The Problem

Modern networks generate traffic faster than traditional systems can monitor.

| Approach | Memory | Accuracy | Speed |
|---|---|---|---|
| Hash Map (exact) | O(N) → crashes | Perfect | Fast |
| Count-Min Sketch(approximate) | Fixed,small | High error | Fast |
| **Elastic Sketch(this project)** | **Fixed ~152KB** | **~99%** | **236M pkt/s** |

---

## Architecture
Two layers work together in a single pipeline:

**Layer 1 — Heavy Guardian** (exact tracking)
```
Every packet → hashed to a bucket → one of three outcomes:
   Same flow?        → increment its counter
   Different flow?  → decrement the reward field
   Reward hits 0?    → evict weak flow to Layer 2, install new flow
```
This is called **Vote-and-Demote**. Frequent flows survive by repeatedly winning votes. Weak flows get pushed out.

**Layer 2 — Count-Min Sketch** (approximate tracking)
```
Evicted flows → hashed 4 ways → 4 counters incremented
Query         → take the minimum of all 4 counters
```
The minimum reduces overestimation from hash collisions.

**Together:** heavy hitters get exact counts, everything else gets a good-enough estimate — all within fixed memory.

---

## Benchmark Results

```
====================================
 ELASTIC SKETCH - NETWORK TELEMETRY
====================================
Total Packets Processed : 1,000,000
Time Elapsed            : 0.004 sec
Throughput              : ~236,568,806 pkt/s

Precision               : 100.0%
Recall                  : 98.8%

Mean Relative Error     : 22.15%  [ flows seen >= 10 packets]
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

- **Language** — Pure C (C99), no external libraries
- **Hashing** — MurmurHash3 finalizer for uniform bucket distribution
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

## to Build & Run

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

## Key Concepts
**Why not just use a Hash Map?**
Hash maps give perfect accuracy but need memory proportional to the number of unique flows. At 10 million flows per second, that's gigabytes — not viable on network hardware.

**Heavy Guardian** — tracks the most frequent flows with exact counts using a hash-indexed bucket array. Each bucket has a `reward` field; competing flows vote against each other, and the loser gets evicted to the Light Part with its full count preserved.

**Count-Min Sketch** — CMS uses fixed memory but treats all flows equally. Heavy hitters get mixed up with light flows in the same counters, causing high error on the flows you actually care about.

**Zipfian Traffic** — real internet traffic follows a power law: a small number of flows generate the vast majority of packets. This generator replicates that with configurable skew (α = 1.5 by default).

**Why O(1)?** — every packet touches exactly one Heavy bucket and at most 4 CMS counters, regardless of total traffic volume.

---

