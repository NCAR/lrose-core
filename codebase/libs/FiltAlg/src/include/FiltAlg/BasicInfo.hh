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
 * @file BasicInfo.hh 
 * @brief information from alg. written to XML in MDV and SPDB
 * @class BasicInfo
 * @brief information from alg. written to XML in MDV and SPDB
 *
 * This is derived from Info
 */

#ifndef BASIC_INFO_H
#define BASIC_INFO_H
#include <string>
#include <toolsa/LogStream.hh>
#include <FiltAlg/Info.hh>

//------------------------------------------------------------------
class BasicInfo : public Info
{
public:

  /**
   * Empty
   */
  BasicInfo(void);

  /**
   * Construct from XML content
   * @param[in] s  XML content
   */
  BasicInfo(const string &s);

  /**
   * Destructor
   */
  virtual ~BasicInfo(void);

  /**
   * Clear to empty
   */
  virtual void dclear(void);

  /**
   * Print
   * @param[in] e Severity indicating logging or not
   */
  virtual void dprint(const LogStream::Log_t e) const;

  /**
   * Initialize
   */
  virtual void dinit(void);

  /**
   * @return XML string for derived information.
   */
  virtual string dwrite_xml(void) const;

  /**
   * Get state from XML data
   * @param[in] s  String from which to get XML
   * @return true if successful
   */
  virtual bool dset_from_xml(const string &s);

protected:
private:

};

#endif
