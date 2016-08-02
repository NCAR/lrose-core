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

#include "Process.hh"
#include "ConstrueFilename.hh"
#include "MosTotalTable.hh"

#include <Spdb/DsSpdb.hh>
#include <rapformats/station_reports.h>
#include <physics/physics.h>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <dsserver/DmapAccess.hh>

#include <dsserver/DsLdataInfo.hh>
 
#include <cstdlib>  
#include <cstdio>
using namespace std;
//
// Routine that actually generates output HTML.
//
void GenOutput( const station_report_t *report, const Params *P,
		const int dataType, const int dataType2);
//
// Small routine to QC metars from parameters and common sense.
//
void QCmetar(Params *P, station_report_t *metar);
//
// Main routine.
//
void Process(Params *P, vector<SpdbTrigger::entry_t> &UnprocessedEntries){

  DsSpdb InSpdb;

  if (P->Debug){
    time_t now = time(NULL);
    cerr << "Processing at time: " << DateTime::strm(now) << endl;
  }

  vector<SpdbTrigger::entry_t>::iterator i;
  for( i=UnprocessedEntries.begin(); i != UnprocessedEntries.end(); i++ ) {

    //
    // Get the SPDB station report data
    //

    if (P->Debug){
      cerr << "Getting data at " << utimstr(i->dataTime);
      cerr << " types " << Spdb::dehashInt32To4Chars(i->dataType);
      cerr << " " << i->dataType2;
      cerr << " from " << P->TriggerUrl << " ...";
    }
    InSpdb.getExact(P->TriggerUrl,
		    i->dataTime,
		    i->dataType,
		    i->dataType2);

    if (P->Debug) cerr << " done." << endl;

    //
    // Disassemble it into a station report.
    //
    station_report_t *report;
    report = (station_report_t *) InSpdb.getChunks()[0].data;
    station_report_from_be( report );
    //
    // Apply Quality Control to the metar.
    //
    QCmetar(P, report);
    //
    // Write the resulting HTML files.
    //

    GenOutput(report, P,
	      i->dataType, i->dataType2);
    //
    // Write the Trend table based on this time.
    //
    MosTotalTable M(report->time - P->TableLookBack,
		    report->time,
		    report->time + P->TableLookAhead,
		    P->MetarUrl,
		    P->TriggerUrl,
		    P->TableOutDir,
		    P->TableArchiveDir,
		    Spdb::dehashInt32To4Chars(i->dataType),
		    P->CalmWindThreshold,
		    P->TableMaxLeadTime,
		    P->MinVisibility,
		    P->MaxVisibility,
		    P->MaxCeiling,
		    P->MinimumLeadTimeOnly,
		    P->Overlap);

    if (M.WriteTable()){
      cerr << "Failed to write table for " << Spdb::dehashInt32To4Chars(i->dataType);
      cerr << " at " << utimstr(report->time) << endl;
    }
    

  } // End of loop through input data.


}

void GenOutput( const station_report_t *report, const Params *P,
		const int dataType, const int dataType2){

  string htmlFile;
  
  ConstrueFilename(report->time, dataType2, dataType,
		   P->OutDir, false, htmlFile);       

  if (P->Debug) {
    cerr << "Getting ready to open realtime file: " << htmlFile << endl;
  }

  //
  // Make sure the dir exists.
  //

  Path path(htmlFile);
  if (path.makeDirRecurse()){
    cerr << "Failed to create directory for " << htmlFile << endl;
    return;
  }

  FILE *fp = fopen(htmlFile.c_str(),"wt");
  if (fp == NULL){
    cerr << "Failed to create " << htmlFile << endl;
    return;
  }


  fprintf(fp,"<html>\n<head>\n<title> Weather data </title>\n</head>\n\n");
  fprintf(fp,"<body>\n<BODY BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" >\n\n");

  //
  // Put in a header stating where we are.
  //
  fprintf(fp,"<P>\n");
  string stationID = Spdb::dehashInt32To4Chars(dataType);

  fprintf(fp,"<B> Location : %s </B> <BR>\n",
	  stationID.c_str());

  fprintf(fp,"<B> Valid for %s UTC </B> <BR> \n",
	  utimstr(report->time));

  fprintf(fp,"<B> Model run time %s UTC </B> <BR> \n",
	  utimstr(report->shared.user_data.int1));

  fprintf(fp,"</P>\n\n");


  //
  // Put the table of data in. Have an empty colum to space things out.
  //
  fprintf(fp,"<P>\n<table>\n");

  fprintf(fp,
	  "<tr><td> <pre> </pre> <td> <B>Parameter</B> </td> <td> <B>Value</B> </td> </tr>\n");

  char Ret[32]; // Buffer string for general use.

  //
  //
  // Wind speed needs to be converted from m/s to knots,
  // then rounded to the nearest 5 knots. If it is less than
  // the calm wind value, just say calm and don't print
  // wind direction.
  //
  //
  // See if we have a measurement.
  //
  if (
      (report->windspd == STATION_NAN) ||
      (report->winddir == STATION_NAN)
      ){
    sprintf(Ret,"%s","Unknown"); // Not likely since it is easy to measure.
  } else {
    //
    // We have a measurement - is it below the calm value
    // for the wind?
    //
    const double Knots2MetersPerSecond = 1.9436346;
    float knots = report->windspd * Knots2MetersPerSecond;
    if (knots < P->CalmWindThreshold) {
      //
      // It's calm
      //
      sprintf(Ret,"%s","Calm");
    } else {
      //
      // Not calm - print the value.
      //
      knots = 5.0*rint(knots/5.0); // Round to 5.0
      float dir = 10.0*rint(report->winddir / 10.0); // Round to 10.0
      sprintf(Ret,"%d/%d degrees/knots",int(dir),int(knots));

    }
  }
 
  fprintf(fp,"<tr> <td> </td> <td> Wind direction/speed </td> <td> %s </td> </tr>\n\n",Ret);
  

  //
  // See if we have to replace the vis value with a string.
  //
  bool ReplacedVisibility = false;
  bool VisOutsideThresholds = false;

  if (report->visibility != STATION_NAN){
    if (report->visibility > P->MaxVisibility){
      sprintf(Ret," &gt %g Km", P->MaxVisibility);
      VisOutsideThresholds = true;
    } 
    if (report->visibility < P->MinVisibility){
      sprintf(Ret," &lt %g Km", P->MinVisibility);
      VisOutsideThresholds = true;
    } 


    if (!(VisOutsideThresholds)){
      for (int k = 0; k < P->VisibilityReplaceValues_n; k++){
	if (P->_VisibilityReplaceValues[k] == report->visibility){
	  ReplacedVisibility = true;
	  sprintf(Ret,"%s",P->_VisibilityReplaceStrings[k]);
	}
      }
    }
    //
    // If not, just take the value. Round it to 0.5Km
    //
    if (
	(!(ReplacedVisibility)) &&
	(!(VisOutsideThresholds))
	){
      float printVal = 0.5*rint(report->visibility / 0.5);
      sprintf(Ret,"%g Km", printVal);
    }
  } else {
    sprintf(Ret,"%s","Unknown");
  }

  fprintf(fp,"<tr> <td> </td> <td> Visibility </td> <td> %s </td> </tr>\n\n",Ret);

  const double Feet2Km = 0.0003048;	 

  if (report->ceiling == STATION_NAN){
    sprintf(Ret, "%s", "Unknown");
  } else {
    //
    // Make the comparison to MaxCeiling in Km but
    // print it out in feet.
    //
    if (report->ceiling > P->MaxCeiling * Feet2Km){
      sprintf(Ret, " &gt %g feet\n",  
	      P->MaxCeiling);
    } else {
      //
      // See if we have to replace this value with a string.
      //
      bool ReplacedCeiling = false;
      for (int k = 0; k < P->CeilingReplaceValues_n; k++){
	if (P->_CeilingReplaceValues[k] == report->ceiling){
	  ReplacedCeiling = true;
	  sprintf(Ret,"%s",P->_CeilingReplaceStrings[k]);
	}
      }
      //
      // If not, just print the value.
      // Round to nearest 50 feet in this case.
      //
      if (!(ReplacedCeiling)){
	float printVal = report->ceiling / Feet2Km; // Ceiling in feet.
	printVal = (float)(50.0*rint(printVal/50.0));
	sprintf(Ret, "%8.0f feet", printVal); // Display in feet.
      }
    }
  }
  fprintf(fp,"<tr> <td> </td> <td> Ceiling </td> <td> %s </td> </tr>\n\n", Ret);
	  
  //
  // Round temperature to the nearest degree.
  //
  if (report->temp == STATION_NAN){
    sprintf(Ret,"%s","Unknown");
  } else {
    sprintf(Ret,"%g C", rint(report->temp));
  }
  fprintf(fp,"<tr> <td> </td> <td> Temperature </td> <td> %s </td> </tr>\n\n", Ret);
  //
  // Round dew point to the nearest integer.
  //
  if (
      (report->relhum == STATION_NAN) ||
      (report->temp == STATION_NAN)
      ){
    sprintf(Ret,"%s","Unknown");
  } else {
    double dewPoint = PHYrhdp(report->temp, report->relhum);  
    sprintf(Ret,"%g C", rint(dewPoint));
  }
  fprintf(fp,"<tr> <td> </td> <td> Dew Point </td> <td> %s </td> </tr>\n\n",Ret);	  
  //
  // Pressure, rounded to the nearset milibar.
  //
  if (report->pres == STATION_NAN){
    sprintf(Ret,"%s","Unknown");
  } else {
    sprintf(Ret,"%d hPa", (int)rint(report->pres));
  }
  fprintf(fp,"<tr> <td> </td> <td> Pressure </td> <td> %s </td> </tr>\n\n",Ret);
  //
  // End the HTML table.
  //
  fprintf(fp,"</P>\n</table>\n\n");
  //
  // Put the generation time in there.
  //
  time_t Now = time(NULL);
  fprintf(fp,"<P> <pre>   </pre> Generated at %s UTC </P>\n",
	  utimstr(Now));
  
  fprintf(fp,"<HR>\n");

  fprintf(fp,"</body>\n</html>\n\n");

  fclose(fp);

  if (P->Debug) {
    cerr << "Closed realtime file: " << htmlFile << endl;
  }

  // Write latest data info file.

  string relPath;
  Path::stripDir(P->OutDir, htmlFile, relPath);

  DsLdataInfo ldata(P->OutDir);
  ldata.setRelDataPath(relPath);
  ldata.setDataFileExt("html");
  ldata.setIsFcast(true);
  int leadTime = dataType2;
  ldata.setLeadTime(leadTime);
  ldata.setWriter("MosSpdb2Html");
  ldata.setDataType("www");
 
  if (ldata.write(report->time - leadTime)) { // Was generateTime.
    cerr << "ERROR  writing  LdataInfo file to : " << P->OutDir << endl;
  }

  // Copy this file to the non-archive dir if required

  if (P->CopyToArchiveDir) {

    string archiveFile;
    
    ConstrueFilename(report->time, dataType2, dataType,
                     P->ArchiveDir, true, archiveFile);       
    
    Path Out(archiveFile);
    if (Out.makeDirRecurse()){
      cerr << "Failed to create directory for " << archiveFile << endl;
      return;
    }

    if (P->Debug) {
      cerr << "Copying to archive file" << endl;
      cerr << "  Realtime file: " << htmlFile << endl;
      cerr << "  Archive file: " << archiveFile << endl;
    }

    if (filecopy_by_name(archiveFile.c_str(), htmlFile.c_str())) {
      cerr << "ERROR - Process" << endl;
      cerr << "Cannot copy file to archive dir" << endl;
      return;
    }

    // Write latest archive data info file.
    
    string archiveRelPath;
    Path::stripDir(P->ArchiveDir, archiveFile, archiveRelPath);
    
    DsLdataInfo archiveLdata(P->ArchiveDir);
    archiveLdata.setRelDataPath(archiveRelPath);
    archiveLdata.setDataFileExt("html");
    archiveLdata.setIsFcast(true);
    int leadTime = dataType2;
    archiveLdata.setLeadTime(leadTime);
    archiveLdata.setWriter("MosSpdb2Html");
    archiveLdata.setDataType("www");
    if (archiveLdata.write(report->time - leadTime)) { // Was generateTime.
      cerr << "ERROR  writing  LdataInfo file to : " << P->ArchiveDir << endl;
    }
    
  }

}

///////////////////////////////////////////////////////
//
// Small routine to QC metars from parameters and common sense.
//
void QCmetar(Params *P, station_report_t *metar){
  //
  // Wind speed. Parameter is in knots, metar is in m/s.
  //
  const float MetersPerSecToKnots = 1.9436346;
  if (
      (metar->windspd != STATION_NAN) &&
      (metar->windspd * MetersPerSecToKnots > P->MaxWindSpeed)
      )
    metar->windspd = STATION_NAN;
  //
  // Temperature
  //
  if (
      (metar->temp != STATION_NAN) &&
      ((metar->temp > P->TempRange.Max) || (metar->temp < P->TempRange.Min ))
      )
    metar->temp = STATION_NAN;
  //
  // Pressure
  //
  if (
      (metar->pres != STATION_NAN) &&
      ((metar->pres > P->PressureRange.Max) || (metar->pres < P->PressureRange.Min ))
      )
    metar->pres = STATION_NAN;
  //
  // Relative humidity
  //
  if (
      (metar->relhum != STATION_NAN) &&
      ((metar->relhum > P->RelHumRange.Max) || (metar->relhum < P->RelHumRange.Min ))
      )
    metar->relhum = STATION_NAN;
  //
  // Wind dir.
  //
  if (
      (metar->winddir != STATION_NAN) &&
      ((metar->winddir > 360.0) || (metar->winddir < 0.0 ))
      )
    metar->winddir = STATION_NAN;
  //
  // Ceiling.
  //
  if (
      (metar->ceiling != STATION_NAN) &&
      (metar->ceiling < 0.0 )
      )
    metar->ceiling = STATION_NAN;
  //
  // Visibility.
  //
  if (
      (metar->visibility != STATION_NAN) &&
      (metar->visibility < 0.0 )
      )
    metar->visibility = STATION_NAN;
}



