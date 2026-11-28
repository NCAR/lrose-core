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
//
// Simple program to move certain distances/directions from a central point.
// Niles Oien June 2005.
//
#include <iostream>
#include <toolsa/pjg_flat.h>
#include "Params.hh"

using namespace std;


int main( int argc, char *argv[] )
{

  
  Params P;

  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  //
  // Loop through directions and distances.
  //
  for (int id=0; id < P.directions_n; id++){
    cerr << "Direction : " << P._directions[id] << endl;

    for (int idist=0; idist < P.distances_n; idist++){
      double lat,lon;
      PJGLatLonPlusRTheta(P.centralLat, P.centralLon,
			  P._distances[idist], P._directions[id],
			  &lat, &lon);
      cerr << "    " << "Distance : " << P._distances[idist] << " : ";
      cerr << lat << ", " << lon << endl;
    }
    cerr << endl;
  }



   return 0;

}











