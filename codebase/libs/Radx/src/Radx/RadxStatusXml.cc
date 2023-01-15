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
// RadxStatusXml.cc
//
// Object for handling statusXml messsages
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2023
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxStatusXml.hh>
#include <Radx/RadxMsg.hh>
#include <iostream>
using namespace std;

//////////////
// Constructor

RadxStatusXml::RadxStatusXml()
  
{
}

/////////////////////////////
// Copy constructor
//

RadxStatusXml::RadxStatusXml(const RadxStatusXml &rhs)
     
{
  _xmlStr = rhs._xmlStr;
}

/////////////
// destructor

RadxStatusXml::~RadxStatusXml()

{
}

/////////////////////////////////////////////////////////
// clear the data in the object

void RadxStatusXml::clear()
  
{
  _xmlStr.clear();
}

/////////////////////////////////////////////////////////
// print

void RadxStatusXml::print(ostream &out) const
  
{
  
  out << "--------------- RadxStatusXml ---------------" << endl;
  out << _xmlStr << endl;
  out << "--------------------------------------------" << endl;

}

/////////////////////////////////////////////////////////
// serialize into a RadxMsg

void RadxStatusXml::serialize(RadxMsg &msg)
  
{

  // init

  msg.clearAll();
  msg.setMsgType(RadxMsg::RadxStatusXmlMsg);
  msg.addPart(_statusXmlStringPartId, _xmlStr.c_str(), _xmlStr.size() + 1);

  // assemble the message from the parts

  msg.assemble();

}

/////////////////////////////////////////////////////////
// deserialize from a RadxMsg
// return 0 on success, -1 on failure

int RadxStatusXml::deserialize(const RadxMsg &msg)
  
{
  
  // initialize object

  clear();

  // check type
  
  if (msg.getMsgType() != RadxMsg::RadxStatusXmlMsg) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxStatusXml::deserialize" << endl;
    cerr << "  incorrect message type" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the xml str

  const RadxMsg::Part *xmlStrPart = msg.getPartByType(_statusXmlStringPartId);
  if (xmlStrPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxStatusXml::deserialize" << endl;
    cerr << "  No xml string part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // check for NULL
  
  char *xml = (char *) xmlStrPart->getBuf();
  size_t bufLen = xmlStrPart->getLength();
  if (xml[bufLen - 1] != '\0') {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxStatusXml::deserialize" << endl;
    cerr << "  XML string not null terminated" << endl;
    string xmlStr(xml, bufLen);
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;    
  }

  _xmlStr = xml;
  return 0;

}

