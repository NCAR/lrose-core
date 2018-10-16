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

#include <string>
#include <cstdio>
#include <cstdlib>
#include <toolsa/TaStr.hh>
#include <didss/DsURL.hh>
#include <toolsa/HttpURL.hh>
#include <toolsa/port.h>
using namespace std;

//////////////
// constructor

DsURL::DsURL() : URL()
{
  setURLStr("nonep:://no-host::no-file");
}

DsURL::DsURL(const string &urlStr) : URL()
{
  setURLStr(urlStr);
}

/////////////////////
// virtual destructor

DsURL::~DsURL()
{

}

/////////////////////
// initialize

void DsURL::_init()
{

  URL::_init();
  _datatype.clear();
  _translator.clear();
  _paramFile.clear();
  _useProxy = false;
  _useTunnel = false;
  _useHttp = false;
  _forwardingPort = -1;
  _forwardingHost.clear();
  _httpHeader.clear();
  
}

//////////////////////
// set the URL string

int DsURL::setURLStr(const string &urlStr)
{

  _init(); // Base class init
  _urlStr = urlStr;

  // check for http - use special logic

  if (urlStr.find("http://") == 0) {
    return _decodeHttpUrl(urlStr);
  }
  
  // is this a simple URL? - i.e. all file, nothing else
  // if there are no colon delimiters, and no '?',
  // then the entire URL is interpreted as the file part
  
  if (_urlStr.find(Colon) == string::npos) {
    size_t argStart = _urlStr.find(QuestionMark);
    if (argStart == string::npos) {
      _host = "localhost";
      _file = _urlStr;
    } else {
      _host = "localhost";
      _file.assign(_urlStr, 0, argStart);
      string args;
      args.assign(_urlStr, argStart + QuestionMark.size(), string::npos);
      _loadArgPairs(args);
    }
    _isValid = true;
    return 0;
  }
  
  // get protocol
  size_t currPos = 0;
  size_t nextPos = _urlStr.find(Colon);
  if (nextPos == string::npos) {
    // no protocol found
    _errString  = "DsURL string has no protocol.";
    _errString += _urlStr;
    _protocol = "no_protocol";
    return -1;
  }
  _protocol.assign(_urlStr, 0, nextPos - currPos);
  currPos = nextPos + Colon.size();

  // Check that the protocol ends in a 'p'.
  if (_protocol.at(_protocol.size() - 1) != 'p') {
    _errString  = "URL string has illegal protocol. ";
    _errString += "Protocol must end in a \'p\'.";
    _errString += _urlStr;
    _protocol = "no_protocol";
    return -1;
  }
  
  // set data type
  // Strip the 'p' off the end of the protocol for the datatype.

  _datatype.assign(_protocol, 0, _protocol.size() - 1);
  
  // translator
  
  nextPos = _urlStr.find(Colon, currPos);
  if (nextPos == string::npos) {
    _errString  = "Could not find ':' for end of translator: ";
    _errString += _urlStr;
    _translator = "no-translator";
    return -1;
  }
  _translator.assign(_urlStr, currPos, nextPos - currPos);
  currPos = nextPos + Colon.size();
  
  // paramfile
  
  nextPos = _urlStr.find(SlashSlash, currPos);
  if (nextPos == string::npos) {
    _errString  = "Could not find '//' for end of paramFile: ";
    _errString += _urlStr;
    _paramFile = "no-paramfile";
    return -1;
  }
  _paramFile.assign(_urlStr, currPos, nextPos - currPos);
  currPos = nextPos + SlashSlash.size();

  // host

  nextPos = _urlStr.find(Colon, currPos);
  if (nextPos == string::npos) {
    _errString  = "Could not find ':' for end of host: ";
    _errString += _urlStr;
    _paramFile = "no-host";
    return -1;
  }
  _host.assign(_urlStr, currPos, nextPos - currPos);
  if (_host.size() == 0) {
    _host = "localhost";
  }
  currPos = nextPos + Colon.size();

  // port

  nextPos = _urlStr.find(Colon, currPos);
  if (nextPos == string::npos) {
    _errString  = "Could not find delimiter for end of port: ";
    _errString += _urlStr;
    _port = -8888;
    return -1;
  }

  string portString(_urlStr, currPos, nextPos - currPos);
  if (portString.size() > 0) {
    if (portString == "default" ) {
      _port = DefaultPortIndicator;
    } else {
      if (sscanf(portString.c_str(), "%d", &_port) != 1) {
        _errString = "Bad port: ";
        _errString = portString;
        _errString = " url: ";
        _errString += _urlStr;
        _port = -1;
        return -1;
      }
    }
  }
  currPos = nextPos + Colon.size();

  // file

  nextPos = _urlStr.find(QuestionMark, currPos);
  bool hasArgs = true;
  if (nextPos == string::npos) {
    hasArgs = false;
  }
  _file.assign(_urlStr, currPos, nextPos - currPos);
  currPos = nextPos + QuestionMark.size();

  // args
  
  if (hasArgs) {

    string args;
    if (currPos >= _urlStr.size()) {
      // Args delimiter present, but no args
      args = "";
    } else {
      // The rest of the string is args.
      args.assign(_urlStr, currPos, string::npos);
      // look for name=val pairs in the args
      _loadArgPairs(args);
    }

    for (size_t ii = 0; ii < _argPairs.size(); ii++) {
      if (_argPairs[ii].first == "proxy_url") {
        _useProxy = true;
      }
      if (_argPairs[ii].first == "tunnel_url") {
        _useTunnel = true;
      }
      if (_argPairs[ii].first == "forward_url") {
        DsURL fwd(_argPairs[ii].second);
        _forwardingPort = fwd.getPort();
        _forwardingHost = fwd.getHost();
      }
      if (_argPairs[ii].first == "use_http") {
        _useHttp = true;
      }
    }

  } // if (hasArgs)

  _isValid = true;
  return 0;

}

/////////////////////////////////////////////////////
// get URL string
//
// Computes string from previously stored components

string DsURL::getURLStr() const
{

  // if already valid, return URL that was used to
  // set the object state

  if (_isValid) {
    return _urlStr;
  }

  // if not valid, then individual properties have been set
  // and we need to construct the URL
  
  _urlStr = _protocol;

  _urlStr += Colon;
  _urlStr += _translator;

  _urlStr += Colon;
  _urlStr += _paramFile;

  _urlStr += SlashSlash;
  _urlStr += _host;

  _urlStr += Colon;
  char buf[20];
  sprintf(buf, "%d", _port);
  _urlStr += buf;

  _urlStr += Colon;
  _urlStr += _file;

  if (_args.size() > 0) {
    _urlStr += QuestionMark;
    _urlStr += _args;
  }
  
  // Update the validity flag.
  _isValid = true;
  
  return _urlStr;
}

///////////////////////////////////////////////////////
// Check the URL for consistency.
// If URL member is not valid, try to create a valid
// one from the stored properties.
// Sets the _isValid flag appropriately.

bool DsURL::checkValid()
{

  // already valid?
  
  if (_isValid) {
    return true;
  }

  // construct the URL string from individual properties
  
  getURLStr();
  if (_isValid) {
    return true;
  } else {
    return false;
  }
}

///////////////////////////////////////////////
// Check a URL string for consistency.
// Static method. Can be called without object.

bool DsURL::checkValid(string url)
{
  DsURL test(url);
  if (test.isValid()) {
    return true;
  } else {
    return false;
  }
}

/////////////////////////////////////////////////////////////////////////
// set the members for local access only
// no params or translation.

void DsURL::setForLocalAccessOnly()
{
  _translator.clear();
  _paramFile.clear();
  _host = "localhost";
  _port = -1;
}

/////////////////////////////////////////////////////////////////////////
// print

void DsURL::print(ostream &out) const
{
  
  out << "  urlStr: " << _urlStr << endl;
  out << "  isValid: " << _isValid << endl;
  out << "  protocol: " << _protocol << endl;
  out << "  datatype: " << _datatype << endl;
  out << "  translator: " << _translator << endl;
  out << "  paramFile: " << _paramFile << endl;
  out << "  host: " << _host << endl;
  out << "  port: " << _port << endl;
  out << "  file: " << _file << endl;
  out << "  args: " << _args << endl;

  out << "  nArgs: " << _argList.size() << endl;
  if (_argList.size() > 0) {
    out << "  argList: " << endl;
    for (size_t ii = 0; ii < _argPairs.size(); ii++) {
      out << "    " << _argList[ii] << endl;
    }
  }

  out << "  nArgPairs: " << _argPairs.size() << endl;
  if (_argPairs.size() > 0) {
    out << "  argPairs: " << endl;
    for (size_t ii = 0; ii < _argPairs.size(); ii++) {
      out << "    " << _argPairs[ii].first
	  << " = " << _argPairs[ii].second << endl;
    }
  }

  out << "  computed urlStr: " << getURLStr() << endl;
  if (getURLStr() != _urlStr) {
    out << "  NOTE: computed url differs from original url" << endl;
  }

  out << "  useProxy: " << _useProxy << endl;
  out << "  useTunnel: " << _useTunnel << endl;
  out << "  useHttp: " << _useHttp << endl;
  out << "  forwardingPort: " << _forwardingPort << endl;
  out << "  forwardingHost: " << _forwardingHost << endl;
  out << "  httpHeader: " << endl;
  out << "-------------------------------" << endl;
  out << _httpHeader;
  out << "-------------------------------" << endl;

}

//////////////////////////////////////////////////////////////////////
// Check whether the URL specifies forwarding through a proxy
// or tunnel.
//
// Returns true if forwarding is specified, false otherwise.

bool DsURL::forwardingActive() const

{

  for (size_t ii = 0; ii < _argPairs.size(); ii++) {
    if (_argPairs[ii].first == "proxy_url") {
      return true;
    }
    if (_argPairs[ii].first == "tunnel_url") {
      return true;
    }
    if (_argPairs[ii].first == "forward_url") {
      return true;
    }
    if (_argPairs[ii].first == "use_http") {
      return true;
    }
  }

  return false;

}

//////////////////////////////////////////////////////////////////////
// Prepare for forwarding through a proxy or tunnel.
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
//   useProxy()      -  forwarding via proxy?
//   useTunnel()     - forwarding via tunnel?
//   useHttp()       - add http headers
//   getForwardingPort() - port for forwarding address
//   getForwardingHost() - host name for forwarding address
//   getHttpHeader() - http header for forwarding

int DsURL::prepareForwarding(const string &calling_info,
			     int content_length,
			     int server_port /* = -1*/ ) const

{

  _errString = "ERROR - DsURL::prepareForwarding\n";
  TaStr::AddStr(_errString, "  Called by: ", calling_info);
  
  char str[1024];

  _useProxy = false;
  _useTunnel = false;
  // _useHttp = false;
  _httpHeader = "";
  _forwardingPort = 0;
  _forwardingHost = "";

  // check if we even need to be here

  if (!_useHttp && !forwardingActive()) {
    return 0;
  }

  // search for proxy url and tunnel url in args

  string proxyUrl, tunnelUrl, useHttp;

  for (size_t ii = 0; ii < _argPairs.size(); ii++) {
    if (_argPairs[ii].first == "proxy_url") {
      proxyUrl = _argPairs[ii].second;
    }
    if (_argPairs[ii].first == "tunnel_url") {
      tunnelUrl = _argPairs[ii].second;
    }
    if (_argPairs[ii].first == "forward_url") {
      tunnelUrl = _argPairs[ii].second;
    }
    if (_argPairs[ii].first == "use_http") {
      useHttp = _argPairs[ii].second;
    }
  }

  // if proxy is set, must also have http url
  
  if (proxyUrl.size() > 0) {
    if (tunnelUrl.size() == 0) {
      TaStr::AddStr(_errString, "  ",
		    "  If proxy URL specified, must have tunnel URL also.");
      TaStr::AddStr(_errString, "  url: ", _urlStr);
      TaStr::AddStr(_errString, "  args: ", _args);
      return -1;
    }
    _useProxy = true;
  } else if (tunnelUrl.size() > 0) {
    _useTunnel = true;
  } else if (useHttp.size() > 0) {
    for (size_t ii = 0; ii < useHttp.size(); ii++) {
      useHttp[ii] = tolower(useHttp[ii]);
    }
    if (useHttp == "true") {
      _useHttp = true;
    }
  }

  // if proxy, construct header for contacting proxy

  if (_useProxy) {

    // POST line

    snprintf(str,1024, "POST %s HTTP/1.0\r\n", tunnelUrl.c_str());
    _httpHeader += str;

    // referer

    const char *user = getenv("USER");
    if (user == NULL) {
      user = "unknown";
    }

    // Limit the characters copied from user to str to avoid 
    // unbounded array copy
    
    snprintf(str,1024, "Referer: user %s on host %s, info %s\r\n",
	    user, PORThostname(), calling_info.c_str());
    _httpHeader += str;

    // keep-alive

    _httpHeader += "Proxy-Connection: Keep-Alive\r\n";

    // User-Agent
    
    if (server_port > 0) {
      sprintf(str, "User-Agent: DsServer 1.0: %s:%d\r\n",
	      _host.c_str(), server_port);
    } else {
      sprintf(str, "User-Agent: DsServer 1.0: %s:%d\r\n",
	      _host.c_str(), _port);
    }
    _httpHeader += str;

    sprintf(str, "Host: %s:%d\r\n", _host.c_str(), _port);

    // host and port for proxy to contact

    HttpURL t_url(tunnelUrl);
    int port = t_url.getPort();
    const char *hostname = t_url.getHost().c_str();
    sprintf(str, "Host: %s:%d\r\n", hostname, port);
    _httpHeader += str;

    if (port <= 0) {
      TaStr::AddStr(_errString, "  ", "Tunnel port not set.");
      TaStr::AddStr(_errString, "  url: ", _urlStr);
      return -1;
    }
    
    if (t_url.getHost().size() == 0) {
      TaStr::AddStr(_errString, "  ", "Tunnel host not set.");
      TaStr::AddStr(_errString, "  url: ", _urlStr);
      return -1;
    }
    
    // host and port for calling routine to contact

    HttpURL p_url(proxyUrl);
    _forwardingPort = p_url.getPort();
    _forwardingHost = p_url.getHost();

    if (_forwardingPort <= 0) {
      TaStr::AddStr(_errString, "  ", "Proxy port not set.");
      TaStr::AddStr(_errString, "  url: ", _urlStr);
      return -1;
    }

    if (_forwardingHost.size() == 0) {
      TaStr::AddStr(_errString, "  ", "Proxy host not set.");
      TaStr::AddStr(_errString, "  url: ", _urlStr);
      return -1;
    }

    // length of content message to follow
    // We need to add 20 bytes for the socket header, which
    // is 8 magic-cookie bytes + 12 SKU_header_t bytes
    
    sprintf(str, "Content-length: %d\r\n\r\n", content_length + 20);
    _httpHeader += str;

    return 0;

  } // if (_useProxy)
  
  // if tunnel, construct header for contacting tunnel
  
  if (_useTunnel) {

    // tunnel script name, port and host to contact
    
    HttpURL t_url(tunnelUrl);
    _forwardingPort = t_url.getPort();
    _forwardingHost = t_url.getHost();
    const string &script = t_url.getFile();

    if (script.size() == 0) {
      TaStr::AddStr(_errString, "  ", "Tunnel script name not set.");
      TaStr::AddStr(_errString, "  url: ", _urlStr);
      return -1;
    }
    
    if (_forwardingPort <= 0) {
      TaStr::AddStr(_errString, "  ", "Tunnel port not set.");
      TaStr::AddStr(_errString, "  url: ", _urlStr);
      return -1;
    }

    if (_forwardingHost.size() == 0) {
      TaStr::AddStr(_errString, "  ", "Tunnel host not set.");
      TaStr::AddStr(_errString, "  url: ", _urlStr);
      return -1;
    }

    // POST line

    sprintf(str, "POST /%s HTTP/1.0\r\n", script.c_str());
    _httpHeader += str;

    // referer

    const char *user = getenv("USER");
    if (user == NULL) {
      user = "unknown";
    }

    // Limit the characters copied from user to str to avoid 
    // unbounded array copy

    snprintf(str,1024, "Referer: user %s on host %s, info %s\r\n",
	    user, PORThostname(), calling_info.c_str());
    _httpHeader += str;
    
    // keep-alive

    _httpHeader += "Connection: Keep-Alive\r\n";

    // User-Agent

    if (server_port > 0) {
      sprintf(str, "User-Agent: DsServer 1.0: %s:%d\r\n",
	      _host.c_str(), server_port);
    } else {
      sprintf(str, "User-Agent: DsServer 1.0: %s:%d\r\n",
	      _host.c_str(), _port);
    }
    _httpHeader += str;

    // host and port for tunnel to contact
    
    sprintf(str, "Host: %s:%d\r\n", _host.c_str(), _port);
    _httpHeader += str;

    // length of content message to follow
    // We need to add 20 bytes for the socket header, which
    // is 8 magic-cookie bytes + 12 SKU_header_t bytes

    sprintf(str, "Content-length: %d\r\n\r\n", content_length + 20);
    _httpHeader += str;

    return 0;

  } // if (_useTunnel)

  if (_useHttp) {

    // just add http header so the packet can be handled
    // by an http-style server
    
    _forwardingPort = _port;
    _forwardingHost = _host;

    _loadHttpHeader(calling_info, content_length);

    return 0;

  } // if (_useHttp)

  return 0;

}

///////////////////////////////////////////////////
// get the URL string with forwarding info removed

string DsURL::getURLStrNoFwd() const

{

  string urlStr = getURLStr();
  if (urlStr.find("http://") == 0) {
    return urlStr;
  }

  // strip off forwarding
  
  size_t tunnel_start = urlStr.find("tunnel_url", 0);
  if (tunnel_start == string::npos) {
    tunnel_start = urlStr.find("forward_url", 0);
  }
  if (tunnel_start == string::npos) {
    tunnel_start = urlStr.find("use_http", 0);
  }
  
  if (tunnel_start != string::npos) {

    // strip off tunnel

    string withoutTunnel = urlStr.substr(0, tunnel_start);
    size_t tunnel_end = urlStr.find("&", tunnel_start);
    if (tunnel_end != string::npos) {
      withoutTunnel += urlStr.substr(tunnel_end + 1, string::npos);
    }

    // strip off proxy
    
    size_t proxy_start = withoutTunnel.find("proxy_url", 0);
    
    if (proxy_start == string::npos) {
      
      // no proxy
      
      // check for trailing ?
      
      if (withoutTunnel[withoutTunnel.size()-1] != '?') {
	return withoutTunnel;
      } else {
	// strip off ?
	return withoutTunnel.substr(0, withoutTunnel.size()-1);
      }

    } else {
      
      string withoutProxy;
      withoutProxy = withoutTunnel.substr(0, proxy_start);
      size_t proxy_end = withoutTunnel.find("&", proxy_start);
      if (proxy_end != string::npos) {
	withoutProxy += withoutTunnel.substr(proxy_end + 1, string::npos);
      }
      
      // check for trailing ?

      if (withoutProxy[withoutProxy.size()-1] != '?') {
	return withoutProxy;
      } else {
	// strip off ?
	return withoutProxy.substr(0, withoutProxy.size()-1);
      }

    } // if (proxy_start ...

  } // if (tunnel_start ...

  // no tunnel present

  return urlStr;

}

/////////////////////////////////////////////////////
// decode HTTP URL

int DsURL::_decodeHttpUrl(const string &urlStr)
  
{
  
  // check if this is an HTTP URL
  HttpURL httpUrl(urlStr);
  if (!httpUrl.isValid()) {
    // not a valid http URL either
    _isValid = false;
    return -1;
  }

  // set state from Http URL

  _protocol = httpUrl.getProtocol();
  _host = httpUrl.getHost();
  _port = httpUrl.getPort();
  _file = httpUrl.getFile();
  _args = httpUrl.getArgs();
  _argPairs = httpUrl.getArgPairs();
  _useHttp = true;

  // use args as appropriate

  vector<pair<string, string> > unusedPairs;
  for (size_t ii = 0; ii < _argPairs.size(); ii++) {
    
    // set state from args
    pair<string, string> pr = _argPairs[ii];
    if (pr.first == "datatype") {
      _datatype = pr.second;
      _protocol = _datatype + "p";
    } else if (pr.first == "protocol") {
      _protocol = pr.second;
      // Check that the protocol ends in a 'p'.
      if (_protocol[_protocol.size() - 1] == 'p') {
        _datatype = _protocol.substr(0, _protocol.size()-1);
      } else {
        _datatype = "no-datatype";
      }
    } else if (pr.first == "translator") {
      _translator = pr.second;
    } else if (pr.first == "params") {
      _paramFile = pr.second;
    } else {
      // not used, save
      unusedPairs.push_back(pr);
    }
  }

  // save unused args

  _argPairs = unusedPairs;
  _args.clear();
  for (size_t ii = 0; ii < _argPairs.size(); ii++) {
    _args += _argPairs[ii].first;
    _args += Equals;
    _args += _argPairs[ii].second;
    if (ii != _argPairs.size() - 1) {
      _args += Ampersand;
    }
  }

  _isValid = true;
  return 0;

}

/////////////////////////////////////////////////////
// load http header

void DsURL::_loadHttpHeader(const string &userInfo,
                            int contentLength) const
  
{
  
  char str[1024];

  // POST line
  
  sprintf(str, "POST /%s HTTP/1.0\r\n", _urlStr.c_str());
  _httpHeader += str;
  
  // referer
  
  const char *user = getenv("USER");
  if (user == NULL) {
    user = "unknown";
  }

  // Limit the characters copied from user to str to avoid 
  // unbounded array copy

  snprintf(str,1024, "Referer: user %s on host %s, info %s\r\n",
          user, PORThostname(), userInfo.c_str());
  _httpHeader += str;
  
  // keep-alive
  
  _httpHeader += "Connection: Keep-Alive\r\n";

  // User-Agent
  
  sprintf(str, "User-Agent: DsServer 1.0: %s:%d\r\n",
          _host.c_str(), _port);
  _httpHeader += str;

  // host and port for tunnel to contact
  
  sprintf(str, "Host: %s:%d\r\n", _host.c_str(), _port);
  _httpHeader += str;
  
  // length of content message to follow
  // We need to add 20 bytes for the socket header, which
  // is 8 magic-cookie bytes + 12 SKU_header_t bytes
  
  sprintf(str, "Content-length: %d\r\n\r\n", contentLength + 20);
  _httpHeader += str;

}


