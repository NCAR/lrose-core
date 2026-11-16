# LROSE Process and Data Management

LROSE realtime projects use two cooperating management layers:

- The **process layer** defines which application instances should be running,
  monitors their heartbeats, restarts failed processes, and captures logs.
- The **data layer** moves or publishes data, records the latest dataset state,
  and provides standard services through which downstream applications find
  data.

This arrangement lets a realtime simulator replace the live ingest at the
edge of the system while leaving processing, servers, displays, monitoring,
and recovery behavior close to operational use.

## Project and data roots

Two environment variables define the main namespaces:

- `PROJ_DIR` is the version-controlled project hierarchy, conventionally
  exposed as `~/projDir`.
- `DATA_DIR` is the writable data hierarchy. It is commonly on a separate
  disk, with `$PROJ_DIR/data` and `$PROJ_DIR/logs` implemented as symbolic
  links into it.

A typical project is organized by operational role:

```text
$PROJ_DIR/
  control/
    proc_list
    data_list
    crontab
  system/
    dotfiles/
    params/
    scripts/
  ingest/{params,scripts}/
  algorithm/{params,scripts}/
  display/{params,scripts,maps,color_scales}/
  data -> $DATA_DIR
  logs -> $DATA_DIR/logs
```

Shell startup files are normally links to the project versions, for example
`.bashrc -> $PROJ_DIR/system/dotfiles/bashrc`. This makes the environment,
aliases, executable path, project root, data root, and log locations part of
the deployed configuration.

Small projects can use a single set of control files. A multi-host project
normally stores role-specific files such as `proc_list.mgen`,
`data_list.mgen`, and `crontab.mgen`. A host configuration script selects the
role by linking the generic control names to the appropriate files and by
installing the correct data and log links. SPOL's
`projDir/system/scripts/configureHost.py` is an example of this pattern.

## Process supervision

### Process identity and heartbeat

Every managed application instance has a `(process name, instance)` identity.
LROSE applications periodically register that identity, their PID, and a
status string with `procmap`. The registration is a heartbeat as well as a
diagnostic report. A process that exists at the operating-system level but no
longer registers is treated as hung.

`procmap` is therefore the observed process state. Useful checks include:

```text
pcheck                         # compare procmap with the expected process list
ppm                            # detailed procmap display
print_procmap -hb -up -status  # command behind the usual ppm alias
```

### Desired state: `proc_list`

`$PROJ_DIR/control/proc_list` is the desired process state. Each active row
specifies:

```text
process  instance  start_method  stop_method  localhost
```

The instance distinguishes multiple configurations of the same executable.
The start method can name a custom script or use `start_inst(role)` when the
standard `application.instance` parameter and script conventions are enough.
The stop method can similarly be a custom script or `snuff_inst`.

Start scripts normally:

1. Change to the role's parameter directory.
2. Use `running` to avoid launching a duplicate instance.
3. Start the application with its parameter file.
4. Pipe stdout and stderr through `LogFilter`.

Custom scripts are appropriate when an application needs extra environment
variables, setup, or command-line options.

### Automatic recovery and scheduled maintenance

`auto_restart` repeatedly compares `proc_list` with registrations in
`procmap`. A missing or stale process is killed in case it is hung and then
started again. The project crontab ensures that `procmap` and `auto_restart`
themselves remain running and schedules housekeeping such as log-link updates.

The normal system lifecycle is:

- `start_all`: start `procmap`, start the entries in `proc_list`, start
  `auto_restart`, install the project crontab, and initialize log links.
- `stop_all`: remove the crontab, stop automatic restart, stop managed
  processes and `procmap`, and clean up shared-memory resources.

When diagnosing startup, run a failing start script directly before relying
on `auto_restart`; parameter paths, environment setup, and typographical
errors are then visible immediately.

## Logs and retention

`LogFilter` receives application stdout and stderr and writes date-partitioned
logs, normally named by process and instance. Common trees are:

```text
$DATA_DIR/logs/errors
$DATA_DIR/logs/restart
$DATA_DIR/logs/distrib
```

Convenience links point to today's and yesterday's dated directories.
`Janitor` applies configured retention or compression rules so continuously
arriving data and logs do not fill the disk. Janitor configuration is
destructive by design and should first be validated in report mode and against
an isolated simulation data tree.

## Data transport and publication

LROSE uses two main data-flow styles.

### Fast Message Queues

An FMQ is a circular message queue with one writer and multiple readers. It
can be file-backed or shared-memory-backed. The queue has a status/index
buffer, a message buffer, and a lock that enforces the single-writer rule.

Queue sizing must account for message size, arrival rate, and the longest
expected reader delay. Realtime queues normally wrap rather than blocking: a
reader that falls too far behind loses old data but the live pipeline keeps
moving. Blocking behavior is suitable only for controlled archive processing
with a single reader. `FmqMon` is the standard tool for checking message count
and throughput.

`DsFmq` and `DsFmqServer` extend this model to remote reads and writes between
hosts.

### Files and `latest_data_info`

Many stages write complete files such as CfRadial, MDV, CF-NetCDF, SPDB, XML,
or native binary products. The writer then publishes a `latest_data_info`
record in the same dataset directory. Current implementations maintain an XML
summary and a small FMQ of update messages, allowing a reader that is briefly
busy to recover notifications rather than observing only the final filename.

Writers generally use `DsLdataInfo`; it writes the latest-data state and also
registers the dataset with `DataMapper`. Readers poll the latest-data queue and
open the referenced file. This is the normal trigger boundary between
file-oriented processing stages.

## Dataset inventory and health

`DataMapper` is to datasets what `procmap` is to processes. Writers register
activity with it, allowing operators to see the latest data time, last
registration time, file count, byte count, type, and directory. Common checks
are:

```text
pdm                              # summarized DataMapper display
PrintDataMap -all -relt -lreg    # command behind the usual pdm alias
dcheck                           # compare DataMapper with control/data_list
```

`$PROJ_DIR/control/data_list` describes datasets required on a host and their
late and very-late thresholds. `Scout` complements writer registration by
walking configured data trees and reporting file inventory. Together,
`DataMapper`, `Scout`, and `data_list` distinguish several failure modes: an
absent process, a running process that is not producing data, stale input, or
a data directory that exists but is no longer being updated.

`DsServerMgr` manages data-serving processes used by client URLs. Depending on
the data type, clients may reach MDV, SPDB, or other datasets through the
corresponding server rather than reading the directory directly.

## Multi-host data movement

FMQ streams can be relayed between hosts with the DsFmq service layer. File
trees can be copied with `DsFileDist` to `DsFCopyServer`; the destination
recreates the files, latest-data notifications, and DataMapper registrations.
This preserves the normal downstream triggering behavior after distribution.

In a multi-host project, each role should list only the processes and datasets
it owns or requires. Role-specific control files make that contract explicit
and allow the same project repository to configure ingest, processing,
display, archive, and server hosts differently.

## Implications for a realtime simulator

A useful simulator should enter the pipeline at the same interface as live
ingest and should exercise the normal management layer:

1. Keep immutable archive input separate from the simulator's writable
   realtime output tree.
2. Select the replay boundary: copy files and publish `latest_data_info`, or
   replay messages into the appropriate FMQ.
3. Give each simulator a unique process instance, parameters, start method,
   and `proc_list` entry so `procmap` and `auto_restart` supervise it normally.
4. Use a dedicated `$DATA_DIR` or host role for testing. Do not mix simulated
   products with operational data or reuse shared-memory keys from another
   running system.
5. Configure replay rate, time handling, and looping explicitly. Downstream
   applications should consume the simulator output through their normal
   realtime URLs and triggering mechanisms.
6. Add expected outputs to `data_list` with realistic late thresholds.
7. Verify both control planes: use `pcheck`/`ppm` for process health and
   `pdm`/`dcheck` plus `FmqMon` where applicable for data progress.
8. Route logs through `LogFilter`, but use conservative Janitor settings until
   the test data paths have been checked carefully.

For Lucid testing, this means Lucid should remain an ordinary realtime client.
The simulator and upstream processing should populate the MDV and SPDB
datasets, latest-data state, and servers that Lucid would use operationally.
That tests request timing, missing and late data, movie construction, symbolic
products, winds, terrain, and automatic image generation without adding
simulation-specific behavior to Lucid itself.

## Reference material

- [LROSE real-time operations](https://github.com/NCAR/lrose-docs/blob/master/lrose-core/lrose-realtime.md)
- [LROSE data flow and triggering](https://github.com/NCAR/lrose-docs/blob/master/lrose-core/lrose-data-flow.md)
- Local simple-project examples: `~/git/lrose-titan/projects`
- Local multi-host example: `~/git/spol-configuration/`
- Source prompt for this summary: `docs/process_manager.prompt`
