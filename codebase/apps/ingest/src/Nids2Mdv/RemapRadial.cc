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
//////////////////////////////////////////////////////////
// RemapRadial.cc
//
// Remap the radial data onto a cart grid
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Paddy McCarthy added abstract class and RemapRast to generalize
//
// March 1999
//
//////////////////////////////////////////////////////////

#include "RemapRadial.hh"
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
using namespace std;
// #include <mdv/mdv_utils.h>

// constructor

// Constructor for:
//  o realtime mode (single input dir)
//  o pre-specified output geometry
// 
RemapRadial::RemapRadial(const string &progName,
	                 const string &inputDir,
	                 const string &radarName,
	                 const string &outputDir,
                         const int maxDataAge,
                         const bool useLDataInfo,
                         const bool getLatestFileOnly,
                         const bool computeScaleAndBias,
                         const float specifiedScale,
                         const float specifiedBias,
                         const bool debug, const bool verbose,
                         const int nx, const int ny, 
                         const float dx, const float dy,
                         const float minx, const float miny,
                         const string &dataFieldNameLong,
                         const string &dataFieldName,
                         const string &dataUnits,
                         const string &dataTransform,
                         const int dataFieldCode,
                         const long processingDelay) :
  Remap(progName, inputDir, radarName, outputDir,
        maxDataAge, useLDataInfo, getLatestFileOnly,
        computeScaleAndBias, specifiedScale, specifiedBias,
        debug, verbose,
        nx, ny, dx, dy, minx, miny,
        dataFieldNameLong, dataFieldName,
        dataUnits, dataTransform, dataFieldCode,
        processingDelay)

{

  _elevAngle = -1.0;
  _nGates = -1;
  _gateSpacing = -1.0;
  _startRange = -1.0;

  _lutAz = NULL;
  _lutRng = NULL;
  _lutSize = 0;
}

// Constructor for:
//  o archive mode (vector of input dirs)
//  o pre-specified output geometry
// 
RemapRadial::RemapRadial(const string &progName,
	                 vector<string> inputPaths,
	                 const string &radarName,
	                 const string &outputDir,
                         const bool computeScaleAndBias,
                         const float specifiedScale,
                         const float specifiedBias,
                         const bool debug, const bool verbose,
                         const int nx, const int ny, 
                         const float dx, const float dy,
                         const float minx, const float miny,
                         const string &dataFieldNameLong,
                         const string &dataFieldName,
                         const string &dataUnits,
                         const string &dataTransform,
                         const int dataFieldCode,
                         const long processingDelay) :
  Remap(progName, inputPaths, radarName, outputDir,
        computeScaleAndBias, specifiedScale, specifiedBias,
        debug, verbose,
        nx, ny, dx, dy, minx, miny,
        dataFieldNameLong, dataFieldName,
        dataUnits, dataTransform, dataFieldCode,
        processingDelay)

{

  _elevAngle = -1.0;
  _nGates = -1;
  _gateSpacing = -1.0;
  _startRange = -1.0;

  _lutAz = NULL;
  _lutRng = NULL;
  _lutSize = 0;
}

// Constructor for:
//  o realtime mode (single input dir)
//  o output geometry will match input
// 
RemapRadial::RemapRadial(const string &progName,
	                 const string &inputDir,
	                 const string &radarName,
	                 const string &outputDir,
                         const int maxDataAge,
                         const bool useLDataInfo,
                         const bool getLatestFileOnly,
                         const bool computeScaleAndBias,
                         const float specifiedScale,
                         const float specifiedBias,
                         const bool debug, const bool verbose,
                         const string &dataFieldNameLong,
                         const string &dataFieldName,
                         const string &dataUnits,
                         const string &dataTransform,
                         const int dataFieldCode,
                         const long processingDelay) :
  Remap(progName, inputDir, radarName, outputDir,
        maxDataAge, useLDataInfo, getLatestFileOnly,
        computeScaleAndBias, specifiedScale, specifiedBias,
        debug, verbose,
        dataFieldNameLong, dataFieldName,
        dataUnits, dataTransform, dataFieldCode,
        processingDelay)

{

  _elevAngle = -1.0;
  _nGates = -1;
  _gateSpacing = -1.0;
  _startRange = -1.0;

  _lutAz = NULL;
  _lutRng = NULL;
  _lutSize = 0;
}

// Constructor for:
//  o archive mode (vector of input dirs)
//  o output geometry will match input
// 
RemapRadial::RemapRadial(const string &progName,
	                 vector<string> inputPaths,
	                 const string &radarName,
	                 const string &outputDir,
                         const bool computeScaleAndBias,
                         const float specifiedScale,
                         const float specifiedBias,
                         const bool debug, const bool verbose,
                         const string &dataFieldNameLong,
                         const string &dataFieldName,
                         const string &dataUnits,
                         const string &dataTransform,
                         const int dataFieldCode,
                         const long processingDelay) :
  Remap(progName, inputPaths, radarName, outputDir,
        computeScaleAndBias, specifiedScale, specifiedBias,
        debug, verbose,
        dataFieldNameLong, dataFieldName,
        dataUnits, dataTransform, dataFieldCode,
        processingDelay)

{

  _elevAngle = -1.0;
  _nGates = -1;
  _gateSpacing = -1.0;
  _startRange = -1.0;

  _lutAz = NULL;
  _lutRng = NULL;
  _lutSize = 0;
}

// destructor

RemapRadial::~RemapRadial()

{
  if (_lutAz != NULL) {
    delete[] _lutAz;
    _lutSize = 0;
  }
  if (_lutRng != NULL) {
    delete[] _lutRng;
    _lutSize = 0;
  }
}

// process file

int RemapRadial::processFile(const string &filePath)

{
  // 
  // Some systems need a sleep to check for files still being written.
  //   Sleep to make sure file is finished writing (fix) to check activity
  // 
  if (_processingDelay > 0) {
    time_t now;
    struct stat file_stat;

	do {
        if (stat(filePath.c_str(), &file_stat) != 0) {
          int errNum = errno;
          cerr << "ERROR - " << _progName
               << ":RemapRadial::processFile()" << endl;
          cerr << "Cannot stat file: " << filePath << endl;
          cerr << strerror(errNum) << endl;
          return (-1);
        }

		
		now = time(0);
		if(file_stat.st_mtime > now - _processingDelay) sleep(1);

    } while ( time(0) < file_stat.st_mtime + _processingDelay); 
  }

  // open file

  fprintf(stderr,"Working on file %s\n",filePath.c_str());
  FILE *in;
  if ((in = fopen (filePath.c_str(), "rb")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":RemapRadial::processFile()" << endl;
    cerr << "Cannot open file: " << filePath << endl;
    cerr << strerror(errNum) << endl;
    return (-1);
  }

  NIDS_header_t nhdr;
  char *hdr_ptr = (char *) &nhdr;

  // Make sure header area is null terminated for strstr()
  memset(hdr_ptr,0,sizeof(NIDS_header_t));

  // Read in 30 bytes looking for  21 WMO bytes + 9 PIL bytes
  if (ufread(hdr_ptr,30 , 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":RemapRadial::processFile()" << endl;
    cerr << "Cannot detect header in file: " << filePath << endl;
    cerr << strerror(errNum);
    fclose(in);
    return (-1);
  }

  int remaining_hdr_size;

  // IF this contains the end of the WMO+ PIL headers
  if(strstr(hdr_ptr + 27,"\r\r\n") == (hdr_ptr + 27)) {

    if (_verbose) cerr << "WMO + PIL detected in " << filePath << endl;


    // re-read the full header
    hdr_ptr = (char *) &nhdr;
    remaining_hdr_size = sizeof(NIDS_header_t);

  } else {  // No WMO + PIL detected

    if (_verbose) cerr << "No WMO + PIL detected in " << filePath << endl;

    // Only read what's left.
    remaining_hdr_size = sizeof(NIDS_header_t) - 30;
    hdr_ptr = (char *) &nhdr + 30;

  }

  // Fill in NIDS header from file read.
  if (ufread(hdr_ptr, remaining_hdr_size, 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":RemapRadial::processFile()" << endl;
    cerr << "Cannot read header from file: " << filePath << endl;
    cerr << strerror(errNum);
    fclose(in);
    return (-1);
  }
  NIDS_BE_to_mess_header(&nhdr);
  if (_verbose) {
    NIDS_print_mess_hdr(stderr, (char*)"  ", &nhdr);
  }

  // Allocate space for the data, read in the data

  ui08 *rawData = new ui08[nhdr.lendat];
  ui08 *rawPtr = rawData;

  memset(rawData, 0, nhdr.lendat); // Set all data to 0 in case file is truncated and we don't get all the bytes - Niles.

  int numBytesRead = ufread(rawData, sizeof(ui08), nhdr.lendat, in);
  fclose(in); // Done with file.

  if (numBytesRead != nhdr.lendat) {

    struct stat file_stat;
    if (stat(filePath.c_str(), &file_stat) != 0) {
      int errNum = errno;
      cerr << "ERROR - " << _progName
	   << ":RemapRadial::processFile()" << endl;
      cerr << "Cannot stat file: " << filePath << endl;
      cerr << strerror(errNum) << endl;
      return (-1);
    }

    cerr << endl << "WARNING : " << _progName << ":RemapRadial::processFile()" << endl;
    cerr << "Cannot read all radial data from file: " << filePath << endl;
    cerr << "Wanted " << nhdr.lendat <<  " bytes, got " << numBytesRead;
    cerr << " from file of size " << file_stat.st_size;
    cerr << ", difference of " << nhdr.lendat - numBytesRead << " bytes." <<  endl << endl;
    //
    // We did not get the number of bytes we
    // expected from the header size - as if the
    // file has been truncated.
    //
    // This used to be an error condition, however it stared happening for ATEC
    // ranges fairly commonly. Now we initialize the buffer to 0, we've read
    // what we can from the file so we go determinedly (dementedly?) on.
    //
    // Niles Oien April 2010.
    //
    /*    cerr << strerror(errNum) << "\n";
    delete[] rawData;
    return (-1); */
  }
  
  // check product format

  if ( rawData[0] != 0xaf || rawData[1] != 0x1f ) {
    cerr << "ERROR - " << _progName << ":RemapRadial::processFile()" << endl;
    cerr << "File not in radial format: " << filePath << endl;
    delete[] rawData;
    return (-1);
  }

  // compute scale and bias

  _computeScaleAndBias(nhdr);

  // read radial header

  NIDS_radial_header_t rhdr;
  memcpy(&rhdr, rawPtr, sizeof(NIDS_radial_header_t));
  rawPtr += sizeof(NIDS_radial_header_t);
  NIDS_BE_to_radial_header(&rhdr);
  if (_verbose) {
    NIDS_print_radial_hdr(stderr, (char*)"    ", &rhdr);
  }

  double elevAngle = nhdr.pd[0] / 10.0;
  int nGates = rhdr.num_r_bin;
  double gateSpacing = 1.0;
  double startRange = (rhdr.first_r_bin + 0.5) * gateSpacing;

  if (_verbose) {
    fprintf(stderr, "  elevAngle: %g\n", elevAngle);
    fprintf(stderr, "  ngates: %d\n", nGates);
    fprintf(stderr, "  gateSpacing: %g\n", gateSpacing);
    fprintf(stderr, "  startRange: %g\n", startRange);
  }

  // compute the grid lookup table
  if (_preserveInputGeom) {

    // Get the number of radials and the size of the first radial.
    _nRadials = rhdr.num_radials;
    NIDS_beam_header_t bhdr;
    memcpy(&bhdr, rawPtr, sizeof(NIDS_beam_header_t));
    NIDS_BE_to_beam_header(&bhdr);
    _dRadial = bhdr.radial_delta_angle / 10.0;

    _elevAngle = elevAngle;
    _nGates = nGates;
    _gateSpacing = gateSpacing;
    _startRange = startRange;

    // Set or reset the output geometry.

    // Only process the first 360 degrees of data.
    int nRadials = _nRadials;
    if ( ((float) nRadials * _dRadial) > 360) {
      nRadials = (int) (360.0/_dRadial);
    }
  
    _outGeom.setGeometry(_nGates, nRadials,
                         _gateSpacing, _dRadial,
                         _startRange, 0);
  }
  else {
    // Compute the lookup, if necessary, and set the data members.

    _computeGridLookup(elevAngle, nGates, gateSpacing, startRange);
  }

  // allocate array for radial data
  ui08 *radials = new ui08[rhdr.num_radials * nGates];
  memset(radials, 0, rhdr.num_radials * nGates);
  
  // uncompress radials, load up array

  if (_uncompressRadials(rhdr, nGates, rawPtr, radials)) {
    cerr << "File: " << filePath << endl;
    delete[] radials;
    delete[] rawData;
    return (-1);
  }
  
  // set up output file

  _out.clearVol();
  double radarLat = nhdr.lat / 1000.0;
  double radarLon = nhdr.lon / 1000.0;
  double radarHt = nhdr.height * 0.0003048;
  _out.setRadarPos(radarLat, radarLon, radarHt, elevAngle);

  _out.initHeaders(_outGeom.getNx(), _outGeom.getNy(),
                   _outGeom.getDx(), _outGeom.getDy(),
                   _outGeom.getMinx(), _outGeom.getMiny(),
                   _radarName.c_str(),
                   _dataFieldNameLong.c_str(),
                   _dataFieldName.c_str(),
                   _dataUnits.c_str(),
                   _dataTransform.c_str(),
                   _dataFieldCode,
                   _preserveInputGeom);

  _out.loadScaleAndBias(_scale, _bias);

  // Add the radials to the file
  _addRadials(radials);

  // Add the first _outputVal value (lowest bin value) to the user data.
  _out.setUserData0(((float)_outputVal[1])*_scale+_bias);
  
  // write the volume

  time_t scanTime =
    (nhdr.vsdate - 1) * 86400 + nhdr.vstime * 65536 + nhdr.vstim2;

  ta_makedir_recurse(_outputDir.c_str());
  if (_out.writeVol(scanTime, _outputDir.c_str())) {
    cerr << "ERROR - " << _progName << ":RemapRadial::processFile()" << endl;
    cerr << "Cannot write MDV output file to dir: " << _outputDir << endl;
    delete[] radials;
    delete[] rawData;
    return (-1);
  }

  // free up

  delete[] radials;
  delete[] rawData;

  setLastInputFile(filePath);
  setLastOutputFile(_out.getLastOutputFile());

  return (0);

}


////////////////////////////////////////////
// compute the grid remapping lookup table

void RemapRadial::_computeGridLookup(double elevAngle, int nGates,
                                     double gateSpacing, double startRange)

{

  // check if we need new table
  // 
  if (elevAngle == _elevAngle && nGates == _nGates &&
      gateSpacing == _gateSpacing && startRange == _startRange) {
    return;
  }

  // Check if we need new lookup tables.
  // 
  int nptsGrid = _outGeom.getNx() * _outGeom.getNy();
  if (_lutSize != nptsGrid) {

    if (_lutAz != NULL) {
      delete[] _lutAz;
    }
    _lutAz = new si16[nptsGrid];

    if (_lutRng != NULL) {
      delete[] _lutRng;
    }
    _lutRng = new si16[nptsGrid];

    _lutSize = nptsGrid;
  }

  _elevAngle = elevAngle;
  _nGates = nGates;
  _gateSpacing = gateSpacing;
  _startRange = startRange;
  
  if (_verbose) {
    cerr << "Computing lookup" << endl;
  }

  double cosel = cos(elevAngle * DEG_TO_RAD);
  double y = _outGeom.getMiny();
  double dy = _outGeom.getDy();
  
  int ii = 0;

  for (int iy = 0; iy < _outGeom.getNy(); iy++, y += dy) {

    double x = _outGeom.getMinx();
    double dx = _outGeom.getDx();

    for (int ix = 0; ix < _outGeom.getNx(); ix++, x += dx, ii++) {

      // range

      double gndRange = sqrt(x * x + y * y) * cosel;
      int irng = (int) ((gndRange - startRange) / gateSpacing + 0.5);
      if (irng < 0 || irng > nGates - 1) {
	_lutRng[ii] = -1;
      } else {
	_lutRng[ii] = irng;
      }

      // azimuth

      double az;
      if (x == 0 && y == 0) {
	az = 0.0;
      } else {
	az = atan2(x, y) * RAD_TO_DEG;
      }
      if (az < 0.0) {
	az += 360.0;
      }
      int iaz = (int) (az * 10.0 + 0.5);
      if (iaz < 0) iaz += NAZPOS;
      if (iaz > NAZPOS - 1) iaz -= NAZPOS;
      _lutAz[ii] = iaz;

    } // ix

  } // iy
    
}

///////////////////////////////////////////////////////
// _uncompressRadials()
//
// uncompress radials, load up array

int RemapRadial::_uncompressRadials(const NIDS_radial_header_t &rhdr, int nGates,
			      ui08 *rawPtr, ui08 *radials)
  
{

  memset(_azIndex, -1, sizeof(int) * NAZPOS);

  for (int irad = 0; irad < rhdr.num_radials; irad++) {
    
    // read in a radial

    NIDS_beam_header_t bhdr;
    memcpy(&bhdr, rawPtr, sizeof(NIDS_beam_header_t));
    rawPtr += sizeof(NIDS_beam_header_t);
    NIDS_BE_to_beam_header(&bhdr);
    if (_verbose) {
      NIDS_print_beam_hdr(stderr, (char*)"    ", &bhdr);
    }

    // store the radial number of this azimuth in the index

    // this looks correct theoretically 

    for (int ii = bhdr.radial_start_angle;
	 ii <= bhdr.radial_start_angle + bhdr.radial_delta_angle;
	 ii++) {

    // however this seems to match the mosaic more closely - Dixon

//     for (int ii = bhdr.radial_start_angle - bhdr.radial_delta_angle;
// 	 ii <= bhdr.radial_start_angle;
// 	 ii++) {

      int jj = ii;
      if (jj > NAZPOS - 1) {
	jj -= NAZPOS;
      } else if (jj < 0) {
	jj += NAZPOS;
      }
      if (_azIndex[jj] < 0) {
	_azIndex[jj] = irad;
      }
    }

#ifdef DEBUG_AZIMUTHS
    bool doit = false;
    double az =
      bhdr.radial_start_angle / 10.0 + bhdr.radial_delta_angle / 20.0;
    
    if (az < -0.5) {
      az += 360.0;
    }
    if (az >= 359.5) {
      az -= 360.0;
    }
    
    if ((az <= 0.5 || az >= 359.5) ||
	(az <= 90.5 && az >= 89.5) ||
	(az <= 180.5 && az >= 179.5) ||
	(az <= 270.5 && az >= 269.5)) {
      fprintf(stderr, "---------------->> az: %g\n", az);
      doit = true;
    }
#endif

    // Run Length decode this radial

    ui08 *rr = radials + irad * nGates;
    int nbytes_run = bhdr.num_halfwords * 2;
    int nbins = 0;
    for (int run = 0; run < nbytes_run; run++ ) {
      int drun = *rawPtr >> 4;
      int dcode = *rawPtr & 0xf;
      nbins += drun;
      if (nbins > rhdr.num_r_bin) {
	cerr << "ERROR - " << _progName << ":RemapRadial::processFile()" << endl;
	cerr << "Bad gate count" << endl;
	return (-1);
      }
#ifdef DEBUG_AZIMUTHS
      if (doit) {
        memset(rr, 14, drun);
      } else {
        memset(rr, dcode, drun);
      }
#else
      memset(rr, dcode, drun);
#endif
      rr += drun;
      rawPtr++;
    }
    
    if (_verbose) {
      fprintf(stderr, "    nbins expected: %d, number found: %d\n",
	      rhdr.num_r_bin, nbins);
    }

  } // irad

  return (0);

}

void RemapRadial::_addRadials(ui08 *radials)

{
  if (_preserveInputGeom) {
    _doNoRemapping(radials);
  }
  else {
    _doRemapping(radials);
  }
}

///////////////////////////////////////
// _doNoRemapping
//
// Move the radials as-is into the mdv file.

void RemapRadial::_doNoRemapping(ui08 *radials)

{
  ui08 * outRadials = _out.getFieldPlane();

  // Only process the first 360 degrees of data.
  int nRadials = _nRadials;
  if ( ((float) nRadials * _dRadial) > 360) {
    nRadials = (int) (360.0/_dRadial);
  }
  
  for (int i = 0; i < nRadials; i++) {

// cerr << endl << "Radial: " << i << endl;
    // Get the radial offset from the index table (radials don't start at 0)
    int irad = _azIndex[i*10];

// cerr << "Using irad = " << irad << endl;

    for (int j = 0; j < _nGates; j++) {
      int outPos = i * _nGates + j;

      if (irad < 0) {
        outRadials[outPos] = 0;
// cerr << "Gate " << j << " skipped b/c irad is negative." << endl;
      }
      else {
        int inPos = irad * _nGates + j;
        int outval = _outputVal[radials[inPos]];
        if (outval > (int) outRadials[outPos]) {
          outRadials[outPos] = (ui08) outval;
// cerr << "Gate " << j << " given value: " << outval << " (" << (int) outRadials[outPos] << ")" << endl;
        }
// else {
// cerr << "Gate " << j << " with value: " << outval << " skipped b/c existing value is bigger: " << (int) outRadials[outPos] << endl;
// }
      }
    }
  }
}

///////////////////////////////////////
// _doRemapping
//
// Perform remapping into mdv file.

void RemapRadial::_doRemapping(ui08 *radials)

{

  ui08 *cart = _out.getFieldPlane();

  int nptsGrid = _outGeom.getNx() * _outGeom.getNy();
  for (int i = 0; i < nptsGrid; i++, cart++) {

    if (_lutRng[i] >= 0) {

      int iaz = _lutAz[i];
      int irad = _azIndex[iaz];

      if (irad < 0) {
	*cart = 0;
      } else {
	int pos = irad * _nGates + _lutRng[i];
	int outval = _outputVal[radials[pos]];
	if (outval > *cart) {
	  *cart = outval;
	}
      }

    } else {

      *cart = 0;

    }

  } // i

}
