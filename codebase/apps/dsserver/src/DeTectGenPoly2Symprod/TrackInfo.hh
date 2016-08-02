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
 * @file TrackInfo.hh
 *
 * @class TrackInfo
 *
 * DeTect track information for rendering.
 *  
 * @date 9/20/2011
 *
 */

#ifndef TrackInfo_HH
#define TrackInfo_HH

#include <string>
#include <vector>

#include <DeTect/DeTectHorizontalSpdb.hh>
#include <toolsa/DateTime.hh>

#include "Params.hh"


/**
 * @class TrackInfo
 */

class TrackInfo
{
 public:

  //////////////////
  // Public types //
  //////////////////

  typedef struct
  {
    double lat;
    double lon;
    bool coasted_flag;
  } point_t;
  
    
  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  TrackInfo(const DeTectHorizontalSpdb &record);

  /**
   * @brief Destructor
   */

  virtual ~TrackInfo();
  

  /**
   * @brief Note that we are guaranteed that each subsequent record is for
   *        an earlier time that the currently processed records so that we
   *        know that adding points to the end will work.
   */

  void addRecord(const DeTectHorizontalSpdb &record);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the indicated field information.
   */

  bool getFieldInfo(const Params::field_id_t field_id,
		    string &field_name, string &field_units,
		    double &field_value) const;
  
  /**
   * @brief Get the name of the track.
   */

  string getName() const
  {
    return _spdbInfo.getName();
  }
  
  /**
   * @brief Get the ellipse major axis in kilometers.
   */

  double getEllipseMajorKm() const
  {
    return _spdbInfo.getEllipseMajorKm();
  }
  
  /**
   * @brief Get the ellipse minor axis in kilometers.
   */

  double getEllipseMinorKm() const
  {
    return _spdbInfo.getEllipseMinorKm();
  }
  
  /**
   * @brief Get the heading in degrees.
   */

  double getHeading() const
  {
    return _spdbInfo.getHeading();
  }
  
  /**
   * @brief Get the value used to indicate missing data.
   */

  double getMissingValue() const
  {
    return _spdbInfo.missingVal;
  }
  
  /**
   * @brief Get the orientation.
   */

  double getOrientation() const
  {
    return _spdbInfo.getOrientation();
  }
  
  /**
   * @brief Get the predicted heading in degrees.
   */

  double getPHeading() const
  {
    return _spdbInfo.getPHeading();
  }
  
  /**
   * @brief Get the speed in m/s.
   */

  double getSpeed() const
  {
    return _spdbInfo.getSpeed();
  }
  
  /**
   * @brief Get the time of the track.
   */

  DateTime getTime() const
  {
    return _spdbInfo.getTime();
  }
  
  /**
   * @brief Get the track.
   */

  const vector< point_t > &getTrack() const
  {
    return _track;
  }
  
  /**
   * @brief Get the coasted flag.
   *
   * @return Returns true is the SPDB record is for a coasted point, false
   *         otherwise.
   */

  bool isCoasted() const
  {
    return _spdbInfo.isCoasted();
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The SPDB information for the latest track entry.
   */

  DeTectHorizontalSpdb _spdbInfo;
  
  /**
   * @brief The current track.
   */

  vector< point_t > _track;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

};


#endif
