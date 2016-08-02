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
// MM5Print.cc
//
// MM5Print object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
///////////////////////////////////////////////////////////////

#include "MM5Print.hh"
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <mm5/MM5DataV2.hh>
#include <mm5/MM5DataV3.hh>
#include <cstdio>
#include <iomanip>

#include <Mdv/MdvxProj.hh>
#include <rapmath/math_macros.h>
using namespace std;

// Constructor

MM5Print::MM5Print(int argc, char **argv)

{

  OK = TRUE;

  // set programe name
  
  _progName = "MM5Print";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }

  // get TDRP params

  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv,
			    _args.override.list,
			    &_paramsPath)) {
    fprintf(stderr, "ERROR: %s\n", _progName.c_str());
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance, PROCMAP_REGISTER_INTERVAL);
  PMU_auto_register("In MM5Print constructor");

  return;

}

// destructor

MM5Print::~MM5Print()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MM5Print::Run ()
{

  PMU_auto_register("MM5Print::Run");

  int version;

  if (MM5Data::getVersion(_params.input_file_path, version)) {
    return -1;
  }

  MM5Data *inData;
  if (version == 2) {
    inData = new MM5DataV2(_progName, _params.input_file_path,
			   _params.debug, PMU_auto_register);
  } else if (version == 3) {
    inData = new MM5DataV3(_progName, _params.input_file_path,
			   _params.debug, PMU_auto_register);
  }
  
  if (_params.find_headers) {

    inData->findHdrRecords();

  } else {

    if (inData->OK) {

      while(inData->more()) {

	if (inData->read()) {
	  return -1;
	}
	
	if (_params.print_border_map) {
	  
	  _printBorderMap(inData);
	  
	} else if (_params.print_border_gis) {
	  
	  _printBorderGis(inData);
	  
	} else if (_params.print_grid_map) {
	  
	  _printGridMap(inData);
	  
	} else {
	  
	  cout << "Printing file path: " << _params.input_file_path << endl;
	  cout << "  MM5 data Version: " << version << endl;
	  cout << endl;
	  
	  inData->printHeaders(stdout);
	  if (_params.print_model_fields) {
	    inData->printModelFields(stdout, _params.print_fields_full);
	  }
	  if (_params.print_derived_fields) {
	    inData->computeDerivedFields();
	    inData->printDerivedFields(stdout, _params.print_fields_full);
	  }
	  
	  if (_params.check_projection) {
	    _printProjCheck(inData);
	  }
	  
	}

      } // while(inData->more())

    } // if (inData->OK)

  } // if (_params.find_headers)

  delete (inData);
    
  return (0);

}

void MM5Print::_printProjCheck(MM5Data *inData)
  
{
  MdvxProj proj;

  switch(inData->get_proj_type()) { 
     case MM5Data::LAMBERT_CONF :
         proj.initLc2(inData->center_lat, inData->center_lon, 
                      inData->true_lat1, inData->true_lat2);
     break;
     case MM5Data::STEREOGRAPHIC :
	proj.initPolarStereo(inData->center_lat,
                inData->center_lon,inData->center_lon,
		(inData->center_lat < 0.0 ? Mdvx::POLE_SOUTH : Mdvx::POLE_NORTH),
                 0.9328365);
     break;
     case MM5Data::MERCATOR :
	proj.initMercator(inData->center_lat, inData->center_lon);
     break;
  }


  cout << "PROJECTION CHECK" << endl;
  cout << "================" << endl << endl;

  cout << "Values are printed as grid / computed (diff in km)" << endl;

  double maxLatDiffKm = 0.0;
  double maxLonDiffKm = 0.0;
  double lat, lon, xx, yy;
  double gridLat, gridLon;
  double latDiffKm, lonDiffKm;

  cout.fill();
  cout.setf(ios::left);
  cout.precision(7);

  cout << "--->> Southern boundary <<---" << endl << endl;

  for (int i = 0; i < inData->nLon; i++) {
    xx = inData->minx_cross + i * inData->grid_distance;
    yy = inData->miny_cross;
    proj.xy2latlon(xx, yy, lat, lon);
    gridLat = inData->lat[0][i];
    gridLon = inData->lon[0][i];
    latDiffKm = (gridLat - lat) * cos(lat * DEG_TO_RAD) * KM_PER_DEG_AT_EQ;
    lonDiffKm = (gridLon - lon) * cos(lat * DEG_TO_RAD) * KM_PER_DEG_AT_EQ;
    maxLatDiffKm = MAX(maxLatDiffKm, fabs(latDiffKm));
    maxLonDiffKm = MAX(maxLonDiffKm, fabs(lonDiffKm));
    cout << "Lat: "
	 << setw(9) << setprecision(7) << gridLat << "/ "
	 << setw(9) << setprecision(7) << lat
	 << " (" << setw(8) << setprecision(3) << latDiffKm << ")"
	 << ", Lon: "
	 << setw(9) << setprecision(7) << gridLon << "/ "
	 << setw(9) << setprecision(7) << lon
	 << " (" << setw(8) << setprecision(3) << lonDiffKm << ")"
	 << endl;
  }
  cout << endl;
  
  cout << "--->> Northern boundary <<---" << endl << endl;
  
  for (int i = 0; i < inData->nLon; i++) {
    xx = inData->minx_cross + i * inData->grid_distance;
    yy = inData->miny_cross + (inData->nLat - 1) * inData->grid_distance;
    proj.xy2latlon(xx, yy, lat, lon);
    gridLat = inData->lat[inData->nLat - 1][i];
    gridLon = inData->lon[inData->nLat - 1][i];
    latDiffKm = fabs(gridLat - lat) * cos(lat * DEG_TO_RAD) * KM_PER_DEG_AT_EQ;
    lonDiffKm = fabs(gridLon - lon) * cos(lat * DEG_TO_RAD) * KM_PER_DEG_AT_EQ;
    maxLatDiffKm = MAX(maxLatDiffKm, latDiffKm);
    maxLonDiffKm = MAX(maxLonDiffKm, lonDiffKm);
    latDiffKm = (gridLat - lat) * cos(lat * DEG_TO_RAD) * KM_PER_DEG_AT_EQ;
    lonDiffKm = (gridLon - lon) * cos(lat * DEG_TO_RAD) * KM_PER_DEG_AT_EQ;
    maxLatDiffKm = MAX(maxLatDiffKm, fabs(latDiffKm));
    maxLonDiffKm = MAX(maxLonDiffKm, fabs(lonDiffKm));
    cout << "Lat: "
	 << setw(9) << setprecision(7) << gridLat << "/ "
	 << setw(9) << setprecision(7) << lat
	 << " (" << setw(8) << setprecision(3) << latDiffKm << ")"
	 << ", Lon: "
	 << setw(9) << setprecision(7) << gridLon << "/ "
	 << setw(9) << setprecision(7) << lon
	 << " (" << setw(8) << setprecision(3) << lonDiffKm << ")"
	 << endl;
  }
  cout << endl;
  
  cout << "--->> Western boundary <<---" << endl << endl;
  
  for (int i = 0; i < inData->nLat; i++) {
    xx = inData->minx_cross;
    yy = inData->miny_cross + i * inData->grid_distance;
    proj.xy2latlon(xx, yy, lat, lon);
    gridLat = inData->lat[i][0];
    gridLon = inData->lon[i][0];
    latDiffKm = (gridLat - lat) * cos(lat * DEG_TO_RAD) * KM_PER_DEG_AT_EQ;
    lonDiffKm = (gridLon - lon) * cos(lat * DEG_TO_RAD) * KM_PER_DEG_AT_EQ;
    maxLatDiffKm = MAX(maxLatDiffKm, fabs(latDiffKm));
    maxLonDiffKm = MAX(maxLonDiffKm, fabs(lonDiffKm));
    cout << "Lat: "
	 << setw(9) << setprecision(7) << gridLat << "/ "
	 << setw(9) << setprecision(7) << lat
	 << " (" << setw(8) << setprecision(3) << latDiffKm << ")"
	 << ", Lon: "
	 << setw(9) << setprecision(7) << gridLon << "/ "
	 << setw(9) << setprecision(7) << lon
	 << " (" << setw(8) << setprecision(3) << lonDiffKm << ")"
	 << endl;
  }
  cout << endl;
  
  cout << "--->> Eastern boundary <<---" << endl << endl;
  
  for (int i = 0; i < inData->nLat; i++) {
    xx = inData->minx_cross + (inData->nLon - 1) * inData->grid_distance;
    yy = inData->miny_cross + i * inData->grid_distance;
    proj.xy2latlon(xx, yy, lat, lon);
    gridLat = inData->lat[i][inData->nLon - 1];
    gridLon = inData->lon[i][inData->nLon - 1];
    latDiffKm = (gridLat - lat) * cos(lat * DEG_TO_RAD) * KM_PER_DEG_AT_EQ;
    lonDiffKm = (gridLon - lon) * cos(lat * DEG_TO_RAD) * KM_PER_DEG_AT_EQ;
    maxLatDiffKm = MAX(maxLatDiffKm, fabs(latDiffKm));
    maxLonDiffKm = MAX(maxLonDiffKm, fabs(lonDiffKm));
    cout << "Lat: "
	 << setw(9) << setprecision(7) << gridLat << "/ "
	 << setw(9) << setprecision(7) << lat
	 << " (" << setw(8) << setprecision(3) << latDiffKm << ")"
	 << ", Lon: "
	 << setw(9) << setprecision(7) << gridLon << "/ "
	 << setw(9) << setprecision(7) << lon
	 << " (" << setw(8) << setprecision(3) << lonDiffKm << ")"
	 << endl;
  }
  cout << endl;

  cout << "---> Max diff in lat: " << maxLatDiffKm << " km" << endl;
  cout << "---> Max diff in lon: " << maxLonDiffKm << " km" << endl;

  
}

void MM5Print::_printBorderGis(MM5Data *inData)

{

  int poly_count = 0;
  cout << poly_count++ << endl;

  // Southern boundary

  for (int i = 0; i < inData->nLon; i++) {
    cout << inData->lon[0][i] << ", "
	 << inData->lat[0][i] << endl;
  }
  cout << "END" << endl;
  
  // Northern boundary

  cout << poly_count++ << endl;
  for (int i = 0; i < inData->nLon; i++) {
    cout << inData->lon[inData->nLat - 1][i] << ", "
	 << inData->lat[inData->nLat - 1][i] << endl;
  }
  cout << "END" << endl;
  
  // Western boundary

  cout << poly_count++ << endl;
  for (int i = 0; i < inData->nLat; i++) {
    cout << inData->lon[i][0] << ", "
	 << inData->lat[i][0] << endl;
  }
  cout << "END" << endl;
  
  // Eastern boundary

  cout << poly_count++ << endl;
  for (int i = 0; i < inData->nLat; i++) {
    cout << inData->lon[i][inData->nLon - 1] << ", "
	 << inData->lat[i][inData->nLon - 1] << endl;
  }
  cout << "END" << endl;

  // Another to end the group
  cout << "END" << endl;
  
}

void MM5Print::_printBorderMap(MM5Data *inData)

{

  int npoints = 2 * (inData->nLat + inData->nLon) + 4;

  cout << "POLYLINE MM5_border " << npoints << endl;

  // Southern boundary

  for (int i = 0; i < inData->nLon; i++) {
    cout << inData->lat[0][i] << " "
	 << inData->lon[0][i] << endl;
  }
  cout << "-1000.0 -1000.0" << endl;
  
  // Northern boundary

  for (int i = 0; i < inData->nLon; i++) {
    cout << inData->lat[inData->nLat - 1][i] << " "
	 << inData->lon[inData->nLat - 1][i] << endl;
  }
  cout << "-1000.0 -1000.0" << endl;
  
  // Western boundary

  for (int i = 0; i < inData->nLat; i++) {
    cout << inData->lat[i][0] << " "
	 << inData->lon[i][0] << endl;
  }
  cout << "-1000.0 -1000.0" << endl;
  
  // Eastern boundary

  for (int i = 0; i < inData->nLat; i++) {
    cout << inData->lat[i][inData->nLon - 1] << " "
	 << inData->lon[i][inData->nLon - 1] << endl;
  }
  cout << "-1000.0 -1000.0" << endl;
  
}

void MM5Print::_printGridMap(MM5Data *inData)

{

  int npoints =
    inData->nLat * (inData->nLon + 1) + inData->nLon * (inData->nLat + 1);

  cout << "POLYLINE MM5_grid " << npoints << endl;

  // E-W grid

  for (int j = 0; j < inData->nLat; j++) {
    for (int i = 0; i < inData->nLon; i++) {
      cout << inData->lat[j][i] << " "
	   << inData->lon[j][i] << endl;
    }
    cout << "-1000.0 -1000.0" << endl;
  }
  
  // N-S grid

  for (int j = 0; j < inData->nLon; j++) {
    for (int i = 0; i < inData->nLat; i++) {
      cout << inData->lat[i][j] << " "
	   << inData->lon[i][j] << endl;
    }
    cout << "-1000.0 -1000.0" << endl;
  }
  
}

