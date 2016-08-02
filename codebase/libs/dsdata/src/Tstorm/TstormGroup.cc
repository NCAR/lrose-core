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
///////////////////////////////////////////////////////////////
// TstormGroup class
//   Contains information on a group of storms
//
// $Id: TstormGroup.cc,v 1.9 2016/03/03 18:06:33 dixon Exp $
//////////////////////////////////////////////////////////////

#include <cmath>
#include <map>

#include <dataport/port_types.h>
#include <dsdata/TstormGroup.hh>
#include <dsdata/Tstorm.hh>
using namespace std;



TstormGroup::TstormGroup(const bool debug) :
  _debug(debug),
  nSides(0),
  dataTime(DateTime::NEVER),
  expireTime(DateTime::NEVER),
  dbzThreshold(0.0),
  startAz(0.0),
  deltaAz(0.0)
{
  // Do nothing
}

TstormGroup::TstormGroup(const int n_sides,
			 const time_t data_time,
			 const float dbz_threshold,
			 const float start_azimuth,
			 const float delta_azimuth,
			 const TstormGrid &tstorm_grid,
			 const bool debug) :
  _debug(debug),
  nSides(n_sides),
  dataTime(data_time),
  expireTime(data_time + 900),
  dbzThreshold(dbz_threshold),
  startAz(start_azimuth),
  deltaAz(delta_azimuth),
  grid(tstorm_grid)
{
  // Do nothing
}

TstormGroup::TstormGroup(const char* groupPtr, const bool swap_data,
			 const bool debug) :
  _debug(debug)
{
   setData( groupPtr, swap_data );
}

TstormGroup::~TstormGroup() 
{
   clearData();
}

void
TstormGroup::clearData()
{
   vector< Tstorm* >::iterator stormIt;
   
   for( stormIt = tstorms.begin(); stormIt != tstorms.end(); stormIt++ ) {
      delete ( *stormIt );
   }
   tstorms.erase( tstorms.begin(), tstorms.end() );
}

void
TstormGroup::setData(const char* groupPtr, const bool swap_data ) 
{
   //
   // Clear any existing data
   //
   clearData();
   
   //
   // Swap the data buffer, if requested
   //
   if (swap_data)
     tstorm_spdb_buffer_from_BE((ui08 *)groupPtr);
   
   //
   // Set data members
   //
   tstorm_spdb_header_t *header = (tstorm_spdb_header_t *) groupPtr;
   
   nSides       = header->n_poly_sides;
   dataTime     = header->time;
   expireTime   = header->time;
   dbzThreshold = header->low_dbz_threshold;
   startAz      = header->poly_start_az;
   deltaAz      = header->poly_delta_az;
   grid.set( &(header->grid) );

   //
   // Process each storm
   //
   for( int iStorm = 0; iStorm < header->n_entries; iStorm++ ) {

      //
      // Get a pointer to the storm entry
      //
      tstorm_spdb_entry_t *currentStorm = 
	 (tstorm_spdb_entry_t *) ( groupPtr +
				   sizeof(tstorm_spdb_header_t) +
				   (iStorm * sizeof(tstorm_spdb_entry_t)));

      //
      // Set up the storm object
      //
      Tstorm *tstormEntry = new Tstorm( currentStorm, nSides,
                                        grid, startAz, deltaAz,
                                        dataTime );
      tstorms.push_back( tstormEntry );
   }
}


void TstormGroup::addTstorm(Tstorm *new_storm) 
{
  if(remapNewStorm(new_storm)) {
    tstorms.push_back(new_storm);
  }
}


void TstormGroup::sortByAreaDescending()
{
  multimap< double, Tstorm* > sorted_list;
  
  vector< Tstorm* >::iterator tstorm_iter;
  
  for (tstorm_iter = tstorms.begin();
       tstorm_iter != tstorms.end();
       ++tstorm_iter)
  {
    Tstorm *tstorm = *tstorm_iter;
    
    sorted_list.insert(pair< double, Tstorm* >(tstorm->getArea(), tstorm));
  }
  
  tstorms.erase(tstorms.begin(), tstorms.end());
  
  multimap< double, Tstorm* >::reverse_iterator map_iter;
  
  for (map_iter = sorted_list.rbegin();
       map_iter != sorted_list.rend();
       ++map_iter)
    tstorms.push_back(map_iter->second);
  
}


int TstormGroup::getSpdbNumBytes() const
{
  return sizeof(tstorm_spdb_header_t) +
    (tstorms.size() * sizeof(tstorm_spdb_entry_t));
}


void TstormGroup::writeSpdb(ui08 *buffer) const
{
  ui08 *buffer_ptr = buffer;
  
  // Write the header

  tstorm_spdb_header_t *hdr_ptr = (tstorm_spdb_header_t *)buffer_ptr;
  
  memset(hdr_ptr, 0, sizeof(tstorm_spdb_header_t));
  
  hdr_ptr->time = dataTime;
  grid.setGridValues(hdr_ptr->grid);
  hdr_ptr->low_dbz_threshold = dbzThreshold;
  hdr_ptr->n_entries = tstorms.size();
  hdr_ptr->n_poly_sides = nSides;
  hdr_ptr->poly_start_az = startAz;
  hdr_ptr->poly_delta_az = deltaAz;
  
  // Write each storm entry

  buffer_ptr += sizeof(tstorm_spdb_header_t);
  
  vector< Tstorm* >::const_iterator tstorm_iter;
  
  for (tstorm_iter = tstorms.begin(); tstorm_iter != tstorms.end();
       ++tstorm_iter)
  {
    (*tstorm_iter)->setEntryValues(*(tstorm_spdb_entry_t *)buffer_ptr);
    
    buffer_ptr += sizeof(tstorm_spdb_entry_t);
  } /* endfor - tstorm_iter */
  
  // Byte swap the buffer

  tstorm_spdb_buffer_to_BE(buffer);
  
}

bool TstormGroup::remapNewStorm(Tstorm *new_storm) 
{
  // check map projection
  TstormGrid& new_storm_grid = new_storm->getGrid();
  if (_debug)
    new_storm_grid.print(cerr, "new_storm_grid  ");
  if(grid.getProjType() != new_storm_grid.getProjType()) {
    cerr << "Unable to remap this storm because map projections differ." << endl;
    return false;
  }

  // check dx and dy
  if((fabs(grid.getDx() - new_storm_grid.getDx()) < 0.0001) ||
     (fabs(grid.getDy() - new_storm_grid.getDy()) < 0.0001)) {
    return true;
  }

  // alter radials based new dx and dy
  double dr = hypot(grid.getDx(), grid.getDy());
  double new_storm_dr = hypot(new_storm_grid.getDx(), new_storm_grid.getDy());
  double scale_factor = new_storm_dr / dr;
  //  cerr << "grid.getDx(), grid.getDy()" << grid.getDx() << " " << grid.getDy() << endl;
  //  cerr << "dr = " << dr << endl;
  //  cerr << "new_storm_grid.getDx(), new_storm_grid.getDy()" << 
  //   new_storm_grid.getDx() << " " << new_storm_grid.getDy() << endl;
  //  cerr << "new_storm_dr = " << new_storm_dr << endl;
  //  cerr << "scale_factor = " << scale_factor << endl;

  vector<double>& radials = new_storm->getRadials();
  double centroidLat, centroidLon;
  double centroidX, centroidY;
  float *xData;
  float *yData;
  
  int    nPoints      = radials.size();
  double theta        = new_storm->getStartAz() * DEG_TO_RAD;
  double dTheta       = new_storm->getDeltaAz() * DEG_TO_RAD;
  
  xData = new float[nPoints];
  yData = new float[nPoints];
  
  new_storm->getCentroid(&centroidLat, &centroidLon);
  grid.latlon2xy(centroidLat, centroidLon, 
		 centroidX, centroidY );
  //  cerr << "centroidX  = " << centroidX;
  //  cerr << "   centroidY  = " << centroidY << endl;
  
  for( int i = 0; i < nPoints; i++ )
  {

    // this is almost correct -- size is correct, but rotated or under shear.
    xData[i] = centroidX + scale_factor * radials[i] * sin(theta) * new_storm_grid.getDx();
    yData[i] = centroidY + scale_factor * radials[i] * cos(theta) * new_storm_grid.getDy();

    // this is almost correct -- a little large, but oriented correctly
    // xData[i] = centroidX +  (dr/grid.getDx()) * radials[i] * sin(theta) * (scale_factor*new_storm_grid.getDx());
    // yData[i] = centroidY +  (dr/grid.getDy()) * radials[i] * cos(theta) * (scale_factor*new_storm_grid.getDy());

    // this is almost correct -- a little small, but oriented correctly
    //    xData[i] = centroidX + (dr/grid.getDx()) * radials[i] * sin(theta) * new_storm_grid.getDx();
    //    yData[i] = centroidY + (dr/grid.getDy()) * radials[i] * cos(theta) * new_storm_grid.getDy();
    
    //cerr << "xData[i]  = " << xData[i];
    //cerr << "  yData[i]  = " << yData[i];
    //cerr << "  radial = " << radials[i];
    radials[i] = hypot((centroidX-xData[i]), (centroidY-yData[i]));
    //cerr << "  new radial = " << radials[i] << endl;

    theta += dTheta;
  }

  Polyline *new_storm_poly = new Polyline( grid.getOriginLat(), 
					   grid.getOriginLon(), 0.0,
					   nPoints, xData, yData, centroidX,
					   centroidY, dataTime,
					   Polyline::CLOSED );
	  
   
  new_storm->setDetectionPoly(*new_storm_poly);
  new_storm->findMaxRadial();
  new_storm->findLeadingEdge();
  delete[] xData;
  delete[] yData;
  delete new_storm_poly;

  
  return true;
}




