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
// RadxTest.cc
//
// RadxTest object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2011
//
///////////////////////////////////////////////////////////////
//
// RadxTest tests functionality in the Radx lib
//
///////////////////////////////////////////////////////////////

#include "RadxTest.hh"
#include <string>
#include <iostream>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/DateTime.hh>

#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/DoradeRadxFile.hh>
#include <Radx/NexradRadxFile.hh>
#include <Radx/UfRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>

using namespace std;

// Constructor

RadxTest::RadxTest(int argc, char **argv) :
  _args("RadxTest")

{

  OK = TRUE;

  // set programe name

  _progName = strdup("RadxTest");

  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = FALSE;
    return;
  }

  return;

}

// destructor

RadxTest::~RadxTest()

{

  
}

//////////////////////////////////////////////////
// Run

int RadxTest::Run()
{

  
  // create a volume

  RadxVol vol;

  // fill with rays

  for (int isweep = 0; isweep < 3; isweep++) {

    double el = (double) isweep + 0.5;

    for (int iray = 0; iray < 360; iray++) {

      double az = (double) iray;

      RadxRay *ray = new RadxRay;

      ray->setVolumeNumber(0);
      ray->setSweepNumber(isweep);
      ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);

      struct timeval tv;
      gettimeofday(&tv, NULL);
      ray->setTime(tv.tv_sec, tv.tv_usec * 1000);
      ray->setAzimuthDeg(az);
      ray->setElevationDeg(el);
      ray->setFixedAngleDeg(el);
      ray->setIsIndexed(true);
      ray->setAngleResDeg(1.0);

      for (int ifield = 0; ifield < 3; ifield++) {

        char fieldName[64], units[64];
        sprintf(fieldName, "field_%d", ifield);
        sprintf(units, "units_%d", ifield);

        RadxField *field = new RadxField(fieldName, units);
        field->setRangeGeom(0.5, 1.0);

        int nGates = 100;
        Radx::fl32 data[100];
        memset(data, 0, 100 * sizeof(Radx::fl32));
        field->setDataFl32(nGates, data, true);

        ray->addField(field);
        
      }

      vol.addRay(ray);

    } // iray
    
  } // isweep

  vol.loadVolumeInfoFromRays();
  vol.loadSweepInfoFromRays();

  RadxFile *outFile;
  DoradeRadxFile *doradeFile = new DoradeRadxFile();
  outFile = doradeFile;
  outFile->setVerbose(true);
  outFile->setWriteCompressed(true);
  outFile->setCompressionLevel(9);
  outFile->setWriteNativeByteOrder(false);
  outFile->setFileFormat(RadxFile::FILE_FORMAT_DORADE);
      
  if (outFile->writeToDir(vol, "/home/dixon/junk/data/dorade", true, false)) {
    cerr << "ERROR - RadxTest" << endl;
    cerr << outFile->getErrStr() << endl;
  } else {
    cerr << "Wrote file: " << outFile->getPathInUse() << endl;
  }

  delete outFile;

#ifdef NOTNOW  
  DoradeRadxFile file;
  file.setVerbose(true);
  if (file.writeToDir(vol, "/home/dixon/junk/data/dorade", true, false)) {
    cerr << "ERROR - RadxTest" << endl;
    cerr << file.getErrStr() << endl;
  } else {
    cerr << "Wrote file: " << file.getPathInUse() << endl;
  }
#endif

  return 0;

}

