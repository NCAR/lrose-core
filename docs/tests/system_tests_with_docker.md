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
- Mac OSX (This is difficult to do, since there is no mac osx docker base image; I've tried linuxbrew/linuxbrew, centos + install llvm and clang, darwink/ci-mysql, rsmmr/clang, python, ruby.  Nothing seems to work.  The closest is with installing llvm and clang, but the package installer hangs after cmake ... make; need a docker base image that provides uname -a with a response containing darwin.)
### Package Install
- brew (lrose-blaze.rb) 
```
using linuxbrew/linuxbrew …
I tried to brew install lrose-blaze.rb
It gets stuck on make …
==> Downloading https://releases.llvm.org/6.0.0/compiler-rt-6.0.0.src.tar.xz
######################################################################## 100.0%
==> cmake -G Unix Makefiles /tmp/llvm-20180522-29614-1tkwygc/llvm-6.0.0.src -DCMAKE_C_FLAGS_RELEASE=-DNDEBUG -DCMAKE_CXX_FLAGS_RELE
==> make
```
- yum (TODO: make an RPM of LROSE-blaze)
- apt-get (TODO: make an RPM of LROSE-blaze)
### Via Containers
- Docker
### Binary Install (no binary versions at this time)
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

### GUI Tests
For any GUI, I needed to add the ip address of the host computer
```
xhost +nnn.nnn.nn.nnn
```


