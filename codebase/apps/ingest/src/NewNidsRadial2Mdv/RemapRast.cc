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
// Based on Mike Dixon's NewNidsRadial2Mdv program
//
// April 1999
//
//////////////////////////////////////////////////////////

#include "RemapRast.hh"
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
using namespace std;

// constructor

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

// destructor

RemapRast::~RemapRast()

{
}

// process file

// virtual 
int RemapRast::processFile(const string &filePath)

{

  if (_debug) {
    cerr << _progName << " - processing raster file: " << filePath << endl;
  }

  // open file

  FILE *in;
  if ((in = fopen (filePath.c_str(), "rb")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":RemapRast::processRastFile()" << endl;
    cerr << "Cannot open file: " << filePath << endl;
    cerr << strerror(errNum);
    return (-1);
  }

  //MCP Sleep to make sure file is finished writing (fix) to check activity
  struct stat file_stat;

  if (stat(filePath.c_str(), &file_stat) != 0)
  {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":RemapRast::processRastFile()" << endl;
    cerr << "Cannot stat file: " << filePath << endl;
    cerr << strerror(errNum);
    return (-1);
  }

  // Check to see if the file is quiescent

  int prev_filesize = file_stat.st_size;                                                           
  const int FOREVER = true;
  while (FOREVER)                                                                                  
  {
    if (stat(filePath.c_str(), &file_stat) != 0)                                                           
    {
      int errNum = errno;
      cerr << "ERROR - " << _progName << ":RemapRast::processRastFile()" << endl;
      cerr << "Cannot stat file: " << filePath << endl;
      cerr << strerror(errNum);
      return (-1);                                                                                
    }
    else                                                                                           
    {

      if (_processingDelay > 0)                                                                    
      {
        fprintf(stderr, "---> Sleeping %ld seconds\n", _processingDelay);                           
        sleep(_processingDelay);                                                                   
      }

      if (file_stat.st_size == prev_filesize)                                                      
        break;                                                                                     
      else                                                                                         
        sleep(_processingDelay);                                                                   

    } /* endif stat(filename) */                                                                   

  } /* endwhile - FOREVER */                                                                       
  //MCP Quiescent

  // read in NIDS header

  NIDS_header_t nhdr;
  if (ufread(&nhdr, sizeof(NIDS_header_t), 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":RemapRast::processRastFile()" << endl;
    cerr << "Cannot read header from file: " << filePath << endl;
    cerr << strerror(errNum);
    fclose(in);
    return (-1);
  }
  NIDS_BE_to_mess_header(&nhdr);
  if (_verbose) {
    NIDS_print_mess_hdr(stderr, "  ", &nhdr);
  }

  // Allocate space for the data, read in the data

  ui08 *rawData = new ui08[nhdr.lendat];
  ui08 *rawPtr = rawData;
  
  if (ufread(rawData, nhdr.lendat, 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << ":RemapRast::processRastFile()" << endl;
    cerr << "Cannot read raster data from file: " << filePath << endl;
    cerr << strerror(errNum);
    fclose(in);
    delete[] rawData;
    return (-1);
  }
  fclose(in);
  
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
    NIDS_print_raster_hdr(stderr, "    ", &rhdr);
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

  _out.initHeaders(_nx, _ny,
                   _dx, _dy,
                   _minx, _miny,
                   _radarName.c_str(),
                   _dataFieldNameLong.c_str(),
                   _dataFieldName.c_str(),
                   _dataUnits.c_str(),
                   _dataTransform.c_str(),
                   _dataFieldCode);

  _out.loadScaleAndBias(_scale, _bias);

  // perform remapping

  _doRemapping(rasters);

  // write the volume

  time_t scanTime =
    (nhdr.vsdate - 1) * 86400 + nhdr.vstime * 65536 + nhdr.vstim2;

  string outDir = _outputDir;
  outDir += PATH_DELIM;
  outDir += _radarName;

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

  for (int irow = 0; irow < rhdr.num_rows; irow++) {
    
    // read in a row

    NIDS_row_header_t rowhdr;
    memcpy(&rowhdr, rawPtr, sizeof(NIDS_row_header_t));
    rawPtr += sizeof(NIDS_row_header_t);
    NIDS_BE_to_row_header(&rowhdr);
    if (_verbose) {
      NIDS_print_row_hdr(stderr, "    ", &rowhdr);
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

  for (int i = 0; i < _nptsGrid; i++, cart++)
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

