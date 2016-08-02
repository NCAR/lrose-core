/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// Rvp8Driver.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// The Rvp8Driver controls the RVP8. It starts up with an XML
// file for initial configuration. It then listens for incoming
// connections, receives XML instructions and changes the RVP8
// state accordingly.
//
///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include "Rvp8Driver.hh"
#include <rvp8_rap/ServerSocket.hh>
#include <rvp8_rap/TaXml.hh>
#include <rvp8_rap/TaStr.hh>
#include <rvp8_rap/udatetime.h>
#include <rvp8_rap/uusleep.h>
#include <rvp8_rap/pmu.h>
#include "Args.hh"
#include "DspDriver.hh"
using namespace std;

// Constructor

Rvp8Driver::Rvp8Driver(int argc, char **argv)

{

  OK = true;
  _dspDriver = NULL;
  _tsStatus = NULL;

  // set programe name
  
  _progName = "Rvp8Driver";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }

  // set up commands object

  _commands.setDebug(_args.debug);
  _commands.setVerbose(_args.verbose);

  // set up DspDriver object

  if (!_args.sendCommands && !_args.queryCommands && !_args.getStatus) {
    _dspDriver = new DspDriver(!_args.noDsp);
    _dspDriver->setDebug(_args.debug);
    _dspDriver->setVerbose(_args.verbose);
    if (_dspDriver->init()) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Cannot initialize DspDriver" << endl;
      _dspDriver->close();
      delete _dspDriver;
      _dspDriver = NULL;
      OK = false;
      return;
    }
  }

  _tsStatus = new TsStatus(_args);

  // init process mapper registration
  
  if (_args.regWithProcmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _args.instance.c_str(),
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;

}

// destructor

Rvp8Driver::~Rvp8Driver()

{

  if (_dspDriver) {
    delete _dspDriver;
  }
  if (_tsStatus) {
    delete _tsStatus;
  }

}

//////////////////////////////////////////////////
// Run

int Rvp8Driver::Run()
{

  PMU_auto_register("Run");

  // if requested, send commands and exit
  
  if (_args.sendCommands) {
    if (_forwardCommands()) {
      return -1;
    } else {
      return 0;
    }
  }

  // if requested, query commands and exit
  
  if (_args.queryCommands) {
    if (_queryCommandsFromServer()) {
      return -1;
    } else {
      return 0;
    }
  }

  // if requested, get status and exit
  
  if (_args.getStatus) {
    if (_getStatusFromServer()) {
      return -1;
    } else {
      return 0;
    }
  }

  // check for startup xmp path

  if (_args.xmlPath.size() > 0) {

    if (_args.debug) {
      cerr << "Reading startup XML from file: "
           << _args.xmlPath << endl;
    }

    // set the command configuration from the startup file
    
    if (_setStartupCommands()) {
      cerr << "ERROR - Rvp8Driver::Run" << endl;
      cerr << "  Cannot set startup config from file: "
           << _args.xmlPath << endl;
      return -1;
    }

    if (_args.debug) {
      cerr << "-------- Startup command configuration -------------" << endl;
      time_t now = time(NULL);
      cerr << "  time: " << utimstr(now) << endl;
      _commands.print(cerr);
      cerr << "----------------------------------------------------" << endl;
    }

  }

  if (_args.noServer) {
    _runSleep();
  } else {
    _runServer();
  }

  return 0;
  
}

//////////////////////////////////////////////////
// Set startup config

int Rvp8Driver::_setStartupCommands()
{
  
  FILE *xml;
  if ((xml = fopen(_args.xmlPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Rvp8Driver::_setStartupCommands" << endl;
    cerr << "  Cannot open file: " << _args.xmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  string xmlBuf;
  char line[1024];

  while (fgets(line, 1024, xml) != NULL) {
    xmlBuf += line;
  }
  
  fclose(xml);
  
  string noCommentBuf = TaXml::removeComments(xmlBuf);
  _commands.set(noCommentBuf);

  // force all change flags true this first time, so that all
  // commands will be used to configure the DSP
  
  _commands.setChangeFlags(true);

  // configure the DSP

  _commands.configureDsp(_dspDriver);
  
  return 0;
  
}

//////////////////////////////////////////////////
// run in sleep mode

void Rvp8Driver::_runSleep()

{

  PMU_auto_register("Starting runSleep");

  while (true) {
    if (_dspDriver && _dspDriver->readAvailable()) {
      _dspDriver->init();
    }
    PMU_auto_register("Zzz...");
    umsleep(100);
  }
}

  
//////////////////////////////////////////////////
// Run server

void Rvp8Driver::_runServer()

{

  PMU_auto_register("Starting runServer");

  while (true) {

    ServerSocket server;
    if (server.openServer(_args.port)) {
      if (_args.debug) {
        cerr << "ERROR - Rvp8Driver" << endl;
        cerr << "  Cannot open server, port: " << _args.port << endl;
        cerr << "  " << server.getErrStr() << endl;
      }
      if (_dspDriver && _dspDriver->readAvailable()) {
	_dspDriver->init();
      }
      PMU_auto_register("Zzz...");
      umsleep(100);
      continue;
    }

    while (true) {

      // get a client
      
      Socket *sock = NULL;
      while (sock == NULL) {
	PMU_auto_register("Getting client");
        sock = server.getClient(100);
	if (_dspDriver && _dspDriver->readAvailable()) {
	  _dspDriver->init();
	}
      }

      // read incoming message

      string message;
      _readMessage(sock, message);

      // handle the message

      string reply;
      int iret = _handleMessage(message, reply);

      // send the reply

      _sendReply(sock, reply);
  
      // close connection

      sock->close();
      delete sock;
      
    } // while
      
  } // while

}

//////////////////////////////////////////////////
// Read incoming message
//
// Returns 0 on success, -1 on failure

int Rvp8Driver::_readIncoming(Socket *sock, const string &tag, string &xmlBuf)

{
  
  PMU_auto_register("Reading incoming");

  string latestLine;
  string endTok = "</";
  endTok += tag;
  endTok += ">";

  while (true) {
    
    // check for available data
    // time out after 10 secs if no end tag found
    
    int iret = sock->readSelect(10000);
    if (iret != 0) {
      if (sock->getErrNum() == SockUtil::TIMED_OUT) {
        return 0;
      } else {
        cerr << "ERROR - _readIncoming()" << endl;
        cerr << "  readSelect() failed" << endl;
        cerr << "  " << sock->getErrStr() << endl;
        return -1;
      }
    }

    // read all available data
    
    char cc;
    if (sock->readBuffer(&cc, 1, 100) == 0) {
      xmlBuf += cc;
      latestLine += cc;
      if (latestLine.find(endTok) != string::npos) {
	return 0;
      }
      if (cc == '\n') {
	latestLine.clear();
      }
    } else {
      if (sock->getErrNum() != SockUtil::TIMED_OUT) {
        cerr << "ERROR - _readIncoming()" << endl;
        cerr << "  readBuffer() failed" << endl;
        cerr << "  " << sock->getErrStr() << endl;
        return -1;
      }
    }

  }

  return -1;

}

//////////////////////////////////////////////////
// Read message
// Returns 0 on success, -1 on failure

int Rvp8Driver::_readMessage(Socket *sock, string &message)

{
  if (_args.verbose) {
    cerr << "Reading message" << endl;
  }
  if (_readIncoming(sock, "rvp8Message", message)) {
    return -1;
  }
  if (_args.verbose) {
    cerr << "============= incoming message= =============" << endl;
    cerr << message << endl;
    cerr << "=============================================" << endl;
  }
  return 0;
}

//////////////////////////////////////////////////
// Read reply
// Returns 0 on success, -1 on failure

int Rvp8Driver::_readReply(Socket *sock, string &reply)
  
{
  if (_args.verbose) {
    cerr << "Reading reply" << endl;
  }
  if (_readIncoming(sock, "rvp8Message", reply)) {
    return -1;
  }
  if (_args.verbose) {
    cerr << "================== reply ====================" << endl;
    cerr << reply << endl;
    cerr << "=============================================" << endl;
  }
  return 0;
}

//////////////////////////////////////////////////
// Send the reply 
// Returns 0 on success, -1 on failure

int Rvp8Driver::_sendReply(Socket *sock, const string &reply)
  
{
  
  PMU_auto_register("Sending reply");

  if (_args.verbose) {
    cerr << "Sending reply" << endl;
  }

  string message;
  message += TaXml::writeStartTag("rvp8Message", 0);
  message += reply;
  message += TaXml::writeEndTag("rvp8Message", 0);

  if (_args.verbose) {
    cerr << "============= sending reply ===============" << endl;
    cerr << reply << endl;
    cerr << "===========================================" << endl;
  }

  if (sock->writeBuffer((char *) message.c_str(), message.size() + 1)) {
    cerr << "ERROR - _sendReply" << endl;
    cerr << sock->getErrStr() << endl;
    return -1;
  }

  return 0;

}


//////////////////////////////////////////////////
// Handle message
//
// Fills out reply string
// returns 0 on success, -1 on failure

int Rvp8Driver::_handleMessage(const string &message,
                               string &reply)

{

  reply.clear();

  // find the action

  string action;
  if (TaXml::readString(message, "action", action)) {
    string error = "ERROR - Rvp8Driver::_handleMessage\n";
    error += "  Cannot find action tag in message\n";
    reply += TaXml::writeString("returnStatus", 0, "failure");
    reply += TaXml::writeString("error", 0, error);
    return -1;
  }
  
  if (action == "applyCommands") {
    return _applyCommands(message, reply);
  } else if (action == "queryCommands") {
    return _queryCommands(reply);
  } else if (action == "getStatus") {
    return _getStatus(reply);
  } else {
    string error = "ERROR - Rvp8Driver::_handleMessage\n";
    TaStr::AddStr(error, "  Invalid action: ", action);
    reply += TaXml::writeString("returnStatus", 0, "failure");
    reply += TaXml::writeString("error", 0, error);
    return -1;
  }

}

//////////////////////////////////////////////////
// Apply commands
// Fills out reply string
// returns 0 on success, -1 on failure

int Rvp8Driver::_applyCommands(const string &message,
                               string &reply)

{

  // find the commands

  string commands;
  if (TaXml::readString(message, "rvp8Commands", commands)) {
    string error = "ERROR - Rvp8Driver::_applyCommands\n";
    error += "  Cannot find rvp8Commands tag in message\n";
    reply += TaXml::writeString("returnStatus", 0, "failure");
    reply += TaXml::writeString("error", 0, error);
    return -1;
  }

  // strip comments
      
  string noCommentCommands = TaXml::removeComments(commands);

  // set the commands

  _commands.set(noCommentCommands);
  if (_args.debug) {
    time_t now = time(NULL);
    cerr << "-------- Updated command configuration -------------" << endl;
    cerr << "-------- time: " << utimstr(now)
         << " -----------------" << endl;
    _commands.print(cerr);
  }

  // configure the DSP
  // only those aspects which have changed will affect the configuration
  
  _commands.configureDsp(_dspDriver);

  // check read available
  
  if (_dspDriver) {
    _dspDriver->readAvailable();
  }
  
  // sleep 1 sec to be sure
  
  umsleep(1000);

  // success

  reply += TaXml::writeString("returnStatus", 0, "success");
  return 0;

}

//////////////////////////////////////////////////
// Query current command set
// Fills out reply string
// returns 0 on success, -1 on failure

int Rvp8Driver::_queryCommands(string &reply)

{

  if (_args.verbose) {
    cerr << "->> Querying commands" << endl;
  }

  reply += TaXml::writeBoolean("success", 0, true);
  string commandXml;
  _commands.getCommandXml(commandXml);
  reply += commandXml;

  return 0;

}

//////////////////////////////////////////////////
// get current status
// Fills out reply string
// returns 0 on success, -1 on failure

int Rvp8Driver::_getStatus(string &reply)
  
{

  PMU_auto_register("Getting status");

  if (_args.verbose) {
    cerr << "->> Getting status" << endl;
  }

  string status;
  if (_tsStatus->getStatusXml(status)) {
    _commands.getSimulatedStatusXml(status);
  }

  reply += TaXml::writeBoolean("success", 0, true);
  reply += status;

  return 0;

}

//////////////////////////////////////////////////
// Forward commands to server
//
// Returns 0 on success, -1 on failure

int Rvp8Driver::_forwardCommands()

{

  if (_args.verbose) {
    cerr << "Forwarding commands to server" << endl;
  }

  if (_args.xmlPath.size() == 0) {
    cerr << "ERROR - Rvp8Driver::_forwardCommands" << endl;
    cerr << "  No XML command file specificied." << endl;
    return -1;
  }

  // read in XML command file

  FILE *xml;
  if ((xml = fopen(_args.xmlPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Rvp8Driver::_forwardCommands" << endl;
    cerr << "  Cannot open XML file: " << _args.xmlPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  string xmlBuf;
  char line[1024];
  while (fgets(line, 1024, xml) != NULL) {
    xmlBuf += line;
  }
  fclose(xml);

  // create message

  string message;
  message += TaXml::writeStartTag("rvp8Message", 0);
  message += TaXml::writeString("action", 0, "applyCommands");
  message += xmlBuf;
  message += TaXml::writeEndTag("rvp8Message", 0);

  if (_args.verbose) {
    cerr << "============= outgoing commands =============" << endl;
    cerr << message << endl;
    cerr << "=============================================" << endl;
  }

  // open socket to Rvp8Driver

  Socket sock;
  if (sock.open(_args.host.c_str(),
                _args.port)) {
    cerr << "ERROR - Rvp8Driver::_forwardCommands" << endl;
    cerr << "  Cannot connect to Rvp8Driver: host,port: "
         << _args.host << ", "
         << _args.port << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  // write xml string to Rvp8Driver
  
  if (sock.writeBuffer((void *) message.c_str(), message.size() + 1)) {
    cerr << "ERROR - Rvp8Driver::_forwardCommands" << endl;
    cerr << "  Error writing data to Rvp8Driver: host,port: "
         << _args.host << ", "
         << _args.port << endl;
    cerr << "  " << sock.getErrStr() << endl;
    sock.close();
    return -1;
  }

  // read the reply
  
  string reply;
  if (_readReply(&sock, reply) == 0) {
    cerr << reply << endl;
  } else {
    cerr << "ERROR - forwardCommands" << endl;
    cerr << "  Cannot read reply from server" << endl;
  }
  
  // close the socket

  sock.close();

  return 0;

}

//////////////////////////////////////////////////
// Query commands from server
// Returns 0 on success, -1 on failure

int Rvp8Driver::_queryCommandsFromServer()

{

  // create message
  
  string message;
  message += TaXml::writeStartTag("rvp8Message", 0);
  message += TaXml::writeString("action", 0, "queryCommands");
  message += TaXml::writeEndTag("rvp8Message", 0);

  if (_args.debug) {
    cerr << "============= querying commands =============" << endl;
    cerr << message << endl;
    cerr << "=============================================" << endl;
  }

  // open socket to Rvp8Driver
  
  Socket sock;
  if (sock.open(_args.host.c_str(),
                _args.port)) {
    cerr << "ERROR - Rvp8Driver::_queryCommands" << endl;
    cerr << "  Cannot connect to Rvp8Driver: host,port: "
         << _args.host << ", "
         << _args.port << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  // write xml string to Rvp8Driver
  
  if (sock.writeBuffer((void *) message.c_str(), message.size() + 1)) {
    cerr << "ERROR - Rvp8Driver::_queryCommands" << endl;
    cerr << "  Error writing message to Rvp8Driver: host,port: "
         << _args.host << ", "
         << _args.port << endl;
    cerr << "  " << sock.getErrStr() << endl;
    sock.close();
    return -1;
  }
  
  // read the reply
  
  string reply;
  if (_readReply(&sock, reply)) {
    cerr << "ERROR - forwardCommands" << endl;
    cerr << "  Cannot read reply from server" << endl;
  }

  // close the socket

  sock.close();

  if (_args.debug) {
    cerr << "============== command reply ================" << endl;
    cerr << reply << endl;
    cerr << "=============================================" << endl;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// Get status from server
// Returns 0 on success, -1 on failure

int Rvp8Driver::_getStatusFromServer()

{

  // create message
  
  string message;
  message += TaXml::writeStartTag("rvp8Message", 0);
  message += TaXml::writeString("action", 0, "getStatus");
  message += TaXml::writeEndTag("rvp8Message", 0);

  if (_args.debug) {
    cerr << "=============== getting status ==============" << endl;
    cerr << message << endl;
    cerr << "=============================================" << endl;
  }

  // open socket to Rvp8Driver
  
  Socket sock;
  if (sock.open(_args.host.c_str(),
                _args.port)) {
    cerr << "ERROR - Rvp8Driver::_queryCommands" << endl;
    cerr << "  Cannot connect to Rvp8Driver: host,port: "
         << _args.host << ", "
         << _args.port << endl;
    cerr << "  " << sock.getErrStr() << endl;
    return -1;
  }

  // write xml string to Rvp8Driver
  
  if (sock.writeBuffer((void *) message.c_str(), message.size() + 1)) {
    cerr << "ERROR - Rvp8Driver::_queryCommands" << endl;
    cerr << "  Error writing message to Rvp8Driver: host,port: "
         << _args.host << ", "
         << _args.port << endl;
    cerr << "  " << sock.getErrStr() << endl;
    sock.close();
    return -1;
  }
  
  // read the reply
  
  string reply;
  if (_readReply(&sock, reply)) {
    cerr << "ERROR - forwardCommands" << endl;
    cerr << "  Cannot read reply from server" << endl;
  }

  // close the socket

  sock.close();

  if (_args.debug) {
    cerr << "=============== status reply ================" << endl;
    cerr << reply << endl;
    cerr << "=============================================" << endl;
  }
  
  return 0;

}

