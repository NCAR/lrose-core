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

#ifndef _GINIPDB_INC_
#define _GINIPDB_INC_

// C++ Standard Include Files
#include <cstdio>
#include <string>

// No RAP Include Files

// No Local Include Files

using namespace std;

// No Forward Declarations

const int GiniMercatorProj = 1;
const int GiniLambertProj = 3;
const int GiniPolarStereoProj = 5;

// Class Declaration
class GiniPDB
   {
   public:
      ////////////////////////////
      // NO PUBLIC DATA MEMBERS //
      ////////////////////////////

      ////////////////////
      // PUBLIC METHODS //
      ////////////////////

      // Constructors and Destructors
      GiniPDB();
      virtual ~GiniPDB();

      // Set Methods
      void setSource(string source)                 { _source = source; }
      void setCreator(string creator)               { _creator = creator; }
      void setSector(string sector)                 { _sector = sector; }
      void setChannel(string channel)               { _channel = channel; }
      void setNumRecords(int numRecords)            { _numRecords = numRecords; }
      void setSizeRecords(int sizeRecords)          { _sizeRecords = sizeRecords; }
      void setYear(int year)                        { _year = year; }
      void setMonth(int month)                      { _month = month; }
      void setDay(int day)                          { _day = day; }
      void setHour(int hour)                        { _hour = hour; }
      void setMin(int min)                          { _min = min; }
      void setSec(int sec)                          { _sec = sec; }
      void setHSec(int hSec)                        { _hSec = hSec; }
      void setProjection(int projection)            { _projection = projection; }
      void setNx(int nx)                            { _nx = nx; }
      void setNy(int ny)                            { _ny = ny; }
      void setLat1(double lat1)                     { _lat1 = lat1; }
      void setLon1(double lon1)                     { _lon1 = lon1; }
      void setLat2(double lat2)                     { _lat2 = lat2; }
      void setLon2(double lon2)                     { _lon2 = lon2; }
      void setLov(double lov)                       { _lov  = lov; }
      void setDx(double dx)                         { _dx = dx; }
      void setDy(double dy)                         { _dy = dy; }
      void setDi(double di)                         { _di = di; }
      void setDj(double dj)                         { _dj = dj; }
      void setResFlag(int resFlag)                  { _resFlag = resFlag;   }
      void setSouthPole(int southPole)              { _southPole = southPole; }
      void setScanLeftToRight(bool scanLeftToRight) { _scanLeftToRight = scanLeftToRight; }
      void setScanTopToBottom(bool scanTopToBottom) { _scanTopToBottom = scanTopToBottom; }
      void setScanXInFirst(bool scanXInFirst)       { _scanXInFirst = scanXInFirst; }
      void setLatin(double latin)                   { _latin = latin; }
      void setImageRes(int imageRes)                { _imageRes = imageRes; }
      void setCompression(int compression)          { _compression = compression; }
      void setCreatorVersion(int creatorVersion)    { _creatorVersion = creatorVersion; }
      void setPDBSize(int pdbSize)                  { _pdbSize = pdbSize; }
      void setNavIncluded(bool navIncluded)         { _navIncluded = navIncluded; }
      void setCalIncluded(bool calIncluded)         { _calIncluded = calIncluded; }

      // Get Methods
      string getSource()        { return _source; }
      string getCreator()       { return _creator; }
      string getSector()        { return _sector; }
      string getChannel()       { return _channel; }
      int getNumRecords()       { return _numRecords; }
      int getSizeRecords()      { return _sizeRecords; }
      int getYear()             { return _year; }
      int getMonth()            { return _month; }
      int getDay()              { return _day; }
      int getHour()             { return _hour; }
      int getMin()              { return _min; }
      int getSec()              { return _sec; }
      int getHSec()             { return _hSec; }
      int getProjection()       { return _projection; }
      int getNx()               { return _nx; }
      int getNy()               { return _ny; }
      double getLat1()          { return _lat1; }
      double getLon1()          { return _lon1; }
      double getLat2()          { return _lat2; }
      double getLon2()          { return _lon2; }
      double getLov()           { return _lov; }
      double getDx()            { return _dx; }
      double getDy()            { return _dy; }
      double getDi()            { return _di; }
      double getDj()            { return _dj; }
      int getResFlag()          { return _resFlag; }
      int getSouthPole()        { return _southPole; }
      bool getScanLeftToRight() { return _scanLeftToRight; }
      bool getScanTopToBottom() { return _scanTopToBottom; }
      bool getScanXInFirst()    { return _scanXInFirst; }
      double getLatin()         { return _latin; }
      int getImageRes()         { return _imageRes; }
      int getCompression()      { return _compression; }
      int getCreatorVersion()   { return _creatorVersion; }
      int getPDBSize()          { return _pdbSize; }
      bool getNavIncluded()     { return _navIncluded; }
      bool getCalIncluded()     { return _calIncluded; }

      // General Methods
      void decodeGiniPDB(unsigned char pdb[512]);

   protected:
      ///////////////////////////////
      // NO PROTECTED DATA MEMBERS //
      ///////////////////////////////

      //////////////////////////
      // NO PROTECTED METHODS //
      //////////////////////////

   private:
      //////////////////////////
      // PRIVATE DATA MEMBERS //
      //////////////////////////
      string _source;
      string _creator;
      string _sector;
      string _channel;
      int _numRecords;
      int _sizeRecords;
      int _year;
      int _month;
      int _day;
      int _hour;
      int _min;
      int _sec;
      int _hSec;
      int _projection;
      int _nx;
      int _ny;
      double _lat1;
      double _lon1;
      double _lat2; 
      double _lon2;
      double _lov;
      double _dx;
      double _dy;
      double _di;
      double _dj;
      int _resFlag;
      int _southPole;
      bool _scanLeftToRight;
      bool _scanTopToBottom;
      bool _scanXInFirst;
      double _latin;
      int _imageRes;
      int _compression;
      int _creatorVersion;
      int _pdbSize;
      bool _navIncluded;
      bool _calIncluded;

      ////////////////////////
      // NO PRIVATE METHODS //
      ////////////////////////
   }; // End of GiniPDB class declaration
#endif    
