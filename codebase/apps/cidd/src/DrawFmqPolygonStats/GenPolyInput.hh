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
 * @file GenPolyInput.hh
 *
 * @class GenPolyInput
 *
 * Base class for input handlers.
 *  
 * @date 3/13/2009
 *
 */

#ifndef GenPolyInput_HH
#define GenPolyInput_HH

#include <Spdb/DsSpdb.hh>
#include <rapformats/GenPolyStats.hh>
#include <toolsa/DateTime.hh>

#include "Input.hh"

using namespace std;


/** 
 * @class GenPolyInput
 */

class GenPolyInput : public Input
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

  GenPolyInput(const bool debug_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~GenPolyInput(void);
  

  /**
   * @brief Initialize the object.
   *
   * @param[in] cidd_draw_fmq URL of the GenPoly database.
   * @param[in] start_time Start time for database data.
   * @param[in] end_time End time for database data.
   *
   * @return Returns true on success, false on failure.
   */

  bool init(const string &gen_poly_url,
	    const DateTime &start_time,
	    const DateTime &end_time);
  

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
   * @brief Input database.
   */

  DsSpdb _spdb;
  
  /**
   * @brief Chunks read in from the database.
   */

  vector< Spdb::chunk_t > _spdbChunks;
  
  /**
   * @brief Pointer to the next chunk to be processed.
   */

  vector< Spdb::chunk_t >::iterator _currChunk;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  inline static double _getFieldValue(const GenPolyStats &polygon,
				      const string &field_name)
  {
    int field_num;

    if ((field_num = polygon.getFieldNum(field_name)) < 0)
      return -999.0;
    
    return polygon.get1DVal(field_num);
  }
  
};


#endif
