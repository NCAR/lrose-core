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

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <mel_bufr/mel_bufr.h>
#include "BufrVariableDecoder.hh"

using namespace std;

BufrVariableDecoder::BufrVariableDecoder()  
{
}

BufrVariableDecoder::~BufrVariableDecoder()
{
}

 BufrVariableDecoder::Status_t BufrVariableDecoder::getVar(float &floatVar, 
							   char* fxy,  
							   BUFR_Val_t &bv)
{
  int n = BUFR_Get_Value(&bv, 1);

  if ( n == BUFR_EOM)
  {
    return BUFR_DECODER_EOM;
  }
  else if (n == BUFR_EOF)
  {
    return BUFR_DECODER_EOF;
  }
  else
  { 
    char *c_fxy = FXY_String(bv.FXY_Val);

    if ( strcmp( c_fxy, fxy ) == 0)
    {
      floatVar = bv.Val.number;

      return BUFR_DECODER_SUCCESS;
    }
    else
    {
      return BUFR_DECODER_FAILURE;
    }
  }
}

 BufrVariableDecoder::Status_t BufrVariableDecoder::getVar(int &intVar, 
							   char* fxy,  
							   BUFR_Val_t &bv)
{
  int n = BUFR_Get_Value(&bv, 1);

  if ( n == BUFR_EOM)
  {
    return BUFR_DECODER_EOM;
  }
  else if (n == BUFR_EOF)
  {
    return BUFR_DECODER_EOF;
  }
  else
  { 
    char *c_fxy = FXY_String(bv.FXY_Val);

    if ( strcmp( c_fxy, fxy ) == 0)
    {
      intVar = bv.Val.int_number;

      return BUFR_DECODER_SUCCESS;
    }
    else
    {
      return BUFR_DECODER_FAILURE;
    }
  }
}

 BufrVariableDecoder::Status_t BufrVariableDecoder::getVar(char *stringVar, 
							   char* fxy,  
							   BUFR_Val_t &bv)
{
  int n = BUFR_Get_Value(&bv, 1);

  if ( n == BUFR_EOM)
  {
    return BUFR_DECODER_EOM;
  }
  else if (n == BUFR_EOF)
  {
    return BUFR_DECODER_EOF;
  }
  else
  { 
    char *c_fxy = FXY_String(bv.FXY_Val);

    if ( strcmp( c_fxy, fxy ) == 0)
    {
      sprintf(stringVar, "%s", bv.Val.string);

      return BUFR_DECODER_SUCCESS;
    }
    else
    {
      return BUFR_DECODER_FAILURE;
    }
  }
}

BufrVariableDecoder::Status_t BufrVariableDecoder::checkVar(char *fxy, 
							    BUFR_Val_t &bv)
{
  int n = BUFR_Get_Value(&bv, 1);

  if ( n == BUFR_EOM)
  {
    return BUFR_DECODER_EOM;
  }
  else if (n == BUFR_EOF)
  {
    return BUFR_DECODER_EOF;
  }
  else
  { 
    char *c_fxy = FXY_String(bv.FXY_Val);

    if ( strcmp( c_fxy, fxy ) == 0)
    {
      return BUFR_DECODER_SUCCESS;
    }
    else
    {
      return BUFR_DECODER_FAILURE;
    }
  }
}


char * BufrVariableDecoder::getFxyString(BUFR_Val_t &bv)
{
  char c_fxy[32];
  char *c_fxy_ptr = &(c_fxy[0]);
  c_fxy_ptr  = FXY_String(bv.FXY_Val);
  return c_fxy_ptr;
}

BufrVariableDecoder::Status_t BufrVariableDecoder::getValue(float &floatVar, 
							   char* fxy,  
							   BUFR_Val_t &bv)
{
  char *c_fxy = FXY_String(bv.FXY_Val);

  if ( strcmp( c_fxy, fxy ) == 0)
  {
    floatVar = bv.Val.number;
    
    return BUFR_DECODER_SUCCESS;
  }
  else
  {
    return BUFR_DECODER_FAILURE;
  }
}

BufrVariableDecoder::Status_t BufrVariableDecoder::getValue(int &intVar, 
							    char* fxy,  
							    BUFR_Val_t &bv)
{
  char *c_fxy = FXY_String(bv.FXY_Val);

  if ( strcmp( c_fxy, fxy ) == 0)
  {
    intVar = bv.Val.int_number;
    
    return BUFR_DECODER_SUCCESS;
  }
  else
  {
    return BUFR_DECODER_FAILURE;
  }
}

BufrVariableDecoder::Status_t BufrVariableDecoder::getFxyString(char *fxy, 
							     BUFR_Val_t &bv)
{
  int n = BUFR_Get_Value(&bv, 1);

  if ( n == BUFR_EOM)
  {
    return BUFR_DECODER_EOM;
  }
  else if (n == BUFR_EOF)
  {
    return BUFR_DECODER_EOF;
  }
  else
  { 
    sprintf(fxy,"%s", FXY_String(bv.FXY_Val));

    return BUFR_DECODER_SUCCESS;
  }   
}
