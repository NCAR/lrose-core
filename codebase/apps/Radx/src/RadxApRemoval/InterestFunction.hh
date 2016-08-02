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
 * @file InterestFunction.hh
 *
 * @class InterestFunction
 *
 * InterestFunction handles interest functions.
 *  
 * @date 4/8/2002
 *
 */

#ifndef InterestFunction_HH
#define InterestFunction_HH

using namespace std;


/** 
 * @class InterestFunction
 */

class InterestFunction
{

 public:

  //////////////////////
  // Public constants //
  //////////////////////

  /** 
   * @brief Value used to indicate a missing interest value.
   */

  static const double MISSING_INTEREST;
   
  /** 
   * @brief The scale used for scaling the interest values.
   */

  static const float  INTEREST_SCALE;

  /** 
   * @brief The bias used for scaling the interest values.
   */

  static const float  INTEREST_BIAS;

  /** 
   * @brief The value used in the scaled data to indicate a missing interest
   *        value.
   */

  static const int    SCALED_MISSING_INTEREST;

  /** 
   * @brief The minimum value that can be used to represent good data in the
   *        scaled data.  This value depends on the number of bytes used in
   *        the interest FMQ and which scaled value is used to represent
   *        missing data.
   */

  static const int    SCALED_MIN_INTEREST;

  /** 
   * @brief The maximum value that can be used to represent good data in the
   *        scaled data.  This value depends on the number of bytes used in
   *        the interest FMQ and which scaled value is used to represent
   *        missing data.
   */

  static const int    SCALED_MAX_INTEREST;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /** 
   * @brief Constructor
   */

  InterestFunction();

  /** 
   * @brief Destructor
   */

  ~InterestFunction(){};
   
  /**
   * @brief Set the function values.
   *
   * @param[in] x1 X (data) value for the first point in the interest
   *               function.
   * @param[in] y1 Y (interest) value for the first point in the interest
   *               function.
   * @param[in] x2 X (data) value for the second point in the interest
   *               function.
   * @param[in] y2 Y (interest) value for the second point in the interest
   *               function.
   * @param[in] x3 X (data) value for the third point in the interest
   *               function.
   * @param[in] y3 Y (interest) value for the third point in the interest
   *               function.
   * @param[in] x4 X (data) value for the fourth point in the interest
   *               function.
   * @param[in] y4 Y (interest) value for the fourth point in the interest
   *               function.
   * @param[in] x5 X (data) value for the fifth point in the interest
   *               function.
   * @param[in] y5 Y (interest) value for the fifth point in the interest
   *               function.
   * @param[in] x6 X (data) value for the sixth point in the interest
   *               function.
   * @param[in] y6 Y (interest) value for the sixth point in the interest
   *               function.
   * @param[in] weight Weight for this interest function.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int setFunction(const double x1, const double y1,
		  const double x2, const double y2,
		  const double x3, const double y3,
		  const double x4, const double y4,
		  const double x5, const double y5,
		  const double x6, const double y6,
		  const double weight);
   
  /**
   * @brief Apply the interest function to the given value.
   *
   * @param[in] val The data value.
   *
   * @return Returns the associated interest value.
   */

  double apply(const double val) const;

  /**
   * @brief Get the weight for this interest function.
   *
   * @return Returns the weight associated with this interest function.
   */

  double getWeight() const { return _weight; };

private:

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief X (data) value for the first point in the interest function.
   *
   * @todo It would make more sense to store the function as a sorted vector of
   *       pairs.
   */

  double _x1;

  /**
   * @brief X (data) value for the second point in the interest function.
   */

  double _x2;

  /**
   * @brief X (data) value for the third point in the interest function.
   */

  double _x3;

  /**
   * @brief X (data) value for the fourth point in the interest function.
   */

  double _x4;

  /**
   * @brief X (data) value for the fifth point in the interest function.
   */

  double _x5;

  /**
   * @brief X (data) value for the sixth point in the interest function.
   */

  double _x6;

  /**
   * @brief Y (interest) value for the first point in the interest function.
   */

  double _y1;

  /**
   * @brief Y (interest) value for the second point in the interest function.
   */

  double _y2;

  /**
   * @brief Y (interest) value for the third point in the interest function.
   */

  double _y3;

  /**
   * @brief Y (interest) value for the fourth point in the interest function.
   */

  double _y4;

  /**
   * @brief Y (interest) value for the fifth point in the interest function.
   */

  double _y5;

  /**
   * @brief Y (interest) value for the sixth point in the interest function.
   */

  double _y6;

  /**
   * @brief Weight for this interest function.
   */

  double _weight;

  /**
   * @brief Flag indicating whether the function has been successfully set.
   */

  bool _functionSet;
   
};

#endif
