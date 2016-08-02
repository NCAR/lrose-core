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

#include <stdio.h> // Use stdio not stream for file IO.
#include <Spdb/DsSpdb.hh>
#include <rapformats/station_reports.h>
#include <toolsa/umisc.h> 
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <list>
#include <cstdlib>
#include <physics/physics.h>
#include <dsserver/DsLdataInfo.hh>

#include "MosTotalTable.hh"
using namespace std;

/////////////////////////////////////////
//
// Constructor. Just makes copies of the input args.
//
MosTotalTable::MosTotalTable(time_t Start,
			     time_t Now,
			     time_t End,
			     string StationURL,
			     string ForecastURL,
			     string OutDir,
			     string TableArchiveDir,
			     string StationID,
			     float Calm,
			     int MaxLeadTime,
			     float MinVisibility,
			     float MaxVisibility,
			     float MaxCeiling,
			     bool MinimumLeadTimeOnly,
			     bool Overlap){

  _start = Start;
  _now = Now;
  _end = End;
  _stationURL = StationURL;
  _forecastURL = ForecastURL;
  _outDir = OutDir;
  _tableArchiveDir = TableArchiveDir;
  _stationID = StationID;
  _calm = Calm;
  _maxLeadTime = MaxLeadTime;
  _minVisibility = MinVisibility;
  _maxVisibility = MaxVisibility;
  _maxCeiling = MaxCeiling;
  _minimumLeadTimeOnly = MinimumLeadTimeOnly;
  _overlap = Overlap;


}
//////////////////////////////////////////
//
// Destructor. Does nothing.
//
MosTotalTable::~MosTotalTable(){


}
//////////////////////////////////////////
//
// Main routine. Writes the table.
//
int MosTotalTable::WriteTable(){

  const double Knots2MetersPerSecond = 1.9436346;
  //
  // Construe the output file name and open it.
  //
  date_time_t DataTime;
  DataTime.unix_time = _now;
  uconvert_from_utime( &DataTime );

  char TimeStr[32];
  sprintf(TimeStr,"_%02d%02d%02d",
	  DataTime.hour, DataTime.min, DataTime.sec);

  string TimeString( TimeStr );

  char SubDirStr[32];
  sprintf(SubDirStr,"%d%02d%02d",
	  DataTime.year, DataTime.month, DataTime.day);

  string SubDirString( SubDirStr );

  string PathDelim( PATH_DELIM );

  string FileName = _outDir + PathDelim + SubDirString + PathDelim + _stationID + TimeString + ".html";

  Path Dir(FileName);
  if (Dir.makeDirRecurse()){
    fprintf(stderr,"Failed to create directory for %s\n",
	    FileName.c_str());
    return -1;
  }

  FILE *fp;
  //
  // Open the output file.
  //
  fp = fopen(FileName.c_str(),"wt");
  if (fp == NULL){
    fprintf(stderr,"Failed to create %s\n",FileName.c_str());
    return -1;
  }
  //
  // Start generating HTML.
  //
  fprintf(fp,"<html>\n<head>\n<title> Weather data </title>\n</head>\n\n");
  fprintf(fp,"<body>\n<BODY BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" >\n\n");
  //
  // Put in a header stating where we are.
  //
  fprintf(fp,"<P>\n");
  fprintf(fp,"<B> Location : %s </B> <BR>\n", _stationID.c_str());
  fprintf(fp,"<P>\n");
  //
  // Open the database of METARS and see if we have anything
  // for this station.
  //
  DsSpdb Metars;
  int dataType = Spdb::hash4CharsToInt32( _stationID.c_str());

  if (Metars.getInterval(_stationURL, _start, _now,
			 dataType)){
    fprintf(stderr,"getInterval failed from %s\n",
	    _stationURL.c_str());

    fprintf(fp,"<P> getInterval failed from %s </P>\n",
	    _stationURL.c_str());


    fclose(fp);
    return -1;
  }


  //
  // If we have no METARS then say nothing, else generate table.
  //
  if (Metars.getNChunks() > 0){
    fprintf(fp,"<P>\n");
    
    fprintf(fp,"METAR Time History for station %s:\n", _stationID.c_str());
    
    // fprintf(fp,
    //	    "%d station reports found for station %s between %s and %s\n",
    //	    Metars.getNChunks(), _stationID.c_str(), utimstr(_start),
    //	    utimstr(_now));

    fprintf(fp,"<P>\n");    

    //
    // Get the non-forecast METARS and put them in a table.
    // Give table a header first.
    //
    fprintf(fp,"<P>\n<table border>\n");
    //
    fprintf(fp,"<tr><td> <pre> </pre> </td> \n");
    fprintf(fp,"    <td> <B>Time</B> </td> \n"); 
    fprintf(fp,"    <td> <B>Wind Direction</B> </td> \n");
    fprintf(fp,"    <td> <B>Wind Speed</B> </td> \n");
    fprintf(fp,"    <td> <B>Visibility</B> </td> \n");
    fprintf(fp,"    <td> <B>Ceiling</B> </td> \n");
    fprintf(fp,"    <td> <B>Temperature</B> </td> \n");
    fprintf(fp,"    <td> <B>Dew Point</B> </td> \n");
    fprintf(fp,"    <td> <B>Pressure</B> </td> \n"); 
    fprintf(fp,"</tr>\n\n");
    //
    // Add units.
    //
    fprintf(fp,"<tr><td> </td> \n");
    fprintf(fp,"    <td> <B>UTC</B> </td> \n"); 
    fprintf(fp,"    <td> <B>Degrees</B> </td> \n");
    fprintf(fp,"    <td> <B>Knots</B> </td> \n");
    fprintf(fp,"    <td> <B>Km</B> </td> \n");
    fprintf(fp,"    <td> <B>Feet</B> </td> \n");
    fprintf(fp,"    <td> <B>Degrees Deg C</B> </td> \n");
    fprintf(fp,"    <td> <B>Degrees Deg C</B> </td> \n");
    fprintf(fp,"    <td> <B>hPa</B> </td> \n"); 
    fprintf(fp,"</tr>\n\n");    
    //
    // Loop through the stations and put them in the table.
    //
    const vector<Spdb::chunk_t> &chunks = Metars.getChunks();  

    for (int ichunk = 0; ichunk < Metars.getNChunks(); ichunk++){

      station_report_t *S = (station_report_t *)chunks[ichunk].data;
      station_report_from_be( S );

      fprintf(fp,"<tr><td> </td> \n");
      //
      // Time.
      //
      fprintf(fp,"    <td> %s </td> ",utimstr(S->time)); 
      //
      // Wind direction and speed.
      //
      if ((S->windspd == STATION_NAN) ||(S->winddir == STATION_NAN))  {
	fprintf(fp,"    <td> Unknown </td> <td> Unknown </td> \n"); 
      } else {
	//
	float knots = S->windspd * Knots2MetersPerSecond;
	//
	// Print it if it exceeds the calm threshold.
	//
	if (knots < _calm){
	  fprintf(fp,"    <td> Calm </td> <td> Calm </td> \n"); 
	} else {
	  knots = 5.0*rint(knots/5.0); // Round to 5.0
	  float dir = 10.0*rint(S->winddir / 10.0); // Round to 10.0
	  fprintf(fp,"    <td> %.3d </td> <td> %.2d </td> ", int(dir), int(knots)); 
	}
      }
      //
      // Visibility.
      //  
      if (S->visibility <0.0) S->visibility = STATION_NAN;
      //
      if (S->visibility == STATION_NAN){
	fprintf(fp,"    <td> Unknown </td> \n"); 
      } else {
	
	bool VisOutsideThresholds = false;
	
	
	if (S->visibility > _maxVisibility){
	  fprintf(fp,"<td> &gt %g </td>\n", _maxVisibility);
	  VisOutsideThresholds = true;
	} 
	if (S->visibility < _minVisibility){
	  fprintf(fp,"<td> &lt %g </td>\n", _minVisibility);
	  VisOutsideThresholds = true;
	}
	
	if (
	    (!(VisOutsideThresholds))
	    ){
	  float printVal = 0.5*rint(S->visibility / 0.5);
	  fprintf(fp,"   <td> %g </td>\n",printVal);  
	}
      }
      //
      // Ceiling.
      //
      //
      if (S->ceiling < 0.0) S->ceiling = STATION_NAN;
      //
      const double Feet2Km = 0.0003048;
      if (S->ceiling == STATION_NAN){
	fprintf(fp,"    <td> Unknown </td> \n"); 
      } else {
	//
	// Make the comparison to MaxCeiling in Km but
	// print it out in feet.
	//
	if (S->ceiling > _maxCeiling * Feet2Km){
	  fprintf(fp, " <td> &gt %g  </td> \n",  
		  _maxCeiling);
	} else {
	  float printVal = S->ceiling / Feet2Km; // Ceiling in feet.
	  printVal = (float)(50.0*rint(printVal/50.0));
	  fprintf(fp, " <td> %8.0f  </td>\n", printVal); // Display in feet.
	}
      }
      //
      // Temperature, rounded to nearest int.
      //
      if (S->temp == STATION_NAN){
	fprintf(fp,"    <td> Unknown </td> \n"); 
      } else {
	fprintf(fp,"    <td> %d </td> ",int(rint(S->temp))); 
      }
      //
      // Relative humidity, rounded to nearest int.
      //
      if (
	  (S->temp == STATION_NAN) ||
	  (S->relhum == STATION_NAN)
	  ){
	fprintf(fp,"    <td> Unknown </td> \n"); 
      } else {

	double dewPoint = PHYrhdp(S->temp, S->relhum);
	fprintf(fp,"    <td> %d </td> ",int(rint(dewPoint))); 
      }
      //
      // Pressure, to nearest millibar.
      //
      if (S->pres == STATION_NAN){
	fprintf(fp,"    <td> Unknown </td> \n"); 
      } else {
	fprintf(fp,"    <td> %d </td> ",int(rint(S->pres))); 
      }
      fprintf(fp,"</tr>\n");
      //
      // End the non-forecast table.
      //
    }
    fprintf(fp,"</P>\n</table>\n\n");    
    
  } // End of if we have non-forecast data to put in a table.



  ////////////////////////////////////////////////////////////////
  //
  // And now, do it all again for the forecast stations. The code is
  // just different enough to make it worth duplicating.
  //
  ////////////////////////////////////////////////////////////////
  //
  // Open the database of forecast METARS and see if we have anything
  // for this station.
  //
  DsSpdb Forecasts;

  int iret;

  if (_overlap){
    //
    // For debugging, request the same time period as
    // for the actual METARS.
    //
    _end = _now; _now = _start; 
  }


  iret = Forecasts.getInterval(_forecastURL, _now, _end,
			       dataType);

  if (iret){
    fprintf(stderr,"getInterval failed from %s\n",
	    _forecastURL.c_str());

    fprintf(fp,"<P> getInterval failed from %s </P>\n",
	    _forecastURL.c_str());


    fclose(fp);
    return -1;
  }
  //
  // Push back the entries into a list if the lead time is short enough,
  // and if it is in the interval we want.
  //
  const vector<Spdb::chunk_t> &Chunks = Forecasts.getChunks();  
  int NumIn = 0;

  list<MosTotalTable::entry_t> UnprocessedEntries;

  for (int ichunk = 0; ichunk < Forecasts.getNChunks(); ichunk++){
    if (Chunks[ichunk].data_type2 <= _maxLeadTime) {
      NumIn++;
      MosTotalTable::entry_t E;
      E.dataTime = Chunks[ichunk].valid_time;
      E.dataType = Chunks[ichunk].data_type;
      E.dataType2= Chunks[ichunk].data_type2;

      time_t dataTime = E.dataTime + E.dataType2;
      if (
	  (dataTime >= _now) &&
	  (dataTime <= _end)
	  ){
	UnprocessedEntries.push_back( E );
      }
    }
  }

  
  //fprintf(stderr,"%d METARS found between %s and %s with lead time less than %d\n",
  //	  NumIn, utimstr(_now), utimstr(_end), _maxLeadTime);

  //
  // If requested (it usually will be) make sure that for each
  // data time, only the minimum lead time is displayed.
  //   

  //list<MosTotalTable::entry_t>::iterator q;
  //fprintf(stderr,"Getting rid of duplicate entries.\nBEFORE:\n");
  //for( q=UnprocessedEntries.begin(); q != UnprocessedEntries.end(); q++ ) {
  //  fprintf(stderr,"Data at %s\n",utimstr(q->dataTime));
  //}

  if (_minimumLeadTimeOnly){
    list<MosTotalTable::entry_t>::iterator i,j, OneBeforeEnd; 
    OneBeforeEnd =  UnprocessedEntries.end(); OneBeforeEnd --;

 
    for( i=UnprocessedEntries.begin(); i != OneBeforeEnd; i++ ) {
      for( j=i, j++; j != UnprocessedEntries.end(); j++ ) {

	if (i->dataTime == j->dataTime){ 
	  //
	  // Duplicate entry, delete the one with
	  // the greater lead time.
	  //
	  NumIn--;
	  if (j->dataType2 <= i->dataType2){
	    i=UnprocessedEntries.erase(i); i--;
	    j=i;  j++;
	  } else {
	    j=UnprocessedEntries.erase(j);
	    i=j; i--;
	  }
	}
	OneBeforeEnd =  UnprocessedEntries.end(); OneBeforeEnd --;
      }
    }
  }

  //fprintf(stderr,"Getting rid of duplicate entries.\nAFTER:\n");
  //for( q=UnprocessedEntries.begin(); q != UnprocessedEntries.end(); q++ ) {
  //  fprintf(stderr,"Data at %s\n",utimstr(q->dataTime));
  //}

  time_t latestValidTime = time(NULL);
  int latestLeadTime = 0;

  if (NumIn > 0){
    //
    // If we have no Forecasts then say nothing, else generate table.
    //
    fprintf(fp,"<P>\n");
    //
    // fprintf(fp,
    //	    "%d forecasts found at station location %s between %s and %s\n",
    //	    NumIn, _stationID.c_str(), utimstr(_now+1),
    //	    utimstr(_end));
    //
    fprintf(fp,"MOS Forecast Time History for station %s:\n",
	    _stationID.c_str());
    //
    //
    fprintf(fp,"<P>\n");    
    //
    // Get the non-forecast Forecasts and put them in a table.
    // Give table a header first.
    //
    fprintf(fp,"<P>\n<table border>\n");
    //
    fprintf(fp,"<tr><td> <pre> </pre> </td> \n");
    fprintf(fp,"    <td> <B>Time</B> </td> \n"); 
    fprintf(fp,"    <td> <B>Wind Direction</B> </td> \n");
    fprintf(fp,"    <td> <B>Wind Speed</B> </td> \n");
    fprintf(fp,"    <td> <B>Visibility</B> </td> \n");
    fprintf(fp,"    <td> <B>Ceiling</B> </td> \n");
    fprintf(fp,"    <td> <B>Temperature</B> </td> \n");
    fprintf(fp,"    <td> <B>Dew Point</B> </td> \n");
    fprintf(fp,"    <td> <B>Pressure</B> </td> \n"); 
    fprintf(fp,"    <td> <B>Model Run Time</B> </td> \n"); 
    if (!(_minimumLeadTimeOnly)){
      fprintf(fp,"    <td> <B>Lead Time</B> </td> \n"); 
    }
    fprintf(fp,"</tr>\n");
    //
    // Add units.
    //
    fprintf(fp,"<tr><td> </td> \n");
    fprintf(fp,"    <td> <B>UTC</B> </td> \n"); 
    fprintf(fp,"    <td> <B>Degrees</B> </td> \n");
    fprintf(fp,"    <td> <B>Knots</B> </td> \n");
    fprintf(fp,"    <td> <B>Km</B> </td> \n");
    fprintf(fp,"    <td> <B>Feet</B> </td> \n");
    fprintf(fp,"    <td> <B>Degrees Deg C</B> </td> \n");
    fprintf(fp,"    <td> <B>Degrees Deg C</B> </td> \n");
    fprintf(fp,"    <td> <B>hPa</B> </td> \n"); 
    fprintf(fp,"    <td> <B>UTC</B> </td> \n"); 
    if (!(_minimumLeadTimeOnly)){
      fprintf(fp,"    <td> <B>HH:MM</B> </td> \n"); 
    }

    fprintf(fp,"</tr>\n");    
    //
    // Loop through the stations and put them in the table.
    //

    list<MosTotalTable::entry_t>::iterator i; 

    for( i=UnprocessedEntries.begin(); i != UnprocessedEntries.end(); i++ ) {
      //
      // See if we can read in a single chunk.
      // Since we checked what chunks were there before we
      // should be able to - if not, fail and exit.
      //
      DsSpdb singleForecast;

      singleForecast.getExact(_forecastURL, i->dataTime,
			      i->dataType, i->dataType2);

      if (singleForecast.getNChunks() == 0){
	fprintf(stderr,"Get exact failed - exiting.\n");
	exit(-1);
      }

      const vector<Spdb::chunk_t> &chunks = singleForecast.getChunks();

      station_report_t *S = (station_report_t *)chunks[0].data;
      station_report_from_be( S );

      int LeadTime = chunks[0].data_type2;

      if (LeadTime <= _maxLeadTime){
	int LeadHour = (int)floor(LeadTime / 3600.0);
	int LeadMinute = (LeadTime - 3600*LeadHour)/60;

	fprintf(fp,"<tr><td> </td> \n");
	//
	// Valid Time.
	//

	//fprintf(stderr,"TIME : %s Lead time : %d VALID : %s\n",
	//	utimstr(S->time), LeadTime, utimstr(S->time + LeadTime));

	fprintf(fp,"    <td> %s </td> ",utimstr(S->time)); 
        latestValidTime = S->time;
        latestLeadTime = LeadTime;

	//
	// Wind direction and speed.
	//
	if ((S->windspd == STATION_NAN) ||(S->winddir == STATION_NAN))  {
	  fprintf(fp,"    <td> Unknown </td> <td> Unknown </td> \n"); 
	} else {
	  //
	  float knots = S->windspd * Knots2MetersPerSecond;
	  //
	  // Print it if it exceeds the calm threshold.
	  //
	  if (knots < _calm){
	    fprintf(fp,"    <td> Calm </td> <td> Calm </td> \n"); 
	  } else {
	    knots = 5.0*rint(knots/5.0); // Round to 5.0
	    float dir = 10.0*rint(S->winddir / 10.0); // Round to 10.0
	    fprintf(fp,"    <td> %.3d </td> <td> %.2d </td> ", int(dir), int(knots)); 
	  }
	}
	//
	// Visibility.
	//  
	if (S->visibility < 0.0) S->visibility = STATION_NAN;
	//
	if (S->visibility == STATION_NAN){
	  fprintf(fp,"    <td> Unknown </td> \n"); 
	} else {
	  
	  bool VisOutsideThresholds = false;
	
	
	  if (S->visibility > _maxVisibility){
	    fprintf(fp,"<td> &gt %g </td>\n", _maxVisibility);
	    VisOutsideThresholds = true;
	  } 
	  if (S->visibility < _minVisibility){
	    fprintf(fp,"<td> &lt %g </td>\n", _minVisibility);
	    VisOutsideThresholds = true;
	  }
	  
	  if (
	      (!(VisOutsideThresholds))
	      ){
	    float printVal = 0.5*rint(S->visibility / 0.5);
	    fprintf(fp,"   <td> %g </td>\n",printVal);  
	  }
	}
	//
	// Ceiling.
	//
	//
	const double Feet2Km = 0.0003048;
	//
	if (S->ceiling < 0.0) S->ceiling = STATION_NAN;
	//
	if (S->ceiling == STATION_NAN){
	  fprintf(fp,"    <td> Unknown </td> \n"); 
	} else {
	  //
	  // Make the comparison to MaxCeiling in Km but
	  // print it out in feet.
	  //
	  if (S->ceiling > _maxCeiling * Feet2Km){
	    fprintf(fp, " <td> &gt %g  </td> \n",  
		    _maxCeiling);
	  } else {
	    float printVal = S->ceiling / Feet2Km; // Ceiling in feet.
	    printVal = (float)(50.0*rint(printVal/50.0));
	    fprintf(fp, " <td> %8.0f  </td>\n", printVal); // Display in feet.
	  }
	}
	//
	// Temperature, rounded to nearest int.
	//
	if (S->temp == STATION_NAN){
	  fprintf(fp,"    <td> Unknown </td> \n"); 
	} else {
	  fprintf(fp,"    <td> %d </td> ",int(rint(S->temp))); 
	}
	//
	// Relative humidity, rounded to nearest int.
	//
	if (
	    (S->temp == STATION_NAN) ||
	    (S->relhum == STATION_NAN)
	    ){
	  fprintf(fp,"    <td> Unknown </td> \n"); 
	} else {
	  
	  double dewPoint = PHYrhdp(S->temp, S->relhum);
	  fprintf(fp,"    <td> %d </td> ",int(rint(dewPoint))); 
	}
	//
	// Pressure, to nearest millibar.
	//
	if (S->pres == STATION_NAN){
	  fprintf(fp,"    <td> Unknown </td> \n"); 
	} else {
	  fprintf(fp,"    <td> %d </td> ",int(rint(S->pres))); 
	}
	//
	// Generation Time. Stored as a user defined field in the
	// MOS stuff.
	//
	fprintf(fp,"    <td> %s </td> ",utimstr(S->shared.user_data.int1)); 
	//
	if (!(_minimumLeadTimeOnly)){
	  //
	  // Lead Time.
	  //
	  fprintf(fp,"    <td> %02d:%02d </td> ",
		  LeadHour, LeadMinute); 
	}

	fprintf(fp,"</tr>\n\n");
      }
    }
    //
    // End the forecast table.
    //
    fprintf(fp,"</P>\n</table>\n\n\n");    

  } // End of if we have forecast data to put in a table.

  //
  // Finish up and close HTML file.
  //
  time_t now = time(NULL);

  fprintf(fp,"<P> <pre>   </pre> Generated at %s UTC </P>\n",
          utimstr(now));
 
  fprintf(fp,"<P> <pre>   </pre> Based on station report at %s UTC </P>\n",
          utimstr(_now));


 
  fprintf(fp,"<HR>\n");
  fprintf(fp,"</body>\n</html>\n\n");
                                            
  fclose( fp );

  // Write latest data info file.

  string relPath;
  Path::stripDir(_outDir, FileName, relPath);
  DsLdataInfo ldata(_outDir);
  ldata.setRelDataPath(relPath);
  ldata.setDataFileExt("html");
  ldata.setIsFcast(true);
  ldata.setLeadTime(latestLeadTime);
  ldata.setWriter("MosSpdb2Html");
  ldata.setDataType("www");
  if (ldata.write(latestValidTime - latestLeadTime)) {
    cerr << "ERROR  writing table LdataInfo file to : " << _outDir << endl;
  }

  //
  // Copy the file to the archival directory. The filename just uses
  // the file generation time to be unique.
  //
  date_time_t Now;
  Now.unix_time = time(NULL);
  uconvert_from_utime( &Now );

  char NowDate[32];
  sprintf(NowDate,"%d%02d%02d",
	  Now.year, Now.month, Now.day);

  char NowTime[32];
  sprintf(NowTime,"%0d%02d%02d",
	  Now.hour, Now.min, Now.sec);

  string NowDateStr( NowDate );
  string NowTimeStr( NowTime );
  //
  // Somewhat long winded archive name to ensure uniqueness.
  //
  string ArchiveName = _tableArchiveDir + PathDelim + NowDateStr + 
    PathDelim + _stationID + "_" + NowTimeStr +
    TimeString + "_" + SubDirString + ".html";

  Path archiveDir(ArchiveName);
  if (archiveDir.makeDirRecurse()){
    fprintf(stderr,"Failed to create directory for %s\n",
	    ArchiveName.c_str());
    return -1;
  }
  //
  // Copy this into place.
  //

  if (filecopy_by_name(ArchiveName.c_str(), FileName.c_str())) {
    cerr << "ERROR - MosTotalTable" << endl;
    cerr << "Cannot copy file to archive dir" << endl;
    return -1;
  }

  // Write archive latest data info file.

  string archiveRelPath;
  Path::stripDir(_tableArchiveDir, ArchiveName, archiveRelPath);
  DsLdataInfo archiveLdata(_tableArchiveDir);
  archiveLdata.setRelDataPath(archiveRelPath);
  archiveLdata.setDataFileExt("html");
  archiveLdata.setIsFcast(true);
  archiveLdata.setLeadTime(latestLeadTime);
  archiveLdata.setWriter("MosSpdb2Html");
  archiveLdata.setDataType("www");
  if (archiveLdata.write(latestValidTime - latestLeadTime)) {
    cerr << "ERROR  writing archive table LdataInfo file to : "
         << _tableArchiveDir << endl;
  }

  return 0;

}

