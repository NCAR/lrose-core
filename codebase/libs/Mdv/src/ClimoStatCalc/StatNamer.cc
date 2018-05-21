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
/*********************************************************************
 * StatNamer: Class for generating names for the statistic fields.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>

#include <Mdv/climo/StatNamer.hh>

using namespace std;


/**********************************************************************
 * Constructor
 */

StatNamer::StatNamer()
{
}


/**********************************************************************
 * Destructor
 */

StatNamer::~StatNamer(void)
{
}
  

/**********************************************************************
 * getStatFieldName() - Construct the statistic field name given the
 *                      statistic type, the base data field name and
 *                      the statistic parameter.
 */

string StatNamer::getStatFieldName(const Mdvx::climo_type_t stat_type,
				   const string &data_field_name,
				   const double parameter1,
				   const double parameter2)
{
  switch(stat_type)
  {
  case Mdvx::CLIMO_TYPE_MIN :
    return "Min " + data_field_name;
    
  case Mdvx::CLIMO_TYPE_MIN_DATE :
    return "Min date " + data_field_name;
    
  case Mdvx::CLIMO_TYPE_MAX :
    return "Max " + data_field_name;
    
  case Mdvx::CLIMO_TYPE_MAX_DATE :
    return "Max date " + data_field_name;
    
  case Mdvx::CLIMO_TYPE_MEAN :
    return "Mean " + data_field_name;
    
  case Mdvx::CLIMO_TYPE_STD_DEV :
    return "Std " + data_field_name;
    
  case Mdvx::CLIMO_TYPE_NUM_OBS :
    return "Num obs " + data_field_name;
    
  case Mdvx::CLIMO_TYPE_NUM_OBS_GT :
  {
    char parameter_string[80];
    sprintf(parameter_string, "%.2f", parameter1);
    
    return "Obs > " + string(parameter_string) + " " + data_field_name;
  }
    
  case Mdvx::CLIMO_TYPE_NUM_OBS_GE :
  {
    char parameter_string[80];
    sprintf(parameter_string, "%.2f", parameter1);
    
    return "Obs >= " + string(parameter_string) + " " + data_field_name;
  }
    
  case Mdvx::CLIMO_TYPE_NUM_OBS_LT :
  {
    char parameter_string[80];
    sprintf(parameter_string, "%.2f", parameter1);
    
    return "Obs < " + string(parameter_string) + " " + data_field_name;
  }
    
  case Mdvx::CLIMO_TYPE_NUM_OBS_LE :
  {
    char parameter_string[80];
    sprintf(parameter_string, "%.2f", parameter1);
    
    return "Obs <= " + string(parameter_string) + " " + data_field_name;
  }

  case Mdvx::CLIMO_TYPE_NUM_TIMES :
    return "Num times " + data_field_name;
    
  case Mdvx::CLIMO_TYPE_PERCENT :
    return "Percent obs " + data_field_name;
    
  case Mdvx::CLIMO_TYPE_PERCENT_GT :
  {
    char parameter_string[80];
    sprintf(parameter_string, "%.2f", parameter1);
    
    return "Percent > " + string(parameter_string) + " " + data_field_name;
  }
    
  case Mdvx::CLIMO_TYPE_PERCENT_GE :
  {
    char parameter_string[80];
    sprintf(parameter_string, "%.2f", parameter1);
    
    return "Percent >= " + string(parameter_string) + " " + data_field_name;
  }
    
  case Mdvx::CLIMO_TYPE_PERCENT_LT :
  {
    char parameter_string[80];
    sprintf(parameter_string, "%.2f", parameter1);
    
    return "Percent < " + string(parameter_string) + " " + data_field_name;
  }
    
  case Mdvx::CLIMO_TYPE_PERCENT_LE :
  {
    char parameter_string[80];
    sprintf(parameter_string, "%.2f", parameter1);
    
    return "Percent <= " + string(parameter_string) + " " + data_field_name;
  }

  }
  
  return "Unknown stat type";
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
