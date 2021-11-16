# Prepare for build depending on the OS

This document explains how to prepare a bare-bones OS for an LROSE download and build.

## Centos 7

```
  yum update -y
  yum install -y epel-release
  yum install -y git
```

WARNING - this will update to the latest sub-version of the release.
For example, from Centos 7.5 to 7.6.

If you do not want to do this, you can omit this step.

## Centos 8 and latest, Alma linux 8 and latest

```
  dnf -y update
  dnf install -y epel-release
  dnf install -y 'dnf-command(config-manager)'
  dnf config-manager --set-enabled powertools
  dnf install -y python2 python3
  alternatives --set python /usr/bin/python3
  dnf install -y git
```

## Fedora

```
  yum update -y
  yum install -y git
```

## Oracle 8

```
  dnf update -y
  dnf install -y git
```

## Debian

```
  apt-get update
  apt-get install -y git
```

## Suse

```
  zypper update -y
  zypper install -y git
```

# Checkout lrose bootstrap

```
  mkdir git
  cd git
  git clone https://github.com/ncar/lrose-bootstrap
```



