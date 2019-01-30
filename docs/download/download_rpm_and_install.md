# Download RPM and install - RHEL, RedHat, Fedora

1. [prepare](#prepare)
2. [download](#download)
3. [install](#install)
4. [verify](#verify)
5. [upgrade](#upgrade)

<a name="prepare"/>

## 1. Prepare

It is a good idea to update your OS first:

```
  yum update -y
```

WARNING - this will update to the latest sub-version of the release.
For example, from Centos 7.5 to 7.6.

If you do not want to do this, you can omit this step.

<a name="download"/>

## 2. Download

Download the RPM file from:

```
  https://github.com/NCAR/lrose-core/releases
```

A typical RPM release would be:

```
  lrose-core-20190129-1.centos_7.x86_64.rpm
```

Choose the RPM that matches your operating system.

<a name="install"/>

## 3. Install

Assume the download is in:

```
  $HOME/Downloads
```

Then:

```
  cd ~/Downloads
  yum install -y ./lrose-core-20190129-1.centos_7.x86_64.rpm
```

Make sure you include the leading ```.```.

<a name="verify"/>

## 4. Verify

LROSE will be installed in:

```
  /usr/local/lrose/bin
  /usr/local/lrose/scripts
  /usr/local/lrose/include
  /usr/local/lrose/lib
  /usr/local/lrose/share
```

To run the binaries, and scripts, you will need to add:

```
  /usr/local/lrose/bin
  /usr/local/lrose/scripts
```

to your path.

Test the installation by running the commands:

```
  /usr/local/lrose/bin/RadxPrint -h
  /usr/local/lrose/bin/RadxConvert -h
  /usr/local/lrose/bin/Radx2Grid -h
  /usr/local/lrose/bin/HawkEye
```

If you have trouble with runtime libraries, you may need to add the library directory to your LD_LIBRARY_PATH:

```
  LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lrose/lib
```

## 5. Upgrade

To upgrade, you need to download a new version of the RPM.

Assume the new RPM is in:

```
  $HOME/downloads
```

Then:

```
  cd ~/Downloads
  yum install -y ./lrose-core-20190130-1.centos_7.x86_64.rpm
```

This will upgrade to the new version.

You can also downgrade by installing an older version of the RPM.

