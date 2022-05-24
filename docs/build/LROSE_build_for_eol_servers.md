# Building LROSE for deployment on EOL servers

The LROSE binaries are installed in the ```/opt/local/lrose/bin``` crossmount on the EOL servers.

The is an ```/opt/local``` for each version of the OS.

You need to perform the build on a machine with the correct OS version.

1. [prepare](#prepare)
2. [download](#download)
3. [build core](#build-core)
4. [build CIDD](#build-cidd)
5. [install into /opt/local](#install)
6. [make symbolic link](#make-link)

<a name="prepare"/>

## 1. Prepare the OS

Install the required packages. See:

* [LROSE package dependencies](./lrose_package_dependencies.md)

<a name="download"/>

## 2. Download from GitHub

Create a working directory, clone lrose-core

```
  mkdir -p ~/git
  cd ~/git
  git clone https://github.com/ncar/lrose-core 
```

<a name="build-core"/>

## 3. Build LROSE core plus CSU apps.

Temporary install in ```/tmp/lrose-core```.

```
  cd ~/git/lrose-core/build/scripts
  ./checkout_and_build_auto.py --prefix /tmp/lrose-core --fractl --vortrac --samurai --cmake3 --installAllRuntimeLibs
```

NOTE - the ```--cmake3``` is only needed for Centos 7, not for Centos 8.

<a name="build-cidd"/>

## 4. Build CIDD

Temporary install in ```/tmp/lrose-cidd```.

```
  cd ~/git/lrose-core/build/scripts
  ./build_cidd.py --prefix /tmp/lrose-cidd --installLroseRuntimeLibs
```

<a name="install"/>

## 5. Install into /opt/local

We make a directory for today's date, and copy the binaries etc in there.

```
  cd /opt/local
  mkdir lrose-20201017 # or whatever the date is
  cd /tmp/lrose-core
  rsync -av * /opt/local/lrose-20201017
  cd /tmp/lrose-cidd/bin
  rsync -av lrose-cidd_runtime_libs CIDD SpectraPlot SpectraScope CiddParams2JazzXml /opt/local/lrose-20201017/bin
```

<a name="make-link"/>

## 6. Make link to latest

```
  cd /opt/local
  rm lrose (this is the link to the current version)
  ln -s lrose-20201017 lrose
```
