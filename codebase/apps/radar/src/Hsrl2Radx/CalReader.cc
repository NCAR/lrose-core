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
///////////////////////////////////////////////////////////////
// CalReader.cc
//
// Brad Schoenrock, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2017
//
///////////////////////////////////////////////////////////////

#include "CalReader.hh"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

//////////////////////////////////////////////////
// read in variables from calibration files
// stores them in a multidimentional array

CalReader CalReader::readCalVals(const char* file, const char* variable)
{

  CalReader dataBlock;
  bool rightBlock=false;//each block of the calval file has the data in the 
  //calvals class, and we want to scan through the correct block and return 
  //just that data. Don't bother returning data from other blocks. 

  std::ifstream infile(file);
  if (!infile) {
    cerr << "INFO - CalReader::readCalVals" << endl;
    cerr << "  CalVals file: " << file << endl;
  } 

  std::string line;
  while (std::getline(infile, line)) {
    std::stringstream ss;
    ss << line;
    
    line=removeWhitespace(line);//remove space from begining or end of line
    
    if(checkForChar("#",line)>-1) {
      //remove comments which begin with the #
      line=line.substr(0, checkForChar("#",line));
    }
    
    line=removeWhitespace(line);//remove space from begining or end of line
      
    if(line.length()>0) {

      //only look at lines with non 0 length
	 
      if( (line.at(0)>='a' && line.at(0)<='z') || 
          (line.at(0)>='A' && line.at(0)<='Z') ) {

        //if the first char is a letter then it is a variable name
	     
        int hasUnits=-1;
	
        hasUnits=checkForChar("(",line); // units are captured in ()
	
        string varName;
        string units;
        
        if(hasUnits==-1) {
          varName=line;
          units="none";
        } else{
          varName=line.substr(0, hasUnits);
          units=line.substr(hasUnits+1,line.length()-hasUnits-2);
        }
	      
        varName=removeWhitespace(varName);
        units=removeWhitespace(units);
	
        if(varName==variable) {
          rightBlock=true;
        } else {
          rightBlock=false;
        }
        
        if(rightBlock) {
          dataBlock.setVarName(varName);
          dataBlock.setVarUnits(units);
        }
	
      } // if( (line.at(0)>='a' ....
	      
      if(rightBlock && line.at(0)>='0' && line.at(0)<='9' ) {
        
        //if the first char is a number then it is a date/data pair
	      
        string date; //extract date
        date=line.substr(0,checkForChar(",",line));
        date=removeWhitespace(date);
	
        int day=0;
        int mon=0;
        int year=0;
        int hour=0;
        int min=0;
        int sec=0;
        
        string temp;//holds each segment of the date string in turn. 
	
        temp=date.substr(0,checkForChar("-",date)); // day
        istringstream ( temp ) >> day;
        date=date.substr(checkForChar("-",date)+1,date.length());
	
        temp=date.substr(0,checkForChar("-",date)); // month
        for (int i=0; temp[i]; i++) {
          //make sure month letters are all lowercase
          temp[i] = tolower(temp[i]);
        }

        if(temp=="jan")
          mon=1;
        if(temp=="feb")
          mon=2;
        if(temp=="mar")
          mon=3;
        if(temp=="apr")
          mon=4;
        if(temp=="may")
          mon=5;
        if(temp=="jun")
          mon=6;
        if(temp=="jul")
          mon=7;
        if(temp=="aug")
          mon=8;
        if(temp=="sep")
          mon=9;
        if(temp=="oct")
          mon=10;
        if(temp=="nov")
          mon=11;
        if(temp=="dec")
          mon=12;

        date=date.substr(checkForChar("-",date)+1,date.length());

        temp=date.substr(0,2); // year, two digit format 
        istringstream ( temp ) >> year;
        year=year+2000;
        date=date.substr(2,date.length());
	
        date=removeWhitespace(date);
	      
        if(date.length()>0) {
          //if there is a time stamp
          temp=date.substr(0,checkForChar(":",date)); // hour
          istringstream ( temp ) >> hour;
          date=date.substr(checkForChar(":",date)+1,date.length());
        }
        if(date.length()>0 && checkForChar(":",date)>-1 ) {
          //if there is a timestamp and it has seconds listed
          temp=date.substr(0,checkForChar(":",date)); // min
          istringstream ( temp ) >> min;
          date=date.substr(checkForChar(":",date)+1,date.length());
        } else if(date.length()>0 && checkForChar(":",date)==-1 ) {
          //if timestamp has no seconds read in the minutes and be done
          temp=date; // min
          istringstream ( temp ) >> min;
          date="";
        }

        if(date.length()>0) {
          temp=date; //sec
          istringstream ( temp ) >> sec;
          date="";
        }
	      	    
        RadxTime dateStamp(year, mon, day, hour, min, sec, 0.0);
        //passing 0.0 for subseconds
	      
        int strData=-1; //hold the position of ' or " if data is a string
        int numData=-1; //hold the position of [ if the data is a number
	
        strData=max(checkForChar("'",line),checkForChar("\"",line));
        //string data is captured in ' quotes or " quotes
        numData=checkForChar("[",line);
        //numbers are captured in []	    
	
        string strValue;
        vector<double> numValue;//check numValue.size() to see its length 
        //if length is 0 there is no numerical data and is string data. 
	
        if(strData==-1 && numData==-1) {
          cerr<<"WARNING: calvals_gvhsrl has an improperly formmated line with a date but no data."<<'\n';
          strValue="";//don't want errors it just gives an empty result
        } else if(strData > -1 && numData > -1) {
          cerr<<"WARNING: calvals_gvhsrl has an improperly formmated line with a date and numerical data and string typed data."<<'\n';
          strValue="";//don't want errors it just gives an empty result
        } else if(strData > -1) {
          strValue=line.substr(strData+1,line.length()-strData-2);
        } else if(numData > -1) {
          strValue="";//don't want errors it just gives an empty result
          string numStrTemp;
          numStrTemp = line.substr(numData+1,line.length()-numData);
          //we need the start and end value for the numbers, 
          //and any multipliers that come after. 
          //numStrTemp does still have the ] but not the [.
          numStrTemp=removeWhitespace(numStrTemp);
          
          double hasComma;
          hasComma = checkForChar(",",numStrTemp);
          //this finds the first , which delimits different numbers. 
          
          string temp;
          double data;
          
          while(hasComma>-1) {
            temp=numStrTemp.substr(0,hasComma);
            istringstream ( temp ) >> data;
            numValue.push_back(data);
            numStrTemp=numStrTemp.substr(hasComma+1,numStrTemp.length());
            numStrTemp=removeWhitespace(numStrTemp);
            //find the next comma
            hasComma = checkForChar(",",numStrTemp);
          }
		  
          double endNumData;
          endNumData = checkForChar("]",numStrTemp);
          // ] is not always the end of the string, multipliers are used
		  
          temp=numStrTemp.substr(0,endNumData);
          istringstream ( temp ) >> data;
          numValue.push_back(data);
          numStrTemp=numStrTemp.substr(endNumData+1,numStrTemp.length());
          
          numStrTemp=removeWhitespace(numStrTemp);
          
          if(numStrTemp.length()>0) {
            double mod=1;
            if(checkForChar("/",numStrTemp)>-1) {
              numStrTemp=numStrTemp.substr(1,numStrTemp.length());
              istringstream ( numStrTemp ) >> mod;
              for(unsigned int i=0; i<numValue.size(); i++) {
                numValue.at(i)=numValue.at(i)/mod;
              }
            } else if(checkForChar("*",numStrTemp)>-1) {
              numStrTemp=numStrTemp.substr(1,numStrTemp.length());
              istringstream ( numStrTemp ) >> mod;
              for(unsigned int i=0; i<numValue.size(); i++) {
                numValue.at(i)=numValue.at(i)*mod;
              }
            } else {
              cerr << "WARNING: calvals_gvhsrl has an improperly formatted "
                   << "line with non-modifier text after numerical data." 
                   << '\n';
            }
          } // if(numStrTemp.length()>0)
		  		 
        }
        
        if(rightBlock) {
          dataBlock.addTime(dateStamp);
          if(strData > -1) {
            dataBlock.setIsStr();
            dataBlock.addDataStr(strValue);
          }
          if(numData > -1) {
            dataBlock.setIsNum();
            dataBlock.addDataNum(numValue);
          }
        }
	
      } // if(rightBlock && line.at(0)>='0'
      //if the first char is a number then it is a date/data pair, 
      //end that processing
	  
    } //end line length check
    
  } //end reading calvals file line by line

  dataBlock=sortTime(dataBlock);
  
  return dataBlock;
  
}// end of _readCalvals function 


//this function removes spaces from the begining and end of strings

string CalReader::removeWhitespace(string s)
{
  while(s.length()>0 && (s.substr(0, 1)==" ") ) {
    s=s.substr(1,s.length());
  }
  while(s.length()>0 && (s.substr(s.length()-1, 1)==" ") ) {
    s=s.substr(0,s.length()-1);
  }
  return s;
}


int CalReader::checkForChar(string subSt, string str)
//checks string for a particular substring and returns the location of the 
//start of that substring, returns -1 if not found. 
{
  for(unsigned int i=0;i<str.length();i++) {
    if( (str.substr(i, subSt.length())==subSt) ) {
      return i;
    }
  }
  
  return -1;
}


////////////////////////////////////////////////////////////  
// constructors

CalReader::CalReader()
{}

// constructor for string type data

CalReader::CalReader(string inName,
                     string inUnits,
                     vector< RadxTime > inTime, 
		     vector<string> inDataStr)
{
  varName=inName;
  varUnits=inUnits;
  time=inTime;
  dataStr=inDataStr;
  isStr=true;
  isNum=false;
}

// constructor for num type data

CalReader::CalReader(string inName,
                     string inUnits,
                     vector< RadxTime > inTime, 
		     vector< vector<double> > inDataNum)
{
  varName=inName;
  varUnits=inUnits;
  time=inTime;
  dataNum=inDataNum;
  isStr=false;
  isNum=true;
}

void CalReader::setVarName(string inName)
{
  varName=inName;
}

void CalReader::setVarUnits(string inUnits)
{
  varUnits=inUnits;
}

void CalReader::setTime(vector<RadxTime> inTime)
{
  time=inTime;
}

void CalReader::addTime(RadxTime inTime)
{
  time.push_back(inTime);
}

void CalReader::setDataStr(vector<string> inDataStr)
{
  if(isStr) {
    dataStr=inDataStr;
  }
}

void CalReader::addDataStr(string inDataStr)
{
  if(isStr) {
    dataStr.push_back(inDataStr);
  }
}

void CalReader::setDataNum(vector< vector<double> > inDataNum)
{
  if(isNum) {
    dataNum=inDataNum;
  }
}

void CalReader::addDataNum(vector<double> inDataNum)
{
  if(isNum) {
    dataNum.push_back(inDataNum);
  }
}

void CalReader::setIsStr()
{
  isStr=true;
  isNum=false;
}

void CalReader::setIsNum()
{
  isNum=true;
  isStr=false;
}

string CalReader::getVarName()
{
  return varName;
}

string CalReader::getVarUnits()
{
  return varUnits;
}

vector<RadxTime> CalReader::getTime()
{
  return time;
}

vector<string> CalReader::getDataStr()
{
  return dataStr;
}

vector< vector<double> > CalReader::getDataNum()
{
  return dataNum;
}

bool CalReader::dataTypeisNum()
{
  assert(isStr!=isNum);
  //data should be either string type or numbers, not both or neither. 
  return isNum;    
}

bool CalReader::dataTypeisStr()
{
  assert(isStr!=isNum);
  //data should be either string type or numbers, not both or neither. 
  return isStr;    
}

void CalReader::printBlock()
{

  cout<<"Printing out block of CalReader data."<<'\n';
  cout<<varName<<'\n';
  cout<<varUnits<<'\n';
  
  cout<<"time.size()="<<time.size()<<'\n';
  cout<<"dataNum.size()="<<dataNum.size()<<'\n';
  cout<<"dataStr.size()="<<dataStr.size()<<'\n';

  
  if(isNum) {
    cout<<"numerical type data"<<'\n'; 
    assert(time.size()==dataNum.size());
  }
  if(isStr) {
    cout<<"string type data"<<'\n'; 
    assert(time.size()==dataStr.size());
  }

  for(unsigned int i=0;i<time.size();i++) {
    cout<<time.at(i)<<'\n';
    
    if(isStr) {
      cout<<dataStr.at(i)<<'\n';
    }
      
    if(isNum) {
      for(unsigned int j=0;j<(dataNum.at(i)).size();j++) {
        cout<<(dataNum.at(i)).at(j)<<" ";
      }
      cout<<'\n';
    }
    
  } // i

}
  
CalReader CalReader::sortTime(CalReader toSort)
{

  for(unsigned int i=0; i<(toSort.time).size()-1;i++) {  

    for(unsigned int j=0; j<(toSort.time).size()-1;j++) {
      
      if( ((toSort.time).at(j)).asDouble() < 
          ((toSort.time).at(j+1)).asDouble() ) {

        RadxTime tempT=(toSort.time).at(j);
        (toSort.time).at(j)=(toSort.time).at(j+1);
        (toSort.time).at(j+1)=tempT;
	      
        if(toSort.isNum) {
          assert( (toSort.dataNum).size() == (toSort.time).size() );
          vector<double> tempD=(toSort.dataNum).at(j);
          (toSort.dataNum).at(j)=(toSort.dataNum).at(j+1);
          (toSort.dataNum).at(j+1)=tempD;
        }
        if(toSort.isStr) {
          assert( (toSort.dataStr).size() == (toSort.time).size() );
          string tempS=(toSort.dataStr).at(j);
          (toSort.dataStr).at(j)=(toSort.dataStr).at(j+1);
          (toSort.dataStr).at(j+1)=tempS;
        }
 
      } // if

    } // j

  } // i
    
  return toSort; 
  
}


int CalReader::dateMatch(CalReader calIn, RadxTime check)
{
  
  for(unsigned int i=0;i<(calIn.time).size();i++) {
    if(check > (calIn.time).at(i)) {
      return i;
    }
  }
  return 0;
  
}

// destructor

CalReader::~CalReader()
{
}

