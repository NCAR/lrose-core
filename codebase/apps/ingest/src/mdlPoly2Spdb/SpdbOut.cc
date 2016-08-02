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

#include "SpdbOut.hh"

#include <rapformats/GenPoly.hh>
#include <Spdb/DsSpdb.hh>

using namespace std;

// constructor. Does nothing.
 
SpdbOut::SpdbOut(){
  return;  
}
 
// destructor. Does nothing.
 
SpdbOut::~SpdbOut(){
  return;
}
    
// public method.
 
void SpdbOut::WriteOut(Params *P,
		       int num,
		       double *lat,
		       double *lon,
		       time_t dataTime,
		       int id,
		       double weight){


 

  if (P->debug){
    fprintf(stderr,"For ID %d the points are :\n", id);
    for (int i=0; i < num; i++){
      fprintf(stderr,
	      "%g\t%g\n",lat[i],lon[i]);
    }
  }

  
  // create the object.
  DsSpdb D;
  D.clearPutChunks();
  D.clearUrls();
  D.addUrl( P->OutUrl );


  GenPoly G;

  G.setId( id );
  G.setTime( dataTime );
  G.setExpireTime( dataTime + P->expiry );
  G.setName( "mdlPoly2Spdb" );
  G.setNLevels(1);
  G.setClosedFlag( true );
  G.setFieldInfo("interest:no_units");
  G.addVal( weight );

  GenPoly::vertex_t v;
  for (int i=0; i < num; i++){
    v.lat = lat[i]; v.lon = lon[i]; G.addVertex( v );
  }

  if (!(G.assemble())){
    fprintf(stderr,"Assemble failed on Genpoly!\n");
    fprintf(stderr,"%s\n", G.getErrStr().c_str());
    exit(-1); // Unlikely.
  }

  D.addPutChunk( id, dataTime,
                dataTime + P->expiry, G.getBufLen(),
                G.getBufPtr() ); 
  
 
  if (D.put(SPDB_GENERIC_POLYLINE_ID, SPDB_GENERIC_POLYLINE_LABEL) != 0){
    fprintf(stderr,   "Error writing chunk.\n");
  }
  
  
  return;
}


