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
 * @file FilterList.cc
 *
 * @class FilterList
 *
 * FilterList is a class the controls access to a list of filters.
 *  
 * @date 7/17/2008
 *
 */

using namespace std;

#include "FilterList.hh"


/**
 * Constructor
 */

FilterList::FilterList() 
{
}
   

/**
 * Destructor
 */

FilterList::~FilterList() 
{
  clearFieldInfo();
}
   

/**
 * addFilter()
 */

void FilterList::addFilter(Filter *filter)
{
  _filterList.push_back(filter);
}


/**
 * calcFinal()
 */

void FilterList::calcFinal(const int ibeam, const int igate,
			   const float dbz_val)
{
  vector< Filter* >::iterator filter;
  
  for (filter = _filterList.begin(); filter != _filterList.end(); ++filter)
    (*filter)->calcFinal(ibeam, igate, dbz_val);
}


/**
 * calcInterest()
 */

void FilterList::calcInterest(const FilterBeamInfo::InterestType int_type,
			      const double field_val,
			      const int ibeam, const int igate)
{
  vector< Filter* >::iterator filter;
  
  for (filter = _filterList.begin(); filter != _filterList.end(); ++filter)
    (*filter)->calcInterest(int_type, field_val, ibeam, igate);
}


/**
 * clearFieldInfo()
 */

void FilterList::clearFieldInfo()
{
  _fieldInfo.erase(_fieldInfo.begin(), _fieldInfo.end());
}


/**
 * filterData()
 */

void FilterList::filterData(vector< DsBeamData > &data_ptrs)
{
  // Loop through the data, filtering as we go.  Note that the interest
  // fields seem to be offset from the raw data fields by az_radius.
  // This seems to have been done to take into account the beams around
  // the current beam that are used to calculate the feature fields.
  // If I have time, I'll look into changing how this is done so things
  // are a little less confusing.

  int n_gates = data_ptrs[0].get_ngates();
  int n_beams = _filterList[0]->getNumBeams();
  int az_radius = _filterList[0]->getAzRadius();
  
  for (int ibeam = 0; ibeam < n_beams; ++ibeam)
  {
    int ibeam_data = ibeam + az_radius;
    
    for (int igate = 0; igate < n_gates; ++igate)
    {
      // Evaluate the final filter flag

      bool cum_filter_flag = false;
      bool first_filter = true;
      
      vector< Filter* >::iterator filter_iter;
      for (filter_iter = _filterList.begin(); filter_iter != _filterList.end();
	   ++filter_iter)
      {
	Filter *filter = *filter_iter;
	bool filter_flag = filter->getFilterFlag(ibeam, igate);
	
	if (first_filter)
	{
	  cum_filter_flag = filter_flag;
	  first_filter = false;
	  continue;
	}
	
	Filter::CombineType combine_type = filter->getCombineType();
	
	switch(combine_type)
	{
	case Filter::COMBINE_AND :
	  cum_filter_flag &= filter_flag;
	  break;
	  
	case Filter::COMBINE_OR :
	  cum_filter_flag |= filter_flag;
	  break;
	}
      } /* endfor - filter_iter */
      
      // Filter the data 

      if (cum_filter_flag)
      {
	// Set each of the filtering fields to missing

	map< int, int, less<int> >::iterator it;
	for( it = _fieldInfo.begin(); it != _fieldInfo.end(); it++ )
	  data_ptrs[ibeam_data].set_to_missing((*it).first, igate);
      }
      
    } /* endfor - igate */
  } /* endfor - ibeam */
}


/**
 * setBeam()
 */

void FilterList::setBeam(const double azimuth)
{
  vector< Filter* >::iterator filter;
  
  for (filter = _filterList.begin(); filter != _filterList.end(); ++filter)
    (*filter)->setBeam(azimuth);
}


/**
 * putEndOfVolume()
 */

void FilterList::putEndOfVolume(const time_t end_time, const int volume_num)
{
  vector< Filter* >::iterator filter;
  
  for (filter = _filterList.begin(); filter != _filterList.end(); ++filter)
    (*filter)->putEndOfVolume(end_time, volume_num);
}


/**
 * putStartOfVolume()
 */

void FilterList::putStartOfVolume(const time_t start_time,
				  const int volume_num)
{
  vector< Filter* >::iterator filter;
  
  for (filter = _filterList.begin(); filter != _filterList.end(); ++filter)
    (*filter)->putStartOfVolume(start_time, volume_num);
}


/**
 * setFieldInfo()
 */

void FilterList::setFieldInfo(const int index, const int missing_val)
{
  _fieldInfo[index] = missing_val;
}


/**
 * setTerrain()
 */

void FilterList::setTerrain(const float terrain_val)
{
  vector< Filter* >::iterator filter;
  
  for (filter = _filterList.begin(); filter != _filterList.end(); ++filter)
    (*filter)->setTerrain(terrain_val);
}


/**
 * setTilt()
 */

void FilterList::setTilt(const int n_gates)
{
  vector< Filter* >::iterator filter;
  
  for (filter = _filterList.begin(); filter != _filterList.end(); ++filter)
    (*filter)->setTilt(n_gates);
}


/**
 * writeInterest()
 */

int FilterList::writeInterest(const time_t start_time, const time_t end_time, 
			      DsRadarParams& sample_params,
			      const double elev_angle,
			      const int volume_num, const int tilt_num)
{
  vector< Filter* >::iterator filter;
  
  for (filter = _filterList.begin(); filter != _filterList.end(); ++filter)
  {
    if ((*filter)->writeInterest(start_time, end_time, sample_params, 
				 elev_angle, volume_num, tilt_num) != 0)
      return -1;
  }

  return 0;
}
