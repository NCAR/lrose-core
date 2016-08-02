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
 *   $Locker:  $
 *   $Date: 2016/03/04 02:22:11 $
 *   $Id: CalcRwpHandler.hh,v 1.4 2016/03/04 02:22:11 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * CalcRwpHandler: Class that supplies the RWP field by calculating it
 *                 from the RNW, SNOW, TEMP and HGT fields and the
 *                 available pressure data.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef CalcRwpHandler_H
#define CalcRwpHandler_H

#include "PressHandler.hh"
#include "RwpHandler.hh"

using namespace std;


class CalcRwpHandler : public RwpHandler
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  CalcRwpHandler(const string &url,
		 const DateTime &gen_time, const DateTime &fcst_time);
  
  CalcRwpHandler(const string &url,
		 const DateTime &time_centroid);

  /*********************************************************************
   * Destructor
   */

  virtual ~CalcRwpHandler();


  /*********************************************************************
   * init() - Initialize the data.
   *
   * Returns true on success, false on failure.
   */

  bool init(const bool get_pressure_from_field,
	    const string &rnw_field_name,
	    const string &snow_field_name,
	    const string &tk_field_name,
	    const string &hgt_field_name,
	    const string &press_field_name,
	    const bool height_increasing);


  /*********************************************************************
   * getRwpField() - Get the RWP field.
   *
   * Returns a pointer to the field on success, 0 on failure.  The pointer
   * is then owned by the calling method and must be deleted there.
   */

  virtual MdvxField *getRwpField();


protected:
  
  ///////////////////////
  // Private constants //
  ///////////////////////

  static const double RR;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _heightIncreasing;
  
  // Pointers to the fields used to calculate the RWP field

  MdvxField *_rnwField;
  MdvxField *_snowField;
  MdvxField *_tkField;
  MdvxField *_hgtField;
  
  // Pointer to the object used to obtain the pressure data

  PressHandler *_pressHandler;


  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _calcRhoa() - Calculate the rhoa value.
   *
   * Returns the calculated value.
   */

  inline static double _calcRhoa(const fl32 pressure, const fl32 temperature_k)
  {
    return pressure / (temperature_k * RR);
  }
  

  /*********************************************************************
   * _calcRwater() - Calculate the rwater value.
   *
   * Returns the calculated value.
   */

  inline static double _calcRwater(const fl32 rnw, const fl32 snow,
				   const double rhoa)
  {
    return (rnw + snow) * rhoa * 1000.0;
  }
  

  /*********************************************************************
   * _createRwp() - Create the RWP (VIL) field.
   *
   * Returns a pointer to the created field on success, 0 on failure.
   */

  MdvxField *_createRwp() const;
  

  /*********************************************************************
   * _createSurfaceField() - Create a blank surface field so the values can be
   *                         filled in later.  The field will have the
   *                         same X/Y dimensions and forecast time as the
   *                         given field header.
   *
   * Returns a pointer to the created field on success, 0 on failure.
   */

  MdvxField *_createSurfaceField(const Mdvx::field_header_t in_field_hdr,
				 const fl32 bad_data_value,
				 const string &field_name_long,
				 const string &field_name,
				 const string &units) const;
  

  /*********************************************************************
   * _projectionsMatch() - Checks to see if the projections of the input
   *                       fields match.  If they don't match, we can't do
   *                       the calculations.
   *
   * Returns true if they match, false otherwise.
   */

  bool _projectionsMatch(const bool check_press_field) const;
  
  
};

#endif
