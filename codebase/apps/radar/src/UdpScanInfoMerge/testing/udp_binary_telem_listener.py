"""Standalone listener for binary pedestal telemetry on UDP port 5002."""
import socket
import threading
import time
import tkinter as tk
from datetime import datetime, timezone
from tkinter import font

from udp_binary_protocol import unpack_binary_telemetry


UDP_BINARY_BROADCAST_PORT = 5002

BG_DARK = "#1e1e1e"
BG_PANEL = "#2d2d2d"
FG_WHITE = "white"
FG_AZ = "#4CAF50"
FG_EL = "#00BCD4"
FG_TRANSIT_ACTIVE = "lightgreen"
FG_MUTED = "#aaaaaa"
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


def format_delta_seconds(delta_seconds):
    sign = "+" if delta_seconds >= 0 else "-"
    return f"{sign}{abs(delta_seconds) * 1000.0:.1f} ms"


def scan_type_label(scan_type):
    return SCAN_TYPE_LABELS.get(int(scan_type), f"{int(scan_type):02d}")


class UdpBinaryTelemListener:
    def __init__(self, root):
        self.root = root
        self.root.title("Binary UDP Telemetry Listener")
        self.root.configure(bg=BG_DARK)
        self.running = True

        self.header_font = font.Font(family="Helvetica", size=18, weight="bold")
        self.large_font = font.Font(family="Helvetica", size=42, weight="bold")
        self.meta_font = font.Font(family="Helvetica", size=12, weight="bold")
        self.status_font = font.Font(family="Helvetica", size=11, weight="bold")
        self.data_font = font.Font(family="Courier", size=12, weight="bold")

        panel = tk.Frame(root, bg=BG_PANEL, bd=2, relief=tk.RIDGE)
        panel.pack(fill=tk.BOTH, expand=True, padx=12, pady=12)
        panel.grid_columnconfigure(0, weight=1)
        panel.grid_columnconfigure(1, weight=1)

        self.timestamp_var = tk.StringVar(value="Telem: —")
        tk.Label(
            panel, textvariable=self.timestamp_var, fg=FG_WHITE, bg=BG_PANEL,
            font=self.data_font, anchor="w",
        ).grid(row=0, column=0, sticky="w", padx=12, pady=(10, 2))

        self.status_var = tk.StringVar(
            value=f"Listening on UDP port {UDP_BINARY_BROADCAST_PORT}"
        )
        tk.Label(
            panel, textvariable=self.status_var, fg=FG_MUTED, bg=BG_PANEL,
            font=self.status_font, anchor="e",
        ).grid(row=0, column=1, sticky="e", padx=12, pady=(10, 2))

        self.utc_var = tk.StringVar(value="UTC: —")
        tk.Label(
            panel, textvariable=self.utc_var, fg=FG_MUTED, bg=BG_PANEL,
            font=self.data_font, anchor="w",
        ).grid(row=1, column=0, sticky="w", padx=12, pady=(0, 4))

        self.delta_var = tk.StringVar(value="Delta: —")
        tk.Label(
            panel, textvariable=self.delta_var, fg=FG_MUTED, bg=BG_PANEL,
            font=self.data_font, anchor="e",
        ).grid(row=1, column=1, sticky="e", padx=12, pady=(0, 4))

        tk.Label(
            panel, text="AZIMUTH", fg=FG_AZ, bg=BG_PANEL,
            font=self.header_font,
        ).grid(row=2, column=0, pady=(8, 0), sticky="ew")
        tk.Label(
            panel, text="ELEVATION", fg=FG_EL, bg=BG_PANEL,
            font=self.header_font,
        ).grid(row=2, column=1, pady=(8, 0), sticky="ew")

        self.az_var = tk.StringVar(value="—")
        self.el_var = tk.StringVar(value="—")
        tk.Label(
            panel, textvariable=self.az_var, fg=FG_WHITE, bg=BG_PANEL,
            font=self.large_font,
        ).grid(row=3, column=0, pady=(0, 8), sticky="ew")
        tk.Label(
            panel, textvariable=self.el_var, fg=FG_WHITE, bg=BG_PANEL,
            font=self.large_font,
        ).grid(row=3, column=1, pady=(0, 8), sticky="ew")

        self.scan_type_var = tk.StringVar(value="Scan Type: —")
        tk.Label(
            panel, textvariable=self.scan_type_var, fg=FG_WHITE, bg=BG_PANEL,
            font=self.meta_font, anchor="w",
        ).grid(row=4, column=0, columnspan=2, sticky="w", padx=12, pady=(4, 2))

        volume_sweep_frame = tk.Frame(panel, bg=BG_PANEL)
        volume_sweep_frame.grid(
            row=5, column=0, columnspan=2, sticky="w", padx=12, pady=2
        )
        self.volume_var = tk.StringVar(value="Vol #: —")
        self.sweep_var = tk.StringVar(value="Sweep #: —")
        tk.Label(
            volume_sweep_frame, textvariable=self.volume_var, fg=FG_WHITE,
            bg=BG_PANEL, font=self.meta_font,
        ).pack(side=tk.LEFT, padx=(0, 24))
        tk.Label(
            volume_sweep_frame, textvariable=self.sweep_var, fg=FG_WHITE,
            bg=BG_PANEL, font=self.meta_font,
        ).pack(side=tk.LEFT)

        self.scan_name_var = tk.StringVar(value="Scan: —")
        tk.Label(
            panel, textvariable=self.scan_name_var, fg=FG_WHITE, bg=BG_PANEL,
            font=self.meta_font, anchor="w",
        ).grid(row=6, column=0, columnspan=2, sticky="w", padx=12, pady=2)

        target_transit_frame = tk.Frame(panel, bg=BG_PANEL)
        target_transit_frame.grid(
            row=7, column=0, columnspan=2, sticky="w",
            padx=12, pady=(2, 12),
        )
        self.target_var = tk.StringVar(value="Target Angle: —")
        self.target_label = tk.Label(
            target_transit_frame, textvariable=self.target_var, fg=FG_WHITE,
            bg=BG_PANEL, font=self.meta_font,
        )
        self.target_label.pack(side=tk.LEFT, padx=(0, 24))
        tk.Label(
            target_transit_frame, text="Transit:", fg=FG_WHITE, bg=BG_PANEL,
            font=self.meta_font,
        ).pack(side=tk.LEFT)
        self.transit_var = tk.StringVar(value="—")
        self.transit_label = tk.Label(
            target_transit_frame, textvariable=self.transit_var, fg=FG_WHITE,
            bg=BG_PANEL, font=self.meta_font,
        )
        self.transit_label.pack(side=tk.LEFT, padx=(6, 0))

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(("", UDP_BINARY_BROADCAST_PORT))
        self.sock.settimeout(1.0)

        self.thread = threading.Thread(target=self._listen_loop, daemon=True)
        self.thread.start()
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def _listen_loop(self):
        while self.running:
            try:
                packet, addr = self.sock.recvfrom(65535)
            except socket.timeout:
                continue
            except OSError:
                break
            arrival_utc = time.time()
            self.root.after(0, self._handle_packet, packet, addr, arrival_utc)

    def _handle_packet(self, packet, addr, arrival_utc):
        try:
            parsed = unpack_binary_telemetry(packet)
        except ValueError as exc:
            self.status_var.set(f"Parse error from {addr[0]}:{addr[1]}: {exc}")
            return

        delta = arrival_utc - parsed["timestamp"]
        self.status_var.set(
            f"Listening on UDP port {UDP_BINARY_BROADCAST_PORT}"
            f"  |  last: {addr[0]}:{addr[1]}"
        )
        self.timestamp_var.set(f"Telem: {format_timestamp(parsed['timestamp'])}")
        self.utc_var.set(f"UTC: {format_timestamp(arrival_utc)}")
        self.delta_var.set(f"Delta: {format_delta_seconds(delta)}")
        self.az_var.set(f"{parsed['az']:.1f}°")
        self.el_var.set(f"{parsed['el']:.1f}°")
        self.scan_type_var.set(f"Scan Type: {scan_type_label(parsed['type'])}")
        self.volume_var.set(f"Vol #: {parsed['vol']}")
        self.sweep_var.set(f"Sweep #: {parsed['sweep']}")
        self.scan_name_var.set(f"Scan: {parsed['scan'] or '—'}")
        if abs(parsed["fixed"] - TARGET_ANGLE_IDLE) < 0.05:
            self.target_var.set("Target Angle: idle")
            self.target_label.config(fg=FG_IDLE)
        else:
            self.target_var.set(f"Target Angle: {parsed['fixed']:.1f}°")
            self.target_label.config(fg=FG_WHITE)
        self.transit_var.set("Yes" if parsed["transit"] else "No")
        self.transit_label.config(
            fg=FG_TRANSIT_ACTIVE if parsed["transit"] else FG_WHITE
        )

    def on_close(self):
        self.running = False
        try:
            self.sock.close()
        except OSError:
            pass
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    UdpBinaryTelemListener(root)
    root.mainloop()
