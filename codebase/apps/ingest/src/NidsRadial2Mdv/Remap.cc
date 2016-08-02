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
// Remap.cc
//
// Remap the radial data onto a cart grid
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
//////////////////////////////////////////////////////////

#include "Remap.hh"
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
#include <Mdv/mdv/mdv_utils.h>
using namespace std;

// constructor

Remap::Remap(const string &prog_name, const Params &params,
	     const string &radar_name, const bool is_dbz,
	     const string &output_dir, const bool use_specified_fieldname,
	     const char *specified_fieldname) :
  _progName(prog_name), _params(params),
  _out(prog_name, params)

{

  _radarName = radar_name;
  _isDbz = is_dbz;
  _outputDir = output_dir;

  _elevAngle = -1.0;
  _nGates = -1;
  _gateSpacing = -1.0;
  _startRange = -1.0;

  _nptsGrid = _params.output_grid.nx * _params.output_grid.ny;
  _lutAz = new si16[_nptsGrid];
  _lutRng = new si16[_nptsGrid];

  _useSpecifiedFieldname = use_specified_fieldname;
  _specifiedFieldname = (char *)specified_fieldname;

  return;

}

// destructor

Remap::~Remap()

{
  delete[] _lutAz;
  delete[] _lutRng;
}

// process file

int Remap::processFile(const string &filePath)

{

  if (_params.debug) {
    cerr << _progName << " - processing file: " << filePath << endl;
  }

  // open file

  FILE *in;
  if ((in = fopen (filePath.c_str(), "rb")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":Remap::processFile()" << endl;
    cerr << "Cannot open file: " << filePath << endl;
    cerr << strerror(errNum);
    return (-1);
  }

  // read in NIDS header

  NIDS_header_t nhdr;
  if (ufread(&nhdr, sizeof(NIDS_header_t), 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":Remap::processFile()" << endl;
    cerr << "Cannot read header from file: " << filePath << endl;
    cerr << strerror(errNum);
    fclose(in);
    return (-1);
  }
  NIDS_BE_to_mess_header(&nhdr);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
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
	   << ":Remap::processFile()" << endl;
      cerr << "Cannot stat file: " << filePath << endl;
      cerr << strerror(errNum) << endl;
      return (-1);
    }

    cerr << endl << "WARNING : " << _progName << ":Remap::processFile()" << endl;
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
    cerr << "ERROR - " << _progName << ":Remap::processFile()" << endl;
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
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    NIDS_print_radial_hdr(stderr, (char*)"    ", &rhdr);
  }

  double elevAngle = nhdr.pd[0] / 10.0;
  int nGates = rhdr.num_r_bin;
  double gateSpacing = 1.0;
  double startRange = (rhdr.first_r_bin + 0.5) * gateSpacing;

  if (_params.debug) {
    fprintf(stderr, "  elevAngle: %g\n", elevAngle);
    fprintf(stderr, "  ngates: %d\n", nGates);
    fprintf(stderr, "  gateSpacing: %g\n", gateSpacing);
    fprintf(stderr, "  startRange: %g\n", startRange);
  }

  // compute the grid lookup table

  _computeGridLookup(elevAngle, nGates, gateSpacing, startRange);

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


  if (_useSpecifiedFieldname){
    _out.initHeaders(_radarName.c_str(), _specifiedFieldname,
		     _specifiedFieldname,  _specifiedFieldname,
		     0);
  } else {
    if (_isDbz) {
      _out.initHeaders(_radarName.c_str(), (char*)"dBZ", (char*)"dBZ", (char*)"dBZ",
		       MDV_get_field_code_from_abbrev((char*)"DBZ"));
    } else {
      _out.initHeaders(_radarName.c_str(), (char*)"vel", (char*)"m/s", (char*)"none",
		       MDV_get_field_code_from_abbrev((char*)"R VEL"));
    }
  }
  _out.loadScaleAndBias(_scale, _bias);

  // perform remapping
  
  _doRemapping(radials);

  // write the volume

  time_t scanTime =
    (nhdr.vsdate - 1) * 86400 + nhdr.vstime * 65536 + nhdr.vstim2;

  ta_makedir_recurse(_outputDir.c_str());
  if (_out.writeVol(scanTime, _outputDir.c_str())) {
    cerr << "ERROR - " << _progName << ":Remap::processFile()" << endl;
    cerr << "Cannot write MDV output file to dir: " << _outputDir << endl;
    delete[] radials;
    delete[] rawData;
    return (-1);
  }

  // free up

  delete[] radials;
  delete[] rawData;

  return (0);

}


////////////////////////////////////////////
// compute the grid remapping lookup table

void Remap::_computeGridLookup(double elevAngle, int nGates,
			       double gateSpacing, double startRange)

{

  // check if we need new table

  if (elevAngle == _elevAngle && nGates == _nGates &&
      gateSpacing == _gateSpacing && startRange == _startRange) {
    return;
  }

  _elevAngle = elevAngle;
  _nGates = nGates;
  _gateSpacing = gateSpacing;
  _startRange = startRange;
  
  if (_params.debug) {
    cerr << "Computing lookup" << endl;
  }

  double cosel = cos(elevAngle * DEG_TO_RAD);
  double y = _params.output_grid.miny;
  double dy = _params.output_grid.dy;
  
  int ii = 0;

  for (int iy = 0; iy < _params.output_grid.ny; iy++, y += dy) {

    double x = _params.output_grid.minx;
    double dx = _params.output_grid.dx;

    for (int ix = 0; ix < _params.output_grid.nx; ix++, x += dx, ii++) {

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

//////////////////////////////////////////////////////
// compute scale and bias, and load up outputVal array

void Remap::_computeScaleAndBias(NIDS_header_t &nhdr)

{

  // Straighten out the data thresholds from header

  double levels[16];
  for (int i = 1; i < 16; i++ ) {
    ui08 msbyte, lsbyte;
    msbyte = (nhdr.pd[i+1] & 0xff00) >> 8;
    lsbyte = (nhdr.pd[i+1] & 0x00ff);
    levels[i] = lsbyte;
    if (msbyte & 0x01) {
      levels[i] *= -1.0;
    }
    if (msbyte & 0x10) {
      levels[i] /= 10.0;
    }
  }

  // compute the interval between bins.

  double interval = ((levels[15] - levels[1]) / 14.0);
  
  // compute a scale, bias based on the dynamic range of the data

  _scale = interval / 10.0;
  _bias =  levels[1] - interval;

  // compute output byte value for each data level

  _outputVal[0] = 0;
  for(int i = 1; i <= 15; i++) {
    _outputVal[i] = (int) ((levels[i] - _bias) / _scale + 0.5);
  }

}

///////////////////////////////////////////////////////
// _uncompressRadials()
//
// uncompress radials, load up array

int Remap::_uncompressRadials(const NIDS_radial_header_t &rhdr, int nGates,
			      ui08 *rawPtr, ui08 *radials)
  
{

  memset(_azIndex, -1, sizeof(int) * NAZPOS);

  for (int irad = 0; irad < rhdr.num_radials; irad++) {
    
    // read in a radial

    NIDS_beam_header_t bhdr;
    memcpy(&bhdr, rawPtr, sizeof(NIDS_beam_header_t));
    rawPtr += sizeof(NIDS_beam_header_t);
    NIDS_BE_to_beam_header(&bhdr);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
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
	cerr << "ERROR - " << _progName << ":Remap::processFile()" << endl;
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
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "    nbins expected: %d, number found: %d\n",
	      rhdr.num_r_bin, nbins);
    }

  } // irad

  return (0);

}

///////////////////////////////////////
// _doRemapping
//
// Perform remapping into output grid

void Remap::_doRemapping(ui08 *radials)

{

  ui08 *cart = _out.getFieldPlane();

  for (int i = 0; i < _nptsGrid; i++, cart++) {

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
