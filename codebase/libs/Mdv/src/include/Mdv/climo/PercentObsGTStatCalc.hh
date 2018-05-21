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
 * PercentObsGTStatCalc: Class for calculating the percent of
 *                       observations greater than a given value
 *                       statistic.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef PercentObsGTStatCalc_HH
#define PercentObsGTStatCalc_HH

#include <Mdv/climo/PercentObsStatCalc.hh>

using namespace std;

class PercentObsGTStatCalc : public PercentObsStatCalc
{
 public:

  /////////////////////////////
  // Constructors/destructor //
  /////////////////////////////

  /**********************************************************************
   * Constructor
   */

  PercentObsGTStatCalc(const double cutoff_value,
		       const bool debug_flag = false,
		       const bool check_z_levels = true);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~PercentObsGTStatCalc(void);
  

  /**********************************************************************
   * getStatName() - Return the name for this statistic.
   */

  virtual string getStatName(const string field_name = "")
  {
    return StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_PERCENT_GT,
				       field_name,
				       _cutoffValue);
  }
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  double _cutoffValue;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _getNumObsStatFieldName() - Gets the field name for the number of
   *                             observations field that corresponds to
   *                             this percent stat field.
   */

  virtual string _getNumObsStatFieldName(const string &data_field_name) const
  {
    return StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_NUM_OBS_GT,
				       data_field_name,
				       _cutoffValue);
  }


  /*********************************************************************
   * _getStatFieldName() - Gets the field name for this statistic.
   */

  virtual string _getStatFieldName(const string &data_field_name) const
  {
    return StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_PERCENT_GT,
				       data_field_name,
				       _cutoffValue);
  }


  /*********************************************************************
   * _meetsCondition() - Checks condition for this counter statistic.
   *
   * Returns true if the data point meets the condition set for this
   * statistic, false otherwise.
   */

  virtual bool _meetsCondition(const double data_value) const
  {
    if (data_value > _cutoffValue)
      return true;

    return false;
  }


};


#endif
