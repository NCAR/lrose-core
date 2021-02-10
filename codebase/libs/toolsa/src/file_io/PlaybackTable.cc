// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file PlaybackTable.cc 
 */

#include <toolsa/PlaybackTable.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <cstdio>
#include <algorithm>
#include <sys/stat.h>
using std::vector;
using std::string;

const std::string PlaybackTable::_lineTag = "Line";
const std::string PlaybackTable::Line::_fileTag = "File";
const std::string PlaybackTable::Line::_twrittenTag = "Twritten";
const std::string PlaybackTable::Line::_isObsTag = "IsObs";
const std::string PlaybackTable::Line::_dirTag = "Dir";
const std::string PlaybackTable::Line::_gentimeTag = "Gentime";
const std::string PlaybackTable::Line::_leadsecondsTag = "LeadSeconds";

//------------------------------------------------------------------
static bool _loadFile(const string &fname, string &inp)
{
  struct stat statBuf;
  if (stat(fname.c_str(), &statBuf) != 0)
  {
    LOG(ERROR) << "Cannot stat file " << fname;
    return false;
  }
  int fileLen = static_cast<int>(statBuf.st_size);
  
  // open file
  FILE *in;
  if ((in = fopen(fname.c_str(), "r")) == NULL)
  {
    LOG(ERROR) << "Cannot open file " << fname;
    return false;
  }

  // read in buffer
  char *fileBuf = new char[fileLen + 1];
  if (static_cast<int>(fread(fileBuf, 1, fileLen, in)) != fileLen)
  {
    LOG(ERROR) << "Cannot read file " << fname;
    fclose(in);
    delete[] fileBuf;
    return false;
  }

  // ensure null termination
  fileBuf[fileLen] = '\0';

  // close file
  fclose(in);

  // put into the string
  inp = fileBuf;
  delete[] fileBuf;
  return true;
}

//----------------------------------------------------------------------
PlaybackTable::PlaybackTable(void): _ok(true)
{
}

//----------------------------------------------------------------------
PlaybackTable::PlaybackTable(const std::string &fileName) : _ok(true)
{
  string str;
  _ok = _loadFile(fileName, str);
  if (!_ok)
  {
    return;
  }     
  vector<string> lines;
  if (TaXml::readStringArray(str, _lineTag, lines) != 0)
  {
    LOG(ERROR) << "reading tag '" << _lineTag << "' in file " << fileName;
    _ok = false;
    return;
  }
  for (size_t i=0; i<lines.size(); ++i)
  {
    PlaybackTable::Line line(lines[i]);
    if (line.ok())
    {
      _files.push_back(line);
    }
    else
    {
      LOG(WARNING) << "bad line in file";
    }
  }
}

//----------------------------------------------------------------------
PlaybackTable::~PlaybackTable(void)
{
}

//----------------------------------------------------------------------
void PlaybackTable::print(void) const
{
  for (size_t i=0; i<_files.size(); ++i)
  {
    _files[i].print();
  }
}

//----------------------------------------------------------------------
void PlaybackTable::merge(const PlaybackTable &f)
{
  for (size_t i=0; i<f._files.size(); ++i)
  {
    _files.push_back(f._files[i]);
  }
  sortFiles();
}

//----------------------------------------------------------------------
void PlaybackTable::append(const std::string &fileName,
			     const time_t &twritten,
			     const std::string &dir, const time_t &gentime)
{
  _files.push_back(Line(fileName, twritten, dir, gentime));
}

//----------------------------------------------------------------------
void PlaybackTable::append(const std::string &fileName,
			     const time_t &twritten,
			     const std::string &dir, const time_t &gentime,
			     int leadseconds)
{
  _files.push_back(Line(fileName, twritten, dir, gentime, leadseconds));
}

//----------------------------------------------------------------------
void PlaybackTable::sortFiles(void)
{
  sort(_files.begin(), _files.end());
}

//----------------------------------------------------------------------
void PlaybackTable::write(const std::string &outputFname)
{
  string out = _xml();
  FILE *fp = fopen(outputFname.c_str(), "w");
  if (fp != NULL)
  {
    fprintf(fp, out.c_str());
    fclose(fp);
    LOG(DEBUG) << "Wrote output to file " << outputFname;
  }
  else
  {
    LOG(ERROR) << "writing to file " << outputFname;
  }
}    

//----------------------------------------------------------------------
void PlaybackTable::read(int i, std::string &file, bool &isObs,
			   std::string &dir, time_t &gentime,
			   int &leadtime, int &secondsSinceStart) const
{
  secondsSinceStart = _files[i].getTwritten() - _files[0].getTwritten();
  _files[i].read(file, isObs, dir, gentime, leadtime);
}
		 
//----------------------------------------------------------------------
std::string PlaybackTable::_xml(void) const
{
  string ret = "";
  for (size_t i=0; i<_files.size(); ++i)
  {
    string s = _files[i].xml();
    ret += TaXml::writeString(_lineTag, 1, s);
  }
  return ret;
}

//----------------------------------------------------------------------
PlaybackTable::Line::Line(const std::string &fileLine) : _ok(true)
{
  string s;
  if (TaXml::readString(fileLine, _fileTag, _fileName) != 0)
  {
    _ok = false;
    LOG(ERROR) << "Reading tag '" << _fileTag << "' in string " << fileLine;
    _fileName = "unknown";
  }
  if (TaXml::readTime(fileLine, _twrittenTag, _timeWritten) != 0)
  {
    _ok = false;
    LOG(ERROR) << "Reading tag '" << _twrittenTag << "' in string " << fileLine;
    _timeWritten = 0;
  }
  if (TaXml::readBoolean(fileLine, _isObsTag, _isObs) != 0)
  {
    _ok = false;
    LOG(ERROR) << "Reading tag '" << _isObsTag << "' in string " << fileLine;
    _isObs = false;
  }
  if (TaXml::readString(fileLine, _dirTag, _dir) != 0)
  {
    _ok = false;
    LOG(ERROR) << "Reading tag '" << _dirTag << "' in string " << fileLine;
    _dir = "";
  }
    
  if (TaXml::readTime(fileLine, _gentimeTag, _genTime) != 0)
  {
    _ok = false;
    LOG(ERROR) << "Reading tag '" << _gentimeTag << "' in string " << fileLine;
    _genTime = 0;
  }
  if (TaXml::readInt(fileLine, _leadsecondsTag, _leadtime) != 0)
  {
    _ok = false;
    LOG(ERROR) << "Reading tag '" << _leadsecondsTag
	       << "' in string " << fileLine;
    _leadtime = 0;
  }
}

//----------------------------------------------------------------------
PlaybackTable::Line::Line(const std::string &file, const time_t &twritten,
			    const std::string &dir, const time_t &genTime) :
  _ok(true), _fileName(file), _timeWritten(twritten), _isObs(true),
  _dir(dir), _genTime(genTime), _leadtime(0)
{
}

//----------------------------------------------------------------------
PlaybackTable::Line::Line(const std::string &file, const time_t &twritten,
			    const std::string &dir, const time_t &genTime,
			    int leadSeconds) :
  _ok(true), _fileName(file), _timeWritten(twritten), _isObs(false),
  _dir(dir), _genTime(genTime), _leadtime(leadSeconds)
{
}

//----------------------------------------------------------------------
PlaybackTable::Line::~Line(void)
{
}

//----------------------------------------------------------------------
void PlaybackTable::Line::print(void) const
{
  if (_isObs)
  {
    printf("%s %s %s\n", DateTime::strn(_timeWritten).c_str(),
	   DateTime::strn(_genTime).c_str(), _fileName.c_str());
  }
  else
  {
    printf("%s %s+%08d %s\n", DateTime::strn(_timeWritten).c_str(),
	   DateTime::strn(_genTime).c_str(), _leadtime, _fileName.c_str());
  }
}

//----------------------------------------------------------------------
bool PlaybackTable::Line::operator<(const PlaybackTable::Line &f) const
{
  if (_timeWritten < f._timeWritten)
  {
    return true;
  }
  else if (_timeWritten > f._timeWritten)
  {
    return false;
  }
  else
  {
    return _fileName < f._fileName;
  }
}

//----------------------------------------------------------------------
std::string PlaybackTable::Line::xml(void) const
{
  string s = "";
  s += TaXml::writeString(_fileTag, 0, _fileName);
  s += TaXml::writeTime(_twrittenTag, 0, _timeWritten);
  s += TaXml::writeBoolean(_isObsTag, 0, _isObs);
  s += TaXml::writeString(_dirTag, 0, _dir);
  s += TaXml::writeTime(_gentimeTag, 0, _genTime);
  s += TaXml::writeInt(_leadsecondsTag, 0, _leadtime);
  return s;
}

//----------------------------------------------------------------------
void PlaybackTable::Line::read(std::string &file, bool &isObs,
				 std::string &dir, time_t &gentime,
				 int &leadtime) const
{
  file = _fileName;
  isObs = _isObs;
  dir = _dir;
  gentime = _genTime;
  leadtime = _leadtime;
}
