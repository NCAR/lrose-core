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
/*
 * main.cpp
 *
 * Created Nov 2010
 *      Author: Mike Dixon
 *      Author: Based on code by Charlie Martin
 *      EOL, NCAR, Boulder, CO, USA
 */

#include <QApplication>
#include <QPushButton>

#include <iostream>
#include <boost/program_options.hpp>
#include "QtConfig.h"
#include "AScopeReader.h"
#include "AScope.h"
#include <radar/iwrf_data.h>

using namespace std;

double _refreshHz;       ///< The scope refresh rate in Hz
string _serverHost; ///< The host name for the time series server
int _serverPort;         ///< The port for the time series server
string _serverFmq; ///< The FMQ name, if using fmq instead of tcp
int _debugLevel;
string _saveDir;            ///< The image save directory
string _title;
bool _simulMode;
int _radarId;
int _burstChan;

namespace po = boost::program_options;

//////////////////////////////////////////////////////////////////////
///
/// get parameters that are specified in the configuration file.
/// These can be overriden by command line specifications.
void getConfigParams()
{

  QtConfig config("TcpScope", "TcpScope");
  
  _saveDir = config.getString("SaveDir", ".");
  _title = config.getString("Title", "TcpScope");
  
  _refreshHz    = config.getDouble("RefreshHz",  50.0);
  _serverHost = "localhost";
  _serverPort = 10000;
  _serverFmq.clear();
  _debugLevel = 0;
  _radarId = 0;
  _burstChan = -1;

}

//////////////////////////////////////////////////////////////////////
//
/// Parse the command line options, and also set some options
/// that are not specified on the command line.
/// @return The runtime options that can be passed to the
/// threads that interact with the RR314.
void parseOptions(int argc,
                  char** argv)
{
  
  // get the options
  po::options_description descripts("Options");
  descripts.add_options()
    ("help", "describe options")
    ("title", po::value<string>(&_title), "Set the title")
    ("RefreshHz", po::value<double>(&_refreshHz), "Refresh rate (Hz)")
    ("fmq", po::value<string>(&_serverFmq), "Set the FMQ path - if FMQ mode")
    ("host", po::value<string>(&_serverHost), "Set the server host")
    ("port", po::value<int>(&_serverPort), "Set the server port")
    ("simul", "use simultanous mode")
    ("radarId", po::value<int>(&_radarId),
     "Set radarId if data contains multiple IDs, 0 uses all data")
    ("burstChan", po::value<int>(&_burstChan),
     "Set burst channel (0 to 3) in alternating mode")
    ("debug", po::value<int>(&_debugLevel),
     "Set the debug level: 0, 1, or 2. 0 is the default")
    ;

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, descripts), vm);
  }
  catch(exception & ex) {
    cerr << "ERROR parsing command line: " << ex.what() << endl;
    cerr << descripts << endl;
    exit(1);
  }
  po::notify(vm);

  if (vm.count("help")) {
    cout << descripts << endl;
    exit(1);
  }

  _simulMode = false;
  if (vm.count("simul")) {
    _simulMode = true;
  }

}


int
  main (int argc, char** argv) {

  // get the configuration parameters from the configuration file
  getConfigParams();

  // parse the command line optins, substituting for config params.
  parseOptions(argc, argv);

  if (_debugLevel) {
    cerr << "Running tcpscope, title: " << _title << endl;
    if (_serverFmq.size() > 0) {
      cerr << "  server fmq: " << _serverFmq << endl;
    } else {
      cerr << "  server host: " << _serverHost << endl;
      cerr << "  server port: " << _serverPort << endl;
    }
  }

  QApplication app(argc, argv);
  
  // create the scope

  AScope scope(_refreshHz, _saveDir);
  scope.setWindowTitle(QString(_title.c_str()));
  scope.show();

  // create the data source reader
  
  AScopeReader reader(_serverHost, _serverPort, _serverFmq,
                      _simulMode, scope, _radarId, _burstChan, _debugLevel);
  
  // connect the reader to the scope to receive new time series data
  
  scope.connect(&reader, SIGNAL(newItem(AScope::TimeSeries)),
                &scope, SLOT(newTSItemSlot(AScope::TimeSeries)));
  
  // connect the scope to the reader to return used time series data

  scope.connect(&scope, SIGNAL(returnTSItem(AScope::TimeSeries)),
                &reader, SLOT(returnItemSlot(AScope::TimeSeries)));

  return app.exec();
}
