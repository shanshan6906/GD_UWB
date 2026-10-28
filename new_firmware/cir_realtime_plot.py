# -*- coding: utf-8 -*-
"""Play a CIR bin described by the radar test-app manual.

Manual rev0.5 (Logging in binary format):
  Identifier (7) + Sequence ID and Frame ID (4) + CIR IQ.
  num_rx_mode 1 returns Rx2 only. num_rx_mode 2 returns Rx1 then Rx2.
  Each receiver is I block then Q block, int16 big-endian (32-bit complex per tap).
  Range of tap index is (index - IndexRef) * 15.024 cm. IndexRef 30 is 0 m.

Default capture layout inserts one antenna-state byte at byte 8, between the
identifier and the sequence field. The value cycles with the antenna switch.

Plots follow the distinct values of the 8th-byte antenna state.
num_rx_mode 1: one I/Q plot and one MAG plot per antenna state.
num_rx_mode 2: the original three-plot block per antenna state
(clutter magnitude, Rx1 I/Q, Rx2 I/Q).
MAG defaults to the uint32 values in the bin, and can be switched to a
host-side sqrt(I^2+Q^2). Playback steps every antenna group once per
frame_interval_ms.
"""

from __future__ import annotations

import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
from matplotlib.widgets import Button, RadioButtons
from scipy.interpolate import CubicSpline

plt.rcParams["font.sans-serif"] = ["Microsoft YaHei", "SimHei", "DengXian", "DejaVu Sans"]
plt.rcParams["axes.unicode_minus"] = False

try:
    import tkinter as tk
    from tkinter import filedialog
except ImportError:  # pragma: no cover
    tk = None
    filedialog = None

MARKER_A = bytes([0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00])
# Manual Table 1: each tap is 15.024 cm (c = 3e8, 998.4 MHz round trip).
RANGE_STEP_M = 0.15024
HEADER_KEYS = (
    "frame_interval_ms",
    "IndexRef",
    "CIR_tap_start",
    "CIR_tap_end",
    "num_rx_mode",
    "sensing_mode",
    "gain_idx",
    "scale_bit",
    "power_code",
    "sensing_interval_ms",
)


@dataclass
class Packet:
    seq: int
    frame: int
    rf0: int
    i1: np.ndarray
    q1: np.ndarray
    i2: np.ndarray
    q2: np.ndarray
    m1: np.ndarray = field(default_factory=lambda: np.zeros(0, dtype=np.int64))
    m2: np.ndarray = field(default_factory=lambda: np.zeros(0, dtype=np.int64))
    nbytes: int = 0


@dataclass
class Recording:
    path: Path
    cfg: dict
    groups: dict[int, list[Packet]] = field(default_factory=dict)
    interval_ms: int = 100
    index_ref: int = 30
    tap_start: int = 0
    taps: int = 0
    n_rx: int = 2
    has_rf0: bool = True
    has_mag: bool = False
    header_len: int = 12

    @property
    def play_count(self) -> int:
        return max((len(v) for v in self.groups.values()), default=0)


def _find_markers(data: bytes) -> list[int]:
    positions = []
    start = 0
    while True:
        i = data.find(MARKER_A, start)
        if i < 0:
            break
        positions.append(i)
        start = i + 1
    return positions


def _cfg_from_text(text: str) -> dict:
    cfg = {}
    for key in HEADER_KEYS:
        match = re.search(rf"{re.escape(key)} value:\s*(-?\d+)", text)
        if match:
            cfg[key] = int(match.group(1))
    return cfg


def _i16_blocks(payload: bytes, taps: int, n_blocks: int) -> list[np.ndarray]:
    count = len(payload) // 2
    if count <= 0:
        return [np.zeros(0, dtype=np.float64) for _ in range(4)]
    samples = np.frombuffer(payload[: count * 2], dtype=">i2").astype(np.float64)
    blocks = []
    for n in range(n_blocks):
        blocks.append(samples[n * taps : (n + 1) * taps])
    while len(blocks) < 4:
        blocks.append(np.zeros(0, dtype=np.float64))
    return blocks


def _u32_block(payload: bytes, taps: int) -> np.ndarray:
    nbytes = min(len(payload) // 4 * 4, taps * 4)
    if nbytes <= 0:
        return np.zeros(0, dtype=np.int64)
    return np.frombuffer(payload[:nbytes], dtype=">u4").astype(np.int64)


def load_bin(path: str | Path) -> Recording:
    path = Path(path)
    data = path.read_bytes()
    marks = _find_markers(data)
    if len(marks) < 1:
        raise ValueError("文件里没有 CIR 同步头 FF FF 00 FF FF FF 00")

    cfg = _cfg_from_text(data[: marks[0]].decode("ascii", errors="replace"))
    tap_start = int(cfg.get("CIR_tap_start", 0))
    tap_end = int(cfg.get("CIR_tap_end", tap_start))
    taps = max(tap_end - tap_start + 1, 1)
    n_rx = 2 if int(cfg.get("num_rx_mode", 2)) >= 2 else 1
    n_blocks = n_rx * 2
    payload = n_blocks * taps * 2

    gaps = [marks[k + 1] - marks[k] for k in range(len(marks) - 1)]
    stride = max(set(gaps), key=gaps.count) if gaps else (12 + payload)
    mag_bytes = n_rx * taps * 4
    # Default: identifier + antenna byte + sequence/frame, then optional magnitude.
    layouts = (
        (12 + payload, True, 7, 8, 12, False),
        (12 + payload + mag_bytes, True, 7, 8, 12, True),
        (11 + payload, False, None, 7, 11, False),
        (11 + payload + mag_bytes, False, None, 7, 11, True),
        (13 + payload, True, 8, 9, 13, False),
        (13 + payload + mag_bytes, True, 8, 9, 13, True),
    )
    matched = next((item for item in layouts if item[0] == stride), None)
    if matched is None:
        raise ValueError(
            f"包间隔 {stride} 字节，与 {taps} 点、{n_rx} 路接收的长度对不上"
        )
    _, has_rf0, rf0_off, hdr_off, iq_off, has_mag = matched

    groups: dict[int, list[Packet]] = {0: [], 1: []}
    iq_bytes = n_blocks * taps * 2
    for pos in marks:
        end = pos + stride
        pkt = data[pos:end]
        if len(pkt) < iq_off:
            continue
        rf0 = int(pkt[rf0_off]) if has_rf0 else -1
        seq = (pkt[hdr_off] << 16) | (pkt[hdr_off + 1] << 8) | pkt[hdr_off + 2]
        frame = int(pkt[hdr_off + 3])
        blocks = _i16_blocks(pkt[iq_off:], taps, n_blocks)
        if n_rx == 1:
            # Manual: num_rx_mode 1 returns Rx2 only.
            i1 = q1 = np.zeros(0, dtype=np.float64)
            i2, q2 = blocks[0], blocks[1]
        else:
            i1, q1, i2, q2 = blocks[0], blocks[1], blocks[2], blocks[3]
        m1 = m2 = np.zeros(0, dtype=np.int64)
        if has_mag and len(pkt) > iq_off + iq_bytes:
            mag = pkt[iq_off + iq_bytes :]
            if n_rx == 1:
                m2 = _u32_block(mag, taps)
            else:
                m1 = _u32_block(mag, taps)
                m2 = _u32_block(mag[taps * 4 :], taps)
        bucket = rf0 if has_rf0 else 0
        groups.setdefault(bucket, []).append(
            Packet(seq, frame, rf0, i1, q1, i2, q2, m1, m2, nbytes=len(pkt))
        )

    for packets in groups.values():
        packets.sort(key=lambda item: item.seq)

    interval = int(cfg.get("frame_interval_ms", 100))
    return Recording(
        path=path,
        cfg=cfg,
        groups=groups,
        interval_ms=max(interval, 1),
        index_ref=int(cfg.get("IndexRef", 30)),
        tap_start=tap_start,
        taps=taps,
        n_rx=n_rx,
        has_rf0=has_rf0,
        has_mag=has_mag,
        header_len=iq_off,
    )


def range_m(tap_start: int, count: int, index_ref: int) -> np.ndarray:
    idx = tap_start + np.arange(count, dtype=np.float64)
    return (idx - index_ref) * RANGE_STEP_M


def magnitude(i_data: np.ndarray, q_data: np.ndarray) -> np.ndarray:
    n = min(len(i_data), len(q_data))
    if n <= 0:
        return np.zeros(0, dtype=np.float64)
    ii = i_data[:n]
    qq = q_data[:n]
    return np.sqrt(ii * ii + qq * qq)


def dense_curve(x: np.ndarray, y: np.ndarray) -> tuple[np.ndarray, np.ndarray]:
    if len(x) < 4:
        return x, y
    xs = np.linspace(x[0], x[-1], len(x) * 8)
    ys = CubicSpline(x, y)(xs)
    return xs, np.maximum(ys, 0.0)


def parabolic_peak(x: np.ndarray, y: np.ndarray) -> tuple[float, float]:
    i = int(np.argmax(y))
    if i <= 0 or i >= len(y) - 1:
        return float(x[i]), float(y[i])
    y0, y1, y2 = float(y[i - 1]), float(y[i]), float(y[i + 1])
    denom = y0 - 2.0 * y1 + y2
    if denom == 0.0:
        return float(x[i]), y1
    delta = 0.5 * (y0 - y2) / denom
    dx = float(x[i] - x[i - 1])
    amp = y1 - 0.25 * (y0 - y2) * delta
    return float(x[i]) + delta * dx, amp


class Player:
    def __init__(self) -> None:
        self.rec: Recording | None = None
        self.k = 0
        self.playing = False
        self.use_reported = True
        self.states: list[int] = []
        self.panels: list[dict] = []
        self._anns: list = []
        self._control_axes: list = []
        self._tk = None
        if tk is not None:
            self._tk = tk.Tk()
            self._tk.withdraw()

        self.fig = plt.figure("Real-Time Plot", figsize=(14.5, 8.2))
        ax_open = self.fig.add_axes([0.05, 0.025, 0.10, 0.045])
        ax_play = self.fig.add_axes([0.16, 0.025, 0.10, 0.045])
        ax_mag = self.fig.add_axes([0.30, 0.012, 0.16, 0.075])
        self._control_axes = [ax_open, ax_play, ax_mag]
        self.btn_open = Button(ax_open, "打开文件")
        self.btn_play = Button(ax_play, "暂停")
        self.mag_radio = RadioButtons(ax_mag, ("上报 MAG", "计算 MAG"), active=0)
        self.btn_open.on_clicked(lambda _event: self.open_dialog())
        self.btn_play.on_clicked(lambda _event: self.toggle())
        self.mag_radio.on_clicked(self._on_mag_source)

        self.timer = self.fig.canvas.new_timer(interval=100)
        self.timer.add_callback(self._tick)
        self.fig.suptitle("打开一个 CIR bin 文件", fontsize=12)

    def _on_mag_source(self, label: str) -> None:
        self.use_reported = label.startswith("上报")
        if getattr(self, "_suspend_mag", False) or self.rec is None or not self.panels:
            return
        self._apply_limits()
        self.draw()

    def _antenna_states(self) -> list[int]:
        assert self.rec is not None
        return sorted(ant for ant, packets in self.rec.groups.items() if packets)

    def _style_ax(self, ax, title: str) -> None:
        ax.set_title(title, fontsize=10)
        ax.set_xlabel("Range [m]")
        ax.set_ylabel("Amplitude")
        ax.grid(True, which="major", alpha=0.45)

    def _rebuild_axes(self) -> None:
        assert self.rec is not None
        for ann in self._anns:
            ann.remove()
        self._anns = []
        for ax in list(self.fig.axes):
            if ax not in self._control_axes:
                self.fig.delaxes(ax)
        self.panels = []
        self.states = self._antenna_states()
        n = max(len(self.states), 1)
        dual = self.rec.n_rx >= 2
        if dual:
            self.fig.set_size_inches(max(14.5, 6.2 * n), 8.2)
            gs = self.fig.add_gridspec(
                2, 2 * n,
                height_ratios=[1.35, 1.0],
                left=0.05, right=0.985, top=0.88, bottom=0.14,
                hspace=0.38, wspace=0.35,
            )
            for col, ant in enumerate(self.states):
                abs_ax = self.fig.add_subplot(gs[0, col * 2: col * 2 + 2])
                iq1 = self.fig.add_subplot(gs[1, col * 2])
                iq2 = self.fig.add_subplot(gs[1, col * 2 + 1])
                self._style_ax(abs_ax, f"Fr Clutter (abs)   ant={ant}")
                self._style_ax(iq1, "Rx1 I/Q")
                self._style_ax(iq2, "Rx2 I/Q")
                self.panels.append({"ant": ant, "abs": abs_ax, "iq1": iq1, "iq2": iq2})
        else:
            self.fig.set_size_inches(max(12.0, 3.6 * n), 8.2)
            gs = self.fig.add_gridspec(
                2, n,
                height_ratios=[1.15, 1.0],
                left=0.06, right=0.985, top=0.88, bottom=0.14,
                hspace=0.38, wspace=0.32,
            )
            for col, ant in enumerate(self.states):
                mag_ax = self.fig.add_subplot(gs[0, col])
                iq_ax = self.fig.add_subplot(gs[1, col])
                self._style_ax(mag_ax, f"Rx2 MAG   ant={ant}")
                self._style_ax(iq_ax, "Rx2 I/Q")
                self.panels.append({"ant": ant, "mag": mag_ax, "iq": iq_ax})

    def open_dialog(self) -> None:
        if filedialog is None:
            print("当前环境没有文件选择对话框，请用参数传入 bin 路径")
            return
        initial = str(Path(__file__).resolve().parent)
        path = filedialog.askopenfilename(
            title="选择 CIR bin 文件",
            initialdir=initial,
            filetypes=[("CIR bin", "*.bin"), ("All files", "*.*")],
        )
        if path:
            self.load(path)

    def load(self, path: str | Path) -> None:
        self.timer.stop()
        self.playing = False
        try:
            self.rec = load_bin(path)
        except Exception as exc:
            self.fig.suptitle(f"无法解析: {exc}", fontsize=12)
            self.fig.canvas.draw_idle()
            return
        self.k = 0
        self._suspend_mag = True
        self.use_reported = bool(self.rec.has_mag)
        self.mag_radio.set_active(0 if self.use_reported else 1)
        self._suspend_mag = False
        self._rebuild_axes()
        self._apply_limits()
        self.timer.interval = self.rec.interval_ms
        self.playing = True
        self.btn_play.label.set_text("暂停")
        self.draw()
        self.timer.start()

    def toggle(self) -> None:
        if self.rec is None:
            return
        if self.playing:
            self.timer.stop()
            self.playing = False
            self.btn_play.label.set_text("播放")
        else:
            self.timer.start()
            self.playing = True
            self.btn_play.label.set_text("暂停")
        self.fig.canvas.draw_idle()

    def _tick(self) -> None:
        if self.rec is None or self.rec.play_count <= 0:
            return
        self.k = (self.k + 1) % self.rec.play_count
        self.draw()

    def _apply_limits(self) -> None:
        assert self.rec is not None
        x0 = x1 = None
        for packets in self.rec.groups.values():
            for pkt in packets:
                for arr in (pkt.i2, pkt.i1, pkt.q2, pkt.q1):
                    if len(arr):
                        xs = range_m(self.rec.tap_start, len(arr), self.rec.index_ref)
                        x0, x1 = float(xs[0]), float(xs[-1])
                        break
                if x0 is not None:
                    break
            if x0 is not None:
                break
        if x0 is None or x1 is None:
            return
        pad = 0.02 * (x1 - x0 if x1 > x0 else 1.0)
        data_axes = [ax for ax in self.fig.axes if ax not in self._control_axes]
        for ax in data_axes:
            ax.set_xlim(x0 - pad, x1 + pad)
            ax.set_autoscalex_on(False)
            ax.set_autoscaley_on(False)

        for panel in self.panels:
            packets = self.rec.groups.get(panel["ant"], [])
            if "abs" in panel:
                peak = 1.0
                for pkt in packets:
                    for rx in (1, 2):
                        mag = self._channel_mag(pkt, rx)
                        if len(mag):
                            peak = max(peak, float(np.max(mag)))
                panel["abs"].set_ylim(0.0, peak * 1.12)
                for ax, names in (
                    (panel["iq1"], ("i1", "q1")),
                    (panel["iq2"], ("i2", "q2")),
                ):
                    iq_peak = 1.0
                    for pkt in packets:
                        for name in names:
                            arr = getattr(pkt, name)
                            if len(arr):
                                iq_peak = max(iq_peak, float(np.max(np.abs(arr))))
                    ax.set_ylim(-iq_peak * 1.12, iq_peak * 1.12)
            else:
                peak = 1.0
                iq_peak = 1.0
                for pkt in packets:
                    mag = self._channel_mag(pkt, 2)
                    if len(mag):
                        peak = max(peak, float(np.max(mag)))
                    for arr in (pkt.i2, pkt.q2):
                        if len(arr):
                            iq_peak = max(iq_peak, float(np.max(np.abs(arr))))
                panel["mag"].set_ylim(0.0, peak * 1.12)
                panel["iq"].set_ylim(-iq_peak * 1.12, iq_peak * 1.12)

    def _channel_mag(self, pkt: Packet, rx: int) -> np.ndarray:
        reported = pkt.m1 if rx == 1 else pkt.m2
        ii = pkt.i1 if rx == 1 else pkt.i2
        qq = pkt.q1 if rx == 1 else pkt.q2
        if self.use_reported and self.rec is not None and self.rec.has_mag and len(reported):
            return reported.astype(np.float64)
        return magnitude(ii, qq)

    def _clear_lines(self, ax) -> None:
        for artist in list(ax.lines):
            artist.remove()
        legend = ax.get_legend()
        if legend is not None:
            legend.remove()

    def draw(self) -> None:
        if self.rec is None:
            return
        for ann in self._anns:
            ann.remove()
        self._anns = []
        cfg = self.rec.cfg
        info = "GI {g}, SB {s}, PO {p}".format(
            g=cfg.get("gain_idx", "-"),
            s=cfg.get("scale_bit", "-"),
            p=cfg.get("power_code", "-"),
        )
        mag_note = "上报 MAG" if self.use_reported and self.rec.has_mag else "计算 MAG"
        if self.use_reported and not self.rec.has_mag:
            mag_note = "无上报 MAG，使用计算值"
        for panel in self.panels:
            packets = self.rec.groups.get(panel["ant"], [])
            pkt = packets[self.k % len(packets)] if packets else None
            ant = panel["ant"]
            if "abs" in panel:
                self._clear_lines(panel["abs"])
                if pkt is None:
                    panel["abs"].set_title(f"Fr Clutter (abs)   ant={ant}   无数据")
                else:
                    panel["abs"].set_title(f"Fr Clutter (abs)   ant={ant}   seq={pkt.seq}")
                    self._plot_abs(panel["abs"], pkt)
                self._plot_iq(panel["iq1"], pkt, "i1", "q1", "Rx1 I", "Rx1 Q")
                self._plot_iq(panel["iq2"], pkt, "i2", "q2", "Rx2 I", "Rx2 Q")
            else:
                self._clear_lines(panel["mag"])
                if pkt is None:
                    panel["mag"].set_title(f"Rx2 MAG   ant={ant}   无数据")
                else:
                    panel["mag"].set_title(f"Rx2 MAG   ant={ant}   seq={pkt.seq}")
                    self._plot_one_mag(panel["mag"], pkt, 2, "Rx2")
                self._plot_iq(panel["iq"], pkt, "i2", "q2", "Rx2 I", "Rx2 Q")

        state = "播放" if self.playing else "暂停"
        self.fig.suptitle(
            f"{self.rec.path.name}    {state}  {self.k + 1}/{self.rec.play_count}"
            f"    frame_interval_ms={self.rec.interval_ms}    {info}    {mag_note}",
            fontsize=11,
        )
        self.fig.canvas.draw_idle()

    def _plot_mag_curve(self, ax, name: str, mag: np.ndarray, color: str, text_side: str) -> None:
        if self.rec is None or len(mag) < 2:
            return
        x = range_m(self.rec.tap_start, len(mag), self.rec.index_ref)
        xs, ys = dense_curve(x, mag)
        ax.plot(xs, ys, color=color, lw=1.15, label=f"{name} Clutter interp")
        ax.plot(x, mag, "x", color=color, ms=4.5, mew=0.8, label=f"{name} Clutter sampled")
        leak_x, leak_y = parabolic_peak(x, mag)
        y0, y1 = ax.get_ylim()
        if text_side == "left":
            text_x, text_y, ha = x[0] + 0.3, y0 + 0.62 * (y1 - y0), "left"
        else:
            text_x, text_y, ha = leak_x + 2.6, y0 + 0.38 * (y1 - y0), "left"
        ann = ax.annotate(
            f"{name} leak ({leak_x:.2f}, {leak_y:.2f})",
            xy=(leak_x, leak_y),
            xytext=(text_x, text_y),
            color=color,
            fontsize=8,
            arrowprops={"arrowstyle": "->", "color": color, "lw": 0.8},
            ha=ha,
            va="center",
        )
        self._anns.append(ann)

    def _plot_abs(self, ax, pkt: Packet) -> None:
        self._plot_mag_curve(ax, "Rx1", self._channel_mag(pkt, 1), "C0", "left")
        self._plot_mag_curve(ax, "Rx2", self._channel_mag(pkt, 2), "C3", "right")
        if ax.lines:
            ax.legend(loc="upper right", fontsize=7.5, framealpha=0.9)

    def _plot_one_mag(self, ax, pkt: Packet, rx: int, name: str) -> None:
        self._plot_mag_curve(ax, name, self._channel_mag(pkt, rx), "C0", "left")
        if ax.lines:
            ax.legend(loc="upper right", fontsize=7.5, framealpha=0.9)

    def _plot_iq(self, ax, pkt: Packet | None, i_name: str, q_name: str, i_label: str, q_label: str) -> None:
        self._clear_lines(ax)
        if pkt is None:
            return
        ii = getattr(pkt, i_name)
        qq = getattr(pkt, q_name)
        if len(ii):
            x = range_m(self.rec.tap_start, len(ii), self.rec.index_ref)
            ax.plot(x, ii, color="C0", lw=1.0, marker="x", ms=3.5, mew=0.7, label=i_label)
        if len(qq):
            x = range_m(self.rec.tap_start, len(qq), self.rec.index_ref)
            ax.plot(x, qq, color="C1", lw=1.0, label=q_label)
        if ax.lines:
            ax.legend(loc="upper right", fontsize=8, framealpha=0.9)


def _n(values: np.ndarray) -> str:
    return str(len(values))


def _ints(values: np.ndarray) -> str:
    return ",".join(str(int(v)) for v in values)


def write_parse_txt(rec: Recording, out: Path) -> None:
    """Write one packet per line using the rev0.5 field names."""
    cfg = rec.cfg
    r0 = (rec.tap_start - rec.index_ref) * RANGE_STEP_M
    r1 = (rec.tap_start + rec.taps - 1 - rec.index_ref) * RANGE_STEP_M
    full = rec.header_len + rec.n_rx * 2 * rec.taps * 2
    if rec.has_mag:
        full += rec.n_rx * rec.taps * 4
    lines = [
        f"# 源文件: {rec.path.name}",
        "# 格式: CBU5000-V210-SOLN-0005 RadarDriver Test APP Manual rev0.5",
        "# Identifier(7) = FF FF 00 FF FF FF 00",
        "# Sequence ID = 随后 3 字节大端；Frame ID = 第 4 字节。sensing_mode=0 只递增 Sequence；sensing_mode=1 时 Frame 每帧 +1，Sequence 每条 CIR +1 且不按帧清零",
        (
            "# 默认格式：在手册字段上插入第 8 字节，表示天线状态；其后才是 Sequence ID / Frame ID"
            if rec.has_rf0
            else "# 本文件与手册图一致：Identifier 后直接是 Sequence ID / Frame ID"
        ),
        (
            "# 幅度跟在最后一路 Q 后面。1 路为 Rx2_Mag；2 路为 Rx1_Mag 然后 Rx2_Mag。每点 uint32 大端，round(sqrt(I^2+Q^2))"
            if rec.has_mag
            else "# 本文件没有幅度字段"
        ),
        f"# 每 tap 15.024 cm，IndexRef={rec.index_ref} 为 0 m。tap {rec.tap_start}..{rec.tap_start + rec.taps - 1} = {r0:.3f} m .. {r1:.3f} m",
        "# num_rx_mode=1 只输出 Rx2；num_rx_mode=2 先 Rx1 再 Rx2。每路先全部 I 再全部 Q，int16 大端（每 tap 32-bit complex）",
        (
            f"# 采集: sensing_mode={cfg.get('sensing_mode', '-')} num_rx_mode={rec.n_rx} "
            f"taps={rec.taps} frame_interval_ms={rec.interval_ms} 完整包 {full} 字节"
        ),
        "# trunc=1 表示该包未收全，IQ 只保留已收全的点",
        "# I组数、Q组数、MAG组数是该包每一路实际收到的点数，写在对应数据列前面",
    ]
    cols = ["idx", "len", "trunc", "identifier", "sequence_id", "frame_id"]
    if rec.has_rf0:
        cols.insert(4, "antenna")
    if rec.n_rx >= 2:
        cols.extend(["Rx1_I组数", "Rx1_Q组数", "Rx2_I组数", "Rx2_Q组数"])
        if rec.has_mag:
            cols.extend(["Rx1_MAG组数", "Rx2_MAG组数"])
        cols.extend(["Rx1_I", "Rx1_Q", "Rx2_I", "Rx2_Q"])
        if rec.has_mag:
            cols.extend(["Rx1_Mag", "Rx2_Mag"])
    else:
        cols.extend(["I组数", "Q组数"])
        if rec.has_mag:
            cols.append("MAG组数")
        cols.extend(["Rx2_I", "Rx2_Q"])
        if rec.has_mag:
            cols.append("Rx2_Mag")
    lines.append("# 列: " + " ".join(cols))
    lines.append("\t".join(cols))

    packets = [pkt for group in rec.groups.values() for pkt in group]
    packets.sort(key=lambda pkt: pkt.seq)
    ident = "FF FF 00 FF FF FF 00"
    for idx, pkt in enumerate(packets, start=1):
        row = [
            str(idx),
            str(pkt.nbytes),
            "0" if pkt.nbytes == full else "1",
            ident,
        ]
        if rec.has_rf0:
            row.append(f"{pkt.rf0:02X}")
        row.extend([str(pkt.seq), str(pkt.frame)])
        if rec.n_rx >= 2:
            row.extend([_n(pkt.i1), _n(pkt.q1), _n(pkt.i2), _n(pkt.q2)])
            if rec.has_mag:
                row.extend([_n(pkt.m1), _n(pkt.m2)])
            row.extend([_ints(pkt.i1), _ints(pkt.q1), _ints(pkt.i2), _ints(pkt.q2)])
            if rec.has_mag:
                row.extend([_ints(pkt.m1), _ints(pkt.m2)])
        else:
            row.extend([_n(pkt.i2), _n(pkt.q2)])
            if rec.has_mag:
                row.append(_n(pkt.m2))
            row.extend([_ints(pkt.i2), _ints(pkt.q2)])
            if rec.has_mag:
                row.append(_ints(pkt.m2))
        lines.append("\t".join(row))
    out.write_text("\n".join(lines) + "\n", encoding="utf-8-sig")


def main() -> None:
    if len(sys.argv) >= 3 and sys.argv[1] == "--txt":
        src = Path(sys.argv[2])
        rec = load_bin(src)
        dst = Path(sys.argv[3]) if len(sys.argv) >= 4 else src.with_suffix(".txt")
        write_parse_txt(rec, dst)
        print(dst)
        return
    player = Player()
    if len(sys.argv) > 1:
        player.load(sys.argv[1])
    else:
        player.open_dialog()
    plt.show()


if __name__ == "__main__":
    main()
