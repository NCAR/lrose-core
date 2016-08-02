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

# ifndef    OrderedList_H
# define    OrderedList_H

#include <vector>

/**
 * @class OrderedList
 * @brief ordered lists
 *
 * The orderedlist class supports ordering lists based on data values
 * or on another set of input weight values.
 *
 * Permutation indices are used to order, leaving the original data values
 * unmodified.
 */ 
class OrderedList
{
public:

  /**
   * default constructor..empty list
   */
  OrderedList(void);

  /**
   * Destructor
   */
  virtual ~OrderedList(void);

  /**
   * Debug print
   */
  void print(void) const;
   
  /**
   * @return # of things in the list.
   */
  inline int num(void) const {return _n;}

  /**
   * Clear so list is empty
   */
  void clear(void);

  /**
   * add a value, weight assumed = 1.0
   * @param[in] v  Value to add
   */
  void addToListUnordered(double v);

  /**
   * add a value and weight
   * 
   * @param[in] dw  Data and weight, with fist = value, second = weight.
   */
  void addToListUnordered(const std::pair<double,double> &dw);
  
  /**
   * order the data values
   */
  void order(void);

  /**
   * order the weight values
   */
  void orderWeights(void);

  /**
   * @return percentile'th data value from top (data ordered)
   * @param[in] p Percentile
   */
  double percentile(double p);

  /**
   * @return percentile'th data value from top (weight ordered)
   * @param[in] p Percentile
   */
  double weightPercentile(double p);
  
  /**
   * @return average of values between wp0 and wp1 percentile (weight ordered)
   * @param[in] wp0 Lower Percentile
   * @param[in] wp1 Higher Percentile
   */
  double weightConstrainedAverage(double wp0, double wp1);
  
  /**
   * @return ith ordered data value index (from bottom up)
   * @param[in] i
   */
  int ithPerm(int i);


  private:  

  /**
   * values and permutation values.
   */
  std::vector<std::pair<double,int> > _val; 

  /**
   * weight values and permutations for those values.
   */
  std::vector<std::pair<double,int> > _w;

  /**
   * # of data items.
   */
  int _n;

  /**
   * true if data ordered permutations are set.
   */
  bool _is_ordered;

  /**
   * true if weight ordered permutations are set.
   */
  bool _is_ordered_weights;

  inline int _ithi(int i) const {return (_val.begin() + i)->second;}
  inline int _ithwi(int i) const {return (_w.begin() + i)->second;}
  inline double _ithd(int i) const {return (_val.begin() + i)->first;}
  inline double _ithwd(int i) const {return (_w.begin() + i)->first;}
};

# endif
