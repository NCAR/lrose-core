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
// Server.cc
//
// File Server object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2000
//
///////////////////////////////////////////////////////////////


#include <rapformats/GenPt.hh>
#include <rapformats/ComboPt.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <Spdb/DsSpdbMsg.hh>

#include <stdio.h> // For the formatting of the time labels

#include "Server.hh"
using namespace std;



/*********************************************************************
 * Constructor
 *
 * Inherits from DsSymprodServer
 */

Server::Server(const string &prog_name,
	       Params *initialParams) :
  DsSymprodServer(prog_name,
		  initialParams->instance,
		  (void*)(initialParams),
		  initialParams->port,
		  initialParams->qmax,
		  initialParams->max_clients,
		  initialParams->no_threads,
		  initialParams->debug >= Params::DEBUG_NORM,
		  initialParams->debug >= Params::DEBUG_VERBOSE),
  _symprodVertAlign(Symprod::VERT_ALIGN_CENTER),
  _symprodHorizAlign(Symprod::HORIZ_ALIGN_CENTER),
  _symprodFontStyle(Symprod::TEXT_NORM)
{
  // Do nothing
}


/*********************************************************************
 * loadLocalParams() - Load local params if they are to be overridden.
 */

int Server::loadLocalParams(const string &paramFile,
			    void **serverParams)
{
  Params  *localParams;
  char   **tdrpOverrideList = NULL;
  bool     expandEnvVars = true;

  const string routine_name = "_allocLocalParams";

  if (_isDebug)
    cerr << "Loading new params from file: " << paramFile << endl;

  localParams = new Params(*((Params*)_initialParams));
  if (localParams->load((char*)paramFile.c_str(),
			tdrpOverrideList,
			expandEnvVars,
			_isVerbose) != 0)
  {
    cerr << "ERROR - " << _executableName << "::" << routine_name << endl;
    cerr << "Cannot load parameter file: " << paramFile << endl;
    
    delete localParams;
    return -1;
  }

  // Make sure the local params are correct

  if (localParams->field_info_n < 1)
  {
    cerr << "ERROR - " << _executableName << "::" << routine_name << endl;
    cerr << "Error in parameter file: " << paramFile << endl;
    cerr << "Must specify at least 1 field name to be displayed" << endl;
    
    delete localParams;
    return -1;
  }
  
  if (_isVerbose)
    localParams->print(stderr, PRINT_SHORT);

 // Convert param values to Symprod object values

  switch (localParams->text_vert_align)
  {
  case Params::VERT_ALIGN_TOP :
    _symprodVertAlign = Symprod::VERT_ALIGN_TOP;
    break;
    
  case Params::VERT_ALIGN_CENTER :
    _symprodVertAlign = Symprod::VERT_ALIGN_CENTER;
    break;
    
  case Params::VERT_ALIGN_BOTTOM :
    _symprodVertAlign = Symprod::VERT_ALIGN_BOTTOM;
    break;
  } /* endswitch - localParams->text_vert_align */

  switch (localParams->text_horiz_align)
  {
  case Params::HORIZ_ALIGN_LEFT :
    _symprodHorizAlign = Symprod::HORIZ_ALIGN_LEFT;
    break;
    
  case Params::HORIZ_ALIGN_CENTER :
    _symprodHorizAlign = Symprod::HORIZ_ALIGN_CENTER;
    break;
    
  case Params::HORIZ_ALIGN_RIGHT :
    _symprodHorizAlign = Symprod::HORIZ_ALIGN_RIGHT;
    break;
  } /* endswitch - localParams->text_horiz_align */
  
  switch (localParams->text_font_style)
  {
  case Params::TEXT_NORM :
    _symprodFontStyle = Symprod::TEXT_NORM;
    break;
    
  case Params::TEXT_BOLD :
    _symprodFontStyle = Symprod::TEXT_BOLD;
    break;
    
  case Params::TEXT_ITALICS :
    _symprodFontStyle = Symprod::TEXT_ITALICS;
    break;
    
  case Params::TEXT_SUBSCRIPT :
    _symprodFontStyle = Symprod::TEXT_SUBSCRIPT;
    break;
    
  case Params::TEXT_SUPERSCRIPT :
    _symprodFontStyle = Symprod::TEXT_SUPERSCRIPT;
    break;
    
  case Params::TEXT_UNDERLINE :
    _symprodFontStyle = Symprod::TEXT_UNDERLINE;
    break;
    
  case Params::TEXT_STRIKETHROUGH :
    _symprodFontStyle = Symprod::TEXT_STRIKETHROUGH;
    break;
  } /* endswitch - localParams->text_font_style */
  
  *serverParams = (void*)localParams;
  return 0;
}


/*********************************************************************
 * convertToSymprod() - Convert the given data chunk from the SPDB
 *                      database to symprod format.
 *
 * Returns 0 on success, -1 on failure
 */

int Server::convertToSymprod(const void *params,
			     const string &dir_path,
			     const int prod_id,
			     const string &prod_label,
			     const Spdb::chunk_ref_t &chunk_ref,
                             const Spdb::aux_ref_t &aux_ref,
			     const void *spdb_data,
			     const int spdb_len,
			     MemBuf &symprod_buf)
{
  const string routine_name = "convertToSymprod";
  Params *serverParams = (Params*) params;
  GenPt point;
  ComboPt pt;
  // Convert the SPDB data to a GenPt
  
  switch (prod_id) {    
    case SPDB_GENERIC_POINT_ID:
      point.disassemble(spdb_data, spdb_len);
    break;

    case SPDB_COMBO_POINT_ID:
      pt.disassemble(spdb_data, spdb_len);
	  if(serverParams->combopt_side == 1) {
		point = pt.get1DPoint(); 
	  } else {
		point = pt.get2DPoint(); 
	  }

    break;

    default: 
        cerr << "ERROR - " << _executableName << ":Server::convertToSymprod" << endl;
        cerr << "  Incorrect prod_id: " << prod_id << endl;
        cerr << "Should be SPDB_GENERIC_POINT_ID or SPDB_COMBO_POINT_ID "<< endl;
        return -1;
    break;
  }

  // If these is a field named saveTime, and it is requested
  // that it be displayed, do so.

  bool haveSaveTime = false;
  date_time_t saveTime;

  int fn = point.getFieldNum("saveTime");
  if (fn != -1){
    double saveTime_d = point.get1DVal(fn);
    saveTime.unix_time = (time_t) saveTime_d;
    uconvert_from_utime( &saveTime );
    haveSaveTime = true;
  }  

  // create Symprod object

  time_t now = time(NULL);

  Symprod prod(now, now,
	       chunk_ref.valid_time,
	       chunk_ref.expire_time,
	       chunk_ref.data_type,
	       chunk_ref.data_type2,
	       SPDB_GENERIC_POINT_LABEL);

  //
  // Set the colors up according to thresholding, if requested.
  //
  char *colorToUse = serverParams->text_color; // The default.
  //
  if (serverParams->do_color_thresholding){
    double fieldVal=0.0;
    bool gotFieldVal = false;
    //
    // See if we are color coding by age.
    //
    if (strcmp(serverParams->color_threshold_field_name,"dataAge")){
      //
      // We are not, so get the color from the field value.
      //
      int fn = point.getFieldNum( serverParams->color_threshold_field_name );
      if (fn != -1){
	fieldVal = point.get1DVal(fn);
	gotFieldVal = true;
      }
    } else {
      //
      // We are color coding by data age, so set the fieldVal to the
      // data age.
      //
      const DsSpdbMsg& readMsg = this->getReadMsg();
      
      const time_t refTime = (const time_t) readMsg.getRefTime();

      fieldVal = double(refTime - chunk_ref.valid_time);
      if (fieldVal >= 0.0) gotFieldVal = true;

    }
    //
    // If we have a fieldVal, then set the color appropriately.
    //
    if (gotFieldVal){
      for (int i=0; i < serverParams->color_thesholds_n; i++){
	if (
	    (fieldVal >= serverParams->_color_thesholds[i].minval) &&
	    (fieldVal <  serverParams->_color_thesholds[i].maxval)
	    ){
	  colorToUse = serverParams->_color_thesholds[i].color;
	  break;
	}
      }
    }
  }


  if (serverParams->do_text_labelling){

    // Construct the data string
    string text_string;
    bool first_field = true;

    if(serverParams->add_point_text &&  point.getText().size() > 0) {
	  text_string += point.getText();
      text_string += " ";
	}

    
    for (int i = 0; i < serverParams->field_info_n; ++i)
      {
	char field_value[BUFSIZ];
	
	// Get the field number for the field
	
	int field_num = point.getFieldNum(serverParams->_field_info[i].name);
    
	if (field_num < 0)
	  STRcopy(field_value, serverParams->missing_value_string, BUFSIZ);
	else
	  sprintf(field_value, serverParams->value_format_string,
		  point.get1DVal(field_num) *
		  serverParams->_field_info[i].multiplier);
    
	// Add the field value to the text string

	if (!first_field)
	  text_string += serverParams->field_delim;
    
	text_string += field_value;
	
	first_field = false;
	
      } /* endfor - i */


    // Add the string to the Symprod object

    prod.addText(text_string.c_str(),
		 point.getLat(), point.getLon(),
		 colorToUse, serverParams->text_background_color,
		 serverParams->text_offset.x, serverParams->text_offset.y,
		 _symprodVertAlign, _symprodHorizAlign,
		 serverParams->text_font_size,
		 _symprodFontStyle, serverParams->font_name);
    
  } /* End of   if (serverParams->do_text_labelling) */


  if (serverParams->draw_crosses){

    // Draw crosses at GenPt locations, if requested.
    // Niles as per Mike's suggestion.

    // Setup simple cross icon
    Symprod::ppt_t cross_icon[5] =
    {
      { -serverParams->cross_icon_size,  0                              },
      {  serverParams->cross_icon_size,  0                              }, 
      {  0,                              0                              }, 
      {  0,                              -serverParams->cross_icon_size }, 
      {  0,                              serverParams->cross_icon_size  }
    };


    MemBuf crossBuf;

    Symprod::wpt_t wpt;
    wpt.lat = point.getLat();
    wpt.lon = point.getLon();
    crossBuf.add(&wpt, sizeof(wpt));


    prod.addStrokedIcons(colorToUse,
			 5, cross_icon,
			 1,
			 (Symprod::wpt_t *) crossBuf.getPtr(),
			 0,0,serverParams->cross_line_width);


  } /* End of if (serverParams->draw_crosses) */

  //
  // Label with data times, if requested.
  // Set up the SaveTimeLable while we're at it, although it may not
  // be used. Declare it now so that it has sufficient scope.
  char saveTimeLabel[64];

  if (serverParams->do_time_labelling){

    date_time_t dataTime;
    dataTime.unix_time = point.getTime();
    uconvert_from_utime( &dataTime );
    //
    // Set the format for the label appropriately.
    //
    char timeLabel[64];

    switch( serverParams->time_format ){
      
    case Params::TIME_LABEL_HHMM :
      sprintf(timeLabel,"%02d:%02d",
	      dataTime.hour, dataTime.min);
      //
      if (haveSaveTime){
	sprintf(saveTimeLabel,"Saved %02d:%02d %d min diff",
		saveTime.hour, saveTime.min,
		(int)rint(fabs((double)saveTime.unix_time-dataTime.unix_time)/60.0));
      }
      break;

    case Params::TIME_LABEL_HHMMSS :
      sprintf(timeLabel,"%02d:%02d:%02d",
	      dataTime.hour, dataTime.min, dataTime.sec);
      //
      if (haveSaveTime){
	sprintf(saveTimeLabel,"Saved %02d:%02d:%02d %d min diff",
		saveTime.hour, saveTime.min, saveTime.sec,
		(int)rint(fabs((double)saveTime.unix_time-dataTime.unix_time)/60.0));
      }
      break;
      //
      // If time_format is set to something weird, we default to the following.
      //
    case Params::TIME_LABEL_YYYYMMDDHHMMSS :
    default :
      sprintf(timeLabel,"%d/%02d/%02d %02d:%02d:%02d",
	      dataTime.year, dataTime.month, dataTime.day,
	      dataTime.hour, dataTime.min, dataTime.sec);
     //
      if (haveSaveTime){
	sprintf(saveTimeLabel,"Saved %d/%02d/%02d %02d:%02d:%02d %d min diff",
		saveTime.year, saveTime.month, saveTime.day,
		saveTime.hour, saveTime.min, saveTime.sec,
		(int)rint(fabs((double)saveTime.unix_time-dataTime.unix_time)/60.0));
      }
      break;

    }

    prod.addText(timeLabel,
		 point.getLat(), point.getLon(),
		 colorToUse,
		 serverParams->text_background_color,
		 serverParams->time_text_offset.x, 
		 serverParams->time_text_offset.y,
		 _symprodVertAlign, _symprodHorizAlign,
		 serverParams->text_font_size,
		 _symprodFontStyle, serverParams->font_name);

  }

  if (
      (serverParams->show_SaveTime) && 
      (haveSaveTime)
      ){
   

    prod.addText(saveTimeLabel,
		 point.getLat(), point.getLon(),
		 colorToUse,
		 serverParams->text_background_color,
		 serverParams->delay_text_offset.x, 
		 serverParams->delay_text_offset.y,
		 _symprodVertAlign, _symprodHorizAlign,
		 serverParams->text_font_size,
		 _symprodFontStyle, serverParams->font_name);
    
  }

  // set return buffer

  if (_isVerbose)
    prod.print(cerr);

  prod.serialize(symprod_buf);

  return 0;
}


/*********************************************************************
 * _handleGet() - Handle a get request -- sets the prod_id and prod_label 
 *                in the get info appropriately.
 *
 * Returns 0 on success, -1 on failure
 */

int Server::_handleGet(const void *localParams,
		       const DsSpdbMsg &inMsg, 
		       const string &dirPath,
		       Socket &socket)
  
{
  cerr << "*** Entering Server::_handleGet()" << endl;
  
  Params *params = (Params *)localParams;
  
  switch (params->timing_type)
  {
  case Params::TIMING_NORMAL :
    return DsSymprodServer::_handleGet(localParams, inMsg, dirPath, socket);
    break;
    
  case Params::TIMING_DAILY :
    return _handleDailyGet(localParams, inMsg, dirPath, socket);
    break;
  }
  
  return -1;
}


/*********************************************************************
 * _handleGet() - Handle a daily get request.
 *
 * Returns 0 on success, -1 on failure
 */

int Server::_handleDailyGet(const void *localParams,
			    const DsSpdbMsg &inMsg, 
			    const string &dirPath,
			    Socket &socket)
  
{
  // For a daily get, we need to replace the original request times with a
  // request to get all data between the beginning of the day and the request
  // time.

  DsSpdbMsg new_msg = inMsg;
  
  // DsSpdbMsg::info_t msg_info = new_msg.getInfo();
  DateTime new_end_time(new_msg.getRefTime());
  DateTime new_start_time = new_end_time;
  new_start_time.setHour(0);
  new_start_time.setMin(0);
  new_start_time.setSec(0);
  
  new_msg.setMode(DsSpdbMsg::DS_SPDB_GET_MODE_INTERVAL);
  new_msg.setStartTime(new_start_time.utime());
  new_msg.setEndTime(new_end_time.utime());

  return DsSymprodServer::_handleGet(localParams, new_msg, dirPath, socket);
}

