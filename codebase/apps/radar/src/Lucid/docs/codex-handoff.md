# Lucid Codex handoff

Snapshot: 2026-11-02, source committed through `56a849403`. At this snapshot
the source tree is clean; `main_window.png`, `main_window2.png`,
`vsection2.png`, `vsection3.png`, and `vsection_window.png` are untracked test
screenshots.

## Architecture

Lucid is a Qt 6 replacement for the XView CIDD application. Much of the data
model and terminology still comes from `apps/cidd/src/CIDD`; `old/` contains
additional copied legacy code. The application is intentionally transitional,
with legacy code, `#ifdef`s, and some HawkEye-derived vertical-view code still
present.

- `Main.cc` owns process startup, Qt application lifetime, and signal handling.
- `GuiManager` is the main coordinator. Its timer drives horizontal reads and
  rendering, it owns the menus/dialogs, and it connects the horizontal and
  vertical workflows.
- `GlobalData` remains the shared CIDD-style state. `GlobalData::mread` contains
  one `MdvReader` per configured data field; `h_win.page` is the selected field.
- `MdvReader` handles MDV/Radx access and retains returned headers, projection,
  data, color map, and vertical-section geometry.
- `WorldPlot` owns world/pixel transforms, margins, axes, cached images, grid
  rendering, and filled-contour rendering.
- `HorizView` draws the plan view and handles zoom, pan, cursor values, click
  reports, and vertical-section route selection.
- `VlevelManager` and `VlevelSelector` manage available vertical levels and the
  level-selection axis.
- `VertManager` is the separate vertical-section `QMainWindow`; `VertView`
  renders the returned section. These classes originated in HawkEye and still
  contain legacy radar-oriented scaffolding.
- `Params`, generated from `paramdef.Lucid`, and `LegacyParams` bridge the new
  TDRP configuration to remaining CIDD structures. Parameter design is still
  evolving and broad TDRP cleanup was deliberately deferred.

## Relevant files changed

- `Main.cc`: termination signals now use an async-signal-safe pipe and a Qt
  notifier, allowing normal Qt/C++ cleanup outside the POSIX signal handler.
- `GlobalData.cc`: the field menu defaults to one column.
- `MdvReader.cc/.hh`: horizontal request state/retry and shutdown handling were
  hardened; single-level vertical data are guarded; vertical sections use
  immutable request/result objects on worker threads; stale generations are
  discarded; worker threads are tracked for shutdown. A process-wide mutex
  serializes MDV/Radx NetCDF/HDF5 reads because the installed HDF5 library
  crashed when horizontal and vertical workers entered it concurrently.
- `VlevelManager.cc`, `VlevelSelector.cc`, `WorldPlot.cc/.hh`: corrected initial
  level limits and made aspect-ratio preservation configurable. The level
  selector disables it so its axis uses the intended min/max immediately.
- `WorldPlot.cc/.hh`, `HorizView.cc`: added filled contours for rectangular and
  distorted grids. Color boundaries are derived from persistent color-map
  entries, invalid topology falls back to cell rendering, and values outside
  the configured range use the bottom/top colors. Polar radar remains on its
  existing renderer by design.
- `ClickReport.hh`, `GuiManager.cc/.hh`, `HorizView.cc/.hh`, `Makefile`: added
  cursor-value overlay and a compact click-report dialog containing metadata
  plus values from the saved field-data queue. An open report refreshes when a
  newly selected field is added to that queue.
- `HorizView.cc/.hh`: normalized mouse input early. Left drag zooms; middle or
  Option-left drag pans with a live translated preview; right click builds a
  vertical route; Shift-right opens the context menu. On macOS the fallback
  maps Meta-modified left clicks to right clicks because Qt maps the Control
  key to `MetaModifier`. The effective button is captured on press for stable
  drag behavior.
- `VsectionRoute.cc/.hh`: new value type storing route points in world and
  lat/lon coordinates with cumulative distance.
- `GuiManager.cc/.hh`: retains the completed route, requests it for the current
  field, reissues it after field changes, ignores completions for superseded
  fields, and installs successful results in the vertical window.
- `VertManager.cc/.hh`, `VertView.cc/.hh`: replaced the HawkEye startup resize
  workaround with layout ownership, derive world limits from the returned
  route and vertical levels, render the section as colored cells, and show
  field/time/route metadata.
- `Params.cc/.hh`, `paramdef.Lucid`, `LegacyParams.cc`, and
  `testing/CIDD.precip.test`: recent work added/tuned vertical-window and color
  scale geometry parameters. Treat generated `Params` files consistently with
  the project's TDRP generation workflow.

## Current implementation status

The basic display workflow has been exercised successfully:

- Horizontal field display, zoom, active pan preview, cursor values, and click
  reports work.
- Click-report cells are compact and left-aligned; date and time are separate.
- Filled contours work for matching and distorted projections, with safe
  fallback to grid cells.
- A right-button route may contain one or more segments; double-right-click
  completes it, Escape cancels it, and Backspace removes the last point.
- Lucid asynchronously requests the MDV vertical section for the selected
  field, opens a separate window, and renders it across the full widget.
- Changing fields reuses the saved route and updates `VertView`.
- Rapid/concurrent horizontal and vertical access no longer crashes HDF5;
  library calls are serialized while remaining off the GUI thread.
- Ctrl-C follows the Qt shutdown path without the earlier thread-storage
  warnings in the tested cases.

There are no automated GUI tests. The statements above are based on interactive
testing and successful builds.

## Outstanding bugs and tasks

1. Clean up vertical-window presentation: axis/grid labeling, color-scale
   placement, margins, sizing, and platform-specific window placement still
   need refinement and regression testing.
2. Review the recent placement code. Temporary `XXXXXXXX...`/`SSSSSS...` debug
   output remains, and `GuiManager` calls `_vert->move(getTopRightPos())` while
   `VertManager` separately sets geometry for `_vertWindow`. The child view is
   layout-managed, so the intended target is likely the top-level window.
3. Re-request the retained vertical route when display time/movie frame changes,
   if that is the desired behavior. It currently refreshes on route completion
   and field selection only.
4. Decide vertical-read failure policy. Horizontal reads retry with capped
   backoff; vertical reads report failure but do not retry. In-flight vertical
   workers are not cancelled; generations make obsolete results harmless.
5. Continue removing unused HawkEye radar/beam code and stale comments from
   `VertView`/`VertManager`, then validate their zoom and overlay actions.
6. Consider whether the vertical display should remain colored grid cells or
   gain filled contours/interpolation. The current cell renderer correctly
   supports nonuniform vertical levels.
7. Revisit TDRP naming, defaults, generated files, and the legacy-parameter
   bridge after the display design stabilizes.
8. Test macOS Control-click/Option-drag behavior and Linux three-button behavior
   after further input changes.
9. The global MDV read mutex trades throughput for correctness. Keep it unless
   the deployed NetCDF/HDF5 stack is demonstrably thread-safe or I/O is moved
   to a single explicit queue.

## Important design decisions

- Add concise comments for non-obvious intent, ownership/lifetime, concurrency,
  invariants, failure policy, and tradeoffs; do not restate obvious operations.
- Normalize mouse buttons at the event boundary and retain the effective press
  button throughout a drag.
- A vertical route is display geometry, not field-specific state, so it is
  retained and reused across field selections.
- Vertical workers receive self-contained request snapshots and return complete
  result objects. Only the GUI thread installs results into `MdvReader`.
- Per-reader generations reject older requests; `GuiManager` also rejects a
  completion from a reader that is no longer the selected field.
- `VertView` keeps a non-owning pointer to an `MdvReader`; readers live for the
  application lifetime. Header/data replacement occurs atomically on the GUI
  thread.
- Qt layouts own widget geometry. Plot transforms are rebuilt from actual resize
  events rather than guessed construction-time dimensions.
- Missing/bad data remain holes unless mapping parameters explicitly map them
  to the data minimum. Invalid contour geometry falls back to a usable grid
  renderer instead of producing a blank plot.

## Build and test procedures

From `codebase/apps/radar/src/Lucid`, with the LROSE build/install environment
and Qt 6 configured:

```bash
make -j4
git diff --check
./Lucid -p testing/CIDD.precip.test -debug
```

Suggested interactive smoke test:

1. Confirm the initial vertical-level selector limits are correct.
2. Select several fields and verify the horizontal plot, hover value, and an
   open click report update.
3. Zoom with left drag; pan with middle drag (Option-drag on a Mac) and verify
   the live preview and Zoom Back behavior.
4. Build a multi-segment route with right clicks and finish with a right double
   click. Verify the vertical plot fills its window and reports the correct
   field, time, route length, and vertical units.
5. Change fields repeatedly while reads are active. Both views should settle on
   the latest field without a crash or stale vertical result.
6. Exercise rectangular and distorted filled-contour fields, including values
   below and above the color-map bounds.
7. Press Ctrl-C during an active MDV request and verify clean shutdown.

Before committing, inspect `git status --short`; screenshots and generated/build
artifacts should not be committed accidentally.
