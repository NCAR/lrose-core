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
/////////////////////////////////////////////////////////
// $Id: Blender.hh,v 1.3 2016/03/04 02:22:10 dixon Exp $
//
// Blends two Mdv files into one
//
// Yan Chen, RAL, NCAR
//
// Dec. 2007
//
//////////////////////////////////////////////////////////

# ifndef    BLENDER_HH
# define    BLENDER_HH

// C++ include files
#include <string>

// System/RAP include files
#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>

// Local include files
#include "Params.hh"
using namespace std;


class Blender {
  
public:

  // constructor
  Blender(const string &prog_name, const Params &params); 


  // destructor
  ~Blender();

  // Blend files
  int blendFiles(DsMdvx &sh_file, DsMdvx &nh_file);

protected:

private:

  const string &_progName;
  const Params &_params;

  bool _files_match(
    Mdvx::master_header_t &mhdr1,
    Mdvx::master_header_t &mhdr2,
    string &err_str
  );

  bool _fields_match(
    MdvxField *field1,
    MdvxField *field2,
    string &err_str
  );

  char* _timeStr(const time_t ttime);

  int _write_output(DsMdvx&, string&);

};


# endif     /* BLENDER_HH */
