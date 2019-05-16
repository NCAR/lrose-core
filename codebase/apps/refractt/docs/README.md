# NOTES ON REFRACT INSTALLATION

Thanks to Dave Albo.

## Building

The LROSE source code build uses standard libraries plus:

```
  libs/Refract
```

and the apps are here:

```
  apps/refract
```

## Apps

The important apps are:

* Refract
* RefractCalib

Other apps that get built are probably not needed for real time.
The build is standard in lrose-core.

## Configuration

Configuration examples (e.g. for SPOL) may be found at:

```
  ../example_configs
```

with subdirs:

```
  alg                  algorithm scripts and parameters
  display              CIDD scripts and parameters
  envs                 see below
```

### envs

Because we were testing this on several different radars, we created environment variables for each, so that the param and script files could be used for any radar.  Each script sources one of the files in this directory.

The most recent of these files is env_set.realtime, which we've been using to test real time data recently.  Within that file the values you would want to modify for sure are:

  BIN_DIR
  RAP_DATA_DIR
  ALG_HOST


## RefractCalib

This is run once and exit on a range of times to produce calibrations, using

```
   alg/bin/start_RefractCalib.realtime
```
 
which you edit to set the range of times.

It does support a different calibration for day and night, but it doesn't do that now because env_set.realtime sets the same file for day and for night.

### Refract

This is set up to run in real time using

```
  alg/bin/start_Refract.realtime
```

with commented out archive mode command lines.

