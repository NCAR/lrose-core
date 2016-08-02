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
#include <toolsa/copyright.h>

/*----------------------------------------------------------------*/
/* @file AngleCombiner.hh
 * @brief Combining angles,which have wraparound potential at 0=360 degrees.
 * @class AngleCombiner
 * @brief Combining angles,which have wraparound potential at 0=360 degrees.
 *
 * A vector of angles with optional weights can be combined into an average.
 *
 * This class tries to handle the fact than angles wrap around.  It is in fact
 * a non-deterministic problem...if the angles are all different directions,
 * an average may make no sense. If they are all within a small range,
 * averaging does make sense.
 *
 * The implementation is as a vector of angles, confidences, and weights.
 */

# ifndef    ANGLE_COMBINER_HH
# define    ANGLE_COMBINER_HH

#include <vector>

class AngleCombiner
{
public:

  /** 
   * Constructor for combining angles.
   * 
   * @param[in] n  Maximum number of angles to combine
   * @param[in] is360  true if range = [-180,180].
   *                    false if range = [0,180) such as orientation of a line
   */
  AngleCombiner(int n, bool is_360=false);
                                                 
  /**
   * Constructor for a weighted combination of angles
   *
   * @param[in] weight  The weights to use
   * @param[in] is_360  true if range = [-180,180].
   *                    false if range = [0,180) such as orientation of a line
   */
  AngleCombiner(const std::vector<double> &weight, bool is360=false);

  /**
   * Destructor
   */
  virtual ~AngleCombiner(void);

  /**
   *  Clear all internal values so there are no angles stored internally.
   */
  void clearValues(void);
  
  /**
   * Set ith angle/confidence locally.  
   * Either this method or setBad() must be called in increasing
   * order i=0,1,2,..
   *
   * @param[in] i  Index
   * @param[in] angle  Angle value
   * @param[in] conf  Confidence value
   */
  void setGood(int i, double angle, double conf);

  /**
   * Set ith angle/confidence to missing data.
   * Either this method or setGood() must be called in increasing
   * order i=0,1,2,..
   * @param[in] i  Index
   */
  void setBad(int i);

  /**
   * Compute the combined angle and confidence using the combiner algorithm
   *
   * @param[out] v  Value (combined angle)
   * @param[out] c  Confidence in the value
   *
   * @return true if able to run the alg and produce results
   */
  bool getCombineAngleConf(double &v, double &c);

  /**
   *
   * Compute the combined angle using the combiner algorithm, without
   * confidence values
   *
   * @param[out] v  Value (combined angle)
   *
   * @return true if able to run the alg and produce results
   */
  bool getCombineAngle(double &v);

  /**
   * @return true if input angle is a special 'bad' angle value
   *
   * @param[in] angle
   */
  static bool isBadAngle(const double angle);

private:  

  /**
   * @struct AngleConfWeight_t
   * @brief angle/confidence/weight
   */
  typedef struct
  {
    double angle;     /**< Angle (degrees) */
    double conf;      /**< Confidence */
    double weight;    /**< Weight */
  } AngleConfWeight_t;


  /**
   * Vector of angles/confidence/weight values
   */
  std::vector<AngleConfWeight_t> _acw; 

  int _n;        /**< # of angle/conf/weights in place... */
  int _max_n;    /**< max # of angles/conf/weights that can go in. */
  bool _is_set;  /**< true if _angle and _conf have been computed */
  double _angle; /**< combined angle. */
  double _conf;  /**< combined confidence. */
  bool _is_360;  /**< true if have full circle [-180,180), false
		  *   if have only orientations [0,180) */

  void _setValues(void);
  void _setOrientationValues(void);
  void _setMotionValues(void);
  bool _orientationRange(double &a0, double &a1);
  bool _motionRange(double &a0, double &a1);
  bool _actualRange(double &a0, double &a1) const;
  void _adjustedRange(double min_good, double adjustement_add,
		      double &a0, double &a1) const;
  void _doAdjust(double min, double add);
  void _simpleCombine(void);
  void _largeSmallCombine(void);
  bool _checkRange(bool &error);
};

#endif
