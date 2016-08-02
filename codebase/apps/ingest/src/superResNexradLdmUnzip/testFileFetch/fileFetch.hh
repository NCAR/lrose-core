/////////////////////////////////////////////////////////////
// fileFetch.hh
//
// Class that searches for the next file in LDM delivered
// nexrad super res data.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef FILE_FETCH_H
#define FILE_FETCH_H

#include <toolsa/umisc.h>
#include <vector>

using namespace std;

class fileFetch {
  
public:

  // Constructor. Makes copies of inputs.
  fileFetch ( char *dirName, int maxFileAgeSecs, bool deleteOldFiles,
	      char *subString, bool debug, int timeoutSecs );

  // return next filename, if any. Caller to allocate space.
  void getNextFile(char *filename, date_time_t *filenameTime,
		   int *seqNum, bool *isEOV);

  // Destructor.
  ~fileFetch ();


protected:
  
private:

  int _maxFileAge;
  bool _deleteOldFiles;
  char *_inDir;
  vector <string> _fileNames;
  int _mode;
  time_t _lastTime;
  char *_subString;
  bool _debug;
  int _timeoutSecs;
  time_t _timeOfLastSuccess;

  date_time_t _filenameTime;
  int _fileSeqNum;
  int _fileVersion;
  char _fileVolChar;
  int _seqNum;

  const static int SEARCH_NEW_VOL_MODE = 0;
  const static int SEARCH_NEXT_FILE_MODE = 1;
  

  int _scanDir(char *topDir, int depth);
  void _parseElements(char *filename );

};

#endif





