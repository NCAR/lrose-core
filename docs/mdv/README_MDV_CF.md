# NOTE: Cartesian data MDV files - NetCDF default

Please note that in this version of LROSE (and TITAN), any Cartesian files will be written out 
by default using the NetCDF version of the MDV format. Radx2Grid is an example of an application that writes MDV files.

Previously the default was to write the binary version of MDV.

Both formats should worked interchangeably within LROSE and TITAN. But for non-LROSE applications this can cause confusion.

The output format for Cartesian MDV files can be controlled via the MDV_WRITE_FORMAT environment variable. To force writing of binary MDV files, use on of the following:

```
  setenv MDV_WRITE_FORMAT FORMAT_MDV (csh or tcsh)
  export MDV_WRITE_FORMAT=FORMAT_MDV (sh or bash)
```

and to force the usage of NetCDF, the default, use:

 ```
  setenv MDV_WRITE_FORMAT FORMAT_NCF (csh or tcsh)
  export MDV_WRITE_FORMAT=FORMAT_NCF (sh or bash)
```


