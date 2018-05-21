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
// InputUdp.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2003
//
///////////////////////////////////////////////////////////////
//
// InputUdp reads data in from UDP packets
//
////////////////////////////////////////////////////////////////

#include "InputUdp.hh"
#include "Fields.hh"
#include <toolsa/pmu.h>
#include <toolsa/sockutil.h>
#include <cmath>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#if defined(__linux)
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

using namespace std;

// Constructor

InputUdp::InputUdp(const string &prog_name,
		   const Params &params) :
  _progName(prog_name),
  _params(params),
  _beam(params)
  
{

  _udpFd = -1;
  _volHdrAvail = false;
  _volHdrIsNew = false;
  _nBeamPacketsSave = 0;
  _beamIsNew = false;

}


// destructor

InputUdp::~InputUdp()

{

  closeUdp();
  
}

///////////////////
// open port

int InputUdp::openUdp()
  
{

  closeUdp();
  
  // get socket

  if  ((_udpFd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    int errNum = errno;
    cerr << "ERROR - InputUdp::open" << endl;
    cerr << "  Could not create socket." << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // set the socket for reuse

  int val = 1;
  int valen = sizeof(val);
  setsockopt(_udpFd, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);
  
  // bind local address to the socket

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof (addr));
  addr.sin_port = htons(_params.udp_port);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if (::bind (_udpFd, (struct sockaddr *) &addr, sizeof (addr)) < 0) {
    int errNum = errno;
    cerr << "ERROR - InputUdp::open" << endl;
    cerr << "  Bind error, udp port: " << _params.udp_port << endl;
    cerr << "  " << strerror(errNum) << endl;
    closeUdp();
    return -1;
  }

  if (_params.debug) {
    cerr << "Opened UDP port: " << _params.udp_port << endl;
  }
  
  return 0;
  

}

/////////////
// close port

void InputUdp::closeUdp()
  
{
  
  if (_udpFd >= 0) {
    close(_udpFd);
  }
  _udpFd = -1;

}

///////////////////////////////////////////
// read data from socket, puts into buffer
//
// Returns 0 on success, -1 on failure

int InputUdp::readPacket()

{

  //ipmet: inicializa variavel _EndVol
  _EndVol=false;

  PMU_auto_register("Reading UDP");
  _volHdrIsNew = false;
  _beamIsNew = false;
  
  // check for data, using select
  
  while (true) {

    int iret = SKU_read_select(_udpFd, 1000);

    if (iret == 1) {
      break;
    } // success

    if (iret == -2) {
      cerr << "ERROR - InputUdp::readPacket()" << endl;
      cerr << "  Cannot perform select on UDP socket" << endl;
      return -1;
    }

    // timeout, so register with procmap

    PMU_auto_register("Zzzzz");

  }

  struct sockaddr_in from_name;
  socklen_t fromLen = sizeof(from_name);
  int len = recvfrom(_udpFd, _buf, maxUdpBytes, 0,
		     (struct sockaddr *) &from_name, &fromLen);
  if (len < 0) {
    int errNum = errno;
    cerr << "ERROR - InputUdp::readPacket()" << endl;
    cerr << "  recvfrom: " << strerror(errNum) << endl;
    return -1;
  }

  if (len < 2) {
    cerr << "ERROR reading packet, too short, len: " << len << endl;
    return -1;
  }

  ui16 packetCode;
  memcpy(&packetCode, _buf, sizeof(packetCode));
  
  int pType = BE_to_ui16(packetCode) & UDP_TYPE;
  int pSize = BE_to_ui16(packetCode) & UDP_SIZE;
  
  if (pType == 0x0000) {

    memcpy(&_volHdr, _buf, sizeof(_volHdr));
    volHdrFromBe(_volHdr);

    _volHdrAvail = true;
    _volHdrIsNew = true;

    // byte width is constant, get from first field

    _byteWidth = Fields::getByteWidth(_volHdr.momData[0]);

    // compute number of packets to save

    _nPacketsPerField = 1 +
      (_volHdr.nBins - 1) / _volHdr.nBinsPacket;
    _nBeamPacketsSave = _volHdr.nMoments * _nPacketsPerField * _nBeamsSave;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Hdr packet, nbytes, pSize: "
	   << len << ", " << pSize << endl;
      printVolHdr(_volHdr, "  ", cerr);
      cerr << "    _byteWidth: " << _byteWidth << endl;
      cerr << "    _nPacketsPerField: " << _nPacketsPerField << endl;
      cerr << "    _nBeamsSave: " << _nBeamsSave << endl;
      cerr << "    _nBeamPacketsSave: " << _nBeamPacketsSave << endl;
    }
    
    return 0;

  } else if (pType == 0x0800) {
    
    if (!_volHdrAvail) {
      // need to wait for a volume header
      return 0;
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Beam packet, nbytes, pSize:"
	   << len << ", " << pSize << endl;
    }
    if (len < (int) sizeof(beam_header_t)) {
      cerr << "ERROR - InputUdp::readPacket" << endl;
      cerr << "  Packet too small for ray header" << endl;
      return 0;
    }
    
    // allocate space for this packet, copy it in, push it onto
    // the deque
    
    ui08 *rayPacket = new ui08[len];
    memcpy(rayPacket, _buf, len);
    beam_header_t *hdr = (beam_header_t* ) rayPacket;
    beamHdrFromBe(*hdr);
    int fieldId = _volHdr.momData[hdr->momDataIndex];
    int byteWidth = Fields::getByteWidth(fieldId);
    if (byteWidth == 2) {
      BE_from_array_16(hdr->momData, hdr->momDataSize);
    }
    _rayPackets.push_back(rayPacket);

    // if deque is large enough, assemble the next beam

    if ((int) _rayPackets.size() >= _nBeamPacketsSave) {
      if (_assembleBeam() == 0) {
	_beamIsNew = true;
      }
      return 0;
    }
    

     if (_params.debug >= Params::DEBUG_VERBOSE) {
       // printBeamHdr(*hdr, "  ", cerr);
     }

  } else if (pType == 0x1000) {
    
    //ipmet: EndVol eh testada na rotina _processUdp() do SigmetUdp2Dsr.cc 
    _EndVol = TRUE;

    if (_params.debug) {


      cerr << "----->>>>> End packet, nbytes, pSize:"
	   << len << ", " << pSize << endl;
    }
      
  } else {

    cerr << "Unknown packet type, pType, nbytes, pSize: "
	 << hex << pType << dec << ", " << len << ", " << pSize << endl;

  }

  return 0;

}

void InputUdp::volHdrFromBe(volume_header_t &hdr)

{

  hdr.packetCode = BE_to_ui16(hdr.packetCode);
  hdr.polType = BE_to_ui16(hdr.polType);

  BE_from_array_32(&hdr.latAsInt, sizeof(hdr.latAsInt));
  BE_from_array_32(&hdr.lonAsInt, sizeof(hdr.lonAsInt));

  hdr.sweepStartTime.sec = BE_to_ui32(hdr.sweepStartTime.sec);
  hdr.sweepStartTime.millsec = BE_to_ui16(hdr.sweepStartTime.millsec);
  hdr.sweepStartTime.year = BE_to_ui16(hdr.sweepStartTime.year);
  hdr.sweepStartTime.month = BE_to_ui16(hdr.sweepStartTime.month);
  hdr.sweepStartTime.day = BE_to_ui16(hdr.sweepStartTime.day);

  hdr.rangeFirstBinCm = BE_to_ui32(hdr.rangeFirstBinCm);
  hdr.binSpacingCm = BE_to_ui32(hdr.binSpacingCm);
  hdr.nBins = BE_to_ui32(hdr.nBins);

  hdr.nBinsPacket = BE_to_ui16(hdr.nBinsPacket);
  hdr.nMoments = BE_to_si16(hdr.nMoments);

  BE_from_array_16(hdr.momData, sizeof(hdr.momData));

  hdr.dualPrtScheme = BE_to_ui32(hdr.dualPrtScheme);
  hdr.prf = BE_to_si32(hdr.prf);
  hdr.wavelengthCmOver100 = BE_to_si32(hdr.wavelengthCmOver100);

}

void InputUdp::beamHdrFromBe(beam_header_t &hdr)

{

  hdr.packetCode = BE_to_ui16(hdr.packetCode);
  hdr.packetSeqNum = BE_to_ui16(hdr.packetSeqNum);

  BE_from_array_16(&hdr.startAz, sizeof(ui16));
  BE_from_array_16(&hdr.endAz, sizeof(ui16));
  BE_from_array_16(&hdr.endElev, sizeof(ui16));

  hdr.timeOffsetSecs = BE_to_ui16(hdr.timeOffsetSecs);
  hdr.momDataSize = BE_to_ui16(hdr.momDataSize);
  hdr.nBinsBeam = BE_to_ui16(hdr.nBinsBeam);

}

void InputUdp::printVolHdr(const volume_header_t &hdr,
			   const string &spacer,
			   ostream &out)
  
{

  out << spacer << "====== Vol header =====" << endl;

  out << spacer << "  hdr.packetCode: " << hdr.packetCode << endl;
  out << spacer << "  polType: " << hdr.polType << endl;

  out << spacer << "  siteNameShort: " << hdr.siteNameShort << endl;
  out << spacer << "  siteNameLong: " << hdr.siteNameLong << endl;

  out << spacer << "  latAsInt: " << (hdr.latAsInt / pow(2.0, 32.0)) * 360.0 << endl;
  out << spacer << "  lonAsInt: " << (hdr.lonAsInt / pow(2.0, 32.0)) * 360.0 << endl;

  out << spacer << "  sweepStartTime.sec: " 
      << hdr.sweepStartTime.sec << endl;
  out << spacer << "  sweepStartTime.millsec: " 
      << hdr.sweepStartTime.millsec << endl;
  out << spacer << "  sweepStartTime.year: " 
      << hdr.sweepStartTime.year << endl;
  out << spacer << "  sweepStartTime.month: " 
      << hdr.sweepStartTime.month << endl;
  out << spacer << "  sweepStartTime.day: " 
      << hdr.sweepStartTime.day << endl;

  out << spacer << "  rangeFirstBinCm: " << hdr.rangeFirstBinCm << endl;
  out << spacer << "  binSpacingCm: " << hdr.binSpacingCm << endl;
  out << spacer << "  nBins: " << hdr.nBins << endl;

  out << spacer << "  nBinsPacket: " << hdr.nBinsPacket << endl;
  out << spacer << "  nMoments: " << hdr.nMoments << endl;

  out << spacer << "  Moment names: " << endl;
  for (int i = 0; i < hdr.nMoments; i++) {
    out << spacer << "    " << Fields::getName(hdr.momData[i]) << endl;
  }

  out << spacer << "  dualPrtScheme: " << hdr.dualPrtScheme << endl;
  out << spacer << "  prf: " << hdr.prf << endl;
  out << spacer << "  wavelengthCmOver100: " << hdr.wavelengthCmOver100 << endl;

  out << spacer << "  taskName: " << hdr.taskName << endl;
  out << spacer << "  displayId: " << (int) hdr.displayId << endl;

}

void InputUdp::printBeamHdr(const beam_header_t &hdr,
			   const string &spacer,
			   ostream &out)
  
{

  out << spacer << "====== Beam header =====" << endl;

  out << spacer << "  hdr.packetCode: " << hdr.packetCode << endl;
  out << spacer << "  packetSeqNum: " << hdr.packetSeqNum << endl;

  out << spacer << "  siteNameShort: " << hdr.siteNameShort << endl;

  out << spacer << "  startAz: "
      << (hdr.startAz / 65536.0) * 360.0 << endl;
  out << spacer << "  endAz: "
      << (hdr.endAz / 65536.0) * 360.0 << endl;
  out << spacer << "  endElev: "
      << (hdr.endElev / 65536.0) * 360.0 << endl;

  out << spacer << "  timeOffsetSecs: " << hdr.timeOffsetSecs << endl;
  out << spacer << "  momDataIndex: " << (int) hdr.momDataIndex << endl;
  out << spacer << "  origin0RangeIndex: " << (int) hdr.origin0RangeIndex << endl;
  out << spacer << "  momDataSize: " << hdr.momDataSize << endl;
  out << spacer << "  nBinsBeam: " << hdr.nBinsBeam << endl;
  out << spacer << "  displayId: " << (int) hdr.displayId << endl;

  //   cerr << "Vals: ";
  //   ui16 *val = (ui16*) hdr.momData;
  //   for (int i = 0; i < hdr.nBinsBeam; i++, val++) {
  //     cerr << " " << *val << "/" << ((double) *val - 32767.0) / 100.0;
  //   }
  //   cerr << endl;

}

///////////////////////////////////////
// assemble a beam from saved packets

int InputUdp::_assembleBeam()

{

  // clear the beam

  _beam.clear();

  // set volume and ray headers

  _beam.setVolHdr(_volHdr);

  beam_header_t *beamHdr = (beam_header_t *) _rayPackets[0];
  _beam.setBeamHdr(*beamHdr);
  int seqNum = beamHdr->packetSeqNum;
  
  // go through the deque, adding all packets with this sequence num
  // until the beam is complete

  int dataIndex = 0;
  int rangeIndex = 0;
  
  while (!_beam.isComplete()) {
    
    deque<ui08 *>:: iterator ii;
    for (ii = _rayPackets.begin(); ii != _rayPackets.end(); ii++) {
      ui08 *packet = (ui08 *) *ii;
      beamHdr = (beam_header_t *) packet;
      if (beamHdr->packetSeqNum == seqNum &&
	  beamHdr->momDataIndex == dataIndex &&
	  beamHdr->origin0RangeIndex == rangeIndex) {
	_beam.addBeamPacket(*beamHdr);
	delete[] packet;
	_rayPackets.erase(ii);
	rangeIndex++;
	if (rangeIndex == _nPacketsPerField) {
	  dataIndex++;
	  rangeIndex = 0;
	}
	break;
      }
    } // ii
    
    if (ii == _rayPackets.end()) {
      // could not make a complete beam
      break;
    }
    
  } // while
  
  if (_beam.isComplete()) {

    return 0;

  } else {

    // clean up possible extra packets with this sequence number

    if (_params.debug) {
      cerr << "Cleaning up extra packets" << endl;
    }

    deque<ui08 *>:: iterator ii;
    for (ii = _rayPackets.begin(); ii != _rayPackets.end(); ii++) {
      ui08 *packet = (ui08 *) *ii;
      beamHdr = (beam_header_t *) packet;
      if (beamHdr->packetSeqNum == seqNum)  {
	delete[] packet;
	_rayPackets.erase(ii);
      }
    } // ii
    
    // beam incomplete

    return -1;

  }

}

