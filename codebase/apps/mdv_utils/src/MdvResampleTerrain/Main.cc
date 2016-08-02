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

#include <toolsa/str.h>
#include <toolsa/port.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h> 

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <Mdv/DsMdvxInput.hh>

static void tidy_and_exit (int sig);
//
// Simple code to remap latlon projection terrain MDV
// files output by WorldTerrain onto a flat earth projection.
//
// Could be modified in the future to do lambert conformal I suppose.
//
// Niles Oien April 2003.
//
int main(int argc, char **argv)
{

  //
  // See if -h has been specified.
  //
  bool gotMinusH = false;
  for (int i=0; i < argc; i++){
    if (!(strcmp("-h",argv[i]))){
      gotMinusH = true;
    }
  }

  if ((gotMinusH) || (argc < 9)){
    cerr << "USAGE : MdvResampleTerrain inUrl latOrig lonOrig Dx Dy Nx Ny outUrl" << endl;
    cerr << "EXAMPLE : " << endl;
    cerr << "MdvResampleTerrain ./latLonTerrain.mdv 41.5 -107.2 1.0 1.0 500 500 ./flatEarthTerrain.mdv" << endl;
    cerr << endl;
    cerr << "The basic idea is that a latLon projection MDV file output by" << endl;
    cerr << "WorldTerrain is remapped to a flat earth projection. Niles Oien." << endl;
    exit(0);
  }

 
  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  //
  // Grab some stuff from the command line arguments.
  //

  char inUrl[1024], outUrl[1024];

  sprintf(inUrl, "%s", argv[1]);
  double latOrig = atof(argv[2]);
  double lonOrig = atof(argv[3]);

  double Dx = atof(argv[4]);
  double Dy = atof(argv[5]);

  int Nx = atoi(argv[6]);
  int Ny = atoi(argv[7]);
  sprintf(outUrl, "%s", argv[8]);

  double minx = -Dx*double(Nx)/2.0;
  double miny = -Dy*double(Ny)/2.0;

  //
  // Tell the user how we interpreted the command line.
  //
  cerr << "Converting " << inUrl << " to " << outUrl << endl;
  cerr << "Origin lat, lon : " << latOrig << ", " << lonOrig << endl;
  cerr << "Dx, Dy, Nx, Ny : " << Dx << ", " << Dy << ", " << Nx << ", " << Ny << endl;
  cerr << "Implied minx, miny : " << minx << ", " << miny << endl;

  DsMdvx mdvx;
  mdvx.setDebug( true );

  mdvx.clearRead();
  mdvx.clearWrite(); 
  //
  // Set up so that the remap happens on the read.
  //
  mdvx.setReadRemapFlat(Nx,
			Ny,
			minx,
			miny,
			Dx,
			Dy,
			latOrig,
			lonOrig,
			0.0);
			
  mdvx.setReadPath(inUrl);
  mdvx.printReadRequest(cerr);
  //
  // Read the volume. The remapping happens here so this is a bit slow.
  //
  if (mdvx.readVolume()) {
    cerr << "ERROR :" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  cerr << "Working on file: " << mdvx.getPathInUse() << endl;
  //
  // Get the terrain (assume it's field 0 - pretty safe).
  //
  MdvxField *InField;
  InField = mdvx.getFieldByNum( 0 ); 
  //
  // Write the data out.
  //
  if(mdvx.writeToPath( outUrl )) {
    cerr << "ERROR!" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  return 0;
    
}

static void tidy_and_exit (int sig)

{

  exit(sig);

}
