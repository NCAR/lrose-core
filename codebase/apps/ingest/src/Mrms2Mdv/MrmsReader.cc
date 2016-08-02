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
///////////////////////////////////////////////////////////////////////////
//
// Read a MRMS Cartesian binary file header info and data.		
//
///////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <zlib.h>
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/DateTime.hh>
#include "MrmsReader.hh"

MrmsReader::MrmsReader()
{
  _debug = 0;
  _swap_flag = 0;
  _zhgt = NULL;
  _data = NULL;
}

MrmsReader::MrmsReader(bool debug)
{
  _debug = debug;
  _swap_flag = 0;
  _zhgt = NULL;
  _data = NULL;
}

MrmsReader::MrmsReader(bool debug, int swap_flag)
{
  _debug = debug;
  _swap_flag = swap_flag;
  _zhgt = NULL;
  _data = NULL;
}

MrmsReader::~MrmsReader()
{
  _clear();
}

void MrmsReader::_clear()
{
  if(_zhgt)
    delete [] _zhgt;
  _radarnam.clear();
  if(_data)
    delete [] _data;
  _zhgt = NULL;
  _data = NULL;
}

float MrmsReader::getMinY()
{
  return _nw_lat - (_ny * _dy);
}

int MrmsReader::readFile(char *filePath)
{
  _clear();

    /*--------------------------*/
    /*** 0. Declare variables ***/ 
    /*--------------------------*/
    
    short int *binary_data = 0;

    int yr,mo,day,hr,min,sec;
    int map_scale, dxy_scale, z_scale;
    char temp_varname[20];
    char temp_varunit[6];

    int temp;
    char chartemp[4];



    /*-------------------------*/
    /*** 1. Open binary file ***/ 
    /*-------------------------*/
        
    char open_mode[3];
    gzFile   fp_gzip;

    sprintf(open_mode,"%s","rb");
    open_mode[2] = '\0';

    if ( (fp_gzip = gzopen(filePath,open_mode) ) == (gzFile) NULL )
    {
      cout<<"+++ERROR: Could not open "<<filePath<<endl;
      return -1;
    }


    /*---------------------------------------*/
    /*** 2. Read binary header information ***/ 
    /*---------------------------------------*/
    
    //reading time (UTC)
    gzread( fp_gzip, &yr,sizeof(int));  // 1-4
    if (_swap_flag==1) byteswap(yr);
    
    gzread( fp_gzip, &mo,sizeof(int));  // 5-8 
    if (_swap_flag==1) byteswap(mo);
    
    gzread( fp_gzip, &day,sizeof(int)); // 9-12
    if (_swap_flag==1) byteswap(day);
    
    gzread( fp_gzip, &hr,sizeof(int));  // 13-16
    if (_swap_flag==1) byteswap(hr);
    
    gzread( fp_gzip, &min,sizeof(int)); // 17-20
    if (_swap_flag==1) byteswap(min);
    
    gzread( fp_gzip, &sec,sizeof(int)); // 21-24
    if (_swap_flag==1) byteswap(sec);

    DateTime data_time = DateTime(yr, mo, day, hr, min, sec);
    _epoch_seconds = (long)data_time.utime();
    
    //read dimensions
    gzread( fp_gzip,&temp,sizeof(int)) ;  // 25-28
    if (_swap_flag==1) byteswap(temp);
    _nx = temp;
    
    gzread( fp_gzip, &temp,sizeof(int)) ;  // 29-32
    if (_swap_flag==1) byteswap(temp);
    _ny = temp;
    
    gzread( fp_gzip, &temp,sizeof(int)) ;  // 33-36
    if (_swap_flag==1) byteswap(temp);
    _nz = temp;


    //read deprecated value (map projection type)
    gzread(fp_gzip,&chartemp,4*sizeof(char));  // 37-40


    //read map scale factor
    gzread(fp_gzip,&temp,sizeof(int));  // 41-44
    if (_swap_flag==1) byteswap(temp);
    map_scale = temp;
      
      
    //read deprecated value (trulat1)
    gzread( fp_gzip,&temp,sizeof(int)) ;   // 45-48
    
    //read deprecated value (trulat2)
    gzread( fp_gzip, &temp,sizeof(int)) ;  // 49-52
    
    //read deprecated value (trulon)
    gzread( fp_gzip, &temp,sizeof(int)) ;  // 53-56
    
      
    //read nw lat/lon coordinates
    gzread(fp_gzip,&temp,sizeof(int));  // 57-60
    if (_swap_flag==1) byteswap(temp);
    _nw_lon = (float)temp/(float)map_scale;
    
    gzread(fp_gzip,&temp,sizeof(int));  // 61-64
    if (_swap_flag==1) byteswap(temp);
    _nw_lat = (float)temp/(float)map_scale;


    //read deprecated value (xy_scale)
    gzread(fp_gzip,&temp,sizeof(int));  // 65-68


    //read dx and dy    
    gzread(fp_gzip,&temp,sizeof(int));  // 69-72
    if (_swap_flag==1) byteswap(temp);
    _dx = (float)temp;

    gzread(fp_gzip,&temp,sizeof(int));  // 73-76
    if (_swap_flag==1) byteswap(temp);
    _dy = (float)temp;


    //read dx and dy scaling factor and unscale
    gzread(fp_gzip,&temp,sizeof(int));  // 77-80
    if (_swap_flag==1) byteswap(temp);
    dxy_scale = temp;
      
    _dx = _dx/float(dxy_scale);
    _dy = _dy/float(dxy_scale);

    _zhgt = new float[_nz];

    //read heights      
    for(int k=0; k<_nz; k++)  // 81- [80+nz*4]  (let [80+nz*4] = X)
    {
      gzread(fp_gzip,&temp,sizeof(int));
      if (_swap_flag==1) byteswap(temp);
      _zhgt[k] = (float)temp;
    }


    //read z scale 
    gzread(fp_gzip,&temp,sizeof(int));  // [X+1] - [X+4]
    if (_swap_flag==1) byteswap(temp);
    z_scale = temp;
    
    //Unscale heights, if needed
    if( (z_scale != 1) && (z_scale != 0) )
    {
      for(int k=0; k<_nz; k++) _zhgt[k] /= (float)z_scale;
    }
      
      
    //read junk (place holders for future use)
    for(int j = 0; j < 10; j++)  // [X+5] - [X+44]
    {
      gzread(fp_gzip,&temp,sizeof(int)); 
    }
      
      
    //read variable name
    gzread(fp_gzip,temp_varname,20*sizeof(char));  // [X+45] - [X+64]
    if (_swap_flag==1) byteswap(temp_varname,20);    
    temp_varname[19]='\0';
    
    strncpy(_varname, temp_varname, 20);

    //read variable unit
    gzread(fp_gzip,temp_varunit,6*sizeof(char));  // [X+65] - [X+70]
    if (_swap_flag==1) byteswap(temp_varunit,6);    
    temp_varunit[5]='\0';
    
    strncpy(_varunit, temp_varunit,6);


    //read variable scaling factor
    gzread(fp_gzip,&_var_scale,sizeof(int));  // [X+71] - [X+74]
    if (_swap_flag==1) byteswap(_var_scale);


    //read variable missing flag
    gzread(fp_gzip,&_missing_val,sizeof(int));  // [X+75] - [X+78]
    if (_swap_flag==1) byteswap(_missing_val);


    //read number of radars affecting this product
    gzread(fp_gzip,&_nradars,sizeof(int));  // [X+79] - [X+82]
    if (_swap_flag==1) byteswap(_nradars);
      
      
    //read in names of radars
    char temp_radarnam[5];
      
    for(int i=0;i<_nradars;i++)  // [X+83] - [X+82+nradars*4]
    {
      gzread(fp_gzip,temp_radarnam,4*sizeof(char));
      if (_swap_flag==1) byteswap(temp_radarnam,4);
      temp_radarnam[4]='\0';
      
      _radarnam.push_back(temp_radarnam);
    }



    /*-------------------------*/
    /*** 3. Read binary data ***/ 
    /*-------------------------*/
    
    int num = _nx*_ny*_nz;
    binary_data = new short int[num];
      
    //read data array    [X+83+nradars*4] - [X+82+nradars*4+num*2]
    gzread(fp_gzip,binary_data,num*sizeof(short int));  
    if (_swap_flag==1) byteswap(binary_data,num);

    if(_data)                                                                                                                                                                   
      delete [] _data;
                                                                                                                                                              
    _data = new float[num];
    for(int i = 0; i < num; i++)
      _data[i] = (float)binary_data[i] / (float)_var_scale;

    delete [] binary_data;
      
    //_reOrderNS_2_SN(_data, _nx, _ny);

    /*------------------------------*/
    /*** 4. Close file and return ***/ 
    /*------------------------------*/
   
    //close file      
    gzclose( fp_gzip );

    return 0;
}


/*------------------------------------------------------------------

	Function:	byteswap (version 1)
		
	Purpose:	Perform byte swap operation on various data
	            types		
				
	Input:		data = some value
					
	Output:		value byte swapped
	
------------------------------------------------------------------*/
template < class Data_Type >
void MrmsReader::byteswap( Data_Type &data )
{
  unsigned int num_bytes = sizeof( Data_Type );
  char *char_data = reinterpret_cast< char * >( &data );
  char *temp = new char[ num_bytes ];

  for( unsigned int i = 0; i < num_bytes; i++ )
  {
    temp[ i ] = char_data[ num_bytes - i - 1 ];
  }

  for( unsigned int i = 0; i < num_bytes; i++ )
  {
    char_data[ i ] = temp[ i ];
  }
  delete [] temp;
}
  


/*------------------------------------------------------------------

	Function:	byteswap (version 2)
		
	Purpose:	Perform byte swap operation on an array of
	            various data types		
				
	Input:		data = array of values
					
	Output:		array of byte swapped values
	
------------------------------------------------------------------*/
template < class Data_Type >
void MrmsReader::byteswap( Data_Type *data_array, int num_elements )
{
  int num_bytes = sizeof( Data_Type );
  char *temp = new char[ num_bytes ];
  char *char_data;

  for( int i = 0; i < num_elements; i++ )
  {
    char_data = reinterpret_cast< char * >( &data_array[ i ] );

    for( int i = 0; i < num_bytes; i++ )
    {
      temp[ i ] = char_data[ num_bytes - i - 1 ];
    }

    for( int i = 0; i < num_bytes; i++ )
    {
      char_data[ i ] = temp[ i ];
    }
  }
  delete [] temp;
}

// reorders data from North to South to South to North
// East/West increments remain the same
// the data starts in the lower left hand corner 
// instead of the upper left hand corner. 
void MrmsReader::_reOrderNS_2_SN (float *data, int numX, int numY)
{
  int j = 0;
  MemBuf *inData = new MemBuf();

  float *bufPtr = (float *) inData->load 
                     ((void *) data, numX * numY * sizeof (float));

  for (int y = numY - 1; y >= 0; y--) {
    for (int x = 0;  x < numX; x++) {
      data[j] = bufPtr[(y * numX) + x];
      j++;
    }
  }

  delete inData;

}
