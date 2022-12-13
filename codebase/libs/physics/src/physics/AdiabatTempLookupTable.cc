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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:43:53 $
//   $Id: AdiabatTempLookupTable.cc,v 1.8 2016/03/03 18:43:53 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AdiabatTempLookupTable : Class controlling the lookup table for
 *                          obtaining temperature in K on a pseudoadiabat,
 *                          given pressure in hPa and theta_e in K.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2001
 *
 * Nancy Rehak
 * Based on RIP code from the MM5 model.
 *
 *********************************************************************/


#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>
#include <physics/AdiabatTempLookupTable.hh>
using namespace std;


// Define constants

const double AdiabatTempLookupTable::MISSING_DATA_VALUE = 1.0e9;
const int AdiabatTempLookupTable::NUM_COMMENT_LINES = 14;


/*********************************************************************
 * Constructor
 */

AdiabatTempLookupTable::AdiabatTempLookupTable(const string &lookup_filename) :
  _lookupFilename(lookup_filename),
  _lookupGenerated(false),
  _numPressureLevels(0),
  _numThetaELevels(0),
  _pressureLevels(NULL),
  _thetaELevels(NULL),
  _lookupTable(NULL)
{
}


/*********************************************************************
 * Destructor
 */

AdiabatTempLookupTable::~AdiabatTempLookupTable()
{
  _freeArrays();
}


/*********************************************************************
 * getTemperature() - Retrieves the temperature on a pseudoadiabat
 *                    given pressure in hPa and theta_e in K.
 *
 * Returns the calculated temperature value from the table on success.
 * Returns MISSING_DATA_VALUE if there is an error calculating the value.
 */

double AdiabatTempLookupTable::getTemperature(const double pressure,
					      const double theta_e)
{
  static const string method_name = "AdiabatTempLookupTable::getTemperature()";
  
  // Make sure the lookup table has been read in

  if (!_lookupGenerated)
  {
    if (!_loadTableFromFile())
      return MISSING_DATA_VALUE;
  }
  
  // Look for the theta_e level in the theta_e level table

  int theta_e_index = -1;
  for (unsigned int i = 0; i < _numThetaELevels - 1; ++i)
  {
    if (theta_e >= _thetaELevels[i] &&
	theta_e < _thetaELevels[i+1])
    {
      theta_e_index = i;
      break;
    }
  }
  
  // Look for the pressure level in the pressure level table

  int pressure_index = -1;
  for (unsigned int i = 0; i < _numPressureLevels - 1; ++i)
  {
    if (pressure <= _pressureLevels[i] &&
	pressure > _pressureLevels[i+1])
    {
      pressure_index = i;
      break;
    }
  }
  
  if (theta_e_index < 0 || pressure_index < 0)
  {
    //cerr << "ERROR: " << method_name << endl;
    //cerr << "Outside of lookup table bounds" << endl;
    //cerr << "pressure = " << pressure << ", theta_e = " << theta_e << endl;
    
    return MISSING_DATA_VALUE;
  }
  
  // See if any of the lookup table temperature values are missing

  if (_lookupTable[_calcIndex(pressure_index, theta_e_index)] > MISSING_DATA_VALUE ||
      _lookupTable[_calcIndex(pressure_index+1, theta_e_index)] > MISSING_DATA_VALUE ||
      _lookupTable[_calcIndex(pressure_index, theta_e_index+1)] > MISSING_DATA_VALUE ||
      _lookupTable[_calcIndex(pressure_index+1, theta_e_index+1)] > MISSING_DATA_VALUE)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Tried to access missing temperature in lookup table." << endl;
    cerr << "Pressure and theta_e probably unreasonable." << endl;
    cerr << "Pressure = " << pressure << ", theta_e = " << theta_e << endl;
    
    return MISSING_DATA_VALUE;
  }
  
  // Now calculate the temperature value using interpolation

  double pressure_frac1 = (_pressureLevels[pressure_index] - pressure) /
    (_pressureLevels[pressure_index] - _pressureLevels[pressure_index+1]);
  double pressure_frac2 = 1.0 - pressure_frac1;
  
  double theta_e_frac1 = (theta_e - _thetaELevels[theta_e_index]) /
    (_thetaELevels[theta_e_index+1] - _thetaELevels[theta_e_index]);
  double theta_e_frac2 = 1.0 - theta_e_frac1;
  
  return (pressure_frac2 * theta_e_frac2 *
	  _lookupTable[_calcIndex(pressure_index, theta_e_index)]) +
    (pressure_frac1 * theta_e_frac2 *
     _lookupTable[_calcIndex(pressure_index+1, theta_e_index)]) +
    (pressure_frac2 * theta_e_frac1 *
     _lookupTable[_calcIndex(pressure_index, theta_e_index+1)]) +
    (pressure_frac1 * theta_e_frac1 *
     _lookupTable[_calcIndex(pressure_index+1, theta_e_index+1)]);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

static char *STRdup(const char *s)
 
{
  char *ret_string = (char *) malloc(strlen(s) + 1);
  strcpy(ret_string, s);
  return(ret_string);
}

#ifndef EOS
#define EOS ((char) 0)
#endif

static char *STRncopy(char *s1, const char *s2, int maxs1)
{
  if (!s1 || !s2)
    return NULL;
  
  if (maxs1 > 0)
  {
    strncpy(s1, s2, (size_t) (maxs1-1));
    s1[maxs1-1] = EOS;
  }
  return(s1);
}

static char *STRconcat(char *s1, const char *s2, int maxs1)
{
  int maxc; /* maximum chars to append */
  
  maxc = maxs1 - (int) strlen(s1) - 1;
  
  return strncat(s1, s2,  (size_t) maxc);
}

/*******************************************************
 *
 * file_uncompress()
 *
 * Uncompresses file if:
 *  (a) file is compressed and
 *  (b) the uncompressed file doesn't already exist.
 *
 * Search is done for compressed or gzipped files.
 *
 * Returns 1 if uncompression done, 0 if not, -1 if error
 */

#define MAX_PATH_LEN 1024

int AdiabatTempLookupTable::_file_uncompress(const char *path)

{

  char compressed_path[MAX_PATH_LEN];
  char call_str[BUFSIZ];
  struct stat file_stat;
  int iret;

  /*
   * if file uncompressed file exists, return now
   */

  if (stat(path, &file_stat) == 0) {
    return (0);
  }
    
  /*
   * Check if the compressed file exists.
   * If this is the case, uncompress the file.
   */

  STRncopy(compressed_path, path, MAX_PATH_LEN);
  STRconcat(compressed_path, ".Z", MAX_PATH_LEN);

  if (stat(compressed_path, &file_stat) == 0) {
    
    /*
     * uncompress file
     */
    
    sprintf(call_str, "uncompress -f %s", compressed_path);

    iret = system (call_str);

    if (iret) {
      fprintf(stderr, "WARNING - could not uncompress file\n");
      fprintf(stderr, "  Return from %s: iret = %d\n",
	      call_str, iret);
      return -1;
    } else {
      return 0;
    }

  }

  /*
   * Check if the gzipped file exists.
   * If this is the case, gunzip the file.
   */

  STRncopy(compressed_path, path, MAX_PATH_LEN);
  STRconcat(compressed_path, ".gz", MAX_PATH_LEN);
  
  if (stat(compressed_path, &file_stat) == 0) {

    /*
     * gunzip file
     */
    
    sprintf(call_str, "gunzip -f %s", compressed_path);

    iret = system (call_str);

    if (iret) {
      fprintf(stderr, "WARNING - could not gunzip file\n");
      fprintf(stderr, "  Return from %s: iret = %d\n",
	      call_str, iret);
      return -1;
    } else {
      return 0;
    }
    
  }

  /*
   * Check if the bzipped file exists.
   * If this is the case, gunzip the file.
   */

  STRncopy(compressed_path, path, MAX_PATH_LEN);
  STRconcat(compressed_path, ".bz2", MAX_PATH_LEN);
  
  if (stat(compressed_path, &file_stat) == 0) {

    /*
     * gunzip file
     */
    
    snprintf(call_str, BUFSIZ, "bunzip2 -f %s", compressed_path);

    iret = system (call_str);

    if (iret) {
      fprintf(stderr, "WARNING - could not bunzip2 file\n");
      fprintf(stderr, "  Return from %s: iret = %d\n",
	      call_str, iret);
      return -1;
    } else {
      return 0;
    }
    
  }

  /*
   * file does not exist
   */

  return 0;

}

/*********************************************************
 * _fopen_uncompress()
 *
 * Uncompresses the file if necessary, then opens it
 *
 * Return is identical to fopen()
 */

FILE* AdiabatTempLookupTable::_fopen_uncompress(const char *filename, const char *type)

{
  FILE *local_file;
  char *filename_copy = STRdup(filename);
  
  _file_uncompress(filename_copy);
  local_file = fopen(filename_copy, type);

  free(filename_copy);
  
  return local_file;
}

/*********************************************************************
 * _loadTableFromFile() - Load the lookup table from the file.
 *
 * Returns true if the table was loaded successfully, false otherwise.
 */

bool AdiabatTempLookupTable::_loadTableFromFile()
{
  static const string method_name = "AdiabatTempLookupTable::_loadTableFromFile()";
  
  // Initialize the lookup generated flag.  Then, if we have an error
  // reading the lookup file the object will still act properly.

  _lookupGenerated = false;
  
  // Reclaim memory from any previously loaded lookup file

  _numPressureLevels = 0;
  _numThetaELevels = 0;

  _freeArrays();
  
  // Now try to read in the lookup table from the file

  // Open the file.  Make a copy of the filename because
  // ta_fopen_uncompress() has a bad side-effect of altering
  // the filename sent into it.

  FILE *lookup_table_file;
  char *uncompressed_filename = strdup(_lookupFilename.c_str());
  
  if (uncompressed_filename == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error allocating space for filename string copy" << endl;
    
    return false;
  }
  
  if ((lookup_table_file = _fopen_uncompress(uncompressed_filename, "r"))
      == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening lookup table file: " << _lookupFilename << endl;
    
    free(uncompressed_filename);
    
    return false;
  }
  
  free(uncompressed_filename);
  
  // Skip the comment lines at the top of the file

  char lineBuffer[BUFSIZ];
  for (int i = 0; i < NUM_COMMENT_LINES; ++i)
    fgets(lineBuffer, BUFSIZ, lookup_table_file);
  
  // Read in the table dimensions

  if (fscanf(lookup_table_file, "%u %u",
	     &_numThetaELevels, &_numPressureLevels) != 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in table dimensions from lookup table file: " <<
      _lookupFilename << endl;
    
    fclose(lookup_table_file);
    
    return false;
  }
  
  // Allocate space for the table arrays

  _pressureLevels = new double[_numPressureLevels];
  _thetaELevels = new double[_numThetaELevels];
  
  _lookupTable = new double[_numPressureLevels * _numThetaELevels];
  
  // Read in the theta_e levels from the file

  for (unsigned int i = 0; i < _numThetaELevels; ++i)
  {
    if (fscanf(lookup_table_file, "%lf", &_thetaELevels[i]) != 1)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading in thetaELevel[" << i <<
	"] from lookup table file: " << _lookupFilename << endl;
      
      fclose(lookup_table_file);
      
      return false;
    }
  } /* endfor - i */
  
  // Read in the pressure levels from the file

  for (unsigned int i = 0; i < _numPressureLevels; ++i)
  {
    if (fscanf(lookup_table_file, "%lf", &_pressureLevels[i]) != 1)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading in pressureLevel[" << i <<
	"] from lookup table file: " << _lookupFilename << endl;
      
      fclose(lookup_table_file);
      
      return false;
    }
  } /* endfor - i */
  
  // Read in the temperature values from the file

  for (unsigned int theta_e = 0; theta_e < _numThetaELevels; ++theta_e)
  {
    for (unsigned int pressure = 0; pressure < _numPressureLevels; ++pressure)
    {
      int index = _calcIndex(pressure, theta_e);
      
      if (index < 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error reading temperature value from lookup table file: " <<
	  _lookupFilename << endl;
	cerr << "Invalid pressure index/theta_e index combination" << endl;
	cerr << "pressure index = " << pressure << ", theta_e index = " <<
	  theta_e << endl;
	
	fclose(lookup_table_file);
	
	return false;
      }
      
      if (fscanf(lookup_table_file, "%lf", &_lookupTable[index]) != 1)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error reading temperature value from lookup table file: " <<
	  _lookupFilename << endl;
	cerr << "pressure index = " << pressure << ", theta_e index = " <<
	  theta_e << endl;
	
	fclose(lookup_table_file);
	
	return false;
      }
      
    } /* endfor - pressure */
  } /* endfor - theta_e */
  
  // If we get here, everything was read in successfully.  Clean up and
  // return.

  fclose(lookup_table_file);
  
  _lookupGenerated = true;
  
  return true;
}

/*********************************************************************
 * free up arrays
 */

void AdiabatTempLookupTable::_freeArrays()
{
  if (_pressureLevels) {
    delete[] _pressureLevels;
    _pressureLevels = NULL;
  }
  if (_thetaELevels) {
    delete[] _thetaELevels;
    _thetaELevels = NULL;
  }
  if (_lookupTable) {
    delete[] _lookupTable;
    _lookupTable = NULL;
  }
}


