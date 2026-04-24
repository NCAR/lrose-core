# Prepare for build on LINUX OS

This document explains how to prepare a bare-bones OS for an LROSE download and build.

## 1. Install git and python

These steps need to be performed as ```root```, or using ```sudo```.

### RHEL8 - Alma, Rocky

```
  dnf -y update
  dnf install -y epel-release
  dnf install -y 'dnf-command(config-manager)'
  dnf config-manager --set-enabled powertools
  dnf install -y python2 python3
  alternatives --set python /usr/bin/python3
  dnf install -y git
```

### RHEL 9, 10 - Alma, Rocky

```
  yum update -y
  yum install -y epel-release
  yum install -y git
  yum install -y python
```

### Fedora

```
  yum update -y
  yum install -y git
  yum install -y python
```

### Debian, ubuntu

```
  apt-get update
  apt-get install -y git
  apt-get install -y python
```

### Suse

```
  zypper update -y
  zypper install -y git
  zypper install -y python
```

### Oracle

```
  dnf update -y
  dnf install -y git
  dnf install -y python
```

## 2. Checkout lrose bootstrap

```
  mkdir git
  cd git
  git clone https://github.com/ncar/lrose-bootstrap
```

## 3. Install package dependencies

This must be run as ```root``` or using ```sudo```.

```
  cd lrose-bootstrap/scripts
  ./install_linux_packages.py
```



