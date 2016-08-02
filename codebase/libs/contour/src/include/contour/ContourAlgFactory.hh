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
 *  $Id: ContourAlgFactory.hh,v 1.3 2016/03/03 18:17:42 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	ContourAlgFactory
// 
// Author:	G. M. Cunning
// 
// Date:	Wed Jan  7 12:29:16 2004
// 
// Description:	Implements the Factory pattern to create different type 
//		of contouring algorithms.
// 
// 


# ifndef    CONTOUR_ALG_FACTORY_H
# define    CONTOUR_ALG_FACTORY_H

// C++ include files
#include <string>

// System/RAP include files

// Local include files
#include <contour/ContourAlg.hh>

using namespace std;

// forward declarations

class ContourAlgFactory {
  
public:

  ////////////////////
  // public members //
  ////////////////////

  typedef enum {
    SIMPLE_BOUNDARY_ALG = 0
  } alg_type_t;
  
  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  ContourAlgFactory();
  ContourAlgFactory(const ContourAlgFactory &);

  // destructor
  virtual ~ContourAlgFactory();

  // Create the given contour algorithm based on information in the
  // parameter file.
  //
  // Returns a pointer to the ContourAlg object if the creation was
  // successful, 0 otherwise.

  static ContourAlg *createContourAlg(const alg_type_t& alg_type,
				      const bool& debug_flag = false);


protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////

  /////////////////////
  // private methods //
  /////////////////////

  static ContourAlg *_createSimpleBoundaryContourAlg(const bool& debug_flag = false);


};


# endif     /* CONTOUR_ALG_FACTORY_H */
