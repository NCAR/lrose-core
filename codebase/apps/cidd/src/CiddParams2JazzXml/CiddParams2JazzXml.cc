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
//   $Date: 2016/03/07 18:28:24 $
//   $Id: CiddParams2JazzXml.cc,v 1.2 2016/03/07 18:28:24 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CiddParams2JazzXml: CiddParams2JazzXml program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2010
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <string>

#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "CiddParams2JazzXml.hh"
#include "JazzXml.hh"

using namespace std;

// Global variables

CiddParams2JazzXml *CiddParams2JazzXml::_instance =
     (CiddParams2JazzXml *)NULL;


/*********************************************************************
 * Constructor
 */

CiddParams2JazzXml::CiddParams2JazzXml(int argc, char **argv)
{
  static const string method_name = "CiddParams2JazzXml::CiddParams2JazzXml()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (CiddParams2JazzXml *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);

}


/*********************************************************************
 * Destructor
 */

CiddParams2JazzXml::~CiddParams2JazzXml()
{
  // Free contained objects

  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

CiddParams2JazzXml *CiddParams2JazzXml::Inst(int argc, char **argv)
{
  if (_instance == (CiddParams2JazzXml *)NULL)
    new CiddParams2JazzXml(argc, argv);
  
  return(_instance);
}

CiddParams2JazzXml *CiddParams2JazzXml::Inst()
{
  assert(_instance != (CiddParams2JazzXml *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * run()
 */

void CiddParams2JazzXml::run()
{
  static const string method_name = "CiddParams2JazzXml::run()";
  
  // Read the CIDD parameter file

  CiddParamFile cidd_param_file;
  
  if (!cidd_param_file.init(_args->ciddParamFileName))
    return;
  
  // Generate the XML

  JazzXml xml;
  
  if (!xml.init())
    return;
  
  if (!xml.generateXml(cidd_param_file))
    return;
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
