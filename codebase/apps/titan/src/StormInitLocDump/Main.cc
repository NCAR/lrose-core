// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

#include <iostream>
#include <stdio.h>
#include <cstring>
#include <Spdb/DsSpdb.hh>
#include <rapformats/GenPt.hh>
#include <toolsa/umisc.h>

int main(int argc, char *argv[]){


  bool gotStart=false, gotEnd=false, gotUrl = false;
  time_t start=0, end=0;
  char *url=NULL;

  date_time_t startTime, endTime;

  for (int i=1; i < argc; i++){
 
    if (!(strcmp(argv[i],"-url"))){
      i++;
      if (i < argc) {
	url = argv[i];
	gotUrl = true;
      } else {
	cerr << "-url needs argument." << endl;
	exit(-1);
      }
    }

    if (!(strcmp(argv[i],"-start"))){
      i++;
      if (i < argc){
        if (6==sscanf(argv[i],"%d %d %d %d %d %d",
                      &startTime.year, &startTime.month, &startTime.day,
                      &startTime.hour, &startTime.min, &startTime.sec)){
          gotStart = true;
          uconvert_to_utime( &startTime );
          start = startTime.unix_time;
        }
      }
    }

   if (!(strcmp(argv[i],"-end"))){
      i++;
      if (i < argc){
        if (6==sscanf(argv[i],"%d %d %d %d %d %d",
                      &endTime.year, &endTime.month, &endTime.day,
                      &endTime.hour, &endTime.min, &endTime.sec)){
          gotEnd = true;
          uconvert_to_utime( &endTime );
          end = endTime.unix_time;
        }
      }
    }
  }

  if (!(gotStart && gotEnd && gotUrl)){
    cerr << "Specify start and end times." << endl;
    cerr << "use -start -end and -url" << endl;
    exit(-1);
  }
  

   DsSpdb In;

   if (In.getInterval(url, start, end)){
     cerr << "get interval failed." << endl;
     cerr << "URL : " << url << endl;
     cerr << "Start : " << utimstr(start) << endl;
     cerr << "End : " << utimstr(end) << endl;
     exit(-1);
   }

   int numPoints = In.getNChunks();

   cerr << numPoints << " points found." << endl;

   for (int ip=0; ip < numPoints; ip++){

    GenPt G;
    if (0 != G.disassemble(In.getChunks()[ip].data,
                           In.getChunks()[ip].len)){
      cerr << "GenPt dissassembly failed for point " << ip << endl;
      exit(-1);
    }


    cerr << "Point " << ip+1 << " : ";

    double lat = G.getLat();
    double lon = G.getLon();
    time_t dataTime = G.getTime();

    if (
	(lat == -90.0) &&
	(lon == 0)
	){
      cerr << "  Test point, has no physical validity." << endl;
    } else {

      cerr << "  (" << lat << ", " << lon << ") " << utimstr(dataTime);

      int simpleNum = -1;
      int fn = G.getFieldNum("simpleTrackNumber");
      if (fn != -1){
	simpleNum =(int)rint(G.get1DVal(fn));
      }

      int complexNum = -1;
      fn = G.getFieldNum("complexTrackNumber");
      if (fn != -1){
	complexNum = (int)rint(G.get1DVal(fn));
      }

      cerr << " complex " << complexNum << " simple " << simpleNum << endl;


      fn = G.getFieldNum("saveTime");
      if (fn != -1){
	double saveTime_d = G.get1DVal(fn);
	time_t saveTime = (time_t) saveTime_d;
	int delay = (int)(saveTime - dataTime);
	
	int delayMin = (int)floor(double(delay)/60.0);
	int delaySecs = delay - delayMin*60;
	
	if (delayMin < 1440){
	  cerr << "  Data saved at " << utimstr(saveTime);
	  cerr << " delay " << delay;
	  cerr << " (" << delayMin << " minutes ";
	  cerr << delaySecs << " seconds)" << endl;
	} else {
	  cerr << "  Data saved in archive mode." << endl;
	}
      }
    }
   }

  return 0;

}
