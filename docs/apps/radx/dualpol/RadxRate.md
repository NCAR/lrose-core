## RadxRate application

### Purpose

RadxRate estimates precip rate from dual-pol moments in polar-coordinate radar data.

In order to compute rate, RadxRate also computes KDP, the attenuation correction, and PID.

You may optionally use the attenuation-corrected fields for computing precip rate.

### Usage

To see the usage, run:

```
  RadxRate -h
```

on the command line.

The usage is available [here](./RadxRateUsage.md).

### Generating the main parameter file

RadxRate reads in a main parameter file, which provides overall control to the mode of operation, reading in the data, and writing out the results.

To generate a default parameter file, run the following command:

```
  RadxRate -print_params > RadxRateParams.test
```

This will generate the parameter file ```RadxRateParams.test```.

Here is an [example](./RadxRateParams.md) main parameter file.

In that file, you will find the following parameter:

```
///////////// RATE_params_file_path /////////////////
//
// Path for parameters for computing RATE.
//
// If set to use-defaults, no parameter file will be read in, and the 
//   default parameters will be used.
//
//
// Type: string
//

RATE_params_file_path = "use-defaults";

```

If you leave this set to ```use-defaults```, the default settings will be used for computing precip rate.

If you set this to a path containing the rate-specific parameters, this will override the default parameters.

Also you will need to set a parameter file path for KDP computations, and for PID computations.

### Generating the rate-specific parameter file

To generate a rate-specific parameter, run the following command:

```
  RadxRate -print_params_rate > RateParams.test
```

This will generate the parameter file ```RateParams.test```.

Here is an [example](./RateParams.md) rate-specific parameter file.

### Interest maps and thresholds for PID Fuzzy Logic

See [RadxPid](./RadxPid.md) for details on dealing with the fuzzy logic parameters for PID.

### Updating the parameter files as the RadxRate app changes

Sometimes when the application is updated, the parameters will change or be augmented.

You can update the parameter files using the instructions below.

To update the main parameter file, run commands similar to the following:

```
  RadxRate -params RadxRate.test -print_params > tempfile
  mv tempfile RadxRate.test
```

Make sure you create a temporary file first, and then move the file into place.
If you try to perform this in one step, you will destroy your original parameter file.

To update the rate-specific parameter file, run commands similar to the following:

```
  RadxRate -params_rate RateParams.test -print_params_rate > tempfile
  mv tempfile RateParams.test
```

To update the PID-specific parameter file, run commands similar to the following:

```
  RadxRate -params_pid PidParams.test -print_params_pid > tempfile
  mv tempfile PidParams.test
```

To update the KDP-specific parameter file, run commands similar to the following:

```
  RadxRate -params_kdp KdpParams.test -print_params_kdp > tempfile
  mv tempfile KdpParams.test
```

### Running RadxRate by specifying files on the command line

The following is an example of running RadxRate, specifying which files you want to process:

```
RadxRate -params RadxRate.test -debug -f /tmp/cfradial/test/20150626/cfrad.20150626_00*.nc
```

### Running RadxRate by specifying the start and end times

The following command would produce the same result as that above:

```
RadxRate -params RadxRate.test -debug -start "2015 06 26 00 00 00" -end "2015 06 26 01 00 00"
```

In the latter case, you need to ensure that the ```input_dir``` parameter is correctly set in RadxRate.test, so that the archive files can be found.
