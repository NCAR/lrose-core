# Lucid Codex handoff

Snapshot: 2026-11-12. Source is committed and pushed through `64212c323`,
which comments out the Route Winds implementation. The current worktree also
contains an update to the section catalog at the top of `paramdef.Lucid`, its
regenerated `Params.cc`, and this handoff update.

## Architecture

Lucid is the Qt replacement for the XView CIDD application. CIDD in
`apps/cidd/src/CIDD` remains the behavioral reference; `old/` contains other
legacy code retained for comparison. The application is intentionally still
transitional and retains some CIDD-style shared structures.

- `Lucid` initializes global state, field readers, projections, overlays,
  symbolic products, movie timing, and Qt application lifetime.
- `GuiManager` owns the main window and menus. Its timer coordinates MDV reads,
  rendering, Products, movies, reload, vertical sections, and automatic image
  generation.
- `GlobalData` retains shared CIDD-style state. `mread` contains one
  `MdvReader` per configured field; `h_win.page` is the selected field.
- `MdvReader` performs asynchronous horizontal and vertical MDV/Radx reads.
  Completed generations are installed on the GUI thread; stale generations
  are rejected.
- `WorldPlot` owns transforms, axes, margins, cached images, grid rendering,
  filled and line contours, range rings, and symbolic-product composition.
- `HorizView` renders the plan view and handles zoom, live-preview pan, cursor
  values, click reports, vertical-section selection, Product hover picking,
  and immutable movie composites.
- `VertManager` owns the vertical-section window and configuration menu;
  `VertView` renders the section, cursor/click values, and selected horizontal
  height or radar beam-height curve.
- `TimeControl` owns accepted and pending movie state, construction/playback,
  and playback timing. `TimePlotWidget` renders the time axis, availability,
  build progress, and interval gestures.
- `ProductMgr` captures display state and starts independent SPDB workers. It
  uses request generations and request keys to reject stale completions and
  installs decoded Product snapshots on the GUI thread.
- `Product`, `SymprodRender`, and `SymprodRenderObj` own and render decoded
  symbolic-product graphics using `QPainter`.
- `AutomaticImageGenerator` builds the immutable field/height/zoom/vertical-
  route job matrix, tracks generation readiness, and constructs safe catalog
  output paths.
- `Params` is generated from `paramdef.Lucid`. `LegacyParams` translates CIDD
  parameter files into a temporary Lucid TDRP file.

## Display functionality

- Horizontal grids support polygon and filled-contour rendering for
  rectangular and distorted grids, plus polar-radar polygons. Cells outside
  the current display domain are culled for non-polar rendering.
- Line contours can be overlaid on filled rendering or displayed alone with
  render method `NONE`. Render method, line contours, contour levels,
  column-max mode, and color-scale bounds can be changed from each field's
  context menu. Color-scale endpoints can be reset to their original values.
- Horizontal and vertical missing/out-of-scale data follow the same policy:
  values outside the color-scale bounds are not painted, leaving the missing
  data color visible.
- Column-max mode issues a separate horizontal MDV request using
  `setReadComposite()`. Vertical sections continue to use normal 3-D data.
- Reload invalidates and re-requests the horizontal field and an open vertical
  section.
- Mouse input is normalized at the event boundary. Left drag zooms, middle or
  Option-left drag pans, and right click selects a vertical section.
- Vertical sections support single-segment and multi-segment selection. The
  vertical window refreshes immediately on field and render-configuration
  changes.
- The vertical configuration menu controls selection mode, height bounds, and
  the selected-horizontal-height overlay.
- For polar fields, the selected elevation is converted to a radar beam-height
  curve using radar location and altitude. Invalid radar metadata suppresses
  the curve.
- Cursor values and click reports work in both views. Filled-contour cursor
  values are sampled from the underlying grid without regenerating contours.
- Concurrent MDV NetCDF/HDF5 access is serialized because the deployed stack
  was observed to crash when horizontal and vertical workers entered it
  concurrently.

## Symbolic products

- SPDB requests run independently of MDV reads. Per-product generations reject
  stale completions, and bounded retry prevents tight loops against failed
  servers.
- Product toggles, global enable, display changes, reload, and DataMapper
  expiry participate in request or repaint decisions.
- Retrieval or decode failure retains the previous valid interactive snapshot.
- Qt renderers cover text, polylines, icon lines, stroked icons, named and
  bitmap icons, arcs, rectangles, and station-report chunks.
- Product availability is queried independently of graphics requests and is
  displayed in `TimePlotWidget`.
- `symprod_debug != SYMPROD_DEBUG_OFF` enables request/result diagnostics.
  Request-local debug state is captured before the worker starts.
- Local METAR testing with `testing/CIDD.precip.test` returns and displays both
  `METARS-simple` and `METARS-labels` at the correct archive times.

Known Product issues still worth revisiting:

- A hover-picked object can remain from the previous movie frame until the
  cursor moves.
- Named-icon and bitmap-icon picking measures only the first icon origin in a
  multi-icon object.
- A successful zero-chunk response installs an empty snapshot. Decide whether
  that should mean a valid empty result or preserve the prior snapshot.
- Worker diagnostics can interleave because multiple threads write to `cerr`.

## Movie construction and playback

- Start builds sequentially from frame zero, independent of the selected frame
  at the time Start is pressed.
- Horizontal frames are captured only after the grid and exact symbolic-
  product request keys are ready. If a vertical window has a valid route,
  matching vertical composites are captured at the same frame index.
- Playback uses immutable `QImage` caches rather than mutable live layers.
- The time plot shows the frame being built in orange and completed frames in
  green. During playback, the displayed frame is gray.
- Normal and sweep playback, dwell, endpoint delay, Stop, and return to live
  display are implemented.
- View changes that invalidate cached geometry are disabled while the movie
  owns the canvases. Height changes may proceed in the background and become
  visible after Stop.
- Right-drag interval expansion requires Accept and updates the movie start and
  end widgets. Editing the frame interval no longer starts a build implicitly.
- A field selected after stopping a movie immediately returns to the normal
  non-movie request/render path.

The movie builder still waits indefinitely for an enabled Product that never
reaches its desired key. Stop remains available; a future movie-specific
timeout or best-effort policy may be useful.

## Interactive image output

- File -> Save-Image captures the horizontal view and, when visible, the
  vertical view. Names contain the view, sanitized field, selected UTC time,
  and a collision suffix so existing files are not replaced.
- Time controller Output Loop writes the immutable movie caches. From idle it
  first builds the complete loop; during playback it exports the existing
  cache.
- Loop output can launch `movie_convert_script` asynchronously with the
  ordered horizontal and vertical still lists.

## Automatic catalog image generation

Automatic generation is implemented; older notes describing it as scaffolding
are obsolete.

- `AutomaticImageGenerator` creates horizontal jobs for every enabled display
  field, configured height, and configured zoom. Optional vertical jobs cover
  every enabled field and configured `images_vsection_spec` route.
- `CREATE_IMAGES_THEN_EXIT` runs one batch and returns a success/failure exit
  status.
- `CREATE_IMAGES_ON_ARCHIVE_SCHEDULE` runs serialized batches from archive
  start through archive end at the configured interval.
- `CREATE_IMAGES_ON_REALTIME_SCHEDULE` aligns target times to the realtime
  interval and delay. Missed intervals are coalesced instead of queued.
- Realtime batches save and restore the accepted interactive field, time,
  height, zoom, frame index, and vertical route while the scheduler waits.
- Each job waits for its exact MDV generation. Requests have a configurable
  timeout, cancellation, failure recording, and continuation to the next job.
- Symbolic products have an explicit image policy:
  `images_require_all_symprods = TRUE` waits for all enabled Products and
  otherwise times out; the default `FALSE` waits for current requests to settle
  and saves with the available overlays.
- Images are published atomically with `QSaveFile`. Optional horizontal geo XML
  and `_latest_data_info` are written before the catalog notification.
- `make_images_transparent` converts exact matches of `background_color` to
  transparent pixels, following the CIDD transparency-key convention. Opaque
  output formats are rejected when transparency is enabled.
- Output naming distinguishes fields, multiple heights, multiple zooms, and
  vertical routes. Day directories and horizontal/vertical subdirectories are
  supported.
- Startup validation checks timeout range, image writer availability,
  transparency compatibility, output paths/subdirectories, delimiter safety,
  archive/realtime intervals, and scheduled filename time resolution.
- Per-batch and final summaries report attempted, saved, failed, timed-out, and
  product-omission image counts plus elapsed batch time.

The implementation has passed first-order archive generation, realtime
failure/recovery, and legacy CIDD-parameter translation tests. The next major
step is rigorous testing with the planned realtime and archive simulation
system, including delayed/missing MDV and SPDB data, recovery, naming,
sidecars, transparency, multiple heights/zooms, and vertical images.

## Parameters and legacy conversion

- `symprods_enabled_at_startup` is the canonical global Product startup
  parameter. Legacy `cidd.products_on` maps to it; the duplicate `products_on`
  parameter was removed.
- A static parameter audit is saved in `docs/parameter_status.txt`. It lists
  parameters with no current runtime read and noteworthy inconsistencies in
  `paramdef.Lucid` and `LegacyParams.cc`.
- The section catalog at the top of `paramdef.Lucid` now matches the actual
  sections. The nonexistent `WINDOWS` and retired `AUTO_VERT` entries were
  removed, and `VERT_VIEW` was corrected to `VSECTION_VIEW`.
- Route Winds was specialized legacy aviation functionality and will not be
  implemented in Lucid. Its parameters and code are currently commented out
  to retain context; remove the entire subsystem during later cleanup.
- Important cleanup candidates from the audit include duplicate vertical
  margin parameters, ignored projection offsets, ignored `terrain_active`, old
  debug booleans, obsolete XView rendering controls, and plugin/help settings
  with no Qt implementation.
- `LegacyParams::_readImageGenTdrp()` appears to overwrite `terrain_active`,
  likely from an old copy/paste error. Do not rely on that behavior when terrain
  cleanup begins.
- TDRP parameter design is still evolving. Current operational testing mainly
  uses CIDD parameter files through `LegacyParams`; checked-in Lucid parameter
  files under `testing/` may lag the generated schema.
- `tdrp_gen` regenerates `Params.hh` and `Params.cc` automatically when
  `paramdef.Lucid` changes and `make` runs.

## Important design decisions

- Use CIDD as the behavioral reference, but render with Qt rather than carrying
  forward the X11 drawing context.
- Add concise higher-level comments around non-obvious ownership, lifetime,
  concurrency, invariants, failure policy, and tradeoffs. Do not restate
  obvious operations.
- Worker threads own request-local state. Only the GUI thread publishes MDV or
  Product results and requests repaints.
- Product snapshot replacement is all-or-nothing. Failed interactive requests
  should not erase a useful displayed snapshot.
- Temporal Product selection is centralized so drawing and picking follow the
  same configured validity rules.
- Normalize mouse buttons at press time and retain the effective button for the
  complete gesture.
- Movie frames are GUI-thread-owned immutable composites. Horizontal and
  active vertical caches share the same frame index and request generation.
- Vertical routes are display geometry, not field-specific state, and survive
  field changes.
- Missing/bad grid values remain holes unless explicit mapping says otherwise.
  Invalid contour topology falls back to polygon rendering.
- Automatic image work is serialized through the GUI display state; fixed
  delays are not used to guess when asynchronous requests are complete.

## Outstanding work

1. Build the realtime/archive simulation environment and create a repeatable
   image-generation test matrix covering success, timeout, stale completion,
   recovery, and scheduler coalescing.
2. Verify automatic PNG transparency, geo XML, `_latest_data_info`, day
   directories, and DataMapper behavior with downstream consumers.
3. Exercise automatic combinations of multiple fields, heights, zooms, and
   vertical routes, including filename uniqueness and state restoration.
4. Continue visual comparison of symbolic products with CIDD and address the
   picking issues listed above.
5. Decide whether movies need a Product timeout/best-effort policy comparable
   to automatic images.
6. Continue vertical-window presentation cleanup and regression testing.
7. Remove Route Winds completely when the current commented history is no
   longer needed.
8. Work through `docs/parameter_status.txt` incrementally as affected features
   are cleaned up.
9. There are no automated GUI tests yet; current validation is interactive.

## Build and test

From `codebase/apps/radar/src/Lucid`, with the LROSE environment and Qt 6:

```bash
make -j4
git diff --check
./Lucid -p testing/CIDD.precip.test -debug
```

Suggested interactive smoke test:

1. Verify field, height, zoom, pan, render-method, contour, color-scale, and
   column-max changes in the horizontal view.
2. Select single- and multi-segment vertical sections; change fields and render
   settings while the vertical window is open.
3. Exercise horizontal and vertical cursor values and click reports.
4. Toggle Products globally and individually, then compare METAR placement and
   timing with CIDD.
5. Build and play horizontal-only and horizontal-plus-vertical movies. Verify
   progress colors, displayed-frame gray, Stop, field selection after Stop,
   dwell, endpoint delay, and sweep playback.
6. Test time-canvas selection, zoom, pan, automatic extension, right-drag plus
   Accept, and interval editing.
7. Exercise Reload with an open vertical section and enabled Products.
8. Test interactive Save-Image and Output Loop without overwriting prior files.
9. Run automatic create-once, archive schedule, and realtime schedule modes;
   inspect image names, summaries, optional sidecars, and exit status.
10. Press Ctrl-C during active MDV/SPDB/image requests and verify orderly
    shutdown.

Before committing, run `git status --short`. Preserve unrelated worktree files
and leave comparison screenshots untracked unless intentionally adding them as
test assets.

## Continuing on another machine

Commit and push both Lucid source changes and this handoff. On the other
machine, pull `lrose-core` and the private `~/git/codex` configuration
repository, run its `install.sh` if links need refreshing, and start Codex in
this directory. A useful opening request is:

```text
Read AGENTS.md, docs/codex-handoff.md, and docs/parameter_status.txt. Then
review the recent Lucid git history and current worktree before changing code.
```
