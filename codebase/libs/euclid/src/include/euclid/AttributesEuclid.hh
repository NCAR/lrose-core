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
 * @file AttributesEuclid.hh
 * @brief Attributes class extended to have certain Euclidian specific values
 *
 * @class AttributesEuclid
 * @brief Attributes class extended to have certain Euclidian specific values
 *
 * Specific attributes are set/cleared/accessed by class methods
 */
# ifndef    ATTRIBUTES_EUCLID_HH
# define    ATTRIBUTES_EUCLID_HH

#include <toolsa/Attributes.hh>

class MotionVector;
class DataAtt;

class AttributesEuclid : public Attributes
{

public:

  /**
   * Constructor, no attributes in place
   */
  AttributesEuclid(void);

  /**
   * Construct the average of attributes from all inputs
   * @param[in] a  
   */
  AttributesEuclid(const std::vector<AttributesEuclid> &a);

  /**
   * Destructor
   */
  virtual ~AttributesEuclid(void);
 
  /**
   * Create a MotionVector object from local attribute values
   *
   * @return true if the needed named attributes were there to do this
   *
   * @param[out] v  
   */
  bool getMotionVector(MotionVector &v) const;

  /**
   * Add attributes for a MotionVector object
   *
   * @param[in] v  Motion Vector object
   */
  void setMotionVector(const MotionVector &v);

  /**
   * Remove attributes for a MotionVector from state, if they are there.
   */
  void removeMotionVector(void);

  /**
   * Return average MotionVector of local and input object, where the
   * attributes specify the MotionVector values
   *
   * @param[in] a1  Object to average with local object
   * @param[out] v  Averaged MotionVector
   *
   * @return true for success
   */
  bool averageMotionVector(const AttributesEuclid &a1, MotionVector &v) const;

  /**
   * Return magnitude of MotionVector, if attributes specify a MotionVector
   *
   * @param[out] s  Speed
   *
   * @return true if speed was set
   */
  bool getMotionSpeed(double &s) const;

  /**
   * Return x component of motion, if there is a MotionVector in the attributes
   *
   * @param[out] vx  X component of motion
   *
   * @return true if able to return the speed
   */
  bool getMotionX(double &vx) const;

  /**
   * Return y component of motion, if there is a MotionVector in the attributes
   *
   * @param[out] vy  Y component of motion
   *
   * @return true if able to return the speed
   */
  bool getMotionY(double &vy) const;

  /**
   * If a MotionVector is present, reverse its 'handedness' (rotate by 180)
   */
  void reverseMotionHandedness(void);

  /**
   * Return a quality value 
   * 
   * @param[out] q  Quality
   *
   * @return true if named attribute for quality was in local state and quality
   *         was returned.
   */
  bool getQuality(double &q) const;

  /**
   * Add quality attribute
   *
   * @param[in] q  quality value
   */
  void setQuality(double q);

  /**
   * Return average quality  of input and local ojects
   *
   * @param[in] a1  Input object 
   * @param[out] v  Averagte quality
   *
   * @return true for success
   */
  bool averageQuality(const AttributesEuclid &a1, double &v) const;

  /**
   * Remove quality attribute from local state, if it is there.
   */
  void removeQuality(void);

  /**
   * Return a time value
   *
   * @return true if attribute for time was present in local state and time
   * could be returned.
   *
   * @param[out] t  Returned time
   */
  bool getTime(time_t &t) const;

  /**
   * Add time attribute
   *
   * @param[in] t  Time value
   */
  void setTime(const time_t &t);

  /**
   * Remove time attribute, if it is there
   */
  void removeTime(void);

  /**
   * Return a DataAtt object created using local attribute values
   * 
   * @return true if local attributes were there to create a DataAtt
   *
   * @param[out] v    Returned DataAtt
   */
  bool getDataAtt(DataAtt &v) const;

  /**
   * Add DataAtt to the local state
   *
   * @param[in] v  DataAtt object
   */
  void setDataAtt(const DataAtt &v);

  /**
   * Remove DataAtt from local state if it was there.
   */
  void removeDataAtt(void);

  /**
   * Get DataAtt max value, if DataAtt is in the local state
   *
   * @param[out] m  Max value
   *
   * @return false if no DataAtt
   */
  bool getMaxDataAtt(double &m) const;

  /**
   * Get DataAtt average value, if DataAtt is in the local state
   *
   * @param[out] m  Average value
   *
   * @return false if no DataAtt
   */
  bool getAverageDataAtt(double &m) const;

protected:
private:
  
}; 

#endif
