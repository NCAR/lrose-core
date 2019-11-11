# HawkEye
What is HawkEye?

Quote from [NSF-LROSE](https://nsf-lrose.github.io/howtorun_HawkEye.html)
> HawkEye is a lidar and radar display tool. It can display real-time and archived CfRadial (and Dorade) data either in BSCAN,  PPI, or RHI geometry.

## Polar (PPI/RHI) Display
![alt_text](./images/Polar_Display.png)


## BSCAN Display
![alt_text](./images/BSCAN_Display.png)

## How to install HawkEye

### Linux
| From ... | Download Location | Install with ... | Start Command |
|----------|-------------------|------------------|---------------|
| source   | [NSF LROSE](https://nsf-lrose.github.io/software.html)| python build script | `/install/path/bin/HawkEye`|
| container (Docker) | [NSF LROSE](https://nsf-lrose.github.io/software.html)| `docker pull nsflrose/lrose-blaze`| lrose wrapper script (ask Bruno) `lrose -h`| 
|          | | | or `docker run lrose-blaze <various args>`|
| RPM | (test)[lrose-alpha](https://github.com/NCAR/lrose-alpha/releases)`lrose-blaze-20180629.x86_64.rpm`|`rpm -i lrose-blaze-yyyymmdd.x86_64.rpm` | `/usr/local/lrose/bin/HawkEye` |


### MacOS
| From ... | Download Location | Install with ... | Start Command |
|----------|-------------------|------------------|---------------|
| source   | [NSF LROSE](https://nsf-lrose.github.io/software.html)| python build script | `/install/path/bin/HawkEye`|
| container (Docker) | same as for Linux above | same as for Linux above| same as for Linux above|
| brew |  | `brew install lrose-blaze.rb` | `/usr/local/bin/HawkEye`|
|      |  |                            | ** menus may not work ** |
|      |  |                            | click away then back |
| App | (test)[lrose-release-test](https://github.com/NCAR/lrose-release-test/releases)`HawkEye_Blaze.dmg`| download .dmg file | click on App |
| |                                              | drag icon to Applications folder | |



### Windows
| as ... |
|----------|
|[Linux subsystem](https://github.com/NCAR/lrose-core/issues/61) |


## How to use HawkEye
Generally, just start HawkEye via the command line or click the App. 

```
HawkEye
HawkEye -h
HawkEye -f test_data/cfradial/kddc/20150626/cfrad.20150626_025610.151_to_20150626_030145.891_KDDC_v270_Surveillance_SUR.nc
HawkEye -params field_project.HawkEye.params
```

Then, you can play with the ...
1. parameter file
2. color scales
3. directory structure

### The parameter file

* Many parameters available to customize the display
* Many parameters can also be set on the command line

#### How to generate one
```
HawkEye --print_params 
HawkEye --print_params > field_project.HawkEye.params
```
#### Where is a default one?
#### The Top 10 Parameters

These are my favorites, the parameters I check and change to get things running.
1. archive vs. realtime mode
```
begin_in_archive_mode = TRUE; or FALSE;
```
2. parameter/field names
```
fields = {
        label = “DBZ”,
        raw_name = “DBZ”,
        filtered_name = “”,
        units = “dBZ”,
        color_map = “dbz.colors”,
        shortcut = “1”
};
```
3. start date and time
```
archive_start_time = “1970 01 01 00 00 00”;
```
4. some kind of end time 
```
archive_stop_time = “1970 01 01 00 00 00”;
archive_time_span_secs = 3600;
```
5. color scales
```
color_scale_dir = “../share/color_scales”;
```
6. data source ** Note: There is an expected directory structure for the data files
```
archive_data_url = “/data/cfradial/kddc”;
```
7. display mode (POLAR or BSCAN)
```
display_mode = POLAR_DISPLAY;
```
8. saving images to file
```
images_output_dir = “/tmp/images/HawkEye”;
```
9. image file format (png, jpg, gif)
```
images_file_name_extension = “png”;
```
10. debug mode
```
debug = DEBUG_NORM;
```

### The Color Scales
The color scales are expected to be in a particular location. However, there are some internal, default color scales:
*  default
*  rainbow
*  eldoraDbz
*  spolDbz
*  eldoraVel
*  spolVel
*  spolDiv

You can set the directory for color scales in 2 ways:
* Set to the absolute path
* Set as a path relative to the location of the application binary
executable.

### Expected directory structure and data file names
```
+-- field_project
|   +-- YYYYMMDD
|       +-- cfrad.YYYYMMDD_HHMMSS.SSS_to_YYYYMMDD_HHMMSS.SSS_*
```


## The Future of HawkEye - advertisement

* Boundary Editor
* Open Params File menu option
* Merge SOLOII into HawkEye
* undo editing
* A few screen shots

## Boundary Editor ##
![alt_text](./images/BoundaryEditorScreen0.jpg "Boundary Editor")

#### Boundary Editor Video Tutorial:
https://vimeo.com/369963107


### Examine as Spreadsheet
![Examine as Spreadsheet](./images/HawkEye_SOLOII_examine.png "Examine as Spreadsheet")

### Color Palette Editor
![alt_text](./images/HawkEye_SOLOII_color_palette.png "Color Palette Editor")

### Predefined Color Scales
![alt_text](./images/HawkEye_SOLOII_sample_color_palettes.png "Predefined Color Scales")

## Demo with Data
~/Workshop2019
start_HawkEye.test  

data run from time 00:00 to 03:00:00
