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
// SigAirMet2Spdb.hh
//
// SigAirMet2Spdb object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#ifndef SigAirMet2Spdb_H
#define SigAirMet2Spdb_H

#include <string>
#include <vector>
#include <didss/DsInputPath.hh>
#include <rapformats/SigAirMet.hh>
#include <Spdb/StationLoc.hh>
#include <Spdb/FirLoc.hh>
#include "ReadStates.hh"
#include "Args.hh"
#include "Params.hh"
#include "GC_PolyTrunc.hh"
class DsSpdb;

using namespace std;

// enum typedefs
//   DECODE_SUCCESS = able to decode message; will save to output URL(s)
//   DECODE_FAILURE = unable to decode message; will not save to output URL(s)
//   DECODE_MSG_NO_SAVE = able to decode message; will not save to
//                        output URL(s), e.g., cancel messages which
//                        update a previous SPDB chunk
//   DECODE_TYPE_NO_WANT = able to find SIGMET or AIRMET (type) string but do not
//                         want this type of message; will not save to output
//                         URL(s). e.g., found AIRMET string but only want
//                         to save SIGMETs

typedef enum {
  DECODE_SUCCESS = 0,
  DECODE_FAILURE = -1,
  DECODE_MSG_NO_SAVE = 2,
  DECODE_TYPE_NO_WANT = 3
} decode_return_t;

// structure for holding header info for US AIRMETS, needs to be
// re-used for sub-AIRMETS

typedef struct {
  string msg_num;
  string header_type;
  bool is_update;
  string update_num;
  time_t start_time;
  time_t end_time;
  size_t sub_msg_counter;
} us_airmet_hdr_info_t;

////////////////////////
// This class

class SigAirMet2Spdb {
  
public:

  // constructor

  SigAirMet2Spdb (int argc, char **argv);

  // destructor
  
  ~SigAirMet2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  typedef struct {
    double lat0, lon0;
    double lat1, lon1;
    double dist, bearing;
  } line_segment_t;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  time_t _fileTime;
  SigAirMet _decoded;
  vector<string> _msgToks;
  vector<string> _directions;
  vector<bool> _used;
  size_t _startTokNum;
  size_t _lastTokUsed;
  bool _mayBeUS;
  bool _isUSAirmet;
  bool _isUSAirmetHeader;
  us_airmet_hdr_info_t _usAirmetHdrInfo;
  bool _tcFound;
  bool _centroidFromObsStation;

  bool _doCancel;
  string _cancelId;
  sigairmet_group_t _cancelGroup;

  // consts

  static const int _halfMonthSecs = 15 * 86400;
  static const int _halfYearSecs = 180 * 86400;

  // station location

  StationLoc _stationLoc;

  // FIR location

  FirLoc _firLoc;
  
  // State abbreviations

  ReadStates _readStates;

  // Input objects

  DsInputPath *_inputPath;

  // Functions to process the file and decode the message/report.

  int _processFile (const char *file_path);

  void _splitReport(const string &reportStr,
                    vector<string> &messages);

  int _splitOnMultipleTc(const string &reportStr,
                         vector<string> &messages);
  
  int _handleMessage(int messageNum,
                     const string &messageStr,
                     DsSpdb &asciiSpdb, DsSpdb &transSpdb);
  decode_return_t _decodeMsg(const string &reportStr);

  // Functions to Set() various fields in the object

  int _setId();
  int _setWx();
  int _getWxType(int startPos, string &wxType);
  void _setAmend();
  void _setReissue();
  void _setSource();
  int _setTimes();    
  void _setObsOrFcast();
  int _setObsTime(const string &timeStr);
  int _setFcastTime(const string &timeStr);
  void _setCentroidFromStationId();
  void _setFlightLevels();  
  bool _setFir();
  bool _setVertices();
  void _setMovement();
  void _setIntensity();
  void _setFcastPosition();
  void _setOutlookPosition();
  void _setText(const string &reportStr, const bool useReportString);
  bool _setCentroidFromFir();

  // Functions to check for strictness items. Only logged, not saved to object.

  bool _hasMWO(size_t &mwoIdx);
  void _hasFirCtaOrAircraft(const bool &foundFir,
			    const bool &foundMwo, const size_t &mwoIdx);
  void _hasValidWx();

  // Functions to handle CANCEL

  void _overwriteCancelled();

  // Functions to handle NIL message/report

  bool _isNil();
  
  // Functions to find vertices depending on type

  int _verticesFromLine(const size_t &startIdx,  const size_t &endIdx,
			const vector <string> &toks,
			vector <double> &lats, vector <double> &lons);
  int _verticesFromCenter(const size_t &startIdx, const size_t &endIdx,
			  const vector <string> &toks, 
			  const bool &needWidth, const bool &forceCenter,
			  vector <double> &lats, vector <double> &lons);
  int _verticesFromPolygon(const size_t &startIdx,  const size_t &endIdx,
			   const vector <string> &toks,
			   vector <double> &lats, vector <double> &lons);
  int _verticesFromFir(const size_t &startIdx, const size_t &endIdx,
		       const vector <string> &toks,
		       vector <double> &lats, vector <double> &lons);

  // Functions to handle time -- in Utils.cc

  int _computeIssueTime(const string &timeTok, time_t &issueTime);
  int _findStartEndTimes(time_t &startTime, time_t &endTime);
  int _computeStartEndTimes(const string &timeTok,
                            time_t &startTime, time_t &endTime);
  void _correctTimeForMonthChange(time_t &reportTime);
  void _correctTimeForFileTime(time_t &computedTime);

  // Functions to test whether a token is a specific type -- in Utils.cc

  bool _allAlpha(const string &tok);
  bool _allDigits(const string &tok);
  bool _hasDigit(const string &tok);
  bool _isStationBogus(const string &tok);
  bool _isWxTok(const string &tok);
  bool _isNil(const size_t startIdx,
	      const vector <string> toks);
  bool _isStateAbbr(const string &tok,
		    const string &prevTok,
		    const string &nextTok);
  bool _isLineTok(const string &tok);
  bool _isTimeTok(const string &tok, time_t &isTime);
  int _isFLTok(const string &tok, const string &tok2);
  bool _isDirTok(const string &tok);
  int _isSpeedTok(const string &tok);
  bool _isIntensityTok(const string &tok);
  bool _isDiameterTok(const string &tok, int &diam);
  bool _isMovementTok(const vector <string> &toks, 
		      const size_t &moveIdx,
		      vector <size_t> &moveToks,
		      string &moveType,
		      string &moveDir,
		      string &moveSpeed);
  bool _isStatMovTok(const string &tok, const string &tok2, 
		     bool &useTok2);
  bool _isFcastOutlookWxType();
  bool _isSinglePointWxType();
  bool _isFirTok(const string &tok);
  bool _isFirName(const vector <string> &toks, size_t &firIdx,
		  vector <size_t> &firToks, string &firName);
  bool _isIdTok(const string &tok,
		string &strippedTok);
  bool _skipUSHeader(const vector <string> toks,
		     const size_t useStartIdx,
		     const size_t useEndIdx,
		     size_t &wantStartIdx);
  bool _isLocationIndicator(const size_t &startIdx,
			    const size_t &endIdx,
			    const vector <string> &toks,
			    size_t &idx);

  // Functions for specific message types -- in Utils.cc

  bool _isTestMessage(size_t &startIdx,
		      const vector <string> &toks,
		      const int &minNumTest);
  bool _isAmendMessage(size_t &startIdx,
		       const vector <string> &toks,
		       string &id,
		       string &qualifier);
  void _checkCancel(const size_t &startIdx,
                    const vector <string> toks);
  bool _isReissueMessage(size_t &startIdx,
			 const vector <string> &toks,
			 string &id,
			 string &qualifier);
  bool _isSeeAlsoMessage(size_t &startIdx,
			 const vector <string> &toks,
			 string &id,
			 string &qualifier);

  // Functions to find specific values -- in Utils.cc

  bool _getMinMaxLatLon(const vector <double> &lats,
			const vector <double> &lons,
			double &minLat,
			double &maxLat,
			double &minLon,
			double &maxLon);

  // Functions to find flight levels -- in Utils.cc

  int _findFlightLevels(const vector <string> &toks, 
			const vector <bool> &usedToks,
			const size_t &startIdx,
			const size_t &endIdx,
			bool &foundTop,
			bool &foundBot,
			int &bot,
			int &top,
			vector <size_t> &usedIdxs);

  // Functions to find ID and qualifier -- in Utils.cc

  bool _findIdAndQualifier(const size_t &startIdx,
			   const vector <string> &toks,
			   string &id,
			   string &qualifier,
			   bool &foundId,
			   bool &foundQualifier,
			   vector <size_t> &usedIdxs);

  int _mergeIdAndQualifier(string &id,
                           const string &qualifier);

  // Functions to do conversions -- in Utils.cc

  bool _dir2Angle(const string &dir, double &angle);
  bool _dir2ReverseAngle(const string &dir, double &angle);
  bool _speed2Kph(const string &speed, double &speed_kph);
  void _latLon2XY(const double &lat, const double &lon,
		  double &x, double &y);
  void _XY2LatLon(const double &x, const double &y,
		  double &lat, double &lon);

  // Functions to find specific matches -- in Utils.cc

  bool _findNSEWchars(const string &tok,
		      bool &foundNS, size_t &nsPos,
		      bool &foundEW, size_t &ewPos);
  bool _findWidth(const size_t &startIdx, const size_t &endIdx,
		  const vector <string> &toks, int &width,
		  size_t &usedStartIdx, size_t &usedEndIdx);
  bool _findFrom(const size_t &startIdx,
		 const size_t &endIdx,
		 const vector <string> &toks,
		 size_t &fromIdx);
  bool _findMatchInToks(const string &tok,
			const size_t &startIdx,
			const size_t &endIdx,
			const vector <string> &toks,
			size_t &idxInToks);
  int _findStringNTimes(const string &tok,
			const string &wantstr);

  // Functions to do string manipulation -- in Utils.cc

  void _stripLeadingSpacer(const string &spacer,
			   const string &instr,
			   string &outstr);
  void _stripTrailingSpacer(const string &spacer,
			    const string &instr,
			    string &outstr);
  void _chopSpacer2End(const string &spacer,
		       const string &instr,
		       string &outstr);
  void _stripTermChars(const string &instr, string &outstr);
  bool _lastString(const string &tok, const string &wantstr);
  bool _firstString(const string &tok, const string &wantstr);
  int _isImbedDelimiter(size_t &startIdx, const vector <string> &toks,
			const string &delimiter);

  // Functions for (re)tokenizing -- in Utils.cc

  static void _tokenizeStr(const string &str, const string &spacer,
			   vector<string> &toks);
  static void _tokenize(const string &str, const string &spacer,
			vector<string> &toks);
  bool _replaceSpacerRetokenize(const string &spacer1,
				const string &spacer2,
				const string &spacer3,
				size_t &startTok,
				vector<string> &intoks,
				vector<string> &toks);

  // Functions for handling US AIRMETS -- in USAirmet.cc

  void _setUSAirmetID(const vector <string> toks,
                      const size_t useStartIdx);
  bool _isValidUSAirmetHeaderType(const string &type);
  bool _isValidUSAirmetType(const string &type);
  bool _isValidUSAirmetTypeForHeader(const string &header_type,
				     const string &type);
  bool _getUSAirmetUpdate(const vector <string> toks,
			  const size_t useStartIdx,
			  string &updateNum);
  void _assembleUSAirmetId(const string &updateNum,
			   const string &subMsgId,
			   const string &msgNum,
			   string &id);
  bool _disassembleUSAirmetId(const string &id,
			      string &updateNum,
			      string &subMsgId,
			      string &msgNum);
  bool _getUSAirmetSubMessageId(string &id);
  void _USAirmetCheckCancel();
  bool _findUSAirmetQualifier(const vector <string> toks,
			      const size_t useStartIdx,
			      const string type,
			      string &qualifier);
  bool _findUSAirmetType(const string &tok, 
			 const string &nextTok,
			 bool &is_header,
			 string &header_type,
			 string &type);

  // Functions for points -- in Points.cc

  int _findPoints(const vector<string> &toks,
		  const size_t &startIdx,
		  const size_t &endIdx,
		  const bool &skipIcaoStation,
		  const int &minPtsNeeded,
		  vector <double> &lats,
		  vector <double> &lons,
		  vector <bool> &sourceIsLatLon,
		  size_t &usedStartIdx, size_t &usedEndIdx);

  // Functions for polygons -- in Points.cc

  void _fixPolylinePoints(vector<double> &inlat, vector<double> &inlon,
                          vector<bool>&sourceIsLatLon, bool isClosed);

  bool _checkExceedsMaxLen(vector<double> &lats,
                           vector<double> &lons);
    
  int _removePtFarFromOthers(vector<double> &lats,
                             vector<double> &lons);

  int _removeLongSides(vector<double> &lats,
                       vector<double> &lons);
  
  int _removeSmallAngles(vector<double> &lats,
                         vector<double> &lons,
                         bool isClosed);

  bool _isValidPolygon(vector<double> &inlat, vector<double> &inlon);


  // Functions for stations -- in Points.cc

  bool _searchForStation(const bool &centroid_set,
			 const double &clat, const double &clon,
			 const string &station,
			 double &lat, double &lon, double &alt);
  bool _modifyStationPosition(const string &modloc,
			      double &inlat, double &inlon,
			      double &outlat, double &outlon);
  bool _searchForStationString(size_t &startIdx, vector<string> &toks,
			       double &lat, double &lon, size_t &endIdx);

  
  // Functions for lat-lon -- in Points.cc

  bool _hasLatLon(const string &tok, double &lat, double &lon);
  bool _hasLatOrLon(const string &tok, bool &isLat, double &value);
  bool _searchForLatLonString(size_t &startIdx, const vector<string> &toks,
			      string &latlonstr, size_t &endIdx);

  // Functions for FIRs -- in ModifyFirBoundary.cc

  bool _searchForModifyFir(const size_t &startIdx,
			   const size_t &endIdx,
			   const vector <string> &toks,
			   const vector <double> &firLats,
			   const vector <double> &firLons,
			   vector <double> &outlats,
			   vector <double> &outlons);
  bool _searchForModifyFirWith2PtsLineAndDir(const size_t &startIdx,
					     const size_t &endIdx,
					     const vector <string> &toks,
					     const vector <double> &firLats,
					     const vector <double> &firLons,
					     vector <double> &outlats,
					     vector <double> &outlons);
  bool _searchForModifyFirWithLatLonLineAndDir(const size_t &startIdx,
					       const size_t &endIdx,
					       const vector <string> &toks,
					       const vector <double> &firLats,
					       const vector <double> &firLons,
					       vector <double> &outlats,
					       vector <double> &outlons);
  size_t _buildLatLonVectorsFromGCPolyTrunc(GC_PolyTrunc &gc,
					    vector <double> &lats,
					    vector <double> &lons);
  bool _searchForDirAndLatOrLon(const size_t &startIdx,
				const size_t &endIdx,
				const vector <string> &toks,
				string &dir,
				bool &isLat,
				double &value,
				size_t &usedStartIdx,
				size_t &usedEndIdx);

  void _handleSfcWindAndVis(int start_pos, string &wx_type);

};

#endif

