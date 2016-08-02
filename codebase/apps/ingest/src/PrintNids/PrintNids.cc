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
// PrintNids.cc
//
// PrintNids object - prints a nids file
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include "PrintNids.hh"
#include <toolsa/umisc.h>
#include <iostream>
using namespace std;

// Constructor

PrintNids::PrintNids(int argc, char **argv) 

{
  
  isOK = true;

  // set programe name

  _progName = "PrintNids";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  return;

}

// destructor

PrintNids::~PrintNids()

{

}

//////////////////////////////////////////////////
// Run

int PrintNids::Run ()
{


  if (_args.debug) {
    cout << _progName << " - processing file: " << _args.filePath << endl;
  }

  // open file

  FILE *in;
  if ((in = fopen (_args.filePath.c_str(), "rb")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << endl;
    cerr << "Cannot open file: " << _args.filePath << endl;
    cerr << strerror(errNum);
    return (-1);
  }

  // read in NIDS header
  
  NIDS_header_t nhdr;
  if (ufread(&nhdr, sizeof(NIDS_header_t), 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << endl;
    cerr << "Cannot read header from file: " << _args.filePath << endl;
    cerr << strerror(errNum);
    fclose(in);
    return (-1);
  }
  NIDS_BE_to_mess_header(&nhdr);
  NIDS_print_mess_hdr(stdout, "  ", &nhdr);
  fflush(stdout);

  // Allocate space for the data, read in the data
  
  ui08 *rawData = new ui08[nhdr.lendat];
  ui08 *rawPtr = rawData;
  
  if (ufread(rawData, nhdr.lendat, 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << endl;
    cerr << "Cannot read radial data from file: " << _args.filePath << endl;
    cerr << strerror(errNum);
    fclose(in);
    delete[] rawData;
    return (-1);
  }
  fclose(in);
  
  // check product format
  
  bool isRadial = false;
  if ( rawData[0] == 0xaf && rawData[1] == 0x1f ) {
    isRadial = true;
  }
  
  // read radial header

  if (isRadial) {

    NIDS_radial_header_t radhdr;
    memcpy(&radhdr, rawPtr, sizeof(NIDS_radial_header_t));
    rawPtr += sizeof(NIDS_radial_header_t);
    NIDS_BE_to_radial_header(&radhdr);
    NIDS_print_radial_hdr(stdout, "    ", &radhdr);
    fflush(stdout);

    double elevAngle = nhdr.pd[0] / 10.0;
    int nGates = radhdr.num_r_bin;
    double gateSpacing = 1.0;
    double startRange = (radhdr.first_r_bin + 0.5) * gateSpacing;
    
    fprintf(stdout, "      elevAngle: %g\n", elevAngle);
    fprintf(stdout, "      ngates: %d\n", nGates);
    fprintf(stdout, "      gateSpacing: %g\n", gateSpacing);
    fprintf(stdout, "      startRange: %g\n", startRange);

    if (_args.fullMode) {
      _printRadial(radhdr, rawPtr);
    }

  } else {

    // raster mode

    NIDS_raster_header_t rastHdr;
    memcpy(&rastHdr, rawPtr, sizeof(NIDS_raster_header_t));
    rawPtr += sizeof(NIDS_raster_header_t);
    NIDS_BE_to_raster_header(&rastHdr);
    NIDS_print_raster_hdr(stdout, "    ", &rastHdr);
    fflush(stdout);

    if (_args.fullMode) {
      _printRaster(rastHdr, rawPtr);
    }

  }

  // free up

  delete[] rawData;

  return (0);

}

// print radial details

void PrintNids::_printRadial(const NIDS_radial_header_t &radhdr, ui08 *rawPtr)

{

  for (int irad = 0; irad < radhdr.num_radials; irad++) {
    
    // read in a radial
    
    NIDS_beam_header_t bhdr;
    memcpy(&bhdr, rawPtr, sizeof(NIDS_beam_header_t));
    rawPtr += sizeof(NIDS_beam_header_t);
    NIDS_BE_to_beam_header(&bhdr);
    NIDS_print_beam_hdr(stdout, "    ", &bhdr);
    
    // Run Length decode this radial

    int nbytes_run = bhdr.num_halfwords * 2;
    int nbins = 0;
    for (int run = 0; run < nbytes_run; run++ ) {
      int drun = *rawPtr >> 4;
      int dcode = *rawPtr & 0xf;
      nbins += drun;
      if (nbins > radhdr.num_r_bin) {
	cerr << "  ERROR - " << _progName << ":Remap::processFile()" << endl;
	cerr << "  Bad gate count, dcode: \a" << dcode << endl;
      }
      rawPtr++;
    }
    
    fprintf(stdout, "-----> nbins expected: %d, number found: %d\n",
	    radhdr.num_r_bin, nbins);
    fflush (stdout);
    
  } // irad


}

// print raster details

void PrintNids::_printRaster(const NIDS_raster_header_t &rastHdr, ui08 *rawPtr)

{

  for (int irow = 0; irow < rastHdr.num_rows; irow++) {
    
    // read in a row
    
    NIDS_row_header_t rowhdr;
    memcpy(&rowhdr, rawPtr, sizeof(NIDS_row_header_t));
    rawPtr += sizeof(NIDS_row_header_t);
    NIDS_BE_to_row_header(&rowhdr);
    NIDS_print_row_hdr(stdout, "    ", &rowhdr);
    
    // Run Length decode this raster

    int nx = 0;
    for (int run = 0; run < rowhdr.num_bytes; run++ ) {
      int drun = *rawPtr >> 4;
      int dcode = *rawPtr & 0xf;
      nx += drun;
      if (nx > rastHdr.num_rows) {
	cerr << "  ERROR - " << _progName << ":Remap::processFile()" << endl;
	cerr << "  Bad row count, dcode: \a" << dcode << endl;
      }
      rawPtr++;
    }

    // we assune a square grid

    fprintf(stdout, "-----> Row %d, nx: %d, found: %d\n",
	    irow, rastHdr.num_rows, nx);
    fflush (stdout);
    
  } // irow


}
