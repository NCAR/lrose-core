Setting up the TS 7800
----------------------

Wiki page
---------

See the Teknologic Systems page:

  http://wiki.embeddedarm.com/wiki/TS-7800

Getting access to the console
------------------------------

When it ships, the 7800 jumpers are set to connect the console to the
serial port.

Connect the serial port to a either:

  (a) a serial port on your computer
  (b) a USB on your computer, using a USB/serial converter.

In either case, a NULL modem is required.

The TrendNet TU-S9 cable works fine.

Install picocom, and then run for example:

  sudo picocom -b 115200 /dev/ttyUSB0

Then power up the TS-7800, and you should see it boot.

Configuring the boot
--------------------

It will boot into a BusyBox shell. Exiting from this shell will allow
it to drop through into the main boot.

Doing ls -al will give something like:

      drwxr-xr-x   13 0        0            1024 Apr 12 14:36 .
      drwxr-xr-x   13 0        0            1024 Apr 12 14:36 ..
      -rw-r--r--    1 0        0               7 Apr 12 14:36 .ash_history
      drwxr-xr-x    2 0        0            2048 Jul 16  2008 bin
      drwxrwxrwt    4 0        0            3620 Apr 12 14:36 dev
      drwxr-xr-x    2 0        0            1024 May  9  2008 devkit
      -rw-r--r--    1 0        0           38530 Oct 10  2008 ehci-hcd.ko
      lrwxrwxrwx    1 0        0              13 Oct 10  2008 etc -> /mnt/root/etc
      lrwxrwxrwx    1 0        0              14 Oct 10  2008 home -> /mnt/root/home
      drwxr-xr-x    2 0        0            1024 Aug 26  2008 lbin
      lrwxrwxrwx    1 0        0              13 Oct 10  2008 lib -> /mnt/root/lib
      lrwxrwxrwx    1 0        0              16 Oct 10  2008 linuxrc -> linuxrc-fastboot
      -rwx------    1 0        0            2173 Aug 26  2008 linuxrc-fastboot
      -rwx------    1 0        0            2309 Aug 26  2008 linuxrc-mtdroot
      -rwx------    1 0        0            2362 Aug 26  2008 linuxrc-nfsroot
      -rwx------    1 0        0            2477 Aug 26  2008 linuxrc-sdroot
      drwx------    2 0        0           12288 Oct 10  2008 lost+found
      drwxr-xr-x    4 0        0            1024 May  9  2008 mnt
      drwxr-xr-x    2 0        0            1024 May  9  2008 onboardflash
      lrwxrwxrwx    1 0        0              13 Oct 10  2008 opt -> /mnt/root/opt
      dr-xr-xr-x   33 0        0               0 Jan  1  1970 proc
      drwxr-xr-x    2 0        0            1024 May  9  2008 root
      drwxr-xr-x    2 0        0            1024 Jul 16  2008 sbin
      -rw-r--r--    1 0        0             218 May  9  2008 shinit
      drwxr-xr-x   10 0        0               0 Jan  1  1970 sys
      lrwxrwxrwx    1 0        0              13 Oct 10  2008 tmp -> /mnt/root/tmp
      -rw-r--r--    1 0        0            6048 Oct 10  2008 ts7800.ko
      -rw-r--r--    1 0        0            3123 Jun  6  2008 ts7800.subr
      lrwxrwxrwx    1 0        0               9 Oct 10  2008 ts7800_nand.ko -> ts7800.ko
      -rw-r--r--    1 0        0           37447 Oct 10  2008 tssdcard.ko
      -rw-r--r--    1 0        0           17573 Apr 28  2010 tsuart1.ko
      -rw-r--r--    1 0        0            5581 Apr 28  2010 tsuart7800.ko
      -rw-r--r--    1 0        0          136601 Oct 10  2008 usbcore.ko
      lrwxrwxrwx    1 0        0              13 Oct 10  2008 usr -> /mnt/root/usr
      lrwxrwxrwx    1 0        0              13 Oct 10  2008 var -> /mnt/root/var

linuxrc is a link to the boot image you want. Setting this to
linux-sdroot will get it to boot using the full-size SD card.

      lrwxrwxrwx    1 0        0              16 Oct 10  2008 linuxrc -> linuxrc-sdroot

Since this action needs to be saved to flash, this is the appropriate
command for setting up to boot from SD:

  rm linuxrc && ln -s linuxrc-sdroot linuxrc && save

Then on the next boot it should go directly to the SD card.

Saving bootable image from SD card to disk
------------------------------------------

This applies to SD cards purchased from the vendor.

  (a) Insert SD card into card reader

  (b) Type 'mount' to see device name (e.g. /dev/sdd)

  (c) Alternatively, install gnome-disk-utility, and then run 'palimpsest', select the
      devide and view its details.

  (d) Copy to image file:

    dd if=/dev/sdd of=/path/to/backup.dd bs=4M conv=fsync

Copying backed-up bootable image to SD card.
--------------------------------------------

    dd if=/path/to/backup.dd of=/dev/sdd bs=4M conv=fsync

Downloading bootable image from web
-----------------------------------

Get file from Technologic:

  ftp://ftp.embeddedarm.com/ts-arm-sbc/ts-7800-linux/binaries/ts-images/512mbsd-squeeze-2.6.34-nov-16-2011.dd.bz2

and run bunzip2. This should be a 512 MB image.

Copying image to SD card
------------------------

  sudo dd if=512mbsd-squeeze-2.6.34-nov-16-2011.dd of=/dev/sdd bs=4M conv=fsync

Resizing image to use full size of the SD card.
-----------------------------------------------

  (a) install gparted and jfsutils
  (b) xhost + (to allow gparted access to X under root)
  (c) gparted /dev/sdd (or whatever device it is)
  (d) select partition 4
  (e) resize to max, hit apply

Setting up network
------------------

Edit:

  /etc/network/interfaces:

For example:

  # Used by ifup(8) and ifdown(8). See the interfaces(5) manpage or
  # /usr/share/doc/ifupdown/examples for more information.

  auto lo
  iface lo inet loopback

  auto eth0 
  #iface eth0 inet dhcp
  iface eth0 inet static
          address 192.168.84.180
          network 192.168.84.0
          netmask 255.255.255.0
          broadcast 192.168.84.255
          gateway 192.168.84.240

Edit /etc/hosts, to add localhost:

  127.0.0.1 localhost ts7800
  # The following lines are desirable for IPv6 capable hosts
  # (added automatically by netbase upgrade)
  ::1     ip6-localhost ip6-loopback
  fe00::0 ip6-localnet
  ff00::0 ip6-mcastprefix
  ff02::1 ip6-allnodes
  ff02::2 ip6-allrouters
  ff02::3 ip6-allhosts

Setting up apt-get
------------------

Since Debian sources have moved, edit

  /etc/apt/sources.list

to contain the following:

  deb http://archive.debian.org/debian etch main
  deb-src http://archive.debian.org/debian etch main

Setting up the terminal type
----------------------------

This version of debian does not understand the more advanced terminal types.

So you need to include:

  setenv TERM xterm

in your .cshrc file to get vi and emacs to work properly.

