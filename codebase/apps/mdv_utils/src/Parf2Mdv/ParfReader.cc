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
#include "ParfReader.hh"
#include <cmath>
#include <cstdlib>

ParfReader::ParfReader(char *filename, Params &parameters)
{
  _parfFileStr = filename;

  _params = parameters;

  _nx = 0;

  _ny = 0;

  _minx = 0;

  _miny = 0;

  _dx = 0.0;

  _dy = 0.0;

  _time = -1;

  _projType = -1;

  _projOriginLat = -9999;

  _projOriginLon = -9999;

  
}

ParfReader::~ParfReader()
{
  for (int i = 0; i < (int) _classPts.size(); i++)
    {
      _classPts[i]->fieldData.erase(_classPts[i]->fieldData.begin(),_classPts[i]->fieldData.end());
    
      delete _classPts[i];
    }
    
  _classPts.erase( _classPts.begin(), _classPts.end());

  for (int i = 0; i < (int) _fieldNames.size(); i++)
    delete[] _fieldNames[i];
  
  _fieldNames.erase(_fieldNames.begin(), _fieldNames.end());

  _minVals.erase(_minVals.begin(), _minVals.end());

}

int ParfReader::readFile()
{
  //
  // registration with procmap
  //
  PMU_force_register("Processing data");

  if (_params.debug)
    cerr << "NcfReader::readFile() : reading " << _parfFileStr.c_str() << endl;

  FILE *fptr;

  if ( (fptr = fopen(_parfFileStr.c_str(), "r")) == NULL)
    {
      cerr << "ParfReader::readFile(): Cannot open file " << _parfFileStr.c_str() << endl;
      return 1;
    }

  char line[1024];

  char word[32];

  bool firstLine = true;

  //
  // Get the projection information
  //
  while( !feof(fptr) && firstLine == true)
    {
      fgets(line, 1024, fptr);

      sscanf( line, "%s", word);
      
      if (_params.debug == Params::DEBUG_VERBOSE)
	cerr << "ParfReader::readFile(): line: " << line << endl;
      
        if (_params.debug == Params::DEBUG_VERBOSE)
	  cerr << "ParfReader::readFile(): word: " << word << endl;
      
      if ( !strcmp(word, "@relation") )
	{
	  _parseGridInfo(line);
	  
	  firstLine = false;
	}
    }

  //
  // Get the names of variables which correspond to columns
  // of data to be mdv fields by the user.
  //
  if (_getFieldNames(fptr))
    {
      cerr << "ParfReader::readFile(): Fatal Error. Exiting" << endl;
      
      return 1;
    }
    
  //
  // Get the columns of data that correspond to desired Mdv fields.
  //
  if (_getFieldData(fptr))
    {
      cerr << "ParfReader::readFile(): Fatal Error. Exiting" << endl;
      
      return 1;
    }

  return 0;
}

int ParfReader:: _getFieldNames(FILE *fptr)
{

  //
  //  The parf file is set up with attribute or variable info 
  //  at the top of the file preceeding the data. Each line which starts with
  //  @attribute or @ignored corresponds to a label for the 
  //  corresponding column of data. The data begins on the line
  //  after the '@data' string.
  //
  bool dataLine = false;

  char line[1024];

  char word[32];

  int attLineCount = 0;

  int colArrayIter = 0;

  //
  // read file headers, stop at data line
  //
  while( !feof(fptr) && dataLine == false)
    {
      fgets(line, 1024, fptr);

      int sret = sscanf( line, "%s", word);

      if (!strcmp(word,"@data"))
	{
	  dataLine = true;
	  continue;
	}

      // 
      // Lets see if we have a line with expected parf header strings
      //
      if ( (!strcmp(word, "@attribute") || !strcmp(word, "@ignored")) 
	   && sret > 0 &&  colArrayIter < _params.cols2field_n)
	{
	  //
	  // Get the field name if the attribute number matches 
	  // one of the columns that the user requests to be 
	  // an mdv field.
	  //
	  if (attLineCount ==  _params._cols2field[colArrayIter])
	    {
	      colArrayIter++;
	      
	      if (_params.debug == Params::DEBUG_VERBOSE)
		{
		  cerr << "ParfReader::readFile(): line: " << line << endl;
		  cerr << "\tattLineCount " << attLineCount << " " 
		       << "\tcolArrayIter " << colArrayIter << endl;
		  
		}
	      
	      char *mdvFieldName = new char[32];

	      //
	      // Search for a quotation mark: " has int value 34
	      //
	      char* firstQuotePtr = strchr(line,34);
	      
	      //
	      // If there are no quotation marks, grab the first word after
	      // '@attribute' for mdv field name
	      //
	      if (firstQuotePtr == NULL)
		{
		  //
		  // Grab the second word
		  //
		  sscanf(line, "%*s %s", mdvFieldName);
		  
		  _fieldNames.push_back(mdvFieldName);
		  
		  
		  if (_params.debug == Params::DEBUG_VERBOSE)
		    cerr << "mdvFieldName " <<  mdvFieldName << endl;
		}
	      else
		{
		  //
		  // We have a first quote so search for the end of the 
		  // quoted string.  End the string by putting in a \0 there.
		  // 
		  char* secondQuotePtr = strrchr(firstQuotePtr + 1,34);
		  
		  if (secondQuotePtr != NULL)
		    secondQuotePtr[0] = '\0';
		  else
		    {
		      //
		      // We shouldnt get here!
		      //
		      cerr << "ParfReader::readFile(): Problem getting field name "
			   << "from attribute!" << endl;
		      
		      delete mdvFieldName;

		      return 1;
		    }
		  
		  //
		  // Record field name
		  // 
		  char tmpName[32];

		  sprintf( tmpName,"%s", firstQuotePtr + 1);
		  
		  //
		  // Eliminate white space from the field name and replace with "_"
		  // 
		  char *tokenPtr;
		  
		  tokenPtr = strtok(tmpName, " ");
		  
		  mdvFieldName[0] = '\0';
	
		  int whiteCount = 0;

		  while (tokenPtr)
		    {
		      if (whiteCount != 0)
			strcat(mdvFieldName,"_");

		      whiteCount++;

		      strcat(mdvFieldName,tokenPtr);
		      
		      tokenPtr = strtok(NULL, " ");
		    }
		 
		  _fieldNames.push_back(mdvFieldName);
		
		  if (_params.debug == Params::DEBUG_VERBOSE)
		    cerr << "\tfieldName: " <<  _fieldNames.back() << "\n"<< endl;
		}
	    }
	  attLineCount++;

	}// end if !strcmp(word,...

    } // end while

  return 0;
}

//
// Grab data from user specified columns. Note that lines exceeding 1024 chars
// are broken with a '&' and continued on the next line. So we check for this
// when reading a parf data line and concatenate as necessary.
//
int ParfReader:: _getFieldData(FILE *fptr)
{

  char line[MAX_DATALINE_LEN];

  //
  // Foreach remaining line in the file get the lat, 
  // lon and the data from user specified columns.
  //
  while(!feof(fptr))
    {
      fgets(line, MAX_DATALINE_LEN, fptr);

      bool endOfLine = false;

      //
      // Check for lines that end in '&' and concatenate with next line.
      //
      while( !endOfLine)
	{
	  //
	  // Get rid of white space at end of line
	  //
	  int strLen = strlen(line);
	 
	  while (isspace(line[strLen - 1]))
	    {
	      line[strLen - 1] = '\0';
	      
	      strLen = strLen - 1;
	    }
	  
	  int totalStrLen = strLen;

	  //
	  // Check for '&'. If it exists get the next line
	  // and concatenate with the first line
	  //
	  if ( line[strLen - 1] == 38 )
	    {
	      line[strLen - 1] = '\0';
      
	      char nextLine[MAX_DATALINE_LEN];

	      fgets(nextLine, MAX_DATALINE_LEN, fptr);
	      
	      int nextStrLen = strlen(nextLine);
	      
	      //
	      // Get rid of white space on the next line
	      // 
	      while( isspace(nextLine[nextStrLen - 1]))
		{
		  nextLine[nextStrLen - 1] = '\0';
		  
		  nextStrLen = nextStrLen - 1;
		}

	      totalStrLen = totalStrLen + nextStrLen;
	      
	      if (totalStrLen > MAX_DATALINE_LEN)
		{
		  cerr << "ParfReader::_getFieldData(): ERROR! Data line length "
		       << "exceeds max line length of " <<  MAX_DATALINE_LEN 
		       << " Check ParfReader.hh for max line length)\n" << endl;
		  
		  exit (1);
		}
	      
	      //
	      // Concatenate with intital line after putting a space
	      // on the initial line since we dont want data running together
	      // 
	      strcat(line, " ");

	      strcat(line, nextLine);
	    }
	  else
	    endOfLine = true;

	}// end while

      classPt *pt = new classPt;

      char *tokenPtr;

      char *lastToken;

      tokenPtr = strtok(line, _params.data_delimiter);

      int count = 0;
     
      int colArrayIter = 0;

      while (tokenPtr)
	{
	  if (_params.debug == Params::DEBUG_VERBOSE)
	    cerr << "ParfReader::readFile(): tokenPtr: " << tokenPtr << endl;

	  if (count == _params._cols2field[colArrayIter])
	    {
	      pt->fieldData.push_back (atof(tokenPtr));

	      colArrayIter++;
	    }
	  
	  //
	  // lat is the fifth element in the line
	  // (counting from 0).
	  //
	  if (count == 5)
	    pt->lat = atof(tokenPtr);

	  //
	  // lon is the sixth element in the line
	  // (counting from zero).
	  //
	  if (count == 6)
	    pt->lon = atof(tokenPtr);
	  
	  lastToken = tokenPtr;

	  tokenPtr = strtok(NULL, _params.data_delimiter);

	  count++;
	}
      
      //
      // If data line is legitimate (at least has cols that are supposed
      // to contain the lat and lon data), save the data
      //
      if ( count  > 6)
	  _classPts.push_back(pt);
      else
	delete pt; 
    }

  if (_params.debug == Params::DEBUG_VERBOSE)
    {
      for (int i = 0; i < (int) _classPts.size(); i++)
	{
	  cerr << "lat: " << _classPts[i]->lat << " lon: " <<  _classPts[i]->lon << " ";

	  for (int j = 0; j < (int)_classPts[i]->fieldData.size(); j++)
	    {
	      cerr << "col(" << _params._cols2field[j] <<  ") " <<  _classPts[i]->fieldData[j] << endl;
	    }
	}      
    }

  _findFieldMinVals();

  return 0;
}

//
// The first line of the file
// 
int ParfReader::_parseGridInfo(char *line)
{
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "ParfReader::_parseGridInfo: Parsing \'" << line << "\' for grid information.\n" << endl;
  
  char *tokenPtr;

  //
  // skip '@relation relationName'
  // 
  tokenPtr = strtok(line, "_");

  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "Skipping: " << tokenPtr << endl;
  
  //
  // Skip projInfo
  //
  tokenPtr = strtok(NULL,"_");

  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "Skipping: " <<  tokenPtr << endl;

  //
  // Get the projection type
  //
  tokenPtr = strtok(NULL,"_");
  
  _projType = atoi(tokenPtr);

  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "_projType = " <<  _projType << endl;

  //
  // Get the projection origin latitude
  //
  tokenPtr = strtok(NULL,"_");
  
  _projOriginLat = atof(tokenPtr);
  
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "_projOriginLat = " <<  _projOriginLat << endl;

  //
  // Get the projection origin latitude
  //
  tokenPtr = strtok(NULL,"_");
  
  _projOriginLon = atof(tokenPtr);

  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "_projOriginLon = " <<  _projOriginLon << endl;

  //
  // Get the projection origin latitude
  //
  tokenPtr = strtok(NULL,"_");
  
  _nx = atoi(tokenPtr);
  
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "_nx = " <<  _nx << endl;

  //
  // Get the projection origin latitude
  //
  tokenPtr = strtok(NULL,"_");
   
  _ny = atoi(tokenPtr);
  
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "_ny = " <<  _ny << endl;
  
  //
  // Get dx
  //
  tokenPtr = strtok(NULL,"_");
  
  _dx = atof(tokenPtr);
  
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "_dx = " <<  _dx << endl;

  //
  // Get dy
  //
  tokenPtr = strtok(NULL,"_");
  
  _dy = atof(tokenPtr);
  
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "_dy = " <<  _dy << endl;
  
  //
  // Get the projection origin latitude
  //
  tokenPtr = strtok(NULL,"_");
  
  _minx = atof(tokenPtr);
  
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "_minx = " <<  _minx << endl;

  //
  // Get the projection origin latitude
  //
  tokenPtr = strtok(NULL,"_");
  
  _miny = atof(tokenPtr);
  
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "_miny = " <<  _miny << endl;
  
  //
  // Get the time  
  //
  tokenPtr = strtok(NULL,"_");
  
  _time = atoi(tokenPtr);
  
  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << "_time = " <<  _time << endl;
  
  return 0;
}

int ParfReader::_findFieldMinVals()
{

  float min;

  for (int j = 0; j < (int)_classPts[0]->fieldData.size(); j++)
    {
      //
      // start by assigning min to first data pt classification
      //
      if ((int) _classPts.size() > 0)
	min = _classPts[0]->fieldData[j];

      //
      // Compare against all other data points in this field
      //
      for (int i = 0; i < (int) _classPts.size(); i++)
	{
	  if (_classPts[i]->fieldData[j] < min )
	    min = _classPts[i]->fieldData[j];
	}
      
      _minVals.push_back( min);
    }
  
  return 0;

}
