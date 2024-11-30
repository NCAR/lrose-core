# ENVIRONMENT VARIABLES USED BY THE LROSE APPS and LIBRARIES

This document describes the most commonly-used environment variables which control the functionality of the LROSE libraries and applications.

NOTE: When a variable may be set to "true" or "false", this is case-insensitive.

## Data system root

### DATA_DIR

Default: none
Action: Top-level data directory for a project. If relative paths are specified in a parameter file, the path is relative to $DATA_DIR.
Libraries: didss, dsserver

## MDV file writes and reads

### MDV_WRITE_FORMAT

Default: FORMAT_NCF
Action: Format for writing MDV files.

Options are:
* FORMAT_NCF (NetCDF CF, the default)
* FORMAT_MDV (legacy 32-bit format)
* FORMAT_XML (XML header and data buffer)

Library: Mdv

### MDV_WRITE_USING_EXTENDED_PATHS

Default: FALSE
Action: Causes MDV apps to add the date to the file name.

Options are:
* TRUE
* FALSE

If TRUE

```  yyyymmdd/hhmmss.mdv```

becomes

```  yyyymmdd/yyyymmdd_hhmmss.mdv```

Library: Mdv
  
### MDV_WRITE_ADD_YEAR_SUBDIR

Default: FALSE
Action: Causes MDV apps to prepend the date to the output path.

Options are:
* TRUE
* FALSE

If TRUE

```  yyyymmdd/hhmmss.mdv```

becomes

```  yyyy/yyyymmdd/hhmmss.mdv```

Library: Mdv
  
### MAX_FORECAST_LEAD_DAYS

Default: 10
Action: When seraching a directory for data stored in forecast file name format (yyyymmdd/g_hhmmss/f_ssssssss.ext) the library needs to know how far back in time to look for forecast data which may be valid at the current time. It looks back a maximum of this number of days.
Library: dids

### MDV_GRID_TOLERANCE

Default: 0.0000001
Action: Tolerance used when checking whether two grids are equal.
Library: Mdv

## Server communication

### DS_COMM_TIMEOUT_MSECS

Default: 30000
Action: This is the timeout, in milli-secs, used in communications between clients and servers. The default value of 30 secs (30000 msecs) was chosen so that programs would not block too long and fail to register with procmap, thereby causing them to be restarted. You may need to inccrease the value over slow lines. If you increase the value, you may be wise to run without the restart layer.
Library: dsserver

### DS_PING_TIMEOUT_MSECS

Default: 10000
Action: This is the timeout, in milli-secs, used when pinging a server to see if it is alive. It is used by clients to check whether a server is up, before making the data request. If the server is down, the client makes a request to the server manager (DsServerMgr) to start the server. The DsServerMgr starts the server and then uses a ping to make sure it is alive, before returning a successful flag to the client.
Library: dsserver

### DS_BASE_PORT

Default: 5430
Action: All ports in the DS server system are calculated relative to this port. If you need to run two server systems on a single host, for eaxmple for  two different users, you can override the default. To be safe, pick a number at least 1000 above the default, because the server ports are in the immediate range above the base.
Library: dsserver

### FCOPY_SERVER_ALLOW_NO_LOCK

Options: TRUE, FALSE
Default: FALSE
Action: Allows DsFCopyServer to not lock files on write. If TRUE, lock files will not be required on write. Normally locking should be used. Only set this to allow writing across a cross-mount, which is not recommended anyway. But if you have to ....
Library: dsserver

### FCOPY_SERVER_TMP_DIR

Default: none
Action: Specify tmp dir for DsFCopyServer on write. Normally DsFCopyServer writes tmp files in the subdirectory being used. If this env var is set, the tmp file will be written to the specified directory. WARNING - make sure the tmp dir and the data dir are on the same partition.
Library: dsserver

### SPDB_ALLOW_NO_LOCK

Default: undefined
Action: If set to "true", the Spdb library will not require a lock on the data base files for reads. Locks are still required for writes. This may be used if you are reading data across a cross-mount for which file locking is not implemented. However, the better strategy is to contact a server which has local access to the data.
Library: Spdb

### CLOSE_SOCKET_IN_CHILD

Default: undefined
Action: If set to "true", the servers will close the listening socket in child processes. This should not be necessary, but was for Linux kernels 2.0.x, and possibly earlier kernels. It is not necessary on Solaris or Linux potato or kernels 2.4 and later. Only set this variable if your system is getting too many open files. You can check this with the 'lsof' command.
Library: dsserver

## LATEST DATA INFO FILES

The _latest_data_info files are small files which are written to a data directory to indicate the time of the latest data written to that directory.

The following files will be written:

* _latest_data_info - legacy, original text file. Kept for backward compatibility.
* _latest_data_info.xml - XML description of the latest data.
* _latest_data_info.stat - status file for a small queue (FMQ) of entries. A queue is needed to ensure that entries are not missed when a number of files are written in rapid succession.
* _latest_data_info.buf - buffer file for a small queue (FMQ) of entries. A queue is needed to ensure that entries are not missed when a number of files are written in rapid succession.
* _latest_data_info.lock - lock file to prevent simultaneous writes of the queue.

### LDATA_FMQ_ACTIVE

Options: true, false
Default: true
Action: By default the FMQ is active. If this is set to "false", the FMQ (file message queue) option will be deactivated. If active, the apps write both a text file and an FMQ containing the latest data info. The FMQ option is useful for cases in which data arrives rapidly and the client may miss data information while polling. Since the FMQ is a queue, the client can read the entries without risk of missing an entry.
Library: didss

### LDATA_FMQ_NSLOTS

Type: integer
Default: 2500
Action: The number of slots in the FMQ, if it is active. The queue wraps once this number of entries is reached. Increase if data arrives very rapidly and the clients may need more time to read the queue.
Library: didss

### LDATA_NO_WRITE

Type: boolean string, true/false
Default: false
Action: If set to "true", no _latest_data_info files will be written. This is sometimes useful in archive mode when you do not want to overwrite the realtime files.
Libraries: toolsa didss dsserver

## THE PROCESS MAPPER

The process management layer relies on ```procmap```, the process mapper. Programs register a hearbeat with procmap, typically once per minute. Normally one procmap runs per host. The auto_restart script checks the procmap list against an expected list of processes, and restarts those which are not running.

### PROCMAP_DIR

Default: not defined
Action: If defined, it should be a directory path. Each time a process registers with procmap, the status string will be written to a file in this directory. This is useful for debugging if a system crashes badly and procmap no longer runs. It is seldom used.
Libraries: toolsa

### PROCMAP_VERBOSE

Default: not defined
Action: If set to "true", verbose debugging messages are printed out during communication with procmap.
Library: toolsa

### DATA MAPPER

The ```DataMapper``` is a small server that keeps track of writes to the data system. Applications register with DataMapper each time data is written to disk.

### DATA_MAPPER_ACTIVE

Default: true
Action: Set to 'false' to prevent apps from registering with the DataMapper.
Library: dsserver

### DATAMAP_VERBOSE

Default: not defined
Action: If set to "true", verbose debugging messages are printed out during communication with DataMapper.
Library: toolsa

### DATA_MAPPER_DEBUG

Default: not defined
Action: Set to 'true' to see debug messages from DATA_MAPPER access requests.
Library: dsserver

