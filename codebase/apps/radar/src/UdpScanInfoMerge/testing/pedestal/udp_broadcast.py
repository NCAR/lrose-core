"""UDP telemetry broadcast at a fixed rate."""
import re
import socket
import threading
import time
from pathlib import Path

from .config import (
    UDP_BINARY_BROADCAST_PORT,
    UDP_BROADCAST_ADDR,
    UDP_BROADCAST_HZ,
    UDP_BROADCAST_PORT,
)
from .protocol import (
    format_udp_binary_telemetry_message,
    format_udp_telemetry_message,
)
from udp_binary_protocol import iter_binary_telemetry_packets


_ASCII_RECORDING_PATTERN = re.compile(
    r"^(?P<scan>.+)_ascii_(?P<timestamp>\d+)\.txt$"
)


def find_recorded_telemetry_pairs(sim_data_dir):
    """Return selectable ASCII recordings that have matching binary files."""
    sim_data_dir = Path(sim_data_dir)
    pairs = {}
    if not sim_data_dir.is_dir():
        return pairs

    for ascii_path in sorted(sim_data_dir.glob("*_ascii_*.txt")):
        match = _ASCII_RECORDING_PATTERN.fullmatch(ascii_path.name)
        if match is None:
            continue
        binary_name = (
            f"{match.group('scan')}_binary_{match.group('timestamp')}.bin"
        )
        binary_path = sim_data_dir / binary_name
        if binary_path.is_file():
            pairs[ascii_path.name] = (ascii_path, binary_path)
    return pairs


def _ascii_telemetry_timestamp(message):
    first_field = message.split(b",", 1)[0]
    if not first_field.startswith(b"t="):
        raise ValueError("ASCII telemetry message does not begin with t=")
    try:
        return float(first_field[2:])
    except ValueError as exc:
        raise ValueError("ASCII telemetry message has an invalid timestamp") from exc


class RecordedTelemetryBroadcaster:
    """Loop a paired recording onto the normal ASCII and binary UDP ports."""

    def __init__(
        self,
        ascii_path,
        binary_path,
        broadcast_addr=UDP_BROADCAST_ADDR,
        ascii_port=UDP_BROADCAST_PORT,
        binary_port=UDP_BINARY_BROADCAST_PORT,
    ):
        self.ascii_path = Path(ascii_path)
        self.binary_path = Path(binary_path)
        self.broadcast_addr = broadcast_addr
        self.ascii_port = ascii_port
        self.binary_port = binary_port
        self._stop_event = threading.Event()
        self._thread = None
        self.last_error = None
        self.packets_sent = 0
        self.loops_completed = 0
        self._frames = self._load_frames()

    @property
    def running(self):
        return self._thread is not None and self._thread.is_alive()

    @property
    def frame_count(self):
        return len(self._frames)

    def _load_frames(self):
        ascii_messages = [
            line
            for line in self.ascii_path.read_bytes().splitlines()
            if line.strip()
        ]
        binary_messages = list(
            iter_binary_telemetry_packets(self.binary_path.read_bytes())
        )
        if not ascii_messages:
            raise ValueError(f"{self.ascii_path.name} contains no telemetry")
        if len(ascii_messages) != len(binary_messages):
            raise ValueError(
                "recording pair has different message counts: "
                f"{len(ascii_messages)} ASCII and {len(binary_messages)} binary"
            )

        return tuple(
            (
                _ascii_telemetry_timestamp(ascii_message),
                ascii_message,
                binary_message,
            )
            for ascii_message, binary_message in zip(
                ascii_messages, binary_messages
            )
        )

    def start(self):
        if self.running:
            return
        self.last_error = None
        self.packets_sent = 0
        self.loops_completed = 0
        self._stop_event.clear()
        self._thread = threading.Thread(
            target=self._broadcast_loop,
            name="recorded-telemetry-broadcast",
            daemon=True,
        )
        self._thread.start()

    def stop(self):
        self._stop_event.set()
        thread = self._thread
        if thread is not None and thread is not threading.current_thread():
            thread.join(timeout=2.0)

    def _broadcast_loop(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        ascii_dest = (self.broadcast_addr, self.ascii_port)
        binary_dest = (self.broadcast_addr, self.binary_port)
        try:
            while not self._stop_event.is_set():
                deadline = time.monotonic()
                previous_timestamp = self._frames[0][0]
                for index, (timestamp, ascii_message, binary_message) in enumerate(
                    self._frames
                ):
                    if index:
                        deadline += max(0.0, timestamp - previous_timestamp)
                        wait_time = max(0.0, deadline - time.monotonic())
                        if self._stop_event.wait(wait_time):
                            return
                    elif self._stop_event.is_set():
                        return

                    sock.sendto(ascii_message, ascii_dest)
                    sock.sendto(binary_message, binary_dest)
                    self.packets_sent += 1
                    previous_timestamp = timestamp
                self.loops_completed += 1
        except OSError as exc:
            self.last_error = exc
        finally:
            sock.close()


class UdpBroadcastMixin:
    """Broadcast text and binary telemetry over UDP."""

    def toggle_telemetry_recording(self):
        if self.recording_active:
            self.stop_telemetry_recording()
        else:
            self.start_telemetry_recording()

    def start_telemetry_recording(self):
        """Open paired files for recording the exact outgoing datagrams."""
        if self.recording_active:
            return

        scan_name = (
            getattr(self, 'active_scan_name', '')
            if getattr(self, 'scan_active', False)
            else 'idle'
        )
        safe_scan_name = re.sub(r"[^A-Za-z0-9._-]+", "_", scan_name).strip("._")
        safe_scan_name = (safe_scan_name or "unnamed_scan")[:80]
        output_dir = Path(__file__).resolve().parents[1] / "sim_data"
        output_dir.mkdir(parents=True, exist_ok=True)

        unix_timestamp = int(time.time())
        while True:
            ascii_path = output_dir / f"{safe_scan_name}_ascii_{unix_timestamp}.txt"
            binary_path = output_dir / f"{safe_scan_name}_binary_{unix_timestamp}.bin"
            if not ascii_path.exists() and not binary_path.exists():
                break
            unix_timestamp += 1

        ascii_file = None
        binary_file = None
        try:
            ascii_file = ascii_path.open("xb")
            binary_file = binary_path.open("xb")
        except OSError as exc:
            if ascii_file is not None:
                ascii_file.close()
            if binary_file is not None:
                binary_file.close()
            try:
                ascii_path.unlink(missing_ok=True)
                binary_path.unlink(missing_ok=True)
            except OSError:
                pass
            self.append_log(f"Could not start telemetry recording: {exc}")
            return

        with self.recording_lock:
            self._ascii_recording_file = ascii_file
            self._binary_recording_file = binary_file
            self._recording_paths = (ascii_path, binary_path)
            self._recording_next_flush = time.monotonic() + 1.0
            self.recording_active = True

        self.record_btn.config(text="recording...", bg="#d32f2f", fg="white")
        self.append_log(
            f"Recording telemetry to {ascii_path.name} and {binary_path.name}."
        )

    def stop_telemetry_recording(self, update_button=True):
        """Flush and close the paired telemetry recording files."""
        with self.recording_lock:
            paths = self._recording_paths
            self._close_telemetry_recording_files()
        if update_button and hasattr(self, "record_btn"):
            self.record_btn.config(text="RECORD", bg="#4CAF50", fg="black")
        if paths:
            self.append_log(
                f"Stopped telemetry recording: {paths[0].name}, {paths[1].name}."
            )

    def _close_telemetry_recording_files(self):
        """Close recording files while ``recording_lock`` is held."""
        for recording_file in (
            self._ascii_recording_file,
            self._binary_recording_file,
        ):
            if recording_file is not None:
                try:
                    recording_file.flush()
                    recording_file.close()
                except OSError:
                    pass
        self._ascii_recording_file = None
        self._binary_recording_file = None
        self._recording_paths = None
        self.recording_active = False

    def _record_telemetry_messages(self, text_payload, binary_message):
        if not self.recording_active:
            return

        error = None
        with self.recording_lock:
            if not self.recording_active:
                return
            try:
                self._ascii_recording_file.write(text_payload + b"\n")
                self._binary_recording_file.write(binary_message)
                now = time.monotonic()
                if now >= self._recording_next_flush:
                    self._ascii_recording_file.flush()
                    self._binary_recording_file.flush()
                    self._recording_next_flush = now + 1.0
            except OSError as exc:
                error = exc
                self._close_telemetry_recording_files()

        if error is not None:
            self.call_on_main_thread(self._handle_recording_error, error)

    def _handle_recording_error(self, error):
        if hasattr(self, "record_btn"):
            self.record_btn.config(text="RECORD", bg="#4CAF50", fg="black")
        self.append_log(f"Telemetry recording stopped after a file error: {error}")

    def start_udp_broadcast(self):
        self._udp_broadcast_thread = threading.Thread(
            target=self._udp_broadcast_loop,
            daemon=True,
        )
        self._udp_broadcast_thread.start()

    def _udp_broadcast_loop(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        text_dest = (UDP_BROADCAST_ADDR, UDP_BROADCAST_PORT)
        binary_dest = (UDP_BROADCAST_ADDR, UDP_BINARY_BROADCAST_PORT)
        interval = 1.0 / UDP_BROADCAST_HZ
        try:
            while self.running:
                loop_start = time.time()
                with self.data_lock:
                    telem = dict(self.telemetry)
                    scan_active = self.scan_active
                    scan_name = getattr(self, 'active_scan_name', '') or ''
                text_message = format_udp_telemetry_message(
                    telem, scan_active=scan_active, scan_name=scan_name,
                )
                binary_message = format_udp_binary_telemetry_message(
                    telem, scan_active=scan_active, scan_name=scan_name,
                )
                text_payload = (
                    text_message.encode('ascii', errors='replace')
                    if text_message is not None
                    else None
                )
                if text_message is not None and binary_message is not None:
                    self._record_telemetry_messages(text_payload, binary_message)
                if text_payload is not None:
                    try:
                        sock.sendto(text_payload, text_dest)
                    except OSError as exc:
                        print(f"UDP text broadcast error: {exc}", flush=True)
                if binary_message is not None:
                    try:
                        sock.sendto(binary_message, binary_dest)
                    except OSError as exc:
                        print(f"UDP binary broadcast error: {exc}", flush=True)
                elapsed = time.time() - loop_start
                time.sleep(max(0.0, interval - elapsed))
        finally:
            sock.close()
