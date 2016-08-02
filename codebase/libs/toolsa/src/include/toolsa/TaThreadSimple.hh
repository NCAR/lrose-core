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
 * @file TaThreadSimple.hh
 * @brief Instantiation of TaThread that uses the method pointer and an index
 *
 * @class TaThreadSimple
 * @brief Instantiation of TaThread that uses a method pointer and has an index
 *        
 * Derived from the TaThread class, implements the run() virtual
 * method, which runs the _method member function.
 * The _method is assumed called in this way:  _method(_info);
 *
 * This class has its own virtual methods  _before() and _after()
 * which are actions to take before and after '_method()' respectively,
 * all three are called from within run(). 
 *
 * The default base action of each of _before() and _after() is no action.
 *
 * The idea is that _method() is all the actions that do not involve the
 * threading and _before() and _after() are actions that do involve the
 * threading.
 *
 * @note the _threadInfo member pointer is set in the TaThreadQue class,
 * all other instantiations must set the pointer.
 *
 * @note the _threadMethod member pointer must be set in the
 * TaThreadQue::clone() method if that class is being used, otherwise the
 * instantiation must set this pointer.
 */

#ifndef TaThreadSimple_h
#define TaThreadSimple_h

#include <toolsa/TaThread.hh>

class TaThreadSimple : public TaThread
{
public:

  /**
   * Constructor 
   * @param[in] index  Index value to use for member _threadIndex
   */
  TaThreadSimple(const int index);

  /**
   * Destructor.
   */
  virtual ~TaThreadSimple(void);

  /**
   * @return the index value 
   */  
  inline int getThreadIndex(void) const {return _threadIndex;}

  /**
   * Perform the computations that this thread computes, using 
   * _info and _method pointers
   *
   * Does this:
   *
   * - _before();
   * - (*_method)(_info);
   * - _after();
   */
  virtual void run(void);

protected:

  int _threadIndex;  /**< An identifier set by constructor */

private:

  inline virtual void _before(void) { }
  inline virtual void _after(void) { }

};


#endif

