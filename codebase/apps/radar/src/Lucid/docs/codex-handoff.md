# Lucid Codex handoff

Snapshot: 2026-11-10, source committed and pushed through `471f55a36`. The only
tracked worktree change is this handoff update. The unrelated untracked
`../ScanInfo2TsFmq/testing/sim_data/` directory belongs to another application
and must be left untouched.

## Architecture

Lucid is the Qt replacement for the XView CIDD application. CIDD in
`apps/cidd/src/CIDD` is the reference implementation; `old/` contains other
legacy code retained for comparison. The application is intentionally still
transitional.

- `Lucid` initializes global state, readers, movie timing, products, and Qt
  application lifetime.
- `GuiManager` owns the main window and menus. Its timer coordinates MDV reads,
  frame rendering, product requests, reload, and horizontal/vertical updates.
- `GlobalData` retains shared CIDD-style state. `mread` contains one
  `MdvReader` per configured field; `h_win.page` is the selected field.
- `MdvReader` performs asynchronous horizontal and vertical MDV/Radx reads.
  Completed results are installed on the GUI thread, and generations reject
  obsolete results.
- `WorldPlot` owns projection transforms, axes, margins, cached images, grid
  rendering, contours, and the transparent symbolic-product image.
- `HorizView` composes the plan view and handles zoom, pan, cursor values,
  click reports, vertical-section routes, Symprod hover picking, and immutable
  composite images used for movie playback.
- `VertManager` owns the vertical-section window and configuration menu;
  `VertView` renders the current section and its overlays.
- `ProductMgr` captures display state and starts independent SPDB worker
  threads. It installs completed results atomically, retains a previous valid
  snapshot on failure, and requests a product-layer repaint through callbacks.
- `Product` owns the immutable decoded snapshot for one configured product and
  applies the configured temporal selection rules at render time.
- `SymprodRender` decodes one Symprod buffer into owned `SymprodRenderObj`
  instances. `SymprodRenderObj` subclasses render natively with `QPainter`.
- `SymprodRenderContext` is the small read-only display state passed through
  the Qt rendering and picking path.
- `Params` is generated from `paramdef.Lucid`; `LegacyParams` translates CIDD
  parameter files into the temporary TDRP-based form used by Lucid.
- `TimeControl` owns the time dialog, accepted and pending movie state, movie
  build/playback state machine, and playback timer. `TimePlotWidget` renders
  the compact time axis, data availability, build progress, and mouse gestures.

## Major implementation completed

### Horizontal and vertical display

- Horizontal polygon and filled-contour rendering supports rectangular and
  distorted grids, current-zoom culling, optional line contours, interactive
  render configuration, color-scale editing/reset, and column-max requests.
- Reload invalidates and re-requests the horizontal field and an open vertical
  section.
- Mouse input is normalized at the event boundary: left drag zooms, middle or
  Option-left drag pans with a live preview, right click builds a vertical
  route, and Shift-right opens the context menu.
- Cursor-value overlays and the click-report dialog work in horizontal and
  vertical views. Filled-contour hover reads use the underlying data grid and
  do not regenerate contours.
- Vertical routes support single-segment and multi-segment modes. The vertical
  section refreshes on field changes and has configuration for plot height
  bounds and the selected horizontal height/beam-height curve.
- Polar fields compute the selected-elevation beam-height curve using radar
  location and altitude; invalid radar locations suppress the curve.
- Concurrent MDV NetCDF/HDF5 access is serialized because the deployed library
  stack was observed to crash when horizontal and vertical workers entered it
  concurrently.

### Symbolic products

- SPDB retrieval runs independently of MDV reads. Per-product generations
  reject stale completions; bounded retry prevents a failed server from being
  polled continuously.
- Product toggles, global enable, display-state changes, reload, and DataMapper
  expiry participate in request or repaint decisions.
- Retrieval/decode failure reports an error without removing the last valid
  displayed snapshot.
- Symprod objects are decoded with bounds checks and owned with `unique_ptr`.
  The old X11 `RenderContext`, inactive `HAVE_XID` rendering, and legacy raw
  ownership were removed from the active implementation and build files.
- Qt renderers exist for text, polylines, icon lines, stroked icons, named and
  bitmap icons, arcs, rectangles, and station-report chunks.
- Hover picking can expose normally hidden details, and selected geometry is
  available through the export API.
- `symprod_debug != SYMPROD_DEBUG_OFF` enables request/result diagnostics.
  `SymprodReadRequest::debug` makes the captured setting available inside the
  worker without accessing `ProductMgr` state from another thread.
- Movie frame times were previously left at zero because CIDD's
  `reset_time_points()` was never ported. `Lucid::_initMovieFrameTimes()` now
  initializes regular, daily-climatology, and yearly-climatology frames after
  `movie.start_time` receives its final adjustment. Epoch bounds come from the
  initialized frames.

### Time controller and data availability

- `TimePlotWidget` is owned by the time dialog, moves and resizes with it, and
  uses `time_plot_bar_height_pixels` as its nominal height. It grows vertically
  when required to keep multiple Product rows from overlapping.
- The visible axis covers the movie interval. UTC labels are clipped only in X,
  leaving the lower margin available for text.
- The selected grid field publishes its MDV time list as dots; the selected
  data time uses a larger dot. Active symbolic products have separate colored
  tick rows and use real SPDB `compileTimeList()` results, so availability is
  complete even when `symprod_short_requests = TRUE`.
- Grid and Product labels have opaque margin-colored backgrounds so dense dots
  and ticks cannot obscure the text. Availability failure is supplementary:
  it preserves the prior list and does not discard otherwise valid data.
- Mouse behavior:
  - single left click selects and immediately retrieves the containing frame;
  - left drag zooms the visible interval without changing frame spacing;
  - middle drag pans by integral frames, retrieves the newly visible range,
    and automatically extends the movie on either side when it crosses a movie
    boundary;
  - right drag maps the old movie boundaries to the rectangle boundaries and
    computes a frame-aligned expanded movie, applied through Accept;
  - double left click fits the complete movie;
  - Shift-right click opens a help dialog describing these gestures.
- Accepted movie changes synchronize the legacy `GlobalData::movie` frame
  times and epoch with the Qt controller. Pans and expansions invalidate MDV
  and Product availability caches and use the normal full retrieval path.

### Movie construction and playback

- Start now always builds the movie sequentially from frame zero through the
  last frame, independent of the frame selected when Start is pressed.
- Each frame is requested through the normal asynchronous MDV/Product path.
  `GuiManager` captures a frame only after its grid is rendered and every
  active Product has installed the exact desired request key; an older valid
  Product snapshot is not considered ready for a new frame.
- `HorizView` owns immutable composite `QImage` frames. Playback displays these
  cached composites rather than mutable live `WorldPlot` layers.
- The time canvas fills the frame currently being built in gold and completed
  frames in green. The status bar also reports the frame being built.
- Playback starts automatically after every frame is cached. The dwell control
  determines normal frame duration. Non-sweep playback moves first-to-last,
  applies the configured end delay, and restarts at frame zero. Sweep playback
  reverses at both ends and applies the end delay at each endpoint.
- Stop cancels an active build or playback timer. Manual time selection exits
  cached playback and returns to live rendering.

## Current Symprod test status

Using `testing/CIDD.precip.test`, local `Metar2Symprod` requests now use the
correct archive interval. The observed diagnostics were:

```text
requested=2022/06/07 22:30:00 UTC to 2022/06/08 00:12:00 UTC
status=0 chunks=8 bytes=18964 stale=no
```

Both `METARS-simple` and `METARS-labels` return data. METAR station products are
visible in Lucid and comparison screenshots reviewed during this session were
favorable at first order: stations appeared at corresponding locations with
temperature/dew point, weather, labels, winds, and ceiling information.
Remaining differences appeared mainly related to Qt font metrics, spacing,
symbol layout, and the differing plot aspect/domain.

The worker request messages can interleave because two threads write directly
to `cerr`. The structured `SYMPROD result` messages execute on the GUI thread
and remain readable.

## Outstanding bugs and tasks

1. Exercise the new movie builder with operational MDV and SPDB sources.
   Verify strict first-to-last construction, grid/Product alignment, progress
   colors, dwell changes, end delays, normal looping, Sweep reversal, Stop
   during build, and Stop during playback.
2. The movie builder currently waits indefinitely if an active Product never
   reaches its desired request key. Stop remains available and Product retry
   continues, but a future timeout/failure policy should allow a grid-only or
   explicitly incomplete frame instead of stalling forever.
3. Cached movie composites are sized to the horizontal widget at build time
   and can consume substantial memory for very large frame counts. Decide
   whether resize/display-setting changes should invalidate the loop and
   whether a cache memory limit or reduced representation is needed.
4. Movie playback currently caches the horizontal composite only. Decide
   whether an open vertical section should be captured and animated in lockstep.
5. Test `METARS-simple` and `METARS-labels` separately against CIDD, then test
   zoom scaling, label suppression, reload, and product toggling.
6. Address two review findings in the new picking path:
   - A picked Symprod can remain from the previous movie frame until the cursor
     moves. Clear/reselect it on frame changes or validate it against the
     current `Product::_selectForDisplay()` result before drawing.
   - Named-icon and bitmap-icon picking measures only `_iconOrigins[0]`.
     Compute the minimum distance over every rendered icon origin.
7. Consider moving worker diagnostic formatting to one buffered line or the GUI
   thread so concurrent request messages do not interleave.
8. A successful zero-chunk SPDB response currently installs an empty snapshot
   and reports the product as ready. Decide whether it should preserve the old
   snapshot or explicitly report “no products.”
9. Continue visual comparison of station-model layout, especially text
   justification, wind-barb geometry, background treatment, and dense overlap.
10. Picking/export data paths exist, but the complete legacy draw/export UI has
   not been ported.
11. Continue vertical-window presentation cleanup and regression tests.
12. TDRP parameter names/defaults and checked-in Lucid test parameter files are
   intentionally deferred while the parameter design remains fluid. Current
   testing primarily uses CIDD parameter files through `LegacyParams`.
13. There are no automated GUI tests; current validation is interactive.

## Important design decisions

- Use CIDD as the behavioral reference, but implement rendering with Qt rather
  than carrying forward the X11 drawing context.
- Add concise higher-level comments for non-obvious intent, lifetime,
  concurrency, invariants, failure policy, and tradeoffs. Do not restate
  obvious operations.
- SPDB workers own request-local state. Only the GUI thread publishes decoded
  snapshots and invokes repaint/status callbacks.
- Snapshot replacement is all-or-nothing. Malformed or failed responses must
  not erase still-useful displayed products.
- Temporal selection is centralized in `Product::_selectForDisplay()` so normal
  drawing and picking use the same configured render rules.
- Normalize mouse buttons on press and retain the effective button for the
  entire gesture.
- Treat availability lists as independent snapshots from displayed data.
  Short Product requests must not substitute for the SPDB time list.
- Movie frames are GUI-thread-owned immutable composites. Construction is
  sequential; playback begins only after every requested layer for every frame
  is ready.
- Vertical routes are display geometry rather than field-specific state and
  are retained across field changes.
- Missing/bad grid values remain holes unless explicit mapping parameters say
  otherwise. Invalid contour topology falls back to polygon rendering.
- `tdrp_gen` regenerates `Params.hh` and `Params.cc` automatically when
  `paramdef.Lucid` changes and `make` is run.

## Build and test

From `codebase/apps/radar/src/Lucid`, with the LROSE environment and Qt 6:

```bash
make -j4
git diff --check
./Lucid -p testing/CIDD.precip.test -debug
```

Suggested smoke test:

1. Confirm the selected archive time and the `SYMPROD result` request interval.
2. Compare one METAR product at a time with CIDD at the same frame and zoom.
3. Toggle each product and the global Products enabled action.
4. Zoom and pan; verify station placement, symbol scaling, and clipping.
5. Advance movie frames and ensure products follow the frame validity window.
6. Exercise time-canvas click, zoom, pan, automatic extension, right-drag
   expansion, double-click fit, and Shift-right help. Confirm availability dots
   and ticks refresh for the displayed interval.
7. Press Start from a middle frame and confirm construction nevertheless begins
   at frame zero. Verify gold/green progress, automatic playback, dwell and
   delay, Sweep direction changes, and Stop in both build and playback phases.
8. Use Reload and confirm MDV, products, and an open vertical section refresh.
9. Exercise horizontal and vertical hover/click values and vertical route modes.
10. Change fields while reads are active and verify that stale completions do
   not replace the current display.
11. Press Ctrl-C during active MDV/SPDB or movie-build requests and verify clean
    shutdown.

Before committing, run `git status --short`. In particular, leave comparison
screenshots untracked unless they are intentionally being added as test assets.

## Continuing on another machine

Commit and push both the Lucid source and this handoff. On the other machine,
pull `lrose-core` and the private `~/git/codex` configuration repository, run
its `install.sh` if links need refreshing, and start Codex in this directory.
Use an opening request such as:

```text
Read AGENTS.md and docs/codex-handoff.md, then review the recent Lucid git
history and current worktree. Summarize the current state before changing code.
```
