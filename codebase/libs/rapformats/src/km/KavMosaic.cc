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

/*********************************************************************
 * KavMosaic: class manipulating a Kavouras mosaic file.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstring>
#include <rapformats/KavMosaic.hh>
using namespace std;


// Static definitions

const string KavMosaic::_MONTH_ARRAY[] = { "JAN", "FEB", "MAR", "APR",
					   "MAY", "JUN", "JUL", "AUG",
					   "SEP", "OCT", "NOV", "DEC" };


/**********************************************************************
 * Constructor
 */

KavMosaic::KavMosaic()
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

KavMosaic::~KavMosaic()
{
  // Do nothing
}


/**********************************************************************
 * writeFile() - Write the Kavouras file to the given output directory.
 *
 * Returns true on success, false on failure.
 */

bool KavMosaic::writeFile(const string output_dir) {

  // Write the file information to the output buffer.

  _outputBuffer.reset();

  _writeHeaderToOutputBuffer();
  _writeGridToOutputBuffer();
  
  // Write the output buffer to the output file.

  return true;
}


/**********************************************************************
 * PROTECTED METHODS
 **********************************************************************/

/**********************************************************************
 * _writeGridToOutputBuffer() - Write the data grid to the output buffer.
 */

void KavMosaic::_writeGridToOutputBuffer()
{
  for (int y = 0; y < _header.ny; ++y)
  {
    // char *row_ptr = _grid + (y * _header.nx);
    
    
  } /* endfor - y */
  
}


/**********************************************************************
 * _writeHeaderToOutputBuffer() - Write the header information to the
 *                                output buffer.
 */

void KavMosaic::_writeHeaderToOutputBuffer()
{
  char empty_bytes[KAV_DATA_OFFSET];
  
  memset(empty_bytes, 0, sizeof(empty_bytes));
  
#ifdef NEW_DBS_HEADER

  _outputBuffer.add((void *)_header.filename, 12);   // start byte 0
  
  _outputBuffer.add((void *)_header.validtime, 4);   // start byte 12
  
  _outputBuffer.add((void *)empty_bytes, 2);         // start byte 16
  
  _outputBuffer.add((void *)_header.validdate, 6);   // start byte 18

  /* There is an inconsistancy in the docs for this 2 bytes vs 4 bytes */
  _outputBuffer.add((void *)_header.projection, 2);  // start byte 24
  
  ui16 lat0 = (int)(_header.lat0 * 100.0);
  _outputBuffer.add((void *)&lat0, 2);                // start byte 26

  ui16 lon0 = (int)(_header.lon0 * 100.0);
  _outputBuffer.add((void *)&lon0, 2);                // start byte 28

  ui16 lon_width = (int)(_header.lon_width * 100.0);
  _outputBuffer.add((void *)&lon_width, 2);           // start byte 30
  
  ui16 aspect = (int)(_header.aspect * 10000.0);
  _outputBuffer.add((void *)&aspect, 2);              // start byte 32
  
  ui16 nx = _header.nx;
  _outputBuffer.add((void *)&nx, 2);                 // start byte 34
  
  ui16 ny = _header.ny;
  _outputBuffer.add((void *)&ny, 2);                 // start byte 36
  
  _outputBuffer.add((void *)empty_bytes, 6);         // start byte 38
  
  // Output site info

  ui16 nsites = _header.nsites;
  _outputBuffer.add((void *)&nsites, 2);             // start byte 44
  
  for (int i = 0; i < MAX_SITES; ++i)                // start byte 46
    _outputBuffer.add((void *)&(_header.site_id[i][0]), 3);

  for (int i = 0; i < MAX_SITES; ++i)               // start byte 646
  {
    ui16 site_x = _header.site_x[i];
    _outputBuffer.add((void *)&site_x, 2);
  }
  
  for (int i = 0; i < MAX_SITES; ++i)              // start byte 1046
  {
    ui16 site_y = _header.site_y[i];
    _outputBuffer.add((void *)&site_y, 2);
  }
  
  for (int i = 0; i < MAX_SITES; ++i)              // start byte 1446
  {
    ui16 site_status = _header.site_status[i];
    _outputBuffer.add((void *)&site_status, 2);
  }

  // start byte 1846

  _outputBuffer.add((void *)empty_bytes, KAV_DATA_OFFSET - 1846);
  
#else
  
  _outputBuffer.add((void *)_header.day, 2);    // start byte 0
  _outputBuffer.add((void *)_header.mon, 3);    // start byte 2
  _outputBuffer.add((void *)_header.year, 2);   // start byte 5
  
  _outputBuffer.add((void *)empty_bytes, 1);    // start byte 7
  
  _outputBuffer.add((void *)_header.hour, 2);   // start byte 8
  _outputBuffer.add((void *)_header.min, 2);    // start byte 10
  
  _outputBuffer.add((void *)empty_bytes, 1);    // start byte 12
  
  // Output site info
  
  for (int i = 0; i < MAX_SITES; ++i)          // start byte 13
    _outputBuffer.add((void *) &(_header.site_id[i][0]), 3);

  for (int i = 0; i < MAX_SITES; ++i)          // start byte 397
  {
    ui16 site_x = _header.site_x[i];
    _outputBuffer.add((void *)&site_x, 2);
  }
  
  for (int i = 0; i < MAX_SITES; ++i)          // start byte 653
  {
    ui16 site_y = _header.site_y[i];
    _outputBuffer.add((void *)&site_y, 2);
  }
  
  for (int i = 0; i < MAX_SITES; ++i)          // start byte 909
  {
    ui16 site_status = _header.site_status[i];
    _outputBuffer.add((void *)&site_status, 2);
  }

  // start byte 1165

  _outputBuffer.add((void *)empty_bytes, KAV_DATA_OFFSET - 1165);
  
#endif

}
