#ifndef __COMMS_H
#define __COMMS_H

/*
				comms.h

        Class header for the Comms class heirarchy.

        Base Comms class offers basic functionality:
        Open, Close, Init, Connect, Reconnect, Disconnect, Read, Write
        Read/Write are from standard non-blocking files.

        Child classes of Comms are Socket & Serial.

        Serial class offers specific functionality to set port paramaters etc.
        Children of Serial are LeasedLine, Hayes and X28        
        These contain there own specific initialisation and connection  
        routines.       

        The Socket class contains its own connection functionality.     

	The x28params file contains parameters for the x28 port operation.
	The following specifies the expected format:
	If no file is passed, the x28 will look for a default file called
	"x28params"

	X28PORT:/dev/ttyf2
	BAUD:9600
	PADPROMPT:>>
	CONNSTR:COM
	CONNSTR:[CALL ACCEPTED]
	DISCONNSTR:CLR 
	DISCONNSTR:RESET 
	DISCONNSTR:CLEAR 
	DISCONNSTR:[DTE ORIGIN] 
	DISCONNSTR:[NUMBER BUSY]
	DISCONNSTR:NOT OBTAINABLE
	FREESTR:FREE
	FREESTR:[READY]
	FREESTR:free
	ENGSTR:ENGAGED
	ENGSTR:[DTE WAITING]
	ENGSTR:engaged
	SETSTR:SET,1:1,2:0,3:2,4:10,5:1,6:1,7:2,8:0,9:0,10:0,12:1,13:0
	CONNTIMEOUT:30
	PADESCCHAR:^P
	CALLPREFIX:C
*/

#ifdef sgi
#define _SGI_MP_SOURCE
#endif
#include "rdr.h"
#include "utils.h"
#include "spinlock.h"
#include <sys/termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string>
#include <map>
#include "ipnamerescache.h"

// if these enumerations are expanded, the Get.... functions in
// rpcomms.c will need to have the new strings added, and the upper
// limit enumeration modified
enum	ConnModes {CM_GENERIC,CM_SERIAL,CM_X28,CM_HAYES,CM_SOCKET,CM_SOCKX25};
enum	ConnStates {IDLE,CONNECTREQ,CONNECTED,CONNLOST,LISTENING,BROKENPIPE};
enum	CommErr	{OK,NOTOPEN,PORTERR,HAYESIFACE,X28IFACE,SOCKHOST,SOCKCONN,NOSOCK,HOSTERR};
enum 	ConnFails {CONN,BUSY,TMOUT,DTE,DER};
enum	ConnDirs {OUTGOING,INCOMING};

extern	char *ConnStateStr[];
extern	char *ConnModeString[];
extern  bool noRp_SO_KEEPALIVE;
extern  bool noRp_SO_LINGER;

ConnModes Str2ConnMode(char *str);

/* 
	wait_resplist will wait for a string from fdes which matches
	any of the strings in strlist.
	Returns TRUE if matched before timeout expires else FALSE
*/

bool wait_resplist(int fdes, strlist *resplist, int timeout);

void InitGetAddrByName();
int GetAddrByName(const char *name, in_addr *rethostaddr);
void InitGetNameByAddr();
int GetNameByAddr(const void *addr, char *retname, int maxchars);

class Comm {
public:
	Comm(); 	
	virtual ~Comm();
	ConnModes		ConnMode;				// Connection mode of this object
	ConnDirs		ConnDir;				// call direction INCOMING/OUTGOING
	ConnStates		ConnState;			//	Connection state
	bool                    isConnected() 
	  { return ConnState == CONNECTED; };
	ConnFails		ConnFail;				// last conn fail mode
	int			ConnReqCount;		// number of (re)connections attempted
	int			ConnCount;			// number of (re)connections successful
	int			commfd;							// comm. file descriptor
	int			ConnectTimeOut;		// tenth of secs
	bool			IsSocket;						// true for sockets to allow recv/send use
	bool			quiet;
	bool			terminate;  // if set, fail straight out of any timed loops
	FILE			*commfile;					// comm. FILE (SAME AS commfd)
	virtual bool		Open(char *pathname = 0);
	virtual bool		Close();		// base close
	/*
		Init is the generic initialise and will reset the state of the 
		Comm object. THIS WILL ALSO CLOSE AN OPEN Comm object
	*/
	virtual bool		Init(char *initstring = 0);
	virtual int		Read(char	*RdBuff = 0,int maxch = -1,int pos = 0);	// return number read
	virtual int		Write(char *WrBuff = 0,int numch = -1,int pos = 0);// return number written
	/*
	 * WriteBuffer will try to tx sz bytes of data from the buffer
	 * It will return -1 for error,  0 if not complete,  1 if complete
	 * WriteBuffer can be called repeatedly with NO PARAMETERS
	 * to continue sending a buffer until completion
	 */
	virtual int		WriteBuffer(char *buffer = 0, int sz = -1);
	virtual ConnStates	Connect(char* connstr, int timeout = 0);		// generic connect
	virtual	ConnStates	ConnectNB(char* connstr);	// generic connect(non-block)
	virtual	ConnStates	Listen(char *_string = 0);	// initiate incoming connection mode
	virtual	ConnStates	CheckListen(int fd = -1);	// check for incoming connection
	virtual ConnStates	CheckConnect();	// check non-block connect
	virtual ConnStates	Reconnect(int timeout = 0);	// reconnect, to last dest.
	virtual ConnStates	Disconnect();// generic disconnect
	virtual ConnStates	getConnState();// returns ConnState
	virtual	void		FlushIO();	// generic flush of I/O
	int 			PacedWrite(char* wrtstr, int msperch);
	virtual bool		WaitResp(strlist* resplist, int tenthstmout);
	virtual bool		WaitRespSuccFail(strlist* succlist, 
				    strlist* faillist,  
				    int tenthstmout);
	// virtual sighandler(int signo, ... );
	char			*instr,*outstr;
	int			instrmax,outstrmax,instrlen,outstrlen;
	CommErr			err;
	void			NewInOutStr(int	InSz,int OutSz);
	char			*writebuffer;
	int			writebuffsz, writebuffoutpos, writebuffcomplete;
	bool			HoldConnect;
	char			lastconnstr[128];
	char			connectedtostr[128];
	char			InitString[128];
	void			SetInitString(char *initstr);
	};


class Serial : public Comm {		// std tty services
public:
	Serial();
	~Serial();
	void			FlushIO();
	virtual 	bool 	Open(char *portname = 0);
	virtual		bool 	Close();
	ConnStates	Connect(char *connstr, int timeout = 0);		// blocking connect call
	ConnStates	Reconnect(int timeout = 0);	// reconnect, to last dest.
	ConnStates	Disconnect();
	virtual		bool 	Init(char *initstring = 0);
  virtual		void			SetTermio();
	virtual		void			SetTermParams();
	void			SetVminVtm(int Vmin, int Vtm);
	void			SetBaud(tcflag_t Baud);
	virtual 	bool WaitResp(strlist* resplist, int tenthstmout);
	termios  	termdta,oldtermdta;
	int				vtime,vmin;
	char			PortName[128];
	};

class X28 : public Serial {
public:
	X28(char *x28params = 0); 	// text file containing startup params
	~X28();
	bool 	Init(char *initstring = 0); 			
	virtual   bool   Open(char *portname = 0);
	virtual   bool   Close();
	ConnStates Connect(char *connstr, int timeout = 0);		// blocking connect call
	ConnStates ConnectNB(char *connstr); 	// non-blocking connect call
	virtual	ConnStates	Listen(char *_string);			// set up for incoming connection
	virtual	ConnStates	CheckListen(int fd = -1);		// check for incoming connection
	ConnStates CheckConnect();	// poll non-blocked connect state progress
	ConnStates Disconnect();
	ConnStates	Reconnect(int timeout = 0);
	bool		X28Free();
private:
	bool		PadEsc();
	void		ReadX28Params(char *x28params = 0);
	char		x28portstr[128],setstr[128];
	bool		timeout;
	strlist		engstr,freestr,connstr,disconnstr,padprompt;
	int		x28timer;
	char		padescstr[32];
	char		callprefix[64],disconncmd[64];
	};

class Hayes : public Serial {
public:
  Hayes(char *Hayesparams = 0);   // text file containing startup params
  ~Hayes();
  bool   Init(char *initstring = 0);       
  virtual   bool   Open(char *portname = 0);
  virtual   bool   Close();
  ConnStates Connect(char *connstr, int timeout = 0);    // blocking connect call
  ConnStates ConnectNB(char *connstr);  // non-blocking connect call
  ConnStates CheckConnect();  // poll non-blocked connect state progress
  ConnStates Disconnect();
  ConnStates  Reconnect(int timeout = 0);
private:
  bool   HayesEsc();
  void      ReadHayesParams(char *x28params = 0);
  char      Hayesportstr[128],DefaultInitStr[128];
  bool   timeout;
  strlist   engstr,freestr,connsuccstr,connfailstr, disconnstr,Hayesprompt;
  int       Hayestimer;
  char      Hayesescstr[16], HayesAT[16];
  char      callprefix[64],disconncmd[64];
  };


// The string passed to the Listen call MUST be of the form
// "listen_fd port_no"
class rpSocket : public Comm {    // std tcp/ip Socket services
public:
	rpSocket();
	~rpSocket();
	virtual		void	    FlushIO();
	virtual		bool	    Open(char *pathname = 0);	// pathname not relevant
	virtual		bool	    Close();
	virtual	 	ConnStates  Connect(char *hostname, int timeout = 0); // hostname may be 
	virtual		ConnStates  ConnectNB(char *hostname);	// "name port"
	virtual		ConnStates  Listen(char *_string = 0);	// wait for incoming connection, string is ASCII 
	virtual		ConnStates  CheckListen(int fd = -1);	// check for incoming connection
	virtual 	ConnStates  CheckConnect();
	virtual		ConnStates  Disconnect();
	virtual		ConnStates  Reconnect(int timeout = 0);
	virtual		bool	    setSockOpts(int fd = -1);
	sockaddr_in	sin;
	u_short		connectPort;
	u_short		listenPort;
	int		listenfd;
	char		listenstr[128];	// string contains "listenfd listenPort"
	int		shutdownmode;
	};

// The string passed to the Listen call MUST be of the form
// "listen_fd port_no"
/* class rpUDPSocket : public Comm {    // UDP Socket services */
/* public: */
/* 	rpSocket(); */
/* 	~rpSocket(); */
/* 	virtual		bool	    Open(char *pathname = 0);	// pathname not relevant */
/* 	virtual		bool	    Close(); */
/* 	virtual	 	ConnStates  Connect(char *hostname, int timeout = 0); // hostname may be  */
/* 	virtual		ConnStates  ConnectNB(char *hostname);	// "name port" */
/* 	virtual		ConnStates  Listen(char *_string = 0);	// wait for incoming connection, string is ASCII  */
/* 	virtual		ConnStates  CheckListen(int fd = -1);	// check for incoming connection */
/* 	virtual 	ConnStates  CheckConnect(); */
/* 	virtual		ConnStates  Disconnect(); */
/* 	virtual		ConnStates  Reconnect(int timeout = 0); */
/* 	virtual		bool	    setSockOpts(int fd = -1); */
/* 	sockaddr_in	sin; */
/* 	u_short		connectPort; */
/* 	u_short		listenPort; */
/* 	int		listenfd; */
/* 	char		listenstr[128];	// string contains "listenfd listenPort" */
/* 	int		shutdownmode; */
/* 	}; */

/*
 * "2 step" Socket/X25 comms
 * ProtXlatHost stays constant, X25 "address string" changes
 */

class SockX25 : public rpSocket {		
	char	  ProtXlatHost[128];
public:
	SockX25();
	~SockX25();
	virtual	  bool	Open(char *protxlatname = 0);	// ip addr [port] of prot xlat
	virtual   ConnStates Connect(char *address, int timeout = 0);  // address form = host port X25addr
	virtual   ConnStates ConnectNB(char *X25Addr);
	virtual   ConnStates CheckConnect();
//	virtual	  ConnStates	Listen(char *_string = 0);			// initiate incoming connection mode
	virtual   ConnStates Disconnect();
	virtual   ConnStates Reconnect(int timeout = 0);
	strlist	  hostprompt,connstr,confirmstr;
	char	  hostesc[64];
	char 	  disconnstr[64],closestr[64];
	char	  lastconnstr[64],callprefix[64];
	};

#endif /* __COMMS_H	*/

