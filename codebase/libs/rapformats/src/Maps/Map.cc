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
//   $Date: 2016/03/03 18:45:39 $
//   $Id: Map.cc,v 1.10 2016/03/03 18:45:39 dixon Exp $
//   $Revision: 1.10 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Map.cc: class representing a map.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string.h>

#include <rapformats/Map.hh>
#include <rapformats/MapIcon.hh>
#include <rapformats/MapPolyline.hh>
#include <toolsa/str.h>

/**********************************************************************
 * Initialize statics
 */

const string Map::MAP_NAME_STRING = "MAP_NAME";
const string Map::TRANSFORM_STRING = "TRANSFORM";
const string Map::PROJECTION_STRING = "PROJECTION";
const string Map::ICONDEF_STRING = "ICONDEF";
const string Map::ICON_STRING = "ICON";
const string Map::POLYLINE_STRING = "POLYLINE";
const string Map::LABEL_STRING = "LABEL";
const string Map::SIMPLE_LABEL_STRING = "SIMPLELABEL";

/**********************************************************************
 * Constructor
 */

Map::Map(void) :
  _mapName("unnamed")
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

Map::~Map(void)
{
  _clear();
  
}


//////////////////////////
// Input/Output methods //
//////////////////////////

/**********************************************************************
 * read() - Read the map information from the given file stream.
 */

bool Map::read(FILE *stream)
{
//  // Prime strtok_r;
//  str_ptr = strtok_r(map_buf,"\n",&lasts);
//
//  while (str_ptr != NULL) {        /* read all lines in buffer */
//    if(*str_ptr != '#') {
//
//      if(strncasecmp(str_ptr,"MAP_NAME",8) == 0) {    /* Currently Ignore */
//	str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	continue;
//      }
//
//      if(strncasecmp(str_ptr,"TRANSFORM",9) == 0) {    /* Currently Ignore */
//	str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	continue;
//      }
//
//      if(strncasecmp(str_ptr,"PROJECTION",10) == 0) {    /* Currently Ignore */
//	str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	continue;
//      }
//
//      if(strncasecmp(str_ptr,"ICONDEF",7) == 0) {        /* describes an icon's coordinates in pixels */
//	index = ov->num_icondefs;
//	if(index >= ov->num_alloc_icondefs) {
//	  if(ov->num_alloc_icondefs == 0) { /* start with space for 2 */
//	    ov->geo_icondef = (Geo_feat_icondef_t **)
//	      calloc(2,sizeof(Geo_feat_icondef_t *));
//	    ov->num_alloc_icondefs = 2;
//	  } else { /* Double the space */
//	    ov->num_alloc_icondefs *= 2;
//	    ov->geo_icondef = (Geo_feat_icondef_t **) 
//	      realloc(ov->geo_icondef, ov->num_alloc_icondefs * sizeof(Geo_feat_icondef_t *)); 
//	  }
//	}
//	if(ov->geo_icondef == NULL) {
//	  fprintf(stderr,"Unable to allocate memory for Icon definition pointer array!\n");
//	  exit(-1);
//	}
//
//	if(STRparse(str_ptr,cfield,256,32,64) != 3) {
//	  fprintf(stderr,"Error in ICONDEF line: %s\n",str_ptr);
//	  exit(-1);
//	}
//	/* get space for the icon definition */
//	ov->geo_icondef[index] = (Geo_feat_icondef_t *) calloc(1,sizeof(Geo_feat_icondef_t));
//	ZERO_STRUCT(ov->geo_icondef[index]);
//
//	if(ov->geo_icondef[index] == NULL) {
//	  fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
//	  exit(-1);
//	}
//	STRcopy(ov->geo_icondef[index]->name,cfield[1],NAME_LENGTH);
//	num_points = atoi(cfield[2]);
//
//	/* Get space for points in the icon */
//	ov->geo_icondef[index]->x = (short *) calloc(1,num_points * sizeof(short));
//	ov->geo_icondef[index]->y = (short *) calloc(1,num_points * sizeof(short));
//	
//	if(ov->geo_icondef[index]->x == NULL || ov->geo_icondef[index]->y == NULL) {
//	  fprintf(stderr,"Error!: Unable to allocate space for icon points in file %s, num points: %d\n",
//		  ov->map_file_name,num_points);
//	  exit(-1);
//	}
//
//	/* Read in all of the points */
//	for(j=0,point = 0; j < num_points; j++) {
//	  
//	  str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//
//	  if(str_ptr != NULL && STRparse(str_ptr,cfield,256,32,64) == 2) {
//	    ov->geo_icondef[index]->x[point] = atoi(cfield[0]);
//	    ov->geo_icondef[index]->y[point] = atoi(cfield[1]);
//	    point++;
//	  }
//	}
//	ov->geo_icondef[index]->num_points = point;
//	ov->num_icondefs++;
//	str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	continue;
//      }
//
//      if(strncasecmp(str_ptr,"ICON ",5) == 0) {    
//	index = ov->num_icons;
//	if(index >= ov->num_alloc_icons) {
//	  if(ov->num_alloc_icons == 0) { /* start with space for 2 */
//	    ov->geo_icon = (Geo_feat_icon_t **)
//	      calloc(2,sizeof(Geo_feat_icon_t *));
//	    ov->num_alloc_icons = 2;
//	  } else {  /* Double the space */
//	    ov->num_alloc_icons *= 2;
//	    ov->geo_icon = (Geo_feat_icon_t **) 
//	      realloc(ov->geo_icon, ov->num_alloc_icons * sizeof(Geo_feat_icon_t *)); 
//	  }
//	}
//	if(ov->geo_icon == NULL) {
//	  fprintf(stderr,"Unable to allocate memory for Icon pointer array!\n");
//	  exit(-1);
//	}
//
//	/* get space for the Icon */
//	ov->geo_icon[index] = (Geo_feat_icon_t *) calloc(1,sizeof(Geo_feat_icon_t));
//	ZERO_STRUCT(ov->geo_icon[index]);
//
//	if(ov->geo_icon[index] == NULL) {
//	  fprintf(stderr,"Unable to allocate memory for Icon definition!\n");
//	  exit(-1);
//	}
//	
//	if((num_fields = STRparse(str_ptr,cfield,256,32,64)) < 6) {
//	  fprintf(stderr,"Error in ICON line: %s\n",str_ptr);
//	  exit(-1);
//	}
//
//	/* find the definition for the line segments that make up the icon */
//	ov->geo_icon[index]->icon = NULL;
//	found = 0;
//	for(j=0; j < ov->num_icondefs && found == 0; j++) {
//	  if(strcmp(ov->geo_icondef[j]->name,cfield[1]) == 0) {
//	    ov->geo_icon[index]->icon = ov->geo_icondef[j];
//	    found = 1;
//	  }
//	}
//
//	if(found == 0) {    
//	  fprintf(stderr,"No Icon definition: %s found in file %s!\n",cfield[1],ov->map_file_name);
//	  str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	  continue;
//	}
//
//	/* record its position */
//	ov->geo_icon[index]->lat = atof(cfield[2]);
//	ov->geo_icon[index]->lon = atof(cfield[3]);
//	ov->geo_icon[index]->text_x = atoi(cfield[4]);
//	ov->geo_icon[index]->text_y = atoi(cfield[5]);
//
//	/* gather up remaining text fields */
//	ov->geo_icon[index]->label[0] = '\0';
//	len = 2;
//	for(j = 6; j < num_fields && len < LABEL_LENGTH; j++ ) {
//	  strncat(ov->geo_icon[index]->label,cfield[j],LABEL_LENGTH - len);
//	  len = strlen(ov->geo_icon[index]->label) +1;
//	  
//	  // Separate multiple text label fiedds with spaces.
//	  if( j < num_fields -1) {
//	    strncat(ov->geo_icon[index]->label," ",LABEL_LENGTH - len);
//	    len = strlen(ov->geo_icon[index]->label) +1;
//	  }
//	}
//	{
//	  int labellen = strlen(ov->geo_icon[index]->label);
//	  if (labellen > 1) {
//	    if (ov->geo_icon[index]->label[labellen-1] == ' ') {
//	      ov->geo_icon[index]->label[labellen-1] = '\0';
//	    }
//	  }
//	}
//
//	ov->num_icons++;
//	str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	continue;
//      }
//
//      if(strncasecmp(str_ptr,"POLYLINE",8) == 0) {    
//	index = ov->num_polylines;
//	if(index >= ov->num_alloc_polylines) {
//	  if(ov->num_alloc_polylines == 0) { /* start with space for 2 */
//	    ov->geo_polyline = (Geo_feat_polyline_t **)
//	      calloc(2,sizeof(Geo_feat_polyline_t *));
//	    ov->num_alloc_polylines = 2;
//	  } else {  /* Double the space */
//	    ov->num_alloc_polylines *= 2;
//	    ov->geo_polyline = (Geo_feat_polyline_t **) 
//	      realloc(ov->geo_polyline, ov->num_alloc_polylines * sizeof(Geo_feat_polyline_t *)); 
//	  }
//	}
//	if(ov->geo_polyline == NULL) {
//	  fprintf(stderr,"Unable to allocate memory for Polyline pointer array!\n");
//	  exit(-1);
//	}
//
//	if((STRparse(str_ptr,cfield,256,32,64)) != 3) {
//	  fprintf(stderr,"Error in POLYLINE line: %s\n",str_ptr);
//	  exit(-1);
//	}
//	/* get space for the Polyline definition */
//	ov->geo_polyline[index] = (Geo_feat_polyline_t *) calloc(1,sizeof(Geo_feat_polyline_t));
//	ZERO_STRUCT(ov->geo_polyline[index]);
//	
//	if(ov->geo_polyline[index] == NULL) {
//	  fprintf(stderr,"Unable to allocate memory for Polyline definition!\n");
//	  exit(-1);
//	}
//	STRcopy(ov->geo_polyline[index]->label,cfield[1],LABEL_LENGTH);
//	num_points = atoi(cfield[2]);
//	if(num_points <=0 ) {
//	  fprintf(stderr,"Warning!: Bad POLYLINE Definition. File: %s, Line: %s\n",name_buf,str_ptr);
//	  fprintf(stderr,"        : Format should be:    POLYLINE Label #points\n");
//	  fprintf(stderr,"        : Skipping \n");
//	  str_ptr = strtok_r(NULL,"\n",&lasts); // move to next line
//	  continue;
//	}
//
//	/* Get space for points in the polyline */
//	ov->geo_polyline[index]->lat = (double *) calloc(1,num_points * sizeof(double));
//	ov->geo_polyline[index]->lon = (double *) calloc(1,num_points * sizeof(double));
//	ov->geo_polyline[index]->local_x = (double *) calloc(1,num_points * sizeof(double));
//	ov->geo_polyline[index]->local_y = (double *) calloc(1,num_points * sizeof(double));
//	
//	if(ov->geo_polyline[index]->lat == NULL || ov->geo_polyline[index]->lon == NULL) {
//	  fprintf(stderr,"Error!: Unable to allocate space for polyline points in file %s, num points: %d\n",
//		  ov->map_file_name,num_points);
//	  exit(-1);
//	}
//
//	/* Read in all of the points */
//	for(j=0,point = 0; j < num_points; j++) {
//	  str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	  if(str_ptr != NULL && STRparse(str_ptr,cfield,256,32,64) >= 2) {
//	    ov->geo_polyline[index]->lat[point] = atof(cfield[0]);
//	    ov->geo_polyline[index]->lon[point] = atof(cfield[1]);
//	    point++;
//	  }
//	}
//	ov->geo_polyline[index]->num_points = point;
//	ov->num_polylines++;
//	str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	continue;
//      }
//
//      if(strncasecmp(str_ptr,"LABEL",5) == 0) {    
//	index = ov->num_labels;
//	if(index >= ov->num_alloc_labels) {
//	  if(ov->num_alloc_labels == 0) { /* start with space for 2 */
//	    ov->geo_label = (Geo_feat_label_t **)
//	      calloc(2,sizeof(Geo_feat_label_t *));
//	    ov->num_alloc_labels = 2;
//	  } else {  /* Double the space */
//	    ov->num_alloc_labels *=2;
//	    ov->geo_label = (Geo_feat_label_t **) 
//	      realloc(ov->geo_label, ov->num_alloc_labels * sizeof(Geo_feat_label_t *)); 
//	  }
//	}
//	if(ov->geo_label == NULL) {
//	  fprintf(stderr,"Unable to allocate memory for Label pointer array!\n");
//	  exit(-1);
//	}
//
//	ov->num_labels++;
//                     
//	/* get space for the Label definition */
//	ov->geo_label[index] = (Geo_feat_label_t *) calloc(1,sizeof(Geo_feat_label_t));
//	ZERO_STRUCT(ov->geo_label[index]);
//
//	if(ov->geo_label[index] == NULL) {
//	  fprintf(stderr,"Unable to allocate memory for Label definition!\n");
//	  exit(-1);
//	}
//
//	if((num_fields = STRparse(str_ptr,cfield,256,32,64)) < 8) {
//	  fprintf(stderr,"Too few fields in LABEL line: %s\n",str_ptr);
//	  str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	  continue;
//	} 
//	ov->geo_label[index]->min_lat = atof(cfield[1]);
//	ov->geo_label[index]->min_lon = atof(cfield[2]);
//	ov->geo_label[index]->max_lat = atof(cfield[3]);
//	ov->geo_label[index]->max_lon = atof(cfield[4]);
//	ov->geo_label[index]->rotation = atof(cfield[5]);
//	ov->geo_label[index]->attach_lat = atof(cfield[6]);
//	ov->geo_label[index]->attach_lon = atof(cfield[7]);
//
//	ov->geo_label[index]->string[0] = '\0';
//	len = 2;
//	for(j = 8; j < num_fields && len < NAME_LENGTH; j++) {
//	  strncat(ov->geo_label[index]->string,cfield[j],NAME_LENGTH - len);
//	  len = strlen(ov->geo_label[index]->string) +1;
//	  if(j < num_fields -1)
//	    strncat(ov->geo_label[index]->string," ",NAME_LENGTH - len);
//	  len = strlen(ov->geo_label[index]->string) +1;
//	}
//	str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	continue;
//      }
//
//
//      if(strncasecmp(str_ptr,"SIMPLELABEL",11) == 0) {    
//	index = ov->num_labels;
//	if(index >= ov->num_alloc_labels) {
//	  if(ov->num_alloc_labels == 0) { /* start with space for 2 */
//	    ov->geo_label = (Geo_feat_label_t **)
//	      calloc(2,sizeof(Geo_feat_label_t *));
//	    ov->num_alloc_labels = 2;
//	  } else {  /* Double the space */
//	    ov->num_alloc_labels *=2;
//	    ov->geo_label = (Geo_feat_label_t **) 
//	      realloc(ov->geo_label, ov->num_alloc_labels * sizeof(Geo_feat_label_t *)); 
//	  }
//	}
//	if(ov->geo_label == NULL) {
//	  fprintf(stderr,"Unable to allocate memory for Label pointer array!\n");
//	  exit(-1);
//	}
//	ov->num_labels++;
//
//	/* get space for the Label definition */
//	ov->geo_label[index] = (Geo_feat_label_t *) calloc(1,sizeof(Geo_feat_label_t));
//	ZERO_STRUCT(ov->geo_label[index]);
//
//	if(ov->geo_label[index] == NULL) {
//	  fprintf(stderr,"Unable to allocate memory for Label definition!\n");
//	  exit(-1);
//	}
//
//	if((num_fields = STRparse(str_ptr,cfield,256,32,64)) < 4) {
//	  fprintf(stderr,"Too few fields in SIMPLELABEL line: %s\n",str_ptr);
//	  str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	  continue;
//	} 
//	ov->geo_label[index]->min_lat = atof(cfield[1]);
//	ov->geo_label[index]->min_lon = atof(cfield[2]);
//	ov->geo_label[index]->max_lat = atof(cfield[1]);
//	ov->geo_label[index]->max_lon = atof(cfield[2]);
//	ov->geo_label[index]->rotation = 0;
//	ov->geo_label[index]->attach_lat = atof(cfield[1]);
//	ov->geo_label[index]->attach_lon = atof(cfield[2]);
//
//	ov->geo_label[index]->string[0] = '\0';
//	len = 2;
//	for(j = 3; j < num_fields && len < NAME_LENGTH; j++) {
//	  strncat(ov->geo_label[index]->string,cfield[j],NAME_LENGTH - len);
//	  len = strlen(ov->geo_label[index]->string) +1;
//	  if(j < num_fields -1)
//	    strncat(ov->geo_label[index]->string," ",NAME_LENGTH - len);
//	  len = strlen(ov->geo_label[index]->string) +1;
//	}
//	str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//	continue;
//      }
//
//    } 
//
//    // Nothing matches
//    str_ptr = strtok_r(NULL,"\n",&lasts); // grab next line
//    
//  }  // End of while additional lines exist in buffer

  // Clear out any current map information

  _clear();
  
  // Read in the new map information

  ssize_t bytes_read;
  char *line = 0;
  size_t line_len = 0;

  while ((bytes_read = getline(&line, &line_len, stream)) >= 0)
  {
    // Remove any comments from the line

    for (size_t i = 0; i < line_len; ++i)
    {
      if (line[i] == '\0')
	break;
      if (line[i] == '#')
      {
	line[i] = '\0';
	break;
      }
    }
    
    // Pull the first token off of the line and continue processing
    // based on that token. If there isn't a token, then this line was
    // either blank or a comment line and should be skipped

    char *line_copy = STRdup(line);
    
    char *token;
    if ((token = strtok(line_copy, " \t\n")) == 0)
      continue;
    
    if ((string)token == MAP_NAME_STRING)
    {
      // Extract the map name from the line

      char *map_name = strtok(0, " \t\n");
      if (map_name != 0)
	_mapName = map_name;
    }
    else if ((string)token == TRANSFORM_STRING)
    {
      // Currently ignore these lines
    }
    else if ((string)token == PROJECTION_STRING)
    {
      // Currently ignore these lines
    }
    else if ((string)token == ICONDEF_STRING)
    {
    }
    else if ((string)token == ICON_STRING)
    {
      MapIcon *icon = new MapIcon();
      
      if (!icon->read(line, stream))
	return false;
      
      _objectList.push_back(icon);
    }
    else if ((string)token == POLYLINE_STRING)
    {
      MapPolyline *polyline = new MapPolyline();
      
      if (!polyline->read(line, stream))
	return false;

      _objectList.push_back(polyline);
    }
    else if ((string)token == LABEL_STRING)
    {
    }
    else if ((string)token == SIMPLE_LABEL_STRING)
    {
    }
    
    free(line_copy);
  }
  
  return true;
}


/**********************************************************************
 * write() - Write the map information to the given file stream.
 */

void Map::write(FILE *stream) const
{
  fprintf(stream, "%s %s\n",
	  MAP_NAME_STRING.c_str(), _mapName.c_str());
  fprintf(stream,
	  "PROJECTION 6 33.012759 -113.990335 41.503629 -106.7778 6358.38 1.0\n");
  
  vector< MapObject* >::const_iterator object_iterator;
  for (object_iterator = _objectList.begin();
       object_iterator != _objectList.end();
       object_iterator++)
    {
      (*object_iterator)->write(stream);
    }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
/**********************************************************************
 * _clear() - Clear out the current map information.
 */

void Map::_clear()
{
  // Clear the object list

  vector< MapObject* >::iterator obj_iter;
  for (obj_iter = _objectList.begin();
       obj_iter != _objectList.end();
       ++obj_iter)
    delete *obj_iter;

  _objectList.clear();

  // Clear the icon list

  _iconList.clear();
  
  // Reset the map name

  _mapName = "";
}
