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
/////////////////////////////////////////////////////////////
// Edr_expanded.cc
//
// C++ class for dealing with Edr data
//    More space was added to allow for future expansion.  4 letter ICAO
//    identifies allowed for internations usage
//
// Gary Blackburn, RAP, NCAR
// PO Box 3000, Boulder, CO, USA
//
// November 2016
//
//////////////////////////////////////////////////////////////


#include <rapformats/Edr_expanded.hh>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
using namespace std;
const fl32 EDR::VALUE_UNKNOWN = -9999.0;

///////////////
// constructor

EDR::EDR()
{
 MEM_zero(_edr);
}

/////////////
// destructor

EDR::~EDR()
{

}

// set methods

void EDR::setEdr(const Edr_t &edr)
{
  _edr = edr;
}


///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void EDR::assemble()
  
{

  _memBuf.free();
  
  Edr_t edr = _edr;
  _edr_to_BE(edr);
  _memBuf.add(&edr, (size_t)sizeof(edr));
  
}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int EDR::disassemble(const void *buf, int len)

{

  int minLen = sizeof(Edr_t);
  if (len < minLen) {
    cerr << "ERROR - EDR::disassemble" << endl;
    cerr << "  Buffer too small for disassemble" << endl;
    cerr << "  Buf len: " << len << endl;
    cerr << "  Min len: " << minLen << endl;
    return -1;
  }


  memcpy(&_edr, (ui08 *) buf, sizeof(Edr_t));
  _edr_from_BE(_edr);
  
  return 0;

}

//////////////////////
// printing object


void EDR::print(ostream &out, string spacer /* = ""*/ ) const

{

  out << spacer << "-----------------------------------" << endl;
  out << spacer << "EDR Report" << endl;
  out << spacer << "data time: " << DateTime::str(_edr.time) << endl;
  out << spacer << "file time: " << DateTime::str(_edr.fileTime) << endl;
  out << spacer << "latitude (deg): " << _edr.lat << endl;
  out << spacer << "longitude (deg): " << _edr.lon << endl;
  out << spacer << "altitude (ft): "  << _edr.alt << endl;
  out << spacer << "interpolated location: ";
     if ( _edr.interpLoc) 
      out << "true" << endl;
     else 
        out << "false" << endl;
  out << spacer << "EDR peak: " << _edr.edrPeak << endl;
  out << spacer << "EDR median: " << _edr.edrAve << endl;
  out << spacer << "wind speed: " << _edr.wspd << endl;
  out << spacer << "wind dir: " << _edr.wdir << endl;
  out << spacer << "temperature: " << _edr.sat << endl;
  out << spacer << "QC conf : " << _edr.qcConf << endl;
  out << spacer << "QC thresh : " << _edr.qcThresh << endl;
  out << spacer << "QC version : " << _edr.qcVersion << endl;
  out << spacer << "EDR alg version : " << _edr.edrAlgVersion << endl;
  out << spacer << "tail number: " << _edr.aircraftRegNum<< endl;
  out << spacer << "encoded tail number: " << _edr.encodedAircraftRegNum<< endl;
  out << spacer << "flight number: " << _edr.flightNum << endl;
  out << spacer << "origination airport: " << _edr.origAirport << endl;
  out << spacer << "destination airport: " << _edr.destAirport << endl;
  out << spacer << "airline ID: " << _edr.airlineId << endl;
  out << spacer << "source name: " << _edr.sourceName << endl;
  out << spacer << "source format: " << _edr.sourceFmt << endl;
  if (_edr.mach != EDR::VALUE_UNKNOWN)
  {
          out << spacer << "   Mach: " << _edr.mach << endl;
  }

  if (_edr.rms != EDR::VALUE_UNKNOWN)
  {
          out << spacer << "   Rms: " << _edr.rms << endl;
  }

  if (_edr.computedAirspeed != EDR::VALUE_UNKNOWN)
  {
          out << spacer << "   Computed Air Speed: " << _edr.computedAirspeed << endl;
  }

  out << spacer << "Flags:" << endl;

  if (_edr.PeakConf != (int)EDR::VALUE_UNKNOWN)
  {
          out << spacer << "   PeakConf: " << _edr.PeakConf << endl;
  }

  if (_edr.MeanConf != (int)EDR::VALUE_UNKNOWN)
  {
          out << spacer << "   MeanConf: " << _edr.MeanConf << endl;
  }

  if (_edr.runningMinConf != EDR::VALUE_UNKNOWN)
  {
          out << spacer << "   runningMinConf " <<  _edr.runningMinConf << endl;
  }

  if (_edr.maxNumBad != (int)EDR::VALUE_UNKNOWN)
  {
          out << spacer << "   maxNumBad " <<  _edr.maxNumBad << endl;
  }

  if (_edr.QcDescriptionBitFlags != (int)EDR::VALUE_UNKNOWN)
  {
        // print each flag value and its cooresponding meaning once defined
        if (_edr.QcDescriptionBitFlags > 0) 
           out << spacer   << "   QC desciption Flags " << endl;
        if (_edr.QcDescriptionBitFlags & BELOW_MIN_ALT)
          out << spacer << "      Below minimum altitude " << endl;
        if (_edr.QcDescriptionBitFlags & FAILED_ONBOARD_QC)
          out << spacer << "      Failed on board QC" << endl;
        if (_edr.QcDescriptionBitFlags & FAILED_BOUNDS_CK)
          out << spacer << "      Failed bounds check" << endl;
        if (_edr.QcDescriptionBitFlags & BAD_TAIL)
          out << spacer << "      Bad Tail" << endl;
        if (_edr.QcDescriptionBitFlags & LOW_ONBOARD_QC)
          out << spacer << "      Low on board QC" << endl;
        if (_edr.QcDescriptionBitFlags & UNKNOWN_TAIL_STATUS)
          out << spacer << "      Unknown ground based QC status" << endl;

  }

  if (_edr.PeakLocation != (int)EDR::VALUE_UNKNOWN)
  {
          out << spacer << "   PeakLocaton: " << _edr.PeakLocation << endl;
  }

  if (_edr.NumGood != (int)EDR::VALUE_UNKNOWN)
  {
          out << spacer << "   NumGood: " << _edr.NumGood << endl;
  }


  for (int i = 0; i < NUM_QC_INDS - 2; i++)
  {
       if (_edr.edrAveQcInds[i] != (int)EDR::VALUE_UNKNOWN)
       {
          out << spacer << "   edrAveQcInd: " << _edr.edrAveQcInds[i] << endl;
       }
  }

  for (int i = 0; i < EDR::NUM_QC_FLAGS; i++)
  {
       if (_edr.edrPeakQcFlags[i] != (int)EDR::VALUE_UNKNOWN)
          out << spacer << "   edrPeakQcFlag: " << _edr.edrPeakQcFlags[i] << endl;
  }

  for (int i = 0; i < EDR::NUM_QC_FLAGS; i++)
  {
       if (_edr.edrAveQcFlags[i] != (int)EDR::VALUE_UNKNOWN)
          out << spacer << "   edrAveQcFlag: " << _edr.edrAveQcFlags[i] << endl;
  }

}

/////////////////
// byte swapping

void EDR::_edr_to_BE(Edr_t &rep)
{
  BE_from_array_32(&rep, NBYTES_32);
}

void EDR::_edr_from_BE(Edr_t &rep)
{
  BE_to_array_32(&rep, NBYTES_32);
}



