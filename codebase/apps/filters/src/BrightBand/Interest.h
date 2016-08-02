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
//////////////////////////////////////////////////////////
// Interest.h:  Interest associated with each plane of data
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#ifndef Interest_h
#define Interest_h

#include <time.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/globals.h>

using namespace std;

class Interest {
  
public:

  // constructor
       
  Interest(const bool debug_flag);
  
  // destructor
  
  virtual ~Interest();

  // Add interest fields to output file

  bool addInterestFields(DsMdvx &output_file,
			 const char *long_field_name,
			 const char *short_field_name) const;
  
   
  // Access methods

  inline fl32 getInterestValue(const int index) const
  {
    return interest_values[index];
  }
  
  inline int getHeightValue(const int index) const
  {
    return ht[index];
  }
  
  // data members - note that ht values are integers which describe
  // which plane is of interest;  when these values are written to the
  // output file, they are translated into km

  fl32 *interest_values;
  int *ht;
  int npoints;

protected:
  
  //
  // Constants
  //
  static const double _BAD_INTEREST_VALUE;
  static const int _BAD_HT_VALUE;
  
  bool _debug;
  
  MdvxField *_dbzField;
  Mdvx::field_header_t _dbzFieldHdr;
  
  void _initInterestFields(MdvxField *dbz_field);
  
};

#endif
