# ENVIRONMENT VARIABLES USED BY THE LROSE APPS and LIBRARIES

This document describes the most commonly-used environment variables which control the functionality of the LROSE libraries and applications.

NOTE: When a variable may be set to "true" or "false", this is case-insensitive.

## DS SERVER SYSTEM

The DS server system is the server layer for DIDSS (Data ingest
and distributed server system).

| Variable | Default value | Description | Code library |
| -------- |:-------------:|:-----------:|:------------:|
| RAP_DATA_DIR | undefined | If set, URLs point to locations relative to RAP_DATA_DIR, unless the file part of the URL starts with / or ., in which case the absolute or relative paths are used. | didss dsserver |
  | DATA_DIR  | undefined  | Alternartive to RAP_DATA_DIR. If RAP_DATA_DIR is defined, it is used. If not, DATA_DIR is used if defined.  | didss dsserver  | 
  | DATA_MAPPER_ACTIVE  | true  | Set to 'false' to turn off automatic access from clients to DataMapper.  | dsserver  | 
  | DATA_MAPPER_DEBUG  | undefined  | Set to 'true' to see debug messages from DATA_MAPPER access requests.  | dsserver  | 
  | DS_COMM_TIMEOUT_MSECS  | 30000  | This is the timeout, in milli-secs, used in communications between clients and servers. The default value of 30 secs (30000 msecs) was chosen so that programs would not block too long and fail to register with procmap, thereby causing them to be restarted. You may need to inccrease the value over slow lines. If you increase the value, you may be wise to run without the restart layer.  | dsserver  | 
  | DS_PING_TIMEOUT_MSECS  | 10000  | This is the timeout, in milli-secs, used when pinging a server to see if it is alive. It is used by clients to check whether a server is up, before making the data request. If the server is down, the client makes a request to the server manager (DsServerMgr) to start the server. The DsServerMgr starts the server and then uses a ping to make sure it is alive, before returning a successful flag to the client.  | dsserver  | 
  | DS_BASE_PORT  | 5430  | All ports in the DS server system are calculated relative to this port. If you need to run two server systems on a single host, for eaxmple for  two different users, you can override the default. To be safe, pick a number at least 1000 above the default, because the server ports are in the immediate range above the base.  | dsserver  | 
  | SPDB_ALLOW_NO_LOCK  | undefined  | If set to "true", the Spdb library will not require a lock on the data base files for reads. Locks are still required for writes. This may be used if you are reading data across a cross-mount for which file locking is not implemented. However, the better strategy is to contact a server which has local access to the data.  | Spdb  | 
  | CLOSE_SOCKET_IN_CHILD  | undefined  | If set to "true", the servers will close the listening socket in child processes. This should not be necessary, but was for Linux kernels 2.0.x, and possibly earlier kernels. It is not necessary on Solaris or Linux potato or kernels 2.4 and later. Only set this variable if your system is getting too many open files. You can check this with the 'lsof' command.  | dsserver  | 
  | MAX_FORECAST_LEAD_DAYS  | 10  | When seraching a directory for data stored in forecast file name format (yyyymmdd/g_hhmmss/f_ssssssss.ext) the library needs to know how far back in time to look for forecast data which may be valid at the current time. It looks back a maximum of this number of days. | didss |
| MDV_WRITE_FORMAT | FORMAT_NCF | Format for writing MDV files. Options are:
* FORMAT_NCF (NetCDF CF, the default)
* FORMAT_MDV (legacy 32-bit format)
* FORMAT_XML (XML header and data buffer)
| Mdvx |

