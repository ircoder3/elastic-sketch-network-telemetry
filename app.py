"""
app.py — Elastic Sketch Network Telemetry
"""
import os, time, ctypes
import numpy as np
import streamlit as st
import plotly.graph_objects as go
from plotly.subplots import make_subplots

# ── load DLL ──────────────────────────────────────────────
BASE = os.path.dirname(os.path.abspath(__file__))
DLL  = os.path.join(BASE, "elastic_sketch.dll")
SO   = os.path.join(BASE, "elastic_sketch.so")
PATH = DLL if os.path.exists(DLL) else SO

import os

try:
    # 1. Explicitly tell Windows this directory is safe for DLLs
    if hasattr(os, 'add_dll_directory'):
        os.add_dll_directory(os.path.dirname(os.path.abspath(__file__)))
    
    # 2. Load the DLL using winmode=0 to allow dependency loading
    lib = ctypes.CDLL(PATH, winmode=0)

except OSError as e:
    st.error(f"DLL Load Error: {e}")
    st.stop()

class TelData(ctypes.Structure):
    _fields_ = [
        ("throughput",       ctypes.c_double),
        ("active_guardians", ctypes.c_uint32),
        ("total_evictions",  ctypes.c_uint32),
    ]

lib.init_system.argtypes   = []
lib.init_system.restype    = None
lib.free_system.argtypes   = []
lib.free_system.restype    = None
lib.process_chunk.argtypes = [ctypes.POINTER(ctypes.c_uint32), ctypes.c_int]
lib.process_chunk.restype  = TelData
lib.query_flow.argtypes    = [ctypes.c_uint32]
lib.query_flow.restype     = ctypes.c_uint32
lib.get_mre.argtypes       = [
    ctypes.POINTER(ctypes.c_uint32),
    ctypes.POINTER(ctypes.c_uint32),
    ctypes.c_int
]
lib.get_mre.restype = ctypes.c_double

# ── page config ────────────────────────────────────────────
st.set_page_config(
    page_title="Elastic Sketch Network Telemetry",
    page_icon="🌐",
    layout="wide",
    initial_sidebar_state="collapsed"
)

# ── CSS ────────────────────────────────────────────────────
st.markdown("""
<style>
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&family=JetBrains+Mono:wght@400;500;600&display=swap');

html, body, [class*="css"] {
    font-family: 'Inter', sans-serif !important;
}
.mono { font-family: 'JetBrains Mono', monospace !important; }

/* ── Global Theme (Slate Dark) ── */
.main { background: #0b1120; }
.block-container {
    padding-top: 4rem !important;
    padding-bottom: 2rem !important;
    max-width: 100% !important;
}

section[data-testid="stSidebar"] { display: none !important; }

/* ── heading block ── */
.page-heading {
    border-bottom: 2px solid #1e293b;
    padding-bottom: 16px;
    margin-bottom: 20px;
    margin-top: 10px;
}
.page-title {
    font-family: 'Inter', sans-serif;
    font-size: 32px; 
    font-weight: 700;
    color: #ffffff;
    letter-spacing: 0.5px;
    margin: 0;
    line-height: 1.3;
}
.page-sub {
    font-family: 'JetBrains Mono', monospace;
    font-size: 13px;
    color: #94a3b8; 
    margin-top: 6px;
    letter-spacing: 0.08em;
}

/* ── param bar ── */
.param-bar {
    background: #0f172a;
    border: 1px solid #1e293b;
    border-radius: 10px;
    padding: 12px 18px 6px;
    margin-bottom: 14px;
}
.param-label {
    font-family: 'JetBrains Mono', monospace;
    font-size: 11px;
    font-weight: 600;
    color: #38bdf8; 
    letter-spacing: 0.1em;
    text-transform: uppercase;
    margin-bottom: 4px;
    white-space: nowrap;
}

/* ── metric cards ── */
div[data-testid="stMetric"] {
    background: #0f172a !important;
    border: 1px solid #1e293b !important;
    border-radius: 12px !important;
    padding: 16px !important;
    box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
}
div[data-testid="stMetricLabel"] p {
    color: #94a3b8 !important;
    font-family: 'JetBrains Mono', monospace !important;
    font-size: 12px !important;
    font-weight: 600 !important;
    letter-spacing: 0.08em !important;
    text-transform: uppercase !important;
}
div[data-testid="stMetricValue"] {
    color: #34d399 !important; 
    font-family: 'JetBrains Mono', monospace !important;
    font-size: 26px !important;
    font-weight: 600 !important;
}

/* ── buttons ── */
div[data-testid="stButton"] > button {
    font-family: 'Inter', sans-serif !important;
    font-size: 16px !important; 
    font-weight: 600 !important;
    border-radius: 8px !important;
    padding: 12px 24px !important; 
    transition: all .2s !important;
    height: auto !important;
}
/* Primary Button Override (Start) */
div[data-testid="stButton"] > button[kind="primary"] {
    background-color: #0284c7 !important;
    color: #f8fafc !important;
    border: 1px solid #0369a1 !important;
}
div[data-testid="stButton"] > button[kind="primary"]:hover {
    background-color: #0369a1 !important;
    border: 1px solid #0c4a6e !important;
}

/* ── log ── */
.log-box {
    background: #0f172a;
    border: 1px solid #1e293b;
    border-radius: 8px;
    padding: 12px 16px;
    font-size: 11px;
    font-family: 'JetBrains Mono', monospace;
    height: 180px;
    overflow-y: auto;
    color: #cbd5e1;
    line-height: 1.85;
}

/* ── conclusion cards ── */
.concl-card {
    background: #0f172a;
    border: 1px solid #1e293b;
    border-radius: 10px;
    padding: 16px;
    height: 100%;
}
.concl-title {
    font-family: 'Inter', sans-serif;
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 0.08em;
    text-transform: uppercase;
    margin-bottom: 8px;
}
.concl-body {
    font-family: 'Inter', sans-serif;
    font-size: 13px;
    color: #cbd5e1;
    line-height: 1.65;
}

/* ── section labels ── */
.sec-label {
    font-family: 'JetBrains Mono', monospace;
    font-size: 12px;
    font-weight: 600;
    color: #38bdf8;
    letter-spacing: 0.1em;
    text-transform: uppercase;
    margin-bottom: 12px;
}

/* ── footer ── */
.footer-text {
    text-align: center;
    font-family: 'JetBrains Mono', monospace;
    font-size: 11px;
    color: #64748b;
    letter-spacing: 0.1em;
    margin-top: 30px;
    padding-top: 15px;
    border-top: 1px solid #1e293b;
}
hr { border-color: #1e293b !important; margin: 2em 0 !important;}

/* slider value colour */
div[data-testid="stSlider"] p {
    color: #f8fafc !important;
    font-family: 'JetBrains Mono', monospace !important;
    font-size: 12px !important;
    font-weight: 500 !important;
}
</style>
""", unsafe_allow_html=True)

# ── session state ──────────────────────────────────────────
def init_state():
    defs = dict(
        running=False, initialized=False, tick=0,
        total_pkts=0, ground_truth={},
        throughput_hist=[], precision_hist=[],
        mre_hist=[], eviction_hist=[], log=[]
    )
    for k, v in defs.items():
        if k not in st.session_state:
            st.session_state[k] = v

init_state()

# ── Zipfian ────────────────────────────────────────────────
@st.cache_data
def zipf_probs(alpha, n):
    r = np.arange(1, n + 1, dtype=np.float64)
    p = 1.0 / np.power(r, alpha)
    return p / p.sum()

def gen_batch(alpha, n_flows, size):
    p = zipf_probs(alpha, n_flows)
    return np.random.choice(
        np.arange(1, n_flows + 1, dtype=np.uint32),
        size=size, p=p
    )

# ══════════════════════════════════════════════════════════
# PAGE LAYOUT
# ══════════════════════════════════════════════════════════

# ── 1. Heading ─────────────────────────────────────────────
head_l, head_r = st.columns([5, 2])
with head_l:
    st.markdown("""
    <div class="page-heading">
        <div class="page-title">🌐 Elastic Sketch Network Telemetry</div>
        <div class="page-sub">Real-Time Network Telemetry Dashboard</div>
    </div>
    """, unsafe_allow_html=True)
with head_r:
    badge_slot = st.empty()

def badge(text, fg, bg, border, pkts=None):
    # Added packet formatting to show alongside the status
    pkt_html = f"&nbsp;&nbsp;&nbsp;<span style='color:#38bdf8;'>{pkts:,} pkts</span>" if pkts is not None else ""
    badge_slot.markdown(
        f'<div style="background:{bg};color:{fg};border:1px solid {border};'
        f'padding:8px 16px;border-radius:24px;font-family:JetBrains Mono,'
        f'monospace;font-size:12px;font-weight:600;text-align:center;margin-top:10px;">'
        f'{text}{pkt_html}</div>',
        unsafe_allow_html=True
    )

badge("● Idle", "#94a3b8", "#0f172a", "#1e293b", st.session_state.total_pkts)

# ── 2. Parameter bar (horizontal, top) ────────────────────
st.markdown(
    "<div class='sec-label'>Dashboard Controls</div>",
    unsafe_allow_html=True
)

p1,p2,p3,p4,p5,p6,p7,p8 = st.columns([2,2,2,2,2,2,1.5,1.5])

with p1:
    st.markdown("<div class='param-label'>Zipf Alpha</div>", unsafe_allow_html=True)
    alpha = st.slider("alpha", 0.5, 2.5, 1.2, 0.1, label_visibility="collapsed")
with p2:
    st.markdown("<div class='param-label'>Unique Flows</div>", unsafe_allow_html=True)
    n_flows = st.slider("flows", 500, 10000, 2000, 500, label_visibility="collapsed")
with p3:
    st.markdown("<div class='param-label'>Batch Size</div>", unsafe_allow_html=True)
    batch_size = st.select_slider(
        "batch",
        options=[10_000, 25_000, 50_000, 100_000, 200_000],
        value=50_000,
        format_func=lambda x: f"{x:,}",
        label_visibility="collapsed"
    )
with p4:
    st.markdown("<div class='param-label'>Num Cycles</div>", unsafe_allow_html=True)
    sim_cycles = st.slider("cycles", 5, 100, 20, label_visibility="collapsed")
with p5:
    st.markdown("<div class='param-label'>Delay (ms)</div>", unsafe_allow_html=True)
    tick_delay = st.slider("delay", 50, 800, 200, 50, label_visibility="collapsed")
with p6:
    st.markdown("<div class='param-label'>HH Thresh (%)</div>", unsafe_allow_html=True)
    hh_pct = st.slider("hh", 0.1, 5.0, 1.0, 0.1, label_visibility="collapsed") / 100.0
with p7:
    st.markdown("<div class='param-label'>&nbsp;</div>", unsafe_allow_html=True)
    run_btn = st.button("▷ Start", type="primary", use_container_width=True)
with p8:
    st.markdown("<div class='param-label'>&nbsp;</div>", unsafe_allow_html=True)
    reset_btn = st.button("↺ Reset", use_container_width=True)

st.divider()

# ── 3. Metric cards ────────────────────────────────────────
mc = st.columns(5)
ph = [c.empty() for c in mc]

LABELS = ["◈ Throughput", "◉ Precision", "◌ Light MRE", "◎ Evictions", "◐ Occupancy"]
HELPS  = ["Packets processed per second", "Heavy hitter detection accuracy",
          "Mean relative error on light flows", "Total Vote-and-Demote evictions",
          "Active Heavy Guardian buckets"]

def show_metrics(tp=None, pr=None, mre=None, ev=None, occ=None):
    vals = [
        f"{tp:.2f} Mpps"  if tp  is not None else "—",
        f"{pr:.1f}%"      if pr  is not None else "—",
        f"{mre:.1f}%"     if mre is not None else "—",
        f"{ev:,}"         if ev  is not None else "—",
        f"{occ}/1024"     if occ is not None else "—",
    ]
    for p, l, h, v in zip(ph, LABELS, HELPS, vals):
        p.metric(l, v, help=h)

show_metrics()
st.divider()

# ── 4. Chart placeholders ──────────────────────────────────
row1 = st.columns([3, 2])
ph_tput = row1[0].empty()
ph_dist = row1[1].empty()

row2 = st.columns(2)
ph_hh  = row2[0].empty()
ph_acc = row2[1].empty()

ph_log   = st.empty()
ph_concl = st.empty()

# ── plot theme ─────────────────────────────────────────────
BG   = "#0b1120"
PLOT = "#0f172a"
FONT = "#94a3b8"
GRID = "#1e293b"
GRN  = "#34d399"
ACC  = "#38bdf8"
PUR  = "#a78bda"

def _base_layout(title, h):
    return dict(
        title=dict(text=title, font=dict(color=ACC, size=13, family="Inter", weight="bold")),
        paper_bgcolor=BG, plot_bgcolor=PLOT,
        font=dict(color=FONT, family="Inter", size=11),
        margin=dict(l=40, r=10, t=45, b=28),
        height=h
    )

def plot_throughput():
    h = st.session_state.throughput_hist
    if not h: return
    fig = go.Figure(go.Scatter(
        y=h, mode="lines", fill="tozeroy",
        line=dict(color=GRN, width=2.5),
        fillcolor="rgba(52, 211, 153, 0.1)"
    ))
    fig.update_layout(**_base_layout("Live Throughput (Mpps)", 220))
    fig.update_xaxes(showgrid=True, gridcolor=GRID)
    fig.update_yaxes(showgrid=True, gridcolor=GRID, rangemode="tozero")
    ph_tput.plotly_chart(fig, use_container_width=True, config={"displayModeBar": False})

def plot_dist(gt):
    if not gt: return
    top = sorted(gt.items(), key=lambda x: x[1], reverse=True)[:30]
    fig = go.Figure(go.Bar(
        x=[str(f) for f, _ in top],
        y=[c for _, c in top],
        marker_color=ACC,
        marker_opacity=0.8
    ))
    fig.update_layout(**_base_layout("Zipf Distribution (top 30 flows)", 220))
    fig.update_xaxes(showgrid=False, tickangle=45)
    fig.update_yaxes(showgrid=True, gridcolor=GRID)
    ph_dist.plotly_chart(fig, use_container_width=True, config={"displayModeBar": False})

def plot_hh(gt):
    if not gt: return
    top    = sorted(gt.items(), key=lambda x: x[1], reverse=True)[:10]
    labels = [f"Flow {f}" for f, _ in top]
    truths = [c for _, c in top]
    estims = [lib.query_flow(ctypes.c_uint32(f)) for f, _ in top]
    fig = go.Figure()
    fig.add_bar(name="Ground Truth", x=labels, y=truths, marker_color="rgba(56, 189, 248, 0.8)")
    fig.add_bar(name="Sketch Estimate", x=labels, y=estims, marker_color="rgba(52, 211, 153, 0.7)")
    lay = _base_layout("Top 10 Heavy Hitters — Truth vs Estimate", 300)
    lay["margin"]["b"] = 55
    lay["barmode"]     = "group"
    lay["legend"]      = dict(orientation="h", yanchor="bottom", y=1.02, font=dict(size=10))
    fig.update_layout(**lay)
    fig.update_xaxes(showgrid=False, tickangle=30)
    fig.update_yaxes(showgrid=True, gridcolor=GRID)
    ph_hh.plotly_chart(fig, use_container_width=True, config={"displayModeBar": False})

def plot_acc():
    mh  = st.session_state.mre_hist
    ph2 = st.session_state.precision_hist
    if not mh: return
    fig = make_subplots(specs=[[{"secondary_y": True}]])
    fig.add_trace(go.Scatter(
        y=ph2, name="Precision %", line=dict(color=GRN, width=2.5)
    ), secondary_y=False)
    fig.add_trace(go.Scatter(
        y=mh, name="Light MRE %", line=dict(color=ACC, width=2.5, dash="dot")
    ), secondary_y=True)
    lay = _base_layout("Accuracy over Time", 300)
    lay["margin"]["r"] = 40
    lay["legend"]      = dict(orientation="h", yanchor="bottom", y=1.02, font=dict(size=10))
    fig.update_layout(**lay)
    fig.update_xaxes(showgrid=True, gridcolor=GRID)
    ph_acc.plotly_chart(fig, use_container_width=True, config={"displayModeBar": False})

def render_log():
    lines = st.session_state.log[-35:]
    html  = ("<div class='log-box'>" + "<br>".join(reversed(lines)) + "</div>")
    ph_log.markdown(html, unsafe_allow_html=True)

def render_conclusions():
    if not st.session_state.throughput_hist: return
    tp   = st.session_state.throughput_hist[-1]
    prec = st.session_state.precision_hist[-1]
    mre  = st.session_state.mre_hist[-1]
    evic = st.session_state.eviction_hist[-1]
    pkts = st.session_state.total_pkts

    with ph_concl.container():
        st.divider()
        st.markdown("<div class='sec-label'>What We Can Conclude</div>", unsafe_allow_html=True)
        c1, c2, c3, c4 = st.columns(4)
        cards = [
            (c1, GRN, "Throughput", f"Processed {pkts:,} packets at {tp:.2f} Mpps. The O(1) pipeline handles high-speed traffic with no degradation as volume scales."),
            (c2, ACC, "Precision", f"Heavy hitter precision: {prec:.1f}%. Vote-and-Demote keeps frequent flows in the Heavy Guardian with near-exact counts."),
            (c3, PUR, "Light MRE", f"Mean Relative Error on light flows: {mre:.1f}%. Count-Min Sketch trades a small accuracy loss for a fixed, minimal memory footprint."),
            (c4, "#facc15", "Evictions", f"{evic:,} total evictions across {sim_cycles} cycles. Each eviction moves a weak flow to CMS — memory stays fixed regardless of flow count."),
        ]
        for col, color, title, body in cards:
            with col:
                st.markdown(
                    f"<div class='concl-card' style='border-left:3px solid {color}; border-radius:0 10px 10px 0;'>"
                    f"<div class='concl-title' style='color:{color};'>{title}</div>"
                    f"<div class='concl-body'>{body}</div></div>",
                    unsafe_allow_html=True
                )

# ── reset ──────────────────────────────────────────────────
if reset_btn:
    if st.session_state.initialized:
        lib.free_system()
    for k in list(st.session_state.keys()):
        del st.session_state[k]
    init_state()
    st.rerun()

# ── start ──────────────────────────────────────────────────
if run_btn:
    if not st.session_state.initialized:
        lib.init_system()
        st.session_state.initialized = True
        st.session_state.log.append("System initialised.")
    st.session_state.running = True

# ── simulation loop ────────────────────────────────────────
if st.session_state.running:
    gt   = st.session_state.ground_truth
    prog = st.progress(0, text="Starting simulation...")

    for cycle in range(sim_cycles):
        batch = gen_batch(alpha, n_flows, batch_size)
        for fid in batch:
            gt[int(fid)] = gt.get(int(fid), 0) + 1
        
        # Update packet count and render badge live
        st.session_state.total_pkts += batch_size
        badge("● Running", ACC, "rgba(56, 189, 248, 0.15)", "#0284c7", st.session_state.total_pkts)

        c_arr = batch.ctypes.data_as(ctypes.POINTER(ctypes.c_uint32))
        tel   = lib.process_chunk(c_arr, ctypes.c_int(batch_size))

        tput  = tel.throughput / 1e6
        st.session_state.throughput_hist.append(round(tput, 3))
        st.session_state.eviction_hist.append(tel.total_evictions)

        threshold = max(int(st.session_state.total_pkts * hh_pct), 1)
        true_hh   = sum(1 for c in gt.values() if c >= threshold)
        found_hh  = sum(
            1 for fid, cnt in gt.items()
            if cnt >= threshold and lib.query_flow(ctypes.c_uint32(fid)) > 0
        )
        prec = (min(found_hh / true_hh * 100, 100.0) if true_hh > 0 else 100.0)
        st.session_state.precision_hist.append(round(prec, 2))

        sample = [(f, c) for f, c in gt.items() if c >= 5][:500]
        if sample:
            f_arr  = (ctypes.c_uint32 * len(sample))(*[f for f, _ in sample])
            c_arr2 = (ctypes.c_uint32 * len(sample))(*[c for _, c in sample])
            mre = lib.get_mre(f_arr, c_arr2, ctypes.c_int(len(sample))) * 100
        else:
            mre = 0.0
        st.session_state.mre_hist.append(round(mre, 2))

        st.session_state.log.append(
            f"[T{st.session_state.tick + cycle + 1:03d}] "
            f"tput={tput:.2f}Mpps | evict={tel.total_evictions} | "
            f"prec={prec:.1f}% | mre={mre:.1f}%"
        )

        show_metrics(tput, prec, mre, tel.total_evictions, tel.active_guardians)
        plot_throughput()
        plot_dist(gt)
        plot_hh(gt)
        plot_acc()
        render_log()

        prog.progress((cycle + 1) / sim_cycles, text=f"Cycle {cycle + 1} / {sim_cycles}")
        time.sleep(tick_delay / 1000.0)

    st.session_state.tick    += sim_cycles
    st.session_state.running  = False
    prog.empty()
    badge("● Live", GRN, "rgba(52, 211, 153, 0.15)", "#059669", st.session_state.total_pkts)
    st.session_state.log.append(
        f"[DONE] {sim_cycles} cycles complete. Total: {st.session_state.total_pkts:,} packets."
    )
    render_log()
    render_conclusions()

elif st.session_state.tick > 0:
    gt   = st.session_state.ground_truth
    last = lambda h: h[-1] if h else None
    show_metrics(
        last(st.session_state.throughput_hist),
        last(st.session_state.precision_hist),
        last(st.session_state.mre_hist),
        last(st.session_state.eviction_hist),
        None
    )
    badge("● Live", GRN, "rgba(52, 211, 153, 0.15)", "#059669", st.session_state.total_pkts)
    plot_throughput()
    plot_dist(gt)
    plot_hh(gt)
    plot_acc()
    render_log()
    render_conclusions()

# ── flow query console ─────────────────────────────────────
st.divider()
st.markdown("<div class='sec-label'>◈ Flow Query Console</div>", unsafe_allow_html=True)
qc1, qc2, qc3 = st.columns([2, 1, 3])
with qc1:
    qid = st.number_input(
        "Flow ID", min_value=1,
        max_value=max(n_flows, 1), value=1,
        label_visibility="collapsed"
    )
with qc2:
    do_q = st.button("Query Database", type="primary", use_container_width=True)
with qc3:
    if do_q and st.session_state.initialized:
        est  = lib.query_flow(ctypes.c_uint32(qid))
        true = st.session_state.ground_truth.get(int(qid), 0)
        if true > 0:
            err = abs(est - true) / true * 100
            msg = (f"Flow **{qid}** → Estimate: **{est:,}** | True: **{true:,}** | Error: **{err:.1f}%**")
            (st.success if err < 5 else st.warning)(msg)
        elif est > 0:
            st.info(f"Flow **{qid}** → Estimate: **{est:,}**")
        else:
            st.warning(f"Flow **{qid}** not seen yet.")
    elif do_q:
        st.info("Run the simulation first.")

# ── footer ─────────────────────────────────────────────────
st.markdown(
    "<div class='footer-text'>"
    "Experiential Learning &nbsp;·&nbsp; "
    "Project 2026 &nbsp;·&nbsp; "
    "Design and Analysis of Algorithms"
    "</div>",
    unsafe_allow_html=True
)