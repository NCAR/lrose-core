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
// RemapRast.cc
//
// Remap the raster data onto a larger/smaller cart grid
//
// Marty Petach, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Based on Mike Dixon's NidsRadial2Mdv program
//
// April 1999
//
//////////////////////////////////////////////////////////

#include "RemapRast.hh"
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
RemapRast::RemapRast(const string &progName,
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

}

// Constructor for:
//  o archive mode (vector of input dirs)
//  o pre-specified output geometry
// 
RemapRast::RemapRast(const string &progName,
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

}

// Constructor for:
//  o realtime mode (single input dir)
//  o output geometry will match input
// 
RemapRast::RemapRast(const string &progName,
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

}

// Constructor for:
//  o archive mode (vector of input dirs)
//  o output geometry will match input
// 
RemapRast::RemapRast(const string &progName,
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

}

// destructor

RemapRast::~RemapRast()

{
}

// process file

// virtual 
int RemapRast::processFile(const string &filePath)

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

  FILE *in;
  if ((in = fopen (filePath.c_str(), "rb")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":RemapRast::processRastFile()" << endl;
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

	  // re-read the full header
	  hdr_ptr = (char *) &nhdr;
	  remaining_hdr_size = sizeof(NIDS_header_t);

  } else {  // No WMO + PIL detected

	  // Only read what's left.
	  remaining_hdr_size = sizeof(NIDS_header_t) - 30;
	  hdr_ptr = (char *) &nhdr + 30;
  }

  // Fill in NIDS header from file read.
  if (ufread(hdr_ptr, remaining_hdr_size, 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":RemapRast::processRastFile()" << endl;
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
  
  if (_preserveInputGeom) {
    // 
    // I don't know how to implement this.
    // I don't think all the necessary info is in the NIDS headers...
    // 
    cerr << "ERROR: Raster source data cannot be left in its original "
         << "geometry -- you must specify grid geometry in param file.";
    exit(1);

    NIDS_raster_header_t tmp;
    memcpy(&tmp, rawPtr, sizeof(NIDS_raster_header_t));
    NIDS_BE_to_raster_header(&tmp);
    // _outGeom.setGeometry(tmp.num_rows, tmp.num_rows, );
  }
  else {
    // Check that the num rows is the same as the param file.
    NIDS_raster_header_t tmp;
    memcpy(&tmp, rawPtr, sizeof(NIDS_raster_header_t));
    NIDS_BE_to_raster_header(&tmp);
    if (_outGeom.getNx() != tmp.num_rows) {
      cerr << "ERROR: Number of cols in source data file ("
           << tmp.num_rows << ") does not match the number of "
           << "cols specified in the output geometry (" << _outGeom.getNx()
           << "). Remapping is not supported, nor is preserving the "
           << "original geometry, as there is not enough info in the "
           << "NIDS file to make an MDV file." << endl;
      exit(1);
    }
    if (_outGeom.getNy() != tmp.num_rows) {
      cerr << "ERROR: Number of rows in source data file ("
           << tmp.num_rows << ") does not match the number of "
           << "rows specified in the output geometry (" << _outGeom.getNy()
           << "). Remapping is not supported, nor is preserving the "
           << "original geometry, as there is not enough info in the "
           << "NIDS file to make an MDV file." << endl;
      exit(1);
    }
  }

  // check product format

  if (( rawData[0] != 0xba || rawData[1] != 0x07 ) &&
      ( rawData[0] != 0xba || rawData[1] != 0x0f )) {
    cerr << "ERROR - " << _progName << ":RemapRast::processRastFile()" << endl;
    cerr << "File not in raster format: " << filePath << endl;
    delete[] rawData;
    return (-1);
  }

  // compute scale and bias

  _computeScaleAndBias(nhdr);

  // read raster header

  NIDS_raster_header_t rhdr;
  memcpy(&rhdr, rawPtr, sizeof(NIDS_raster_header_t));
  rawPtr += sizeof(NIDS_raster_header_t);
  NIDS_BE_to_raster_header(&rhdr);
  if (_verbose) {
    NIDS_print_raster_hdr(stderr, (char*)"    ", &rhdr);
  }

  // allocate array for raster data,
  // assume number of columns equals number of rows

  ui08 *rasters = new ui08[rhdr.num_rows * rhdr.num_rows];
  memset(rasters, 0, rhdr.num_rows * rhdr.num_rows);
  
  // uncompress rasters, load up array

  if (_uncompressRasters(rhdr, rhdr.num_rows, rawPtr, rasters)) {
    cerr << "File: " << filePath << endl;
    delete[] rasters;
    delete[] rawData;
    return (-1);
  }
  
  // set up output file
  _out.clearVol();
  double radarLat = nhdr.lat / 1000.0;
  double radarLon = nhdr.lon / 1000.0;
  double radarHt = 0.5;
  _out.setRadarPos(radarLat, radarLon, radarHt, 0.5);

  _out.initHeaders(_outGeom.getNx(), _outGeom.getNy(),
                   _outGeom.getDx(), _outGeom.getDy(),
                   _outGeom.getMinx(), _outGeom.getMiny(),
                   _radarName.c_str(),
                   _dataFieldNameLong.c_str(),
                   _dataFieldName.c_str(),
                   _dataUnits.c_str(),
                   _dataTransform.c_str(),
                   _dataFieldCode,
                   false);

  _out.loadScaleAndBias(_scale, _bias);

  // perform remapping

  _doRemapping(rasters);

  // write the volume

  time_t scanTime =
    (nhdr.vsdate - 1) * 86400 + nhdr.vstime * 65536 + nhdr.vstim2;

  string outDir = _outputDir;
  if (_verbose) {
    cerr << "Sending Output to: " <<  outDir << endl;
  }


  ta_makedir_recurse(outDir.c_str());
  if (_out.writeVol(scanTime, outDir.c_str())) {
    cerr << "ERROR - " << _progName << ":RemapRast::processRastFile()" << endl;
    cerr << "Cannot write MDV output file to dir: " << _outputDir << endl;
    delete[] rasters;
    delete[] rawData;
    return (-1);
  }

  // free up

  delete[] rasters;
  delete[] rawData;

  setLastInputFile(filePath);
  setLastOutputFile(_out.getLastOutputFile());

  return (0);

}

///////////////////////////////////////////////////////
// _uncompressRasters()
//
// uncompress rasters, load up array

int RemapRast::_uncompressRasters(const NIDS_raster_header_t &rhdr,
                                  int nRows,
                                  ui08 *rawPtr,
                                  ui08 *rasters)

{

  if (rhdr.num_rows != _outGeom.getNy()) {
    cerr << "WARNING: Rows in data file (" << rhdr.num_rows
         << ") do not match rows in output grid ("
         << _outGeom.getNy() << ")!!!" << endl;
  }

  for (int irow = 0; irow < rhdr.num_rows; irow++) {
    
    // read in a row

    NIDS_row_header_t rowhdr;
    memcpy(&rowhdr, rawPtr, sizeof(NIDS_row_header_t));
    rawPtr += sizeof(NIDS_row_header_t);
    NIDS_BE_to_row_header(&rowhdr);
    if (_verbose) {
      NIDS_print_row_hdr(stderr, (char*)"    ", &rowhdr);
    }

    // Run Length decode this row

    // Flip in y direction
    //MCP ui08 *rr = rasters + irow * nRows;
    ui08 *rr = rasters + (nRows - 1 - irow) * nRows;

    int nbytes_run = rowhdr.num_bytes;
    int nbins = 0;
    for (int run = 0; run < nbytes_run; run++ ) {
      int drun = *rawPtr >> 4;
      int dcode = *rawPtr & 0xf;
      nbins += drun;
      if (nbins > rhdr.num_rows) {
	cerr << "ERROR - " << _progName << ":RemapRast::processRastFile()" << endl;
	cerr << "Bad row count" << endl;
	return (-1);
      }

      // Set rr (aka rasters) to have byte value that
      // can be written directly to MDV file

      memset(rr, _outputVal[dcode], drun);
      rr += drun;
      rawPtr++;
    }
    
    if (nbins != _outGeom.getNx()) {
      cerr << "WARNING: Cols in data file (" << rowhdr.num_bytes
           << ") do not match cols in output grid ("
           << _outGeom.getNx() << ")!!!" << endl;
    }

    if (_verbose) {
      fprintf(stderr, "    rows expected: %d, number found: %d\n",
	      rhdr.num_rows, nbins);
    }

  } // irow

  return (0);

}

///////////////////////////////////////                    
// _doRemapping                                          
//                                      
// Perform remapping into output grid   
  
void RemapRast::_doRemapping(ui08 *rasters)

{
  
  ui08 *cart = _out.getFieldPlane();
  
//for (int i = 0; i < 16; i++)
//{
//  fprintf(stderr, "i, _outputVal[i] %2d %d\n", i, _outputVal[i]);
//}

//for (int i = 0; i < _nptsGrid; i++)
//{
//  if (rasters[i] > 0)
//    fprintf(stderr, "i, rasters[i] %6d %d\n", i, rasters[i]);
//}

  int nptsGrid = _outGeom.getNx() * _outGeom.getNy();
  for (int i = 0; i < nptsGrid; i++, cart++)
  {
    // data already converted, ready for MDV file...

    int outval = rasters[i];
    if (outval > *cart)
    {
      *cart = outval;
    }      
    else
    {
      *cart = 0;
    }

  } // i                                      

}

