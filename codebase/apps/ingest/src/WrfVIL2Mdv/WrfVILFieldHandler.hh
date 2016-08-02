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

/************************************************************************
 * WrfVILFieldHandler: Class for handling fields from a netCDF file that is
 *                    in the format used for the NSSL mosaics produced over
 *                    the CONUS.
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 2008
 *
 * Dan Megenahrdt
 *
 ************************************************************************/

#ifndef WrfVILFieldHandler_HH
#define WrfVILFieldHandler_HH

#include <vector>
#include <toolsa/mem.h>
#include "Params.hh"
#include "FieldHandler.hh"

using namespace std;


class WrfVILFieldHandler : public FieldHandler
{
 public:

  /////////////////////////////
  // Constructors/destructor //
  /////////////////////////////

  /**********************************************************************
   * Constructor
   */

  WrfVILFieldHandler(const bool user_defined_bad_missing, 
                     const float bad_missing_val,
		     const bool redefine_bad_missing, 
                     const float new_bad_missing_val,
		     const bool scale_data,
		     const float multiplicative_factor,
                     const string field_name,
		     const bool debug_flag = false);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~WrfVILFieldHandler(void);
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /**********************************************************************
   * extractField() - Extract the field from the given netCDF file.
   */

//  virtual MdvxField *extractField(const int nc_file_id);
  virtual MdvxField *extractField(const int nc_file_id, 
				  const time_t data_time, 
				  const int extract_time, 
				  const float *data);
  virtual float *extractData(const int nc_file_id);
  virtual vector <time_t> extractTimes(const int nc_file_id);
  
protected:

  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    int level_type;
    double level;
  } level_info_t;
  
  
  ///////////////////////
  // Protected methods //
  ///////////////////////
  
};


#endif
