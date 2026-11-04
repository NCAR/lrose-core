"""Standalone UDP listener for pedestal telemetry broadcast messages."""
import socket
import threading
import time
import tkinter as tk
from datetime import datetime, timezone
from tkinter import font

UDP_BROADCAST_PORT = 5001
MAX_LINES = 10
RAW_TEXT_WIDTH = 75

BG_DARK = "#1e1e1e"
BG_PANEL = "#2d2d2d"
FG_WHITE = "white"
FG_AZ = "#4CAF50"
FG_EL = "#00BCD4"
FG_TRANSIT_ACTIVE = "lightgreen"
FG_MUTED = "#aaaaaa"
FG_RAW = "#4CAF50"
FG_IDLE = "#FF9800"
TARGET_ANGLE_IDLE = 400.0

SCAN_TYPE_LABELS = {
    1: "1.0 - Survey",
    2: "2.0 - Sector Abs",
    3: "3.0 - Sector Const",
    4: "4.0 - RHI Vert Abs",
    5: "5.0 - RHI Vert Const",
    6: "6.0 - Sector Fast",
    7: "7.0 - RHI Vert Fast",
}


def format_timestamp(epoch_seconds):
    dt = datetime.fromtimestamp(epoch_seconds, tz=timezone.utc)
    sec_with_frac = dt.second + dt.microsecond / 1_000_000
    return dt.strftime("%Y-%m-%d %H:%M:") + f"{sec_with_frac:.1f}"


def format_utc_now(epoch_seconds=None):
    """Format current (or given) wall time as UTC ``YYYY-MM-DD HH:MM:SS.s``."""
    if epoch_seconds is None:
        epoch_seconds = time.time()
    return format_timestamp(epoch_seconds)


def timestamp_delta_seconds(telem_timestamp, arrival_utc=None):
    """Seconds between stamped telem time and listener arrival (UTC)."""
    if arrival_utc is None:
        arrival_utc = time.time()
    return float(arrival_utc) - float(telem_timestamp)


def format_delta_seconds(delta_seconds):
    sign = "+" if delta_seconds >= 0 else "-"
    return f"{sign}{abs(delta_seconds) * 1000.0:.1f} ms"


def scan_type_label(scan_type):
    return SCAN_TYPE_LABELS.get(int(scan_type), f"{int(scan_type):02d}")


def parse_telemetry_message(line):
    """Parse ``t=...,az=...,el=...,type=...,vol=...,sweep=...,fixed=...,tr=...,scan=...``."""
    fields = {}
    for part in line.split(","):
        if "=" not in part:
            return None
        key, value = part.split("=", 1)
        fields[key.strip()] = value.strip()
    required = ("t", "az", "el", "type", "vol", "sweep", "fixed", "tr")
    if any(key not in fields for key in required):
        return None
    try:
        return {
            "timestamp": float(fields["t"]),
            "az": float(fields["az"]),
            "el": float(fields["el"]),
            "type": int(fields["type"]),
            "vol": int(fields["vol"]),
            "sweep": int(fields["sweep"]),
            "fixed": float(fields["fixed"]),
            "transit": int(fields["tr"]) != 0,
            "scan": fields.get("scan", ""),
        }
    except (ValueError, IndexError):
        return None


class UdpTelemListener:
    def __init__(self, root):
        self.root = root
        self.root.title("UDP Telemetry Listener")
        self.root.configure(bg=BG_DARK)
        self.running = True

        self.header_font = font.Font(family="Helvetica", size=18, weight="bold")
        self.large_font = font.Font(family="Helvetica", size=42, weight="bold")
        self.meta_font = font.Font(family="Helvetica", size=12, weight="bold")
        self.status_font = font.Font(family="Helvetica", size=11, weight="bold")
        self.data_font = font.Font(family="Courier", size=12, weight="bold")

        self.display_frame = tk.Frame(self.root, bg=BG_PANEL, bd=2, relief=tk.RIDGE)
        self.display_frame.pack(fill=tk.X, padx=12, pady=(12, 8))
        self.display_frame.grid_columnconfigure(0, weight=1)
        self.display_frame.grid_columnconfigure(1, weight=1)

        self.timestamp_var = tk.StringVar(value="Telem: —")
        tk.Label(
            self.display_frame,
            textvariable=self.timestamp_var,
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=self.data_font,
            anchor="w",
        ).grid(row=0, column=0, columnspan=2, sticky="w", padx=12, pady=(10, 2))

        self.status_var = tk.StringVar(value=f"Listening on UDP port {UDP_BROADCAST_PORT}")
        tk.Label(
            self.display_frame,
            textvariable=self.status_var,
            fg=FG_MUTED,
            bg=BG_PANEL,
            font=self.status_font,
            anchor="e",
        ).grid(row=0, column=1, sticky="e", padx=12, pady=(10, 2))

        self.utc_var = tk.StringVar(value="UTC: —")
        tk.Label(
            self.display_frame,
            textvariable=self.utc_var,
            fg=FG_MUTED,
            bg=BG_PANEL,
            font=self.data_font,
            anchor="w",
        ).grid(row=1, column=0, sticky="w", padx=12, pady=(0, 4))

        self.delta_var = tk.StringVar(value="Delta: —")
        tk.Label(
            self.display_frame,
            textvariable=self.delta_var,
            fg=FG_MUTED,
            bg=BG_PANEL,
            font=self.data_font,
            anchor="e",
        ).grid(row=1, column=1, sticky="e", padx=12, pady=(0, 4))

        tk.Label(
            self.display_frame,
            text="AZIMUTH",
            fg=FG_AZ,
            bg=BG_PANEL,
            font=self.header_font,
            anchor="center",
        ).grid(row=2, column=0, pady=(8, 0), sticky="ew")
        tk.Label(
            self.display_frame,
            text="ELEVATION",
            fg=FG_EL,
            bg=BG_PANEL,
            font=self.header_font,
            anchor="center",
        ).grid(row=2, column=1, pady=(8, 0), sticky="ew")

        self.az_var = tk.StringVar(value="—")
        self.el_var = tk.StringVar(value="—")
        tk.Label(
            self.display_frame,
            textvariable=self.az_var,
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=self.large_font,
            anchor="center",
        ).grid(row=3, column=0, pady=(0, 8), sticky="ew")
        tk.Label(
            self.display_frame,
            textvariable=self.el_var,
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=self.large_font,
            anchor="center",
        ).grid(row=3, column=1, pady=(0, 8), sticky="ew")

        self.scan_type_var = tk.StringVar(value="Scan Type: —")
        tk.Label(
            self.display_frame,
            textvariable=self.scan_type_var,
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=self.meta_font,
            anchor="w",
        ).grid(row=4, column=0, columnspan=2, sticky="w", padx=12, pady=(4, 2))

        vol_tilt_frame = tk.Frame(self.display_frame, bg=BG_PANEL)
        vol_tilt_frame.grid(row=5, column=0, columnspan=2, sticky="w", padx=12, pady=2)
        self.vol_var = tk.StringVar(value="Vol #: —")
        self.tilt_var = tk.StringVar(value="Sweep #: —")
        tk.Label(
            vol_tilt_frame,
            textvariable=self.vol_var,
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=self.meta_font,
        ).pack(side=tk.LEFT, padx=(0, 24))
        tk.Label(
            vol_tilt_frame,
            textvariable=self.tilt_var,
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=self.meta_font,
        ).pack(side=tk.LEFT)

        self.scan_name_var = tk.StringVar(value="Scan: —")
        tk.Label(
            self.display_frame,
            textvariable=self.scan_name_var,
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=self.meta_font,
            anchor="w",
        ).grid(row=6, column=0, columnspan=2, sticky="w", padx=12, pady=2)

        target_transit_frame = tk.Frame(self.display_frame, bg=BG_PANEL)
        target_transit_frame.grid(row=7, column=0, columnspan=2, sticky="w", padx=12, pady=(2, 12))
        self.target_var = tk.StringVar(value="Target Angle: —")
        self.target_lbl = tk.Label(
            target_transit_frame,
            textvariable=self.target_var,
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=self.meta_font,
        )
        self.target_lbl.pack(side=tk.LEFT, padx=(0, 24))
        tk.Label(
            target_transit_frame,
            text="Transit:",
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=self.meta_font,
        ).pack(side=tk.LEFT)
        self.transit_var = tk.StringVar(value="—")
        self.transit_lbl = tk.Label(
            target_transit_frame,
            textvariable=self.transit_var,
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=self.meta_font,
        )
        self.transit_lbl.pack(side=tk.LEFT, padx=(6, 0))

        log_frame = tk.Frame(self.root, bg=BG_PANEL, bd=2, relief=tk.SUNKEN)
        log_frame.pack(fill=tk.BOTH, expand=True, padx=12, pady=(0, 12))
        tk.Label(
            log_frame,
            text="Raw Messages",
            fg=FG_MUTED,
            bg=BG_PANEL,
            font=self.status_font,
            anchor="w",
        ).pack(fill=tk.X, padx=8, pady=(8, 4))
        self.text = tk.Text(
            log_frame,
            height=MAX_LINES,
            width=RAW_TEXT_WIDTH,
            font=("Courier", 10),
            wrap=tk.NONE,
            state=tk.DISABLED,
            bg=BG_DARK,
            fg=FG_RAW,
            insertbackground=FG_RAW,
            relief=tk.FLAT,
            bd=0,
        )
        self.text.pack(fill=tk.BOTH, expand=True, padx=8, pady=(0, 8))

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(("", UDP_BROADCAST_PORT))
        self.sock.settimeout(1.0)

        self.thread = threading.Thread(target=self._listen_loop, daemon=True)
        self.thread.start()

        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def _listen_loop(self):
        while self.running:
            try:
                data, addr = self.sock.recvfrom(4096)
            except socket.timeout:
                continue
            except OSError:
                break
            arrival_utc = time.time()
            line = data.decode("ascii", errors="replace").rstrip("\r\n")
            self.root.after(0, self._handle_message, line, addr, arrival_utc)

    def _handle_message(self, line, addr, arrival_utc):
        self._append_line(line)
        parsed = parse_telemetry_message(line)
        if parsed is None:
            self.status_var.set(f"Parse error from {addr[0]}:{addr[1]}")
            return
        delta = timestamp_delta_seconds(parsed["timestamp"], arrival_utc)
        self.status_var.set(f"Listening on UDP port {UDP_BROADCAST_PORT}  |  last: {addr[0]}:{addr[1]}")
        self.timestamp_var.set(f"Telem: {format_timestamp(parsed['timestamp'])}")
        self.utc_var.set(f"UTC: {format_utc_now(arrival_utc)}")
        self.delta_var.set(f"Delta: {format_delta_seconds(delta)}")
        self.az_var.set(f"{parsed['az']:.1f}°")
        self.el_var.set(f"{parsed['el']:.1f}°")
        self.scan_type_var.set(f"Scan Type: {scan_type_label(parsed['type'])}")
        self.vol_var.set(f"Vol #: {parsed['vol']}")
        self.tilt_var.set(f"Sweep #: {parsed['sweep']}")
        scan_name = parsed.get("scan") or "—"
        self.scan_name_var.set(f"Scan: {scan_name}")
        if abs(parsed["fixed"] - TARGET_ANGLE_IDLE) < 0.05:
            self.target_var.set("Target Angle: idle")
            self.target_lbl.config(fg=FG_IDLE)
        else:
            self.target_var.set(f"Target Angle: {parsed['fixed']:.1f}°")
            self.target_lbl.config(fg=FG_WHITE)
        transit_text = "Yes" if parsed["transit"] else "No"
        self.transit_var.set(transit_text)
        self.transit_lbl.config(fg=FG_TRANSIT_ACTIVE if parsed["transit"] else FG_WHITE)

    def _append_line(self, line):
        self.text.configure(state=tk.NORMAL)
        self.text.insert(tk.END, line + "\n")
        line_count = int(self.text.index("end-1c").split(".")[0])
        if line_count > MAX_LINES:
            self.text.delete("1.0", f"{line_count - MAX_LINES + 1}.0")
        self.text.see(tk.END)
        self.text.configure(state=tk.DISABLED)

    def on_close(self):
        self.running = False
        try:
            self.sock.close()
        except OSError:
            pass
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    UdpTelemListener(root)
    root.mainloop()
