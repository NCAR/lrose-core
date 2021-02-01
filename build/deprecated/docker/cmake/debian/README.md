# Building .deb package files for Debian-type OS versions (debian, ubuntu)

## Building .deb package files for LROSE using docker

We use docker containers to build the .deb package files for various Debian-based versions of LINUX.

To make use of these you will need to install docker.

These builds have been tested on the following versions:

  * debian 9
  * ubuntu 16.04
  * ubuntu 18.04
  * ubuntu 18.10

## Steps in the process

The following are the steps required in the process:

| Step      | Script to run  |
| --------- | -------------  |
| Create custom container | ```make_custom_image.debian``` |
| Perform the lrose build | ```do_lrose_build.debian``` |
| Create the package | ```make_package.debian``` |
| Install and test the package | ```install_pkg_and_test.debian``` |

For the test step, the package is installed into a clean container, and one of the applications is run to make sure the installation was successful.

## Examples of running the scripts

### Debian 9 for blaze

```
  make_custom_image.debian debian 9
  do_lrose_build.debian debian 9 blaze
  make_package.debian debian 9 blaze
  install_pkg_and_test.debian debian 9 blaze
```

### Ubuntu 16.04 for blaze

```
  make_custom_image.debian ubuntu 16.04
  do_lrose_build.debian ubuntu 16.04 blaze
  make_package.debian ubuntu 16.04 blaze
  install_pkg_and_test.debian ubuntu 16.04 blaze
```

### Ubuntu 18.04 for blaze

```
  make_custom_image.debian ubuntu 18.04
  do_lrose_build.debian ubuntu 18.04 blaze
  make_package.debian ubuntu 18.04 blaze
  install_pkg_and_test.debian ubuntu 18.04 blaze
```

### Ubuntu 18.10 for blaze

```
  make_custom_image.debian ubuntu 18.10
  do_lrose_build.debian ubuntu 18.10 blaze
  make_package.debian ubuntu 18.10 blaze
  install_pkg_and_test.debian ubuntu 18.10 blaze
```

## Location of .deb files

The .deb files are built in the containers, and then copied across onto cross-mounted locations on the host.

After the .deb files are built they are placed in /tmp.

For example:

```
  /tmp/debian-9-blaze/pkgs/lrose-blaze-20190127.debian-9.amd64.deb
  /tmp/ubuntu-16.04-blaze/pkgs/lrose-blaze-20190127.ubuntu-16.04.amd64.deb
  /tmp/ubuntu-18.04-blaze/pkgs/lrose-blaze-20190127.ubuntu-18.04.amd64.deb
  /tmp/ubuntu-18.10-blaze/pkgs/lrose-blaze-20190127.ubuntu-18.10.amd64.deb
```

These are also copied into the release directories:

```
  $HOME/releases/lrose-blaze
  $HOME/releases/lrose-core
```

## Installing the packages on a host system

You use apt-get to install the package on your host.
For example:

```
  apt-get update
  apt-get install -y ./lrose-blaze-20190127.ubuntu-18.04.amd64.deb
```

Note that you need to specify the absolute path, hence the '.'.

  

