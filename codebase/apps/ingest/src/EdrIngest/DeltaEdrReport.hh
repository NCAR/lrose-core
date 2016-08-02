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
/*************************************************************************
 *  DeltaEdrReport.hh :
 *                    class for decoding Delta specific reports.
 *                    There two classifications of Delta reports, 
 *                    Heartbeat and Triggered - Functionality will need 
 *                    to be added to fill in null reports using an ASDI
 *                    feed.
 *
 *                    Delta reports use a ARINC 620 specification hybrid for
 *                    triggered reports; differences are centered around how 
 *                    lat/lon values are stored.  Delta used existing MDCRS
 *                    reports for heartbeat messages.  This reports are
 *                    formatted using ARINC 239 specifications.
 *
 *                    This is an implementation of the EdrReport base class
 *                    
 **************************************************************************/

#ifndef DeltaEdrReport_HH
#define DeltaEdrReport_HH

#include <stdio.h>

#include "EdrReport.hh"
#include <rapformats/Edr.hh>
#include <toolsa/DateTime.hh>

using namespace std;

class DeltaEdrReport : public EdrReport
{

public:

 // Constructor.

  DeltaEdrReport(Params *params, 
                 DateTime &msgtime, 
                 char* tailNum, 
                 char* flightId, 
                 char* aircraftRegistryNum,
                 downlink_t &downlink_time);


 // Destructor

  ~DeltaEdrReport(void);

  static const string out_string[7];

  // Member functions

  int setAndBufferEdr (DateTime msgTime);
                           // Amount of processing is different depending on the carrier,
                           // some require interpolation for aircraft position.  Output
                           // is stored in a SPDB data container.  The Edr.hh rapformat
                           // is used.  Once all reports in a downlinked message are
			   // processed the data is written out.

  status_t decodeAscii (char* buffer);
                           // Each carrier has specific formats for downlinked EDR
			   // reports; turbulence data in all messages is encrypted.  The EDR
			   // report will have to be decoded up until one can determine
			   // what type of format is used before calling this method.  This 
                           // "preamble" contains ARINC Aircraft to ground to NCAR routing 
                           // information.  Sometimes, however, it is necessary to read and partially 
                           // decode part of the actual message to determine the format.
			   // When this happens the format argument is used to store the
			   // rest of that line.  The rest of the message is accessed by  
                           // the buffer pointer.  The decoded report is stored in the command line
                           // argument, report, using the "rapformat" defined in /rapformats/Edr.hh


protected:


private:

 static const char *_className(void)
 {
    return (char *)"DeltaEdrReport";
 }

 bool isDelta767 (const char* tailNumber);

 status_t decode_turb_chars (float fields[], int *num_fields, char coded_output[]);

 int decode620format (char *tokenptr, char *format, DateTime &msgTime);

 int decodeMDCRSformat (char *tokenptr, char *format, DateTime &msgTime);

};

#endif

