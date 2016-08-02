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
 * @file RadarMsg.hh
 *
 * @class RadarMsg
 *
 * RadarMsg is a class that controls radar messages.
 *  
 * @date 4/5/2002
 *
 */

#ifndef RadarMsg_HH
#define RadarMsg_HH

using namespace std;

#include <rapformats/DsRadarBeam.hh>
#include <rapformats/DsRadarMsg.hh>
#include <rapformats/DsRadarParams.hh>


/** 
 * @class RadarMsg
 */

class RadarMsg
{
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /** 
   * @brief Constructor
   *
   * @param[in] radar_msg Radar message.
   * @param[in] content Mask indicating what infomration the radar message
   *                    contains.
   */

  RadarMsg(const DsRadarMsg& radar_msg, const int content);

  /**
   * @brief Destructor
   */

  ~RadarMsg();

  /**
   * @brief Determine if the radar message contains radar flags.
   *
   * @return Returns true if the message contains radar flags, false
   *         otherwise.
   */

  bool isFlags() const;
   
  /**
   * @brief Get the radar beam information from the message.
   *
   * @return Returns the radar beam information.
   */

  DsRadarBeam& getRadarBeam() { return _msg->getRadarBeam(); }
   
  /**
   * @brief Get the radar parameters from the message.
   *
   * @return Returns the radar parameters.
   */

  DsRadarParams& getRadarParams() { return _msg->getRadarParams(); }

  /**
   * @brief Get access to the message information.
   *
   * @return Returns a reference to the message object.
   */

  DsRadarMsg& getRadarMsg() { return *_msg; }

  /**
   * @brief Get the mask indicating what type of information the message
   *        contains.
   *
   * @return Returns the message content mask.
   */

  int getMsgContent() { return _msgContent; }

  /**
   * @brief Get the message field parameters.
   *
   * @return Returns the message field parameters.
   */

  vector< DsFieldParams* >& getFieldParams()
    { return _msg->getFieldParams(); }

  /**
   * @brief Get the tilt number for the message.
   *
   * @return Returns the message tilt number.
   */

  int getTiltNum() const;

  /**
   * @brief Get the volume number for the message.
   *
   * @return Returns the message volume number.
   */

  int getVolumeNum() const;

  /**
   * @brief Get the time stamp for the message.
   *
   * @return Returns the message time stamp.
   */

  time_t getTime() const;

  /**
   * @brief Get the number of gates in the message.
   *
   * @return Returns the number of gates.
   */

  int getNumGates() const;

  /**
   * @brief Get the gate spacing in the message.
   *
   * @return Returns the gate spacing in km.
   */

  float getGateSpacing() const;

  /**
   * @brief Get the start range in the message.
   *
   * @return Returns the start range in km.
   */

  float getStartRange() const;
   
private:

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief The radar message.
   */

  DsRadarMsg *_msg;

  /**
   * @brief The message content mask.  This mask indicates what information
   *        is included in the message.
   */

  int _msgContent;
};

#endif
