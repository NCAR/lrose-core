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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/04 02:22:11 $
 *   $Id: SpdbDataRetriever.hh,v 1.2 2016/03/04 02:22:11 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * SpdbDataRetriever: Class for retrieving SPDB data.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef SpdbDataRetriever_HH
#define SpdbDataRetriever_HH

#include <vector>

#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>

using namespace std;


class SpdbDataRetriever
{
 public:

  /////////////////////////////
  // Constructors/destructor //
  /////////////////////////////

  /**********************************************************************
   * Constructor
   */

  SpdbDataRetriever(const string &input_url,
		    const int product_id = 0);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~SpdbDataRetriever(void);
  

  /**********************************************************************
   * getData() - Retrieve the SPDB data for the given time range.
   */

  const vector< Spdb::chunk_t > getData(const DateTime start_time,
					const DateTime end_time,
					const int data_type = 0,
					const int data_type2 = 0);


  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * getInputUrl() - Retrieve the input URL for this retriever.
   */

  string getInputUrl() const
  {
    return _inputUrl;
  }


protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  DsSpdb _spdb;
  string _inputUrl;
  

};


#endif
