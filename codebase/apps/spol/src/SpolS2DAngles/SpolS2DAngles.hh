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
/////////////////////////////////////////////////////////////
// SpolS2DAngles.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2014
//
///////////////////////////////////////////////////////////////
//
// SpolS2DAngles reads angles from the TS-7800, listens fo
// a client, and writes the angles to the client.
//
////////////////////////////////////////////////////////////////

#ifndef SpolS2DAngles_H
#define SpolS2DAngles_H

#include <string>
#include <ctime>
#include <dataport/port_types.h>
#include <radar/spol_angles.hh>
#include <pthread.h>

#include "Args.hh"
#include "Params.hh"
class Socket;

using namespace std;

////////////////////////
// This class

class SpolS2DAngles {
  
public:

  // constructor

  SpolS2DAngles(int argc, char **argv);

  // destructor
  
  ~SpolS2DAngles();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
private:

  string _progName;
  Args _args;
  char *_paramsPath;
  Params _params;

  // controlling the bus

  typedef struct { 
    si32 pad1[3];
    si32 config; 
    si32 pad2[8];
    si32 rowa_mux; 
    si32 rowb_mux; 
    si32 rowc_mux; 
    si32 rowd_mux; 
  } bus_data_t;
  
  bus_data_t *_busData;
  
  // reading angles via the bus

  typedef struct { 
    ui16 iaz;
    ui16 iel;
  } raw_angles_t;
  
  volatile raw_angles_t *_rawAngles;

  // computing rates

  struct timeval _prevTimeForRate;
  double _prevAz;
  double _prevEl;
  double _sumDeltaAz;
  double _sumDeltaEl;
  double _azRate;
  double _elRate;

  // reading command from TCP client

  ui32 _command;
  
  // writing angle data to TCP client
  
  ui32 _seqNum;
  spol_short_angle_t _angles;

  // writing angles to readouts using serial ports

  int _fdEl;
  int _fdAz;

  // Thread mutex variables and access methods.

  pthread_mutex_t _readAnglesMutex;
  
  // prototypes

  int _displayAngles();
  void _readAngles(double &az, double &el);
  void _computeRates(double az, double el);

  int _initPc104();
  int _openComDevice(const char *deviceName);

  void _lcdInit(int fd);
  void _lcdOn(int fd);
  void _lcdClear(int fd);
  void _setAutoScroll(int fd, bool state);

  void _lcdSetCursorPosn(int fd, int col, int row);
  void _setCursorCoord(int fd, int xx, int yy);

  void _lcdSetCurrentFont(int fd, int fontId);

  void _setFontMetrics(int fd,
                       int lineMargin,
                       int topMargin,
                       int charSpace,
                       int lineSpace,
                       int scrollPoint);

  void _initLabel(int fd, int labelId,
                  int x1, int y1,
                  int x2, int y2,
                  int fontId);

  void _updateLabelText(int fd, int labelId,
                        char *text);

  void _initTextWindow(int fd, int windowId,
                       int x1, int y1,
                       int x2, int y2,
                       int fontId);

  void _setTextWindow(int fd, int windowId);
  void _clearTextWindow(int fd, int windowId);

  void _drawLine(int fd,
                 int x1, int y1,
                 int x2, int y2);
  
  void _fillRect(int fd, int color,
                 int x1, int y1,
                 int x2, int y2);

  int _writeLCD2041(double el, double az);
  int _writeGLK24064R(double el, double az);

  void _spawnServerThread();
  static void *_setupServer(void * context);
  void *_runTcpServer();
  int _handleClient(Socket *sock);
  int _readCommand(Socket *sock);
  int _writeAngles(Socket *sock);

};

#endif
