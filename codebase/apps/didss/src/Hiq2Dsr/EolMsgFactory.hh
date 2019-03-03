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
/*********************************************************************
 * EolMsgFactory : Class representing a message from an HiQ radar
 *                 processor using the old EOL messages.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#ifndef EolMsgFactory_hh
#define EolMsgFactory_hh


#include "EolBeamMsg.hh"
#include "EolRadarMsg.hh"
#include "MsgFactory.hh"

using namespace std;


class EolMsgFactory : public MsgFactory
{

public:

  /*********************************************************************
   * Constructors.
   */

  EolMsgFactory();

  /*********************************************************************
   * Destructor
   */

  virtual ~EolMsgFactory();

  /*********************************************************************
   * createMessage() - Create a EolMsgFactory object using the given buffer
   *                   of data.
   */

  HiqMsg *createMessage(const char *buffer,
			const int buffer_len);


private:

  /*********************************************************************
   * _createBeamMessage() - Create a EolBeamMsg object using the given
   *                        buffer of data.
   */

  static EolBeamMsg *_createBeamMessage(const char *buffer,
					const int buffer_len);


  /*********************************************************************
   * _createRadarMessage() - Create an EolRadarMsg object using the given
   *                         buffer of data.
   */

  static EolRadarMsg *_createRadarMessage(const char *buffer,
					  const int buffer_len);


};

#endif
