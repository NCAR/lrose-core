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
 * @file Endpts.hh
 * @brief Endpoint index handling for line segments
 * @class Endpts
 * @brief Endpoint index handling for line segments
 *
 * Endpts is typically an Attribute attached to a Line.
 * It allows one to refer back to a LineList where the endpoints of the
 * Line can be found. Each endpoint of the Line comes from a particular
 * endpoint of a particular Line in the LineList (not neccessarily the
 * same Line within the LineList).
 */

# ifndef    ENDPTS_H
# define    ENDPTS_H

#include <string>

class Endpts
{
public:
  /**
   * Constructor sets all indices to -1
   */
  Endpts(void);

  /**
   * Constructor that sets values
   * @param[in]  index0  LineList Line index for 0'th endpoint of a Line
   * @param[in]  end0    LineList Line endpoint index for endpoint 0.
   * @param[in]  index1  LineList Line index for 1'th endpoint of a Line
   * @param[in]  end1    LineList Line endpoint index for endpoint 1.
   */
  Endpts(int index0, int end0, int index1, int end1);

  /**
   * Destructor
   */
  virtual ~Endpts();

  /**
   * Operator==
   * @param[in] p
   */
  bool operator==(const Endpts &p) const;

  /**
   * @return XML representation of state
   */
  std::string writeXml(void) const;

  /**
   * Parse an XML string to set state
   * @param[in] xml
   */
  bool readXml(const std::string &xml);

  /**
   * Return the line index and endpoint index for one endpoint of this line
   * @param[in] which 0 or 1 for endpoint of this line
   * @param[out] index
   * @param[out] endpt
   */
  void get(int which, int &index, int &endpt) const;

  /**
   * @return true if the endpoints are for a single Line  in the LineList
   *
   * This is true if both index values are the same and the endpoint indices
   * are 0 and 1 respectively
   */
  inline bool isSimple1(void) const
  {
    if (_index0 != _index1)
      return false;
    return (_endpt0 == 0 && _endpt1 == 1);
  }

  /**
   * Merge input state with local state to give a new state
   * @param[in] i  Values to merge
   *
   * Want to set so the line indices are as far apart as possible and
   * the endpoint indices have the most extent as possible.
   */
  void merge(const Endpts &i);

  /**
   * @return an Endpts object with the '0' values from the local object
   * and the '1' values from the input object
   *
   * @param[in] i  Input object from which to get 1 values
   */
  Endpts average(const Endpts &i) const;

  /**
   * Debug print
   *
   * @param[in] fp
   */
  void print(FILE *fp) const;

  /**
   * Debug print
   */
  void print(void) const;

  /**
   * Debug print
   * @return string representation of debug print
   */
  std::string sprint(void) const;

private:
  int _index0;  /**< 0th endpoint Line index in LineList */
  int _endpt0;  /**< 0th endpoint end index for Line in LineList */
  int _index1;  /**< 1th endpoint Line index in LineList */
  int _endpt1;  /**< 1th endpoint end index for Line in LineList */
};

# endif
