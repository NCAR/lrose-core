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
#ifndef CP2_UDP_SOCKET_HH
#define CP2_UDP_SOCKET_HH
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostAddress>
#include <string>

/**
   @brief Provide UDP socket services.

   CP2QUdpSocket extends the QUdpSocket class to add some default
   behavior, such as increasing buffer sizes, locating broadcast
   interfaces, and providing text translations of IP information.

**/
class CP2UdpSocket: public QUdpSocket {
public:
  /// @param network Set this to the network IP of the interface
  /// that the socket will be bound to. The list of interfaces on the 
  /// system will be scanned for a match on this network. It is acceptable
  /// to only specify the network portion, such as 192.168.1, since
  /// on DHCP served systems, we typically won't know the full address.
  /// @param port The network port.
  /// @param broadcast Set true if the socket will be broadcasting
  /// @param sndBufferSize Set to non-zero to request the send buffer size
  /// to be set.
  /// @param rcvBroadcastSize Set to non-zero to request the receive buffer size 
  /// be set.
  CP2UdpSocket(std::string network, 
	       int port,
	       bool broadcast, 
	       int sndBufferSize, 
	       int rcvBufferSize,
	       bool debug);

  virtual ~CP2UdpSocket();

  /// Send the data
  /// @param data The data
  /// @param size Size in bytes
  /// @returns The number of bytes written, or -1 on error.
  int writeDatagram(const char* data, int size);
  /// @return The IP address for this socket.
  QHostAddress hostAddress();
  /// @return The IP number as a string
  std::string toString();
  /// @return true if the socket could be configured correctly, false otherwise
  bool ok();
  /// @return The most recent error message
  std::string errorMsg();
  int rcvBufferSize()
  { return _rcvBufferSize; };
  int sndBufferSize()
  { return _sndBufferSize; };

protected:
  bool _debug;
  /// set true if the socket will be broadcasting
  bool _broadcast;
  /// Set to non-zero to request the send buffer size
  /// to be set.
  int _sndBufferSize,    // actual send buffer size granted
    _sndBufferSize_Req;  // requested send buffer size
  /// Set to non-zero to request the receive buffer size 
  /// be set.
  int _rcvBufferSize,    // actual rcv buffer size granted
    _rcvBufferSize_Req;  // requested rcv buffer size
  /// The host address
  QHostAddress _hostAddress;
  /// The network interface
  std::string _network;
  /// The network port
  int _port;
  /// set true if socket is usable.
  bool _ok;
  /// The last error message.
  std::string _errorMsg;
};

#endif
