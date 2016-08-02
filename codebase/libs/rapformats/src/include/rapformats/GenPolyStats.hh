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

/////////////////////////////////////////////////////////////
// GenPolyStats.hh
//
// A GenPoly database made specifically for storing statistics
// for polygons at different elevation angles in radar data.
//
// Nancy Rehak, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2009
//////////////////////////////////////////////////////////////

#ifndef _GenPolyStats_hh
#define _GenPolyStats_hh

#include <rapformats/GenPoly.hh>

using namespace std;

class GenPolyStats : public GenPoly
{

public:

  //////////////////////
  // Public constants //
  //////////////////////

  static const string DROPSIZE_THRESH_FIELD_PREFIX;
  static const string THRESHOLD_FIELD_PREFIX;
  
  static const string CENTROID_LAT_FIELD_NAME;
  static const string CENTROID_LON_FIELD_NAME;
  static const string DATA_AREA_FIELD_NAME;
  static const string DATA_CENTROID_LAT_FIELD_NAME;
  static const string DATA_CENTROID_LON_FIELD_NAME;
  static const string DATA_HEIGHT_FIELD_NAME;
  static const string DATA_RANGE_FIELD_NAME;
  static const string ELEV_ANGLE_FIELD_NAME;
  static const string SCAN_MODE_FIELD_NAME;
  static const string SCAN_TIME_OFFSET_FIELD_NAME;
  static const string VLEVEL_INDEX_FIELD_NAME;
  

  ////////////////////
  // Public methods //
  ////////////////////

  // constructor

  GenPolyStats();

  // destructor

  virtual ~GenPolyStats();
  
  /////////////////////
  // Add/set methods //
  /////////////////////

  virtual void addField(const string &field_name,
			const double value,
			const string &units)
  {
    addFieldInfo(field_name, units);
    addVal(value);
  }
  
  virtual void addDropsizeThresh(const string &field_name,
				 const double threshold,
				 const string &units)
  {
    addField(DROPSIZE_THRESH_FIELD_PREFIX + field_name, threshold, units);
  }
  
  virtual void addThreshold(const string &field_name,
			    const double threshold,
			    const string &units)
  {
    addField(THRESHOLD_FIELD_PREFIX + field_name, threshold, units);
  }
  
  virtual void setCentroid(const double lat,
			   const double lon)
  {
    addField(CENTROID_LAT_FIELD_NAME, lat, "deg");
    addField(CENTROID_LON_FIELD_NAME, lon, "deg");
  }
  
  virtual void setDataArea(const double value)
  {
    addField(DATA_AREA_FIELD_NAME, value, "km^2");
  }
  
  virtual void setDataCentroid(const double lat,
			       const double lon)
  {
    addField(DATA_CENTROID_LAT_FIELD_NAME, lat, "deg");
    addField(DATA_CENTROID_LON_FIELD_NAME, lon, "deg");
  }
  
  virtual void setDataHeight(const double value)
  {
    addField(DATA_HEIGHT_FIELD_NAME, value, "km");
  }
  
  virtual void setDataRange(const double value)
  {
    addField(DATA_RANGE_FIELD_NAME, value, "km");
  }
  
  virtual void setElevAngle(const double value)
  {
    addField(ELEV_ANGLE_FIELD_NAME, value, "deg");
  }
  
  virtual void setScanMode(const double value)
  {
    addField(SCAN_MODE_FIELD_NAME, value, "none");
  }
  
  virtual void setScanTimeOffset(const double value)
  {
    addField(SCAN_TIME_OFFSET_FIELD_NAME, value, "secs");
  }
  
  virtual void setVlevelIndex(const double value)
  {
    addField(VLEVEL_INDEX_FIELD_NAME, value, "");
  }
  

  /////////////////
  // Get methods //
  /////////////////

  virtual double getElevAngle() const
  {
    return getField(ELEV_ANGLE_FIELD_NAME);
  }
  
  virtual double getScanMode() const
  {
    return getField(SCAN_MODE_FIELD_NAME);
  }
  
  virtual double getScanTimeOffset() const
  {
    return getField(SCAN_TIME_OFFSET_FIELD_NAME);
  }
  
  virtual double getVlevelIndex() const
  {
    return getField(VLEVEL_INDEX_FIELD_NAME);
  }
  

protected:

  ///////////////////////
  // Protected methods //
  ///////////////////////

  inline double getField(const string &field_name) const
  {
    int field_num;

    if ((field_num = getFieldNum(field_name)) < 0)
      return missingVal;
    
    return get1DVal(field_num);
  }
  

private:

};


#endif
