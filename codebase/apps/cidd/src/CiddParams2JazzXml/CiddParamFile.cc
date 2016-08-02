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
/**
 *
 * @file CiddParamFile.cc
 *
 * @class CiddParamFile
 *
 * Class representing a CIDD parameter file
 *  
 * @date 9/24/2010
 *
 */

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "CiddParamFile.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

CiddParamFile::CiddParamFile ()
{
}


/**********************************************************************
 * Destructor
 */

CiddParamFile::~CiddParamFile(void)
{
}
  

/**********************************************************************
 * init()
 */

bool CiddParamFile::init(const string &param_file_path)
{
  if (!_readFile(param_file_path))
    return false;
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _readFile()
 */

bool CiddParamFile::_readFile(const string &fname)
{
  static const string method_name = "CiddParams2JazzXml::_readCiddParamFile()";
  
  // Check existance and size of file

  struct stat sbuf;

  if (stat(fname.c_str(), &sbuf) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Parameter file doesn't exist: " << fname << endl;
    
    return false;
  }

  // Allocate space for the whole file plus a null
  
  char *params_buf = new char[sbuf.st_size + 1];
  
  // Open the parameter file

  FILE *infile;

  if ((infile = fopen(fname.c_str(),"r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening CIDD parameter file" << endl;
    perror(fname.c_str());
    
    return false;
  }

  // Read the parameter file
  
  size_t bytes_read;
  
  if ((bytes_read = fread(params_buf, 1, sbuf.st_size, infile))
      != (size_t)sbuf.st_size)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem reading CIDD parameter file" << endl;
    cerr << "Expected " << sbuf.st_size << " bytes, found "
	 << bytes_read << " bytes" << endl;
    
    return false;
  }

  // Null terminate the buffer

  params_buf[sbuf.st_size] = '\0';

  // Close the parameter file file

  fclose(infile);
  
  // Load the parameter sections

  if (!_mainParams.init(params_buf, sbuf.st_size + 1))
    return false;

  if (!_guiConfig.init(_mainParams, params_buf, sbuf.st_size + 1))
    return false;
  
  if (!_grids.init(_mainParams, params_buf, sbuf.st_size + 1))
    return false;
  
  if (!_winds.init(_mainParams, params_buf, sbuf.st_size + 1))
    return false;
  
  if (!_maps.init(_mainParams, params_buf, sbuf.st_size + 1))
    return false;
  
  if (!_symprods.init(_mainParams, params_buf, sbuf.st_size + 1))
    return false;
  
  if (!_terrain.init(_mainParams, params_buf, sbuf.st_size + 1))
    return false;
  
  // Reclaim memory

  delete [] params_buf;
  
  return true;
}
