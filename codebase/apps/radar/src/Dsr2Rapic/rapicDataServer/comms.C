/*
  comms.c++

  Routines for the Comms class heirarchy.

  Base Comms class offers basic functionality:
  Socket, Close, Init, Connect, Reconnect, Disconnect, Read, Write
  Read/Write are from standard non-blocking files.

  Child classes of Comms are rpSocket & Serial.

  Serial class offers specific functionality to set port paramaters etc.
  Children of Serial are LeasedLine, Hayes and X28
  These contain there own specific initialisation and connection
  routines.

  The rpSocket class contains its own connection functionality.	

  Connection version are currently blocking types.
  Non-blocking polled versions with state machines will be 
  written in due course.
  Typically the port will sit idle until a CONNECTREQ made,
  The port will then be initialised, 
  If OK a connection req is made (with timeout)
  If connected

  Reconnecting seems to require a Close then rpSocket on Serial ports as
  a disconnection may leave DSR lo from the modem,X28 etc, which seems
  to lock out any further dialogue with a hardware modem controlled port.
	
*/

//#define DEBUG
#undef	TEST

#ifdef sgi
#define _SGI_MP_SOURCE
#endif
#include <fcntl.h>
#include <stdio.h>
#include <termio.h>
#include <string.h>
#define _BSD_SIGNALS
#include <signal.h>
#include <termios.h>
#include <sys/time.h>
// _SGI_MP_SOURCE allows errno for each thread
#include <errno.h>
#include <stropts.h>
#include <sys/select.h>

#include "comms.h"
#include "utils.h"

bool noRp_SO_KEEPALIVE = false;
bool noRp_SO_LINGER = false;

char *ConnStateStr[] = {
  "IDLE", "CONNECT REQ", "CONNECTED", "CONN LOST", 
  "LISTENING", "CONN BROKEN"};
char *ConnModeString[] = {
  "Generic","Serial ","X28    ","Hayes  ","Socket ","SockX25"};

tcflag_t  dflt_iflag = IGNBRK | INPCK | IXON | IXOFF;
tcflag_t  dflt_oflag = 0;
tcflag_t  dflt_cflag = B19200 | CS8 | CREAD | HUPCL;
tcflag_t  dflt_lflag = 0;
char      dflt_line  = 0;

ConnModes Str2ConnMode(char *str) 
{
  if (strstr(str,"SockX25"))
    return CM_SOCKX25;
  if (strstr(str,"Socket"))
    return CM_SOCKET;
  if (strstr(str,"Serial"))
    return CM_SERIAL;
  if (strstr(str,"X28"))
    return CM_X28;
  if (strstr(str,"Hayes"))
    return CM_HAYES;
  else return CM_GENERIC;
}

/*
 * The gethostbyname and gethostbyaddr seem to need to be serialised
 * They use a static variable for the returned hostent,  so multi 
 * threaded calls must copy the result to a local variable 
 */

spinlock *gethostlock = 0;
bool use_new_sock_close_linger = FileExists("rp_sock_close_linger");
int sock_close_linger_time = 15;

void InitGetAddrByName() {
  if (!gethostlock) gethostlock = new spinlock("InitGetAddrByName->gethostlock", 1000); // 10 secs
}

int GetAddrByName(const char *name, in_addr *rethostaddr) {
  int lockok;
  hostent *temp;
  if (useIPNameResCache)
    return IPNameResCache.getAddrByName(name, rethostaddr); // use ip name caching
  if (gethostlock)
    if (!(lockok = gethostlock->get_lock()))  {
      fprintf(stderr, "GetAddrByName ERROR: Failed to get lock\n");
      return 0;
    }
  temp = gethostbyname(name);
  if (temp)
    memcpy(rethostaddr, temp->h_addr_list[0], sizeof(in_addr));
  if (gethostlock && lockok)
    gethostlock->rel_lock();
  return (temp != 0);
}

void InitGetNameByAddr() {
  if (!gethostlock) gethostlock = new spinlock("InitGetNameByAddr->gethostlock", 1000); // 10 secs
}


int GetNameByAddr(const void *addr, char *retname, int maxchars) {
  int lockok;
  hostent *temp;
  if (gethostlock)
    if (!(lockok = gethostlock->get_lock())) {
      fprintf(stderr, "GetNameByAddr ERROR: Failed to get lock\n");
      return 0;
    }
#ifdef sgi
  temp = gethostbyaddr(addr, sizeof(in_addr), AF_INET);
#else
  temp = gethostbyaddr((char*)addr, sizeof(in_addr), AF_INET);
#endif
  if (temp) strncpy(retname, temp->h_name, maxchars);
  if (gethostlock && lockok)
    gethostlock->rel_lock();
  return (temp != 0);
}

Comm::Comm() {
  ConnMode = CM_GENERIC;
  ConnState = IDLE;
  strcpy(connectedtostr, "Not Connected");
  ConnFail = CONN;
  ConnDir = OUTGOING;
  ConnReqCount = 0;
  ConnCount = 0;
  commfd = -1;
  IsSocket = 0;
  quiet = 0;
  commfile = 0;
  instr = outstr = 0;
  NewInOutStr(1024,1024);
  HoldConnect = FALSE;
  err = NOTOPEN;
  lastconnstr[0] = '\0';
  strcpy(InitString, "");
  terminate = FALSE;
  ConnectTimeOut = 100;
  writebuffer = 0;
  writebuffsz = writebuffoutpos = writebuffcomplete = 0;
};

Comm::~Comm() {
  if (instr) {
    delete[] instr;
    instr = 0;
  }
  if (outstr) {
    delete[] outstr;
    outstr = 0;
  }
  Close();
};

bool Comm::Open(char *pathname) {
  if (pathname);
  return FALSE;
}

bool Comm::Init(char *initstring) {
  Close();
  instrlen = outstrlen = 0;
  lastconnstr[0] = '\0';
  if (initstring)
    SetInitString(initstring);
  return TRUE;
};

bool Comm::Close() {
  if (commfd < 0) return TRUE;
  if (isConnected()) Disconnect();
  if (commfile) {						// if there is a STREAM associated with
    fclose(commfile);				// this file, close via fclose
    commfile = 0;
  }
  else {										// if no STREAM, just close file desc.
/* disable shutdown()
    if (IsSocket) {
      if (shutdown(commfd,2) < 0) 
	perror("Comm::Close socket shutdown() failed");
      else
        fprintf(stderr, "Comm::Close socket shutdown() ok\n");
    }
*/
    if (close(commfd) < 0)
      perror("Comm::Close close() failed");
    else
      fprintf(stderr, "Comm::Close close() ok\n");
    commfd = -1;
  };
  err = NOTOPEN;
  //	ConnReqCount = 0; 		// should clear, but Close is called too often at the moment
  ConnState = IDLE;
  strncpy(connectedtostr, "Not Connected", 128);
  return TRUE;
};

/* 
   Base methods for Connect, Disconnect, etc
   all they do is reset the Connstate
   Not worth child objects calling back to these, 
*/

ConnStates Comm::Connect(char *connstr, int timeout) {
  fprintf(stderr,"Comm::Connect Attempting connect to %s\n",connstr);
  if (connstr)   
    { 
      if (lastconnstr != connstr)        // don't copy if connstr is a pointer to lastconnstr
	strncpy(lastconnstr,connstr,64);
    }
  else 
    lastconnstr[0] = 0;
  ConnState = CONNECTED;
  strncpy(connectedtostr, connstr, 128);
  ConnDir = OUTGOING;
  ConnCount++;
  ConnReqCount++;
  if (timeout);
  return ConnState;
};

ConnStates Comm::ConnectNB(char *connstr) {
  fprintf(stderr,"Comm::ConnectNB Attempting connect to %s\n",connstr);
  if (connstr)   
    { 
      if (lastconnstr != connstr)        // don't copy if connstr is a pointer to lastconnstr
	strncpy(lastconnstr,connstr,64);
    }
  else 
    lastconnstr[0] = 0;
  ConnState = CONNECTED;
  strncpy(connectedtostr, connstr, 128);
  ConnDir = OUTGOING;
  ConnCount++;
  ConnReqCount++;
  return ConnState;
};

ConnStates Comm::Listen(char *_string) {
  fprintf(stderr,"Comm::Listen - Waiting for incoming connection\n");
  if (_string) 
    {
      if (lastconnstr != _string)
	strncpy(lastconnstr,_string,64);
    }
  else 
    lastconnstr[0] = 0;
  ConnState = CONNECTED;
  strncpy(connectedtostr, "Waiting for incoming conn.", 128);
  ConnDir = INCOMING;
  ConnCount++;
  ConnReqCount++;
  return ConnState;
};

ConnStates Comm::CheckListen(int fd) {
  return ConnState;
}

ConnStates Comm::CheckConnect() {
  ConnState = CONNECTED;
  return ConnState;
};

ConnStates Comm::Reconnect(int timeout) {
  if (ConnDir == INCOMING)
    return Listen(lastconnstr);
  else 
    return Connect(lastconnstr, timeout);
};

ConnStates Comm::Disconnect() {
  ConnState = IDLE;
  strncpy(connectedtostr, "Not Connected", 128);
  return ConnState;
};

void Comm::FlushIO() {};

void Comm::NewInOutStr(int InSz,int OutSz) {
  if (instr) delete instr;
  if (outstr) delete outstr;
  instrmax = InSz;
  outstrmax = OutSz;	
  instrlen = outstrlen = 0;
  instr = new char[instrmax];
  outstr = new char[outstrmax];
};

int Comm::Read(char *RdBuff,int maxch,int pos) {
  int numrd;
  if (commfd < 0) return 0;
  if (RdBuff && (maxch < 0)) {
    fprintf(stderr,"Comm::Read ERROR called with RdBuff specified, but no maxch\n");
    return 0;
  }
  if (!RdBuff) {													// if no RdBuff use instr
    RdBuff = instr;
    maxch = instrmax;
  }
  if (IsSocket) {
    if (commfd > 0)
      numrd = recv(commfd,&RdBuff[pos],maxch-pos,0);
  }
  else
    numrd = read(commfd,&RdBuff[pos],maxch-pos);
  if ((numrd < 0) && (errno != EWOULDBLOCK)) { // EWOUDLBLOCK common flow control err
    if ((errno == EPIPE) || (errno == ECONNRESET) || (errno == ENOENT)) {
      fprintf(stderr,"Comm::Read - BROKEN PIPE ENCOUNTERED (errno=%d) - Setting commfd = -1\n", errno);
      ConnState = BROKENPIPE;
      strncat(connectedtostr, " - Connection Broken", 128);
/* disable shutdown()
      if (IsSocket) {
	if (shutdown(commfd,2) < 0) // SHUT_RDWR 
	if (shutdown(commfd,0) < 0) // SHUT_RD
	  perror("Comm::Read socket shutdown() failed");
        else
          fprintf(stderr, "Comm::Read socket shutdown() ok\n");
      }
*/
      if (close(commfd) < 0)
        perror("Comm::Read close() failed");
      else
        fprintf(stderr, "Comm::Read close() ok\n");
      commfd = -1;
    }
    else perror("Comm::Read ERROR - ");
  }
  return numrd;
};

int Comm::Write(char *WrBuff,int numch,int pos) {
  int numwrt;

  if (commfd < 0) return 0;
  if (!WrBuff) {													// if no WrBuff use outstr
    WrBuff = outstr;
    numch = strlen(WrBuff);
  }
  if (numch < 0) numch = strlen(WrBuff);	// only occurs if WrBuff decl
  if (IsSocket)
    numwrt = send(commfd,&WrBuff[pos],numch,0);
  else
    numwrt = write(commfd,&WrBuff[pos],numch);
  if ((numwrt < 0) && (errno != EAGAIN)) {	// EAGAIN common flow control
    if ((errno == EPIPE) || (errno == ECONNRESET) || (errno == ENOENT)) {
      fprintf(stderr,"Comm::Write - BROKEN PIPE ENCOUNTERED - Closing commfd\n");
      ConnState = BROKENPIPE;
      strncat(connectedtostr, " - Connection Broken", 128);
/* disable shutdown()
      if (IsSocket) {
	if (shutdown(commfd,2) < 0)   // SHUT_RDWR 
	  perror("Comm::Write socket shutdown() failed");
        else
          fprintf(stderr, "Comm::Write socket shutdown() ok\n");
      }
*/
      if (close(commfd) < 0)
        perror("Comm::Write close() failed");
      else
        fprintf(stderr, "Comm::Write close() ok\n");
      commfd = -1;
    }
    else perror("Comm::Write ERROR - ");
  }
  if ((numwrt < 0) && (errno == EAGAIN))	// flow control, 
    numwrt = 0;				// no real error, just no chars written
  if (!WrBuff && (numwrt < numch)) {
    fprintf(stderr,"Comm::Write - PARTIAL STRING WRITTEN - %s\n",outstr);
  }
  return numwrt;
};

int Comm::PacedWrite(char* wrtstr, int msperch) {
  int chwrt = 0;

  if (commfd < 0) return 0;
  while (*wrtstr && (commfd >= 0) && !terminate) {
    if ((write(commfd,wrtstr,1) < 0) && (errno != EAGAIN)) {	// EAGAIN common flow control
      if ((errno == EPIPE) || (errno == ECONNRESET) || (errno == ENOENT)) {
	fprintf(stderr,"Comm::PacedWrite - BROKEN PIPE ENCOUNTERED - Setting commfd = -1\n");
	ConnState = BROKENPIPE;
	strncat(connectedtostr, " - Connection Broken", 128);
/* disable shutdown()
	if (IsSocket) {
	  if (shutdown(commfd,2) < 0) 
	    perror("Comm::PacedWrite socket shutdown() failed");
          else
            fprintf(stderr, "Comm::PacedWrite socket shutdown() ok\n");
        }
*/
	if (close(commfd) < 0)
          perror("Comm::PacedWrite close() failed");
        else
          fprintf(stderr, "Comm::PacedWrite close() ok\n");
	commfd = -1;
      }
      else perror("Comm::PacedWrite ERROR - ");
    }
    else {
      sec_delay(msperch/1000);
      wrtstr++;
      chwrt++;
    }
  }
  return chwrt;
}

/*
  WaitResp will wait up to the specified period of time for one of
  the responses passed in the list.
  NOTE: NOT SUITABLE FOR DETECTING A STRING AMONGST MASSES OF DATA
  AS THE INSTR BUFFER WILL FILL AND NOT GO ANY FURTHER
  EndOfLine detection and nulling could eliminate this
*/

bool Comm::WaitResp(strlist* resplist, int tenthstmout) {

  bool match = FALSE;
  int			linepos = 0;
  int	rdch;
  timeval	timeout,timenow;
  //	struct timezone tmzone;

  if (commfd < 0) return FALSE;
#ifndef sgi
  gettimeofday(&timeout, 0);
#else
  gettimeofday(&timeout);
#endif
  timeout.tv_sec += tenthstmout / 10;
  timeout.tv_usec += (tenthstmout % 10) * 100000; // convert tenths to usecs
  if (timeout.tv_usec >= 1000000) {
    timeout.tv_sec += timeout.tv_usec / 1000000;
    timeout.tv_usec = timeout.tv_usec % 1000000;
  }
  instrlen = 0;	
  instr[instrlen] = '\0';
  bool	tmoutflag = FALSE;
  while (!tmoutflag && !match && !terminate) {
    rdch = Read(instr,instrmax-1,instrlen);
    if (rdch > 0) {
      instrlen += rdch;
      instr[instrlen] = '\0';
      if (instrlen > instrmax) {
	fprintf(stderr,"Comm::WaitResp - FATAL ERROR, INSTR OVERFLOW\n");
	return FALSE;
      }
      match = resplist->match(instr) != 0;
      if (match) break;
      while (linepos < instrlen) {		// detect end of line, if so reset line
	switch (instr[linepos]) {
	case CR:
	case LF:
	case CTRLZ:
	case 0:
	case '#':  		// copy remainder of line to start of line
	  strncpy(instr,&instr[linepos+1],instrlen-linepos);
	  instrlen -= (linepos+1);
	  linepos = 0;
	  break;
	}
	linepos++;
      }
      if (!match && (instrlen == instrmax)) {	// line buff full, purge half
	linepos = instrmax / 2;
	strncpy(instr,&instr[linepos+1],instrlen-linepos);
	instrlen -= (linepos+1);
	linepos = 0;
      }
    }
#ifndef sgi
    gettimeofday(&timenow, 0);
#else
    gettimeofday(&timenow);
#endif
    tmoutflag = (timenow.tv_sec > timeout.tv_sec) &&
      (timenow.tv_usec > timeout.tv_usec);
    sec_delay(0.01);
  }
  return match;
}

/*
 * WaitRespSuccFail provides an early out mechanism for fail 
 * strings.
 * WaitResp will listen for succ string until timeout regardless 
 * of fail type strings
 */


bool Comm::WaitRespSuccFail(strlist* succlist, 
			    strlist* faillist, 
			    int tenthstmout) {

  bool succ = FALSE, fail = FALSE;
  int	linepos = 0;
  int	rdch;
  timeval	timeout,timenow;
  //	struct timezone tmzone;

  if (commfd < 0) return FALSE;
#ifndef sgi
  gettimeofday(&timeout, 0);
#else
  gettimeofday(&timeout);
#endif
  timeout.tv_sec += tenthstmout / 10;
  timeout.tv_usec += (tenthstmout % 10) * 100000; // convert tenths to usecs
  if (timeout.tv_usec >= 1000000) {
    timeout.tv_sec += timeout.tv_usec / 1000000;
    timeout.tv_usec = timeout.tv_usec % 1000000;
  }
  instrlen = 0;	
  instr[instrlen] = '\0';
  bool	tmoutflag = FALSE;
  while (!tmoutflag && !succ && !fail && !terminate) {
    rdch = Read(instr,instrmax-1,instrlen);
    if (rdch > 0) {
      instrlen += rdch;
      instr[instrlen] = '\0';
      if (instrlen > instrmax) {
	fprintf(stderr,"Comm::WaitResp - FATAL ERROR, INSTR OVERFLOW\n");
	return FALSE;
      }
      succ = succlist->match(instr) != 0;
      if (succ) break;
      fail = faillist->match(instr) != 0;
      if (fail) break;
      while (linepos < instrlen) {		// detect end of line, if so reset line
	switch (instr[linepos]) {
	case CR:
	case LF:
	case CTRLZ:
	case 0:
	case '#':  		// copy remainder of line to start of line
	  strncpy(instr,&instr[linepos+1],instrlen-linepos);
	  instrlen -= (linepos+1);
	  linepos = 0;
	  break;
	}
	linepos++;
      }
      if (!succ && !fail && (instrlen == instrmax)) {	// line buff full, purge half
	linepos = instrmax / 2;
	strncpy(instr,&instr[linepos+1],instrlen-linepos);
	instrlen -= (linepos+1);
	linepos = 0;
      }
    }
#ifndef sgi
    gettimeofday(&timenow, 0);
#else
    gettimeofday(&timenow);
#endif
    tmoutflag = (timenow.tv_sec > timeout.tv_sec) &&
      (timenow.tv_usec > timeout.tv_usec);
    sec_delay(0.01);
  }
  return succ;
}

int Comm::WriteBuffer(char *buffer, int sz) {
  int byteswritten = 0;
  if (buffer) {
    writebuffer = buffer;
    if (sz >= 0) writebuffsz = sz;
    else writebuffsz = strlen(buffer);
    writebuffoutpos = writebuffcomplete = 0;
  }
  if (!writebuffer) return 1;	// no buffer, write is complete
  byteswritten = Write(writebuffer, 
		       writebuffsz - writebuffoutpos, writebuffoutpos);
  if (byteswritten < 0) 
    return -1;	    // error, return -1
  writebuffoutpos += byteswritten;
  if (writebuffoutpos >= writebuffsz) {
    writebuffcomplete = 1;
    writebuffer = 0;
    return 1;	    // write complete, return 1
  }
  else return 0;	    // not yet complete, return 0
}

ConnStates Comm::getConnState()
{
  return ConnState; 
}

void Comm::SetInitString(char *initstr)
{
  strncpy(InitString, initstr, 128);
}


Serial::Serial() : Comm() {
  ConnMode = CM_SERIAL;
  vtime = 0;
  vmin = 0;
  PortName[0] = '\0';;
  termdta.c_iflag = dflt_iflag;
  termdta.c_oflag = dflt_oflag;
  termdta.c_cflag = dflt_cflag;
  termdta.c_lflag = dflt_lflag;
  // termdta.c_line = dflt_line;
};

Serial::~Serial() {}

void Serial::FlushIO() {
  if (commfile) fflush(commfile);
  if (commfd < 0) return;
  ioctl(commfd,TCFLSH,2);        // flush I/P & O/P		
};

bool  Serial::Open(char *portname) {
  Comm::Open();
  if (!portname && (strlen(PortName) == 0)) {
    fprintf(stderr,"Serial::Open called with no portname\n");
    return FALSE;
  }
  if (portname) {
    if (PortName!=portname) strcpy(PortName,portname);
  }
  commfd = open(PortName,O_RDWR | O_NDELAY);
  if (commfd >= 0) {
    err = OK;
    ioctl(commfd,TCGETA,&oldtermdta);
    ioctl(commfd,TCGETA,&termdta);
    FlushIO();
    SetTermParams();
    SetTermio();
    sprintf(connectedtostr, "Connected to %-32s", portname);
  }
  else {
    err = PORTERR;
    sprintf(connectedtostr, "%s open failed", portname);
  }
  // 	fprintf(stderr,"Serial::Open - fd = %d\n");
  if (err == OK) return TRUE;
  else return FALSE;
};

void Serial::SetTermParams() {
  termdta.c_lflag = 0;
  termdta.c_iflag |= IXON | IXOFF;
};

bool	Serial::Close() {
  if (commfd < 0) return TRUE;
  ioctl(commfd,TCSETA,&oldtermdta);
  if (commfile) {						// if there is a STREAM associated with
    fclose(commfile);				// this file, close via fclose
    commfile = 0;
  }
  else {										// if no STREAM, just close file desc.
    if (close(commfd) < 0) perror("Comm::Close() error - ");
    commfd = -1;
  };
  err = NOTOPEN;
  //	ConnReqCount = 0; 		// should clear, but Close is called too often at the moment
  ConnState = IDLE;
  strncpy(connectedtostr, "Not Connected", 128);
  return TRUE;
};

ConnStates Serial::Connect(char *ConnStr, int timeout) {
  if (Open(PortName)) {
    ConnState = CONNECTED;
    strncpy(connectedtostr, PortName, 128);
    ConnCount++;
  }
  else {
    ConnState = IDLE;
    strncpy(connectedtostr, "Not Connected", 128);
  }
  if (PortName) strncpy(lastconnstr,PortName,64);
  else lastconnstr[0] = 0;
  ConnReqCount++;
  return ConnState;
}

ConnStates Serial::Reconnect(int timeout) {
  if (ConnDir == INCOMING)
    return Listen(lastconnstr);
  else {
    if (strlen(lastconnstr) > 0) {
      fprintf(stderr,"Serial Reconnecting to %s\n",lastconnstr);
      Close();
      sec_delay(1.0);
      Open(PortName);
      return Connect(lastconnstr, timeout);
    }
    else {
      ConnState = IDLE;
      strncpy(connectedtostr, "Not Connected", 128);
      return ConnState;
    }
  }
}
	
ConnStates Serial::Disconnect() {
  Close();
  ConnState = IDLE;
  strncpy(connectedtostr, "Not Connected", 128);
  return ConnState;
};

void Serial::SetTermio() {
  if (commfd < 0) return;
  ioctl(commfd,TCSETA,&termdta);
};

bool Serial::Init(char *initstring) {
  Comm::Init(initstring);			// THIS WILL CLOSE THIS PORT
  return TRUE;
};

void Serial::SetVminVtm(int Vmin, int Vtm) {
  if (commfd < 0) {
    fprintf(stderr,"Error-Serial::setvminvtm called with commfd=%d\n",commfd);
    return;
  }
  termdta.c_cc[VMIN] = vmin = Vmin;
  termdta.c_cc[VTIME] = vtime = Vtm;
  ioctl(commfd,TCSETA,&termdta);
};

void Serial::SetBaud(tcflag_t Baud) {
  termdta.c_cflag = (termdta.c_cflag & ~CBAUD) | Baud;
  ioctl(commfd,TCSETA,&termdta);
};
	
/*
  wait up to a given time for a response which CONTAINS a string
  in the passed list,
  return IMMEDIATELY with TRUE if string found before timeout, 
  FALSE otherwise
  BE AWARE THAT MORE CHARS MAY WELL FOLLOW.
*/

bool Serial::WaitResp(strlist* resplist, int tenthstmout) {

  bool match = FALSE;
  int	oldvmin;
  int	oldvtime;

  if (commfd < 0) return FALSE;
  oldvmin = vmin;
  oldvtime = vtime;
  SetVminVtm(0,1);			// wait 1/10 secs per read
  match = Comm::WaitResp(resplist,tenthstmout);
  SetVminVtm(oldvmin,oldvtime);		//  restore prev vmin/vtime
  return match;
}
		
X28::X28(char *x28params) : Serial() {
  ConnMode = CM_X28;
  if (!x28params) {
    x28portstr[0] = '\0';
    setstr[0] = '\0';
    timeout = FALSE;
    //		ConnectTimeOut = 0;
    x28timer = 0;
    padescstr[0] = '\0';
    callprefix[0] = '\0';
    disconncmd[0] = '\0';
  }
};

X28::~X28() {
#ifdef DEBUG
  fprintf(stderr,"Closing x28.\n");
#endif // DEBUG
  FlushIO();
  Serial::Close();
}

bool X28::Init(char *initstring) {
  return Serial::Init(initstring);
}

bool X28::Open(char *portname) {

#ifdef DEBUG
  fprintf(stderr,"Opening x28.\n");
  //		if (x28params) fprintf(stderr,"x28params = %s\n",x28params);
#endif //DEBUG
  strcpy(x28portstr,portname);
  Serial::Open(x28portstr);
  if (commfd < 0) {
    fprintf(stderr,"x28::x28-unable to open %s\n",x28portstr);
    perror(0);
    ConnState = IDLE;
    sprintf(connectedtostr, "%s open failed", x28portstr);
    err = PORTERR;
    return FALSE;
  }
  err = X28IFACE;
  ReadX28Params();
  SetTermio();
  Write("\r",1);
  sec_delay(0.2);
  PacedWrite("SET,2:0\r",200);
  if (!PadEsc()) {
#ifdef DEBUG
    fprintf(stderr,"x28::init FAILED padesc failed\n");
#endif // DEBUG
    return FALSE;
  }
  Write(setstr,strlen(setstr));
  sec_delay(0.50);
  if (!X28Free()) {								
#ifdef DEBUG
    fprintf(stderr,"x28::init x28free failed...attempting disconnect\n");
#endif // DEBUG
    Disconnect();									// attempt disconnect
    if (!X28Free()) {
#ifdef DEBUG
      fprintf(stderr,"x28::init x28free failed...again\n");
#endif // DEBUG
      return FALSE;	// if not free now, give up
    }
  }
  err = OK;
  timeout = FALSE;
  return TRUE;
}

bool X28::Close() {
  return Serial::Close();
}

/*
  escape to PAD control, try pad esc, then CR, then pad esc again
  exit immediately on success, retrun TRUE even if fail
*/
bool	X28::PadEsc() {
  bool ok;

#ifdef DEBUG
  fprintf(stderr,"x28 Pad ESC attempt.\n");
#endif // DEBUG
  if (commfd < 0) return FALSE;
  FlushIO();
  Write(padescstr,strlen(padescstr)); 		// try pas esc
  ok = WaitResp(&padprompt,20);
  if (!ok) {
    Write("\r",1);		// try a humble CR
    ok = WaitResp(&padprompt,20);
    if (!ok) {
      Write(padescstr,strlen(padescstr));// try padesc last time
      ok = WaitResp(&padprompt,20);
    }
  }
#ifdef DEBUG
  fprintf(stderr,"x28 Pad ESC done - OK = %d.\n",OK);
#endif // DEBUG
  if (ok) err = OK;
  else err = X28IFACE;
  return ok;
}
	
bool X28::X28Free() {
  bool ok;
#ifdef DEBUG
  fprintf(stderr,"ATTEMPTING x28free\n");
#endif // DEBUG
  if (commfd < 0) return FALSE;
  FlushIO();
  strcpy(outstr,"STAT\r");
  Write(outstr,strlen(outstr));
  ok = WaitResp(&freestr,20);
#ifdef DEBUG
  if (ok) fprintf(stderr,"x28free OK, str=%s\n",instr);
  else fprintf(stderr,"x28free FAILED, str=%s\n",instr);
#endif // DEBUG
  return ok;
};	

ConnStates X28::Disconnect() {
  bool ok;
#ifdef DEBUG
  fprintf(stderr,"ATTEMPTING x28 disconnect\n");
#endif // DEBUG
  if (commfd < 0) return IDLE;
  if (!PadEsc()) {
    return IDLE;
  }
  FlushIO();
  strcpy(outstr,disconncmd);
  strcat(outstr,"\r");
  Write(outstr,strlen(outstr));
  ok = WaitResp(&disconnstr,100);
#ifdef DEBUG
  if (ok) fprintf(stderr,"x28disconnect OK, str=%s\n",instr);
  else fprintf(stderr,"x28disconnect FAILED, str=%s\n",instr);
#endif // DEBUG
  if (ok) {
    err = OK;
    ConnState = IDLE;
    strncpy(connectedtostr, "Not Connected", 128);
  }
  return IDLE;
}	

ConnStates X28::Connect(char *ConnStr, int timeout) {
  bool ok;

  fprintf(stderr,"X28::Connect Attempting connect to %s\n",ConnStr);
  if (commfd < 0) return IDLE;
  ConnReqCount++;
  ConnState = CONNECTREQ;
  sprintf(connectedtostr, "Attempting %-32s", ConnStr);
  ConnDir = OUTGOING;
  FlushIO();
  if (ConnStr) 
    {
      if (lastconnstr != ConnStr)
	strncpy(lastconnstr,ConnStr,64);
    }
  else 
    lastconnstr[0] = 0;
  strcpy(outstr,callprefix);
  strcat(outstr,ConnStr);
  strcat(outstr,"\r");
  Write(outstr,strlen(outstr));
  if (!timeout) timeout = ConnectTimeOut;
  else timeout *= 10;	// convert secs to tenths
  ok = WaitResp(&connstr,timeout);
#ifdef DEBUG
  if (ok) fprintf(stderr,"x28connect OK, str=%s\n",instr);
  else fprintf(stderr,"x28connect FAILED, str=%s\n",instr);
#endif // DEBUG
  if (ok) {
    ConnState = CONNECTED;
    ConnFail = CONN;
    ConnCount++;
    sprintf(connectedtostr, "%-32s", ConnStr);
  }
  else {
    Disconnect();
    ConnState = IDLE;
    sprintf(connectedtostr, "%-32s connect Failed", ConnStr);
    ConnFail = BUSY;			// simply call all busy at the moment
  }
  return ConnState;
}

ConnStates X28::Listen(char *_string) {

  fprintf(stderr,"X28::Listen - Waiting for incoming connection\n");
  if (commfd < 0) return IDLE;
  if (_string) 
    {
      if (lastconnstr != _string)
	strncpy(lastconnstr,_string,64);
    }
  else 
    lastconnstr[0] = 0;
  ConnState = CONNECTREQ;
  ConnDir = INCOMING;
  strcpy(connectedtostr, "Listening");
  FlushIO();
  CheckListen();
  return ConnState;
}

ConnStates X28::CheckListen(int fd) {
  bool ok;
  ok = WaitResp(&connstr,1);	// wait for 0.1sec before returning
#ifdef DEBUG
  if (ok) fprintf(stderr,"x28connect OK, str=%s\n",instr);
  else fprintf(stderr,"x28connect FAILED, str=%s\n",instr);
#endif // DEBUG
  if (ok) {
    ConnState = CONNECTED;
    ConnFail = CONN;
    ConnCount++;
    sprintf(connectedtostr, "Listen Connected");
  }
  else {
    Disconnect();
    ConnState = IDLE;
    ConnFail = BUSY;			// simply call all busy at the moment
    strncpy(connectedtostr, "Not Connected", 128);
  }
  return ConnState;
}
	
ConnStates X28::CheckConnect() {
  return ConnState;
}

ConnStates X28::Reconnect(int timeout) {
  if (ConnDir == INCOMING)
    return Listen(lastconnstr);
  else {
    if (strlen(lastconnstr) > 0) {
      fprintf(stderr,"X28 Reconnecting to %s\n",lastconnstr);
      Close();
      sec_delay(1.0);
      Open(PortName);
      return Connect(lastconnstr, timeout);
    }
    else {
      ConnState = IDLE;
      strncpy(connectedtostr, "Not Connected", 128);
      return ConnState;
    }
  }
}
	
ConnStates X28::ConnectNB(char* connstr) {
  fprintf(stderr,"X28::ConnectNB not implemented - using Connect\n");
  return Connect(connstr);
}

void X28::ReadX28Params(char *x28params) {
  FILE *x28paramfile;
	
  termdta.c_cflag = (termdta.c_cflag & ~CBAUD) | B9600;
  //	strcpy(x28portstr,"/dev/ttyf2");
  engstr.addstr("ENGAGED");
  engstr.addstr("[DTE WAITING]");
  engstr.addstr("engaged");
  freestr.addstr("FREE");
  freestr.addstr("[READY]");
  freestr.addstr("free");
  connstr.addstr("COM");
  connstr.addstr("[CALL ACCEPTED]");
  disconnstr.addstr("CLR");
  disconnstr.addstr("RESET");
  disconnstr.addstr("CLEAR");
  disconnstr.addstr("[DTE ORIGIN]");
  disconnstr.addstr("[NUMBER BUSY]");
  disconnstr.addstr("NOT OBTAINABLE");
  padprompt.addstr(">>");
  padprompt.addstr("*");
  strcpy(setstr,"SET 1:1 2:0 3:2 4:10 5:1 6:1 7:2 8:0 9:0 10:0 12:1 13:0");
  strcat(setstr,"\r");
  strcpy(padescstr,"\x10");		// CTL-P
  strcpy(callprefix,"C ");
  strcpy(disconncmd,"CLR");
  ConnectTimeOut = 100;				//	wait up to 10 secs for connect
  if (x28params) {
    x28paramfile = 0;
  }
};

Hayes::Hayes(char *Hayesparams) : Serial() {
  ConnMode = CM_HAYES;
  if (!Hayesparams) {
    Hayesportstr[0] = '\0';
    DefaultInitStr[0] = '\0';
    timeout = FALSE;
    ConnectTimeOut = 0;
    Hayestimer = 0;
    Hayesescstr[0] = '\0';
    callprefix[0] = '\0';
    disconncmd[0] = '\0';
  }
};

Hayes::~Hayes() {
#ifdef DEBUG
  fprintf(stderr,"Closing Hayes.\n");
#endif // DEBUG
  FlushIO();
  Serial::Close();
}

bool Hayes::Init(char *initstring) {
  ReadHayesParams();
  return Serial::Init(initstring);
}

bool Hayes::Open(char *portname) {

#ifdef DEBUG
  fprintf(stderr,"Opening Hayes.\n");
  //		if (x28params) fprintf(stderr,"Hayesparams = %s\n",Hayesparams);
#endif //DEBUG
  if (portname)
    strcpy(Hayesportstr,portname);
  Serial::Open(Hayesportstr);
  if (commfd < 0) {
    fprintf(stderr,"Hayes::Open-unable to open %s\n",Hayesportstr);
    perror(0);
    ConnState = IDLE;
    strncpy(connectedtostr, "Not Connected", 128);
    err = PORTERR;
    return FALSE;
  }
  err = HAYESIFACE;
  SetTermio();
  Write("\r",1);
  sec_delay(0.2);
  if (!HayesEsc()) {
    //	#ifdef DEBUG
    fprintf(stderr,"Hayes init FAILED: Hayes esc failure\n");
    //	#endif // DEBUG
    return FALSE;
  }
  if (InitString)
    Write(InitString);
  else
    Write(DefaultInitStr);
  Write("\r",1);
  sec_delay(0.50);
  err = OK;
  timeout = FALSE;
  return TRUE;
}

bool Hayes::Close() {
  //	return Disconnect();									// attempt disconnect
  return Serial::Close();
}
/*
  escape to Hayes control, try 
  exit immediately on success, retrun TRUE even if fail
*/
bool	Hayes::HayesEsc() {
  bool ok = 0;

#ifdef DEBUG
  fprintf(stderr,"Hayes ESC attempt...");
#endif // DEBUG
  if (commfd < 0) return FALSE;
  FlushIO();
  if (!isConnected()) {    // not connected test for response
    Write(HayesAT,strlen(HayesAT)); // try esc
    ok = WaitResp(&Hayesprompt,20);
  }
  else {				    // is connected, escape to command mode
    sec_delay(3.0);
    Write(Hayesescstr,strlen(Hayesescstr)); // try esc
    ok = WaitResp(&Hayesprompt,50);
  }
  if (ok) {
    err = OK;
    //	    printf("OK\n");
  }
  else {
    err = HAYESIFACE;
    printf("FAILED\n");
  }
  return ok;
}
	
ConnStates Hayes::Disconnect() {
  bool ok;
#ifdef DEBUG
  fprintf(stderr,"ATTEMPTING Hayes disconnect\n");
#endif // DEBUG
  if (commfd < 0) return IDLE;
  if (!HayesEsc()) {
    Serial::Close();
    return IDLE;
  }
  FlushIO();
  strcpy(outstr,disconncmd);
  strcat(outstr,"\r");
  Write(outstr,strlen(outstr));
  ok = TRUE;
  /* Hanging up on modem control line enabled tty stops any further comms
     ie will not get a response. Just assume it worked
     sec_delay(1.0);    // delay a bit before proceeding
     Write(HayesAT,strlen(HayesAT)); // try esc
     ok = WaitResp(&disconnstr,100);
     #ifdef DEBUG
     if (ok) fprintf(stderr,"Hayes disconnect OK, str=%s\n",instr);
     else fprintf(stderr,"Hayes disconnect FAILED, str=%s\n",instr);
     #endif // DEBUG
  */
  if (ok) {
    err = OK;
    ConnState = IDLE;
    strncpy(connectedtostr, "Not Connected", 128);
  }
  Serial::Close();
  return ConnState;
}	

ConnStates Hayes::Connect(char *ConnStr, int timeout) {
  bool ok;

  if (isConnected()) 
    Disconnect();
  Open();
#ifdef DEBUG
  fprintf(stderr,"Hayes::Connect Attempting connect to %s\n",ConnStr);
#endif // DEBUG
  if (commfd < 0) return IDLE;
  ConnReqCount++;
  ConnState = CONNECTREQ;
  sprintf(connectedtostr, "Attempting %-32s", ConnStr);
  ConnDir = OUTGOING;
  FlushIO();
  if (ConnStr) 
    {
      if (lastconnstr != ConnStr)
	strncpy(lastconnstr,ConnStr,64);
    }
  else 
    lastconnstr[0] = 0;
  strcpy(outstr,callprefix);
  strcat(outstr,ConnStr);
  strcat(outstr,"\r");
  Write(outstr,strlen(outstr));
  if (!timeout) timeout = ConnectTimeOut;
  else timeout *= 10;	// convert secs to tenths
  ok = WaitRespSuccFail(&connsuccstr, &connfailstr,timeout);
#ifdef DEBUG
  if (ok) fprintf(stderr,"Hayes connect OK, str=%s\n",instr);
  else 
    if (connfailstr.this_str())
      fprintf(stderr,"Hayes connect FAILED, str=%s, failstr=%s\n",
	      instr, connfailstr.this_str());
    else
      fprintf(stderr,"Hayes connect FAILED, str=%s\n",instr);
#endif // DEBUG
  if (ok) {
    fprintf(stderr,"Hayes connected to %s...OK\n",lastconnstr);
    ConnState = CONNECTED;
    ConnFail = CONN;
    ConnCount++;
#ifdef DEBUG
    fprintf(stderr,"Hayes connect OK, str=%s\n",instr);
#endif // DEBUG
    sprintf(connectedtostr, "%-32s", ConnStr);
    sec_delay(1.0);    // delay a bit before proceeding
    // to prevent sending strings before the connect string
    // is complete which may terminate the call
  }
  else {
    Disconnect();
    ConnState = IDLE;
    ConnFail = BUSY;			// simply call all busy at the moment
    sprintf(connectedtostr, "%-32s connect Failed", ConnStr);
    fprintf(stderr,"Hayes connect attempt to %s...FAILED\n",lastconnstr);
  }
  return ConnState;
}

ConnStates Hayes::CheckConnect() {
  return ConnState;
}

ConnStates Hayes::Reconnect(int timeout) {
  if (ConnDir == INCOMING)
    return Listen(lastconnstr);
  else {
    if (strlen(lastconnstr) > 0) {
#ifdef DEBUG
      fprintf(stderr,"Hayes Reconnecting to %s\n",lastconnstr);
#endif // DEBUG
      return Connect(lastconnstr, timeout);
    }
    else {
      ConnState = IDLE;
      strncpy(connectedtostr, "Not Connected", 128);
      return ConnState;
    }
  }
}
	
ConnStates Hayes::ConnectNB(char* ConnStr) {
  fprintf(stderr,"X28::ConnectNB not implemented - using Connect\n");
  return Connect(ConnStr);
}

void Hayes::ReadHayesParams(char *hayesparams) {
	
  termdta.c_cflag = (termdta.c_cflag & ~CBAUD) | B9600;
  engstr.addstr("OK");
  connsuccstr.addstr("CONNECT");
  connfailstr.addstr("BUSY");
  connfailstr.addstr("ERROR");
  connfailstr.addstr("NO DIAL TONE");
  connfailstr.addstr("NO ANSWER");
  disconnstr.addstr("OK");
  disconnstr.addstr("NO CARRIER");
  Hayesprompt.addstr("OK");
  strcpy(DefaultInitStr,"AT&F0&H3&R2&I2S0=0V1E0&B1S15=64");
  strcpy(Hayesescstr,"+++");		
  strcpy(callprefix,"");
  strcpy(disconncmd,"ATH0");
  strcpy(HayesAT, "AT\r");
  ConnectTimeOut = 600;				//	wait up to 10 secs for connect
};

/*

socket.c

Implementation of rpSocket class

*/

#include <sys/socket.h>

rpSocket::rpSocket() : Comm() {
  ConnMode = CM_SOCKET;
  memset((char *) &sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  connectPort = 0;
  listenPort = 0;
  IsSocket = TRUE;
  listenfd = -1;
  
  shutdownmode = -1;	// by default use shutdown mode 0 on close

};

rpSocket::~rpSocket() {
  Close();
};

void rpSocket::FlushIO() {
  /*
    if (ioctl(commfd,I_FLUSH,FLUSHRW) < 0)
    perror("Socket::FlushIO() - ");
  */
};

bool rpSocket::Open(char *pathname) {	// pathname not relevant
  Comm::Open();
  if (pathname);
  commfd = socket(PF_INET,SOCK_STREAM,0);
  if (commfd < 0) {
    err = NOTOPEN;
    perror("rpSocket::Open NO SOCKET");
    return FALSE;
  }
  //	fprintf(stderr,"rpSocket::Open - commfd = %d\n",commfd);

  setSockOpts();
  err = OK;
  return TRUE;
};

bool rpSocket::setSockOpts(int fd)
{
  linger Linger;
  int	val, size;
  /*
   * Typically O_NONBLOCK type writes will return -1 and errno = EAGAIN
   * if unable to write data
   * O_NDELAY will return 0 if unable to write data
   */

  if (fd < 0)	    // not specified, use commfd
    fd = commfd;
#ifdef sgi
  if (fcntl(fd,F_SETFL,O_NDELAY) < 0)
    {
      perror("rpSocket::setSockOpts fcntl failed: ");
      return false;
    }
#else
  if (fcntl(fd,F_SETFL,O_NONBLOCK) < 0)
    {
      perror("rpSocket::setSockOpts fcntl failed: ");
      return false;
    }    
#endif
  /*
    size = sizeof(val);
    getsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&val,&size);
    fprintf(stderr,"rpSocket::setSockOpts - SO_KEEPALIVE=%d\n",val);
    getsockopt(fd,SOL_SOCKET,SO_SNDBUF,&val,&size);
    fprintf(stderr,"rpSocket::setSockOpts - SO_SNDBUF=%d\n",val);
    getsockopt(fd,SOL_SOCKET,SO_RCVBUF,&val,&size);
    fprintf(stderr,"rpSocket::setSockOpts - SO_RCVBUF=%d\n",val);
    Linger.l_onoff = 0;
    Linger.l_linger = 0;
    size = sizeof(Linger);
    getsockopt(fd,SOL_SOCKET,SO_LINGER,&Linger,&size);
    fprintf(stderr,"rpSocket::setSockOpts - SO_LINGER=%d delay=%d\n",Linger.l_onoff,Linger.l_linger);
  */
  val = 1;
  size = sizeof(val);
  if (!noRp_SO_KEEPALIVE)
    {
#ifndef sun
      setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&val,size);	// should detect fail from other end
#else
      setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&val,size);	// should detect fail from other end
#endif
    }
  //if (shutdownmode < 0) {		// if using close only set linger
  Linger.l_onoff = 1;
  Linger.l_linger = 60;
  size = sizeof(Linger);
  if (!noRp_SO_LINGER)
    {
#ifndef sun
      setsockopt(fd,SOL_SOCKET,SO_LINGER,&Linger,size);
#else
      setsockopt(fd,SOL_SOCKET,SO_LINGER,(char *)&Linger,size);
#endif
    }
  //	fprintf(stderr,"rpSocket::setSockOpts - SO_LINGER=%d delay=%d\n",Linger.l_onoff,Linger.l_linger);
  //	}
  /*
    size = sizeof(val);
    getsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&val,&size);
    //	fprintf(stderr,"rpSocket::setSockOpts - SO_KEEPALIVE=%d\n",val);
    val = 1;
    setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&val,size);	// should detect fail from other end
    Linger.l_onoff = 0;
    Linger.l_linger = 0;
    size = sizeof(Linger);
    getsockopt(fd,SOL_SOCKET,SO_LINGER,&Linger,&size);
    fprintf(stderr,"rpSocket::setSockOpts - SO_LINGER=%d delay=%d\n",Linger.l_onoff,Linger.l_linger);
    Linger.l_onoff = 0;
    Linger.l_linger = 0;
    size = sizeof(linger);
    setsockopt(fd,SOL_SOCKET,SO_LINGER,&Linger,size);	// should detect fail from other end
    getsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&val,&size);
    //	fprintf(stderr,"rpSocket::setSockOpts - SO_REUSEADDR=%d\n",val);
    */
  return true;
}

bool rpSocket::Close() {
  int linger_delay = sock_close_linger_time;
  if (listenfd >= 0) {
    listenfd = -1;
  }
  if (use_new_sock_close_linger)
    {
      if (ConnState != LISTENING)
	{
	  if (shutdown(commfd,1) < 0)  // shut down outgoing
	    perror("Socket::Close linger socket shutdown() failed");
          else
            fprintf(stderr, "Socket::Close linger socket shutdown() ok\n");
	  while (linger_delay--)
	    {
	      Read(instr, instrmax, 0);
	      sec_delay(1.0);
	    }
	  if (close(commfd) < 0) 
	    perror("Socket::Close close() failed");
          else
            fprintf(stderr, "Socket::Close close() ok\n");
	}
    }
  else
    {
      if (commfd >= 0) {
	if ((shutdownmode >= 0) && 
	    (ConnState != LISTENING)) {
	  if (shutdown(commfd,shutdownmode) < 0)
	    perror("rpSocket::Close socket shutdown() failed");
          else
            fprintf(stderr, "rpSocket::Close socket shutdown() ok\n");
        }
	if (close(commfd) < 0) 
	  perror("rpSocket::Close() close() failed");
        else
          fprintf(stderr, "rpSocket::Close() close() ok\n");
      }
    }
  commfd = -1;
  ConnState = IDLE;
  strcpy(connectedtostr, "Closed");
  return TRUE;
};

// hostname string may specify "host portno"
ConnStates rpSocket::Connect(char *hostname, int timeout) {
  char	errstr[128],hname[128];
  int portno = 0;
  //    hostent	*hp;
  int		hostnameok;
  timeval	tmout;
  int		tenthstmout;
  //    int	secsleft = 0;
  fd_set	writer;
  int 	ioevents;
  int		val;
  int connrefused=0;

#if (defined sgi) || (defined sun)
  int size;
#else
  socklen_t size;
#endif
  int temp = 0;
  char	tempbuff[1];

  if (isConnected()) {
    if (!quiet)
      fprintf(stderr, "rpSocket::Connect ERROR - Already connected\n");
    return ConnState;
  }
  if (ConnState == LISTENING) {
    if (!quiet)
      fprintf(stderr, "rpSocket::Connect WARNING - Currently LISTENING, closing LISTEN\n");
    Close();
  }
  connectPort = 23;
  if (!quiet)
    fprintf(stderr,"rpSocket::Connect Attempting connect to %s....\n",hostname);
  if (hostname)
    {
      if (lastconnstr != hostname)
	strncpy(lastconnstr,hostname,64);
    }
  else 
    lastconnstr[0] = 0;
  ConnDir = OUTGOING;
  if (commfd < 0) Open("");
  if (!noRp_SO_KEEPALIVE) {
    size = sizeof(val);
#ifndef sun
    getsockopt(commfd,SOL_SOCKET,SO_KEEPALIVE,&val,&size);
    val = 0;
    setsockopt(commfd,SOL_SOCKET,SO_KEEPALIVE,&val,size);	// should detect fail from other end
#else
    getsockopt(commfd,SOL_SOCKET,SO_KEEPALIVE,(char *)&val,&size);
    val = 0;
    setsockopt(commfd,SOL_SOCKET,SO_KEEPALIVE,(char *)&val,size);	// should detect fail from other end
#endif
    //  fprintf(stderr,"rpSocket::Open - SO_KEEPALIVE=%d\n",val);
  }
  if (commfd < 0) {
    if (!quiet)
      perror("\nrpSocket::Connect - FAILED commfd < 0");
    err = NOTOPEN;
    ConnState = IDLE;
    strcpy(connectedtostr, "Couldn't open");
    return ConnState;
  }
  ConnReqCount++;
  sscanf(hostname,"%s %d",hname,&portno);
  //    hp = GetHostByName(hname);
  hostnameok = GetAddrByName(hname, &sin.sin_addr);
  if (!hostnameok) {
    strcpy(errstr,"\nrpSocket::Connect Error HOST=");
    strcat(errstr,hostname);
    strcat(errstr," ");
#ifdef sgi
    if (!quiet)
      herror(errstr);
#else
    if (!quiet)
      fprintf(stderr, "%s ERROR NUMBER=%d\n", errstr, h_errno);
#endif    
    Close();
    err = SOCKHOST;
    ConnState = IDLE;
    sprintf(connectedtostr, "%s Failed - Bad hostname", hostname);
    return ConnState;
  }
  if (portno) 
    connectPort = portno;	
  sin.sin_port = htons(connectPort);
  //    memcpy(&(sin.sin_addr.s_addr), hp->h_addr, hp->h_length);
  memset(&writer, 0, sizeof(writer));
  FD_SET(commfd,&writer);
  if (timeout) tenthstmout = timeout * 10;
  else tenthstmout = ConnectTimeOut;    
  tmout.tv_sec = tenthstmout / 10;
  tmout.tv_usec = (tenthstmout % 10) * 100000;
  //	wait for commfd to become avail for write, ie connect complete

  /* connect() -- check for connection refused
     The main connect() error under Linux and AIX is EINPROGRESS 
     due to non-blocking connections.

     However, in multithreaded 3D-Rapic and under AIX v5.2
     connect() can sometimes fail to connect with ECONNREFUSED.
     We trap this here specifically and set ioevents (a timed out
     flag, see below), the connect will then fail elegantly.

     If not, the connection will fail but the rest of the code will
     attempt to "read" from this connection and causes 3D-Rapic to
     exit inelegantly under AIX v5.2.
  */
  if (connect(commfd,(sockaddr *)&sin,sizeof(sin))==-1)
    {
      if (errno == ECONNREFUSED) 
	  connrefused = 1;
      if (errno != EINPROGRESS)
	  perror("rpSocket::Connect - connect()");
    }
 
  ioevents = select(FD_SETSIZE,0,&writer,0,&tmout);
  
  if (!connrefused && ioevents > 0) 
    {
      temp = recv(commfd, tempbuff, 0, 0);
      if ((temp < 0) && (errno != EAGAIN))
	ioevents = -1;
    }

  if (connrefused || ioevents <= 0) {    // timed out
    //close(commfd);
    if (close(commfd) < 0) perror("rpSocket::Connect close() failed");
    commfd = -1;
    strcpy(errstr,"\nrpSocket::Connect to ");
    strcat(errstr,hostname);
    if (ioevents == 0) strcat(errstr," TIMED OUT ");
    else strcat(errstr," CONNECT FAILED: ");
    if (!quiet) perror(errstr);
    Close();
    err = SOCKCONN;
    ConnState = IDLE;
    sprintf(connectedtostr, "%s Failed - Timed out", hostname);
    return ConnState;
  }
  ConnCount++;
  if (FD_ISSET(commfd,&writer)) ConnState = CONNECTED;
  strncpy(connectedtostr, hostname, 128);
  if (!quiet)
    fprintf(stderr,"rpSocket::Connect Attempt to %s...OK\n", hostname);
  return ConnState;
};

// hostname string may specify "host portno"
ConnStates rpSocket::ConnectNB(char *hostname) {
  fprintf(stderr,"rpSocket::ConnectNB not implemented - using Connect\n");
  return Connect(hostname);
};

/* prime socket for listening, use subsequent CheckListen
 * to actually accept incoming connection
 */
ConnStates rpSocket::Listen(char *_string) {
  // int	tempfd, len;

  int val;
	
#if (defined sgi) || (defined sun)
  int size;
#else
  socklen_t size;
#endif
  if (isConnected()) {
    fprintf(stderr, "rpSocket::Listen - Already connected...disconnecting\n");
    Disconnect();
  }
  size = sizeof(val);
  /* commfd won't be set yet!!!! need to do this after incoming connection
     #ifndef sun
     getsockopt(commfd,SOL_SOCKET,SO_KEEPALIVE,&val,&size);
     val = 1;
     setsockopt(commfd,SOL_SOCKET,SO_KEEPALIVE,&val,size);	// should detect fail from other end
     #else
     getsockopt(commfd,SOL_SOCKET,SO_KEEPALIVE,(char *)&val,&size);
     val = 1;
     setsockopt(commfd,SOL_SOCKET,SO_KEEPALIVE,(char *)&val,size);	// should detect fail from other end
     #endif
  */
  //	fprintf(stderr,"rpSocket::Open - SO_KEEPALIVE=%d\n",val);
  if (_string) 
    {
      if (listenstr != _string)
	strncpy(listenstr,_string,64);
    }
  if (sscanf(listenstr, "%d %hu", &listenfd, &listenPort) < 2)
    {
      fprintf(stderr,"rpSocket::Listen ERROR calling Listen - string must pass \"listenfd portno\" - %s\n", listenstr);	    
      listenfd = -1;
    }
  ConnDir = INCOMING;
  if ((listenPort == 0) || (listenfd < 0))
    {
//       fprintf(stderr,"rpSocket::Listen listenfd/listenport not properly set (%d/%d)\n", 
// 	      listenfd, listenPort);
      ConnState = IDLE;
      strncpy(connectedtostr, "Error, listenPort undefined", 128);
      return ConnState;
    }
  sin.sin_port = htons(listenPort);	
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  if (commfd >= 0) {
/* disable shutdown()
    if (shutdown(commfd,2) < 0) 
      perror("rpSocket::Listen socket shutdown() failed");
    else
      fprintf(stderr, "rpSocket::Listen socket shutdown() ok\n");
*/
    if (close(commfd) < 0)
      perror("rpSocket::Listen close() failed");
    else
      fprintf(stderr, "rpSocket::Listen close() ok\n");
    commfd = -1;
  }
  if (listenfd < 0) {
    err = NOTOPEN;
    fprintf(stderr,"rpSocket::Listen called when listenfd < 0\n");
    ConnState = IDLE;
    strncpy(connectedtostr, "Error, No listenfd", 128);
    return ConnState;
  }
  else {				
    ConnState = LISTENING;
    //	    fprintf(stderr,"rpSocket::Listen - Already listening. Waiting for incoming connection on port %d, RPSrv_fd=%d\n",sin.sin_port,listen_fd);
    return ConnState;
  }
}

ConnStates rpSocket::CheckListen(int fd) {
  int tempfd;
#if (defined sgi) || (defined sun)
  int len;
#else
  socklen_t len;
#endif
  if (ConnState != LISTENING) {
    fprintf(stderr,"rpSocket::CheckListen called when not Listening\n");
    return ConnState;
  }
  if (listenfd < 0) {
    fprintf(stderr,"rpSocket::CheckListen called when listenfd < 0\n");
    return ConnState;
  }

  len = sizeof (sin);
  if (fd >= 0)
    tempfd = fd;    // use fd passed with CheckListen call
  else
    tempfd = accept(listenfd, (sockaddr *)&sin, &len);
  if (tempfd >= 0) {
    //		fprintf(stderr,"rpSocket::CheckListen - Connection from host %s,"
    //		    "port %u fd=%d\n", inet_ntoa(sin.sin_addr), 
    //		    ntohs(sin.sin_port),tempfd);
#ifndef UNDEFINED
    //		if (temphostent = gethostbyaddr(&sin.sin_addr, sizeof(in_addr), AF_INET))
    if (!GetNameByAddr(&sin.sin_addr, lastconnstr, 128))
#else
      ***		if (temphostent = gethostbyaddr((char *)&sin.sin_addr, sizeof(in_addr), AF_INET))
#endif
	strncpy(lastconnstr, inet_ntoa(sin.sin_addr), 64);
    fprintf(stderr,"rpSocket::CheckListen - Connection from host %s\n", lastconnstr);
    //		    "port %u fd=%d\n", lastconnstr, 
    //		    ntohs(sin.sin_port),tempfd);
    ConnState = CONNECTED;
    ConnCount++;
    strncpy(connectedtostr, lastconnstr, 128);
    commfd = tempfd;		// use file handle from accept
    setSockOpts();			// ensure sockOpts are properly set, 
					// accept doesn't always dup listenfd's properties
					// On Linux O_NONBLOCK may not be set
  }
  else {
    //		perror("rpSocket::CheckListen - Failed "); 
    if (errno != EWOULDBLOCK)
      {
	perror("rpSocket::CheckListen - accept failed");
	fprintf(stderr, "rpSocket::CheckListen - listenfd=%d ErrNo=%d\n", listenfd, errno);
      }
    ConnState = LISTENING;
    strncpy(connectedtostr, "Listening", 128);
  }
  return ConnState;
}

ConnStates rpSocket::CheckConnect() {
  return ConnState;
};

ConnStates rpSocket::Disconnect() {
  Close();
  return ConnState;
};

ConnStates rpSocket::Reconnect(int timeout) {
  if (ConnDir == INCOMING) 
    return Listen(listenstr);
  else {
    if (strlen(lastconnstr) > 0) {
      fprintf(stderr,"rpSocket Reconnecting to %s\n",lastconnstr);
      Close();
      Open();
      return Connect(lastconnstr, timeout);
    }
    else {
      Disconnect();
      ConnState = IDLE;
      return IDLE;
    }
  }
};

SockX25::SockX25() : rpSocket() {
  ConnMode = CM_SOCKX25;
  //	hostprompt.addstr("HEAD_OFFICE_PT1>");
  hostprompt.addstr(">");
  connstr.addstr("Open");
  confirmstr.addstr("[confirm]");
  strcpy(hostesc,"\036x");
  strcpy(disconnstr,"disconnect");
  strcpy(closestr,"exit");
  callprefix[0] = 0;
  ConnectTimeOut = 100;
  ProtXlatHost[0] = 0;
}

SockX25::~SockX25() {
}

bool SockX25::Open(char *pathname) {
  int tempint;
  if (pathname) {
    if (sscanf(pathname,"%127s %d",ProtXlatHost,&tempint) == 2)
      connectPort = tempint;
    else
      connectPort = 23;
  }
  else {
    strcpy(ProtXlatHost,"pad-port");
    connectPort = 23;
  }
  return rpSocket::Open(pathname);
}

ConnStates SockX25::Connect(char *X25Addr, int timeout) {
  if (commfd < 0) rpSocket::Open();
  if (X25Addr) 
    {
      if (lastconnstr != X25Addr)
	strncpy(lastconnstr,X25Addr,64);
    }
  else 
    lastconnstr[0] = 0;
  fprintf(stderr,"SockX25::Connect Attempting connect to %s\n",X25Addr);
  if (!timeout) timeout = ConnectTimeOut;
  else timeout *= 10;	// convert secs to tenths
  if (rpSocket::Connect(ProtXlatHost, timeout) != CONNECTED) {
    fprintf(stderr,"SockX25::Connect Failed - Couldn't connect to %s\n",ProtXlatHost);
    Close();
    err = HOSTERR;
    ConnState = IDLE;
    sprintf(connectedtostr, "ProtXLat(%s) connect Failed", ProtXlatHost);
    return ConnState;
  }
  ConnCount--;	// if we got here ConnCount has been incremented
  // by rpSocket::Connect.
  // There is still work to do, so cancel it
  if (!WaitResp(&hostprompt,timeout)) {
    fprintf(stderr,"SockX25::Connect Failed - HOST PROMPT NOT RECEIVED\n");
    Close();
    err = HOSTERR;
    ConnState = IDLE;
    hostprompt.reset();
    if (hostprompt.this_str())
      sprintf(connectedtostr, "SockX25::Connect - ProtXLat(%s) Prompt(%s) not received", 
	      ProtXlatHost, hostprompt.this_str());
    else
      sprintf(connectedtostr, "SockX25::Connect - ProtXLat(%s) Prompt(UNDEFINED!!) not received", 
	      ProtXlatHost);
    return ConnState;
  }
  strcpy(outstr,callprefix);
  strcat(outstr,X25Addr);
  sprintf(connectedtostr, "%s", X25Addr);
  strcat(outstr,"\r");
  Write();
  if (!WaitResp(&connstr,timeout)) {
    fprintf(stderr,"SockX25::Connect Failed - X25 CONNECT TO %s FAILED\n",X25Addr);
    Close();
    ConnState = IDLE;
    sprintf(connectedtostr, "%s: Failed to connect", X25Addr);
    return ConnState;
  }
  ConnState = CONNECTED;
  ConnCount++;					// OK we can increment ConnCount now
  fprintf(stderr,"SockX25::Connect Connected to %s OK\n",X25Addr);
  strncpy(connectedtostr, X25Addr, 128);
  return ConnState;
}

ConnStates SockX25::ConnectNB(char *X25Addr) {
  if (X25Addr);
  fprintf(stderr,"SockX25::ConnectNB not implemented - using Connect\n");
  return Connect(X25Addr);
}

/*
  ConnStates SockX25::Listen(char *string) {
  if (string);
  fprintf(stderr,"SockX25::Listen called - NOT SUPPORTED\n");
  return ConnState;
  }
*/

ConnStates SockX25::CheckConnect() {
  return ConnState;
}

ConnStates SockX25::Disconnect() {
  bool ok;

  ok = FALSE;
  /*  SKIP THE HOST ESC BIT (DOESN'T SEEM OT WORK), JUST CLOSE THE SOCKET
      strcpy(outstr,hostesc);
      Write();
      FlushIO();
      if (!WaitResp(&hostprompt,50))
      fprintf(stderr,"SockX25::Disconnect - ERROR IN HOST ESC\n");
      else {
      strcpy(outstr,disconnstr);
      strcat(outstr,"\r");
      Write();
      if (WaitResp(&confirmstr,30)) {
      strcpy(outstr,"\r");
      Write();
      if (WaitResp(&hostprompt,30)) {
      ok = TRUE;
      strcpy(outstr,closestr);
      strcat(outstr,"\r");
      Write();
      }
      }
      }
  */
  ConnState = IDLE;
  strncpy(connectedtostr, "Not Connected", 128);
  Close();	// where all else may have failed
  // this should succeed anyway.
  return ConnState;
}

ConnStates SockX25::Reconnect(int timeout) {
  if (strlen(lastconnstr) > 0) {
    if (commfd > -1) Disconnect();
    Open(ProtXlatHost);
    return Connect(lastconnstr, timeout);
  }
  else return ConnState;
}



#ifdef TEST
main() {
  char teststr[1024];
  bool done = FALSE;
  ConnStates connstate;
  int chars;
  rpSocket *sock;

  sock = new rpSocket();
  printf("Starting listen\n");
  connstate = sock->Listen("12345");
  while (connstate != CONNECTED) {
    connstate = sock->Accept();
    sleep(1);
  }
  printf("Incoming connection established\n");
  while (!done) {
    chars = sock->Read(teststr,1024,0);
    if (chars < 0) chars = 0;
    teststr[chars] = 0;
    if (teststr[0]) printf("%s",teststr);
    char *temp = teststr;
    while (*temp++) printf("%02d ",*temp);
    fflush(stdout);
    done = strstr(teststr,"exit") != NULL;
    sleep (5);
  }
  delete sock;
};

#endif // TEST

