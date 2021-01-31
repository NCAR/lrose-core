# Building RPM packages for RedHat-type OSs (RHEL, CENTOS, FEDORA)

## Building RPMS using docker

We use docker containers to build the RPMs for various RedHat versions of LINUX.

To make use of these you will need to install docker.

These builds have been tested on the following versions:

  * centos 6
  * centos 7
  * fedora 28
  * fedora 29

## Steps in the process

The following are the steps required in the process:

| Step      | Script to run  |
| --------- | -------------  |
| Create custom container | ```make_custom_image.redhat``` |
| Perform the lrose build | ```do_lrose_build.redhat``` |
| Create the rpm | ```make_package.redhat``` |
| Install and test the rpm | ```install_pkg_and_test.redhat``` |

For the test step, the RPM is installed into a clean container, and one of the applications is run to make sure the installation was successful.

## Examples of running the scripts

### Centos 6 for blaze

```
  make_custom_image.redhat centos 6
  do_lrose_build.redhat centos 6 blaze
  make_package.redhat centos 6 blaze
  install_pkg_and_test.redhat centos 6 blaze
```

### Centos 7 for core

```
  make_custom_image.redhat centos 7
  do_lrose_build.redhat centos 7 core
  make_package.redhat centos 7 core
  install_pkg_and_test.redhat centos 7 core
```

### Fedora 28 for blaze

```
  make_custom_image.redhat fedora 28
  do_lrose_build.redhat fedora 28 blaze
  make_package.redhat fedora 28 blaze
  install_pkg_and_test.redhat fedora 28 blaze
```

### Fedora 29 for core

```
  make_custom_image.redhat fedora 29
  do_lrose_build.redhat fedora 29 core
  make_package.redhat fedora 29 core
  install_pkg_and_test.redhat fedora 29 core
```

## Location of RPMs

The RPMs are built in the containers, and then copied across onto cross-mounted locations on the host.

After the RPMs are built they are placed in /tmp.

For example:

```
  /tmp/centos-6-blaze/pkgs/x86_64/lrose-blaze-20190127.centos_6.x86_64.rpm
  /tmp/centos-7-core/pkgs/x86_64/lrose-blaze-20190127.centos_7.x86_64.rpm
  /tmp/fedora-28-blaze/pkgs/x86_64/lrose-blaze-20190127.fedora_28.x86_64.rpm
  /tmp/fedora-29-core/pkgs/x86_64/lrose-blaze-20190127.fedora_29.x86_64.rpm
```

These are also copied into the release directories:

```
  $HOME/releases/lrose-blaze
  $HOME/releases/lrose-core
```

## Installing the RPMs on a host system

You use yum to install the RPMs on your host.

For RHEL and CENTOS, you first need to install epel-release:

```
  yum install -y epel-release
```

This step is not needed for fedora.

The use yum to install the RPM. For example:

```
  yum install -y ./lrose-blaze-20190127.fedora_29.x86_64.rpm
```

Note that you need to specify the absolute path, hence the '.'.

  

