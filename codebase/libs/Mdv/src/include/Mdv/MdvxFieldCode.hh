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

//////////////////////////////////////////////////////////
// mdv/MdvxFieldCode.hh
//
// Class for providing Mdv field codes
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1999
//
///////////////////////////////////////////////////////////

#ifndef MdvxFieldCode_hh
#define MdvxFieldCode_hh

#define MDVX_MAX_FIELD_CODE  272
#define MDVX_N_FIELD_CODES (MDVX_MAX_FIELD_CODE + 1)

class MdvxFieldCode {
  
public:

  typedef struct {
    int code;
    const char *name;
    const char *units;
    const char *abbrev;
  } entry_t;

  // default constructor
  
  MdvxFieldCode();

  // destructor
  
  virtual ~MdvxFieldCode();

  // lookup entry from code

  static int getEntryByCode(int code, entry_t &entry);

  // lookup entry from name

  static int getEntryByName(const char *name, entry_t &entry);

  // lookup entry from abbrev

  static int getEntryByAbbrev(const char *abbrev, entry_t &entry);

protected:

  static const entry_t _entries[MDVX_N_FIELD_CODES];

private:

};

#endif


