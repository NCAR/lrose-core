# LROSE simulator handoff

Snapshot: 2026-11-15. This document covers the standalone archive-to-realtime
simulators `RadxSim`, `MdvSim`, and `SpdbSim`. It is intentionally separate
from the Lucid-specific handoff. The applications are general LROSE utilities;
Lucid is only one possible downstream consumer.

## Status and source locations

All three initial implementations are present and build with the traditional
LROSE make system. Each application has TDRP parameters, command-line
overrides, procmap registration and heartbeats, interruptible replay waits,
run-once and repeat modes, and normal latest-data publication.

- `apps/Radx/src/RadxSim`: radar-volume replay using `RadxFile` and `RadxVol`.
- `apps/mdv_utils/src/MdvSim`: MDV/NCF replay using `DsMdvx`.
- `apps/spdb_utils/src/SpdbSim`: SPDB chunk replay using `DsSpdb`; format-aware
  payload rewriting is isolated in `PayloadRetimer`.

The applications are registered in the parent Makefiles, generated makefile
templates, lrose-core makefile lists, and CMake files. `Params.cc` and
`Params.hh` are generated from each application's `paramdef.*` file and are
kept with the source in the usual LROSE style.

Build an individual application with:

```bash
cd apps/Radx/src/RadxSim && make -j4
cd apps/mdv_utils/src/MdvSim && make -j4
cd apps/spdb_utils/src/SpdbSim && make -j4
```

Run an executable with `-h` for its short command-line interface, or use
`-print_params` for the complete TDRP schema.

## Shared replay model

Each simulator resolves and validates its archive input before registering the
replay as active. Input remains immutable and output must be separate from the
input location. The first output data-time anchor is wall-clock startup plus
`time_offset_secs`. Scheduling uses the archive interval divided by the
positive `replay_rate`:

```text
mapped_schedule = data_cycle_start
                + (source_schedule - archive_start) / replay_rate

deadline = monotonic_cycle_start
         + (source_schedule - archive_start) / replay_rate
         + delivery_delay
```

RadxSim predates the separate delivery-delay parameter and publishes at its
mapped completion deadline. MdvSim and SpdbSim expose `delivery_delay_secs`.
All waits use an absolute monotonic-clock deadline, preventing processing time
or wall-clock corrections from accumulating drift. Late products are
published immediately. Signals interrupt waits promptly.

On repeat, the next data-time and monotonic anchors advance beyond the prior
cycle by `inter_cycle_gap_secs`; the initial time offset is not reapplied.
Output timestamps therefore continue to increase across cycle boundaries.

The timestamp granularity differs deliberately by data model:

- RadxSim maps every ray through `replay_rate`, then rebuilds sweep and volume
  summaries from the rewritten rays. Both inter-volume and inter-ray timing
  are accelerated.
- MdvSim maps each file's scheduling time through `replay_rate`, then applies
  one offset to all time-bearing headers in that file. Relationships among
  generation, begin, end, centroid, expiration, and forecast times remain
  unchanged within a product.
- SpdbSim maps each chunk valid time through `replay_rate`, then applies one
  offset to its expiration and supported embedded payload times. Relationships
  among times within a chunk remain unchanged. Chunks sharing a source valid
  time are published together in one `DsSpdb::put()` operation.

## RadxSim

RadxSim accepts either explicit file paths (`FILE_LIST`, including paths from
`-f`) or an archive directory and inclusive start/end times (`ARCHIVE`). It
reads formats supported by Radx and writes complete CfRadial volumes.

For each volume it:

1. maps every ray time relative to the earliest resolved archive ray;
2. rebuilds sweep and volume metadata from the rays;
3. writes a complete CfRadial file through `RadxFile::writeToDir()`;
4. publishes `_latest_data_info` through `DsLdataInfo`, which also registers
   the new time with DataMapper.

Output names include start and end times plus subseconds to avoid collisions.
Optional day and year subdirectories and NetCDF compression are supported.
The output file rename is the data commit point; latest-data state is written
only after the completed file is visible.

Example:

```bash
RadxSim -params RadxSim.params \
  -start "2022 06 08 00 00 00" \
  -end   "2022 06 08 01 00 00" \
  -rate 10 -once
```

Current scope is complete-volume publication, not ray-stream or FMQ replay.
The initial implementation is intended for fixed radar data. Inputs with
independent time-bearing moving-platform or georeference metadata need a
defined retiming policy before being treated as supported.

## MdvSim

MdvSim also supports explicit paths and archive-range discovery. The input
mode is independently classified as `NON_FORECAST` or `FORECAST`; one process
must not mix the two hierarchies.

The scheduling time is `time_centroid` for non-forecast data and `time_gen`
for forecast data. Retiming shifts the following values when nonzero:

- master-header `time_gen`, `time_begin`, `time_end`, `time_centroid`,
  `time_expire`, and `forecast_time`;
- each field header's `forecast_time`;
- the master-header cached forecast values, kept consistent with the first
  field.

Before replay, MdvSim detects output-key collisions caused by multiple input
files mapping to the same one-second MDV path and asks the operator to reduce
the replay rate. It writes through `DsMdvx::writeToDir()`, then publishes
`DsLdataInfo` and DataMapper state only after the file write succeeds.

Output format can preserve MDV versus NCF input representation or force
classic MDV or NetCDF. Preserve mode supports MDV and NCF input only. Extended
paths and a year directory are configurable; forecast hierarchy is retained.

Example:

```bash
MdvSim -params MdvSim.params \
  -start "2022 06 08 00 00 00" \
  -end   "2022 06 08 01 00 00" \
  -rate 10 -once
```

## SpdbSim

SpdbSim makes one inclusive interval request with `DsSpdb::getInterval()`,
optionally filtering `data_type` and `data_type2`. It sorts the returned
chunks deterministically and retains the input `DsSpdb` object for the entire
replay because copied `Spdb::chunk_t` objects point into storage owned by that
reader.

Each output group uses `Spdb::putModeAdd`. The original product ID, product
label, data types, tags, and supported compression are preserved unless a
compression override is configured. `DsSpdb::put()` writes the daily SPDB
files, latest-data files, and DataMapper registration.

The initial format-aware payload handlers cover the requested common types:

| SPDB type | Embedded-time behavior |
| --- | --- |
| `generic_pt` | Rewrites `GenPt` time. |
| `stn_report` / `wx_obs` | Rewrites observation time and preserves legacy report, report-plus-XML, or bare XML representation. |
| `ac_posn`, `ac_posn_wmod` | Validates the structure array; time is SPDB metadata, so payload bytes remain unchanged. |
| `ac_georef` | Rewrites each 64-bit `time_secs_utc`. |
| `ac_data` | Validates fixed and client-data lengths; time is SPDB metadata, so payload bytes remain unchanged. |
| `ltg` | Rewrites every basic or extended lightning strike time. |
| `sigmet` | Rewrites start and end times. |
| `sig_air_met` | Rewrites issue, start, end, cancel, observation, forecast, forecast-point, and outlook-point times. |
| `sndg` | Rewrites legacy sounding launch time. |
| `sndg_plus` | Rewrites `Sndg` launch time. |
| `gen_poly` | Rewrites product and expiration times. |

By default an unsupported product ID rejects the replay, preventing stale
embedded timestamps from being published as current data. The optional
`COPY_UNSUPPORTED` policy shifts only chunk metadata and copies the opaque
payload unchanged. Malformed chunks can similarly reject the run or be
skipped with `continue_on_chunk_error`.

Example:

```bash
SpdbSim \
  -inurl /data/archive/spdb/metar \
  -outurl /data/realtime/spdb/metar \
  -start "2022 06 08 00 00 00" \
  -end   "2022 06 08 01 00 00" \
  -rate 10 -once
```

The current SPDB store and API impose signed 32-bit times and signed 32-bit
payload lengths. SpdbSim checks these limits before publication and will not
silently truncate or wrap them. This preserves the legacy limitation rather
than solving the planned post-2037 storage migration.

## Verification completed

The applications have built cleanly with the native make system. RadxSim and
MdvSim were exercised in the simulator project before SpdbSim was added.

SpdbSim was tested end to end with archive data under
`~/data/lucid/precip/spdb`:

- `metar` (product ID 101): 266 WxObs chunks at one valid time were replayed;
  `SpdbQuery -as_xml` showed current embedded observation times.
- `ltg` (product ID 200): chunk valid/expiration times and every extended
  strike time were shifted consistently.
- `spol_rhi` (product ID 60): SPDB metadata and the embedded `GenPt` time were
  shifted consistently.
- Output contained the expected daily `.data` and `.indx` files and all
  `_latest_data_info` variants.

Those fixtures validate the full I/O path and three payload families. The
remaining format handlers compile against their rapformats APIs but would
benefit from dedicated synthetic round-trip tests or representative archive
fixtures.

## Operational cautions and next work

- Give every instance separate archive and realtime paths, procmap instance
  names, logs, and project configuration. Never point output at archive data.
- `time_offset_secs` changes the first output data-time anchor; it does not
  add a wait. `delivery_delay_secs` changes publication deadlines without
  changing data timestamps.
- High replay rates can quantize MdvSim and SpdbSim schedules to one-second
  timestamps. MdvSim rejects path collisions. SPDB permits multiple chunks at
  one valid time under `putModeAdd`, but downstream behavior should be tested
  for the intended product.
- Add automated timing, loop-boundary, signal-termination, malformed-input,
  and DataMapper/downstream-consumer integration tests.
- Add representative tests for every SpdbSim payload handler, including
  network-byte-order round trips and zero-time sentinels.
- Fault injection is not implemented. Deterministic missing, delayed,
  duplicated, out-of-order, paused, and recovering-data modes remain a useful
  next phase shared by all three simulators.
- FMQ and ray-by-ray radar replay are outside the initial scope.

`apps/radar/src/Lucid/docs/simulator_design.txt` records the original RadxSim
design rationale and acceptance criteria. This handoff is the broader current
implementation guide for all three applications.
