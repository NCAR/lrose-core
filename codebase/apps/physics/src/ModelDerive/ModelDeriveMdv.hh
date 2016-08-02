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
 * ModelDeriveMdv: Mdv file handler for ModelDerive.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/

#ifndef ModelDeriveMdv_HH
#define ModelDeriveMdv_HH

#define _in_ModelDeriveMdv_hh

#include <dsdata/TriggerInfo.hh>

#include "Params.hh"
#include "OutputMdv.hh"

class ModelDeriveMdv
{
public:


  ModelDeriveMdv(Params *params);

  ~ModelDeriveMdv();


  /*********************************************************************
   * _processData() - Process the data for the given time.
   */
  bool processData(TriggerInfo &trigger_info);
  
protected:

private:

  Params *_params;

  vector<char*> fields_to_copy;

  /*********************************************************************
   * _readMdvFile() - Read the MDV file for the given time.
   */
  bool _readMdvFile(DsMdvx &input_mdv,
		    TriggerInfo &trigger_info);

  int _create_verify_input_field_list(DsMdvx &input_mdv);

};

#undef _in_ModelDeriveMdv_hh

#endif
