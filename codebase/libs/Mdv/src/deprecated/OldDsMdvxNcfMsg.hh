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

///////////////////////////////////////
// Mdv/OldDsMdvxNcfMsg.hh
//
// DsMessage class for DsMdvx class
///////////////////////////////////////

#ifndef OldDsMdvxNcfMsg_hh
#define OldDsMdvxNcfMsg_hh

#include <didss/DsDataFile.hh>
#include <Mdv/DsMdvxMsg.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <vector>
#include <iostream>
using namespace std;

class OldDsMdvxNcfMsg : public DsMdvxMsg {

  friend class DsMdvx;

public:
  
  OldDsMdvxNcfMsg(memModel_t mem_model = CopyMem);

  virtual ~OldDsMdvxNcfMsg();

  // assemble message to convert to ncf
  // Returns assembled message on success, NULL on error.
  // getErrorStr() returns the error string.
  
  void *assembleConvertMdv2Ncf(const DsMdvx &mdvx,
                               const string &trans_url);
  void *assembleConvertMdv2NcfReturn(const DsMdvx &mdvx);
  
  // assemble message to convert ncf format to mdv
  // Returns assembled message, NULL on failure
  
  void *assembleConvertNcf2Mdv(const DsMdvx &mdvx,
                               const string &trans_url);
  void *assembleConvertNcf2MdvReturn(const DsMdvx &mdvx);

  // assemble message to read all headers from NCF type files
  // these are CF-compliant netCDF
  // Returns assembled message, NULL on failure
  
  void *assembleReadAllHdrsNcf(const DsMdvx &mdvx,
			       const string &trans_url);
  void *assembleReadAllHdrsNcfReturn(const DsMdvx &mdvx);
  
  // assemble message to read NCF type files
  // these are CF-compliant netCDF
  // Returns assembled message, NULL on failure
  
  void *assembleReadNcf(const DsMdvx &mdvx,
                        const string &trans_url);
  void *assembleReadNcfReturn(const DsMdvx &mdvx);
  
  // assemble message to read all headers from Radx type files
  // these are radial radar data
  // Returns assembled message, NULL on failure
  
  void *assembleReadAllHdrsRadx(const DsMdvx &mdvx,
			       const string &trans_url);
  void *assembleReadAllHdrsRadxReturn(const DsMdvx &mdvx);
  
  // assemble message to read RADX type files
  // these are radial radar data
  // Returns assembled message, NULL on failure
  
  void *assembleReadRadx(const DsMdvx &mdvx,
                         const string &trans_url);
  void *assembleReadRadxReturn(const DsMdvx &mdvx);

  // assemble message to constrain ncf using
  // read MDVX read constraints
  // Returns assembled message, NULL on failure
  
  void *assembleConstrainNcf(const DsMdvx &mdvx,
                             const string &trans_url);
  void *assembleConstrainNcfReturn(const DsMdvx &mdvx);

protected:

  void _addConvertMdv2NcfPart(const DsMdvx &mdvx);
  
  void _addNcfHdr(const DsMdvx &mdvx);
  void _addNcfHdrAndData(const DsMdvx &mdvx);
  
  int _getNcfHeaderParts(DsMdvx &mdvx);
  int _getNcfParts(DsMdvx &mdvx);
  int _getConvertMdv2Ncf(DsMdvx &mdvx);

  // disassemble methods

  int _disassembleConvertMdv2Ncf(DsMdvx &mdvx);
  int _disassembleConvertMdv2NcfReturn(DsMdvx &mdvx);

  int _disassembleConvertNcf2Mdv(DsMdvx &mdvx);
  int _disassembleConvertNcf2MdvReturn(DsMdvx &mdvx);

  int _disassembleReadAllHdrsNcf(DsMdvx &mdvx);
  int _disassembleReadAllHdrsNcfReturn(DsMdvx &mdvx);
  
  int _disassembleReadNcf(DsMdvx &mdvx);
  int _disassembleReadNcfReturn(DsMdvx &mdvx);
  
  int _disassembleReadAllHdrsRadx(DsMdvx &mdvx);
  int _disassembleReadAllHdrsRadxReturn(DsMdvx &mdvx);
  
  int _disassembleReadRadx(DsMdvx &mdvx);
  int _disassembleReadRadxReturn(DsMdvx &mdvx);

  int _disassembleConstrainNcf(DsMdvx &mdvx);
  int _disassembleConstrainNcfReturn(DsMdvx &mdvx);

private:

  // Private members with no bodies provided -- do not use until defined.
  OldDsMdvxNcfMsg(const OldDsMdvxNcfMsg & orig);
  void operator= (const OldDsMdvxNcfMsg & other);

};

#endif
