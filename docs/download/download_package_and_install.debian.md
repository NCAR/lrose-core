# Download DEB package and install - Debian, Ubuntu

1. [prepare](#prepare)
2. [download](#download)
3. [install](#install)
4. [verify](#verify)
5. [set path](#set_path)
6. [upgrade](#upgrade)

<a name="prepare"/>

## 1. Prepare

It should update your OS first:

```
  apt-get update
```

It is smoother if you prepare your OS first by installing the dependencies.

You can do this by running a python script in the GitHub lrose-bootstrap repository:

```
  mkdir git
  cd git
  git clone https://github.com/ncar/lrose-bootstrap
  cd lrose-bootstrap/scripts
  sudo ./install_linux_packages.py
```

This does mean that you need python installed and available before running this step.

<a name="download"/>

## 2. Download

Download the .deb file from:

```
  https://github.com/NCAR/lrose-core/releases
```

A typical .deb release would be:

```
  lrose-core-20250811.debian_12.amd64.deb
```

A typical ubuntu release would be:

```
  lrose-core-20250811.ubuntu_24.04.amd64.deb
```

Choose the .deb file that matches your operating system.

<a name="install"/>

## 3. Install

Assume the download is in:

```
  $HOME/Downloads
```

Then:

```
  cd ~/Downloads
  apt-get install -y ./lrose-core-20250811.debian_12.amd64.deb
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

When the time comes to upgrade, you will need to download a new version of the .deb file.

Assume the new file is in:

```
  $HOME/downloads
```

Then:

```
  cd ~/Downloads
  apt-get remove -y lrose-core
  apt-get install -y ./lrose-core-20190130.debian_9.amd64.deb
```

This will upgrade to the new version.

You can also downgrade by installing an older version of the .deb file.


