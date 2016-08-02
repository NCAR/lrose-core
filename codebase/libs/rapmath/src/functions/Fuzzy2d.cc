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
/**
 * @file Fuzzy2d.cc
 */

#include <toolsa/copyright.h>
#include <rapmath/Fuzzy2d.hh>
#include <toolsa/LogStream.hh>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <iostream>

using std::string;
using std::vector;
using std::ostream;
using std::pair;
using std::map;

const int Fuzzy2d::_maxTokenLen = 80;

static bool _get_before_after(const std::vector<double> xv, 
			      double x, double &x0, double &x1)
{
  int n = xv.size();
  if (n <= 0)
  {
    return false;
  }

  if (x <= xv[0])
  {
    x0 = x1 = xv[0];
    return true;
  }
  else if (x >= xv[n-1])
  {
    x0 = x1 = xv[n-1];
    return true;
  }

  vector< double >::const_iterator after;
  
  for (after = xv.begin(); after != xv.end(); ++after)
  {
    if (x < *after)
    {
      break;
    }
  }
  
  vector< double >::const_iterator before = after-1;
  x0 = *before;
  x1 = *after;
  return true;
}

//------------------------------------------------------------------
static int _stringParse(const char *inpstr, const int nchars,
			const int maxFLen, vector<string> &outstr)
{
  register const char * instr;
  register char * tbuf;
  register int i, endFlag, inProgress;
  int	k = 0;
  char	tmpbuf[1000];
  char  outtmp[1000];

  instr = inpstr;
  tbuf = tmpbuf;
  endFlag = 0;
  inProgress = 0;
  i = 0;

  while(*instr == ' ' || *instr == '\t')
  {
    // skip leading white space
    instr++; 
    i++;
  }

  while(*instr != '\0' && *instr != '\n' && i < nchars )
  {
    if(*instr == ' ' || *instr == '\t' )
    {
      if (inProgress)
      {
	// signal end of field
	endFlag = 1;	
      }
      instr++;
    }
    else
    {
      inProgress = 1;	
      *tbuf++ = *instr++;	// move chars
    }
    i++;

    if(endFlag & inProgress)
    {	
      // tmpbuf is filled, terminate 
      *tbuf = '\0';	
      strncpy(outtmp, tmpbuf, maxFLen-1);
      string s = outtmp;
      outstr.push_back(s);
      k++;

      // reset temp buffer
      tbuf = tmpbuf;
      endFlag = 0;
      inProgress = 0;
    }
  }

  if(inProgress)
  {
    // still something in temp buffer
    *tbuf = '\0';
    strncpy(outtmp, tmpbuf, maxFLen-1);
    string s= outtmp;
    outstr.push_back(s);
    k++;
  }

  return(k);
}

//------------------------------------------------------------------
Fuzzy2d::Fuzzy2d(void)
{
  _ok = false;
}

//------------------------------------------------------------------
Fuzzy2d::~Fuzzy2d()
{
}

//------------------------------------------------------------------
double Fuzzy2d::apply(const double x, const double y) const
{
  if (!_ok)
  {
    return -99.99;
  }

  double xx = x, yy = y;
  // are we off the edge in x or in y?(remember pX and pY are sorted)
  if (xx < _x[0])
  {
    xx = _x[0];
  }
  int nx = (int)_x.size();
  if (xx > _x[nx-1])
  {
    xx = _x[nx-1];
  }
  if (yy < _y[0])
  {
    yy = _y[0];
  }
  int ny = (int)_y.size();
  if (yy > _y[ny-1])
  {
    yy = _y[ny-1];
  }

  bool hasX = find(_x.begin(), _x.end(), xx) != _x.end();
  bool hasY = find(_y.begin(), _y.end(), yy) != _y.end();

  if (hasX && hasY)
  {
    return _getValue(xx, yy);
  }

  
  
  // If we get here, we have to interpolate.
  
  double x0, y0, x1, y1;
  if (_get_before_after(_x, xx, x0, x1) && _get_before_after(_y, yy, y0, y1))
  {
    // get value at 4 corners
    double w00, w01, w10, w11;
    w00 = _getValue(x0, y0);
    w01 = _getValue(x0, y1);
    w10 = _getValue(x1, y0);
    w11 = _getValue(x1, y1);

    double interpx, interpy;
    if (x1 == x0)
    {
      interpx = 1.0;
    }
    else
    {
      interpx = (xx - x0)/(x1-x0);
    }
    if (y1 == y0)
    {
      interpy = 1.0;
    }
    else
    {
      interpy = (yy - y0)/(y1-y0);
    }

    // Calculate the interpolation fraction
    return w11*interpx*interpy + w10*interpx*(1-interpy) +
      w01*(1-interpx)*interpy + w00*(1-interpx)*(1-interpy);
  }
  else
  {
    LOG(ERROR) << "Computing";
    return -99.99;
  }
}
  
//------------------------------------------------------------------
void Fuzzy2d::printTable(void) const
{
  printf("table\n");
  printf("-------------\n");

  vector< double >::const_iterator x, y;
  
  for (x = _x.begin(); x!=_x.end(); ++x)
  {
    char buf[1000];
    string oneline;
    sprintf(buf, "%lf", *x);
    oneline = buf;
    
    for (y = _y.begin(); y!= _y.end(); ++y)
    {
      pair< double, double > index(*x, *y);
      map< pair< double, double>, double >::const_iterator value;
      value = _table.find(index);
      sprintf(buf, "  %lf", value->second);
      oneline += buf;
    }
    printf("%s\n", oneline.c_str());
  }
}

//------------------------------------------------------------------
bool Fuzzy2d::readParmFile(const string &filePath)
{
  // Open the file

  FILE *valuesFile;
  
  if ((valuesFile = fopen(filePath.c_str(), "r")) == 0)
  {
    LOG(ERROR) << "opening values file " << filePath;
    perror(filePath.c_str());
    _ok = false;
    return false;
  }
 
  // Read in the x values
  _readX(valuesFile);

  // Figure out # of tokens per line (the y plus data for all x)
  int expNumTokens = _x.size() + 1;
  
  // read in the remaining lines in the file, skipping comments and empty
  // lines
  char line[BUFSIZ];
  while (fgets(line, BUFSIZ, valuesFile) != 0)
  {
    if (!_parseLine(line, BUFSIZ, expNumTokens))
    {
      fclose(valuesFile);
      LOG(ERROR) << "reading values from file " << filePath;
      _ok = false;
      return false;
    }
  }

  // Close the file and reclaim memory
  fclose(valuesFile);

  // Sort the index vectors so we can do some interpolation between
  // values

  sort(_x.begin(), _x.end());
  sort(_y.begin(), _y.end());
  
  _ok = true;
  return true;
}
  
//------------------------------------------------------------------
void Fuzzy2d::_readX(FILE *fp)
{
  _x.clear();

  // BUFSIZ is a constant defined in <cstdio> 
  char line[BUFSIZ];
  char *token = 0;
  // read the first non comment not blank line (should have the lead times)
  while (fgets(line, BUFSIZ, fp) != 0)
  {
    // Skip comment lines
    if (line[0] == '#' || (line[0] == '/' && line[1] == '/'))
    {
      continue;
    }
    
    // Skip blank lines
    if ((token = strtok(line, " ")) == 0)
    {
      continue;
    }
    else
    {
      // got some tokens
      break;
    }
  }
  
  // each token should be an X value
  while (token != 0)
  {
    _x.push_back(atof(token));
    token = strtok(0, " ");
  }
}

//------------------------------------------------------------------
bool Fuzzy2d::_parseLine(const char *line, const int lineLen,
			 const int expectedNumTokens)
{
  if (line[0] == '#' || (line[0] == '/' && line[1] == '/'))
  {
    // Skip comment lines
    return true;
  }
    
  // Parse the line into tokens.  If there aren't any tokens, it is a
  // blank line and can be skipped.  Otherwise, the first token is the
  // forecast generation time and the remaining tokens are the values for
  // each forecast lead time.
  vector<string> vtokens;
  int numTokens = _stringParse(line, lineLen, _maxTokenLen, vtokens);
  if (numTokens == 0)
  {
    // blank line
    return true;
  }    

  if (numTokens != expectedNumTokens)
  {
    LOG(ERROR) << "parsing tokens on line: '" << line;
    LOG(ERROR) << "Expected " << expectedNumTokens << " got " 
	       << numTokens << " tokens";
    return false;
  }
    
  // Save the values
  double y = atof(vtokens[0].c_str());
  _y.push_back(y);
    
  for (int i = 1; i < numTokens; ++i)
  {
    pair< double, double > index(_x[i-1], y);
    _table[index] = atof(vtokens[i].c_str());
  }
  return true;
}

//------------------------------------------------------------------
double Fuzzy2d::_getValue(const double x, const double y) const
{
  pair< double, double > index(x, y);
    
  map< pair< double, double>, double >::const_iterator valuePair =
    _table.find(index);
    
  if (valuePair == _table.end())
  {
    return 0.0;
  }
    
  return valuePair->second;
}
