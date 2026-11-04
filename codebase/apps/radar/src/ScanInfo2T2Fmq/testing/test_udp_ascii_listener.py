#!/usr/bin/env python3
"""Replay recorded antenna telemetry for testing udp_ascii_listener."""

import argparse
import sys
import time
from pathlib import Path

from pedestal.udp_broadcast import (
    RecordedTelemetryBroadcaster,
    find_recorded_telemetry_pairs,
)


PROJECT_DIR = Path(__file__).resolve().parent
SIM_DATA_DIR = PROJECT_DIR / "sim_data"
DEFAULT_RECORDING = "Surv_fast_ascii_1785158255.txt"


def parse_args(recording_names):
    parser = argparse.ArgumentParser(
        description=(
            "Loop a recorded telemetry stream over UDP for testing the "
            "standalone C++ ASCII listener."
        )
    )
    parser.add_argument(
        "-a",
        "--address",
        default="127.0.0.1",
        help="destination IPv4 address (default: %(default)s)",
    )
    parser.add_argument(
        "-p",
        "--port",
        type=int,
        default=55001,
        help="destination port for ASCII telemetry (default: %(default)s)",
    )
    parser.add_argument(
        "-r",
        "--recording",
        choices=recording_names,
        default=(
            DEFAULT_RECORDING
            if DEFAULT_RECORDING in recording_names
            else recording_names[0]
        ),
        help="recording to replay (default: %(default)s)",
    )
    args = parser.parse_args()
    if not 1 <= args.port <= 65535:
        parser.error("port must be in the range 1 through 65535")
    return args


def main():
    recording_pairs = find_recorded_telemetry_pairs(SIM_DATA_DIR)
    if not recording_pairs:
        print(f"No paired telemetry recordings found in {SIM_DATA_DIR}", file=sys.stderr)
        return 1

    args = parse_args(tuple(recording_pairs))
    ascii_path, binary_path = recording_pairs[args.recording]
    broadcaster = RecordedTelemetryBroadcaster(
        ascii_path,
        binary_path,
        broadcast_addr=args.address,
        ascii_port=args.port,
    )

    try:
        broadcaster.start()
        print(
            f"Replaying {args.recording} to {args.address}:{args.port}; "
            "press Ctrl-C to stop",
            flush=True,
        )
        while broadcaster.running:
            time.sleep(0.25)
    except KeyboardInterrupt:
        print("\nStopping telemetry replay...", flush=True)
    finally:
        broadcaster.stop()

    if broadcaster.last_error is not None:
        print(f"Replay stopped after network error: {broadcaster.last_error}", file=sys.stderr)
        return 1

    print(f"Stopped after sending {broadcaster.packets_sent} packets")
    return 0


if __name__ == "__main__":
    sys.exit(main())
