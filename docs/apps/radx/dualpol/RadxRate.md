## RadxRate application

### Purpose

RadxRate computes Precip-Rate from dual-pol moments in polar-coordinate radar data.

In order to compute Precip-Rate, RadxRate also computes KDP, the attenuation correction in Z and ZDR and PID.

Optionally, you may use the attenuation-corrected fields for computing Precip-Rate.

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

Here is an [example](./RadxRateParams.md) RadxRate main parameter file.

In that file, you will find the following parameter:

```
///////////// PRECIP_params_file_path /////////////////
//
// Path for parameters for computing PRECIP.
//
// If set to use-defaults, no parameter file will be read in, and the 
//   default parameters will be used.
//
//
// Type: string
//

PRECIP_params_file_path = "use-defaults";

```

If you leave this set to ```use-defaults```, the default settings will be used for computing Precip-Rate.

If you set this to a path containing the Precip-Rate-specific parameters, this will override the default parameters.

Also you will need to set a parameter file path for KDP computations. For more details, see [RadxKdp](./RadxKdp.md).

And you will need to set a parameter file path for PID computations. For more details, see [RadxPid](./RadxPid.md).

### Generating the Precip-Rate-specific parameter file

To generate a Precip-Rate-specific parameter, run the following command:

```
  RadxRate -print_params_precip > PrecipRateParams.test
```

This will generate the parameter file ```PrecipRateParams.test```.

Here is an [example](./PrecipRateParams.md) Precip-Rate-specific parameter file.

### Interest maps and thresholds for Precip-Rate Fuzzy Logic

In the [PidParams](./PidParams.md) file, you will find the following parameter:

```
///////////// PID_thresholds_file_path ////////////////
//
// File path for fuzzy logic thresholds for PID.
//
// This file contains the thresholds and weights for computing particle 
//   ID.
//
//
// Type: string
//
PID_thresholds_file_path = "./pid_thresholds.nexrad";
```

Set this to a file suitable for the radar transmit mode and wavelength:

| Wavelength                | Transmit mode | thresholds_file_example |
| -------------             | ------------- | ----------------------- |
| S-band                    | Simultaneous  | [pid_thresholds.sband.shv](pid_thresholds.sband.shv.md) |
| S-band                    | Alternating   | [pid_thresholds.sband.alt](pid_thresholds.sband.alt.md) |
| C-band                    | Simultaneous  | [pid_thresholds.cband.shv](pid_thresholds.cband.shv.md) |
| X-band                    | Simultaneous  | [pid_thresholds.xband.shv](pid_thresholds.xband.shv.md) |

### Updating the parameter files as the RadxRate app changes

Sometimes when the application is updated, the parameters will change or be augmented.

You can update the parameter files using the instructions below.

To update the main parameter file, run commands similar to the following:

```
  RadxRate -params RadxRate.test -print_params > tempfile
  mv tempfile RadxRate.test
```

You need to ensure you create a temporary file first, and then move the file into place.
If you try to perform this in one step, you will destroy your original parameter file.

To update the Precip-Rate-specific parameter file, run commands similar to the following:

```
  RadxRate -params_precip PrecipRateParams.test -print_params_precip > tempfile
  mv tempfile PrecipRateParams.test
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

In the latter case, you need to ensure that the ```input_dir``` parameter is correctly set in RadxRate.test.





