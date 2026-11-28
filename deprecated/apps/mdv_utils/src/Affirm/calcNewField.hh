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
#ifndef calcNewField_HH
#define calcNewField_HH

class calcNewField
{
public:
    calcNewField();
    ~calcNewField()
        {
        }
    double processData(const double index, 
		       const class Params * _params, 
		       const double missing_data_value,
		       const double bad_data_value,
		       const int MISSING_DATA_VALUE) const;

    double processData(const double first_value, 
		       const double sec_value,
		       const class Params * _params, 
		       const double first_missing_data_value,
		       const double first_bad_data_value,
		       const double sec_missing_data_value,
		       const double sec_bad_data_value,
		       const int MISSING_DATA_VALUE) const;

    double processData(const double index, 
		       const class Params * _params, 
		       const double missing_data_value,
		       const double bad_data_value,
		       const double min_value,
		       const double max_value,
		       const int MISSING_DATA_VALUE) const;

    double processData(const double index1, 
		       const double index2, 
		       const double index3, 
		       const double index4, 
		       const class Params * _params, 
		       const double missing_data_value,
		       const double bad_data_value,
		       const int MISSING_DATA_VALUE) const;
private:
};

#endif
