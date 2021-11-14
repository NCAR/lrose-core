# Download RPM and install - RHEL, RedHat, Fedora, Alma

1. [prepare](#prepare)
2. [download](#download)
3. [install](#install)
4. [verify](#verify)
5. [set path](#set_path)
6. [upgrade](#upgrade)

<a name="prepare"/>

## 1. Prepare

It is a good idea to update your OS first:

For Centos 7:

```
  yum update -y
  yum install -y epel-release
```

WARNING - this will update to the latest sub-version of the release.
For example, from Centos 7.5 to 7.6.

If you do not want to do this, you can omit this step.

For Centos 8:

```
  dnf -y update
  dnf install -y epel-release
  dnf install -y 'dnf-command(config-manager)'
  dnf config-manager --set-enabled powertools
  dnf install -y python2 python3
  alternatives --set python /usr/bin/python3
```

For Fedora:

```
  yum update -y
```

<a name="download"/>

## 2. Download

Download the RPM file from:

```
  https://github.com/NCAR/lrose-core/releases
```

A typical RPM release would be:

```
  lrose-core-20210214-centos_8.x86_64.rpm
```

Choose the RPM that matches your operating system.

<a name="install"/>

## 3. Install

Assume the download is in:

```
  $HOME/Downloads
```

Then for Centos 7 or Fedora:

```
  cd ~/Downloads
  yum install -y ./lrose-core-20210214-centos_8.x86_64.rpm
```

Then for Centos 8:

```
  cd ~/Downloads
  dnf install -y ./lrose-core-20210214-centos_8.x86_64.rpm
```

Make sure you include the leading ```.```.

<a name="verify"/>

## 4. Verify

LROSE will be installed in:

```
  /usr/local/lrose/bin
  /usr/local/lrose/include
  /usr/local/lrose/lib
  /usr/local/lrose/share
```

Test the installation by running the commands:

```
  /usr/local/lrose/bin/RadxPrint -h
  /usr/local/lrose/bin/RadxConvert -h
  /usr/local/lrose/bin/Radx2Grid -h
  /usr/local/lrose/bin/HawkEye
```

<a name="set_path"/>

## 5. Set Path

Since the binaries and scripts are installed in:

```
  /usr/local/lrose/bin
```

you need to add these directories to your path.

If you have trouble with runtime libraries, you may need to add the
installation library directory to your LD_LIBRARY_PATH:

```
  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lrose/lib
```

<a name="upgrade"/>

## 6. Upgrade

When the time comes to upgrade, you will need to download a new version of the RPM.

Assume the new RPM is in:

```
  $HOME/downloads
```

Then for Centos 7 or Fedora:

```
  cd ~/Downloads
  yum remove -y lrose-core
  yum install -y ./lrose-core-20210215-centos_8.x86_64.rpm
```

or Centos 8:

```
  cd ~/Downloads
  dnf remove -y lrose-core
  dnf install -y ./lrose-core-20210215-centos_8.x86_64.rpm
```

This will upgrade to the new version.

You can also downgrade by installing an older version of the RPM.

