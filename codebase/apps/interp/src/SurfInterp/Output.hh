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
// Output.hh
//
// Output class - handles the output to MDV files
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
///////////////////////////////////////////////////////////////

#ifndef Output_HH
#define Output_HH

#include <euclid/Pjg.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxTimes.hh> 
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/umisc.h>

#include "Params.hh"

using namespace std;


class Output
{
  
public:
  
  /*********************************************************************
   * Constructors
   */

  Output(const DateTime &data_time, const int data_duration,
	 const Pjg &proj, const float altitude,
         const string &dataset_info, const string &dataset_name,
	 const string &dataset_source, const string &prog_name);


  /*********************************************************************
   * Destructor
   */

  ~Output();


  /*********************************************************************
   * addField() - Add the given field to the output file.
   */

  void addField(const char *var_name, const char *short_var_name,
		const char *var_units,
                const float *data,
		const float bad, const float missing,
		const Mdvx::encoding_type_t encoding_type,
		const Mdvx::scaling_type_t scaling_type,
		const double scale, const double bias);


  /*********************************************************************
   * write() - Write the output file to disk.
   *
   * Returns true on success, false on failure.
   */

  bool write(char *output_url);
  

private:
  
  string _progName;

  DsMdvx _mdvx;
  MdvxPjg _outputProj;
  
};

#endif



















