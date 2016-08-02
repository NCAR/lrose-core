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
 * @file Ascii1Output.cc
 *
 * @class Ascii1Output
 *
 * Base class for output handlers.
 *  
 * @date 8/5/2009
 *
 */

#include "Ascii1Output.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

Ascii1Output::Ascii1Output(const string &missing_string,
			   const bool debug_flag, const bool verbose_flag) :
  Output(debug_flag, verbose_flag),
  _missingString(missing_string)
{
}


/*********************************************************************
 * Destructor
 */

Ascii1Output::~Ascii1Output()
{
}


/*********************************************************************
 * addPoint()
 */

bool Ascii1Output::addPoint(const int location_id,
			    const double lat, const double lon,
			    const double alt, const double grid_size,
			    const double value, const double value_nw,
			    const double value_n, const double value_ne,
			    const double value_e, const double value_se,
			    const double value_s, const double value_sw,
			    const double value_w)
{
  cout << _dataTime.getYear() << " " << _dataTime.getMonth() << " "
       << _dataTime.getDay() << " " << _dataTime.getHour() << " "
       << _dataTime.getMin() << " " << _dataTime.getSec() << " ";
  
  cout << location_id << " " << lat << " " << lon << " "
       << alt << " " << grid_size << " ";
  
  if (value == MISSING_VALUE)
    cout << _missingString << " ";
  else
    cout << value << " ";
  
  if (value_nw == MISSING_VALUE)
    cout << _missingString << " ";
  else
    cout << value_nw << " ";
  
  if (value_n == MISSING_VALUE)
    cout << _missingString << " ";
  else
    cout << value_n << " ";
  
  if (value_ne == MISSING_VALUE)
    cout << _missingString << " ";
  else
    cout << value_ne << " ";
  
  if (value_e == MISSING_VALUE)
    cout << _missingString << " ";
  else
    cout << value_e << " ";
  
  if (value_se == MISSING_VALUE)
    cout << _missingString << " ";
  else
    cout << value_se << " ";
  
  if (value_s == MISSING_VALUE)
    cout << _missingString << " ";
  else
    cout << value_s << " ";
  
  if (value_sw == MISSING_VALUE)
    cout << _missingString << " ";
  else
    cout << value_sw << " ";
  
  if (value_w == MISSING_VALUE)
    cout << _missingString << " ";
  else
    cout << value_w << " ";
  
  cout << endl;
  
  return true;
}


/*********************************************************************
 * close()
 */

bool Ascii1Output::close()
{
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _init()
 */

bool Ascii1Output::_init()
{
  return true;
}

