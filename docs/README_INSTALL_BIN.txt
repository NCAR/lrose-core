UCAR/EOL/RAL LROSE core software
================================

README for INSTALLING a BINARY distribution.

LINUX setup
-----------

Radx is primarily intended to run on LINUX and OSX.

Most good, up-to date LINUX distributions should work.

Recommended OSs are:

  Ubuntu
  Debian
  Fedora
  Scientific Linux
  Mac OSX

Required packages for compiling:

  tcsh shell
  perl shell
  python shell

Downloading
-----------

For 64-bit systems, the lrose binary release will be named:

  lrose-yyyymmdd.x86_64.tgz

For 32-bit systems, the lrose binary release will be named:

  lrose-yyyymmdd.i686.tgz

where yyyymmdd is the date of the distribution.

Download this file from:

  ftp.rap.ucar.edu/pub/titan/lrose

Previous releases can be found in:

  ftp.rap.ucar.edu/pub/titan/lrose/previous_releases

Download the file into a tmp area. For example:

  /tmp/lrose_dist

and install from there.

However, any suitable directory can be used for this purpose.

Un-tarring the file
-------------------

Use the command

  tar xvfz lrose-yyyymmdd.x86_64.tgz (64-bit systems)

or

  tar xvfz lrose-yyyymmdd.i686.tgz (32-bit systems)

to un-tar the distribution into the current directory.

yyyymmdd should be substituded with the actual date on the file.

Then, cd into the directory:

  cd lrose-yyyymmdd.x86_64

or

  cd lrose-yyyymmdd.i686

Installing
----------

You will probably need to be root for this step, unless you install
in your user area.

By default, the binaries will be installed in /usr/local/bin. You can specify an
alternative.

  install_bin_release 

will install in /usr/local/bin.

  install_bin_release /opt/local

will install in /opt/local/bin.

The support dynamic libraries will be found in:

  ..../bin/lrose_runtime_libs

i.e. in a subdirectory of the bin directory.

Installing development release
------------------------------

If you want to compile and build against the LROSE libraries, you will need the
development release which has the library and include files in addition to the
binary files.

To do this, use

  install_devel_release

will install in /usr/local/bin, /usr/local/lib and /usr/local/include.

Or, for example, 
  install_devel_release /opt/local

will install in /opt/local/bin, /opt/local/lib and /opt/local/include.

