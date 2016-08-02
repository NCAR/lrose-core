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
#include "CP2UdpSocket.hh"
#include <string>
#include <iostream>
#include <QtCore/QList>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkAddressEntry>
#include <sys/types.h>
#include <sys/socket.h>

///////////////////////////////////////////////////////////
CP2UdpSocket::CP2UdpSocket(std::string network, 
			   int port,
			   bool broadcast, 
			   int sndBufferSize, 
			   int rcvBufferSize,
			   bool debug):
  QUdpSocket(),
  _debug(debug),
  _broadcast(broadcast),
  _sndBufferSize(0),
  _sndBufferSize_Req(sndBufferSize),
  _rcvBufferSize(0),
  _rcvBufferSize_Req(rcvBufferSize),
  _network(network),
  _port(port),
  _ok(false)
{

  QList<QNetworkInterface> allIfaces = QNetworkInterface::allInterfaces();

  QNetworkAddressEntry addrEntry;
  int i;
  bool found = false;
  for (i = 0; i < allIfaces.size(); i++) {
    QNetworkInterface iface = allIfaces[i];
    QList<QNetworkAddressEntry> addrs = iface.addressEntries();
    for (int j = 0; j < addrs.size(); j++) {
      std::string thisIp = addrs[j].ip().toString().toStdString();
      if (thisIp.find(_network)!= std::string::npos) {
	addrEntry = addrs[j];
	found = true;
	break;
      }
    }
    if (found && _debug)
      {
	std::cerr << "CP2UdpSocket::CP2UdpSocket - Found interface " <<
	  allIfaces[i].name().toStdString() << " for network " <<
	  network << " " << std::endl;
	break;
      }
  }

  if (!found) {
    _errorMsg += "Unable to find interface for network ";
    _errorMsg += _network;
    _errorMsg += "\n";
    return;
  }

  if (_debug) {
    std::cerr << "broadcast is " << broadcast << std::endl;
  }

  if (!broadcast) {
    _hostAddress = addrEntry.ip();
  } else {
    /// @todo Find out why QNetworkAddressEntry.broadcast() doesn't work,
    /// or how it is supposed to be used.
    _hostAddress = QHostAddress(addrEntry.ip().toIPv4Address() 
				| ~addrEntry.netmask().toIPv4Address());
    if (_debug) {
      std::cerr << "addrEntry.ip() is " << 
	addrEntry.ip().toString().toStdString() << 
	",  addrEntry.netmask() is " <<
	addrEntry.netmask().toString().toStdString() << std::endl <<
	", hostAddress is " << _hostAddress.toString().toStdString() << 
	" port=" << _port << std::endl;
    }
      
  }

  // bind socket to port/network
  if (_debug) {
    std::cerr << "Binding to addrEntry.ip()  " << 
      addrEntry.ip().toString().toStdString() << 
      ",  addrEntry.netmask() " <<
      addrEntry.netmask().toString().toStdString() << std::endl <<
      "hostAddress is " << _hostAddress.toString().toStdString() << 
      " port=" << _port << std::endl;
  }
  
  if (!broadcast) {
    //     if (!bind(_hostAddress, _port, 
    // 	      QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
    if (!bind(_port, 
	      QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
      _errorMsg += "Unable to bind datagram port ";
      _errorMsg += _hostAddress.toString().toStdString();
      _errorMsg += ":";
      _errorMsg += QString("%1").arg(_port).toStdString();
      _errorMsg += "\n";
      _errorMsg += this->errorString().toStdString();
      _errorMsg += "\n";
      return;
    }
  } else {
    if (!bind(_port, 
	      QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
      _errorMsg += "Unable to bind datagram port ";
      _errorMsg += _hostAddress.toString().toStdString();
      _errorMsg += ":";
      _errorMsg += QString("%1").arg(_port).toStdString();
      _errorMsg += "\n";
      _errorMsg += this->errorString().toStdString();
      _errorMsg += "\n";
      return;
    }
  }

  // set SO_REUSEADDR. I'm not sure why it is both here and in
  // the bind, but let's not make troulbe by taking it out.
  int optval = 1;
  int result = setsockopt(socketDescriptor(), 
			  SOL_SOCKET, 
			  SO_REUSEADDR, 
			  (const char*)&optval, 
			  sizeof(optval)); 

  if (broadcast) {
    // set SO_BROADCAST. Even though we are using the broadcast address, under Linux
    // it seems that we need to set this also.
    int optval = 1;
    result = setsockopt(socketDescriptor(), 
			SOL_SOCKET, 
			SO_BROADCAST, 
			(const char*)&optval, 
			sizeof(optval)); 
  }

  bool sockError = false;

  if (_sndBufferSize_Req) {
    // set the system send buffer size
    int sockbufsize = _sndBufferSize_Req;
    result = setsockopt (socketDescriptor(),
			 SOL_SOCKET,
			 SO_SNDBUF,
			 (char *) &sockbufsize,
			 sizeof sockbufsize);
    if (result) {
      _errorMsg += "Set send buffer size for socket failed\n";
      perror("Setting socket send buffer size ");
      sockError = true;
    }
  }

  if (_rcvBufferSize_Req) {
    // set the system receive buffer size
    int sockbufsize = _rcvBufferSize_Req;
    result = setsockopt (socketDescriptor(),
			 SOL_SOCKET,
			 SO_RCVBUF,
			 (char *) &sockbufsize,
			 sizeof sockbufsize);
    if (result) {
      _errorMsg += "Set receive buffer size for socket failed\n";
      perror("Setting socket send buffer size ");
      sockError = true;
    }
  }

  if (sockError) {
    return;
  }

  // read back the actual granted socket buffer sizes

  socklen_t sz;

  sz = sizeof(_sndBufferSize);

  result = getsockopt (socketDescriptor(),
		       SOL_SOCKET,
		       SO_SNDBUF,
		       (char *) &_sndBufferSize,
		       &sz);
  if (_sndBufferSize != _sndBufferSize_Req) {
    if (_debug) {
      std::cerr << "CP2UdpSocket sndBufferSize requested=" << 
	_sndBufferSize_Req << " granted=" << 
	_sndBufferSize << std::endl;
    }
  }

  result = getsockopt (socketDescriptor(),
		       SOL_SOCKET,
		       SO_RCVBUF,
		       (char *) &_rcvBufferSize,
		       &sz);
  if (_rcvBufferSize != _rcvBufferSize_Req) {
    if (_debug) {
      std::cerr << "CP2UdpSocket rcvBufferSize requested=" << 
	_rcvBufferSize_Req << " granted=" << 
	_rcvBufferSize << std::endl;
    }
  }
  _ok = true;

}

///////////////////////////////////////////////////////////
int
CP2UdpSocket::writeDatagram(const char* data, int size) 
{
  return QUdpSocket::writeDatagram(data, size, _hostAddress, _port);
}

///////////////////////////////////////////////////////////
CP2UdpSocket::~CP2UdpSocket()
{
}

///////////////////////////////////////////////////////////
bool
CP2UdpSocket::ok()
{
  return _ok;
}
///////////////////////////////////////////////////////////
std::string
CP2UdpSocket::errorMsg() {
  return _errorMsg;
}

///////////////////////////////////////////////////////////
std::string
CP2UdpSocket::toString()
{
  return _hostAddress.toString().toAscii().constData();
}

///////////////////////////////////////////////////////////
QHostAddress
CP2UdpSocket::hostAddress()
{
  return _hostAddress;
}


