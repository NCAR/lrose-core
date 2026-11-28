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
#include <Spdb/SoundingGet.hh>
#include <toolsa/umisc.h>
using namespace std;

int main(int argc, char *argv[]){


  if (argc <3) {
    cerr << "Usage : " << argv[0] << " URL Lookback (Hours)    ";
    cerr << "Min alt (Km)    Max alt" << endl;
    exit(-1);
  }

  time_t now = time(NULL);

  SoundingGet sounding;
  sounding.init( argv[1],
		 (int) (atof(argv[2])*60.0 + 0.5),
                  atof(argv[3])*1000.0,
                  atof(argv[4])*1000.0,
                  0.0, 0.0 );

  //
  // Set the time on it.
  //
  int i =sounding.readSounding( now );



  cerr << "I got " << i << endl;

  return 0;

}
