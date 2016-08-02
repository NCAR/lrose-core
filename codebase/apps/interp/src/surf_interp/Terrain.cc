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
/////////////////////////////////////////////////////////////
// Terrain.cc
//
// Terrain class - handles the terrain.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan  1999
//
///////////////////////////////////////////////////////////////
//
//

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>  

#include <toolsa/mem.h> // for umalloc,ufree

#include "Terrain.hh"
using namespace std;
//
// constructor
//
// Reads a LATLON MDV file which has terrain in it.
// If unssuccessful Nx and Ny are set to -1.
//
Terrain::Terrain(char *MDVFile){

  Nx=-1; Ny=-1; // Default - fail.

  mdv = new DsMdvx();

  mdv->setReadPath(MDVFile); 

  if (mdv->readVolume()){
    fprintf(stderr,"Failed to read terrain file %s\n",MDVFile);
    exit(-1);
  }


  Mdvx::master_header_t master_hdr = mdv->getMasterHeader();
 
  // Check that the MDV file is a file with
  // only one field, and that that field is on
  // a lat/lon grid. If not, fail.
  
  if ((master_hdr.user_data != Mdvx::PROJ_LATLON) ||
      (master_hdr.n_fields != 1) ||
      (master_hdr.max_nz > 1)){

    fprintf(stderr,"%s :\nNot a LATLON MDV file with 1 2D field.\n",
	    MDVFile);
    return;
  }

  MdvxField *field = mdv->getFieldByNum(0);

  Mdvx::field_header_t  fhdr = field->getFieldHeader();


  if (fhdr.field_code !=5){ // 5 is the field code for terrain in meters.
    fprintf(stderr,"%s :\nNot a terrain file.\n",
	    MDVFile);
    return;
  }

  field->convertType();

  // OK - pull what we want out of the MDV file and
  // then deallocate it.

  Nx = fhdr.nx;
  Ny = fhdr.ny;
  dx = fhdr.grid_dx;
  dy = fhdr.grid_dy;
  LLlat = fhdr.grid_miny;
  LLlon = fhdr.grid_minx;

  _Terrain = (float *)field->getVol();

  URlon = LLlon + Nx*dx;
  URlat = LLlat + Ny*dy;

  return;

}

////////////////////////////////////////////////////

// Allow the user to access the elevation data
// by lat/lon.

float Terrain::Elevation(float lat, float lon){

  OutsideGrid = 0;
  // return 0 if outside the grid.

  if ((lat > URlat+dy/2.0) ||
      (lat < LLlat-dy/2.0) ||
      (lon < LLlon-dx/2.0) ||
      (lon > URlon+dx/2.0)){

    OutsideGrid = 1;
    return 0.0;
  }

  int j = int((lat-LLlat)/dy);
  int i = int((lon-LLlon)/dx);

  // Correct roundoff as it may lead
  // to an access violation.

  if (i<0) i=0;        if (j<0) j=0;
  if (i>Nx-1) i=Nx-1;  if (j>Ny-1) j=Ny-1;
 
  // Return the indexed value.

  return _Terrain[i+Nx*j];


}

/////////////////////////////////////////////////////

// destructor
//
Terrain::~Terrain(){

  delete mdv;

}




