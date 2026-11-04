"""Standalone GUI for looping recorded ASCII and binary telemetry streams."""
import subprocess
import sys
import threading
import tkinter as tk
from pathlib import Path
from tkinter import ttk

from pedestal.udp_broadcast import (
    RecordedTelemetryBroadcaster,
    find_recorded_telemetry_pairs,
)


PROJECT_DIR = Path(__file__).resolve().parent
SIM_DATA_DIR = PROJECT_DIR / "sim_data"

BG_DARK = "#1e1e1e"
BG_PANEL = "#2d2d2d"
FG_WHITE = "white"
FG_MUTED = "#aaaaaa"
GREEN = "#4CAF50"
RED = "#d32f2f"
BLUE_GREY = "#607D8B"


class TelemetrySimulatorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Telemetry Simulator")
        self.root.configure(bg=BG_DARK)
        self.root.resizable(False, False)
        self.broadcaster = None
        self.recording_pairs = {}

        panel = tk.Frame(root, bg=BG_PANEL, bd=2, relief=tk.RIDGE)
        panel.pack(fill=tk.BOTH, expand=True, padx=12, pady=12)

        title_font = ("Helvetica", 16, "bold")
        label_font = ("Helvetica", 10, "bold")

        tk.Label(
            panel,
            text="Recorded Telemetry Playback",
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=title_font,
        ).pack(padx=16, pady=(14, 12))

        tk.Label(
            panel,
            text="ASCII recording:",
            fg=FG_WHITE,
            bg=BG_PANEL,
            font=label_font,
            anchor="w",
        ).pack(fill=tk.X, padx=16)

        self.recording_var = tk.StringVar()
        self.recording_combo = ttk.Combobox(
            panel,
            textvariable=self.recording_var,
            state="readonly",
            width=52,
        )
        self.recording_combo.pack(fill=tk.X, padx=16, pady=(4, 12))

        playback_buttons = tk.Frame(panel, bg=BG_PANEL)
        playback_buttons.pack(padx=16, pady=(0, 12))
        self.start_button = tk.Button(
            playback_buttons,
            text="START",
            width=14,
            bg=GREEN,
            fg="black",
            font=label_font,
            command=self.start_playback,
        )
        self.start_button.pack(side=tk.LEFT, padx=(0, 8))
        self.stop_button = tk.Button(
            playback_buttons,
            text="STOP",
            width=14,
            bg=RED,
            fg=FG_WHITE,
            font=label_font,
            state=tk.DISABLED,
            command=self.stop_playback,
        )
        self.stop_button.pack(side=tk.LEFT)

        listener_buttons = tk.Frame(panel, bg=BG_PANEL)
        listener_buttons.pack(padx=16, pady=(0, 12))
        tk.Button(
            listener_buttons,
            text="ASCII LISTENER",
            width=18,
            bg=BLUE_GREY,
            fg=FG_WHITE,
            font=label_font,
            command=lambda: self.launch_listener("udp_telem_listener.py"),
        ).pack(side=tk.LEFT, padx=(0, 8))
        tk.Button(
            listener_buttons,
            text="BINARY LISTENER",
            width=18,
            bg=BLUE_GREY,
            fg=FG_WHITE,
            font=label_font,
            command=lambda: self.launch_listener(
                "udp_binary_telem_listener.py"
            ),
        ).pack(side=tk.LEFT)

        self.status_var = tk.StringVar(value="Loading recordings...")
        tk.Label(
            panel,
            textvariable=self.status_var,
            fg=FG_MUTED,
            bg=BG_PANEL,
            font=label_font,
            anchor="w",
            wraplength=480,
        ).pack(fill=tk.X, padx=16, pady=(0, 14))

        self.refresh_recordings()
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)
        self.root.after(200, self.poll_playback)

    def refresh_recordings(self):
        selected = self.recording_var.get()
        self.recording_pairs = find_recorded_telemetry_pairs(SIM_DATA_DIR)
        names = tuple(self.recording_pairs)
        self.recording_combo["values"] = names
        if selected in self.recording_pairs:
            self.recording_var.set(selected)
        elif names:
            self.recording_combo.current(0)
        else:
            self.recording_var.set("")

        if names:
            self.status_var.set(
                f"Ready — {len(names)} complete recording pair(s) found."
            )
            self.start_button.config(state=tk.NORMAL)
        else:
            self.status_var.set(
                "No complete ASCII/binary recording pairs found in sim_data."
            )
            self.start_button.config(state=tk.DISABLED)

    def start_playback(self):
        if self.broadcaster is not None and self.broadcaster.running:
            return
        self.refresh_recordings()
        selected = self.recording_var.get()
        pair = self.recording_pairs.get(selected)
        if pair is None:
            self.status_var.set("Select a complete recording pair.")
            return

        try:
            broadcaster = RecordedTelemetryBroadcaster(*pair)
            broadcaster.start()
        except (OSError, ValueError) as exc:
            self.status_var.set(f"Could not start playback: {exc}")
            return

        self.broadcaster = broadcaster
        self.recording_combo.config(state=tk.DISABLED)
        self.start_button.config(state=tk.DISABLED)
        self.stop_button.config(state=tk.NORMAL)
        self.status_var.set(
            f"Streaming {selected} ({broadcaster.frame_count} messages per loop)."
        )

    def stop_playback(self):
        broadcaster = self.broadcaster
        if broadcaster is not None:
            broadcaster.stop()
        self.broadcaster = None
        self.recording_combo.config(state="readonly")
        self.stop_button.config(state=tk.DISABLED)
        self.start_button.config(
            state=tk.NORMAL if self.recording_pairs else tk.DISABLED
        )
        self.status_var.set("Telemetry playback stopped.")

    def poll_playback(self):
        broadcaster = self.broadcaster
        if broadcaster is not None:
            if broadcaster.running:
                self.status_var.set(
                    f"Streaming {self.recording_var.get()} — "
                    f"{broadcaster.packets_sent} messages sent, "
                    f"{broadcaster.loops_completed} loop(s) completed."
                )
            elif broadcaster.last_error is not None:
                error = broadcaster.last_error
                self.broadcaster = None
                self.recording_combo.config(state="readonly")
                self.stop_button.config(state=tk.DISABLED)
                self.start_button.config(state=tk.NORMAL)
                self.status_var.set(f"Playback stopped after network error: {error}")
        self.root.after(200, self.poll_playback)

    def launch_listener(self, script_name):
        script_path = PROJECT_DIR / script_name
        if not script_path.is_file():
            self.status_var.set(f"Listener not found: {script_path}")
            return

        def _launch():
            try:
                subprocess.Popen(
                    [sys.executable, str(script_path)],
                    cwd=str(PROJECT_DIR),
                )
            except OSError as exc:
                self.root.after(
                    0,
                    self.status_var.set,
                    f"Could not launch {script_name}: {exc}",
                )

        threading.Thread(target=_launch, daemon=True).start()
        self.status_var.set(f"Launched {script_name}.")

    def on_close(self):
        if self.broadcaster is not None:
            self.broadcaster.stop()
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    TelemetrySimulatorApp(root)
    root.mainloop()
