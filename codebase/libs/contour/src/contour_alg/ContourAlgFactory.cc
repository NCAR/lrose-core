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
/*
 *  $Id: ContourAlgFactory.cc,v 1.5 2016/03/03 18:17:41 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	ContourAlgFactory
//
// Author:	G. M. Cunning
//
// Date:	Wed Jan  7 12:23:00 2004
//
// Description: Implements the Factory pattern to create different 
//		type of contouring algorithms.
//
//
//


// C++ include files
#include <iostream>

// System/RAP include files
#include <toolsa/os_config.h>

// Local include files
#include <contour/SimpleBoundaryContourAlg.hh>
#include <contour/ContourAlg.hh>
#include <contour/ContourAlgFactory.hh>

using namespace std;


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

ContourAlgFactory::ContourAlgFactory()
{
  // do nothing
}

ContourAlgFactory::ContourAlgFactory(const ContourAlgFactory &)
{
  // do nothing
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructor
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
ContourAlgFactory::~ContourAlgFactory()
{
  // do nothing
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	ContourAlgFactory::createContourAlg
//
// Description:	Creates a contour algorithm object
//
// Returns:	
//
// Notes:
//
//

ContourAlg* 
ContourAlgFactory::createContourAlg(const alg_type_t& alg_type,
				    const bool& debug_flag)
{
  ContourAlg *contourAlg = 0;

  switch(alg_type) {
  case SIMPLE_BOUNDARY_ALG:
    contourAlg = _createSimpleBoundaryContourAlg(debug_flag);
    break;
  default:
    cerr << "Unknown algorithm type." << endl;
    break;
  } // endswitch -- alg_type

  return contourAlg;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	ContourAlgFactory::_createSimpleContourAlg
//
// Description:	creates a contour alg object.
//
// Returns:	
//
// Notes:
//
//

ContourAlg* 
ContourAlgFactory::_createSimpleBoundaryContourAlg(const bool& debug_flag)
{
  return new SimpleBoundaryContourAlg(0, 10, 
				      SimpleBoundaryContourAlg::CENTER_REF,
				      debug_flag);
}


