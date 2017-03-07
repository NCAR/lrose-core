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
#include "Hsrl2Radx.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxArray.hh>
#include <Radx/NcfRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/pmu.h>
#include <physics/IcaoStdAtmos.hh>
#include "MslFile.hh"
#include "RawFile.hh"

#include <fstream>
#include <sstream>
#include <string>

using namespace std;


//////////////////////////////////////////////////
// read in variables from calibration files
// stores them in a multidimentional array


//void CalReader::ReadCalvals(string pathToCalValsFile, timet time)
vector <vector<double> > CalReader::readBaselineCorrection(const char* file, bool debug)
{
  if(debug)
    cout<< "in readBaselineCorrection"<<'\n';
  
  std::ifstream infile(file);
  std::string line;
  vector<double> vec_binnum;
  vector<double> vec_combined_hi;
  vector<double> vec_combined_lo;
  vector<double> vec_molecular;
  vector<double> vec_crosspol;
  vector<double> vec_mol_I2A;
  vector<double> vec_comb_1064;

  while (std::getline(infile, line))
    {
      std::stringstream ss;
      ss << line;
      if(debug)
	cout<<line<<'\n';
      
      string test = "#";
            
      if(!(line.substr(0, test.length())==test))//comments at begining of file begin with #, ignore those lines and process the rest. 
	{
	  double binnum; 
	  ss>>binnum;
	  vec_binnum.push_back(binnum);
	  if(debug)
	    cout<<"binnum="<<binnum<<'\n';

	  double combined_hi; 
	  ss>>combined_hi;
	  vec_combined_hi.push_back(combined_hi);
	  if(debug)
	    cout<<"combined_hi="<<combined_hi<<'\n';

	  double combined_lo; 
	  ss>>combined_lo;
	  vec_combined_lo.push_back(combined_lo);
	  if(debug)
	    cout<<"combined_lo="<<combined_lo<<'\n';

	  double molecular; 
	  ss>>molecular;
	  vec_molecular.push_back(molecular);
	  if(debug)
	    cout<<"molecular="<<molecular<<'\n';

	  double crosspol; 
	  ss>>crosspol;
	  vec_crosspol.push_back(crosspol);
	  if(debug)
	    cout<<"crosspol="<<crosspol<<'\n';

	  double mol_I2A; 
	  ss>>mol_I2A;
	  vec_mol_I2A.push_back(mol_I2A);
	  if(debug)
	    cout<<"mol_I2A="<<mol_I2A<<'\n';

	  double comb_1064; 
	  ss>>comb_1064;
	  vec_comb_1064.push_back(comb_1064);
	  if(debug)
	    cout<<"comb_1064="<<comb_1064<<'\n';
	}
    }
 
  vector< vector<double> > ans;
  ans.push_back(vec_binnum);
  ans.push_back(vec_combined_hi);
  ans.push_back(vec_combined_lo);
  ans.push_back(vec_molecular);
  ans.push_back(vec_crosspol);
  ans.push_back(vec_mol_I2A);
  ans.push_back(vec_comb_1064);

  return ans;

}

vector <vector<double> > CalReader::readDiffDefaultGeo(const char* file, bool debug)
{
  if(debug)
    cout<< "in _readDiffDefaultGeo"<<'\n';
  std::ifstream infile(file);
  std::string line;
  vector<double> vec_altitudes;
  vector<double> vec_comb_himol;
  vector<double> vec_comb_lomol;
  vector<double> vec_scomb_himol;
  vector<double> vec_scomb_lomol;

  while (std::getline(infile, line))
    {
      std::stringstream ss;
      ss << line;
      if(debug)
	cout<<line<<'\n';

      string test = "#";
            
      if(!(line.substr(0, test.length())==test))//comments at begining of file begin with #, ignore those lines and process the rest. 
	{
	  double altitudes; 
	  ss>>altitudes;
	  vec_altitudes.push_back(altitudes);
	  if(debug)
	    cout<<"altitudes="<<altitudes<<'\n';
	  
	  double comb_himol; 
	  ss>>comb_himol;
	  vec_comb_himol.push_back(comb_himol);
	  if(debug)
	    cout<<"comb_himol="<<comb_himol<<'\n';
	  
	  double comb_lomol; 
	  ss>>comb_lomol;
	  vec_comb_lomol.push_back(comb_lomol);
	  if(debug)
	    cout<<"comb_lomol="<<comb_lomol<<'\n';
	  	  	  
	  double scomb_himol; 
	  ss>>scomb_himol;
	  vec_scomb_himol.push_back(scomb_himol);
	  if(debug)
	    cout<<"scomb_himol="<<scomb_himol<<'\n';
	  
	  double scomb_lomol; 
	  ss>>scomb_lomol;
	  vec_scomb_lomol.push_back(scomb_lomol);
	  if(debug)
	    cout<<"scomb_lomol="<<scomb_lomol<<'\n';
	  
	}
    }

  vector< vector<double> > ans;
  ans.push_back(vec_altitudes);
  ans.push_back(vec_comb_himol);
  ans.push_back(vec_comb_lomol);
  ans.push_back(vec_scomb_himol);
  ans.push_back(vec_scomb_lomol);

  return ans;

} 

vector <vector<double> > CalReader::readGeofileDefault(const char* file, bool debug)
{
  if(debug)
    cout<< "in _readGeofileDefault"<<'\n';
  std::ifstream infile(file);
  std::string line;
  vector<double> vec_range;
  vector<double> vec_geo_corr;

  while (std::getline(infile, line))
    {
      std::stringstream ss;
      ss << line;
      if(debug)
	cout<<line<<'\n';
       
      string test = "#";
            
      if(!(line.substr(0, test.length())==test))//comments at begining of file begin with #, ignore those lines and process the rest. 
	{
	  double range; 
	  ss>>range;
	  vec_range.push_back(range);
	  if(debug)
	    cout<<"range="<<range<<'\n';

	  double geo_corr; 
	  ss>>geo_corr;
	  vec_geo_corr.push_back(geo_corr);
	  if(debug)
	    cout<<"geo_corr="<<geo_corr<<'\n';
	  
	}
    }

  vector< vector<double> > ans;
  ans.push_back(vec_range);
  ans.push_back(vec_geo_corr);

  return ans;

} 

vector <vector<double> > CalReader::readAfterPulse(const char* file, bool debug)
{
  if(debug)
    cout<< "in readAfterPulse"<<'\n';
  std::ifstream infile(file);
  std::string line;
  vector<double> vec_bin;
  vector<double> vec_mol;
  vector<double> vec_comb;
  vector<double> vec_crossPol;
  vector<double> vec_refftMol;
  vector<double> vec_imfftMol;
  vector<double> vec_refftComb;
  vector<double> vec_imfftComb;
  vector<double> vec_refftCPol;
  vector<double> vec_imfftCPol;

  while (std::getline(infile, line))
    {
      std::stringstream ss;
      ss << line;
      if(debug)
	cout<<line<<'\n';
       
      string test = "#";
            
      if(!(line.substr(0, test.length())==test))//comments at begining of file begin with #, ignore those lines and process the rest. 
	{
	  double bin; 
	  ss>>bin;
	  vec_bin.push_back(bin);
	  if(debug)
	    cout<<"bin="<<bin<<'\n';

	  double mol; 
	  ss>>mol;
	  vec_mol.push_back(mol);
	  if(debug)
	    cout<<"mol="<<mol<<'\n';

	  double comb; 
	  ss>>comb;
	  vec_comb.push_back(comb);
	  if(debug)
	    cout<<"comb="<<comb<<'\n';

	  double crossPol; 
	  ss>>crossPol;
	  vec_crossPol.push_back(crossPol);
	  if(debug)
	    cout<<"crossPol="<<crossPol<<'\n';

	  double refftMol; 
	  ss>>refftMol;
	  vec_refftMol.push_back(refftMol);
	  if(debug)
	    cout<<"refftMol="<<refftMol<<'\n';

	  double imfftMol; 
	  ss>>imfftMol;
	  vec_imfftMol.push_back(imfftMol);
	  if(debug)
	    cout<<"imfftMol="<<imfftMol<<'\n';

	  double refftComb; 
	  ss>>refftComb;
	  vec_refftComb.push_back(refftComb);
	  if(debug)
	    cout<<"refftComb="<<refftComb<<'\n';

	  double imfftComb; 
	  ss>>imfftComb;
	  vec_imfftComb.push_back(imfftComb);
	  if(debug)
	    cout<<"imfftComb="<<imfftComb<<'\n';

	  double refftCPol; 
	  ss>>refftCPol;
	  vec_refftCPol.push_back(refftCPol);
	  if(debug)
	    cout<<"refftCPol="<<refftCPol<<'\n';

	  double imfftCPol; 
	  ss>>imfftCPol;
	  vec_imfftCPol.push_back(imfftCPol);
	  if(debug)
	    cout<<"imfftCPol="<<imfftCPol<<'\n';

	}
    }

  vector< vector<double> > ans;
  ans.push_back(vec_bin);
  ans.push_back(vec_mol);
  ans.push_back(vec_comb);
  ans.push_back(vec_crossPol);
  ans.push_back(vec_refftMol);
  ans.push_back(vec_imfftMol);
  ans.push_back(vec_refftComb);
  ans.push_back(vec_imfftComb);
  ans.push_back(vec_refftCPol);
  ans.push_back(vec_imfftCPol);

  return ans;

} 

CalReader CalReader::readCalVals(const char* file, const char* variable, bool debug)
{
  CalReader dataBlock;
  bool rightBlock=false;//each block of the calval file has the data in the calvals class, and we want to scan through the correct block and return just that data. Don't bother returning data from other blocks. 
  if(debug)
    cout<< "in readCalvals"<<'\n';
  std::ifstream infile(file);
  std::string line;
  while (std::getline(infile, line))
    {
      std::stringstream ss;
      ss << line;
      
      line=removeWhitespace(line);//removes whitespace from begining or end of line

      if(checkForChar("#",line)>-1)// now we remove comments which are delimited by the #
	line=line.substr(0, checkForChar("#",line));
      
      line=removeWhitespace(line);//removes whitespace from begining or end of line
      
      if(line.length()>0)//now that we've cleaned up the line, only want to bother with it if it has non-zero length. 
	{
	  if(debug)
	    cout<<line<<'\n';
	  
	  if( (line.at(0)>='a' && line.at(0)<='z') || (line.at(0)>='A' && line.at(0)<='Z') )//if the first char is a letter then it is a variable name
	    {
	      if(debug)
		cout<<"starts with letter"<<'\n';
	      
	      int hasUnits=-1;
	      
	      hasUnits=checkForChar("(",line); // units are captured in ()
	      
	      string varName;
	      string units;

	      if(hasUnits==-1)
		{
		  varName=line;
		  units="none";
		}
	      else
		{
		  varName=line.substr(0, hasUnits);
		  units=line.substr(hasUnits+1,line.length()-hasUnits-2);
		}
	      
	      varName=removeWhitespace(varName);
	      units=removeWhitespace(units);
	      
	      if(varName==variable)
		rightBlock=true;
	      else
		rightBlock=false;

	      if(rightBlock)
		{
		  dataBlock.setVarName(varName);
		  dataBlock.setVarUnits(units);
		}
	      if(debug)
		{
		  cout<<"line="<<line<<'\n';
		  cout<<"varName="<<varName<<'\n';
		  cout<<"units="<<units<<'\n';
		}
	    }
	      
	  if(rightBlock && line.at(0)>='0' && line.at(0)<='9' )//if the first char is a number then it is a date/data pair
	    {
	      if(debug)
		cout<<"starts with number"<<'\n';
	     
	      string date; //extract date
	      date=line.substr(0,checkForChar(",",line));
	      date=removeWhitespace(date);
	      
	      if(debug)
		cout<<"date="<<date<<'\n';
	      
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
	      for (int i=0; temp[i]; i++) //make sure month letters are all lowercase
		temp[i] = tolower(temp[i]);

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
	      
	      if(date.length()>0)//if there is a time stamp
		{
		  temp=date.substr(0,checkForChar(":",date)); // hour
		  istringstream ( temp ) >> hour;
		  date=date.substr(checkForChar(":",date)+1,date.length());
		}
	      if(date.length()>0 && checkForChar(":",date)>-1 )//if there is a timestamp and it has seconds listed
		{  
		  temp=date.substr(0,checkForChar(":",date)); // min
		  istringstream ( temp ) >> min;
		  date=date.substr(checkForChar(":",date)+1,date.length());
		}
	      else if(date.length()>0 && checkForChar(":",date)==-1 )//if there is a timestamp without seconds listed read in the minutes and be done
		{
		  temp=date; // min
		  istringstream ( temp ) >> min;
		  date="";
		}
	      if(date.length()>0)
		{
		  temp=date; //sec
		  istringstream ( temp ) >> sec;
		  date="";
		}
	      	    
       	      if(debug)
		cout<<"day="<<day<<"  :  mon="<<mon<<"  :  year="<<year<<"  :  hour="<<hour<<"  :  min="<<min<<"  :  sec="<<sec<<'\n';
	      
	      RadxTime dateStamp(year, mon, day, hour, min, sec, 0.0);//passing 0.0 for subseconds
	      
	      if(debug)
		cout<<"dateStamp="<<dateStamp<<'\n';
	      
	      int strData=-1; // will hold the position of the ' or " if the data is a string
	      int numData=-1; // will hold the position of the [ if the data is a number
	      
	      strData=max(checkForChar("'",line),checkForChar("\"",line));//string data is captured in ' quotes or " quotes
	      numData=checkForChar("[",line);//numbers are captured in []	    
	      	
	      if(debug)
		cout<<"strData"<<strData<<'\n';
	      if(debug)
		cout<<"numData"<<numData<<'\n';
		  
	      string strValue;
	      vector<double> numValue;//check numValue.size() to see its length, if length is 0 there is no numerical data and is string data. 
	   
	      if(strData==-1 && numData==-1)
		{
		  cerr<<"WARNING: calvals_gvhsrl has an improperly formmated line with a date but no data."<<'\n';
		  strValue="";//don't want errors it just gives an empty result
		}
	      else if(strData > -1 && numData > -1)
		{
		  cerr<<"WARNING: calvals_gvhsrl has an improperly formmated line with a date and numerical data and string typed data."<<'\n';
		  strValue="";//don't want errors it just gives an empty result
		}
	      else if(strData > -1)
		{
		  strValue=line.substr(strData+1,line.length()-strData-2);
		}
	      else if(numData > -1)
		{
		  strValue="";//don't want errors it just gives an empty result
		  string numStrTemp;
		  numStrTemp = line.substr(numData+1,line.length()-numData);
		  //we need the start and end value for the numbers, and any multipliers that come after. numStrTemp does still have the ] but not the [.
		  numStrTemp=removeWhitespace(numStrTemp);
		  
		  double hasComma;
		  hasComma = checkForChar(",",numStrTemp);//this finds the first instance of , which delimits different numbers. 
		  
		  string temp;
		  double data;
		  
		  while(hasComma>-1)
		    {
		      temp=numStrTemp.substr(0,hasComma);
		      
		      istringstream ( temp ) >> data;
		      numValue.push_back(data);
		      
		      numStrTemp=numStrTemp.substr(hasComma+1,numStrTemp.length());
		      numStrTemp=removeWhitespace(numStrTemp);
		      
		      hasComma = checkForChar(",",numStrTemp);//this finds the first instance of , which delimits different numbers. 
		    }
		  
		  double endNumData;
		  endNumData = checkForChar("]",numStrTemp);// the ] is at endNumData which is not nessicarily the end of the string
		  
		  temp=numStrTemp.substr(0,endNumData);
		  istringstream ( temp ) >> data;
		  numValue.push_back(data);
		  numStrTemp=numStrTemp.substr(endNumData+1,numStrTemp.length());
		  
		  numStrTemp=removeWhitespace(numStrTemp);
		 
		  if(numStrTemp.length()>0)
		    {
		      double mod=1;
		      if(checkForChar("/",numStrTemp)>-1)
			{
			  numStrTemp=numStrTemp.substr(1,numStrTemp.length());
			  istringstream ( numStrTemp ) >> mod;
			  for(unsigned int i=0; i<numValue.size(); i++)
			    numValue.at(i)=numValue.at(i)/mod;
			}
		      else if(checkForChar("*",numStrTemp)>-1)
			{
			  numStrTemp=numStrTemp.substr(1,numStrTemp.length());
			  istringstream ( numStrTemp ) >> mod;
			  for(unsigned int i=0; i<numValue.size(); i++)
			    numValue.at(i)=numValue.at(i)*mod;
			}
		      else
			{
			  cerr<<"WARNING: calvals_gvhsrl has an improperly formmated line with non-modifier text after numerical data."<<'\n';
			}
		    }
		  
		  if(debug)
		    {
		      cout<<"checking data reading"<<'\n';
		      for(unsigned int i=0;i<numValue.size();i++)
			cout<<numValue.at(i)<<'\n';;
		    }
		}
	      
	      if(rightBlock)
		{
		  dataBlock.addTime(dateStamp);
		  
		  if(strData > -1)
		    {
		      dataBlock.setIsStr();
		      dataBlock.addDataStr(strValue);
		    }
		  if(numData > -1)
		    {
		      dataBlock.setIsNum();
		      dataBlock.addDataNum(numValue);
		    }
		}
	      
	    }//if the first char is a number then it is a date/data pair, end that processing
	  
	}//end line length check
    
    }//end reading calvals file line by line

  if(debug)
    dataBlock.printBlock();

  return dataBlock;

}// end of _readCalvals function 





string CalReader::removeWhitespace(string s)//this function removes spaces from the begining and end of strings
{
  while(s.length()>0 && (s.substr(0, 1)==" ") )
    s=s.substr(1,s.length());
  while(s.length()>0 && (s.substr(s.length()-1, 1)==" ") )
    s=s.substr(0,s.length()-1);
  return s;
}


int CalReader::checkForChar(string subSt, string str)//checks string for a particular substring and returns the location of the start of that substring, returns -1 if not found. 
{
  for(unsigned int i=0;i<str.length();i++)
    if( (str.substr(i, subSt.length())==subSt) )
      return i;
  
  return -1;
}

  


CalReader::CalReader()
{}

CalReader::CalReader(string inName, string inUnits, vector< RadxTime > inTime, vector<string> inDataStr)//constructor for string type data
{
  varName=inName;
  varUnits=inUnits;
  time=inTime;
  dataStr=inDataStr;
  isStr=true;
  isNum=false;
}

CalReader::CalReader(string inName, string inUnits, vector< RadxTime > inTime, vector< vector<double> > inDataNum)////constructor for num type data
{
  varName=inName;
  varUnits=inUnits;
  time=inTime;
  dataNum=inDataNum;
  isStr=false;
  isNum=true;
}

void CalReader::setVarName(string inName)
{varName=inName;}

void CalReader::setVarUnits(string inUnits)
{varUnits=inUnits;}

void CalReader::setTime(vector<RadxTime> inTime)
{time=inTime;}

void CalReader::addTime(RadxTime inTime)
{time.push_back(inTime);}

void CalReader::setDataStr(vector<string> inDataStr)
{
  if(isStr)
    dataStr=inDataStr;
}

void CalReader::addDataStr(string inDataStr)
{
  if(isStr)
    dataStr.push_back(inDataStr);
}

void CalReader::setDataNum(vector< vector<double> > inDataNum)
{
  if(isNum)
    dataNum=inDataNum;
}

void CalReader::addDataNum(vector<double> inDataNum)
{
  if(isNum)
    dataNum.push_back(inDataNum);
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
{return varName;}

string CalReader::getVarUnits()
{return varUnits;}

vector<RadxTime> CalReader::getTime()
{return time;}

vector<string> CalReader::getDataStr()
{return dataStr;}

vector< vector<double> > CalReader::getDataNum()
{return dataNum;}

bool CalReader::dataTypeisNum()
{
  assert(isStr!=isNum);//data should be either string type or numbers, not both or neither. 
  return isNum;    
}

bool CalReader::dataTypeisStr()
{
  assert(isStr!=isNum);//data should be either string type or numbers, not both or neither. 
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

  
  if(isNum)
    {
      cout<<"numerical type data"<<'\n'; 
      assert(time.size()==dataNum.size());
    }
  if(isStr)
    {
      cout<<"string type data"<<'\n'; 
      assert(time.size()==dataStr.size());
    }

  for(unsigned int i=0;i<time.size();i++)
    {
      cout<<time.at(i)<<'\n';
      
      if(isStr)
	cout<<dataStr.at(i)<<'\n';
      
      if(isNum)
	{
	  for(unsigned int j=0;j<(dataNum.at(i)).size();j++)
	    cout<<(dataNum.at(i)).at(j)<<" ";
	  cout<<'\n';
	}
	
    }

}
  
CalReader CalReader::sortTime(CalReader toSort, bool debug)
{
    
  for(unsigned int i=0; i<(toSort.time).size()-1;i++)
    {  
      for(unsigned int j=0; j<(toSort.time).size()-1;j++)
	{
	  if( ((toSort.time).at(j)).asDouble() < ((toSort.time).at(j+1)).asDouble() )
	    {
	      RadxTime tempT=(toSort.time).at(j);
	      (toSort.time).at(j)=(toSort.time).at(j+1);
	      (toSort.time).at(j+1)=tempT;
	      
	      if(toSort.isNum)
		{
		  assert( (toSort.dataNum).size() == (toSort.time).size() );
		  vector<double> tempD=(toSort.dataNum).at(j);
		  (toSort.dataNum).at(j)=(toSort.dataNum).at(j+1);
		  (toSort.dataNum).at(j+1)=tempD;
		}
	      if(toSort.isStr)
		{
		  assert( (toSort.dataStr).size() == (toSort.time).size() );
		  string tempS=(toSort.dataStr).at(j);
		  (toSort.dataStr).at(j)=(toSort.dataStr).at(j+1);
		  (toSort.dataStr).at(j+1)=tempS;
		}
 
	    }
	}
    }
  
  if(debug)
    {
      toSort.printBlock();
    }
  
  return toSort; 
  
}


int CalReader::dateMatch(CalReader calIn, RadxTime check)
{
  
  for(unsigned int i=0;i<(calIn.time).size();i++)
    {
      if(check > (calIn.time).at(i))
	return i;
    }
  return 0;
  
}


CalReader::~CalReader()
{}

