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
/////////////////////////////////////////////////
// Grib Projection
//
/////////////////////////////////////////////////

#include <string>

#include <euclid/PjgGrib.hh>
using namespace std;

  /**********************************************************************
   * Constructors
   */

PjgGrib::PjgGrib() : 
  Pjg()
{
}

/**********************************************************************
 * Destructor
 */

PjgGrib::~PjgGrib()
{
  // Do nothing
}

/**********************************************************************
 * init() - Initialize the projection.
 */

void PjgGrib::init(const double origin_lat, const double origin_lon,
		   const double latin,
		   const int nx, const int ny, const int nz,
		   const double dx, const double dy, const double dz,
		   const double minx, const double miny, const double minz,
		   const data_ordering_t data_order, const grid_orientation_t grid_orient)
{

  initLc1(origin_lat, origin_lon,
	  latin,
	  nx, ny, nz,
	  dx, dy, dz,
	  minx, miny, minz);
  
  _dataOrder = data_order;

  _gridOrientation = grid_orient;

}


/**********************************************************************
 * print() - Print projection information.
 */

void PjgGrib::print(void)
{

  string gridOrientStr;
  switch (_gridOrientation) {
  case GO_OTHER:
    gridOrientStr = "OTHER";
    break;
  case GO_SN_WE:
    gridOrientStr = "SN_WE";
    break;
  case GO_NS_WE:
    gridOrientStr = "NS_WE";
    break;
  case GO_SN_EW:
    gridOrientStr = "SN_EW";
    break;
  case GO_NS_EW:
    gridOrientStr = "NS_EW";
    break;
  default:
   gridOrientStr  = "unknown";
  }

  string dataOrderStr;
  switch (_dataOrder) {
  case DO_XYZ:
    dataOrderStr = "XYZ";
    break;
  case DO_YXZ:
    dataOrderStr = "YXZ";
    break;
  case DO_XZY:
    dataOrderStr = "XZY";
    break;
  case DO_YZX:
    dataOrderStr = "YZX";
    break;
  case DO_ZXY:
    dataOrderStr = "ZXY";
    break;
  case DO_ZYX:
    dataOrderStr = "ZYX";
    break;
  default:
    dataOrderStr = "unknown";
  }

  cout << "PjgGrib::info --" << endl;
  cout << "\torigin latitude: " << getOriginLat() << endl;
  cout << "\torigin longitude: " << getOriginLon() << endl;
  cout << "\tstandard parallel: " << getStandardParallel() << endl;
  cout << "\tdata order: " << dataOrderStr << endl;
  cout << "\tgrid orientation: " << gridOrientStr << endl;
  cout << "\tnx: " << Pjg::getNx() << endl;
  cout << "\tny: " << Pjg::getNy() << endl;
  cout << "\tnz: " << Pjg::getNz() << endl;
  cout << "\tdx: " << Pjg::getDx() << endl;
  cout << "\tdy: " << Pjg::getDy() << endl;
  cout << "\tdz: " << Pjg::getDz() << endl;
  cout << "\tmin_x: " << Pjg::getMinx() << endl;
  cout << "\tmin_y: " << Pjg::getMiny() << endl;
  cout << "\tmin_z: " << Pjg::getMinz() << endl;
}
