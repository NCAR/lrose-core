
## download; untar release or brew install ...
``` 
$ tar -xvf blaze-20180103.src.tgz   
 
$ less build/build_lrose
$ cd /tmp
```
## build and install netcdf subset for lrose ...
```
$ cd lrose-netcdf
$ cd *net*
```
### set environment variables
```
$ setenv LROSE_INSTALL_DIR /tmp/test/blaze/install
$ setenv LROSE_CORE_DIR /tmp/test/blaze/tmp
$ ls /tmp/test_blaze/install
$ build_and_install_netcdf -x /tmp/test_blaze/install

# there are many warnings, but it should produce ...
# Congratulations! message
```
### verify it is there ...
```
$ ls install
$ cd test_blaze
$ cd blaze
$ cd tmp
```
## build and install lrose ...
```
$ cd lrose-core
$ more README.md
$ cd build
$ more build_lrose.ncar
$ ./build/build_lrose.ncar -x /tmp/test_blaze/install

# a few warnings, but it should produce ..
# SUCCESS
# ALL LIBS INSTALLED
# ALL APPS INSTALLED 
# messages
```
### verify it is there ...
```
$ cd install
$ ls bin
```
## perform some sanity testing ...
```
$ RadxPrint --help
$ RadxPrint -print_params
$ RadxPrint -help
$ Radx2Grid -help
$ pwd
$ cd bin
$ Radx2Grid -print_params
$ RadxBufr -help
$ RadxBufr -print_params
$ RadxConvert -print_params
```
### it would be nice to have something like this to verify the version of the software running
```
$ RadxConvert -version  
```
```
$ RadxConvert -help
$ HawkEye -help
```
### run some data through ...
```
$ RadxBufr -f /scr/sci/salio/200.16.116.24/L2/RMA1/2017/04/30/07/0516/RMA1_0117_02_TH_20170430T070516Z.BUFR
$ ls output
$ RadxPrint -f output/20170430/cfrad.20170430_070516.000_to_20170430_070603.933_1_SUR.nc -rays -data | less
$ HawkEye -f output/20170430/cfrad.20170430_070516.000_to_20170430_070603.933_1_SUR.nc
$ cd ..
$ ls
$ find . -name dbz.colors
```
