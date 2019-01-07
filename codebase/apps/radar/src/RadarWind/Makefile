# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
# ** Copyright UCAR (c) 1990 - 2016                                         
# ** University Corporation for Atmospheric Research (UCAR)                 
# ** National Center for Atmospheric Research (NCAR)                        
# ** Boulder, Colorado, USA                                                 
# ** BSD licence applies - redistribution and use in source and binary      
# ** forms, with or without modification, are permitted provided that       
# ** the following conditions are met:                                      
# ** 1) If the software is modified to produce derivative works,            
# ** such modified software should be clearly marked, so as not             
# ** to confuse it with the version available from UCAR.                    
# ** 2) Redistributions of source code must retain the above copyright      
# ** notice, this list of conditions and the following disclaimer.          
# ** 3) Redistributions in binary form must reproduce the above copyright   
# ** notice, this list of conditions and the following disclaimer in the    
# ** documentation and/or other materials provided with the distribution.   
# ** 4) Neither the name of UCAR nor the names of its contributors,         
# ** if any, may be used to endorse or promote products derived from        
# ** this software without specific prior written permission.               
# ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
# ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
# ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
SRC = $(shell ls *.hh *.cc)

#INCDIRA=/d1/steves/ftp/titan.radx/tdi/include
#LIBDIRA=/d1/steves/ftp/titan.radx/tdi/lib

INCDIRB=/d1/steves/ftp/geographicLib/tdi/include
LIBDIRB=/d1/steves/ftp/geographicLib/tdi/lib

INCDIRC=/d1/steves/cvs/includeg
LIBDIRC=/d1/steves/cvs/lib

INCDIRD=/usr/local/netcdf4/include
LIBDIRD=/usr/local/netcdf4/lib

INCDIRE=/d1/steves/cvs/include
LIBDIRE=/d1/steves/cvs/lib

EIGENLIB = /d1/steves/ftp/eigen/tda/eigen-eigen-6e7488e20373

RadarWind: ${SRC}
	g++ -g -c -I${INCDIRB} -I${INCDIRC} -I${INCDIRD} -I${INCDIRE} \
	  -I${EIGENLIB} RadarWind.cc

	g++ -g -o RadarWind RadarWind.o \
	  -L${LIBDIRB} -lGeographic \
	  -L${LIBDIRE} -lkd -lRadx \
	  -L/usr/local/netcdf4/lib  \
	  -lnetcdf -lhdf5_hl -lhdf5 -lz


# Test tdi/RadarWind.cc
RadarWindi: tdi/RadarWind.cc
	g++ -o RadarWindi.o -g -c -I${INCDIRB} -I${INCDIRC} -I${INCDIRD} -I${INCDIRE} \
	  -I${EIGENLIB} tdi/RadarWind.cc

	g++ -g -o RadarWindi RadarWindi.o \
	  -L${LIBDIRB} -lGeographic \
	  -L${LIBDIRE} -lkd -lRadx \
	  -L/usr/local/netcdf4/lib  \
	  -lnetcdf -lhdf5_hl -lhdf5 -lz



clean:
	/bin/rm -f RadarWind.o RadarWind
	/bin/rm -f Tradx.o Tradx

Tradx: ${SRC}
	g++ -g -c -I${INCDIRB} -I${INCDIRC} -I${INCDIRD} Tradx.cc

	g++ -g -o Tradx Tradx.o \
	  -L/d1/steves/cvs/lib -lRadx \
	  -L${LIBDIRB} -lGeographic \
	  -L/usr/local/netcdf4/lib  \
	  -lnetcdf -lhdf5_hl -lhdf5 -lz


tradx: Tradx
	LD_LIBRARY_PATH=${LIBDIRB}:${LIBDIRD} \
	time ./Tradx \
	  -bugs 5 \
	  -func noneSuch \
	  -inFile /d1/steves/tda/tdswp/swp.1080914234702.TA-ELDR.239.-16.5_AIR_v1.QC \
	  -bugiz 10 \
	  -bugiy 17 \
	  -bugix 29


#	  -inFile /d1/steves/tda/tdswp/swp.1080914234702.TA-ELDR.239.-16.5_AIR_v1.QC \
#	  -inFile /d1/steves/data/radar/miller/tda/all/swp.1050827184700.TF-ELDR.655.15.6_AIR_v-999 \

#	  -inFile /d1/steves/data/radar/patrick.kennedy/CHL20080702_231005.uf \
#	  -inFile /d1/steves/data/radar/patrick.kennedy/CHL20080702_231451.uf \
#	  -inFile /d1/steves/data/radar/patrick.kennedy/PAW20080702_230958.uf \
#	  -inFile /d1/steves/data/radar/patrick.kennedy/PAW20080702_231458.uf \





brodzik:
	LD_LIBRARY_PATH=${LIBDIRB}:${LIBDIRD} ./RadarWind \
	  -bugs 10 \
	  -nx 10 \
	  -ny 10 \
	  -nz 10 \
	  -func noneSuch \
	  -inDir /d1/steves/data/radar/miller/tda \
	  -fileRegex '^swp.1050827185456.TA' \
	  -outFile tempb.nc
olda:
	g++ -g -c -I${INCDIRA} -I${INCDIRB} RadarWind.cc

	g++ -g -o RadarWind RadarWind.o -L${LIBDIRA} -L${LIBDIRB} -lRadx -ltdrp -lnetcdf -lhdf5_hl -lhdf5 -lz -lGeographic

	LD_LIBRARY_PATH=${LIBDIRB} ./RadarWind \
	  -inDir tda \
	  -fileRegex '^swp' \
	  -outFile tempb.nc


oldb:
	g++ -g -c -I${INCDIRB} -I${INCDIRC} -I${INCDIRD} RadarWind.cc
	g++ -g -o RadarWind *.o \
	  -L${LIBDIRB} -lGeographic \
	  -L/rap/dev/lib  -lRadx \
	  -L/usr/local/netcdf4/lib  \
	  -lnetcdf -lhdf5_hl -lhdf5 -lz



testnca: ${SRC}
	g++ -g -c -I${INCDIRD} testnca.cc

	g++ -g -o testnca testnca.o \
	  -L/d1/steves/cvs/lib -lRadx \
	  -L/usr/local/netcdf4/lib  \
	  -lnetcdf -lhdf5_hl -lhdf5 -lz



test:
	LD_LIBRARY_PATH=${LIBDIRB} ./RadxTesta -f tempa.swp


