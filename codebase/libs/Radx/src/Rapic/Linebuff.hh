////////////////////////////////////////////////////////////////////
// Linebuff
//
// unpacked radial array class
//
// RAPIC code (c) BOM Australia
//
// Phil Purdam
//
// Jan 2000
//
/////////////////////////////////////////////////////////////////////

#ifndef _LINEBUFF_HH
#define _LINEBUFF_HH

#include <ctype.h>
#include <cstdio>
#include <cstring>
using namespace std;

#define CR 13
#define LF 10
#define HASH 35
#define PERCENT 37
#define EscKey 27
#define CTRLZ 26
#define CTRLC 3
#define EOT '\004'
#define RADLLENGTH 256
#define MAXDBZRES 256 // current max supported dbz threshold resolution

#define X 0
#define Y 1
#define Z 2
#define XY 2
#define XYZ 3

class Linebuff {

public: 
  
  Linebuff(int buffsz = 2048);
  ~Linebuff();

  int new_scan_ofs; // start of next scan
  bool isBinRadl;
  bool isRadl;
  bool isComment;
  bool EOL;
  bool termCh1, termCh2; // used for binRadl EOL detection
  bool Overflow;
  int lb_size; // current size
  char *line_buff;// line of data buff
  int lb_max;
  char *lb_pnt; // pointer to next vacant pos in buffer

  FILE *in;

  void reset();
  void ensureTerminated(); // ensure line is null terminated
  int openFile(const char *file_path);
  void closeFile();
  bool endOfFile();
  int readNext();
  
  inline bool addchar(char c)
  {
    if (IsFull()) return false;
    if (!lb_size) // first char of line
      {
	switch (c) {
	case '@' : isBinRadl = true; isRadl = true;
	  break;
	case '%' : isRadl = true;
	  break;
	case '/' : isComment = true;
	  break;
	}
      }
    addc(c);
    return true;
  }
  
  inline bool addchar_parsed(char c)
  {
    if (IsFull()) return false;
    if (lb_size == 0) // adding first char in line
      {
	if (isspace(c)) // don't add leading white space
	  return true;
	switch (c) {
	case EOT :      //ignore leading EOT char (rjp 19/5/2008)
          printf("Linebuff::addchar_parsed: EOT CHARACTER FOUND\n ");
	  return true;
	case '@' : isBinRadl = true; isRadl = true;
	  break;
	case '%' : isRadl = true;
	  break;
	case '/' : isComment = true;
	  break;
	}
      }
    if (!isBinRadl) // test for EOL for non BinRadl line
      {
	switch (c) {
	case CR : // CR,LF,CTRLZ,'#',0 all treated as line
	case LF : // terminators
	case '#':
	case CTRLZ :
	case CTRLC :
	case 0 :
	  addc(0); // use null as std line term
	  EOL = true;
	  break;
	default: // don't add leading white space
	  addc(c);
	  if (IsFull1()) { // leave space for term null
	    addc(0); // use null as std line term
	    printf("Linebuff::addchar_parsed: LINE BUFFER OVERFLOW");
	    EOL = Overflow = true;
	  }
	  break;
	}
      }
    else // is a binradl, check for termination
      {
	addc(c);
	if ((c == 0) && (lb_size > 19))
	  {
	    if (termCh1) termCh2 = true; // already have first NULL, this is second
	    else termCh1 = true; // this is first null
	  }
	else termCh1 = termCh2 = false;
	if (termCh2 || IsFull())
	  {
	    EOL = true;
	    if (!termCh2 && IsFull())
	      {
		printf("Linebuff::addchar_parsed: BINARY RADIAL LINE BUFFER OVERFLOW");
		Overflow = true; // buffer filled without true EOL
	      }
	  }
      }
    return Overflow;
  };
  
  inline void addc(char c) 
  { 
    *lb_pnt = c;
    lb_pnt++;
    lb_size++;
  };
  
  inline bool IsBinRadl()
  {
    return isBinRadl = ((lb_size > 0) && (line_buff[0] == '@'));
  };
  
  inline bool IsRadl()
  {
    return isRadl = ((lb_size > 0) && 
		     ((line_buff[0] == '@') || (line_buff[0] == '%')));
  }
  
  inline bool IsComment()
  {
    return isComment = ((lb_size > 0) && (line_buff[0] == '/'));
  }
  
  inline bool IsEOL()
  {
    return EOL;
  }
  
  inline bool IsFull()
  {
    return lb_size >= lb_max;
  }
  
  inline bool IsFull1()
  {
    return lb_size >= lb_max-1;
  }
  
  inline bool IsEndOfRadarImage()
  {
    return (strstr(line_buff, "END RADAR IMAGE") != NULL);
  };

  inline bool IsEndOfImage()
  {
    return (strstr(line_buff, "/IMAGEEND:") != NULL);
  };

  inline void setRepeat()
  {
    _repeat = true;
  }

private:

  bool _repeat;

  
};

#endif // _READRAPICFILE_H
