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
////////////////////////////////////////////////////////////////////////////////
// DATA_ROW.CC:  Class to provide support for ASCII line oriented, columnar data
//
//


#include <cstdio>
#include <cstdlib>
#include <string.h>

#include <toolsa/Except.hh>
#include <toolsa/Data_Row.hh>
using namespace std;

#ifndef HUGE_VAL
#define HUGE_VAL 9.99e29
#endif

////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
// Construct Data_row object- No allocation of internal memory
Data_row::Data_row(void)
{
    len = 0;
    num_fields = 0;
    parsed_string_size = 0;
    field_array_size = 0;
    curr_string = (char *)NULL;
    parsed_string = (char *)NULL;
    c_field = (char **)NULL;
    d_field = (double *)NULL;
}

Data_row::Data_row(char *str)
{
    len = 0;
    num_fields = 0;
    parsed_string_size = 0;
    field_array_size = 0;
    parsed_string = (char *)NULL;
    c_field = (char **)NULL;
    d_field = (double *)NULL;

   curr_string = str;
   parse_fields();	// parse the string into char subfields

}

////////////////////////////////////////////////////////////////////////////////
// SET Functions 

int Data_row::set(char * str)
{
     curr_string = str;
     parse_fields();	// parse the string into char subfields
     return num_fields;
}

////////////////////////////////////////////////////////////////////////////////
// ACCESS Functions 

char *Data_row::get_cfield(int field)
{
    if(field < 0 || field >= num_fields) THROW(INDEX_RANGE,"Can't grab that field");

    return c_field[field];
}

double Data_row::get_dfield(int field)
{
    if(field < 0 || field >= num_fields) THROW(INDEX_RANGE,"Can't grab that field");

    return d_field[field];
}


int Data_row::get_cfield_array(char**& char_ptr_ref)
{
    char_ptr_ref = c_field;
    return num_fields;
}

int Data_row::get_dfield_array(double *&double_ptr_ref)
{
    double_ptr_ref = d_field;
    return num_fields;
    
}

////////////////////////////////////////////////////////////////////////////////
// MODIFY Functions 

void Data_row::scale_field(int field, double scale, double bias)
{
    if(field < 0 || field >= num_fields) THROW(INDEX_RANGE,"Can't grab that field");
    if(d_field[field] == HUGE_VAL) return;	// Don't scale special value
    d_field[field] *= scale;
    d_field[field] += bias;
}

////////////////////////////////////////////////////////////////////////////////
// DESTRUCTOR
// Construct Data_row object- No allocation of internal memory
Data_row::~Data_row(void)
{
    // delete allocated space 'newed'. 
    if(parsed_string != NULL) delete [] parsed_string;
    if(c_field != NULL) {
	delete [] c_field;
	c_field = (char **)NULL;
    }
    if(d_field != NULL) {
	delete d_field;
	d_field = (double *)NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
// PARSE_FIELDS : copy and parse the current string into subfields

void Data_row::parse_fields(void)
{
	int i;
	int in_field;
	int n_fields = 0;
	int cur_field = 0;
	char *ptr,*check_ptr;

	if(curr_string == NULL) THROW(NULL_POINTER,"No string to parse");
	len = strlen(curr_string);
	
	// Make sure internal buffer for parsed string is big enough
	if(len+1 > (int) parsed_string_size) {
	     if(parsed_string != NULL) delete [] parsed_string;
             parsed_string = new char[len+1];
             parsed_string_size = len+1;
	 }
	 strncpy(parsed_string,curr_string,len);
	 parsed_string[len] = '\0';

	 num_fields = 0;  

	// find, count and seperate by nulls, each alphanumeric field
	ptr = parsed_string;
	for(i=0; i <= len; i++,ptr++) {
	   if(*ptr <= ' ') {
	      while(*ptr <= ' ' && i <= len) {
		*ptr++ = '\0';
		i++;
	      }
	      n_fields++;
	   }
       }

       // Allocate space for char ptr array and double array
       if(n_fields > (int) field_array_size) {
           if(c_field != NULL) delete [] c_field;
           if(d_field != NULL) delete [] d_field;
           c_field = new char*[n_fields];
           d_field = new double[n_fields];
       }

       num_fields = n_fields;

        // set char field ptr array and convert each field to doubles
	ptr = parsed_string;
	cur_field = 0;
	in_field = 0;
	for(i=0; i < len; i++,ptr++) {
	  if(*ptr != '\0') {    // is part of a field 
	    if(in_field == 0) { // the start of a new field
	        c_field[cur_field] = ptr;
	        d_field[cur_field] = strtod(ptr,&check_ptr);
	        // Unfortunately there's no universal value defined for NAN
	        if(check_ptr == ptr) d_field[cur_field] = HUGE_VAL; // Not a numerical field
	        cur_field++;
	        in_field = 1;
	    }
	  } else {  // in between fields 
	    in_field = 0;
	  }
      }

      return;
}

