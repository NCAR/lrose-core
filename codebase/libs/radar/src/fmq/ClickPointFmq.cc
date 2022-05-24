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
///////////////////////////////////////////////////////////////
// ClickPointFmq.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2021
//
///////////////////////////////////////////////////////////////
//
// Reads/writes FMQ with click point for displays
//
////////////////////////////////////////////////////////////////

#include <radar/ClickPointFmq.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaXml.hh>
#include <unistd.h>
using namespace std;

const double ClickPointFmq::missingValue = -9999.0;

// Constructors

ClickPointFmq::ClickPointFmq()
{
  _init();
}

ClickPointFmq::ClickPointFmq(const string &url) :
        _url(url)
{
  _init();
}

// destructor

ClickPointFmq::~ClickPointFmq()

{

  _fmq.closeMsgQueue();

}

// initialize

void ClickPointFmq::_init()
{

  _debug = false;
  _verbose = false;

  _msgTimeSecs = 0;
  _dataTimeSecs = 0;
  _dataNanoSecs = 0;
  _elevation = 0;
  _azimuth = 0;
  _rangeKm = 0;
  _gateNum = 0;

}

/////////////////////////////////////////////////
// read click point data from FMQ
// Returns 0 on success, -1 on failure

int ClickPointFmq::read(bool &gotNew)
  
{
  
  // check we have an open FMQ
  
  if (_checkFmqIsOpen()) {
    return -1;
  }

  // read in a new message
  
  if (_fmq.readMsg(&gotNew)) {
    cerr << "ERROR -  ClickPointFmq::read" << endl;
    cerr << "  Cannot read click point info from FMQ" << endl;
    cerr << "  Fmq: " << _url << endl;
    cerr << _fmq.getErrStr() << endl;
    _fmq.closeMsgQueue();
    return -1;
  }
  
  if (!gotNew) {
    // no data
    return 0;
  }
  
  // get the xml, ensure null termination
  
  const void *msg = _fmq.getMsg();
  int len = _fmq.getMsgLen();
  TaArray<char> xml_;
  char *xml = xml_.alloc(len);
  memcpy(xml, msg, len);
  xml[len-1] = '\0';
  _xml = xml;

  if (_verbose) {
    cerr << "=========== latest click point XML ==================" << endl;
    cerr << _xml << endl;
    cerr << "=====================================================" << endl;
  }
  
  // decode the XML

  bool success = true;

  time_t msgTimeSecs;
  if (TaXml::readTime(_xml, "msgTimeSecs", msgTimeSecs)) {
    success = false;
  }
  int writerPid;
  if (TaXml::readInt(_xml, "writerPid", writerPid)) {
    success = false;
  }
  time_t dataTimeSecs;
  if (TaXml::readTime(_xml, "dataTimeSecs", dataTimeSecs)) {
    success = false;
  }
  int dataNanoSecs;
  if (TaXml::readInt(_xml, "dataNanoSecs", dataNanoSecs)) {
    success = false;
  }
  double azimuth;
  if (TaXml::readDouble(_xml, "azimuth", azimuth)) {
    success = false;
  }
  double elevation;
  if (TaXml::readDouble(_xml, "elevation", elevation)) {
    success = false;
  }
  double rangeKm;
  if (TaXml::readDouble(_xml, "rangeKm", rangeKm)) {
    success = false;
  }
  int gateNum;
  if (TaXml::readInt(_xml, "gateNum", gateNum)) {
    success = false;
  }
  if (!success) {
    cerr << "ERROR - ClickPointFmq::read" << endl;
    cerr << "Cannot decode click point XML:" << endl;
    cerr << _xml << endl;
    return -1;
  }

  _msgTimeSecs = msgTimeSecs;
  _writerPid = writerPid;
  _dataTimeSecs = dataTimeSecs;
  _dataNanoSecs = dataNanoSecs;
  _dataTime.set(dataTimeSecs, (double) dataNanoSecs * 1.0e-9);
  _elevation = elevation;
  _azimuth = azimuth;
  _rangeKm = rangeKm;
  _gateNum = gateNum;
  
  if (_debug) {
    cerr << "=========== latest click point XML ==================" << endl;
    cerr << "  msgTime: " << DateTime::strm(_msgTimeSecs) << endl;
    cerr << "  writerPid: " << _writerPid << endl;
    cerr << "  dataTime: " << _dataTime.asString(6) << endl;
    cerr << "  elevation: " << _elevation << endl;
    cerr << "  azimuth: " << _azimuth << endl;
    cerr << "  rangeKm: " << _rangeKm << endl;
    cerr << "  gateNum: " << gateNum << endl;
    cerr << "=====================================================" << endl;
  }

  // check that we did not write this message

  int pid = getpid();
  if (pid == _writerPid) {
    // ignore this message if this process wrote it
    if (_debug) {
      cerr << "Ignoring message, we wrote it" << endl;
    }
    gotNew = false;
    return 0;
  }

  // check that the message is not too old

  time_t now = time(NULL);
  double msgAge = now - _msgTimeSecs;
  if (msgAge > 60.0) {
    // old message
    if (_debug) {
      cerr << "WARNING - ClickPointFmq::read" << endl;
      cerr << "  message is too old, age: " << msgAge << endl;
      cerr << _xml << endl;
    }
    gotNew = false;
    return 0;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////
// write click point data, in XML format, to FMQ

int ClickPointFmq::write(time_t dataTimeSecs,
                         int dataNanoSecs,
                         double azimuth,
                         double elevation,
                         double rangeKm,
                         int gateNum)

{
  
  if (_verbose) {
    cerr << "ClickPointFmq::write() called" << endl;
  }

  // create XML

  time_t now = time(NULL);
  int pid = getpid();

  string xml;

  xml.append(TaXml::writeStartTag("ClickPoint", 0));
  
  xml.append(TaXml::writeTime("msgTimeSecs", 1, now));
  xml.append(TaXml::writeInt("writerPid", 1, pid));
  xml.append(TaXml::writeTime("dataTimeSecs", 1, dataTimeSecs));
  xml.append(TaXml::writeInt("dataNanoSecs", 1, dataNanoSecs));
  xml.append(TaXml::writeDouble("azimuth", 1, azimuth));
  xml.append(TaXml::writeDouble("elevation", 1, elevation));
  xml.append(TaXml::writeDouble("rangeKm", 1, rangeKm));
  xml.append(TaXml::writeInt("gateNum", 1, gateNum));
  
  xml.append(TaXml::writeEndTag("ClickPoint", 0));
  
  // check FMQ is open
  
  if (_checkFmqIsOpen() == 0) {
    
    if (_verbose) {
      cerr << "====>> ClickPointFmq::write() <<====" << endl;
      cerr << "====>> writing Click Point XML to FMQ <<====" << endl;
      cerr << "====>> url: " << _url << " <<====" << endl;
      cerr << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
      cerr << xml;
      cerr << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
    }

    // write to output FMQ
    
    if (_fmq.writeMsg(0, 0, xml.c_str(), xml.size() + 1)) {
      cerr << "ERROR - ClickPointFmq::write()" << endl;
      cerr << "  _url: " << _url << endl;
      cerr << _fmq.getErrStr() << endl;
      _fmq.closeMsgQueue();
      return -1;
    }

  }

  return 0;
}

//////////////////////////////////////////
// Check FMQ for click point XML is open
// returns 0 on success, -1 on failure

int ClickPointFmq::_checkFmqIsOpen()
{
  
  if (_fmq.isOpen()) {
    return 0;
  }
  
  // create output FMQ

  bool compression = false;
  size_t nSlots = 100;
  size_t bufSize = 1000000;
  if (_fmq.initReadWrite(_url.c_str(),
                         "ClickPointFmq", _verbose,
                         DsFmq::END, compression,
                         nSlots, bufSize)) {
    cerr << "WARNING - ClickPointFmq::_checkIsOpen" << endl;
    cerr << "  Cannot create fmq for click point data" << endl;
    cerr << "  URL: " << _url << endl;
    cerr << "  nslots: " << nSlots << endl;
    cerr << "  size: " << bufSize << endl;
    cerr << _fmq.getErrStr() << endl;
    return -1;
  }
  
  return 0;

}
