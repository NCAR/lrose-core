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
 * @file DrawFmqInput.hh
 *
 * @class DrawFmqInput
 *
 * Base class for input handlers.
 *  
 * @date 3/13/2009
 *
 */

#ifndef DrawFmqInput_HH
#define DrawFmqInput_HH

#include "Input.hh"

using namespace std;


/** 
 * @class DrawFmqInput
 */

class DrawFmqInput : public Input
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   */

  DrawFmqInput(const bool apply_polygon_to_all_elevations = false,
	       const bool debug_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~DrawFmqInput(void);
  

  /**
   * @brief Initialize the object.
   *
   * @param[in] prog_name Program name.  Used for debug output.
   * @param[in] cidd_draw_fmq URL of the CIDD draw fmq.
   *
   * @return Returns true on success, false on failure.
   */

  bool init(const string &prog_name,
	    const string &cidd_draw_fmq);
  

  /**
   * @brief Test for end of input.
   *
   * @return Returns true if the input is depleted, false otherwise.
   */

  virtual bool endOfInput();
  

  /**
   * @brief Get the next product from the input stream.
   *
   * @param[out] input_prod The next product from the input stream.
   *
   * @return Returns true on success, false on failure.
   */

  virtual bool getNextProd(Human_Drawn_Data_t &input_prod);
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Flag indicating whether to apply the input polygons to all 
   *        elevation angles.
   */

  bool _applyPolygonToAllElevations;
  
  /**
   * @brief Input CIDD queue.
   */

  DrawQueue _inputQueue;
  
};


#endif
