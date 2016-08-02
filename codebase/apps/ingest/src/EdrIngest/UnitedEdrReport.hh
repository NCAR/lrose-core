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
 *  UnitedEdrReport.hh :
 *                    class for decoding and United specific reports.
 *                    United reports come in bundled with up to 16 Edr reports
 *                    as follows:
 *                        upto 4 reports be "line"
 *                        upto 4 lines per downlink or
 *                    16 minutes of data.
 *                    Interpolation is required to determine location and 
 *                    timestamps of individual reports.
 *
 *                    Turbulence values are encoded.
 *
 *                    This is an implementation of the EdrReport base class
 *
 *                                 
 **************************************************************************/

#ifndef UnitedEdrReport_HH
#define UnitedEdrReport_HH

#include <iostream>
#include <cstdio>
#include <string.h>
#include <map>
#include <fstream>

#include <toolsa/DateTime.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Spdb_typedefs.hh>
#include <rapformats/Edr.hh>
#include <euclid/PjgCalc.hh>
#include <mel_bufr/mel_bufr.h>

#include "UalTailMap.hh"
#include "Params.hh"
#include "lscale.h"
#include "char_rep.h"
#include "turb.h"

#include "EdrReport.hh"


using namespace std;

static map<string,string> ual2fslTailnums;


class UnitedEdrReport: public EdrReport
{

public:

 // Constructors.

 UnitedEdrReport (Params *params, 
                  DateTime &msgtime, 
                  char* tailNum, 
                  char* flightId, 
                  char* aircraftRegistryNum,
                  downlink_t &dnlnk_time);

 UnitedEdrReport (Params *params, 
                  DateTime &msgtime, 
                  char* flightId,
                  char* aircraftRegistryNum);

 // Destructor

 ~UnitedEdrReport(void);

 // Member functions

 void setAndBufferEdr (DateTime msgTime, bool buffer_data = TRUE);
                           // Amount of processing is different depending on the carrier,
                           // some require interpolation for aircraft position.  Output
                           // is written to a SPDB data container.  The Edr.hh rapformat
                           // is used.

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

 status_t decodeBufr (BUFR_Val_t &bv);
                           // Each carrier has specific formats for downlinked EDR
			   // reports; turbulence data in all messages is encrypted.  The EDR
			   // report will have to be decoded up until one can determine
			   // what type of format is used before calling this method.  
                           // The BUFR EDR message starts with the flight number
                           // We will keep processing fields until we find the flight
                           // number. Then we will assume it is an EDR message. If
                           // we fail to find the proper sequence of fields 
                           // we continue and then search for the flight number
                           // again.  After the flight number is found, we get the encoded 
                           // tail number, decode it, and use it to determine the airline.
                           // This code will need to be modified once we get the tailmaps
                           // for Delta and Southwest.  The decoded report is stored in the 
                           // command line argument, report, using the "rapformat" defined 
                           // in rapformats/Edr.hh

protected:


private:

  UalTailMap *tailMap;  

  static char *_className(void)
  {
    return (char *) "UnitedEdrReport";
  }

  int getClosestRep(Edr::edr_t &nearRep);

  int getAircraftSpeed( double &speed, double &dAltDt,
                        double &theta, 
                        Edr::edr_t &nearRep, 
                        time_t recTime);

  int  createTailnumList();

  void decodeEdrDataChars();
  void decodeBufrRepData();

  bool findExactRep();
  int findNextRep();

  int checkBufrVar(char *fxy, BUFR_Val_t &bv);
  int getBufrVar(char *stringVar, char* fxy,  BUFR_Val_t &bv);
  int getBufrVar(int &intVar, char* fxy,  BUFR_Val_t &bv);
  int getBufrVar(float &floatVar, char* fxy,  BUFR_Val_t &bv);


};

#endif

