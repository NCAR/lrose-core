#include "CP2UdpSocket.h"
#include <string>
#include <iostream>
#include <QList>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

///////////////////////////////////////////////////////////
CP2UdpSocket::CP2UdpSocket(
			   std::string network, 
			   int port,
			   bool broadcast, 
			   int sndBufferSize, 
			   int rcvBufferSize):
  QUdpSocket(),
  _network(network),
  _port(port),
  _broadcast(broadcast),
  _sndBufferSize(0),
  _sndBufferSize_Req(sndBufferSize),
  _rcvBufferSize(0),
  _rcvBufferSize_Req(rcvBufferSize),
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
    if (found)
      {
	std::cout << "CP2UdpSocket::CP2UdpSocket - Found interface " <<
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

  std::cout << "broadcast is " << broadcast << std::endl;

  if (!broadcast) {
    _hostAddress = addrEntry.ip();
  } else {
    /// @todo Find out why QNetworkAddressEntry.broadcast() doesn't work,
    /// or how it is supposed to be used.
    _hostAddress = QHostAddress(addrEntry.ip().toIPv4Address() 
				| ~addrEntry.netmask().toIPv4Address());
    std::cout << "addrEntry.ip() is " << 
      addrEntry.ip().toString().toStdString() << 
      ",  addrEntry.netmask() is " <<
      addrEntry.netmask().toString().toStdString() << std::endl <<
      ", hostAddress is " << _hostAddress.toString().toStdString() << 
     " port=" << _port << std::endl;
      
  }

  // bind socket to port/network
  std::cout << "Binding to addrEntry.ip()  " << 
    addrEntry.ip().toString().toStdString() << 
    ",  addrEntry.netmask() " <<
    addrEntry.netmask().toString().toStdString() << std::endl <<
      "hostAddress is " << _hostAddress.toString().toStdString() << 
     " port=" << _port << std::endl;
  
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
    int result = setsockopt(socketDescriptor(), 
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

#ifdef WIN32
  int sz;
#else
  socklen_t sz;
#endif

  sz = sizeof(_sndBufferSize);

  result = getsockopt (socketDescriptor(),
		       SOL_SOCKET,
		       SO_SNDBUF,
		       (char *) &_sndBufferSize,
		       &sz);
  if (_sndBufferSize != _sndBufferSize_Req)
    std::cout << "CP2UdpSocket sndBufferSize requested=" << 
      _sndBufferSize_Req << " granted=" << 
      _sndBufferSize << std::endl;

  result = getsockopt (socketDescriptor(),
		       SOL_SOCKET,
		       SO_RCVBUF,
		       (char *) &_rcvBufferSize,
		       &sz);
  if (_rcvBufferSize != _rcvBufferSize_Req)
    std::cout << "CP2UdpSocket rcvBufferSize requested=" << 
      _rcvBufferSize_Req << " granted=" << 
      _rcvBufferSize << std::endl;

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


