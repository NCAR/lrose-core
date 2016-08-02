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
////////////////////////////////////////////////////////////////////////
// Shape2Map.cc
//
// Shape2Map object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2011
//
///////////////////////////////////////////////////////////////////////
//
// Shape2Map reads an ESRI shape file, and converts this to a
// CIDD-style ASCII map file.
//
// The ASCII map output is written to stdout.
//
///////////////////////////////////////////////////////////////////////

#include <vector>
#include <iomanip>
#include <cstdio>
#include <toolsa/pmu.h>
#include <toolsa/ucopyright.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/DateTime.hh>
#include "Shape2Map.hh"
using namespace std;

// Constructor

Shape2Map::Shape2Map(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Shape2Map";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Shape2Map::~Shape2Map()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Shape2Map::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  int iret = _convertShapeFile(_params.shape_file_path);

  return iret;

}

/************************************************************************
 * Convert the shape file
 */

int Shape2Map::_convertShapeFile(const char *shapePath)
{

  SHPHandle SH;
  if((SH = SHPOpen(shapePath, "rb")) == NULL) {
    fprintf(stdout,"Problem reading shapefile: %s\n", shapePath);
    return -1;
  }
  // Shape Files Found and Open
  
  int n_objects;
  int shape_type;
  SHPGetInfo(SH, &n_objects, &shape_type, NULL, NULL);

  if (_params.debug) {
    fprintf(stderr, "Found %d objects, type %d\n",
            n_objects, shape_type);
    printf("# Conversion of %s -  %d objects, type %d\n#\n",
           shapePath, n_objects, shape_type);
  }

  // loop through the objects

  vector<double> iconLats, iconLons;
  
  for(int ii = 0; ii < n_objects; ii++) {  // Loop through each object
    
    SHPObject *SO = SHPReadObject(SH, ii);    // Load the shape object
    
    switch(SO->nSHPType) {
      
      case SHPT_POLYGON:  // Polyline
      case SHPT_ARC:
      case SHPT_ARCM:
      case SHPT_ARCZ:
        _handlePolyline(SO);
        break;
        
      case SHPT_POINT :  // Icon Instance
      case SHPT_POINTZ:
      case SHPT_POINTM: {
        double lat = SO->padfY[0];
        double lon = SO->padfX[0];
        iconLats.push_back(lat);
        iconLons.push_back(lon);
      } break;
        
      case SHPT_NULL:
      case SHPT_MULTIPOINT:
      case SHPT_POLYGONZ:
      case SHPT_MULTIPOINTZ:
      case SHPT_POLYGONM:
      case SHPT_MULTIPOINTM:
      case SHPT_MULTIPATCH:
      default:
        if (_params.debug) {
          cerr << "WARNING - Shape2Map" << endl;
          cerr << "  Encountered Unsupported Shape type: "
               << SO->nSHPType << endl;
          cerr << "  File: " << shapePath << endl;
        }
    } // switch
    
    if(SO != NULL) SHPDestroyObject(SO);

  }  // End of each object

  // any icons?

  if (iconLats.size() > 0) {

    fprintf(stdout,
            "\nICONDEF CROSS 6\n 0 -5\n 0 5\n 32767 32767\n"
            " -5 0\n 5 0\n 32767 32767\n #\n");

    for (size_t ii = 0; ii < iconLats.size(); ii++) {
      fprintf(stdout,
              "ICON CROSS %10.6f %10.6f 32767 32767 nl\n",
              iconLats[ii], iconLons[ii]);
    }

  }

  return 0;

}

////////////////////////////////////////////
// handle polyline object

  
void Shape2Map::_handlePolyline(SHPObject *SO)

{

  vector<double> lats, lons;
  double prevLat = 0.0;
  double prevLon = 0.0;
  int part_num = 1;

  for(int ii = 0; ii < SO->nVertices; ii++) {
    double lat = SO->padfY[ii];
    double lon = SO->padfX[ii];
    if(SO->panPartStart[part_num] == ii) { // Insert a pen up at each end part index
        lats.push_back(-9999);
        lons.push_back(-9999);
        part_num++;
    }
    if (_params.check_max_distance_between_points && ii > 0) {
      double r, theta;
      PJGLatLon2RTheta(lat, lon, prevLat, prevLon, &r, &theta);
      if (r > _params.max_distance_between_points) {
        lats.push_back(-9999);
        lons.push_back(-9999);
      }
    }
    prevLat = lat;
    prevLon = lon;
    lats.push_back(lat);
    lons.push_back(lon);
  }

  fprintf(stdout,
          "\nPOLYLINE Shape %d\n", (int) lats.size());

  for(size_t ii = 0; ii < lats.size(); ii++) {
    if (lats[ii] < -999) {
      fprintf(stdout, "  32767 32767\n");
    } else {
      fprintf(stdout, "  %10.6f %10.6f\n",
              lats[ii], lons[ii]);
    }
  }

}
