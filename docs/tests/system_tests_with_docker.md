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
- Linux flavors: [Centos](centos_source/README.md) [Ubuntu](ubuntu_source.md) [Debian]() [SUSE]()
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

