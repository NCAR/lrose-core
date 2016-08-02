/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 13:58:59
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// rapicTitanClientArgs.hh: Command line object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2001
//
/////////////////////////////////////////////////////////////

#ifndef RAPIC_TITAN_CLIENT_ARGS_H
#define RAPIC_TITAN_CLIENT_ARGS_H

#include <stream.h>
#include <string>
#include <titan/TitanServer.hh>

class rapicTitanClientArgs {
  
public:

  // constructor

  rapicTitanClientArgs(char *initstring = 0);

  // destructor

  virtual ~rapicTitanClientArgs();

  // parse

  int parse(char *initstring);

  // public data

  string url;

  tserver_read_time_mode_t readTimeMode;
  tserver_track_set_t trackSet;
  time_t requestTime;
  int readTimeMargin;
  int requestComplexNum;

  bool debug;
  bool readLprops;
  bool readDbzHist;
  bool readRuns;
  bool readProjRuns;

protected:
  
private:

  string _progName;
  void _usage(ostream &out);
  
};

#endif

