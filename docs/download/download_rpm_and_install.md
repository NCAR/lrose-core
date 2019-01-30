# Download RPM and install - RHEL, RedHat, Fedora

1. [download](#download)
2. [install](#install)
3. [verify](#verify)
4. [upgrade](#upgrade)

## 1. Download

Download the rpm file from:

```
  https://github.com/NCAR/lrose-core/releases
```

A typical rpm release would be:

```
  lrose-core-20190129-1.centos_7.x86_64.rpm
```

<a name="install"/>

## 2. Install

Assume the download is in:

```
  $HOME/downloads
```

Then:

```
  cd ~/Downloads
  yum install -y ./lrose-core-20190129-1.centos_7.x86_64.rpm
```

Make sure you include the leading '.'.

<a name="verify"/>

## 3. Verify

Lrose will be installed in:

```
  /usr/local/lrose/bin
  /usr/local/lrose/scripts
  /usr/local/lrose/include
  /usr/local/lrose/lib
```

You will need to add:

```
  /usr/local/lrose/bin
  /usr/local/lrose/scripts
```

to your path.

Test by trying the commands:

```
  /usr/local/lrose/bin/RadxPrint -h
  /usr/local/lrose/bin/RadxConvert -h
  /usr/local/lrose/bin/Radx2Grid -h
  /usr/local/lrose/bin/HawkEye
```

## 4. Upgrade

Assume the download is in:

```
  $HOME/downloads
```

Then:

```
  cd ~/Downloads
  yum install -y ./lrose-core-20190129-1.centos_7.x86_64.rpm
```

Make sure you include the leading '.'.

<a name="verify"/>

