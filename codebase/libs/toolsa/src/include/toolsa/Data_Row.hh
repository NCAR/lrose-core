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
// Data_Row.H:  Class to provide support for ASCII line oriented, columnar data
//  Author F. Hage NCAR, RAP Oct 1994
//


#include <cstdio>
#include <math.h>
#include <string.h>
using namespace std;

class Data_row {
protected:
	int	len;		// length of current string;
	int	num_fields;	//  
	size_t	parsed_string_size;	// current size of parse buffer
	size_t	field_array_size;	// current size of field arrays 
	char	*curr_string;	// untouched original string;
	char    *parsed_string; // copy of above with nulls placed where white space was
	char	**c_field;      // array of pointers to found fields
	double	*d_field;	// array of doubles
	Data_row *next;		// Pointer to the next data row in a set
	Data_row *prev;		// Pointer to the previous data row in a set
    
        void parse_fields(void);	// copies and parses the current string into subfileds

public:
 // constructors
    Data_row(void);
    Data_row(char *);

 // Set functions
    int set(char *); // Sets data row to character string - returns number of fields found
  // int set(FILE *); // Sets data row to next line in  open file 
  // not yet implemented

 // Access functions
    int   get_num_fields(void) { return num_fields; };

    char *get_cfield(int field_num);  // returns a pointer to the parsed subfield
    int get_cfield_array(char**& char_ptr_ref);    // returns number of fields in list

    double get_dfield(int field_num); // returns the value of the field_num data column
    int get_dfield_array(double *&dbl_ptr_ref); // returns the number of fields in list

 // Modifiers
    void  scale_field(int field_num,double scale, double bias);

 // destructors
    ~Data_row();
 	 
};
