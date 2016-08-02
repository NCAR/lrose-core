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

#ifndef DS_URL_INCLUDED
#define DS_URL_INCLUDED

#include <toolsa/URL.hh>
using namespace std;

class DsURL : public URL {
  
public:

  // default constructor
  
  DsURL();
  
  // constructor with URL supplied

  DsURL(const string &urlStr);

  // assignment with URL supplied

  inline void operator=(const string &urlStr) {
    setURLStr( urlStr );
  }

  // virtual destructor

  virtual ~DsURL();

  // Set the URL string, and decode it into its component parts.
  // sets valid flag appropriately
  // returns 0 on success, -1 on failure
  
  virtual int setURLStr(const string &urlStr);
  
  // Get the string representation of the URL.
  // Computes string from previously stored components
  // Returns:
  //   On success: a string representing the URL
  //      sets _isValid to true
  //   On error:
  //      An empty string if the URL has no protocol,
  //      or if the subclass cannot encode the protocol-specific part.
  //      sets _isValid to false
  
  virtual string getURLStr() const;
  
  // Check the URL for consistency.
  // If URL member is not valid, try to create a valid
  // one from the stored properties.
  // Sets the _isValid flag appropriately.

  virtual bool checkValid();
  
  // Check a URL string for consistency.
  // Static method. Can be called without object.

  static bool checkValid(string url);
  
  // print

  virtual void print(ostream &out) const;

  // Methods to set individual members of the class.
  // 
  // Using these methods sets the validity of the URL to false.
  //   This is not reset until the string representation is retrieved
  //   via getURLStr().

  virtual inline void setProtocol(const string &protocol) {
    _isValid = false;
    _protocol = protocol;
    if (_protocol[_protocol.size() - 1] == 'p') {
      _datatype = _protocol.substr(0, _protocol.size()-1);
    }
  }
  
  inline void setTranslator(const string &translator) {
    _isValid = false;
    _translator = translator; 
  }

  inline void setParamFile(const string &paramFile) {
    _isValid = false;
    _paramFile = paramFile;   
  }

  inline void setDataType(const string &dataType) {
    _isValid = false;
    _datatype = dataType;
    _protocol = _datatype + "p";
  }

  // Check whether the URL specifies forwarding through a proxy
  // or tunnel.
  //
  // Returns true if forwarding is specified, false otherwise.
  
  bool forwardingActive() const;

  // Prepare for forwarding through a proxy or tunnel.
  //
  // Note: The server protocol allows tunneling (transmission) through
  // A Http server which has RAP's DsServerTunnel (a mod_perl module)
  // installed. Additionally, it will optionally forward the server
  // protocols through an outgoing Http proxy server. You must have
  // a DsServerTunnel in place and use the tunnel_url=URL argument
  // when using the proxy forwarding mechanism.
  // Append the arguments:  ?tunnel_url=http://host/TunnelLocation
  // and then optionally: &proxy_url=http://host:port         
  //
  // Construct the Http header relevant to the URL.
  //
  // 'calling_info' is inserted into the http_header as logging
  // information. Should not have line-feeds or carriage returns.
  //
  // 'content_length' is length of message to follow header.
  //
  // server_port specifies the port for the DsServer to be contacted
  // via the tunnel. If the port is not specified (i.e. defaults to -1)
  // the port is obtained from the URL.
  //
  // Returns 0 on success, -1 on failure
  // On error, use getErrString() to access error string.
  //
  // After calling, you can use the following to check:
  //
  //   useForwarding() - use forwarding or not?
  //   useProxy()      - forwarding via proxy?
  //   useTunnel()     - forwarding via tunnel?
  //   useHttp()       - use http protocol
  //   getForwardingPort() - port for forwarding address
  //   getForwardingHost() - host name for forwarding address
  //   getHttpHeader() - http header for forwarding

  int prepareForwarding(const string &calling_info,
			int content_length,
			int server_port = -1) const;

  /////////////////////////////////////////////////////////////
  // getting info on forwarding after calling checkForwarding()

  bool useForwarding() const {
    return (_useProxy || _useTunnel || _useHttp); 
  }
  bool useProxy() const { return _useProxy; }
  bool useTunnel() const { return _useTunnel; }
  bool useHttp() const { return _useHttp; }
  int getForwardingPort() const { return _forwardingPort; }
  const string &getForwardingHost() const { return _forwardingHost; }
  const string &getHttpHeader() const { return _httpHeader; }

  // Get members

  const string &getDatatype() const { return _datatype; }
  const string &getTranslator() const { return _translator; }
  const string &getParamFile() const  { return _paramFile;  }

  // get the URL string with forwarding info removed
  
  string getURLStrNoFwd() const;

  // set the members for local access only

  void setForLocalAccessOnly();

  // Value to indicate that default port should be used

  static const int DefaultPortIndicator = -999;
  
protected:

  string _datatype;   // The protocol with 'p' stripped off the end.
  string _translator;
  string _paramFile;

  mutable bool _useProxy;
  mutable bool _useTunnel;
  mutable bool _useHttp;
  mutable int _forwardingPort;
  mutable string _forwardingHost;
  mutable string _httpHeader;

private:
  
  virtual void _init();
  int _decodeHttpUrl(const string &urlStr);
  void _loadHttpHeader(const string &userInfo,
                       int contentLength) const;

};

#endif
