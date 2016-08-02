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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:10:59 $
//   $Id: RefFile.cc,v 1.3 2016/03/03 18:10:59 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * RefFile: Class controlling access to a refractivity calibration reference
 *          file.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2009
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Refract/RefFile.hh>
#include <cstdio>

using namespace std;


/*********************************************************************
 * Constructors
 */

RefFile::RefFile() :
  _debug(false),
  _verbose(false),
  _numBeams(0),
  _numGates(0),
  _refN(0.0),
  _calibRef(0)
{
}

  
/*********************************************************************
 * Destructor
 */

RefFile::~RefFile()
{
  delete [] _calibRef;
}


/*********************************************************************
 * loadFile()
 */

bool RefFile::loadFile(const string &ref_file_name,
		       const int num_beams,
		       const int num_gates)
{
  static const string method_name = "RefFile::loadFile()";
  
  // Open the file

  FILE *ref_fp = fopen(ref_file_name.c_str(), "rb");
  if (ref_fp == NULL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot open reference file " << ref_file_name << endl;

    return false;
  }

  // Read the reference N value

  if (fread(&_refN, sizeof(float), 1, ref_fp) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading reference N value from calibration file." << endl;
    
    return false;
  }
  
  // Allocate space and read the calibration information

  _numGates = num_gates;
  _numBeams = num_beams;
  
  size_t data_size = _numGates * _numBeams;
  
  delete [] _calibRef;
  _calibRef = new R_info_t[data_size];

  if (fread(_calibRef, sizeof(R_info_t), data_size, ref_fp) != data_size)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading calibration data from calibration file." << endl;
    
    return false;
  }
  
  // Close the file

  fclose(ref_fp);

  if (_debug)
    cerr << "Reference N = " << _refN << endl;

  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
