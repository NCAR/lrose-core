
### Source Install on Centos
1. build the container in a directory containing the LROSE release, and the [Dockerfile](Dockerfile).  Note the "." at the end of the command.  This is crutial.  It tells Docker to use the contents of the current directory.
```
docker build -t "centos-source:Dockerfile" . 
```
2. run tests
- start the container
```
docker run \
-v /Users/brenda/git/lrose-displays/color_scales:/home/lrose/git/lrose-displays/color_scales \
-v /Users/brenda/test_area/hawkeye:/tmp/test_area/hawkeye \
-v /Users/brenda/lrose/bin/output/20170408:/tmp/output/20170408 \
-e DISPLAY=128.117.80.109:0 -v /tmp/.X11-unix:/tmp/.X11-unix:rw -v /Users/brenda/.Xauthority:/home/lrose/.Xauthority \
-it centos-source:Dockerfile
```
- inside the container, run the tests:  

test HawkEye display  (HawkEye GUI should start; field data should be displayed with appropriate color scales).
```
/usr/local/lrose/bin/HawkEye -params /tmp/test_area/hawkeye/HawkEye_KARX.archive \
-f /tmp/output/20170408/cfrad.20170408_005646.509_to_20170408_010514.500_KARX_Surveillance_SUR.nc
```
test for default color scales  (HawkEye GUI should start; field data should be displayed with appropriate color scales).
```
/usr/local/lrose/bin/HawkEye -f /tmp/output/20170408/cfrad.20170408_005646.509_to_20170408_010514.500_KARX_Surveillance_SUR.nc
```
test RadxConvert 
```
/usr/local/lrose/bin/RadxConvert -h
```
test RadxPrint
```
/usr/local/lrose/bin/RadxPrint -h
```
test Radx2Grid
```
/usr/local/lrose/bin/Radx2Grid -h
```
