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
#include "WindGrid.hh"

//
// Constructor
//
WindGrid::WindGrid()
{
}

//
// Destructor
//
WindGrid::~WindGrid()
{



}

//
// clear(): reset mdv, set pointers to data to NULL
//
void WindGrid::clear()
{
  _ufield = NULL;

  _vfield = NULL;

  _u = NULL;

  _v = NULL;

  _mdv.clearRead();
}

//
// Read mdv wind data for given request time
//
int WindGrid::init(Params &params, time_t requestTime)
{
  _params = params;

  //
  // Set up for reading mdv data, reinitialize DsMdvx object
  //
  _mdv.clearRead();

  _mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);

  _mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  _mdv.setReadScalingType(Mdvx::SCALING_NONE);

  _mdv.setDebug(_params.debug);

  _mdv.setReadTime(Mdvx::READ_CLOSEST, _params.wind_url, 0, requestTime);

  _mdv.addReadField(_params.u_field_name);

  _mdv.addReadField(_params.v_field_name);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) 
    {
      cerr << "WindGrid::init() : Reading data for URL: " << _params.wind_url << endl;
      _mdv.printReadRequest(cerr);
    }

  //
  // perform the read
  //
  if (_mdv.readVolume()) 
    {
      cerr << "WindGrid::init(): ERROR: Cannot read data for url: " << _params.wind_url << endl;
      cerr << "  " << _mdv.getErrStr() << endl;
      return 1;
    }
  
  //
  // Get the relevant data
  //
  _ufield = _mdv.getField(_params.u_field_name);
  
  _vfield = _mdv.getField(_params.v_field_name);
  
  const Mdvx::master_header_t &mhdr = _mdv.getMasterHeader();

  const Mdvx::field_header_t &uHdr  =  _ufield->getFieldHeader();

  const Mdvx::field_header_t &vHdr  =  _vfield->getFieldHeader();

  _uMissing = uHdr.missing_data_value;

  _uBad = uHdr.bad_data_value;

  _vMissing = vHdr.missing_data_value;

  _vBad = vHdr.bad_data_value;

  _u = (float*) _ufield->getVol();

  _v = (float*) _vfield->getVol();

  _dataTime =  mhdr.time_end;

  _proj.init( mhdr, uHdr);

  
  return 0;
}

//
// AdvectPoint: Take point with time associated to it and advect it to
//              the proper position relative to _dataTime and the u and v 
//              wind components.
//
int WindGrid::advectPoint(float initialLat, float initialLon, float &newLat, 
			  float &newLon, time_t pointTime)
{  
  //
  // We find ave U and ave V for all grid pts within _params.radius of the point
  // 
  // First find coordinates of the point
  //
  int gridX,gridY;

  _proj.latlon2xyIndex((double)initialLat, (double)initialLon, gridX, gridY);

  if (_params.debug == Params::DEBUG_VERBOSE)
      cerr << "WindGrid::_advectPoint(): gridX, gridY: " << gridX << " " << gridY << endl;
  
  //
  // Convert the radius of interest to grid units
  //
  double gridRadius = _proj.km2x(_params.radius);

  //
  // Create bounding box in which to search for average u and average v 
  //
  double minX = floor(gridX - gridRadius);
  double maxX = ceil(gridX + gridRadius);
  double minY = floor(gridY - gridRadius);
  double maxY = ceil(gridY + gridRadius);

  if ( minX < 0)
    minX = 0;

  if (maxX > _proj.getNx() - 1)
    maxX = _proj.getNx() - 1;

  if (minY < 0)
    minY = 0;

  if (maxY > _proj.getNy() - 1)
    maxY = _proj.getNy() - 1;

  if (_params.debug == Params::DEBUG_VERBOSE)
    {
      cerr << "WindGrid::_advectPoint(): minX, maxX " <<  minX << " " << maxX << endl;

      cerr << "WindGrid::_advectPoint(): minY, maxY " <<  minY << " " << maxY << endl; 
    }

  //
  // Find the average u and the average v within a radius of gridRadius of 
  // (gridX, gridY).
  //
  float aveU = 0;

  float aveV = 0;

  int count = 0;

  for(int j = (int)minY; j <= (int)maxY; j++)
    {
      for (int i = (int)minX; i <= (int)maxX; i++)
	{
	  //
	  // if the distance from (i.j) to (gridX, gridY) is less than gridRadius
	  // the correponding u and v will go into the average
	  //
	  if ( sqrt((double)((i - gridX)*(i - gridX) + (j - gridY)*(j - gridY))) < gridRadius)
	    {
	      
	      int arrayIndex = _proj.xyIndex2arrayIndex(i,j);
	      
	      if (    _u[arrayIndex] == _uBad || _u[arrayIndex] == _uMissing || 
		      _v[arrayIndex] == _vBad || _v[arrayIndex] == _vMissing)
		{		
		  continue;
		}
	      else
		{
		  aveU = aveU + _u[arrayIndex];
		  
		  aveV = aveV  + _v[arrayIndex];
		  
		  count++;
		}
	      
	    }
	}
    }
  
  //
  // If data is missing or bad, polygon is not advected
  //
  if (count == 0)
    {
      cerr << "WindGrid::_advectPoint(): WARNING: All u.v data around point are missing or bad. " 
	   << "Point not advected. " << endl;

      newLat = initialLat;

      newLon = initialLon;
      
      return 0;
    }

  aveU = aveU/count;
  
  aveV = aveV/count;

  if (_params.debug == Params::DEBUG_VERBOSE)
    {
      cerr << "WindGrid::_advectPoint(): aveU, aveV " <<  aveU << " " << aveV << endl;
    }

  //
  // Find speed (m/s)
  //
  double speed = sqrt(aveU * aveU + aveV * aveV);

  //
  // Find distance in km to advect 
  //
  double advectDistance = (_dataTime - pointTime) * speed / 1000;

  //
  // Find direction of where the wind is from(this will give us an angle between 0,360) 
  // 
  //
  double theta = PHYwind_dir(aveU,aveV);
  
  //
  // where wind is to...
  // 
  if (theta > 180)
    theta = theta - 180;
  else
    theta = theta + 180;

  //
  // Create angle between [-180, 180]
  //
  if (theta > 180 )
    theta = -360 + theta;

  //
  // Advect the point
  //
  _proj.latlonPlusRTheta(initialLat, initialLon, advectDistance, theta, newLat,  newLon);


  if (_params.debug == Params::DEBUG_VERBOSE)
    {
      cerr << "WindGrid::_advectPoint(): deltaT, speed, distance, theta " 
	   <<  _dataTime - pointTime << " " << speed << " " << advectDistance 
	   <<  " " << theta << endl;

      cerr << "WindGrid::_advectPoint(): newLat,  newLon " 
	   <<  newLat << " " <<  newLon << endl;
    }

  return 0;
}


