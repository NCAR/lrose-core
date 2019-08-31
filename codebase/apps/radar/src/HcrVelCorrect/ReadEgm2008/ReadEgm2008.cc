/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// ReadEgm2008.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// ReadEgm2008 reads in Geoid corrections from Egm2008 files
//
////////////////////////////////////////////////////////////////

#include "ReadEgm2008.hh"
#include <string>
#include <cmath>
#include <malloc.h>
#include <iostream>
#include <dataport/port_types.h>
#include <dataport/bigend.h>
#include <toolsa/uusleep.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <didss/DataFileNames.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaFile.hh>
#include <sys/stat.h>
using namespace std;

// Constructor

ReadEgm2008::ReadEgm2008(int argc, char **argv) :
  _args("ReadEgm2008")

{

  OK = TRUE;

  // set programe name

  _progName = strdup("ReadEgm2008");

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

ReadEgm2008::~ReadEgm2008()

{

  
}

//////////////////////////////////////////////////
// Run

int ReadEgm2008::Run()
{

  // get file size

  struct stat fstat;
  if (ta_stat(_params.path, &fstat)) {
    int errNum = errno;
    cerr << "ERROR - cannot stat egm file: " << _params.path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  int nPtsPerDeg = 24;
  int nlat = 180 * nPtsPerDeg + 1;
  int nlon = 360 * nPtsPerDeg;
  int npoints = nlat * nlon;
  ui32 recLen = nlon * sizeof(fl32);

  if (_params.debug) {
    cerr << "==>> nPtsPerDeg: " << nPtsPerDeg << endl;
    cerr << "==>> nlat: " << nlat << endl;
    cerr << "==>> nlon: " << nlon << endl;
    cerr << "==>> npoints: " << npoints << endl;
    cerr << "==>> recLen: " << recLen << endl;
  }
  
  off_t nbytes = fstat.st_size;
  int nexpected = npoints * sizeof(fl32) + nlat * 2 * sizeof(ui32);
  if (nbytes != nexpected) {
    cerr << "ERROR - bad egm2008 file: " << _params.path << endl;
    cerr << "  expected nbytes: " << nexpected << endl;
    cerr << "  file size nbytes: " << nbytes << endl;
    return -1;
  }

  // open elevation data file
  
  FILE *egmFile;
  if ((egmFile = fopen(_params.path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - cannot open egm file: " << _params.path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // geoid data in meters

  fl32 *geoidM = new fl32[npoints];
  int npts = 0;
  bool mustSwap = false;
  
  // read through the file, a fortran record at a time
  
  while (!feof(egmFile)) {
    
    // read starting fortran record len
    
    ui32 recLenStart;
    if (fread(&recLenStart, sizeof(recLenStart), 1, egmFile) != 1) {
      if (feof(egmFile)) {
        break;
      }
      int errNum = errno;
      cerr << "ERROR - cannot read start fort rec len from egm file: " 
           << _params.path << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(egmFile);
      delete[] geoidM;
      return -1;
    }
    if (recLenStart != recLen) {
      recLenStart = BE_from_ui32(recLenStart);
      mustSwap = true;
    }
    if (recLenStart != recLen) {
      cerr << "ERROR - bad egm2008 file: " << _params.path << endl;
      cerr << "  bad recLenStart: " << recLenStart << endl;
      fclose(egmFile);
      delete[] geoidM;
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Start fort rec len: " << recLenStart << endl;
    }
    
    // read in data
    
    int nFl32InRec = recLenStart / sizeof(fl32);
    fl32 *buf = new fl32[nFl32InRec];
    if (fread(buf, sizeof(fl32), nFl32InRec, egmFile) != (size_t) nFl32InRec) {
      int errNum = errno;
      cerr << "ERROR - cannot read data from egm file: " << _params.path << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(egmFile);
      delete[] geoidM;
      delete[] buf;
      return -1;
    }
    
    // swap as needed

    if (mustSwap) {
      BE_from_array_32(buf, nFl32InRec * sizeof(fl32));
    }

    // copy data into array
    
    for (off_t ii = 0; ii < nFl32InRec; ii++) {
      geoidM[npts] = buf[ii];
      npts++;
    }
    delete[] buf;
    
    // read ending fortran record len
    
    ui32 recLenEnd;
    if (fread(&recLenEnd, sizeof(recLenEnd), 1, egmFile) != 1) {
      int errNum = errno;
      cerr << "ERROR - cannot read end fort rec len from egm file: " 
           << _params.path << endl;
      cerr << "  " << strerror(errNum) << endl;
      fclose(egmFile);
      delete[] geoidM;
      return -1;
    }
    if (mustSwap) {
      recLenEnd = BE_from_ui32(recLenEnd);
    }
    if (recLenEnd != recLen) {
      cerr << "ERROR - bad egm2008 file: " << _params.path << endl;
      cerr << "  bad recLenEnd: " << recLenEnd << endl;
      fclose(egmFile);
      delete[] geoidM;
      return -1;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "End fort rec len: " << recLenEnd << endl;
    }
    
  } // while
  
  // close file

  fclose(egmFile);

  if (_params.debug) {
    cerr << "Read npts: " << npts << endl;
  }

  // print out in debug mode
  
  if (_params.debug) {
    int ipt = 0;
    for (int ilat = 0; ilat < nlat; ilat++) {
      double lat = 90.0 - ilat / (double) nPtsPerDeg;
      for (int ilon = 0; ilon < nlon; ilon++, ipt++) {
        double lon = ilon / (double) nPtsPerDeg;
        if (lon > 180.0) {
          lon -= 360.0;
        }
        fprintf(stderr, "lat, lon, geoidM: %10.5f  %10.5f  %10.5f\n", 
                lat, lon, geoidM[ipt]);
      }
    } // ilat
  }

  // get lat/lon for point of interest

  int ilat = (int) (((90.0 - _params.lat) * (double) nPtsPerDeg) + 0.5);
  int ilon = (int) ((_params.lon * (double) nPtsPerDeg) + 0.5);
  if (_params.lon < 0) {
    ilon = (int) (((_params.lon + 360.0) * (double) nPtsPerDeg) + 0.5);
  }
  
  double lat = 90.0 - ilat / (double) nPtsPerDeg;
  double lon = ilon / (double) nPtsPerDeg;
  if (lon > 180) {
    lon -= 360.0;
  }

  int index = ilon + ilat * nlon;
  cerr << "====>> lat, lon, geoid: " 
       << lat << ", " 
       << lon << ", " 
       << geoidM[index] << endl;

  return 0;

}

