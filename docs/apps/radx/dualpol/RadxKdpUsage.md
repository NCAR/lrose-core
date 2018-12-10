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
