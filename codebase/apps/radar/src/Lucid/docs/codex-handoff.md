# Lucid Codex handoff

Snapshot: 2026-11-14. Source and test changes through native polar RHI
rendering and returned-route display are committed and pushed in `421c363fc`
(`implementing polar RHIs`). The current worktree contains only the three
documentation updates described here.

## Architecture

Lucid is the Qt replacement for the XView CIDD application. CIDD in
`apps/cidd/src/CIDD` remains the behavioral reference; `old/` contains other
legacy code retained for comparison. The application is intentionally still
transitional and retains some CIDD-style shared structures.

- `Lucid` initializes global state, field readers, projections, overlays,
  symbolic products, movie timing, and Qt application lifetime.
- `GuiManager` owns the main window and menus. Its timer coordinates MDV reads,
  rendering, Products, reload, vertical sections, and the manager callbacks
  which must touch live widgets.
- `GlobalData` retains the remaining shared CIDD-style display and data state.
  Much of the legacy timing, interaction, click-location, and unused layer
  state has been removed. `mread` contains one `MdvReader` per configured
  field.
- `MdvReader` performs asynchronous horizontal and vertical MDV/Radx reads.
  Horizontal workers receive immutable requests; the reader retains one
  latest-pending request while a blocking read is active. Exact request keys
  allow installed data to be reused, and completed generations are installed
  only on the GUI thread.
- `WorldPlot` owns transforms, axes, margins, cached images, grid rendering,
  filled and line contours, range rings, and symbolic-product composition.
- `HorizView` renders the plan view and handles zoom, live-preview pan, cursor
  values, click reports, vertical-section selection, Product hover picking,
  and immutable movie composites.
- `VertManager` owns the vertical-section window and configuration menu;
  `VertView` renders Cartesian sections and native polar RHIs, cursor/click
  values, terrain and wind overlays, and the selected horizontal height or
  radar beam-height curve.
- `TimeControl` owns the accepted timeline, pending time edits, and playback
  controls. `TimePlotWidget` renders the time axis, availability, build
  progress, and interval gestures.
- `MovieManager` owns movie build/capture/playback state, immutable-cache
  export, and the per-frame dependency timeout. `GuiManager` supplies narrow
  callbacks for requests and view capture.
- `ProductMgr` captures display state and starts independent SPDB workers. It
  uses request generations and request keys to reject stale completions and
  installs decoded Product snapshots on the GUI thread.
- `Product`, `SymprodRender`, and `SymprodRenderObj` own and render decoded
  symbolic-product graphics using `QPainter`.
- `AutomaticImageGenerator` builds the immutable field/height/zoom/vertical-
  route job matrix. `ImageGenerationManager` owns scheduling, readiness,
  timeout, publication, summaries, and lifecycle state, using callbacks for
  the few operations which must touch the live display.
- `Params` is generated from `paramdef.Lucid`. `LegacyParams` translates CIDD
  parameter files into a temporary Lucid TDRP file.

## Display functionality

- Horizontal grids support polygon and filled-contour rendering for
  rectangular and distorted grids, plus polar-radar polygons. Cells outside
  the current display domain are culled for non-polar rendering.
- Filled gridded overlays now preserve the base image. Render method `NONE`
  prepares the composition correctly for line-only overlays instead of
  clearing or retaining an unrelated prior field.
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
- Map layers are cached independently from MDV grids. Global and per-map menu
  changes rebuild that cache immediately without issuing a field request. The
  global Maps toggle now controls painting as intended.
- Mouse input is normalized at the event boundary. Left drag zooms, middle or
  Option-left drag pans, and right click selects a vertical section.
- Vertical sections support single-segment and multi-segment selection. The
  vertical window refreshes immediately on field and render-configuration
  changes.
- Native measured RHIs returned by `DsMdvServer` are detected from
  `PROJ_RHI_RADAR` plus `VERT_TYPE_AZ`. Their range/elevation gates are
  transformed with `BeamHeight` and rendered as distance/height polygons.
  `NONE` is unfilled; `FILLED_CONTOURS` deliberately falls back to polygons.
- Polar RHI cursor and click lookup invert the same beam geometry. The title
  includes the returned azimuth and the displayed axes remain ground distance
  and height MSL in km.
- `DsMdvServer` normally returns the complete measured RHI from the radar to
  maximum range. `MdvReader` retains those returned waypoints and `HorizView`
  redraws the actual path when it differs from the user's shorter selection.
  The original route remains the request definition; adjusted terrain is
  requested against the returned geometry without repeating the primary read.
- The vertical configuration menu controls selection mode, height bounds, and
  the selected-horizontal-height overlay.
- For polar fields, the selected elevation is converted to a radar beam-height
  curve using radar location and altitude. Invalid radar metadata suppresses
  the curve.
- Cursor values and click reports work in both views. Filled-contour cursor
  values are sampled from the underlying grid without regenerating contours.
- Model U/V winds can be rendered over horizontal and vertical fields using
  vectors or weather-map barbs. Vertical symbols retain their true-north
  orientation rather than representing along-section or vertical velocity.
- Terrain can mask model plan views where terrain exceeds the selected height,
  and can be drawn and toggled in the vertical section. It is not applied to
  polar plan-view radar grids.
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

Product details still worth revisiting:

- A successful zero-chunk response installs an empty snapshot. Decide whether
  that should mean a valid empty result or preserve the prior snapshot.
- Worker diagnostics can interleave because multiple threads write to `cerr`.

Multi-instance named and bitmap icons now test every rendered origin when
picking. Movie interaction also clears transient Product picks rather than
carrying a live hover selection into cached playback.

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
- Each frame has a configurable dependency timeout. A missing grid, Product,
  wind, terrain profile, or vertical section stops the build with a message
  naming the outstanding layers instead of leaving the UI in Building state.
- A capture failure follows the same stop path, so movie construction cannot
  remain permanently active after an image-capture error.
- Right-drag interval expansion requires Accept and updates the movie start and
  end widgets. Editing the frame interval no longer starts a build implicitly.
- A field selected after stopping a movie immediately returns to the normal
  non-movie request/render path.

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
  height, zoom, frame index, vertical route, and vertical-window visibility
  while the scheduler waits.
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
- Automatic-generation instances disable conflicting interactive controls.
  Generated captures suppress transient cursor text, click marks, rubber-band
  geometry, pan previews, and picked-Product graphics.
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

## Recent reliability and cleanup work

- The original CIDD movie structure in `GlobalData` has been replaced by the
  timeline owned by `TimeControl`.
- Horizontal MDV cache reuse is active, and a request arriving during a worker
  read replaces the bounded latest-pending slot rather than being lost.
- Movie and automatic-image waits are bounded independently. Live display
  readers may continue their normal retry policy without trapping a batch or
  movie build indefinitely.
- Qt parent ownership now covers the horizontal and vertical views, rubber
  band, menu helper objects, and their actions. Manager objects are still
  deleted early during `GuiManager` teardown to stop callbacks before Qt
  destroys child views.
- The unused `GuiManager` singleton, archive-mode mirror, archive start time,
  legacy timer fields, dead clear method, and unused selected-field getters
  were removed.
- Open and parameter-file Save remain visible but disabled. They are reserved
  for future parameter-file loading and serialization. Save-Image remains
  enabled in interactive mode.
- Routine successful horizontal-read messages are controlled by the debug
  setting; error diagnostics remain unconditional.
- Variable-length legacy contour and layer-field configuration is bounded
  before indexing fixed CIDD-compatible arrays, preventing startup memory
  corruption from oversized parameter lists.
- Field rendering no longer has a dynamic-contour branch; the supported field
  choices are polygons, filled contours, or no fill plus optional line
  contours.
- `docs/lrose-process-data-management.md` summarizes the LROSE process and
  data layers and records the intended approach for a Lucid realtime
  simulator.
- `docs/bug_status.txt` now records all seven concrete review defects as
  resolved. `docs/cleanup_status.txt` distinguishes completed structural work
  from lower-priority cleanup best deferred until simulator testing.

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
- Preserve the user's selected vertical route as the request definition, but
  display server-returned geometry when an MDV service adjusts the route.
  Response geometry must not cause a recursive primary-data request.
- Missing/bad grid values remain holes unless explicit mapping says otherwise.
  Invalid contour topology falls back to polygon rendering.
- Automatic image work is serialized through the GUI display state; fixed
  delays are not used to guess when asynchronous requests are complete.

## Outstanding work

1. Build the realtime/archive simulation environment described in
   `docs/lrose-process-data-management.md`. This is the next development
   focus and should begin in a new Codex thread.
2. Select the replay boundary and source datasets. Prefer immutable archive
   input and a separate writable realtime tree populated through normal LROSE
   file/latest-data or FMQ interfaces; Lucid should remain an ordinary client.
3. Define the simulation project layout, process instances, `proc_list`,
   `data_list`, logs, servers, replay clock/rate, looping policy, and late-data
   thresholds. Keep it isolated from operational data and shared-memory keys.
4. Create a repeatable image-generation test matrix covering success, timeout,
   stale completion, recovery, and scheduler coalescing.
5. Verify automatic PNG transparency, geo XML, `_latest_data_info`, day
   directories, and DataMapper behavior with downstream consumers.
6. Exercise automatic combinations of multiple fields, heights, zooms, and
   vertical routes, including filename uniqueness and state restoration.
7. Continue visual comparison of symbolic products with CIDD and decide the
   desired zero-chunk snapshot policy.
8. Exercise native polar RHIs against CIDD, including full-range returned
   routes, terrain, click values, movies, and Cartesian fallback when no
   measured RHI matches.
9. Consolidate `MdvReader`'s remaining public legacy validity flags with its
   private request/result state.
10. Continue removing demonstrably unused `GlobalData::layers` state and
   simplify terrain/wind request queuing where `MdvReader` now provides the
   latest-request policy.
11. Continue vertical-window presentation cleanup and regression testing.
12. Remove Route Winds completely when the current commented history is no
   longer needed.
13. Work through `docs/parameter_status.txt` incrementally as affected features
   are cleaned up.
14. There are no automated GUI tests yet; current validation is interactive.

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
3. Request a measured polar RHI and compare it with
   `RHI_polar_rendering.png` and CIDD. Verify polygon geometry, km axes, RHI
   title azimuth, cursor/click values, the full-range line in `HorizView`, and
   fallback to a Cartesian section when no measured RHI is suitable.
4. Exercise horizontal and vertical cursor values and click reports.
5. Toggle Products globally and individually, then compare METAR placement and
   timing with CIDD.
6. Build and play horizontal-only and horizontal-plus-vertical movies. Verify
   progress colors, displayed-frame gray, Stop, field selection after Stop,
   dwell, endpoint delay, and sweep playback.
7. Test time-canvas selection, zoom, pan, automatic extension, right-drag plus
   Accept, and interval editing.
8. Exercise Reload with an open vertical section and enabled Products.
9. Test interactive Save-Image and Output Loop without overwriting prior files.
10. Run automatic create-once, archive schedule, and realtime schedule modes;
   inspect image names, summaries, optional sidecars, and exit status.
11. Press Ctrl-C during active MDV/SPDB/image requests and verify orderly
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
Read AGENTS.md, docs/codex-handoff.md,
docs/lrose-process-data-management.md, docs/bug_status.txt, and
docs/cleanup_status.txt. Review the current worktree and recent Lucid history,
then help design and build the isolated realtime/archive simulator described
in the handoff. Do not modify Lucid display code unless simulator testing
identifies a concrete need.
```
