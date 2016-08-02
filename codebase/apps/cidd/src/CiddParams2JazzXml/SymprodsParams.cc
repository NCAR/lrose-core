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
/**
 *
 * @file SymprodsParams.cc
 *
 * @class SymprodsParams
 *
 * Class controlling access to the SYMPRODS section of the CIDD parameter
 * file.
 *  
 * @date 10/5/2010
 *
 */

#include "Csyprod_P.hh"
#include "SymprodsParams.hh"

using namespace std;

/**********************************************************************
 * Constructor
 */

SymprodsParams::SymprodsParams () :
  TdrpParamSection()
{
}


/**********************************************************************
 * Destructor
 */

SymprodsParams::~SymprodsParams(void)
{
}
  

/**********************************************************************
 * init()
 */

bool SymprodsParams::init(const MainParams &main_params,
			  const char *params_buf, const size_t buf_size)
{
  static const string method_name = "SymprodsParams::init()";
  
  // Pull out the SYMPRODS section of the parameters buffer

  const char *param_text;
  long param_text_line_no = 0;
  long param_text_len = 0;
  
  if ((param_text = _findTagText(params_buf, "SYMPRODS",
				 &param_text_len, &param_text_line_no)) == 0 ||
      param_text == 0 || param_text_len <= 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Couldn't find SYMPRODS section in CIDD parameter file" << endl;
    cerr << "Not processing SYMPRODS parameters" << endl;
    
    return true;
  }
  
  // Load the parameters from the buffer

  Csyprod_P symprod;
  
  if (symprod.loadFromBuf("SYMPRODS TDRP Section",
			  0, param_text, param_text_len, param_text_line_no,
			  false, false) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error loading TDRP parameters from <SYMPRODS> section" << endl;
    
    return false;
  }
  
  // Extract the prod_info array and put each product in the symprods vector

  for (int i = 0; i < symprod.prod_info_n; ++i)
  {
    SymprodField field;
    
    field.menuLabel = symprod._prod_info[i].menu_label;
    field.url = symprod._prod_info[i].url;
    field.dataType = symprod._prod_info[i].data_type;
    switch (symprod._prod_info[i].render_type)
    {
    case Csyprod_P::RENDER_ALL :
      field.renderType = SymprodField::RENDER_ALL;
      break;
    case Csyprod_P::RENDER_ALL_VALID :
      field.renderType = SymprodField::RENDER_ALL_VALID;
      break;
    case Csyprod_P::RENDER_VALID_IN_LAST_FRAME :
      field.renderType = SymprodField::RENDER_VALID_IN_LAST_FRAME;
      break;
    case Csyprod_P::RENDER_LATEST_IN_FRAME :
      field.renderType = SymprodField::RENDER_LATEST_IN_FRAME;
      break;
    case Csyprod_P::RENDER_LATEST_IN_LOOP :
      field.renderType = SymprodField::RENDER_LATEST_IN_LOOP;
      break;
    case Csyprod_P::RENDER_FIRST_BEFORE_FRAME_TIME :
      field.renderType = SymprodField::RENDER_FIRST_BEFORE_FRAME_TIME;
      break;
    case Csyprod_P::RENDER_FIRST_BEFORE_DATA_TIME :
      field.renderType = SymprodField::RENDER_FIRST_BEFORE_DATA_TIME;
      break;
    case Csyprod_P::RENDER_FIRST_AFTER_DATA_TIME :
      field.renderType = SymprodField::RENDER_FIRST_AFTER_DATA_TIME;
      break;
    case Csyprod_P::RENDER_ALL_BEFORE_DATA_TIME :
      field.renderType = SymprodField::RENDER_ALL_BEFORE_DATA_TIME;
      break;
    case Csyprod_P::RENDER_ALL_AFTER_DATA_TIME :
      field.renderType = SymprodField::RENDER_ALL_AFTER_DATA_TIME;
      break;
    case Csyprod_P::RENDER_GET_VALID :
      field.renderType = SymprodField::RENDER_GET_VALID;
      break;
    case Csyprod_P::RENDER_GET_VALID_AT_FRAME_TIME :
      field.renderType = SymprodField::RENDER_GET_VALID_AT_FRAME_TIME;
      break;
    }
    field.onByDefault = symprod._prod_info[i].on_by_default;
    field.searchBefore =
      (int)((symprod._prod_info[i].minutes_allow_before * 60.0) + 0.5);
    field.searchAfter =
      (int)((symprod._prod_info[i].minutes_allow_after * 60.0) + 0.5);
    field.textOffThreshold = symprod._prod_info[i].text_off_threshold;
    field.requestDataOnZoom = symprod._prod_info[i].request_data_on_zoom;
    field.requestDataOnVertChange =
      symprod._prod_info[i].request_data_on_vert_change;
    
    _symprodFields.push_back(field);
  }
  
  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
