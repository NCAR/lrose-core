Installing SpolS2DAngles on TS 7800
-----------------------------------

Checkout
--------

Checkout libs:

  mkdir cvs
  cd ~/cvs
  cvs co libs/dataport
  cvs co libs/tdrp
  cvs co libs/toolsa
  cvs co libs/didss
  cvs co libs/dsserver

Checkout apps:

  cd ~/cvs
  cvs co apps/procmap
  cvs co apps/spol/src/SpolS2DAngles

Building
--------

Build the libs:

  cd ~/libs/tdrp
  make opt install

  cd ~/libs/dataport
  make opt install

  cd ~/libs/toolsa
  make opt install

  cd ~/libs/didss
  make opt install

  cd ~/libs/dsserver
  make opt install

Build apps:

  cd ~/cvs/apps/procmap/src/procmap
  make install

  cd ~/cvs/apps/procmap/src/print_procmap
  make install

  cd ~/cvs/apps/spol/src/SpolS2DAngles
  make install

Installing for spol
-------------------

  cd ~/cvs/apps/spol/src/SpolS2DAngles
  sudo cp init_d_angles_script.spol /etc/init.d/angles
  sudo mkdir -p /opt/bin
  sudo mkdir -p /opt/params
  sudo cp SpolS2DAngles /opt/bin
  sudo cp SpolS2DAngles.spol /opt/params

Installing for lab
------------------

  cd ~/cvs/apps/spol/src/SpolS2DAngles
  sudo cp init_d_angles_script.lab /etc/init.d/angles
  sudo mkdir -p /opt/bin
  sudo mkdir -p /opt/params
  sudo cp SpolS2DAngles /opt/bin
  sudo cp SpolS2DAngles.lab /opt/params

Install procmap
---------------

  cd ~/cvs/apps/spol/src/SpolS2DAngles
  sudo cp init_d_procmap_script /etc/init.d/process-mapper

  cd ~/cvs/apps/procmap/src/procmap
  sudo cp procmap /opt/bin

Make links for auto-startup on boot
-----------------------------------

cd /etc/rc3.d
ln -s ../init.d/process-mapper S59process-mapper
ln -s ../init.d/angles S60angles


