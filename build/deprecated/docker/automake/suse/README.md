# Building RPM packages for SUSE-type OSs

## Building RPMS using docker

We use docker containers to build the RPMs for various Suse versions of LINUX.

To make use of these you will need to install docker.

These builds have been tested on the following versions:

  * opensuse latest

## Steps in the process

The following are the steps required in the process:

| Step      | Script to run  |
| --------- | -------------  |
| Create custom container | ```make_custom_image.suse``` |
| Perform the lrose build | ```do_lrose_build.suse``` |
| Create the rpm | ```make_package.suse``` |
| Install and test the rpm | ```install_pkg_and_test.suse``` |

For the test step, the RPM is installed into a clean container, and one of the applications is run to make sure the installation was successful.

## Examples of running the scripts

### opensuse for core

```
  make_custom_image.suse opensuse latest
  do_lrose_build.suse opensuse latest core
  make_package.suse opensuse latest core
  install_pkg_and_test.suse opensuse latest core
```

## Location of RPMs

The RPMs are built in the containers, and then copied across onto cross-mounted locations on the host.

After the RPMs are built they are placed in /tmp.

For example:

```
  /tmp/opensuse-core/pkgs/x86_64/lrose-blaze-20190127.opensuse_latest.x86_64.rpm
```

These are also copied into the release directories:

```
  $HOME/releases/lrose-blaze
  $HOME/releases/lrose-core
```

## Installing the RPMs on a host system

Use zypper to install the RPM. For example:

```
  zypper install -y ./lrose-blaze-20190127.suse_latest.x86_64.rpm
```

Note that you need to specify the absolute path, hence the '.'.

  

