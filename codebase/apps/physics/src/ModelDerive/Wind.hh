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
/* RCS info
 *   $Author: dixon $
 *   $Date: 2016/03/06 23:15:37 $
 *   $Revision: 1.2 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/************************************************************************
 * Wind.hh: File containing class declarations to derive wind variables
 *
 * How to add a new derivable variable:
 *  1) Create a new function class declaration in this file. (example below)
 *  2) Create the constructor/derive/destruct and setOutputNames class functions
 *     in the accompanying .cc file. (Example at top of .cc file)
 *  3) Add the class to the DeriveInterface.hh file getDeriveClassFromName and
 *     setOutputNamesFromClassName functions.
 *  4) Class can now be called by name from the parameter file. Add documentation
 *     about the derive function in paramdef.ModelDerive derive_functions variable.
 *
 *  Example class declaration:  Copy and paste as is changing only the class
 *                               name to your derive function name.
 *

class VariableFromVar1Var2: public DeriveBase {

public:

  VariableFromVar1Var2(const float missing, const float bad, int nx, int ny, int nz);
  
  virtual ~VariableFromVar1Var2();

  int derive(vector<float*> *inputs, vector<float*> *outputs);

  static void setOutputNames();

};

 *
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/

#ifndef WIND_HH
#define WIND_HH

#include <grib2/ProdDefTemp.hh>

#include "DeriveBase.hh"

using namespace std;


class WindSpeedDirectionFromUV: public DeriveBase {

public:

  WindSpeedDirectionFromUV(const float missing, const float bad, int nx, int ny, int nz);
  
  virtual ~WindSpeedDirectionFromUV();

  int derive(vector<float*> *inputs, vector<float*> *outputs);

  static void setOutputNames();

};

#endif
