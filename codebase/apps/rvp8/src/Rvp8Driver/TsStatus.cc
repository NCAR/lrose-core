// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:30:34 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// TsStatus.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
///////////////////////////////////////////////////////////////

#include "TsStatus.hh"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctime>
#include <rvp8_rap/DateTime.hh>
#include <rvp8_rap/uusleep.h>
#include <rvp8_rap/macros.h>
#include <rvp8_rap/ServerSocket.hh>
#include <rvp8_rap/TaXml.hh>
#include "Commands.hh"

using namespace std;

// Constructor

TsStatus::TsStatus(const Args &args) :
        _args(args)
  
{

  // if (_args.verbose) {
  //   _reader.setDebug(true);
  // }

}

// destructor

TsStatus::~TsStatus()

{
  _reader.cleanUp();
}

//////////////////////////////////////////////////
// Get status in XML form

int TsStatus::getStatusXml(string &xml)
{

  // attach to RVP8
  
  if (_reader.connectToTsApi()) {
    cerr << "ERROR - TsStatus::getStatusXml" << endl;
    return -1;
  }
  
  // read info and pulse

  const TsInfo &info = _reader.getInfo();

  TsPulse pulse(info.getClockMhz());
  int count = 0;
  while (count < 1000) {
    if (_reader.readPulse(pulse) == 0) {
      break;
    }
    count++;
  }
  if (count > 999) {
    cerr << "ERROR - TsStatus::getStatusXml" << endl;
    cerr << "  Cannot read pulse" << endl;
    _reader.disconnectFromTsApi();
    return -1;
  }
  _reader.disconnectFromTsApi();

  // derive pulse status

  time_t ptime = pulse.getTime();
  double prf = 1.0 / pulse.getPrt();
  int nGates = pulse.getNGates();
  double el = pulse.getEl();
  double az = pulse.getAz();
  
  // compute range info

  double startRange, theoreticalMaxRange, gateSpacing;
  _computeRangeInfo(startRange, theoreticalMaxRange, gateSpacing);
  double maxRange = startRange + nGates * gateSpacing;

  // form XML string

  xml = "";
  xml += TaXml::writeStartTag("rvp8Status", 0);
  xml += TaXml::writeBoolean("readFromTsApi", 1, true);
  xml += TaXml::writeString("siteName", 1, info.getSiteName());
  xml += TaXml::writeString
    ("majorMode", 1,
     Commands::majorMode2String(info.getMajorMode()));
  xml += TaXml::writeString
    ("polarization", 1,
     Commands::polarization2String(info.getPolarization()));
  xml += TaXml::writeString
    ("phaseCoding", 1,
     Commands::phaseCoding2String(info.getPhaseModSeq()));
  xml += TaXml::writeString
    ("prfMode", 1,
     Commands::prfMode2String(info.getUnfoldMode()));
  xml += TaXml::writeDouble("pulseWidthUs", 1, info.getPulseWidthUs());
  xml += TaXml::writeDouble("dbzCal1km", 1, info.getDbzCalibChan0());
  xml += TaXml::writeDouble("ifdClockMhz", 1, info.getClockMhz());
  xml += TaXml::writeDouble("wavelengthCm", 1, info.getWavelengthCm());
  xml += TaXml::writeDouble("satPowerDbm", 1, info.getSaturationDbm());
  xml += TaXml::writeDouble("rangeMaskResKm", 1, info.getRangeMaskRes() / 1000.0);
  xml += TaXml::writeDouble("startRangeKm", 1, startRange);
  xml += TaXml::writeDouble("maxRangeKm", 1, maxRange);
  xml += TaXml::writeDouble("gateSpacingKm", 1, gateSpacing);
  xml += TaXml::writeDouble("noiseChan0", 1, info.getNoiseDbmChan0());
  xml += TaXml::writeDouble("noiseChan1", 1, info.getNoiseDbmChan1());
  xml += TaXml::writeDouble("noiseSdevChan0", 1, info.getNoiseSdevChan0());
  xml += TaXml::writeDouble("noiseSdevChan1", 1, info.getNoiseSdevChan1());
  xml += TaXml::writeDouble("noiseRangeKm", 1, info.getNoiseRangeKm());
  xml += TaXml::writeDouble("noisePrfHz", 1, info.getNoisePrfHz());
  xml += TaXml::writeString("rdaVersion", 1, info.getVersionString());
  xml += TaXml::writeTime("time", 1, ptime);
  xml += TaXml::writeDouble("prf", 1, prf);
  xml += TaXml::writeInt("nGates", 1, nGates);
  xml += TaXml::writeDouble("el", 1, el);
  xml += TaXml::writeDouble("az", 1, az);
  xml += TaXml::writeEndTag("rvp8Status", 0);

  return 0;

}

///////////////////////////////////////////////////////////////
// compute range information from pulse info

void TsStatus::_computeRangeInfo(double &startRange,
				 double &maxRange,
				 double &gateSpacing)

{

  const TsInfo &info = _reader.getInfo();
  const int *iRangeMask = info.getRangeMask();

  // Find first, last, and total number of range bins in the mask
  // Based on SIGMET code in tsview.c

  int binCount = 0;
  int binStart = 0;
  int binEnd = 0;

  for (int ii = 0; ii < 512; ii++) {
    ui16 mask = iRangeMask[ii];
    if (mask) {
      for (int iBit = 0; iBit < 16; iBit++) {
        if (1 & (mask >> iBit)) {
          int iBin = iBit + (16*ii);
          if (binCount == 0) {
            binStart = iBin;
          }
          binEnd = iBin;
          binCount++;
        }
      }
    }
  }
  
  // range computations

  double fRangeMaskRes = info.getRangeMaskRes();
  startRange = (binStart * fRangeMaskRes) / 1000.0;
  maxRange = (binEnd * fRangeMaskRes) / 1000.0;
  gateSpacing = (maxRange - startRange) / (binCount - 1.0);

}
