## RadxKdp application

### Purpose

RadxKdp computes KDP from PHIDP in polar-coordinate radar data.
It also computes the attenuation correction in Z and ZDR.

### Usage

```
Usage: RadxKdp [args as below]
Compute KDP from radar moments in polar coords
Options:

  [ -h ] produce this list.

  [ -d, -debug ] print debug messages

  [ -end "yyyy mm dd hh mm ss"] end time
           Sets mode to ARCHIVE

  [ -f, -paths ? ] set file paths
           Sets mode to FILELIST

  [ -instance ?] specify the instance
    app will register with procmap using this instance
    forces REALTIME mode

  [ -outdir ? ] set output directory

  [ -start "yyyy mm dd hh mm ss"] start time
           Sets mode to ARCHIVE

  [ -v, -verbose ] print verbose debug messages

  [ -vv, -extra ] print extra verbose debug messages


TDRP args: [options as below]
   [ -params/--params path ] specify params file path
   [ -check_params/--check_params] check which params are not set
   [ -print_params/--print_params [mode]] print parameters
     using following modes, default mode is 'norm'
       short:   main comments only, no help or descr
                structs and arrays on a single line
       norm:    short + descriptions and help
       long:    norm  + arrays and structs expanded
       verbose: long  + private params included
       short_expand:   short with env vars expanded
       norm_expand:    norm with env vars expanded
       long_expand:    long with env vars expanded
       verbose_expand: verbose with env vars expanded
   [ -tdrp_debug] debugging prints for tdrp
   [ -tdrp_usage] print this usage

KDP-specific parameters:
   [ -params_kdp ] specify KDP params file path
     otherwise it is set in the main params file
   [ -print_params_kdp [mode]] print KDP params
     see modes from -print_params above
```

### Generating the main parameter file

RadxKdp reads in a main parameter file, which provides overall control to the mode of operation, reading in the data, and writing out the results.

To generate a default parameter file, run the following command:

```
  RadxKdp -print_params > RadxKdpParams.test
```

This will generate the parameter file ```RadxKdpParams.test```.

Here is an [example](./RadxKdpParams.md) RadxKdp parameter file.

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

Here is an [example](./KdpParams.md) of that file.

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

Similarly, to update the KDP-specific parameter file, run commands similar to the following:

```
  RadxKdp -params_kdp KdpParams.test -print_params_kdp > tempfile
  mv tempfile KdpParams.test
```

### Running RadxKdp by specifying files on the command line

The following is an example of running RadxKdp, specifying which files you want to process:

```
RadxKdp -params RadxKdp.kddc -debug -f /scr/rain1/rsfdata/projects/pecan/cfradial/kddc/moments/20150626/cfrad.20150626_00*.nc
```

### Running RadxKdp by specifying the start and end times

The following command would produce the same result as that above:

```
RadxKdp -params RadxKdp.kddc -debug -start "2015 06 26 00 00 00" -end "2015 06 26 01 00 00"
```

In the latter case, you need to ensure that the ```input_dir``` parameter is correctly set in RadxKdp.kddc.





