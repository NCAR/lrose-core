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

/////////////////////////////////////////////////////////////////////////////
// FIND_TAG_TEXT Search a null terminated string for the text between tags
//
// Searches through input_buf for text between tags of the form <TAG>...Text...</TAG>
// Returns a pointer to the beginning of the text and its length if found.
// text_line_no is used on input to begin counting and is set on output to the starting
// line number of the tagged text

#define TAG_BUF_LEN 256
char *find_tag_text(char *input_buf, char * tag, long *text_len, long *text_line_no)
{
    int start_line_no;
    char *start_ptr;
    char *end_ptr;
    char *ptr;
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
// LOAD_DB_DATA : Allocate and load the data base file - Set Global
// Struct members
// 

void load_db_data( char *fname)
{
    char *db_buf;
    int db_len;
    int ret_stat;
    FILE *infile;

    struct stat sbuf;

    // HTTP Based retrieve 
    if(strncasecmp(fname,"http:",5) == 0) {
	// Allow 5 seconds to retrieve the data 

	if(strlen(gd.http_proxy_url)  > URL_MIN_SIZE) {
	    ret_stat = HTTPgetURL_via_proxy(gd.http_proxy_url, fname,5000, &db_buf, &db_len);
	} else {
	    ret_stat = HTTPgetURL(fname,5000, &db_buf, &db_len);
	}
	if(ret_stat <= 0 || db_len <= 0) {
	    fprintf(stderr,"Could'nt Load Parameter Database from URL: %s,  %d\n", fname,ret_stat);
	    if(stat < 0) {
		fprintf(stderr,"Failed to successfully trasnact with the http server\n");
	    } else {
		fprintf(stderr,"HTTP server couldn't retreive the file - Returned  Stat: %d\n",ret_stat);
	    }
	    fprintf(stderr,"Make sure URL looks like: http://host.domain/dir/filename\n");
	    fprintf(stderr,"The most common problem is usually missing  the :// part \n");
	    fprintf(stderr,"or a misspelled/incorrect host, directory or filename\n");
	    if(strlen(gd.http_proxy_url)  > URL_MIN_SIZE)
	      fprintf(stderr,"Also Check Proxy URL:%s\n",gd.http_proxy_url);
	    exit(-1);
	}
	
    } else { // FILE based retrieve
	// Check existance and size of file
        if(stat(fname,&sbuf) < 0) {
	    fprintf(stderr,"Load Parameter Database %s Doesn't exist\n",fname);
	    exit(-1);
        }

	// Allocate space for the whole file plus a null
	if((db_buf = (char *)  calloc(sbuf.st_size + 1 ,1)) == NULL) {
	    fprintf(stderr,"Problems allocating %ld bytes for parameter file\n",
		    sbuf.st_size);
	     exit(-1);
	}

	// Open DB file
	if((infile = fopen(fname,"r")) == NULL) {
	    fprintf(stderr,"Problems Opening %s\n",fname);
	     exit(-1);
	}

	// Read DB file
	if((db_len = fread(db_buf,1,sbuf.st_size,infile)) != sbuf.st_size) {
	    fprintf(stderr,"Problems Reading %s\n",fname);
	     exit(-1);
	}

	db_buf[sbuf.st_size] = '\0'; // Make sure to null terminate

	// Close DB file
	if(fclose(infile) != 0 )  {
	    fprintf(stderr,"Problems Closing %s\n",fname);
	     exit(-1);
	}


    }

    gd.db_data = db_buf;
    gd.db_data_len = db_len;
}
