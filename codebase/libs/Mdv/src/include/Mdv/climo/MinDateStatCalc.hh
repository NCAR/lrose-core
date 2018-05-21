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
 * MinDateStatCalc: Class for calculating the date of the minimum statistic.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MinDateStatCalc_HH
#define MinDateStatCalc_HH

#include <Mdv/climo/StatCalc.hh>

using namespace std;

class MinDateStatCalc : public StatCalc
{
 public:

  /////////////////////////////
  // Constructors/destructor //
  /////////////////////////////

  /**********************************************************************
   * Constructor
   */

  MinDateStatCalc(const bool debug_flag = false,
		  const bool check_z_levels = true);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~MinDateStatCalc(void);
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /**********************************************************************
   * calcStatistic() - Calculate the statistic field using the given
   *                   information.
   *
   * Note that this method gets the data time from the forecast_time
   * field in the data_field field header, so this must be set appropriately
   * in the incoming data field.
   */

  virtual MdvxField *calcStatistic(const DsMdvx &climo_file,
				   const MdvxField &data_field,
				   const DateTime &climo_time);
  

  /**********************************************************************
   * getStatName() - Return the name for this statistic.
   */

  virtual string getStatName(const string field_name = "")
  {
    return StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MIN_DATE,
				       field_name);
  }
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _createField() - Create a new minimum climatology field from the
   *                  given data field.
   *
   * Return the newly created field on success, 0 on failure.
   */

  MdvxField *_createField(const MdvxField &data_field,
			  const DateTime &climo_time) const;
  

  /*********************************************************************
   * _updateField() - Create a new minimum climatology field that is an
   *                  update of the current minimum field using the given
   *                  information.
   *
   * Return the newly created field on success, 0 on failure.
   */

  MdvxField *_updateField(const DsMdvx &climo_file,
			  const MdvxField &data_field,
			  const MdvxField &curr_min_date_field) const;
  

};


#endif
