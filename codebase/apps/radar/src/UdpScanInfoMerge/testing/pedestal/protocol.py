"""Galil telemetry helpers (no UI dependencies)."""
import re
from datetime import datetime, timezone

from udp_binary_protocol import pack_binary_telemetry

# Galil TIME increments once per servo-loop update. With default TM=1000 each tick is 976 µs.
# Telemetry is emitted every ~10 servo loops (~9.76 ms / 100 Hz). Kept for documentation / future use.
GALIL_TIME_TICK_MICROSECONDS = 976.0
GALIL_TIME_TICK_SECONDS = GALIL_TIME_TICK_MICROSECONDS / 1_000_000.0
TELEM_VALUE_COUNT = 22
TELEM_TIME_INDEX = 18
POINTING_TARGET_IDLE = 400.0
SURVEY_SCAN_TYPES = (1.0,)
SECTOR_SCAN_TYPES = (2.0, 3.0, 6.0)
RHI_SCAN_TYPES = (4.0, 5.0, 7.0)
# Horizontal-scan family: pointing axis is elevation (angle list).
SURVEY_SECTOR_SCAN_TYPES = SURVEY_SCAN_TYPES + SECTOR_SCAN_TYPES


def classify_scan_family(scan_type_id):
    """Return 'survey', 'sector', 'rhi', or None for an unknown type id."""
    try:
        type_id = float(scan_type_id)
    except (TypeError, ValueError):
        return None
    if type_id in SURVEY_SCAN_TYPES:
        return "survey"
    if type_id in SECTOR_SCAN_TYPES:
        return "sector"
    if type_id in RHI_SCAN_TYPES:
        return "rhi"
    return None


def pointing_target_angle(scan_active, scan_type_id, az_trg, el_trg, telem_target=POINTING_TARGET_IDLE):
    """Return pointing-axis target for post-processing; 400 = no active scan."""
    if not scan_active:
        return POINTING_TARGET_IDLE
    try:
        type_id = float(scan_type_id)
    except (TypeError, ValueError):
        return telem_target
    if type_id in SURVEY_SECTOR_SCAN_TYPES:
        return el_trg
    if type_id in RHI_SCAN_TYPES:
        return az_trg
    return telem_target


def axis_motor_enabled(mo_val):
    """Galil _MO: 0 = servo on, 1 = motor off."""
    try:
        return float(mo_val) < 0.5
    except (TypeError, ValueError):
        return False


def normalize_el_pos(el_pos, el_trg):
    """Fold sector-scan EL readings into 0-360."""
    try:
        pos = float(el_pos)
        trg = float(el_trg)
    except (TypeError, ValueError):
        return el_pos
    if trg >= 0:
        while pos < 0:
            pos += 360.0
        while pos >= 360.0:
            pos -= 360.0
    return pos


def format_wall_time(epoch_seconds):
    """Format a ``time.time()`` value as UTC ``YYYY-MM-DD HH:MM:SS.mmm``."""
    if epoch_seconds is None:
        return None
    dt = datetime.fromtimestamp(epoch_seconds, tz=timezone.utc)
    return dt.strftime("%Y-%m-%d %H:%M:%S") + f".{dt.microsecond // 1000:03d}"


def udp_telemetry_fields(telem, scan_active=False, scan_name=''):
    """Normalize the fields shared by the text and binary UDP streams."""
    timestamp = telem.get('timestamp')
    if timestamp is None:
        return None
    scan_type = int(round(float(telem.get('scan_type', 0))))
    volume = int(round(float(telem.get('volume', 0))))
    sweep = int(round(float(telem.get('tilt', 0))))
    transit = 1 if telem.get('transit') else 0
    fixed = pointing_target_angle(
        scan_active,
        telem.get('scan_type', 0),
        telem['az_trg'],
        telem['el_trg'],
        telem.get('target_angle', POINTING_TARGET_IDLE),
    )
    if not scan_active:
        name = 'idle'
    else:
        name = '' if scan_name is None else str(scan_name)
    return {
        "timestamp": float(timestamp),
        "az": float(telem['az_pos']),
        "el": float(telem['el_pos']),
        "type": scan_type,
        "vol": volume,
        "sweep": sweep,
        "fixed": float(fixed),
        "transit": bool(transit),
        "scan": name,
    }


def format_udp_telemetry_message(telem, scan_active=False, scan_name=''):
    """Build a comma-delimited ``label=value`` UDP telemetry line."""
    fields = udp_telemetry_fields(telem, scan_active, scan_name)
    if fields is None:
        return None
    return (
        f"t={fields['timestamp']:.3f},"
        f"az={fields['az']:.1f},"
        f"el={fields['el']:.1f},"
        f"type={fields['type']:02d},"
        f"vol={fields['vol']},"
        f"sweep={fields['sweep']},"
        f"fixed={fields['fixed']:.1f},"
        f"tr={int(fields['transit'])},"
        f"scan={fields['scan']}"
    )


def format_udp_binary_telemetry_message(telem, scan_active=False, scan_name=''):
    """Build a versioned binary UDP telemetry packet."""
    fields = udp_telemetry_fields(telem, scan_active, scan_name)
    if fields is None:
        return None
    return pack_binary_telemetry(fields)


def parse_mg_first_float(resp_bytes):
    parts = re.findall(
        r'[-+]?\d*\.\d+|[-+]?\d+',
        resp_bytes.decode('ascii', errors='ignore'),
    )
    if not parts:
        return None
    try:
        return float(parts[-1])
    except ValueError:
        return None
