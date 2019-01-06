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
| Source   | | | |
| Container (Docker) | | | |
| RPM | | | |


### MacOS
| From ... | Download Location | Install with ... | Start Command |
|----------|-------------------|------------------|---------------|
| Source   | | | |
| Container (Docker) | | | |
| Brew | ** menus may not work ** | | |
|      | click away then back     | | |
| App | | | |


### Windows
| As ... |
|----------|
|[Linux subsystem](https://github.com/NCAR/lrose-core/issues/61) |


## How to use HawkEye
### The parameter file
#### How to generate one
#### Where is a default one?
#### The Top 10 Parameters

These are my favorites, the parameters I check and change to get running.
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
3. start/end date and timeA
```
archive_start_time = “1970 01 01 00 00 00”;
archive_stop_time = “1970 01 01 00 00 00”;
archive_time_span_secs = 3600;
```
4. color scales
```
color_scale_dir = “../share/color_scales”;
```
5. data source ** Note: There is an expected directory structure for the data files
```
archive_data_url = “/data/cfradial/kddc”;
```
6. display mode (POLAR or BSCAN)
```
display_mode = POLAR_DISPLAY;
```
7. saving images to file
```
images_output_dir = “/tmp/images/HawkEye”;
```
8. image file format (png, jpg, gif)
```
images_file_name_extension = “png”;
```
9. debug mode
```
debug = DEBUG_NORM;
```
10. window height and width

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

* Open Params File menu option
* Merge SOLOII into HawkEye
* UNDO 
* A few screen shots

### Demo with Data
~/Workshop2019
start_HawkEye.test  

data run from time 00:00 to 03:00:00
