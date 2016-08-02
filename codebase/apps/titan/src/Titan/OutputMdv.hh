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
/////////////////////////////////////////////////////////////
// OutputMdv.hh
//
// OutputMdv class - handles debug output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#ifndef OutputMdv_HH
#define OutputMdv_HH

#include "Worker.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
using namespace std;

// forward declarations
class InputMdv;

class OutputMdv : public Worker {
  
public:
  
  // constructor
  
  OutputMdv(const string &prog_name,
	    const Params &params,
	    const InputMdv &input_mdv,
	    const string &info,
	    const string &data_set_name,
	    const string &url);
  
  // destructor
  
  virtual ~OutputMdv();
  
  // addField()

  void addField(const char *field_name_long,
		const char *field_name,
		const char *units,
		const char *transform,
		fl32 scale,
		fl32 bias,
		const ui08 *data);

  // for floats

  void addField(const char *field_name_long,
                const char *field_name,
                const char *units,
                const char *transform,
                const fl32 *data);
  
  // write out merged volume

  int writeVol();

  // data members
  static const float missingVal;

protected:
  
private:
  
  const InputMdv &_inputMdv;
  const string _url;
  DsMdvx _mdvx;

};

#endif

