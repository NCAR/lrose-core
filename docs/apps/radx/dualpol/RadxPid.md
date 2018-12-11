## RadxPid application

### Purpose

RadxPid computes PID from dual-pol moments in polar-coordinate radar data.

In order to compute PID, RadxPid also computes KDP and the attenuation correction in Z and ZDR.

Optionally, you may use the attenuation-corrected fields for computing PID.

### Usage

To see the usage, run:

```
  RadxPid -h
```

on the command line.

The usage is available [here](./RadxPidUsage.md).

### Generating the main parameter file

RadxPid reads in a main parameter file, which provides overall control to the mode of operation, reading in the data, and writing out the results.

To generate a default parameter file, run the following command:

```
  RadxPid -print_params > RadxPidParams.test
```

This will generate the parameter file ```RadxPidParams.test```.

Here is an [example](./RadxPidParams.md) RadxPid main parameter file.

In that file, you will find the following parameter:

```
///////////// PID_params_file_path ////////////////////
//
// Path for parameters for PID computations.
//
// If set to use-defaults, no parameter file will be read in, and the 
//   default parameters will be used.
//
//
// Type: string
//

PID_params_file_path = "use-defaults";
```

If you leave this set to ```use-defaults```, the default settings will be used for computing PID.

If you set this to a path containing the PID-specific parameters, this will override the default parameters.

Similarly you will need to set a parameter file path for KDP computations. For more details, see [RadxKdp](./RadxKdp.md).

### Generating the PID-specific parameter file

To generate a PID-specific parameter, run the following command:

```
  RadxPid -print_params_pid > PidParams.test
```

This will generate the parameter file ```RadxPid.test```.

Here is an [example](./PidParams.md) PID-specific parameter file.

### Interest maps and thresholds for PID Fuzzy Logic

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

### Updating the parameter files as the RadxPid app changes

Sometimes when the application is updated, the parameters will change or be augmented.

You can update the parameter files using the instructions below.

To update the main parameter file, run commands similar to the following:

```
  RadxPid -params RadxPid.test -print_params > tempfile
  mv tempfile RadxPid.test
```

You need to ensure you create a temporary file first, and then move the file into place.
If you try to perform this in one step, you will destroy your original parameter file.

To update the PID-specific parameter file, run commands similar to the following:

```
  RadxPid -params_pid PidParams.test -print_params_pid > tempfile
  mv tempfile PidParams.test
```

To update the KDP-specific parameter file, run commands similar to the following:

```
  RadxPid -params_kdp KdpParams.test -print_params_kdp > tempfile
  mv tempfile KdpParams.test
```

### Running RadxPid by specifying files on the command line

The following is an example of running RadxPid, specifying which files you want to process:

```
RadxPid -params RadxPid.test -debug -f /tmp/cfradial/test/20150626/cfrad.20150626_00*.nc
```

### Running RadxPid by specifying the start and end times

The following command would produce the same result as that above:

```
RadxPid -params RadxPid.test -debug -start "2015 06 26 00 00 00" -end "2015 06 26 01 00 00"
```

In the latter case, you need to ensure that the ```input_dir``` parameter is correctly set in ```RadxPid.test```, so that the archive filed can be found.
