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
 * StatCalc: Base class for statistic calculators.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef StatCalc_HH
#define StatCalc_HH

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/climo/StatNamer.hh>
#include <toolsa/DateTime.hh>


using namespace std;


class StatCalc
{
 public:

  /////////////////////////////
  // Constructors/destructor //
  /////////////////////////////

  /**********************************************************************
   * Constructor
   */

  StatCalc(const bool debug_flag = false,
	   const bool check_z_levels = true);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~StatCalc(void);
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /**********************************************************************
   * calcStatistic() - Calculate the statistic field using the given
   *                   information.
   */

  virtual MdvxField *calcStatistic(const DsMdvx &climo_file,
				   const MdvxField &data_field,
				   const DateTime &climo_time) = 0;
  

  /**********************************************************************
   * getStatName() - Return the name for this statistic.
   */

  virtual string getStatName(const string field_name = "") = 0;
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;

  bool _checkZLevels;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**********************************************************************
   * _fieldsMatch() - Check to see if the given fields match based on their
   *                  headers.
   *
   * Returns true if the fields match, false otherwise.
   */

  bool _fieldsMatch(const MdvxField &data_field,
		    const MdvxField &climo_field) const;
  

};


#endif
