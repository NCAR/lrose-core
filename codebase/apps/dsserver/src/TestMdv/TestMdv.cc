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
// TestMdv.cc
//
// TestMdv object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////

#include "TestMdv.hh"
#include "Args.hh"
#include "Params.hh"
#include <didss/DsDataFile.hh>
#include <dsserver/DsLocator.hh>
#include <dsserver/DsMdvAccess.hh>
#include <dsserver/DsSvrMgrSocket.hh>
#include <mdv/MdvVsection.hh>

#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <euclid/TypeGrid.hh>
using namespace std;

// Constructor

TestMdv::TestMdv(int argc, char **argv)

{

  OK = TRUE;
  _progName = NULL;
  _args = NULL;
  _params = NULL;

  // set programe name

  _progName = STRdup("TestMdv");

  // get command line args
  
  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _params = new Params();
  _paramsPath = (char *) "unknown";
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  return;

}

// destructor

TestMdv::~TestMdv()

{

  // free up

  if (_progName) {
    STRfree(_progName);
  }
  if (_args) {
    delete(_args);
  }
  if (_params) {
    delete(_params);
  }

}

//////////////////////////////////////////////////
// Run

int TestMdv::Run()
{

  DateTime fetchDate( DateTime::parseDateTime(_params->data_time) );
  DsURL fetchURL(_params->url);

  if (!fetchURL.isValid()) {
    cerr << "Could not create valid URL." << endl;
    return -1;
  }

  if (_params->mode == Params::GET_VOLUME_MODE) {
    getVolume(fetchURL, fetchDate);
  } else if (_params->mode == Params::GET_VSECTION_MODE) {
    getVsection(fetchURL, fetchDate);
  }

/*
  // Shutdown the server.
  DsServerMsg msg;
  msg.setCategory(DsServerMsg::ServerStatus);
  msg.setType(DsServerMsg::SHUTDOWN);
  void * msgToSend = msg.assemble();
  int msgLen = msg.lengthAssembled();
  Socket sock;
  sock.open("saturn", 10000);
  sock.writeMessage(0, msgToSend, msgLen);
  sock.close();
*/
  
/*
  if (_params->mode == Params::ASCII_COPY_MODE) {
    if (_asciiCopy()) {
      cerr << "Ascii copy failed" << endl;
    }
  } else if (_params->mode == Params::BINARY_COPY_MODE) {
    if (_binaryCopy()) {
      cerr << "Binary copy failed" << endl;
    }
  } else if (_params->mode == Params::PRINTF_MODE) {
    if (_printf()) {
      cerr << "Printf failed" << endl;
    }
  } else {
    cerr << "Unknown mode: " << endl;
  }
*/

  return (0);

}

////////////////////////
// get volume

void TestMdv::getVolume(const DsURL & url, const DateTime & time)
{
  // Todo: use this instead of the DsMdvServerSocket stuff, below.
  // DsLocator locator(url);
  // int status = locator.resolve();
  // if (status < 0) {
  //   cerr << "Error: Could not resolve port on URL: " << url.getURLStr() << endl;
  //   return;
  // }

  // Todo: Take this out.
  cerr << "Getting volume at time: " << time << endl;

/*
  DsSvrMgrSocket smSocket;
  string statusString;
  // Todo: make copy of url so it's not const.
  smstatus = smSocket.findPortForURL(url.getHost().c_str(),
  url, -1, statusString);
  if (smstatus < 0) {
      cout << "Could not get filled-out url from server mgr. errNum: "
           << smSocket.getErrNum() << ". Error String: " << endl
           << "    " << smSocket.getErrString() << endl;
  }
*/

  // Create a field and grid holding the necessary info.
  TypeGrid<unsigned char> grid(Grid::CHAR_GRID);
  MdvFile requestFile("./");
/*
Put this in for normal request.
  MdvField * field = requestFile.addField("Reply Field", grid);
  field->setSourceFieldNum(0);
  field->setDateTime(time);
*/
/*
  field->setBoundingBox(20.0, 117.0, 20.45, 117.5);
  // field->setSearchZRequest(true);
  // field->setPlaneNum(-300);
  GridGeom geom = grid.getGeometry();

  // Either of these work.
  geom.set(1, GridGeom::UNKNOWN_RESOLUTION, 10);
  // geom.set(1, 10, 275);
  grid.setGeometry(geom);
*/

  // TypeGrid<unsigned char> grid2(Grid::CHAR_GRID, grid);
  // MdvField * field2 = requestFile.addField("Reply Field 2", grid2);
  // *field2 = *field;
  // field2->setSourceFieldNum(7);

  // For now do a get closest at the indicated time.
  DsMdvAccess mdv;
  // MDV_master_header_t replyHeader;
  // MDV_init_master_header(&replyHeader);

  // Todo: Change readClosest so it uses the time off the field rather than
  //         a passed-in time.
  string statusString;
  int status = mdv.readClosest(requestFile, url, time,
			       3600, DsDataFile::DS_SEARCH_BEFORE,
			       // 7200, DsDataFile::DS_SEARCH_CENTERED,
			       statusString);
  if (status < 0) {
    cerr << "Error: could not read:" << endl << statusString << endl;
  }

  status = requestFile.write(916512000, MDV_INT8);
  if (status != MDV_SUCCESS) {
	cerr << "Could not write the file!" << endl;
  }
}

/////////////////////////
// get vertical section

void TestMdv::getVsection(const DsURL & url, const DateTime & time)

{

  cerr << "Getting vsection at time: " << time << endl;

  MdvVsection vsect;

  // clear out request - not really necessary since object has just
  // been constructed

  vsect.clearRequest();

  // field numbers and names

  if (_params->vsection_set_field_nums) {
    for (int i = 0; i < _params->vsection_field_nums_n; i++) {
      vsect.addFieldRequest(_params->_vsection_field_nums[i]);
    }
  } else if (_params->vsection_set_field_names) {
    for (int i = 0; i < _params->vsection_field_names_n; i++) {
      vsect.addFieldRequest(_params->_vsection_field_names[i]);
    }
  } 

  // vertical limits

  if (_params->vsection_set_plane_num_limits) {
    vsect.setPlaneNumLimits(_params->vsection_lower_plane_num,
			    _params->vsection_upper_plane_num);
  } else if (_params->vsection_set_plane_vlevel_limits) {
    vsect.setPlaneVlevelLimits(_params->vsection_lower_plane_vlevel,
			       _params->vsection_upper_plane_vlevel);
  }
  // waypoints

  for (int i = 0; i < _params->vsection_waypts_n; i++) {
    vsect.addWayPt(_params->_vsection_waypts[i].lat,
		   _params->_vsection_waypts[i].lon);
  }

  DsMdvAccess mdv;
  string statusString;
  int status = mdv.readClosest(vsect, url, time,
			       3600, DsDataFile::DS_SEARCH_BEFORE,
			       statusString);
  if (status < 0) {
    cerr << "Error: could not read vsect:" << endl << statusString << endl;
  } else {
    vsect.printRequest(cerr);
    vsect.printSampleSummary(cerr);
    cerr << "Nfields: " << vsect.getNFields() << endl;
    for (int ifield = 0; ifield < vsect.getNFields(); ifield++) {
      vsect.printFieldSummary(cerr, ifield);
      vsect.printFieldData(cerr, ifield);
    }
  }

}

