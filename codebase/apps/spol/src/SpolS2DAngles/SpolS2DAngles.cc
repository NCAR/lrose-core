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
///////////////////////////////////////////////////////////////
// IwrfSpolS2DAngles.cc
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

#include "SpolS2DAngles.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/umisc.h>
#include <toolsa/ServerSocket.hh>
#include <sys/mman.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <termios.h>

using namespace std;

// Constructor

SpolS2DAngles::SpolS2DAngles(int argc, char **argv)
  
{

  isOK = true;

  _fdEl = -1;
  _fdAz = -1;

  _busData = NULL;
  _rawAngles = NULL;

  gettimeofday(&_prevTimeForRate, NULL);
  _prevEl = -9999.0;
  _prevAz = -9999.0;
  _sumDeltaAz = 0.0;
  _sumDeltaEl = 0.0;

  memset(&_angles, 0, sizeof(_angles));
  _seqNum = 0;

  pthread_mutex_init(&_readAnglesMutex, NULL);

  // set programe name
  
  _progName = "SpolS2DAngles";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }
  
  // init process mapper registration
  
  if (_params.reg_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  // run as daemon?

  if (_params.run_as_daemon) {
    _params.debug = Params::DEBUG_OFF;
    udaemonize();
  }
  
}

// destructor

SpolS2DAngles::~SpolS2DAngles()

{

  PMU_auto_unregister();
  if (_fdEl > 0) {
    close(_fdEl);
  }
  if (_fdAz > 0) {
    close(_fdAz);
  }

}

//////////////////////////////////////////////////
// Run

int SpolS2DAngles::Run()
{

  PMU_auto_register("Run");

  if (_params.sleep_msecs_on_start > 0) {
    umsleep(_params.sleep_msecs_on_start);
  }

  // init PC104 bus

  if (_initPc104()) {
    return -1;
  }

  // open com ports

  if (_params.pc104_available) {

    _fdEl = _openComDevice(_params.el_serial_device);
    _fdAz = _openComDevice(_params.az_serial_device);
    close(_fdEl);
    close(_fdAz);
    _fdEl = _openComDevice(_params.el_serial_device);
    _fdAz = _openComDevice(_params.az_serial_device);

    if (_fdEl < 0 || _fdAz < 0) {
      cerr << "WARNING - SpolS2DAngles" << endl;
      cerr << "  Cannot write angles to serial displays" << endl;
    }
    if (_params.debug) {
      cerr << "Opened com devices for LCDs" << endl;
      cerr << "  elevation tty, id: " 
           << _params.el_serial_device << ", " << _fdEl << endl;
      cerr << "  azimuth   tty, id: " 
           << _params.az_serial_device << ", " << _fdAz << endl;
    }
    // initialize LCD devices
    _lcdInit(_fdEl);
    _lcdInit(_fdAz);
  }

  // spawn the thread to handle the angle client

  _spawnServerThread();
  
  // read angles

  if (_displayAngles()) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// display angles

int SpolS2DAngles::_displayAngles()
{

  int count = 0;
  int msecsSleep =
    (int) floor((1.0 / _params.angle_display_frequency_hz) * 1000.0 + 0.5);

  // turn on screen again every 10 mins

  int nForRefresh = _params.angle_display_frequency_hz * 600.0;
  int countForRefresh = 0;
  
  while (true) {
    
    PMU_auto_register("Display angles");

    double az, el;
    _readAngles(az, el);
    _computeRates(az, el);
    count++;
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << " ==>> count, az, el: "
           << count << ", " << az << ", " << el << endl;
    }

    if (_params.displays_available) {
      if (_params.display_type == Params::GLK24064R) {
        _writeGLK24064R(el, az);
      } else {
        _writeLCD2041(el, az);
      }
    }

    countForRefresh++;
    if (countForRefresh == nForRefresh) {
      _lcdInit(_fdEl);
      _lcdInit(_fdAz);
      countForRefresh = 0;
    }

    umsleep(msecsSleep);

  } // while

  return 0;

}

//////////////////////////////////////////////////
// read in angles

void SpolS2DAngles::_readAngles(double &az, double &el)
{

  double mult = 360.0 / 65536.0;
  volatile u_int16_t jaz = 0;
  volatile u_int16_t jel = 0;

  if (_params.pc104_available) {
    
    // read the raw angles from the device
    
    pthread_mutex_lock(&_readAnglesMutex);
    jaz = _rawAngles->iaz;
    jel = _rawAngles->iel;
    pthread_mutex_unlock(&_readAnglesMutex);

    // add offsets as appropriate

    az = ((double) jaz * mult) + _params.azimuth_offset_deg;
    el = ((double) jel * mult) + _params.elevation_offset_deg;

    // condition the angles

    if (az > 360.0) {
      az -= 360.0;
    } else if (az < 0.0) {
      az += 360.0;
    }
    
    if (el > 180.0) {
      el -= 360.0;
    } else if (el < -180) {
      el += 360.0;
    }

  } else {

    // simulate

    time_t now = time(NULL);
    az = ((now % 200) / 200.0) * 360.0;
    el = ((now % 1000) / 1000.0) * 45.0;

  }

}

///////////////////////////
// compute angular rates

void SpolS2DAngles::_computeRates(double az, double el)

{

  // initialize

  if (_prevAz < -9990 || _prevEl < -9990) {
    gettimeofday(&_prevTimeForRate, NULL);
    _prevEl = el;
    _prevAz = az;
    _sumDeltaAz = 0.0;
    _sumDeltaEl = 0.0;
    _azRate = 0.0;
    _elRate = 0.0;
    return;
  }

  double deltaAz = az - _prevAz;
  if (deltaAz > 180.0) {
    deltaAz -= 360.0;
  } else if (deltaAz < -180.0) {
    deltaAz += 360.0;
  }
  _sumDeltaAz += deltaAz;

  double deltaEl = el - _prevEl;
  if (deltaEl > 180.0) {
    deltaEl -= 360.0;
  } else if (deltaEl < -180.0) {
    deltaEl += 360.0;
  }
  _sumDeltaEl += deltaEl;

  struct timeval now;
  gettimeofday(&now, NULL);

  double deltaTime =
    (double) (now.tv_sec - _prevTimeForRate.tv_sec) +
    (double) (now.tv_usec - _prevTimeForRate.tv_usec) / 1.0e6;
  
  if (deltaTime > 2.0) {
    _azRate = _sumDeltaAz / deltaTime;
    _elRate = _sumDeltaEl / deltaTime;
    _prevTimeForRate = now;
    _sumDeltaAz = 0.0;
    _sumDeltaEl = 0.0;
  }

  _prevEl = el;
  _prevAz = az;

}

//////////////////////////////////////////////////
// Initialize the PC104 ISA bus

int SpolS2DAngles::_initPc104()
{

  // map bus memory

  if (_params.pc104_available) {

    // get memory page size

    int pageSize = getpagesize();

    // open mem device

    int fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd == -1) {
      perror("open /dev/mem");
      return -1;
    }

    // memory map the bus configuration so that we can
    // do memory-based operations to read/write the bus
    
    _busData = (bus_data_t *)
      mmap(0, pageSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xE8000000);
    if (_busData == MAP_FAILED) {
      perror("mmap - cannot map bus data at address 0xE8000000");
      fprintf(stderr, "retval: %lld\n", (unsigned long long int) _busData); 
      return -1;
    }
    
    // memory map the angles so that we can
    // do memory-based operations to read/write the bus

    _rawAngles = (raw_angles_t *)
      mmap(0, pageSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0xED000000);
    if (_rawAngles == MAP_FAILED) {
      perror("mmap - cannot map raw angles at address 0xed000000");
      fprintf(stderr, "retval: %lld\n", (unsigned long long int) _rawAngles); 
      return -1;
    }

  } else {

    // debug mode - just allocated space

    _busData = new bus_data_t[1];
    _rawAngles = new raw_angles_t[1];

  }

  // read in the original configuration
  // because we only want to overwrite some of the bits
  // (this reads from the ISA bus)

  bus_data_t busDataOrig;
  memcpy(&busDataOrig, _busData, sizeof(busDataOrig));
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Original configuration at start:" << endl;
    cerr << "  rowa_mux: " << hex << busDataOrig.rowa_mux << endl;
    cerr << "  rowb_mux: " << hex << busDataOrig.rowb_mux << endl;
    cerr << "  rowc_mux: " << hex << busDataOrig.rowc_mux << endl;
    cerr << "  rowd_mux: " << hex << busDataOrig.rowd_mux << endl;
    cerr << "  config: " << hex << busDataOrig.config << endl;
    cerr << dec;
  }

  // enable bus
  // copy struct into memory at correct address
  
  bus_data_t busData;
  memset(&busData, 0, sizeof(busData));
  
  busData.rowa_mux = 0x55555555;
  busData.rowb_mux = 0x55555555;
  busData.rowc_mux = 0x00055555;
  busData.rowd_mux = 0x00055555;

  // config bus
  // bits 0-5: set to 8 (001000) - 20ns default + 80ns
  // bits 6-9: set to 3 (0011)   - 20ns default + 30ns
  // bit   10: set to 0 - don't honor ISA 0WS/ENDX signal
  // bit   11: set to 0 - TS special ISA pinout enable not
  // All other bits to 0.
  // write:
  //  0000 0000 0000 0000 0000 0000 1100 1000 = 0x000000c8
  // to
  //  0xE800000c
  
  // get original data for everything but last 13 bits
  
  int32_t origUpper = busDataOrig.config & 0xffffe000;

  // combine the original with the desired command

  busData.config = origUpper | 0x000000C8;

  // copy into place - writes to the bus
  
  memcpy(_busData, &busData, sizeof(busData));
  
  // read back from the bus to check

  bus_data_t busData2;
  memcpy(&busData2, _busData, sizeof(busData));

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Final configuration after setting bits 0-12:" << endl;
    cerr << "  rowa_mux: " << hex << busData2.rowa_mux << endl;
    cerr << "  rowb_mux: " << hex << busData2.rowb_mux << endl;
    cerr << "  rowc_mux: " << hex << busData2.rowc_mux << endl;
    cerr << "  rowd_mux: " << hex << busData2.rowd_mux << endl;
    cerr << "  config: " << hex << busData2.config << endl;
    cerr << dec;
  }

  return 0;

}

///////////////////////////////////////////////////
// open COM port - for angle readouts
// returns device id on success, -1 on error

int SpolS2DAngles::_openComDevice(const char *deviceName)
{
  
  int fd = open(deviceName, O_RDWR|O_NOCTTY|O_NDELAY);
  if (fd < 0) {
    perror(deviceName);
    fprintf(stderr, "ERROR - Cannot open COM device, name: %s\n", deviceName);
    return -1;
  }
  
  // terminal IO options
  struct termios options;
  if (tcgetattr(fd, &options) < 0) {
    perror("Can't get terminal parameters");
    fprintf(stderr, "ERROR - device, name: %s\n", deviceName);
    return -1;
  }
  
  // control options: 8-N-1
  options.c_cflag &= ~PARENB; // no parity
  options.c_cflag &= ~CSTOPB; // 1 stop bit
  options.c_cflag &= ~CSIZE; // mask character size bits
  // options.c_cflag &= ~(CSIZE|PARENB|CRTSCTS);
  options.c_cflag |= (CS8|CLOCAL|CREAD); /* 8 data bits, local line, 
                                          * enable receiver */
  
  // options.c_cflag &= ~CRTSCTS; // disable hardware flow control
  
  // input options
  options.c_iflag = IGNPAR; // ignore parity errors
  
  // output options
  options.c_oflag = 0;
  
  // line/local options
  options.c_lflag = 0;
  
  // control characters
  options.c_cc[VTIME] = 0; // time to wait for data
  options.c_cc[VMIN] = 0; // minimum number of characters to read
  
  // setting baud rate
  cfsetispeed(&options, B19200);
  cfsetospeed(&options, B19200);
  
  // cfsetispeed(&options, B115200);
  // cfsetospeed(&options, B115200);
  
  // flushes data received but not read
  tcflush(fd, TCIFLUSH);
  
  if (tcflush(fd, TCIFLUSH) < 0) {
    perror("TCflush error ");
    fprintf(stderr, "  Serial device: %s\n", deviceName);
    return -1;
  }
  
  // write the settings for the serial port
  tcsetattr(fd, TCSANOW, &options);
  if (tcsetattr(fd, TCSANOW, &options) < 0) {
    perror("Can't set terminal parameters ");
    fprintf(stderr, "  Serial device: %s\n", deviceName);
    return -1;
  }

  return fd;
  
}	

//////////////////////////////////
// set up LCD screen

void SpolS2DAngles::_lcdInit(int fd)
{

  if (fd < 0) {
    return;
  }
  
  // turn on

  _lcdOn(fd);

  // Clear Screen

  _lcdClear(fd);

  // Set auto scroll off
  
  unsigned char command[2];
  command[0] = 254; 
  command[1] = 82; 
  write(fd, command, sizeof(command));

  if (_params.display_type == Params::LCD2041) {

    _lcdSetCurrentFont(fd, 1);
    
  } else if (_params.display_type == Params::GLK24064R) {
    
    // for 24064 displays
    
    // set font - use factory type 2 - 15 pixels high
    
    _lcdSetCurrentFont(fd, 2);
    _setFontMetrics(fd, 1, 0, 2, 1, 1);

    // give time to settle

    umsleep(1000);

    // clear

    _fillRect(_fdEl, 0, 0, 0, 239, 63);
    _fillRect(_fdAz, 0, 0, 0, 239, 63);

    // draw labels
    
    char dateLabel[64];
    char timeLabel[64];
    char elLabel[64];
    char azLabel[64];
    char elRateLabel[64];
    char azRateLabel[64];
    
    sprintf(dateLabel, "Date");
    sprintf(timeLabel, "Time");
    sprintf(elLabel, "Elevation (deg)");
    sprintf(azLabel, "Azimuth (deg)");
    sprintf(elRateLabel, "El rate (deg/s)");
    sprintf(azRateLabel, "Az rate (deg/s)");
    
    if (_params.show_angles_on_both_displays) {

      // azimuth
      
      _setCursorCoord(_fdEl, 0, 0);
      _setCursorCoord(_fdAz, 0, 0);
      write(_fdEl, azLabel, strlen(azLabel));
      write(_fdAz, azLabel, strlen(azLabel));
      
      // elevation
      
      _setCursorCoord(_fdEl, 0, 16);
      _setCursorCoord(_fdAz, 0, 16);
      write(_fdEl, elLabel, strlen(elLabel));
      write(_fdAz, elLabel, strlen(elLabel));
      
      // az rate
      
      _setCursorCoord(_fdEl, 0, 33);
      _setCursorCoord(_fdAz, 0, 33);
      write(_fdEl, azRateLabel, strlen(azRateLabel));
      write(_fdAz, azRateLabel, strlen(azRateLabel));
      
      // el rate
      
      _setCursorCoord(_fdEl, 0, 49);
      _setCursorCoord(_fdAz, 0, 49);
      write(_fdEl, elRateLabel, strlen(elRateLabel));
      write(_fdAz, elRateLabel, strlen(elRateLabel));
      
    } else {

      // angles
      
      _setCursorCoord(_fdEl, 0, 0);
      _setCursorCoord(_fdAz, 0, 0);
      write(_fdEl, elLabel, strlen(elLabel));
      write(_fdAz, azLabel, strlen(azLabel));
      
      // rates
      
      _setCursorCoord(_fdEl, 0, 16);
      _setCursorCoord(_fdAz, 0, 16);
      write(_fdEl, elRateLabel, strlen(elRateLabel));
      write(_fdAz, azRateLabel, strlen(azRateLabel));
      
      // date
      
      _setCursorCoord(_fdEl, 0, 33);
      _setCursorCoord(_fdAz, 0, 33);
      write(_fdEl, dateLabel, strlen(dateLabel));
      write(_fdAz, dateLabel, strlen(dateLabel));
      
      // time
      
      _setCursorCoord(_fdEl, 0, 49);
      _setCursorCoord(_fdAz, 0, 49);
      write(_fdEl, timeLabel, strlen(timeLabel));
      write(_fdAz, timeLabel, strlen(timeLabel));

    }
    
  }

}

//////////////////////////////////
// turn LCD screen on

void SpolS2DAngles::_lcdOn(int fd)
{

  if (fd < 0) {
    return;
  }

  // turn on

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 66; 
  write(fd, command, sizeof(command));

  // set brightness high

  unsigned char command3[3];
  command3[0] = 254; 
  command3[1] = 153; 
  command3[2] = 255; 
  write(fd, command3, sizeof(command3));

  // set contrast medium

  command3[0] = 254; 
  command3[1] = 80; 
  command3[2] = 128; 
  write(fd, command3, sizeof(command3));

}

//////////////////////////////////
// clear LCD screen

void SpolS2DAngles::_lcdClear(int fd)
{

  if (fd < 0) {
    return;
  }

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 88; 

  write(fd, command, sizeof(command));

}

//////////////////////////////////
// set autoscroll state

void SpolS2DAngles::_setAutoScroll(int fd, bool state)
{
  
  if (fd < 0) {
    return;
  }

  unsigned char command[2];
  command[0] = 254; 
  if (state) {
    command[1] = 51;
  } else {
    command[1] = 52;
  }

  write(fd, command, sizeof(command));

}

//////////////////////////////////
// set LCD cursor posn
// this is in character space

void SpolS2DAngles::_lcdSetCursorPosn(int fd, int col, int row)
{

  if (fd < 0) {
    return;
  }

  unsigned char command[4];
  command[0] = 254; 
  command[1] = 71; 
  command[2] = col;
  command[3] = row; 

  write(fd, command, sizeof(command));

}

//////////////////////////////////
// set cursor coord - 24064 only
// this is in pixel space

void SpolS2DAngles::_setCursorCoord(int fd, int xx, int yy)
{

  if (fd < 0) {
    return;
  }

  unsigned char command[4];
  command[0] = 254; 
  command[1] = 121; 
  command[2] = xx;
  command[3] = yy; 

  write(fd, command, sizeof(command));

}

//////////////////////////////////
// set current font

void SpolS2DAngles::_lcdSetCurrentFont(int fd, int fontId)
{

  if (fd < 0) {
    return;
  }

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 49; 
  write(fd, command, sizeof(command));

  si16 id = fontId;
  write(fd, &id, sizeof(id));

}

//////////////////////////////////
// set font spacing etc

void SpolS2DAngles::_setFontMetrics(int fd,
                                    int lineMargin,
                                    int topMargin,
                                    int charSpace,
                                    int lineSpace,
                                    int scrollPoint)
  
{
  
  if (fd < 0) {
    return;
  }

  // command

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 32;
  write(fd, command, sizeof(command));

  // metrics

  unsigned char mbuf[5];
  mbuf[0] = lineMargin;
  mbuf[1] = topMargin; 
  mbuf[2] = charSpace; 
  mbuf[3] = lineSpace; 
  mbuf[4] = scrollPoint; 
  write(fd, mbuf, sizeof(mbuf));

}

//////////////////////////////////
// initalize label space on 24064

void SpolS2DAngles::_initLabel(int fd, int labelId,
                               int x1, int y1,
                               int x2, int y2,
                               int fontId)
                               
{
  
  if (fd < 0) {
    return;
  }

  // command

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 45; 
  write(fd, command, sizeof(command));

  // id

  unsigned char id = labelId;
  write(fd, &id, sizeof(id));

  // coords

  unsigned char cbuf[4];
  cbuf[0] = x1; 
  cbuf[1] = y1; 
  cbuf[2] = x2; 
  cbuf[3] = y2; 
  write(fd, cbuf, sizeof(cbuf));

  // justification

  unsigned char jbuf[2];
  jbuf[0] = 1; // vert justified (0 top, 1 middle, 2 bot)
  jbuf[1] = 1; // horiz justified (0 left, 1 center, 2 right)
  write(fd, jbuf, sizeof(jbuf));

  // font id

  si16 fid = fontId;
  write(fd, &fid, sizeof(fid));

  // background, spacing

  unsigned char sbuf[2];
  sbuf[0] = 0; // state of background pixels (0 off, 1 on)
  sbuf[1] = 2; // spacing for label chars
  write(fd, sbuf, sizeof(sbuf));

}

//////////////////////////////////
// update label text

void SpolS2DAngles::_updateLabelText(int fd, int labelId,
                                     char *text)
                               
{
  
  if (fd < 0) {
    return;
  }
  
  // command

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 46; 
  write(fd, command, sizeof(command));

  // id
  
  unsigned char id = labelId;
  write(fd, &id, sizeof(id));

  // string

  int textLen = strlen(text) + 1;
  write(fd, text, textLen);

}

//////////////////////////////////
// initalize text window on 24064

void SpolS2DAngles::_initTextWindow(int fd, int windowId,
                                    int x1, int y1,
                                    int x2, int y2,
                                    int fontId)
                               
{
  
  if (fd < 0) {
    return;
  }

  // command

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 43; 
  write(fd, command, sizeof(command));

  // id

  unsigned char id = windowId;
  write(fd, &id, sizeof(id));
  
  // coords

  unsigned char cbuf[4];
  cbuf[0] = x1; 
  cbuf[1] = y1; 
  cbuf[2] = x2; 
  cbuf[3] = y2; 
  write(fd, cbuf, sizeof(cbuf));

  // font id

  si16 fid = fontId;
  write(fd, &fid, sizeof(fid));

  // spacing, scrolling

  unsigned char sbuf[3];
  sbuf[0] = 2; // spacing for label chars
  sbuf[1] = 0; // spacing for lines
  sbuf[2] = 100; // number of pixel rows to write before scrolling
  write(fd, sbuf, sizeof(sbuf));

}

//////////////////////////////////
// set the text window

void SpolS2DAngles::_setTextWindow(int fd, int windowId)
                               
{
  
  if (fd < 0) {
    return;
  }
  
  // command

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 42; 
  write(fd, command, sizeof(command));

  // id
  
  unsigned char id = windowId;
  write(fd, &id, sizeof(id));

}

//////////////////////////////////
// clear a text window

void SpolS2DAngles::_clearTextWindow(int fd, int windowId)
                               
{
  
  if (fd < 0) {
    return;
  }
  
  // command

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 44; 
  write(fd, command, sizeof(command));

  // id
  
  unsigned char id = windowId;
  write(fd, &id, sizeof(id));

}

//////////////////////////////////
// draw line

void SpolS2DAngles::_drawLine(int fd,
                              int x1, int y1,
                              int x2, int y2)
                               
{
  
  if (fd < 0) {
    return;
  }

  // command

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 108; 
  write(fd, command, sizeof(command));

  // coords

  unsigned char cbuf[4];
  cbuf[0] = x1; 
  cbuf[1] = y1; 
  cbuf[2] = x2; 
  cbuf[3] = y2; 
  write(fd, cbuf, sizeof(cbuf));

}

//////////////////////////////////
// fill a rectangle

void SpolS2DAngles::_fillRect(int fd, int color,
                              int x1, int y1,
                              int x2, int y2)
                               
{
  
  if (fd < 0) {
    return;
  }

  // command

  unsigned char command[2];
  command[0] = 254; 
  command[1] = 120; 
  write(fd, command, sizeof(command));

  // color and coords

  unsigned char cbuf[5];
  cbuf[0] = color; 
  cbuf[1] = x1; 
  cbuf[2] = y1; 
  cbuf[3] = x2; 
  cbuf[4] = y2; 
  write(fd, cbuf, sizeof(cbuf));

}

/////////////////////////////////////////
// write to LCD 2041 panels

int SpolS2DAngles::_writeLCD2041(double el, double az)
{ 

  if (_fdEl < 0 || _fdAz < 0) {
    return -1;
  }

  if (el > 360) {
    el = el - 360;
  }
  
  if (az > 360) {
    az = az -360;
  } 

  // round angles to 2 digits

  double azRounded = (floor(az*100.0))/100.0; 
  double elRounded = (floor(el*100.0))/100.0; 

  // round rates to 2 digits
  
  double azRateRounded = (floor(_azRate*100.0))/100.0; 
  double elRateRounded = (floor(_elRate*100.0))/100.0; 

  // get time

  struct timeval tv;
  gettimeofday(&tv, NULL);
  DateTime now(tv.tv_sec);
  int msecs = tv.tv_usec / 1000;

  // compute strings
  
  char dateStr[32];
  char timeStr[32];
  char elStr[32];
  char azStr[32];
  char elRateStr[32];
  char azRateStr[32];

  sprintf(dateStr, "Date:     %.4d/%.2d/%.2d",
          now.getYear(), now.getMonth(), now.getDay());
  sprintf(timeStr, "Time:    %.2d:%.2d:%.2d.%.2d",
          now.getHour(), now.getMin(), now.getSec(), msecs / 10);
  sprintf(elStr, "El:     %8.2f deg", elRounded);
  sprintf(azStr, "Az:     %8.2f deg", azRounded);
  sprintf(elRateStr, "El rate: %7.2f d/s", elRateRounded);
  sprintf(azRateStr, "Az rate: %7.2f d/s", azRateRounded);

  if (_params.show_angles_on_both_displays) {

    // write azimuth
    
    _lcdSetCursorPosn(_fdEl, 0, 1);
    _lcdSetCursorPosn(_fdAz, 0, 1);
    write(_fdEl, azStr, strlen(azStr));
    write(_fdAz, azStr, strlen(azStr));
    
    // write elevation
    
    _lcdSetCursorPosn(_fdEl, 0, 2);
    _lcdSetCursorPosn(_fdAz, 0, 2);
    write(_fdEl, elStr, strlen(elStr));
    write(_fdAz, elStr, strlen(elStr));
    
    // write az rate
    
    _lcdSetCursorPosn(_fdEl, 0, 3);
    _lcdSetCursorPosn(_fdAz, 0, 3);
    write(_fdEl, azRateStr, strlen(azRateStr));
    write(_fdAz, azRateStr, strlen(azRateStr));
    
    // write el rate
    
    _lcdSetCursorPosn(_fdEl, 0, 4);
    _lcdSetCursorPosn(_fdAz, 0, 4);
    write(_fdEl, elRateStr, strlen(elRateStr));
    write(_fdAz, elRateStr, strlen(elRateStr));
    
  } else {
  
    // write angle
    
    _lcdSetCursorPosn(_fdEl, 0, 1);
    _lcdSetCursorPosn(_fdAz, 0, 1);
    write(_fdEl, elStr, strlen(elStr));
    write(_fdAz, azStr, strlen(azStr));
    
    // write rate
    
    _lcdSetCursorPosn(_fdEl, 0, 2);
    _lcdSetCursorPosn(_fdAz, 0, 2);
    write(_fdEl, elRateStr, strlen(elRateStr));
    write(_fdAz, azRateStr, strlen(azRateStr));
    
    // write date
    
    _lcdSetCursorPosn(_fdEl, 0, 3);
    _lcdSetCursorPosn(_fdAz, 0, 3);
    write(_fdEl, dateStr, strlen(dateStr));
    write(_fdAz, dateStr, strlen(dateStr));
    
    // write time
    
    _lcdSetCursorPosn(_fdEl, 0, 4);
    _lcdSetCursorPosn(_fdAz, 0, 4);
    write(_fdEl, timeStr, strlen(timeStr));
    write(_fdAz, timeStr, strlen(timeStr));

  }

  if (_params.debug) {
    cerr << "=======>> Wrote to LCD: " << endl;
    cerr << dateStr << endl;
    cerr << timeStr << endl;
    cerr << elStr << endl;
    cerr << azStr << endl;
    cerr << elRateStr << endl;
    cerr << azRateStr << endl;
  }

  return 0;

}

/////////////////////////////////////////
// write to GLK24064 panels

int SpolS2DAngles::_writeGLK24064R(double el, double az)
{ 

  if (_fdEl < 0 || _fdAz < 0) {
    return -1;
  }

  if (el > 360) {
    el = el - 360;
  }
  
  if (az > 360) {
    az = az -360;
  } 

  // round angles to 2 digits

  double azRounded = (floor(az*100.0))/100.0; 
  double elRounded = (floor(el*100.0))/100.0; 

  // round rates to 2 digits
  
  double azRateRounded = (floor(_azRate*100.0))/100.0; 
  double elRateRounded = (floor(_elRate*100.0))/100.0; 

  // get time

  struct timeval tv;
  gettimeofday(&tv, NULL);
  DateTime now(tv.tv_sec);
  int msecs = tv.tv_usec / 1000;

  // compute strings
  
  char dateStr[64];
  char timeStr[64];
  char elStr[64];
  char azStr[64];
  char elRateStr[64];
  char azRateStr[64];

  sprintf(dateStr, "%.4d/%.2d/%.2d",
          now.getYear(), now.getMonth(), now.getDay());
  sprintf(timeStr, "%.2d:%.2d:%.2d.%.2d",
          now.getHour(), now.getMin(), now.getSec(), msecs / 10);
  sprintf(elStr, "%.2f", elRounded);
  sprintf(azStr, "%.2f", azRounded);
  sprintf(elRateStr, "%.2f", elRateRounded);
  sprintf(azRateStr, "%.2f", azRateRounded);

  int startX = 140;
  int endX = 239;

  if (_params.show_angles_on_both_displays) {

    // write azimuth
    
    _setCursorCoord(_fdEl, startX, 0);
    _setCursorCoord(_fdAz, startX, 0);
    
    _fillRect(_fdEl, 0, startX, 0, endX, 15);
    _fillRect(_fdAz, 0, startX, 0, endX, 15);
    
    write(_fdEl, azStr, strlen(azStr));
    write(_fdAz, azStr, strlen(azStr));
    
    // write elevation
    
    _setCursorCoord(_fdEl, startX, 16);
    _setCursorCoord(_fdAz, startX, 16);
    
    _fillRect(_fdEl, 0, startX, 16, endX, 31);
    _fillRect(_fdAz, 0, startX, 16, endX, 31);
    
    write(_fdEl, elStr, strlen(elStr));
    write(_fdAz, elStr, strlen(elStr));
    
    // write azimuth rate
    
    _setCursorCoord(_fdEl, startX, 32);
    _setCursorCoord(_fdAz, startX, 32);
    
    _fillRect(_fdEl, 0, startX, 32, endX, 47);
    _fillRect(_fdAz, 0, startX, 32, endX, 47);
    
    write(_fdEl, azRateStr, strlen(azRateStr));
    write(_fdAz, azRateStr, strlen(azRateStr));
    
    // write elevation rate
    
    _setCursorCoord(_fdEl, startX, 48);
    _setCursorCoord(_fdAz, startX, 48);
    
    _fillRect(_fdEl, 0, startX, 48, endX, 63);
    _fillRect(_fdAz, 0, startX, 48, endX, 63);

    write(_fdEl, elRateStr, strlen(elRateStr));
    write(_fdAz, elRateStr, strlen(elRateStr));
    
  } else {

    // write angles
    
    _setCursorCoord(_fdEl, startX, 0);
    _setCursorCoord(_fdAz, startX, 0);
    
    _fillRect(_fdEl, 0, startX, 0, endX, 15);
    _fillRect(_fdAz, 0, startX, 0, endX, 15);
    
    write(_fdEl, elStr, strlen(elStr));
    write(_fdAz, azStr, strlen(azStr));
    
    // write rates
    
    _setCursorCoord(_fdEl, startX, 16);
    _setCursorCoord(_fdAz, startX, 16);
    
    _fillRect(_fdEl, 0, startX, 16, endX, 31);
    _fillRect(_fdAz, 0, startX, 16, endX, 31);
    
    write(_fdEl, elRateStr, strlen(elRateStr));
    write(_fdAz, azRateStr, strlen(azRateStr));
    
    // write date
    
    _setCursorCoord(_fdEl, startX, 32);
    _setCursorCoord(_fdAz, startX, 32);
    
    _fillRect(_fdEl, 0, startX, 32, endX, 47);
    _fillRect(_fdAz, 0, startX, 32, endX, 47);
    
    write(_fdEl, dateStr, strlen(dateStr));
    write(_fdAz, dateStr, strlen(dateStr));
    
    // write time
    
    _setCursorCoord(_fdEl, startX, 48);
    _setCursorCoord(_fdAz, startX, 48);
    
    _fillRect(_fdEl, 0, startX, 48, endX, 63);
    _fillRect(_fdAz, 0, startX, 48, endX, 63);

    write(_fdEl, timeStr, strlen(timeStr));
    write(_fdAz, timeStr, strlen(timeStr));

  }

  if (_params.debug) {
    cerr << "=======>> Wrote to LCD: " << endl;
    cerr << "date: " << dateStr << endl;
    cerr << "time: " << timeStr << endl;
    cerr << "el: " << elStr << endl;
    cerr << "az: " << azStr << endl;
    cerr << "elRate: " << elRateStr << endl;
    cerr << "azRate: " << azRateStr << endl;
  }

  return 0;

}

///////////////////////////////////////////////////
// Spawn a thread for handling the tcp server

void SpolS2DAngles::_spawnServerThread()

{
  
  // Start a thread.
  
  pthread_t thread;
  int err = pthread_create(&thread, NULL, _setupServer, this);
  
  if (err != 0) {
    // thread creation error
    cerr << "Error in spawnServerThread()" << endl;
    cerr << "Could not create thread to handle client" << endl;
    return;
  }
  
  // thread created successfully

  if (_params.debug) {
    cerr << "---> created tcp client thread, id: " << thread << endl;
  }
  
}

///////////////////////////////////////////////////
// _setupServer(void *context)
// Static member which then calls _runTcpServer()

void *SpolS2DAngles::_setupServer(void * context)
{

  SpolS2DAngles *app = (SpolS2DAngles *) context;
  return app->_runTcpServer();
  
}

//////////////////////////////////////////////////
// Run TCP server

void *SpolS2DAngles::_runTcpServer()
{

  while (true) {

    char msg[1024];
    sprintf(msg, "Opening port: %d", _params.server_port);
    PMU_auto_register(msg);

    // set up listening server
    
    ServerSocket server;
    if (server.openServer(_params.server_port)) {
      if (_params.debug) {
        cerr << "ERROR - SpolS2DAngles" << endl;
        cerr << "  Cannot open server, port: " << _params.server_port << endl;
        cerr << "  " << server.getErrStr() << endl;
      }
      umsleep(100);
      continue;
    }
    
    if (_params.debug) {
      cerr << "Running server, listening on port: "
           << _params.server_port << endl;
    }

    while (true) {

      // register with procmap
      sprintf(msg, "Getting clients, port: %d", _params.server_port);
      PMU_auto_register(msg);
      
      // get a client
      
      Socket *sock = NULL;
      while (sock == NULL) {
        // get client
        sock = server.getClient(1000);
	// register with procmap
	PMU_auto_register(msg);
      }

      if (_params.debug) {
        cerr << "  Got client ..." << endl;
      }

      // child - provide service to client
      
      _handleClient(sock);
      sock->close();
      delete sock;

    } // while
      
  } // while

  return NULL;

}

//////////////////////////////////////////////////
// provide data to client

int SpolS2DAngles::_handleClient(Socket *sock)

{

  if (_params.debug) {
    cerr << "  Child - handling client ..." << endl;
  }
  
  // initialize
  
  MEM_zero(_angles);
  
  // loop until error
  
  while (true) {

    // read command from client - will block

    if (_readCommand(sock)) {
      return -1;
    }

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Command from client: " << hex << _command << dec << endl;
    }

    // get antenna position

    double az, el;
    _readAngles(az, el);
    _angles.azimuth = az;
    _angles.elevation = el;
    
    // increment sequence number
    
    _seqNum++;
    _angles.seq_num = _seqNum;

    if (_params.debug) {
      if (_seqNum % 1000 == 0) {
        cerr << "Angle seq num: " << _seqNum << endl;
      }
    }
    
    // get time
    
    struct timeval tval;
    gettimeofday(&tval, NULL);
    _angles.time_secs_utc = tval.tv_sec;
    _angles.time_nano_secs = tval.tv_usec * 1000;
    
    // write angle posn to client

    if (_writeAngles(sock)) {
      return -1;
    }

  } // while
      
  return -1;

}

//////////////////////////////////////////////////
// read command from client
//
// Returns 0 on success, -1 on failure

int SpolS2DAngles::_readCommand(Socket *sock)

{

  // wait for message from client - will block
  
  if (sock->readSelect()) {
    if (_params.debug) {
      cerr << "ERROR - _readCommand" << endl;
      cerr << "  Error waiting to read from client" << endl;
      cerr << "  " << sock->getErrStr() << endl;
    }
    return -1;
  }

  // read the message

  if (sock->readBuffer((void *) &_command, sizeof(_command))) {
    if (_params.debug) {
      cerr << "ERROR - _readCommand" << endl;
      cerr << "  Error reading command from client" << endl;
      cerr << "  " << sock->getErrStr() << endl;
    }
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// write angle position to client
//
// Returns 0 on success, -1 on failure

int SpolS2DAngles::_writeAngles(Socket *sock)

{
  
  if (sock->writeBuffer((void *) &_angles, sizeof(_angles))) {
    if (_params.debug) {
      cerr << "ERROR - writeResponse" << endl;
      cerr << "  Error writing position  to client" << endl;
      cerr << "  " << sock->getErrStr() << endl;
    }
    return -1;
  }

  return 0;

}

