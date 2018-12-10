## RadxKdp application

### Purpose

RadxKdp computes KDP from PHIDP in polar-coordinate radar data.
It also computes the attenuation correction in Z and ZDR.

### Usage

To see the usage, run:

```
  RadxKdp -h
```

on the command line.

The usage is available [here](./RadxKdpUsage.md).

### Generating the main parameter file

RadxKdp reads in a main parameter file, which provides overall control to the mode of operation, reading in the data, and writing out the results.

To generate a default parameter file, run the following command:

```
  RadxKdp -print_params > RadxKdpParams.test
```

This will generate the parameter file ```RadxKdpParams.test```.

Here is an [example](./RadxKdpParams.md) RadxKdp main parameter file.

In that file, you will find the following parameter:

```
///////////// KDP_params_file_path ////////////////////
//
// Path for parameters for KDP computations.
//
// If set to use-defaults, no parameter file will be read in, and the 
//   default parameters will be used.
//
//
// Type: string
//

KDP_params_file_path = "use-defaults";
```

If you leave this set to ```use-defaults```, the default settings will be used for computing KDP and attenuation.

If you set this to a path containing the KDP-specific parameters, this will override the default parameters.

### Generating the KDP-specific parameter file

To generate a KDP-specific parameter, run the following command:

```
  RadxKdp -print_params_kdp > KdpParams.test
```

This will generate the parameter file ```RadxKdp.test```.

Here is an [example](./KdpParams.md) KDP-specific parameter file.

### Updating the parameter files as the RadxKdp app changes

Sometimes when the application is updated, the parameters will change or be augmented.

You can update the parameter files using the instructions below.

To update the main parameter file, run commands similar to the following:

```
  RadxKdp -params RadxKdp.test -print_params > tempfile
  mv tempfile RadxKdp.test
```

You need to ensure you create a temporary file first, and then move the file into place.
If you try to perform this in one step, you will destroy your original parameter file.

To update the KDP-specific parameter file, run commands similar to the following:

```
  RadxKdp -params_kdp KdpParams.test -print_params_kdp > tempfile
  mv tempfile KdpParams.test
```

### Running RadxKdp by specifying files on the command line

The following is an example of running RadxKdp, specifying which files you want to process:

```
RadxKdp -params RadxKdp.test -debug -f /tmp/cfradial/test/20150626/cfrad.20150626_00*.nc
```

### Running RadxKdp by specifying the start and end times

The following command would produce the same result as that above:

```
RadxKdp -params RadxKdp.test -debug -start "2015 06 26 00 00 00" -end "2015 06 26 01 00 00"
```

In the latter case, you need to ensure that the ```input_dir``` parameter is correctly set in RadxKdp.test.





