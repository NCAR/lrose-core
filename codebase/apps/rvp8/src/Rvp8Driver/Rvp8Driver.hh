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
/////////////////////////////////////////////////////////////
// Rvp8Driver.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////

#ifndef Rvp8Driver_H
#define Rvp8Driver_H

#include "Args.hh"
#include "Commands.hh"
#include "TsStatus.hh"
#include <rvp8_rap/Socket.hh>
#include <string>
class DspDriver;
using namespace std;

class Rvp8Driver {
  
public:

  // constructor

  Rvp8Driver (int argc, char **argv);
  
  // destructor
  
  ~Rvp8Driver();

  // run 

  int Run();

  // data members

  int OK;

protected:
  
private:

  string _progName;
  Args _args;

  Commands _commands;
  DspDriver *_dspDriver;
  TsStatus *_tsStatus;

  int _setStartupCommands();
  void _runSleep();
  void _runServer();

  int _readIncoming(Socket *sock, const string &tag, string &xmlBuf);
  int _readMessage(Socket *sock, string &message);
  int _readReply(Socket *sock, string &reply);
  int _sendReply(Socket *sock, const string &reply);

  int _handleMessage(const string &message, string &reply);
  int _applyCommands(const string &message, string &reply);
  int _queryCommands(string &reply);
  int _getStatus(string &reply);

  int _forwardCommands();
  int _queryCommandsFromServer();
  int _getStatusFromServer();

};

#endif
