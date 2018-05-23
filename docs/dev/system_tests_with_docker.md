# System Tests With Docker Images
Docker images are useful to test the installation of software on various operating systems.  [Docker official images](https://hub.docker.com/explore) provide a clean starting point on which to test LROSE software installation.  A Dockerfile details all the necessary packages and steps needed to install an LROSE release.  

Steps:
1. Build Docker container
2. Run system tests in Docker container. 

The ultimate goal is to run the tests like this ...
```
$ docker-compose up -d
$ ./run_tests
$ docker-compose down
```


### Source Install
- Linux flavors
- Mac OSX
### Package Install
- brew (lrose-blaze.rb)
- yum
- apt-get
### Via Containers
- Docker
### Binary Install
- Linux flavors
- Mac OSX


### Obtain a release of LROSE

I need to build on rain because the Mac doesn’t have libtool.  For a Mac release, add the argument --osx
```
% ./build/create_src_release.py --package=lrose-blaze
```

Where is the release?
```
% rsync /h/eol/brenda/releases/lrose-blaze/lrose-blaze-20180516.src.tgz /h/eol/brenda/test_area/tmp/.
```
then on Mac ..
```
% mv /h/eol/brenda/test_area/tmp/lrose-blaze-20180516.src.tgz /Users/brenda/CI/test_lrose_blaze/
```

### Source Install on Centos
1. build the container in a directory containing the LROSE release, and the [Dockerfile]().  Note the "." at the end of the command.  This is crutial.  It tells Docker to use the contents of the current directory.
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
- inside the container, run the test 
```
/usr/local/lrose/bin/HawkEye -params /tmp/test_area/hawkeye/HawkEye_KARX.archive \
-f /tmp/output/20170408/cfrad.20170408_005646.509_to_20170408_010514.500_KARX_Surveillance_SUR.nc
```
