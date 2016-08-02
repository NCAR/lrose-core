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
#include "DbzGrid.hh"

//
// Constructor
//
DbzGrid::DbzGrid()
{
}

//
// Destructor
//
DbzGrid::~DbzGrid()
{



}

//
// Clear pointers, mdv object
//
void DbzGrid::clear()
{
   _dbzField = NULL;
   
   _dbz = NULL;

   _mdv.clearRead();
}


//
// Read the dbz data for given request time
//
int DbzGrid::init(Params &params, time_t requestTime)
{
  _params = params;

  //
  // Set up for reading mdv data, reinitialize DsMdvx object
  //
  _mdv.clearRead();
  
  _mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);

  _mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  _mdv.setReadScalingType(Mdvx::SCALING_NONE);

  _mdv.setDebug(params.debug);

  _mdv.setReadTime(Mdvx::READ_CLOSEST, params.dbz_url, 300, requestTime);

  _mdv.addReadField(params.dbz_field_name);
  
  if (params.debug >= Params::DEBUG_VERBOSE) 
    {
      cerr << "AdvectPolygon::readMdv() : Reading data for URL: " << params.dbz_url << endl;
      _mdv.printReadRequest(cerr);
    }

  //
  // perform the read
  //
  if (_mdv.readVolume()) 
    {
      cerr << "DbzGrid::readMdv(): ERROR: Cannot read data for url: " << params.dbz_url << endl;
      cerr << "  " << _mdv.getErrStr() << endl;
      return 1;
    }
  
  //
  // Get the relevant data
  //
  _dbzField = _mdv.getField(params.dbz_field_name);
  
  const Mdvx::master_header_t &mhdr = _mdv.getMasterHeader();

  const Mdvx::field_header_t &dbzHdr  =  _dbzField->getFieldHeader();

  _dbzMissing = dbzHdr.missing_data_value;

  _dbzBad = dbzHdr.bad_data_value;

  _dbz = (float*) _dbzField->getVol();

  _dataTime =  mhdr.time_end;

  _proj.init( mhdr, dbzHdr);

  return 0;
}

//
// Calculate average rain rate from dbz field within the polygon passed as an arg.
//
float DbzGrid::calculateRainRate( GenPoly *polygon)
{
 
  //
  // Record vertices of poygon in array of Point_d structs
  // so that we can pass data to euclid/lib function EG_inside_poly
  //
  int numVertices = polygon->getNumVertices();

  Point_d lonsLats[numVertices];
      
  for (int j = 0; j < numVertices; j++)
    {
      GenPoly::vertex_t vertex = polygon->getVertex(j);
      
      lonsLats[j].x = vertex.lon;
      
      lonsLats[j].y = vertex.lat;
    }
  
  int nx = _proj.getNx();
  
  int ny = _proj.getNy();

  //
  // should be smarter about the search, but just for starters
  //
  int count = 0;

  double rainRateAve = 0;

  for (int j = 0; j < ny -1; j++)
    {
      for (int i = 0; i < nx -1; i++)
	{
	  Point_d pt;
	  
	  _proj.xyIndex2latlon( i, j, pt.y, pt.x);
	
	  int arrayIndex = _proj.xyIndex2arrayIndex(i, j);
	  
	  if ( EG_inside_poly(&pt, lonsLats, numVertices) &&
	       (_dbz[arrayIndex] != _dbzBad) && (_dbz[arrayIndex] != _dbzMissing)) 
	    {
	      double z = pow(10, _dbz[arrayIndex]/10);
   
	      rainRateAve += pow(_params.zr.coefficient, -1/_params.zr.exponent) * pow(z, 1/_params.zr.exponent);

	      count++;
	    }
	}
      
    }

  if (count != 0)
    rainRateAve = rainRateAve/count;
  else
    rainRateAve = -1;
  
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "DbzGrid::calculateRainRate: Average rain rate for polygon: " <<  rainRateAve << endl;

  return rainRateAve;
}
