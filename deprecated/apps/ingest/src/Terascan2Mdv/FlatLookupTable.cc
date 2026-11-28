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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * FlatLookupTable.cc : Class controlling the lookup table used for flat
 *                      projections in Terascan2Mdv.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2002
 *
 * Gary Blackburn
 *
 * Based on work by Nancy Rehak
 *
 *********************************************************************/

#include <cmath>
#include <cstdio>
#include <new>
#include <cstdlib>
#include <cstddef>
#include <cctype>
#include <iostream>
//#include <sys/types.h>
#include <sys/stat.h>


#include <toolsa/pjg.h>

#include "FlatLookupTable.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

FlatLookupTable::FlatLookupTable(const string &lookup_filename) :
  _lookupFilename(lookup_filename),
  _lookupGenerated(false)
{
}


/*********************************************************************
 * Destructor
 */

FlatLookupTable::~FlatLookupTable()
{
  // Do nothing
}


/*********************************************************************
 * generate() - Generate a lookup table with the given parameters.
 *              If there is a lookup table saved to disk, read that
 *              table and use it.  Otherwise, generate the table and
 *              write it to disk.
 */

bool FlatLookupTable::generate(const int num_lines, const int num_samples,
			       const double delta_x, const double delta_y,
			       ETXFORM mxfm)
{
  const string method_name = "FlatLookupTable::generate()";
  
  // Save the needed information

  _numLines = num_lines;
  _numSamples = num_samples;
  
  // See if we can load the lookup table from the file

  if (_loadTableFromFile())
    return false;
  
  // Initialize the internal arrays

  // Store input lat/lon values converted to x/y coordinates
  double *x_in;
  double *y_in ;
  if(!(x_in = new(nothrow) double[_numLines * _numSamples])) {
    cerr << "new failed to allocate " << sizeof(double)*_numLines*_numSamples << " bytes." << endl;
    return false;
  }
  if(!(y_in = new(nothrow) double[_numLines * _numSamples])) {
    cerr << "new failed to allocate " << sizeof(double)*_numLines*_numSamples << " bytes." << endl;
    return false;
  }

  // Store output FLAT earth values / multiples of delta x or y 
  double *x_out;
  double *y_out;
  if(!(x_out = new(nothrow) double[_numLines * _numSamples])) {
    cerr << "new failed to allocate " << sizeof(double)*_numLines*_numSamples << " bytes." << endl;
    return false;
  }
  if(!(y_out = new(nothrow) double[_numLines * _numSamples])) {
    cerr << "new failed to allocate " << sizeof(double)*_numLines*_numSamples << " bytes." << endl;
    return false;
  }
  
  /* Define the corresponding km x/y pairs for the input lat/lon data */

  int cnt = 0;
  for (int line = _numLines; line >= 1; line--)
  {
    for (int sample = 1; sample <= _numSamples; sample++)
    {
      double lat, lon;
      double dx, dy;
	
      etxll(mxfm, line, sample, &lat, &lon);

      PJGflat_latlon2xy(lat, lon, &dx, &dy);

      x_in[cnt] = dx;
      y_in[cnt] = dy;

      /* assume we want the same number of grid points as the lat/lon
         data set, just on a flat km projection */
      x_out[cnt] = (sample - _numSamples/2) * delta_x;
      y_out[cnt] = (_numLines/2 - line) * delta_y;

      cnt++;
    }
  }

  // Now calculate the _mapOutput array

  int window = 30;
  cnt = 0;
  int ii; // number of lines (y values) in window
  int jj; // number of columns (x values) in the window

  if(!(_mapOutput = new(nothrow) int[_numLines * _numSamples])) {
    cerr << "new failed to allocate " << sizeof(int)*_numLines*_numSamples << " bytes." << endl;
    return false;
  }
   

  for (int i = 0; i < _numLines; i++)
  {
    for (int j = 0; j < _numSamples; j++)
    {
      // calculate output grid cell index
      int index = i * _numSamples +j;
      cnt = 0;

      // box used to find corresponding input data (based on latlon) grid cell 
      int loweri = i - window;
      int upperi = i + window;

      if (loweri < 0) loweri = 0;
      if (upperi > _numLines) upperi = _numLines;

      int lowerj = j - window;
      int upperj = j + window;

      if (lowerj < 0) lowerj = 0;
      if (upperj > _numSamples) upperj = _numSamples;

      // starting a the bottom of the box, search in the y direction
      for (ii = loweri; ii < upperi; ii++)
      {
	cnt = ii * _numSamples;
        // compare flat grid spacing to lat/lon converted to x/y spacing
        if (y_out[index] > y_in[cnt])
        {
          cnt += _numSamples;
        }
        else
        {
          // search in the x direction
	  for (jj = lowerj; jj < upperj; jj++)
	  {
	    cnt = ii * _numSamples + jj;
	    if (x_out[index] <= x_in[cnt])
            {
	      break;
	    }
	  }

	  int cnti = ii;
	  int cntj = jj;

	  /* Provide one iteration on Y, now that we've found a match in X */
	  if (fabs(y_out[index] <= y_in[cnt]) > 1.5)
	  {
	    for (int i2 = loweri; i2 < upperi; i2++)
            {
	      cnt = i2 * _numSamples + cntj;
              if (y_out[index] < y_in[cnt])
              {
                cnti = i2;
                break;
              }
	    }
	  }
          cnt = cnti * _numSamples + cntj;
          // cout << "in: " << x_in[cnt] << ", " << y_in[cnt] << "  out: " << x_out[index] << ", " << y_out[index] << endl;
 
          break;

	}  // if
      } // for ii=
      _mapOutput[index] = cnt;

    }   /* for j= */
  }     /* for i= */
  //  fclose(file_ptr);

  delete [] x_in;
  delete [] y_in;
  delete [] x_out;
  delete [] y_out;

  // Write the lookup table to the lookup file

  _writeTableToFile();

  return true;
}


/*********************************************************************
 * getIndex() - 
 */

int FlatLookupTable::getIndex(int i0, int j0)
{
  return _mapOutput[i0 * _numSamples + j0];
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _loadTableFromFile() - Load the lookup table from the file.
 *
 * Returns true if the table was loaded successfully, false otherwise.
 */

bool FlatLookupTable::_loadTableFromFile()
{
  const string method_name = "FlatLookupTable::_loadTableFromFile()";
  
  // Precalculate needed value

  int num_elements = _numLines * _numSamples;
  
  // See if the lookup file looks like it matches the data we're
  // looking at

  struct stat file_stat;
  
  if (stat(_lookupFilename.c_str(), &file_stat) != 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Error stating input lookup table file" << endl;
    perror(_lookupFilename.c_str());
    
    return false;
  }
  
  if ((int)file_stat.st_size != (int)(num_elements * sizeof(int)))
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Lookup file size does not match expected size" << endl;
    cerr << "File size: " << file_stat.st_size << " bytes" << endl;
    cerr << "Expected size: " << num_elements * sizeof(int) << " bytes" << endl;
    cerr << "Not using lookup table file" << endl;
    
    return false;
  }
  
  // Open the input file

  FILE *file_ptr;

  if ((file_ptr = fopen(_lookupFilename.c_str(), "r")) == 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Error opening input file" << endl;
    perror(_lookupFilename.c_str());
    
    return false;
  }
  
  // Now read in the _mapOutput array

  _mapOutput = new int[num_elements];

  if ((int)(fread((void *)_mapOutput, sizeof(int), num_elements, file_ptr))
      != num_elements)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading lookup table values from file" << endl;
    perror(_lookupFilename.c_str());
      
    fclose(file_ptr);
      
    return false;
  }

  // Finally, close the input file and return

  fclose(file_ptr);

  return true;
}


/*********************************************************************
 * _writeTableToFile() - Writes the lookup table to the file.
 *
 * Returns true if the table was loaded successfully, false otherwise.
 */

bool FlatLookupTable::_writeTableToFile()
{
  const string method_name = "FlatLookupTable::_writeTableToFile()";
  
  // Open the output file

  FILE *file_ptr;

  if ((file_ptr = fopen(_lookupFilename.c_str(), "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening output file" << endl;
    perror(_lookupFilename.c_str());
    
    return false;
  }

  // Write the _mapOutput array to the file

  int num_elements = _numLines * _numSamples;
  
  if ((int)fwrite((void *)_mapOutput, sizeof(int), num_elements, file_ptr)
      != num_elements)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing to output file" << endl;
    perror(_lookupFilename.c_str());

    fclose(file_ptr);
    
    return false;
  }
  
  // Close the output file

  fclose(file_ptr);

  return true;
}
