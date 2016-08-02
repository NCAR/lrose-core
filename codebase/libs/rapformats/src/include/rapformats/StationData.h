/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/************************************************************************
 * StationData.h : header file for the StationData class.  This
 *                     class controls reports from a station that reports
 *                     by appending new reports to the end of an ASCII
 *                     file.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1996
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef StationData_H
#define StationData_H

/*
 **************************** includes **********************************
 */

#include <rapformats/station_reports.h>

/*
 ******************************* defines ********************************
 */

typedef enum
{
  STATION_DATA_SUCCESS,

  STATION_DATA_NO_NEW_DATA,
  STATION_DATA_FILE_STAT_ERROR
} StationDataStatus;

#define DEF_HISTORY_POINTS 256

typedef int (*StationDataFillProc)(char *data_line,
				   station_report_t *report);

/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class StationHistory
{
public:
  // Constructor
  StationHistory(int size = DEF_HISTORY_POINTS);
  ~StationHistory();

  // Returns STATION_NAN if point is an outlier 
  double AddPoint(double accum, time_t time);

  // Returns a regression fit using the last N points
  double GetRate(int num_points);

private:

  int array_size;
  int num_used;

  double *accum_hist;
  time_t *time_hist;

};

class StationData
{
 public:
  
  //
  // Constructors
  //

  // This is the regular constructor.  The station prefix and suffix are
  // used to build the input file names.  The input file names are built
  // from the current date and the prefix and suffix in the following
  // way:
  //        <prefix>YYMMDD.<suffix>
  StationData(char *input_dir,
	      char *station_prefix,
	      char *station_suffix,
	      char *station_label,
	      double station_lat,
	      double station_lon,
	      double station_alt,
	      double station_threshold,
	      StationDataFillProc fill_report,
	      int debug_flag = FALSE,
	      int num_reg_points = -1,
	      int self_accum_mode = FALSE);

  // This is the demo mode constructor.  It gets data from a single input
  // file.
  StationData(char *input_dir,
	      char *demo_filename,
	      char *station_label,
	      double station_lat,
	      double station_lon,
	      double station_threshold,
	      double station_alt,
	      StationDataFillProc fill_report,
	      int debug_flag = FALSE,
	      int num_reg_points = -1,
	      int self_accum_mode = FALSE);
  
  // Destructor

  ~StationData(void);
  
  // Get the next station report

  StationDataStatus getNextReport(station_report_t *report,
				  int calc_precip_rate,
				  int report_age,
				  int history_secs);
  
 private:
  
  int _demoMode;
  int _debugMode;
  int _selfAccumMode;
  int _num_reg_points;

  char *_inputDir;
  char *_stationPrefix;
  char *_stationSuffix;
  
  char *_demoFilename;
  
  char *_stationLabel;
  
  double _stationLat;
  double _stationLon;
  double _stationAlt;
  double _station_transition_threshold;

  double _accum_total;
  double _accum_total2;
  
  StationHistory _shist1;
  StationHistory _shist2;

  StationDataFillProc _fillReport;
  
  time_t _lastTime;
  size_t _lastSize;
  
};


#endif
