### How do I install TITAN?  I don't need to modify the source code; I just want to run the software.

Install the binary version of the lrose release from 20180626.  You can just use the binary version.  
Here are the steps I used to install  ...
```
     mkdir bin_test_area
     cd bin_test_area/
     wget https://github.com/NCAR/lrose-core/releases/download/lrose-20180626/lrose-20180626.bin.x86_64.tgz
     tar xvfz lrose-20180626.bin.x86_64.tgz
     ls
     cd lrose-20180626.bin.x86_64
     ./install_bin_release.py --verbose --prefix=/opt/local/lrose-20180626
     /opt/local/lrose/bin/RadxPrint -h
     /opt/local/lrose-20180626/bin/RadxPrint -h
     /opt/local/lrose-20180626/bin/Titan -h
```

