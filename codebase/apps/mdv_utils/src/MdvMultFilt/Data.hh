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
/************************************************************************

Header: Data.hh

Author: Dave Albo

Date:   Fri Jan 27 16:56:22 2006

Description: Holds data info locally, Data is not created or destroyed locally.

*************************************************************************/

# ifndef    Data_HH
# define    Data_HH

/* System include files / Local include files */
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include "Params.hh"
using namespace std;

/* Class definition */
class Data
{
  friend class TempData;

public:

  // default constructor, takes mdv data inputs and builds state from that.
  Data(MdvxField *f);
  
  // copy constructor
  Data(const Data &);
  
  // destructor
  virtual ~Data();

  // operator overloading
  void operator=(const Data &);
  bool operator==(const Data &) const;

  ////////////////////////////////////////////////////////////////
  /////////////////////// simple accessors ///////////////////////
  ////////////////////////////////////////////////////////////////

  void print(void) const;
  inline fl32 *get_data(void) { return _data;}

  ////////////////////////////////////////////////////////////////
  /////////////////////// simple filters /////////////////////////
  ////////////////////////////////////////////////////////////////

  // return min/max/mean or false when all data is missing.
  bool evaluate(double &min, double &max, double &mean) const;

  // change data missing/bad to input value.
  void change(fl32 newbad);

  // filter using params:
  // for filter_combine, inputs assumed synchronized.
  void filter_thresh(Params::mdv_thresh_parm_t &p);
  void filter_smooth(Params::mdv_smooth_parm_t &p);
  void filter_combine(fl32 *data, fl32 missing, fl32 bad, int n,
		      Params::mdv_combine_parm_t &p);
  void filter_expand_value(Params::mdv_expand_value_parm_t &p);
  void filter_passthrough(void);

  // synchronize the data, used before filter_combine.
  // on exit:
  //  true and :
  //       internal data has been changed, with bad=missing=newbad.
  //       the input data has not been changed, but newbad is a value not
  //       present in the data.
  //  false and:
  //       nothing has been changed, no need to change.
  //      
  bool synchronize_data(fl32 *data, int n, fl32 missing,
			fl32 bad, fl32 &newbad);

protected:

  ////////////////////////////////////////////////////////////////
  /////////////////////// protected members //////////////////////
  ////////////////////////////////////////////////////////////////

  fl32 *_data;       // pointer to data owned by someone else.
  int _ndata;        // size of data
  fl32 _missing;     // data missing value.
  fl32 _bad;         // data bad value.
  int _nx, _ny, _nz; // data dimensions

private:  

};


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

# endif     /* Data_HH */
