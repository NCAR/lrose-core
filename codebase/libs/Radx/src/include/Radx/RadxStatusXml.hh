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
// RadxStatusXml.hh
//
// Object for handling statusXml messsages
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2023
//
///////////////////////////////////////////////////////////////

#ifndef RadxStatusXml_HH
#define RadxStatusXml_HH

#include <string>
#include <Radx/Radx.hh>
class RadxMsg;
using namespace std;

//////////////////////////////////////////////////////////////////////
/// CLASS FOR HANDLING STATUS XML MESSAGES
/// 

class RadxStatusXml {

public:

  /// Constructor
  
  RadxStatusXml();
  
  /// Copy constructor
  
  RadxStatusXml(const RadxStatusXml &rhs);

  /// Destructor
  
  virtual ~RadxStatusXml();

  //////////////////////////////////////////////////////////////////
  /// \name Set methods:
  //@{

  /// Set xml string

  void setXmlStr(const string &val) { _xmlStr = val; }

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Get methods:
  //@{

  /// Get xml string

  inline const string &getXmlStr() const { return _xmlStr; }

  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Clearing data:
  //@{

  /// Clear all of the data in the object.
  
  void clear();
  
  //@}

  //////////////////////////////////////////////////////////////////
  /// \name Printing:
  //@{
  
  /// Print metadata.
  
  void print(ostream &out) const;
  
  //@}

  /// \name Serialization:
  //@{

  // serialize into a RadxMsg
  
  void serialize(RadxMsg &msg);
  
  // deserialize from a RadxMsg
  // return 0 on success, -1 on failure

  int deserialize(const RadxMsg &msg);

  //@}
  
protected:
  
private:

  string _xmlStr;

  // private methods
  
  void _init();
  RadxStatusXml & _copy(const RadxStatusXml &rhs);

  /////////////////////////////////////////////////
  // serialization
  /////////////////////////////////////////////////
  
  static const int _statusXmlStringPartId = 1;
  
};

#endif
