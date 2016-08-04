############################################################
#
# Building LROSE on LINUX
#
# This readme shows you how to build the system and get
# the binaries installed in /usr/local (by default) or
# wherever you wish to install them.
#
############################################################

1. ftp tar file from the lrose distribution
-------------------------------------------

Create a directory for the distribution:

  cd
  mkdir lrose_build
  cd lrose_build

Download the distribution:

  ftp ftp.rap.ucar.edu
  log in as anonymous
  cd pub/titan/lrose
  get lrose-yyyymmdd.src.tgz
  quit

Alternatively, use a browser to download the file from:

  http://www.ral.ucar.edu/projects/titan/lrose

2. untar the distribution
-------------------------

  cd
  cd lrose_build
  tar xvfz lrose-yyyymmddsrc.tgz

The distribution will be unpacked into a subdirectory:

  lrose_build/lrose-yyyymmdd.src

3. Run the build scripts:
-------------------------

  cd lrose_build/lrose-yyyymmdd.src

To install in /usr/local, run:

  ./build_netcdf
  ./build_lrose

This will install in:

  /usr/local/include
  /usr/local/lib
  /usr/local/bin

This will first build and install netcdf, and then lrose.

To install in a different location, specify the install dir.
For example:

  ./build_netcdf /tmp/lrose
  ./build_lrose /tmp/lrose

will install in

  /tmp/lrose/include
  /tmp/lrose/lib
  /tmp/lrose/bin


4. Checking the build
---------------------

The build checks are run automatically at the end of the build script.

However, you also can run the checks independently:

After the build, you can check the build as follows:

  ./distribs/lrose/check_libs -p /usr/local
  ./distribs/lrose/check_apps -p /usr/local

5. Handling build errors
------------------------

If the build does not complete successfully, you will need to
track down the errors. It is the first errors in the build that
are the most important.

If you get errors, go into the directory giving problems, and
run the make as follows:

  make |& less

and scroll for errors.

Alternatively, run

  make >& make.log

and then inspect the make.log file.





