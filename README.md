# Elastic Sketch — Network Telemetry Engine

A memory-efficient network flow monitoring system built in **Pure C**, featuring a **real-time telemetry dashboard** powered by **Python, Streamlit, and Plotly**.
Designed to identify heavy network flows with high accuracy while using only **~152 KB of memory**.

Elastic Sketch combines **Heavy Guardian** (catches likely heavy flows precisely) and **Count-Min Sketch** (estimates the rest with shared counters) to detect heavy network flows using constant memory.


## Metrics

| Metric | Meaning |
|---|---|
| Throughput | Packets processed per second (Mpps) |
| Precision | % of flagged heavy hitters that were actually heavy |
| Light MRE | Error on small-flow size estimates (lower = better) |
| Occupancy | How full the Heavy Guardian table is |

## Run locally

```bash
make
pip install -r requirements.txt
streamlit run app.py
```

---
