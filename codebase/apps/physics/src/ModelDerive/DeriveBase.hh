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
 *   $Revision: 1.3 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/************************************************************************
 * DeriveBase: Base abstract class for all model derive operations.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/

#ifndef DeriveBase_HH
#define DeriveBase_HH

using namespace std;

#include <string>
#include <vector>

class DeriveBase {

public:

  DeriveBase(const float missing, const float bad, int nx, int ny, int nz);
  virtual ~DeriveBase();

  virtual int derive(vector<float*> *inputs, vector<float*> *outputs);
  
  static int getNumberOfInputs() {
    return numberOfInputs;
  }

  static int getNumberOfOutputs() {
    return numberOfOutputs;
  }

  static const vector<string> * const getOutputShortNames() {
    if(output_short_names.size() > 0)
      return &output_short_names;
    else
      return NULL;
  }

  static const vector<string> * const getOutputLongNames() {
    if(output_long_names.size() > 0)
      return &output_long_names;
    else
      return NULL;
  }

  static const vector<string> * const getOutputUnits() {
    if(output_units.size() > 0)
      return &output_units;
    else
      return NULL;
  }

protected:

  const float missing;
  const float bad;

  int nx, ny, nz;
  int nPts;

  static vector<string> output_short_names;
  static vector<string> output_long_names;
  static vector<string> output_units;
  static int numberOfInputs;
  static int numberOfOutputs;

private:

};

#endif
