"""Binary wire format shared by the UDP broadcaster and test listener."""
import struct


BINARY_TELEMETRY_MAGIC = b"TLM1"
# magic, timestamp, az, el, type, volume, sweep, fixed, transit, scan-name length
BINARY_TELEMETRY_HEADER = struct.Struct("!4sdffhiifBH")
MAX_SCAN_NAME_BYTES = 65535


def pack_binary_telemetry(fields):
    """Pack normalized telemetry fields into a version-1 network-order packet."""
    scan_bytes = str(fields["scan"]).encode("utf-8")
    if len(scan_bytes) > MAX_SCAN_NAME_BYTES:
        raise ValueError("scan name is too long for binary telemetry")
    return BINARY_TELEMETRY_HEADER.pack(
        BINARY_TELEMETRY_MAGIC,
        float(fields["timestamp"]),
        float(fields["az"]),
        float(fields["el"]),
        int(fields["type"]),
        int(fields["vol"]),
        int(fields["sweep"]),
        float(fields["fixed"]),
        1 if fields["transit"] else 0,
        len(scan_bytes),
    ) + scan_bytes


def unpack_binary_telemetry(packet):
    """Decode and validate a version-1 binary telemetry packet."""
    if len(packet) < BINARY_TELEMETRY_HEADER.size:
        raise ValueError("binary telemetry packet is too short")

    (
        magic,
        timestamp,
        az,
        el,
        scan_type,
        volume,
        sweep,
        fixed,
        transit,
        scan_length,
    ) = BINARY_TELEMETRY_HEADER.unpack_from(packet)
    if magic != BINARY_TELEMETRY_MAGIC:
        raise ValueError("unrecognized binary telemetry packet")

    expected_size = BINARY_TELEMETRY_HEADER.size + scan_length
    if len(packet) != expected_size:
        raise ValueError(
            f"binary telemetry packet has {len(packet)} bytes; expected {expected_size}"
        )

    try:
        scan_name = packet[BINARY_TELEMETRY_HEADER.size:].decode("utf-8")
    except UnicodeDecodeError as exc:
        raise ValueError("scan name is not valid UTF-8") from exc

    return {
        "timestamp": timestamp,
        "az": az,
        "el": el,
        "type": scan_type,
        "vol": volume,
        "sweep": sweep,
        "fixed": fixed,
        "transit": transit != 0,
        "scan": scan_name,
    }


def iter_binary_telemetry_packets(data):
    """Yield self-delimiting packets from a binary telemetry recording."""
    offset = 0
    while offset < len(data):
        remaining = len(data) - offset
        if remaining < BINARY_TELEMETRY_HEADER.size:
            raise ValueError(
                f"truncated binary telemetry header at byte offset {offset}"
            )

        header = BINARY_TELEMETRY_HEADER.unpack_from(data, offset)
        if header[0] != BINARY_TELEMETRY_MAGIC:
            raise ValueError(
                f"unrecognized binary telemetry packet at byte offset {offset}"
            )
        packet_size = BINARY_TELEMETRY_HEADER.size + header[-1]
        packet_end = offset + packet_size
        if packet_end > len(data):
            raise ValueError(
                f"truncated binary telemetry packet at byte offset {offset}"
            )

        packet = data[offset:packet_end]
        unpack_binary_telemetry(packet)
        yield packet
        offset = packet_end
