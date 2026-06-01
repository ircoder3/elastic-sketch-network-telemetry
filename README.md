# Elastic Sketch — Network Telemetry Engine

A memory-efficient network flow monitoring system built in Pure C.

Designed to identify heavy network flows with high accuracy while using only **~152 KB of memory**.

---

## 🚀 Features

- Detects heavy-hitter network flows with near-exact accuracy
- Fixed memory footprint (~152 KB)
- Processes up to **236M packets/sec**
- Combines **Heavy Guardian** and **Count-Min Sketch**
- Simulates realistic traffic using **Zipfian distributions**
- Pure C implementation with no external dependencies

---

## 🚦 Problem

Modern networks generate traffic faster than traditional systems can monitor efficiently.

- **Hash Map (Exact Tracking)** → Accurate, but memory usage grows with the number of network flows.

- **Count-Min Sketch (Approximate Tracking)** → Uses fixed memory, but introduces estimation errors due to hash collisions.

- **Elastic Sketch (This Project)** → Maintains only **~152 KB memory**, achieves **~99% accuracy**, and processes **236M packets/sec**.

---

## 🛠️ Tech Stack

- **Language** — Pure C (no external libraries)
- **Hashing** — MurmurHash3
- **Traffic Simulation** — Zipfian Distribution (α = 1.5)
- **Compiler** — GCC (O2 Optimisation)
- **Timing** — POSIX `clock_gettime()`

---

## 🏗️ Architecture

Two layers work together in a single pipeline:

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

Performance measured using **1,000,000 synthetic packets** generated from a Zipfian traffic distribution.

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

## 📁 Project Structure

```text
elastic-sketch-network-telemetry/
├── src/
│   ├── heavy_guardian.c/h
│   ├── count_min_sketch.c/h
│   ├── elastic_sketch.c/h
│   ├── traffic_gen.c/h
│   └── main.c
│
├── tests/
│   └── benchmark.c
│
├── Makefile
└── README.md
```

---

## 📦 Installation

### Prerequisites

- GCC Compiler
- MinGW-w64 (Windows)

### Clone Repository

```bash
git clone https://github.com/ircoder3/elastic-sketch-network-telemetry.git
cd elastic-sketch-network-telemetry
```

### 1️⃣ Main Program

```bash
gcc -Wall -O2 -std=c99 -o elastic_sketch \
  src/heavy_guardian.c src/count_min_sketch.c \
  src/elastic_sketch.c src/traffic_gen.c src/main.c -lm

./elastic_sketch
```

### 2️⃣ Benchmark Suite

```bash
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