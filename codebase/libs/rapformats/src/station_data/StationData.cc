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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:45:40 $
//   $Id: StationData.cc,v 1.19 2016/03/03 18:45:40 dixon Exp $
//   $Revision: 1.19 $
//   $State: Exp $
//

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * StationData.cc: Class controlling data from a station that
 *                 reports by appending new data to the end
 *                 of an ASCII file.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1996
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <sys/types.h>
#include <sys/stat.h>

#include <toolsa/os_config.h>

#include <rapformats/station_file.h>
#include <rapformats/station_reports.h>

#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>

#include <rapformats/StationData.h>

#define MAX_ALLOWED_PRECIP_RATE 10.0 // mm/hr - R. Rasmussen 3/97
 
/**********************************************************************
 * Forward declarations for static functions.
 */

/**********************************************************************
 * Global variables.
 */

/**********************************************************************
 * StationHistory Member functions.
 * StationHistory collects a history of time vs accumulation
 * It rejects outlier points and can compute a rate based
 * on fitting a line to recent points
 */
StationHistory::StationHistory(int size /* = DEF_HISTORY_POINTS*/)
{
    accum_hist = new double[size];
    time_hist = new time_t[size];

    num_used = 0;
    array_size = size;
}

StationHistory::~StationHistory()
{
    delete []accum_hist;
    delete []time_hist;
}

//***********************************************************************
// Returns accum if not an outlier, STATION_NAN otherwise.
// This version assumes the point is an outlier if the
// accumulation is less than 0.15 mm from the mean
// of the last 10 points
double StationHistory::AddPoint(double accum, time_t time)
{
    int i;
    int npoints;
    double sum;
    double mean;


    // compute the max mumber of points to include in the mean
    npoints = (array_size > 10) ? 10 : array_size;
    npoints = (npoints > num_used) ? num_used : npoints;

    // compute the mean of the latest points
    sum = 0.0;
    for(i = num_used -1 ; i >= num_used - npoints; i--) {
	sum += accum_hist[i];
    }

    if(num_used > 0) { 
        mean = sum/npoints;
    } else {
	mean = accum;
    }

    // Check for outlier status
    if(accum < (mean - 0.15)) {
 	fprintf(stderr,"Outlier rejected -10pt mean: %g, accum: %g %s\n",
	    mean,accum,ctime(&time));
	return STATION_NAN;
    }

    // Keep track of the point
    if(num_used < array_size) {    // Array is not full
	accum_hist[num_used] = accum;
	time_hist[num_used] = time;
	num_used++;
    } else { // the array is full - Shuffle down one
	for(i=0; i < num_used -1; i++) {
	    accum_hist[i] = accum_hist[i+1];
	    time_hist[i] = time_hist[i+1];
	}

	// Store the latest
	accum_hist[num_used -1] = accum;
	time_hist[num_used -1] = time;
    }


    return accum;
}

//***********************************************************************
// Compute a linear least squares fit to a line
// Use a maximum of num_points
// Use a minimum of num_points/2
// Returns 0.0 if too few points to fit.
// Also clamp a negative slope to 0.0 as this is impossible with accumulation

double
StationHistory::GetRate(int num_points)
{
    int i;
    int index;
    int npoints;
    double sum_x = 0.0;
    double sum_y = 0.0;
    double slope = 0.0;
    double sum_difsq = 0.0;
    double mean_x;
    double temp;

    npoints = (num_points > num_used) ? num_used : num_points;
    if(npoints < (num_points / 2)) {
        return 0.0;
    }

    // Compute the sums 
    for(i=0,index = (num_used - npoints -1); i < npoints; i++,index++){
	sum_x += (double) time_hist[index];
	sum_y += accum_hist[index];
    }
    mean_x = sum_x / npoints;

    for(i=0,index = (num_used - npoints -1); i < npoints; i++,index++){ 
	temp = time_hist[index] - mean_x;
	sum_difsq += temp * temp;
	slope += temp * accum_hist[index];
    }
    slope /= sum_difsq;

	if(slope < 0.0) slope = 0.0;

    // printf("Rate = %g mm/hr using %d points\n",slope*3600,npoints);

    return slope;
}

/**********************************************************************
 * StationData Constructors
 */

StationData::StationData(char *input_dir,
			 char *station_prefix,
			 char *station_suffix,
			 char *station_label,
			 double station_lat,
			 double station_lon,
			 double station_alt,
			 double station_threshold,
			 StationDataFillProc fill_report,
			 int debug_flag,
			 int num_reg_points,
			 int self_accum_mode)
{
  // This constructor implies that we are not running in demo mode
  _demoMode = FALSE;
  _demoFilename = (char *)NULL;
  
  _debugMode = debug_flag;
  _num_reg_points = num_reg_points;

  _selfAccumMode = self_accum_mode;  // Must produce running total ourselves
  
  // Save the station prefix and suffix values.
  _inputDir = STRdup(input_dir);
  _stationPrefix = STRdup(station_prefix);
  _stationSuffix = STRdup(station_suffix);
  
  // Save the constant station information
  _stationLabel = STRdup(station_label);
  _stationLat = station_lat;
  _stationLon = station_lon;
  _stationAlt = station_alt;
  _station_transition_threshold = station_threshold;

  _accum_total = 0.0;
  
  // Save the fill report routine
  _fillReport = fill_report;

  // Initialize the last data received time and size
  _lastTime = 0;
  _lastSize = 0;
}


// Constructor for Demo mode
StationData::StationData(char *input_dir,
			 char *demo_filename,
			 char *station_label,
			 double station_lat,
			 double station_lon,
			 double station_alt,
			 double station_threshold,
			 StationDataFillProc fill_report,
			 int debug_flag,
			 int num_reg_points,
			 int self_accum_mode)
{
  // This constructor implies that we are running in demo mode
  _demoMode = TRUE;
  _demoFilename = STRdup(demo_filename);
  
  _debugMode = debug_flag;
  _num_reg_points = num_reg_points;

  _selfAccumMode = self_accum_mode;  // Must produce running total ourselves
  
  // Initialize the station prefix and suffix since they will not be
  // used in this case.
  _inputDir = STRdup(input_dir);
  _stationPrefix = (char *)NULL;
  _stationSuffix = (char *)NULL;
  
  // Save the constant station information
  _stationLabel = STRdup(station_label);
  _stationLat = station_lat;
  _stationLon = station_lon;
  _stationAlt = station_alt;
  _station_transition_threshold = station_threshold;
  
  _accum_total = 0.0;
  
  // Save the fill report routine
  _fillReport = fill_report;

  // Initialize the last data received time
  _lastTime = 0;
  _lastSize = 0;
}


/**********************************************************************
 * Destructor
 */

StationData::~StationData(void)
{
  // Free the string data in the object

  STRfree(_inputDir);
  STRfree(_stationPrefix);
  STRfree(_stationSuffix);
  STRfree(_demoFilename);
  STRfree(_stationLabel);

}


/**********************************************************************
 * getLatestReport() - Get the latest station report from the current
 *                     station report file.
 */

StationDataStatus
  StationData::getNextReport(station_report_t *return_report,
			     int calc_precip_rate,
			     int report_age,
			     int history_secs)
{
  int j;
  int num_lines1,num_lines2;
  int seconds_missing;
  int num_transitions = 0;
  time_t first_d_time = 0;
  time_t second_d_time = 0;
  time_t current_time;				// Now
  time_t data_end_time;
  time_t data_start_time;
  struct tm *data_end_time_struct;
  struct tm *data_start_time_struct;
  station_report_t curr_report;
  station_report_t prev_report;
  char fname[MAX_PATH_LEN];
  char *last_line;			// pointers to data file srtings
  char **line_list1;
  char **line_list2;
  double latest_accum, earliest_accum;
  double first_delta;
  double second_delta = 0;
  double running_tally = 0;
  char buf[1024];
  struct stat sbuf;

  // reports come in a few seconds late 
  data_end_time = time(0) - report_age;
  data_end_time_struct = gmtime(&data_end_time);      

  if (_demoMode)
    sprintf(fname, "%s/%s", _inputDir, _demoFilename);
  else
  {
    strftime(buf, 32, "%y%m%d", data_end_time_struct);
    sprintf(fname, "%s/%s%s.%s",
	    _inputDir,_stationPrefix, buf, _stationSuffix);
  } /* endif - _demoMode */
     
  // Check to see if the file has been updated. 
  if (ta_stat(fname, &sbuf) < 0)
    return(STATION_DATA_FILE_STAT_ERROR);

  if(sbuf.st_size < 10) return(STATION_DATA_NO_NEW_DATA); 

  data_start_time = data_end_time - history_secs;
  data_start_time_struct = gmtime(&data_start_time);

  current_time = time(0);

  // Output file has been modified and has been quesceint for at
  // least a second or we are in demo mode
  if ((sbuf.st_mtime > _lastTime) && 
      (sbuf.st_size > (int) _lastSize) &&
       ( _demoMode == TRUE  || (sbuf.st_mtime < current_time - 1)))
  {
    Station_file *curr_station_file;
    
    _lastSize = sbuf.st_size;                                 
    _lastTime = sbuf.st_mtime;                                 
    if (_debugMode)
      printf("File %s has been modified - examining\n",fname);

    // Open & examine the latest file;
    curr_station_file = new Station_file(fname);
    curr_station_file->get_last_line(last_line);

    // Convert the last line of the file into the report information
    _fillReport(last_line, &curr_report);

    if(_selfAccumMode) {   // tally the precip

	  _accum_total += curr_report.liquid_accum;

	  if (curr_report.msg_id == STATION_REPORT) {
	    _accum_total2 += curr_report.shared.station.liquid_accum2;
	  }

	  curr_report.liquid_accum = _accum_total;

	  if (curr_report.msg_id == STATION_REPORT) {
	    curr_report.shared.station.liquid_accum2 = _accum_total2;
	  }

	  running_tally = _accum_total;
    }

    // Update History and reject outliers
    curr_report.liquid_accum =
      _shist1.AddPoint((double)curr_report.liquid_accum,curr_report.time);
    if (curr_report.msg_id == STATION_REPORT) {
      curr_report.shared.station.liquid_accum2 =
	_shist2.AddPoint((double)curr_report.shared.station.liquid_accum2,
			 curr_report.time);
    }


    // Fill in the report information that is constant for the station
    STRcopy(curr_report.station_label, _stationLabel, ST_LABEL_SIZE);
    curr_report.lat = _stationLat;
    curr_report.lon = _stationLon;
    curr_report.alt = _stationAlt;
    
    if (_debugMode)
    {
      time_t print_time = curr_report.time;
      
      fprintf(stdout,
	      "  report time = %s", ctime(&print_time));
    }
    
    if (calc_precip_rate) // Rate not not included in the incomming data - Calc from accumulation
    {

     if(_num_reg_points > 2) { // Use the Regression method

      // Produces a rate of mm/second
      curr_report.precip_rate = _shist1.GetRate(_num_reg_points);

      // Convert to mm/hr
      curr_report.precip_rate *= 3600;

     } else { // Use timing between the last two transitions to compute rate.

      // Save the data end time and ending liquid accumulation values
      data_end_time = curr_report.time;
      latest_accum = curr_report.liquid_accum;

      // get the last part of the file
      curr_station_file->get_last_nsec(history_secs,
				       line_list1,
				       num_lines1);
      // look at the earliest data point
      _fillReport(line_list1[0], &prev_report);

      data_start_time = prev_report.time;
      earliest_accum = prev_report.liquid_accum;

      // Find out if any data is missing.
      // Presumably, these missing seconds of data
      // are in the previous day's file
      seconds_missing = history_secs - (data_end_time - data_start_time);

      // Avoid opening another file in demo mode
      if (_demoMode) seconds_missing = 0;

      if (_debugMode) printf("Using %d data points\n",num_lines1);
	
      first_delta = latest_accum;   // start at the end of time :-)
       
      // Scan file for the latest two transitions
      // loop back in time :-)
      for (j = num_lines1 -1; j >= 0 && num_transitions < 2; j--)
      {
	_fillReport(line_list1[j], &prev_report);
	
	if(_selfAccumMode) {
	      if(prev_report.liquid_accum >= _station_transition_threshold) {
	          second_d_time = first_d_time;
	          second_delta = first_delta;
	          first_d_time = prev_report.time;
	          first_delta =  running_tally;
	          num_transitions++;
	      }
	      running_tally -= prev_report.liquid_accum;

	 } else {   
	      if (first_delta - prev_report.liquid_accum >= _station_transition_threshold) {
	          second_d_time = first_d_time;
	          second_delta = first_delta;
	          first_d_time = prev_report.time;
	          first_delta = prev_report.liquid_accum;
	          num_transitions++;
	      }

	} // end if(_selfAccumMode) 
      } /* endfor - j */

      //  See if the time span crosses days
      if (data_end_time_struct->tm_yday !=  data_start_time_struct->tm_yday &&
	  (seconds_missing > 120) && (num_transitions < 2))
      {
	// Build name of previous day's file and open a Station_file
	strftime(buf, 32, "%y%m%d", data_start_time_struct);
	sprintf(fname, "%s%s.%s", _stationPrefix, buf, _stationSuffix);
	if (_debugMode)
	  printf("File %s is also being examined\n",fname);
	Station_file prev_station_file(fname);

	prev_station_file.get_last_nsec(seconds_missing,
					line_list2,
					num_lines2);
	if (_debugMode)
	  printf("Using %d data points\n",num_lines2);
        // loop back in time :-)
        for (j = num_lines2 -1; j >= 0 && num_transitions < 2; j--)
        {
	  _fillReport(line_list2[j], &prev_report);
	
	  if(_selfAccumMode) {
	      if(prev_report.liquid_accum >= _station_transition_threshold) {
	          second_d_time = first_d_time;
	          second_delta = first_delta;
	          first_d_time = prev_report.time;
	          first_delta =  running_tally;
	          num_transitions++;
	      }
	      running_tally -= prev_report.liquid_accum;

	 } else {   
	      if (first_delta - prev_report.liquid_accum >= _station_transition_threshold) {
	          second_d_time = first_d_time;
	          second_delta = first_delta;
	          first_d_time = prev_report.time;
	          first_delta = prev_report.liquid_accum;
	          num_transitions++;
	      }

	  } // end if(_selfAccumMode) 
	   
        } /* endfor - j */

	// record what the time and value of the earliest report is.
	_fillReport(line_list2[0], &prev_report);
	data_start_time = prev_report.time;
	earliest_accum = prev_report.liquid_accum;

      } /* endif - need to look at yesterday's file */

      // Calc rate over the last N minutes.
      switch(num_transitions)
      {
      default:
      case 0: 
	curr_report.precip_rate = 0.0;
	break;
	 
      case 1:
	if ((data_end_time - data_start_time) > 0)
	{
	  curr_report.precip_rate =  (latest_accum - earliest_accum) /
	    ((double)(data_end_time - data_start_time) / 3600.0);
	  if (curr_report.precip_rate < 0.0)
	    curr_report.precip_rate = 0.0;
	}
	else
	{
	  curr_report.precip_rate = STATION_NAN;
	} /* endif - curr_report.precip_rate < 0.0 */
	break;

      case 2:
	curr_report.precip_rate =  (first_delta - second_delta) /
	  ((double)(first_d_time - second_d_time) / 3600.0);
	if (curr_report.precip_rate < 0.0)
	  curr_report.precip_rate = 0.0;
      } /* endswitch - num_transitions */

      if(curr_report.precip_rate > MAX_ALLOWED_PRECIP_RATE) {
	   curr_report.precip_rate = MAX_ALLOWED_PRECIP_RATE;
      }

      if (_debugMode)
	printf("Late: %g, Early: %g  Precip_rate:%g mm/hr \n",
	       latest_accum, earliest_accum, curr_report.precip_rate);
     } // End of compute by timeing transitions

    } /* endif - calc_precip_rate */

    delete curr_station_file;

    
  }
  else
  {
    if (sbuf.st_mtime >= current_time - 1 && _debugMode)
    {
      printf("File <%s> not stable for more than one second\n",
	     fname);
    }
    
    return(STATION_DATA_NO_NEW_DATA);
  }
  
  *return_report = curr_report;
      
  return(STATION_DATA_SUCCESS);
}
