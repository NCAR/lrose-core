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

#include <cstdio>
#include <iostream>

#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRadar.hh>
#include <Mdv/MdvxTimeStamp.hh>
#include <Mdv/DsMdvx.hh>   
#include <Mdv/DsMdvxTimes.hh> 

#include <signal.h>
using namespace std;

int main(int argc, char *argv[]){


  // Set up an DsMdvxTimes object.

  DsMdvxTimes D; 


  if (D.setRealtime("mdvp:://anlab1::wsmr/mdv/radarCart",
		    3600)){
    cerr << "Failed to set realtime mode." << endl;
    exit(-1);
  }      

  time_t t;
  while (D.getNext(t)==0){

    cout << "Doing time " << utimstr(t) << endl;

  }       

  return 0;

}


