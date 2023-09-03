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
/////////////////////////////////////////////////////////////////////////////
// CIDD_PARAMS.CC Routines for handling/ initialising Parameters
//
//

#define CIDD_PARAMS
#include "cidd.h"
#include "cidd_params.h"

/////////////////////////////////////////////////////////////////////////////
// FIND_TAG_TEXT Search a null terminated string for the text between tags
//
// Searches through input_buf for text between tags of the form <TAG>...Text...</TAG>
// Returns a pointer to the beginning of the text and its length if found.
// text_line_no is used on input to begin counting and is set on output to the starting
// line number of the tagged text

#define TAG_BUF_LEN 256
const char *find_tag_text(const char *input_buf, const char * tag, long *text_len, long *text_line_no)
{
    int start_line_no;
    const char *start_ptr;
    const char *end_ptr;
    const char *ptr;
    char end_tag[TAG_BUF_LEN];
    char start_tag[TAG_BUF_LEN];

    // Reasonable tag check - Give up
    if(strlen(tag) > TAG_BUF_LEN - 5) {
	fprintf(stderr,"Unreasonable tag: %s - TOO long!\n",tag);
        *text_len = 0;
        return NULL;
    }

    // Clear the string buffers
    memset(start_tag,0,TAG_BUF_LEN);
    memset(end_tag,0,TAG_BUF_LEN);
    
    start_line_no = *text_line_no;

    sprintf(start_tag,"<%s>",tag);
    sprintf(end_tag,"</%s>",tag);

    // Search for Start tag
    if((start_ptr = strstr(input_buf,start_tag)) == NULL) {
        *text_len = 0;
        *text_line_no = start_line_no;
	return NULL;
    }
    start_ptr +=  strlen(start_tag); // Skip ofer tag to get to the text

    // Search for end tag after the start tag
    if((end_ptr = strstr(start_ptr,end_tag)) == NULL) {
        *text_len = 0;
        *text_line_no = start_line_no;
	return NULL;
    }
    end_ptr--;  // Skip back one character to last text character

    // Compute the length of the text_tag
    *text_len = (long) (end_ptr - start_ptr);

    // Count the lines before the starting tag
    ptr = input_buf;
    while(((ptr = strchr(ptr,'\n')) != NULL) && (ptr < start_ptr)) {
	ptr++; // Move past the found NL
	start_line_no++;
    }

    *text_line_no = start_line_no;
    return start_ptr;
}


/////////////////////////////////////////////////////////////////////////////
// LOAD_DB_DATA_DEFAULT : Allocate and load the parameter data base using the
//                        default parameter settings and set Global Struct
//                        members
// 

void load_db_data_default(char* &db_buf, int &db_len)
{
  // Generate the full params string.  The non-TDRP portions are kept in
  // static strings while the TDRP portions are loaded from the default
  // parameters

  gd.gui_P = new Cgui_P;
  gd.syprod_P = new Csyprod_P;
  gd.draw_P = new Cdraw_P;
  gd.images_P = new Cimages_P;
  gd.layers.earth._P = new Cterrain_P;
  gd.layers.route_wind._P = new Croutes_P;
  
  string params_text = ParamsTextMasterHeader;
  params_text += get_default_tdrp_params("GUI_CONFIG", gd.gui_P);
  params_text += ParamsTextGrids;
  params_text += ParamsTextWinds;
  params_text += ParamsTextMaps;
  params_text += ParamsTextMainParams;
  params_text += get_default_tdrp_params("DRAW_EXPORT", gd.draw_P);
  params_text += get_default_tdrp_params("IMAGE_GENERATION", gd.images_P);
  params_text += get_default_tdrp_params("SYMPRODS", gd.syprod_P);
  params_text += get_default_tdrp_params("TERRAIN",
					 gd.layers.earth._P);
  params_text += get_default_tdrp_params("ROUTE_WINDS",
					 gd.layers.route_wind._P);
  

  // Allocate space for the buffer copy

  db_len = params_text.size() + 1;
  
  if ((db_buf = (char *)calloc(db_len, 1)) == NULL) {
    fprintf(stderr,"Problems allocating %ld bytes for parameter file\n",
	    (long)db_len);
    exit(-1);
  }

  // Copy the parameters into the buffer

  memcpy(db_buf, params_text.c_str(), db_len - 1);
  db_buf[db_len-1] = '\0';
}


/////////////////////////////////////////////////////////////////////////////
// LOAD_DB_DATA_FILE : Allocate and load the parameter data base from a file
//                     and set Global Struct members
// 

void load_db_data_file(const string &fname, char* &db_buf, int &db_len)
{
    FILE *infile;

    // create temp buffer
    
    int tmpLen = 1000000;
    char *tmpBuf = new char[tmpLen];

    // Open DB file

    if((infile = fopen(fname.c_str(),"r")) == NULL) {
      perror(fname.c_str());
      fprintf(stderr,"Problems Opening %s\n",fname.c_str());
      exit(-1);
    }

    // Read into tmp buf
    db_len = fread(tmpBuf,1,tmpLen,infile);
    if(db_len <= 0) {
      perror(fname.c_str());
      fprintf(stderr,"Problems Reading %s\n",fname.c_str());
      exit(-1);
    }
    
    // Allocate space for the whole file plus a null
    if((db_buf = (char *)  calloc(db_len + 1, 1)) == NULL) {
      fprintf(stderr,"Problems allocating %d bytes for parameter file\n",
	      db_len);
      exit(-1);
    }
    
    // copy in

    memcpy(db_buf, tmpBuf, db_len);
    db_buf[db_len] = '\0'; // Make sure to null terminate
    delete[] tmpBuf;

    // Close DB file
    if(fclose(infile) != 0 )  {
      fprintf(stderr,"Problems Closing %s\n",fname.c_str());
      exit(-1);
    }


}


/////////////////////////////////////////////////////////////////////////////
// LOAD_DB_DATA_HTTP : Allocate and load the parameter data base from a Web
//                     server and set Global Struct members
// 

void load_db_data_http(const string &fname, char* &db_buf, int &db_len)
{
  int ret_stat;
  
  // Allow 5 seconds to retrieve the data 

  if(strlen(gd.http_proxy_url)  > URL_MIN_SIZE) {
    ret_stat = HTTPgetURL_via_proxy(gd.http_proxy_url, fname.c_str(), 5000,
				    &db_buf, &db_len);
  } else {
    ret_stat = HTTPgetURL(fname.c_str(), 5000, &db_buf, &db_len);
  }

  if(ret_stat <= 0 || db_len <= 0) {
    fprintf(stderr,"Could'nt Load Parameter Database from URL: %s,  %d\n",
	    fname.c_str(), ret_stat);
    if(stat < 0) {
      fprintf(stderr,"Failed to successfully trasnact with the http server\n");
    } else {
      fprintf(stderr,
	      "HTTP server couldn't retreive the file - Returned  Stat: %d\n",
	      ret_stat);
    }
    fprintf(stderr,
	    "Make sure URL looks like: http://host.domain/dir/filename\n");
    fprintf(stderr,
	    "The most common problem is usually missing  the :// part \n");
    fprintf(stderr,"or a misspelled/incorrect host, directory or filename\n");
    if(strlen(gd.http_proxy_url)  > URL_MIN_SIZE)
      fprintf(stderr,"Also Check Proxy URL:%s\n",gd.http_proxy_url);
    exit(-1);
  }
	
}


/////////////////////////////////////////////////////////////////////////////
// LOAD_DB_DATA : Allocate and load the data base file - Set Global
// Struct members
// 

void load_db_data(const string &fname)
{
    char *db_buf;
    int db_len;

    if (fname == "") {
      // Default parameters
      load_db_data_default(db_buf, db_len);
    } else if(strncasecmp(fname.c_str(), "http:", 5) == 0) {
      // HTTP Based retrieve 
      load_db_data_http(fname, db_buf, db_len);
    } else {
      // FILE based retrieve
      load_db_data_file(fname, db_buf, db_len);
    }

    gd.db_data = db_buf;
    gd.db_data_len = db_len;
}
