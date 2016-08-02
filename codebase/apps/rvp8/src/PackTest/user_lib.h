/* *************************************
 * *                                   *
 * *  User Library Related Structures  *
 * *                                   *
 * *************************************
 * File: include/user_lib.h
 *
 *  COPYRIGHT (c) 1989, 1990, 1992, 1995, 1996, 1997, 1998, 2001, 2002,
 *                    2003, 2004, 2005, 2006 BY 
 *              VAISALA INC., WESTFORD MASSACHUSETTS, U.S.A.  
 * 
 * THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
 * ONLY  IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
 * INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE  OR  ANY OTHER
 * COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
 * OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
 * TRANSFERED. 
 */ 
#ifndef SIGMET_USER_LIB_H
#define SIGMET_USER_LIB_H 1

#define      BADSEM         1
#define USER_BADSEM         (USER_FACILITY + BADSEM)
#define      BADEF          2
#define USER_BADEF          (USER_FACILITY + BADEF)
#define      BWL_UNLEV      3
#define USER_BWL_UNLEV      (USER_FACILITY + BWL_UNLEV)
#define      CANTBE_OPER    4
#define USER_CANTBE_OPER    (USER_FACILITY + CANTBE_OPER)
#define      CANTBE_ROOT    5
#define USER_CANTBE_ROOT    (USER_FACILITY + CANTBE_ROOT)
#define      CANTUID_OPER   6
#define USER_CANTUID_OPER   (USER_FACILITY + CANTUID_OPER)
#define      CANT_SETPRI    7
#define USER_CANT_SETPRI    (USER_FACILITY + CANT_SETPRI)
#define      CLOSEDIR_SEQ   8
#define USER_CLOSEDIR_SEQ   (USER_FACILITY + CLOSEDIR_SEQ)
#define      CLUSTER_BAD    9
#define USER_CLUSTER_BAD    (USER_FACILITY + CLUSTER_BAD)
#define      CLUSTER_ERROR  10
#define USER_CLUSTER_ERROR  (USER_FACILITY + CLUSTER_ERROR)
#define      CLUSTER_CHANGE 11
#define USER_CLUSTER_CHANGE (USER_FACILITY + CLUSTER_CHANGE)
#define      CLUSTER_UNDEF  12
#define USER_CLUSTER_UNDEF  (USER_FACILITY + CLUSTER_UNDEF)
#define      DEVALLOC       13
#define USER_DEVALLOC       (USER_FACILITY + DEVALLOC)
#define      DEVTIMEOUT     14
#define USER_DEVTIMEOUT     (USER_FACILITY + DEVTIMEOUT)
#define      ERROR_SIG      15
#define USER_ERROR_SIG      (USER_FACILITY + ERROR_SIG)
#define      EXIT_CALLED    16
#define USER_EXIT_CALLED    (USER_FACILITY + EXIT_CALLED)
#define      FILE_NAME      19
#define USER_FILE_NAME      (USER_FACILITY + FILE_NAME )
#define      IMAPADDR       20
#define USER_IMAPADDR       (USER_FACILITY + IMAPADDR)
#define      IMAPCHAN       21
#define USER_IMAPCHAN       (USER_FACILITY + IMAPCHAN)
#define      IMAPFULL       22
#define USER_IMAPFULL       (USER_FACILITY + IMAPFULL)
#define      IMAPSIZE       23
#define USER_IMAPSIZE       (USER_FACILITY + IMAPSIZE)
#define      LOCK_ORDER     24
#define USER_LOCK_ORDER     (USER_FACILITY + LOCK_ORDER)
#define      TEXTMSG        25
#define USER_TEXTMSG        (USER_FACILITY + TEXTMSG)

#define      NETMAX_EXCEEDED     27
#define USER_NETMAX_EXCEEDED     (USER_FACILITY + NETMAX_EXCEEDED)
#define      NETPACKET_COMMAND   28
#define USER_NETPACKET_COMMAND   (USER_FACILITY + NETPACKET_COMMAND)
#define      NETPACKET_NOFILE    29
#define USER_NETPACKET_NOFILE    (USER_FACILITY + NETPACKET_NOFILE)
#define      NETPACKET_NOZERO    30
#define USER_NETPACKET_NOZERO    (USER_FACILITY + NETPACKET_NOZERO)
#define      NETPACKET_OVERFLOW  31
#define USER_NETPACKET_OVERFLOW  (USER_FACILITY + NETPACKET_OVERFLOW)
#define      NETPORT_INUSE       32
#define USER_NETPORT_INUSE       (USER_FACILITY + NETPORT_INUSE)
#define      NETPORT_INVALID     33
#define USER_NETPORT_INVALID     (USER_FACILITY + NETPORT_INVALID)

#define      NO_ENV         35
#define USER_NO_ENV         (USER_FACILITY + NO_ENV)
#define      NO_FTOK        36
#define USER_NO_FTOK        (USER_FACILITY + NO_FTOK)
#define      OPENDIR_SEQ    37
#define USER_OPENDIR_SEQ    (USER_FACILITY + OPENDIR_SEQ)
#define      PROCESS_START  38
#define USER_PROCESS_START  (USER_FACILITY + PROCESS_START)
#define      RELOCK         40
#define USER_RELOCK         (USER_FACILITY + RELOCK)
#define      REUNLOCK       41
#define USER_REUNLOCK       (USER_FACILITY + REUNLOCK)
#define      SCH_CONFLICT   42
#define USER_SCH_CONFLICT   (USER_FACILITY + SCH_CONFLICT)
#define      SIG_FREE       43
#define USER_SIG_FREE       (USER_FACILITY + SIG_FREE)
#define      SIG_MALLOC     44
#define USER_SIG_MALLOC     (USER_FACILITY + SIG_MALLOC)
#define      TEST_NUMBER    46
#define USER_TEST_NUMBER    (USER_FACILITY + TEST_NUMBER)
#define      TEST_STRING    47
#define USER_TEST_STRING    (USER_FACILITY + TEST_STRING)
#define      UNCOMP_IN      48
#define USER_UNCOMP_IN      (USER_FACILITY + UNCOMP_IN)
#define      UNCOMP_OUT     49
#define USER_UNCOMP_OUT     (USER_FACILITY + UNCOMP_OUT)
#define      UNIX_SIG       50
#define USER_UNIX_SIG       (USER_FACILITY + UNIX_SIG)
#define      USE_MMAP       51
#define USER_USE_MMAP       (USER_FACILITY + USE_MMAP)
#define      USE_SHORT      52
#define USER_USE_SHORT       (USER_FACILITY + USE_SHORT)
#define      WAITEF         53
#define USER_WAITEF         (USER_FACILITY + WAITEF)
#define      INTERNALERR    54
#define USER_INTERNALERR    (USER_FACILITY + INTERNALERR)
#define      ZLEN_FILE      55
#define USER_ZLEN_FILE      (USER_FACILITY + ZLEN_FILE)
#define      NOTAFIFO       56
#define USER_NOTAFIFO       (USER_FACILITY + NOTAFIFO)
#define      NOTATTY        57
#define USER_NOTATTY        (USER_FACILITY + NOTATTY)
#define      HOST_RESOLVE     58
#define USER_HOST_RESOLVE     (USER_FACILITY + HOST_RESOLVE)
#define      HOST_UNREACHABLE 59
#define USER_HOST_UNREACHABLE (USER_FACILITY + HOST_UNREACHABLE)
#define      HOST_NOLISTEN    60
#define USER_HOST_NOLISTEN    (USER_FACILITY + HOST_NOLISTEN)
#define      HOST_REACHABLE   61
#define USER_HOST_REACHABLE   (USER_FACILITY + HOST_REACHABLE)
#define      SHMEM_FOUND      62
#define USER_SHMEM_FOUND      (USER_FACILITY + SHMEM_FOUND)
#define      SHMEM_MISSING    63
#define USER_SHMEM_MISSING    (USER_FACILITY + SHMEM_MISSING)
#define      DAEMON_FD        64
#define USER_DAEMON_FD        (USER_FACILITY + DAEMON_FD)
#define      NOT_LOCKED       65
#define USER_NOT_LOCKED       (USER_FACILITY + NOT_LOCKED )
#define      INVENTORY_DUP    66
#define USER_INVENTORY_DUP    (USER_FACILITY + INVENTORY_DUP )
#define      INVENTORY_FULL   67
#define USER_INVENTORY_FULL   (USER_FACILITY + INVENTORY_FULL )
#define      NET_SERVER_DOWN  68
#define USER_NET_SERVER_DOWN  (USER_FACILITY + NET_SERVER_DOWN )
#define      NET_SERVER_UP    69
#define USER_NET_SERVER_UP    (USER_FACILITY + NET_SERVER_UP )

#define      ERRNO_EPERM           100
#define USER_ERRNO_EPERM           (USER_FACILITY+ERRNO_EPERM)
#define      ERRNO_ENOENT          101
#define USER_ERRNO_ENOENT          (USER_FACILITY+ERRNO_ENOENT)
#define      ERRNO_ESRCH           102
#define USER_ERRNO_ESRCH           (USER_FACILITY+ERRNO_ESRCH)
#define      ERRNO_EINTR           103
#define USER_ERRNO_EINTR           (USER_FACILITY+ERRNO_EINTR)
#define      ERRNO_EIO             104
#define USER_ERRNO_EIO             (USER_FACILITY+ERRNO_EIO)
#define      ERRNO_ENXIO           105
#define USER_ERRNO_ENXIO           (USER_FACILITY+ERRNO_ENXIO)
#define      ERRNO_E2BIG           106
#define USER_ERRNO_E2BIG           (USER_FACILITY+ERRNO_E2BIG)
#define      ERRNO_ENOEXEC         107
#define USER_ERRNO_ENOEXEC         (USER_FACILITY+ERRNO_ENOEXEC)
#define      ERRNO_EBADF           108
#define USER_ERRNO_EBADF           (USER_FACILITY+ERRNO_EBADF)
#define      ERRNO_ECHILD          109
#define USER_ERRNO_ECHILD          (USER_FACILITY+ERRNO_ECHILD)
#define      ERRNO_EAGAIN          110
#define USER_ERRNO_EAGAIN          (USER_FACILITY+ERRNO_EAGAIN)
#define      ERRNO_ENOMEM          111
#define USER_ERRNO_ENOMEM          (USER_FACILITY+ERRNO_ENOMEM)
#define      ERRNO_EACCES          112
#define USER_ERRNO_EACCES          (USER_FACILITY+ERRNO_EACCES)
#define      ERRNO_EFAULT          113
#define USER_ERRNO_EFAULT          (USER_FACILITY+ERRNO_EFAULT)
#define      ERRNO_ENOTBLK         114
#define USER_ERRNO_ENOTBLK         (USER_FACILITY+ERRNO_ENOTBLK)
#define      ERRNO_EBUSY           115
#define USER_ERRNO_EBUSY           (USER_FACILITY+ERRNO_EBUSY)
#define      ERRNO_EEXIST          116
#define USER_ERRNO_EEXIST          (USER_FACILITY+ERRNO_EEXIST)
#define      ERRNO_EXDEV           117
#define USER_ERRNO_EXDEV           (USER_FACILITY+ERRNO_EXDEV)
#define      ERRNO_ENODEV          118
#define USER_ERRNO_ENODEV          (USER_FACILITY+ERRNO_ENODEV)
#define      ERRNO_ENOTDIR         119
#define USER_ERRNO_ENOTDIR         (USER_FACILITY+ERRNO_ENOTDIR)
#define      ERRNO_EISDIR          120
#define USER_ERRNO_EISDIR          (USER_FACILITY+ERRNO_EISDIR)
#define      ERRNO_EINVAL          121
#define USER_ERRNO_EINVAL          (USER_FACILITY+ERRNO_EINVAL)
#define      ERRNO_ENFILE          122
#define USER_ERRNO_ENFILE          (USER_FACILITY+ERRNO_ENFILE)
#define      ERRNO_EMFILE          123
#define USER_ERRNO_EMFILE          (USER_FACILITY+ERRNO_EMFILE)
#define      ERRNO_ENOTTY          124
#define USER_ERRNO_ENOTTY          (USER_FACILITY+ERRNO_ENOTTY)
#define      ERRNO_ETXTBSY         125
#define USER_ERRNO_ETXTBSY         (USER_FACILITY+ERRNO_ETXTBSY)
#define      ERRNO_EFBIG           126
#define USER_ERRNO_EFBIG           (USER_FACILITY+ERRNO_EFBIG)
#define      ERRNO_ENOSPC          127
#define USER_ERRNO_ENOSPC          (USER_FACILITY+ERRNO_ENOSPC)
#define      ERRNO_ESPIPE          128
#define USER_ERRNO_ESPIPE          (USER_FACILITY+ERRNO_ESPIPE)
#define      ERRNO_EROFS           129
#define USER_ERRNO_EROFS           (USER_FACILITY+ERRNO_EROFS)
#define      ERRNO_EMLINK          130
#define USER_ERRNO_EMLINK          (USER_FACILITY+ERRNO_EMLINK)
#define      ERRNO_EPIPE           131
#define USER_ERRNO_EPIPE           (USER_FACILITY+ERRNO_EPIPE)
#define      ERRNO_EDOM            132
#define USER_ERRNO_EDOM            (USER_FACILITY+ERRNO_EDOM)
#define      ERRNO_ERANGE          133
#define USER_ERRNO_ERANGE          (USER_FACILITY+ERRNO_ERANGE)
#define      ERRNO_EWOULDBLOCK     134
#define USER_ERRNO_EWOULDBLOCK     (USER_FACILITY+ERRNO_EWOULDBLOCK)
#define      ERRNO_EINPROGRESS     135
#define USER_ERRNO_EINPROGRESS     (USER_FACILITY+ERRNO_EINPROGRESS)
#define      ERRNO_EALREADY        136
#define USER_ERRNO_EALREADY        (USER_FACILITY+ERRNO_EALREADY)
#define      ERRNO_ENOTSOCK        137
#define USER_ERRNO_ENOTSOCK        (USER_FACILITY+ERRNO_ENOTSOCK)
#define      ERRNO_EDESTADDRREQ    138
#define USER_ERRNO_EDESTADDRREQ    (USER_FACILITY+ERRNO_EDESTADDRREQ)
#define      ERRNO_EMSGSIZE        139
#define USER_ERRNO_EMSGSIZE        (USER_FACILITY+ERRNO_EMSGSIZE)
#define      ERRNO_EPROTOTYPE      140
#define USER_ERRNO_EPROTOTYPE      (USER_FACILITY+ERRNO_EPROTOTYPE)
#define      ERRNO_ENOPROTOOPT     141
#define USER_ERRNO_ENOPROTOOPT     (USER_FACILITY+ERRNO_ENOPROTOOPT)
#define      ERRNO_EPROTONOSUPPORT 142
#define USER_ERRNO_EPROTONOSUPPORT (USER_FACILITY+ERRNO_EPROTONOSUPPORT)
#define      ERRNO_ESOCKTNOSUPPORT 143
#define USER_ERRNO_ESOCKTNOSUPPORT (USER_FACILITY+ERRNO_ESOCKTNOSUPPORT)
#define      ERRNO_EOPNOTSUPP      144
#define USER_ERRNO_EOPNOTSUPP      (USER_FACILITY+ERRNO_EOPNOTSUPP)
#define      ERRNO_EPFNOSUPPORT    145
#define USER_ERRNO_EPFNOSUPPORT    (USER_FACILITY+ERRNO_EPFNOSUPPORT)
#define      ERRNO_EAFNOSUPPORT    146
#define USER_ERRNO_EAFNOSUPPORT    (USER_FACILITY+ERRNO_EAFNOSUPPORT)
#define      ERRNO_EADDRINUSE      147
#define USER_ERRNO_EADDRINUSE      (USER_FACILITY+ERRNO_EADDRINUSE)
#define      ERRNO_EADDRNOTAVAIL   148
#define USER_ERRNO_EADDRNOTAVAIL   (USER_FACILITY+ERRNO_EADDRNOTAVAIL)
#define      ERRNO_ENETDOWN        149
#define USER_ERRNO_ENETDOWN        (USER_FACILITY+ERRNO_ENETDOWN)
#define      ERRNO_ENETUNREACH     150
#define USER_ERRNO_ENETUNREACH     (USER_FACILITY+ERRNO_ENETUNREACH)
#define      ERRNO_ENETRESET       151
#define USER_ERRNO_ENETRESET       (USER_FACILITY+ERRNO_ENETRESET)
#define      ERRNO_ECONNABORTED    152
#define USER_ERRNO_ECONNABORTED    (USER_FACILITY+ERRNO_ECONNABORTED)
#define      ERRNO_ECONNRESET      153
#define USER_ERRNO_ECONNRESET      (USER_FACILITY+ERRNO_ECONNRESET)
#define      ERRNO_ENOBUFS         154
#define USER_ERRNO_ENOBUFS         (USER_FACILITY+ERRNO_ENOBUFS)
#define      ERRNO_EISCONN         155
#define USER_ERRNO_EISCONN         (USER_FACILITY+ERRNO_EISCONN)
#define      ERRNO_ENOTCONN        156
#define USER_ERRNO_ENOTCONN        (USER_FACILITY+ERRNO_ENOTCONN)
#define      ERRNO_ESHUTDOWN       157
#define USER_ERRNO_ESHUTDOWN       (USER_FACILITY+ERRNO_ESHUTDOWN)
#define      ERRNO_ETOOMANYREFS    158
#define USER_ERRNO_ETOOMANYREFS    (USER_FACILITY+ERRNO_ETOOMANYREFS)
#define      ERRNO_ETIMEDOUT       159
#define USER_ERRNO_ETIMEDOUT       (USER_FACILITY+ERRNO_ETIMEDOUT)
#define      ERRNO_ECONNREFUSED    160
#define USER_ERRNO_ECONNREFUSED    (USER_FACILITY+ERRNO_ECONNREFUSED)
#define      ERRNO_ELOOP           161
#define USER_ERRNO_ELOOP           (USER_FACILITY+ERRNO_ELOOP)
#define      ERRNO_ENAMETOOLONG    162
#define USER_ERRNO_ENAMETOOLONG    (USER_FACILITY+ERRNO_ENAMETOOLONG)
#define      ERRNO_EHOSTDOWN       163
#define USER_ERRNO_EHOSTDOWN       (USER_FACILITY+ERRNO_EHOSTDOWN)
#define      ERRNO_EHOSTUNREACH    164
#define USER_ERRNO_EHOSTUNREACH    (USER_FACILITY+ERRNO_EHOSTUNREACH)
#define      ERRNO_ENOTEMPTY       168
#define USER_ERRNO_ENOTEMPTY       (USER_FACILITY+ERRNO_ENOTEMPTY)

#define      ERRNO_NONSIGMET       169
#define USER_ERRNO_NONSIGMET       (USER_FACILITY+ERRNO_NONSIGMET)

#ifndef MESSAGE_ONLY                      

/* Standard types are included in sigtypes.h.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* =============== Function prototypes for user library =================== */

/* Big Bit Field Set/Clear/Test routines.  These permit you to pack
 * bit fields more efficiently.  The return value is the value of the
 * bit prior to the call.  Three different routines are defined for
 * one-byte, two-byte, and four-byte internal accesses.  This is only
 * important if the array will also be accessed by other C code.
 */
UINT1 bbput_1( volatile void *barray_a, SINT4 ibit_a, UINT1 iVal_a ) ;
UINT1 bbput_2( volatile void *barray_a, SINT4 ibit_a, UINT1 iVal_a ) ;
UINT1 bbput_4( volatile void *barray_a, SINT4 ibit_a, UINT1 iVal_a ) ;

UINT1 bbclr_1( volatile void *barray_a, SINT4 ibit_a ) ;
UINT1 bbclr_2( volatile void *barray_a, SINT4 ibit_a ) ;
UINT1 bbclr_4( volatile void *barray_a, SINT4 ibit_a ) ;

UINT1 bbset_1( volatile void *barray_a, SINT4 ibit_a ) ;
UINT1 bbset_2( volatile void *barray_a, SINT4 ibit_a ) ;
UINT1 bbset_4( volatile void *barray_a, SINT4 ibit_a ) ;

UINT1 bbtst_1( volatile const void *barray_a, SINT4 ibit_a ) ;
UINT1 bbtst_2( volatile const void *barray_a, SINT4 ibit_a ) ;
UINT1 bbtst_4( volatile const void *barray_a, SINT4 ibit_a ) ;

/* Byte-fifo routines.  The FIFOs hold a string of bytes (actually,
 * unsigned shorts, so that extra bits can be passed with each byte if
 * needed).  Use the BFIFO macro to create the FIFO itself.
 */
void  bfifo_init ( volatile void *bfifo_a, SINT4 structsize_a ) ;
SINT4 bfifo_read ( volatile void *bfifo_a ) ;
SINT4 bfifo_read_array( volatile void *bfifo_a, UINT1 *chrs_a, SINT4 iMaxCount_a );
SINT4 bfifo_write( volatile void *bfifo_a, UINT2 chr_a ) ;
SINT4 bfifo_write_array( volatile void *bfifo_a, const UINT1 *chrs_a, SINT4 iCount_a );
SINT4 bfifo_used ( volatile void *bfifo_a ) ;
SINT4 bfifo_free ( volatile void *bfifo_a ) ;

/* Discrete Fourier Transform and Window routines
 */
FLT4 fSetWindowCoefficients
( SINT4 iWinType_a, SINT4 iSize_a, FLT4 fWinCoefs_a[3] ) ;

FLT4 fWindowValue( FLT4 fPoint_a, SINT4 iSize_a, const FLT4 fWinCoefs_a[3] ) ;
FLT4 fWindowDepthDB( SINT4 iWinType_a ) ;

MESSAGE dft_complex
( const FLT4 *freal_in_a,	/* Real input data */
  const FLT4 *fimag_in_a,	/* Imaginary input data */
  SINT4 inc_in_a,		/* Increment between input values */
  FLT4 *freal_out_a,		/* Real and imaginary output data */
  FLT4 *fimag_out_a,		/*   (okay to overlap input data) */
  SINT4 inc_out_a,		/* Increment between output values */
  SINT4 iSize_a,		/* Transform size */
  UINT4 iflags_a ) ;		/* Control Flags (from FFT_xxx) */

/* Simple angle conversion and testing routines.  A few of these
 * functions are also available in macro form when you're concerned
 * about overhead in function calls.
 */
SINT4 binside( BIN2 left_a, BIN2 test_a, BIN2 right_a ) ;
SINT4 binEncompased( BIN2 left_a, BIN2 test_a, BIN2 right_a ) ;

FLT4 fElDegFromBin2( BIN2 binang ) ;
FLT4 fPDegFromBin2( BIN2 binang ) ;
FLT4 fRadianFromBin2( BIN2 binang ) ;
FLT4 fDegFromBin2( BIN2 binang ) ;

FLT8 fElDegFromBin4( BIN4 binang ) ;
FLT8 fPDegFromBin4( BIN4 binang ) ;
FLT8 fDegFromBin4( BIN4 binang ) ;
FLT8 fRadianFromBin4( BIN4 binang ) ;

FLT8 fDegFromText( const char *sAngle_a ) ;

BIN2 iBin2FromBcd( UINT2 ibcd_a ) ;
BIN2 iBin2FromFDeg( FLT4 angle ) ;
BIN2 iBin2FromRadian( FLT4 fRadian_a ) ;

BIN4 iBin4FromFDeg( FLT8 angle ) ;
BIN4 iBin4FromRadian( FLT8 angle ) ;

#define IBIN2FROMRADIAN( RADIAN ) \
  ( (BIN2)( 0xFFFF & NINT( (RADIAN) * (32768.0 / FPI  ) ) ) )
#define IBIN2FROMFDEG( ANGLE ) \
  ( (BIN2)( 0xFFFF & NINT( (ANGLE)  * (32768.0 / 180.0) ) ) )

/* Latitude and Longitude conversion routines.  These have both
 * reentrent and non-reentrent forms.
 */
char* sLatFromRadian
( FLT8 fAngle_a,		/* Angle in radians */
  char *sBuffer_a,
  int  iBufLen_a );
char* sLonFromRadian
  ( FLT8 fAngle_a,		/* Angle in radians */
    char *sBuffer_a,
    int  iBufLen_a );

char* srdeg_to_lat		/* NOT REENTRENT! */
( FLT4 latitude_a );
char* srdeg_to_lon		/* NOT REENTRENT! */
( FLT4 longitude_a );

SINT4 c_cmd_lookup		/* In file c_cmd_lookup.c */
  (char* scmd_a,		/* Character string to lookup */
   char* slist_a[],		/* Array of strings to compare against */
   SINT4 ilistn_a,		/* Number of strings in the list */
   UINT2* lamb_a		/* Set to non-zero if ambiguous */
   );

void cancel_schedule_ef		/* File: schedule.c */
  ( void ) ;

void compress_words		/* File: compress.c */
 (const SINT2 inbuf_a[],	/* Input array of data to be compressed */
  SINT4 inlen_a,		/* Length of INBUF (Positive or zero) */
  SINT2 ioutbuf_a[],		/* Output array to hold compressed data */
  SINT4 *ioutlen_a) ;		/* Number of words written to IOUTBUF */

void convert_alt_data		/* File: data_types.c */
  (UINT4 idtype_a,
   SINT4 icount_a,
   const UINT1 *in_a,
   UINT1 *out_a,
   const struct data_convert *pconvert_a ) ;

  void ConvertDataFrom8           /* File: data_types.c */
(UINT4 iInDType_a,     /* Input data type */
 SINT4 icount_a,       /* Number of bins */
 const UINT1 *in_a,    /* Input pointer */
 UINT2 *out_a,         /* Output pointer */
 const struct data_convert *pconvert_a ); /* Convert structure */

  void ConvertDataFrom16            /* File: data_types.c */
(UINT4 iInDType_a,     /* Input data type */
 SINT4 icount_a,       /* Number of bins */
 const UINT2 *in_a,    /* Input pointer */
 UINT1 *out_a,         /* Output pointer */
 const struct data_convert *pconvert_a ); /* Convert structure */

MESSAGE create_sem_set		/* File: semaphore.c */
( const char* sname_a,
  UINT4 isem_count_a,
  UINT4 iflg_count_a,
  UINT4 isem_cluster_a ) ;

  /* Given two points on the earth, return the distance between them
   * and the azimuths from each to the other.
   */
void earth_to_polar
  (volatile const struct latlon8 *earth1_a,
   volatile const struct latlon8 *fearth2_a,
   volatile FLT8 *frange_a,
   volatile FLT8 *faz1to2_a,
   volatile FLT8 *faz2to1_a) ;

  /* Routine to convert a starting point, and a polar offset to an ending
   * point in earth coordinates.  This routine will not work if the starting
   * point is at either the North or South poles.  Also the range must be
   * less than half way around the earth.
   */
void EarthFromPolar
(volatile const struct latlon8 *Start_a,   /* Origin position in radians */
 volatile const struct D2Polar8 *pPolar_a,
 volatile struct latlon8 *End_a);

MESSAGE errno_to_message	/* File: error_report.c */
  ( int errno_a ); 

MESSAGE errno_to_message_va	
  ( int errno_a, UINT4 icount_a, ... ) ; 

FLT4  fambiguous_range		/* File: fambiguous_range.c */
  ( SINT4 prf );


FLT4 fcurverange                /* File: earth.c */
  ( FLT4 fheight_a );

FLT4 fDataScalePerBit           /* File: data_types.c */
  (UINT4 idtype_a,
   const struct data_convert *pconvert_a );

FLT4 fDataStartValue            /* File: data_types.c */
( UINT4 idtype_a,
  const struct data_convert *pconvert_a );

FLT4 fDataScaleStartLimit
( UINT2 idata_a, FLT4 fNumIn_a );

FLT4 fDataScaleStepLimit
( UINT2 idata_a, FLT4 fNumIn_a );

FLT4  fdata_to_user		/* File: data_types.c */
  (UINT2 idata,
   UINT4 idtype,
   const struct data_convert *pconvert_a );

FLT8 fear_from_slr_elv          /* File: earth.c */
  ( FLT8 fslr_a, FLT8 felv_a );

FLT4 fearthcurve                /* File: earth.c */
  ( FLT4 frange_a );

FLT8 felv_from_ear_hgt          /* File: earth.c */
  ( FLT8 fear_a, FLT8 fhgt_a, FLT8 fTow_a );

  FLT8 felv_from_slr_hgt          /* File: earth.c */
( FLT8 fslr_a, FLT8 fhgt_a, FLT8 fTower_a );

FLT8 fhgt_from_ear_elv          /* File: earth.c */
( FLT8 fear_a, FLT8 felv_a, FLT8 fTow_a );

FLT8 fhgt_from_slr_ear          /* File: earth.c */
( FLT8 fslr_a, FLT8 fear_a, FLT8 fTow_a );

FLT8 fhgt_from_slr_elv          /* File: earth.c */
( FLT8 fslr_a, FLT8 felv_a, FLT8 fTow_a );

void FillDataConvert            /* File: data_types.c */
( volatile struct data_convert *pConvert_a,
  SINT4 iPrf_a,          /* PRF in Hertz */
  UINT2 iTriggerCase_a,  /* Type of dual PRF */
  SINT4 iWaveLength_a,   /* Wavelength in 1/100 of cm */
  UINT2 iPolarization_a, /* Polarization type */
  UINT4 iColorFlags_a    /* Flags containing units switches */
  );

FLT4 fNyquistVelocity           /* File: fnyquist_vel.c */
(SINT4 iprf_a,            /* PRF used (higher one in dual mode) */
 UINT2 iprfcase_a,        /* The multiple prf mode. See dsp_lib.h */
 SINT4 iwavelength_a,     /* Wavelength in 1/100 of cm */
 UINT2 iPolarization_a ); /* Polarization mode, see dsp_lib.h */

FLT4 fNyquistWidth           /* File: fnyquist_vel.c */
(SINT4 iprf_a,            /* PRF used (higher one in dual mode) */
 SINT4 iwavelength_a,     /* Wavelength in 1/100 of cm */
 UINT2 iPolarization_a ); /* Polarization mode, see dsp_lib.h */

  /* This function computes the low PRF given the high PRF and the 
   * dual PRF flag stored in IRIS products.
   */
  FLT4 fPrfLowFromHighCase   /* File: fnyquist_vel.c */
  ( SINT4 iHighPrf_a,
    UINT2 iPrfCase_a );      /* The multiple prf mode. See dsp_lib.h */
       
FLT8 fslr_from_ear_elv          /* File: earth.c */
  ( FLT8 fear_a, FLT8 felv_a );

FLT8 fslr_from_ear_hgt          /* File: earth.c */
  ( FLT8 fear_a, FLT8 fhgt_a, FLT8 fTow_a );

FLT8 fslr_from_hgt_elv          /* File: earth.c */
  ( FLT8 fhgt_a, FLT8 felv_a, FLT8 fTow_a );

char *get_established_name	/* File: error_report.c */
  ( void );

void get_process_name           /* File: process.c */
  (char *sname_a,
   SINT4 isize_a,      /* Size of name storage */
   int argc_a,
   char **argv_a );

void getbtime			/* File: timesubs.c */
  ( volatile UINT4* time );

#ifdef INCLUDE_SYSTIME_PROTOTYPES
UINT4 iGetBtimeTVal		/* File: timesubs.c */
  ( volatile struct timeval *pTVal_a ) ;
#endif /* #ifdef INCLUDE_SYSTIME_PROTOTYPES */

UINT4 btimeGrace		/* File: timesubs.c */
  ( UINT4 idiff_a ) ;

void GetTimeFull		/* File: timesubs.c */
  (volatile struct full_time_spec* pTime_a,
   int iFlags_a );

#ifdef INCLUDE_SYSTIME_PROTOTYPES
void TimeFullFromTimeval
  (volatile struct full_time_spec *pFullTime_a,
   const struct timeval *pTV_a,
   int iFlags_a );
#endif  /* #ifdef INCLUDE_SYSTIME_PROTOTYPES */

#define TIME_FLG_LOCAL (0)
#define TIME_FLG_UTC   (1)
#define TIME_FLG_GMT   (1)  /* Use "UTC" in new code */
void GetTimeYmds		/* File: timesubs.c */
  (volatile struct ymds_time* time,
   int iFlags_a );

BIN2  iBin2NarrowMidPt( BIN2 ia_a, BIN2 ib_a ) ;
BIN2 iBin2OrderedMidPt( BIN2 ia_a, BIN2 ib_a ) ;

SINT4 idata_alt_type		/* File: data_types.c */
  (SINT4 idata_a) ;

UINT4 idata_resolution          /* File: data_types.c */
  ( UINT4 idata_a );

UINT4 idatatype_from_index      /* File: data_types.c */
 (UINT4 index_a);

SINT2 iDataFromDspIndex         /* File: DataNames.c */
( UINT1 iIndex_a );

SINT2 iData2FromDspIndex        /* File: DataNames.c */
( UINT1 iIndex_a );

SINT2 iDataFromTaskIndex         /* File: DataNames.c */
( UINT1 iIndex_a );

SINT2 iDataFromName6            /* File: DataNames.c */
( const char *sDataName6_a );   /* Null terminated name */

#ifdef SIGMET_UF_H
UINT1 iDataFromUfName2          /* File: DataNames.c */
( const char *sName2_a,
  const struct uf_data_map Map_a[] );
#endif  /* #ifdef SIGMET_UF_H */

#define DATATYPE_IN_SETUP    1
#define DATATYPE_IN_OPTIONS  2
UINT4 iflag_from_datatype       /* File: data_types.c */
( UINT4 data_type );

void ignore_next_signal		/* In error_report.c */
( int signal_a ) ;

SINT4 ihhmmss			/* File: TimeNames.c */
( volatile const char* );

MESSAGE imapclose		/* In file mapio.c */
( const void* iaddr_a,
  SINT4 isize_a,
  SINT4 ichan_a );

MESSAGE imapcreate		/* In file mapio.c */
( const char* sname_a,
  SINT4 idessize_a,
  void** iaddr_a,
  SINT4* isize_a,
  SINT4* ichan_a );

MESSAGE imapopen		/* In file mapio.c */
( const char* sname_a,
  UINT2 lwrite_a,
  void** iaddr_a,
  SINT4* isize_a,
  SINT4* ichan_a );

MESSAGE imapopen_short		/* In file mapio.c */
( const char* sname_a,
  UINT2 lwrite_a,
  void** iaddr_a,
  SINT4* isize_a,
  SINT4* ichan_a,
  UINT4 imaxsize_a);

MESSAGE iMessageMod_va		/* File: error_report.c */
( MESSAGE iMessage_a, UINT4 icount_a, ... ) ;

SINT4 imonth_from_txt		/* File: TimeNames.c */
( volatile const char* stext_a );

/* Low level math functions on simple integers and bit patterns.
 */
UINT4 iMpyU64
( UINT4 iarg1_a, UINT4 iarg2_a, UINT4 *iLowProd_a ) ;

UINT4 iEuclidGCD( UINT4 iM_a, UINT4 iN_a ) ;
UINT4 iEuclidLCM( UINT4 iM_a, UINT4 iN_a ) ;

SINT4 iRationalDenom( FLT8 fArg_a, SINT4 iMaxDenom_a ) ;

SINT4 ilog_base_two( UINT4 ivalue_a ) ;
UINT4 ipower_of_two( UINT4 ivalue_a ) ;
SINT4  num_bits_set( UINT4 ivalue_a ) ;

/* Handy comparison functions of different data types for direct use
 * with the C-library QSORT() routine.
 */
int  iQsortCmpFLT4( const void *arg1_a, const void *arg2_a ) ;
int  iQsortCmpFLT8( const void *arg1_a, const void *arg2_a ) ;

int iQsortCmpSINT1( const void *arg1_a, const void *arg2_a ) ;
int iQsortCmpSINT2( const void *arg1_a, const void *arg2_a ) ;
int iQsortCmpSINT4( const void *arg1_a, const void *arg2_a ) ;

int iQsortCmpUINT1( const void *arg1_a, const void *arg2_a ) ;
int iQsortCmpUINT2( const void *arg1_a, const void *arg2_a ) ;
int iQsortCmpUINT4( const void *arg1_a, const void *arg2_a ) ;

/* Functions to find the Kth smallest element from an array of FLT4 or
 * SINT4 values.  Corresponding MEDIAN macros can also be used.
 */
FLT4  fKthSmallest(  FLT4 data_a[], SINT4 iSize_a, SINT4 kth_a ) ;
SINT4 iKthSmallest( SINT4 data_a[], SINT4 iSize_a, SINT4 kth_a ) ;

#define FMEDIAN( DATA, SIZE ) fKthSmallest( DATA, SIZE, ((SIZE)-1)/2 )
#define IMEDIAN( DATA, SIZE ) iKthSmallest( DATA, SIZE, ((SIZE)-1)/2 )


UINT4 index_from_datatype       /* In file data_types.c */
( UINT4 data_type ) ;

MESSAGE init_sem_set		/* File: semaphore.c */
( UINT4 isem_cluster_a ) ;

SINT4 iPackTime                 /* File: timesubs.c */
( SINT2 ihour_a,
  SINT2 iminute_a,
  SINT2 isecond_a ) ;

/* This function computes the IRIS dual PRF flag given the high PRF
 * and low PRFs being used.
 */
UINT2 iPrfCaseFromHighLow      /* File: fnyquist_vel.c */
( FLT4 fHighPrf_a,
  FLT4 fLowPrf_a );

UINT1 iProjectionFromName16     /* File: ProjectionNames.c */
( const char* sProjection16_a );   /* Projection type string */

UINT1 iProjectionFromProj4       /* File: ProjectionNames.c */
( const char* sProj4Name_a );   /* Projection type string */

SINT4 itrimlen			/* File: str_subs.c */
(volatile const char *string_a,	/* String to search */
 SINT4 ilength_a ) ;		/* Number of characters to search */

UINT2 iuser_to_data		/* File: data_types.c */
( FLT4  fuser,
  UINT4 idtype,
  const struct data_convert *pconvert_a );

SINT4 iYearFromFile		/* File: timesubs.c */
  (SINT4 iyear);

typedef SINT4 ( iYmdsCmpSub_f )
       ( volatile const struct ymds_time *time1_a,
	 volatile const struct ymds_time *time2_a ) ;

/* Compare/Subtract two YMDS time structures, ignoring milliseconds
 */
#define MAX_SUBTRACT_SECONDS ( 366 * 24 * 3600 )

iYmdsCmpSub_f iymds_compare ;	/* File: timesubs.c */
iYmdsCmpSub_f iymds_subtract ;	/* File: timesubs.c */

/* Compare two YMDS time structures, including milliseconds.
 */
#define MAX_SUBTRACT_MILLISECONDS ( 21 * 24 * 3600 * 1000 )

iYmdsCmpSub_f iYmdsCompareMs ;	/* File: timesubs.c */
iYmdsCmpSub_f iYmdsSubtractMs ;	/* File: timesubs.c */

SINT4 iymds_compare_serv	/* File: timesubs.c */
  (volatile const struct serv_ymds_time* time1,
   volatile const struct serv_ymds_time* time2 );

MESSAGE kill_sem_set		/* File: semaphore.c */
  ( UINT4 isem_cluster_a ) ;

void kilo_count_accum		/* File: KiloCount.c */
(volatile struct kilo_count *kcnt_a,
 SINT4 ival_a) ;

UINT2 lAutoRestartRequested	/* File: error_report.c */
  ( void ) ;

UINT1 lbin2_in_a_row( BIN2 ia_a, BIN2 ib_a, BIN2 ic_a ) ;

UINT2 liris_observer		/* File: username.c */
  ( const char *susername );

UINT2 liris_operator		/* File: username.c */
  ( const char *susername );

UINT2 c_lfind_command		/* File: c_lfind_command.c */
  (int icount_a,		/* Number of strings to compare against */
   char* string_vector_a[],	/* Array of strings to compare */
   char* scommand_a		/* String to check against the array */
   );

void lock_sem			/* File: semaphore.c */
  ( UINT4 num_a ) ;

UINT2 locked_by_me		/* File: semaphore.c */
  ( UINT4 num_a ) ;

UINT2 read_sem			/* File: semaphore.c */
  ( UINT4 num_a, UINT4 *ipid_a ) ;

  UINT2 lSemSetCheck              /* File: semaphore.c */
( UINT4 isem_cluster_a ); /* Cluster number parameter */

UINT2 lshmem_inuse		/* In shmem.c */
  (const char *segname_a,
   UINT2 lprint_a ); 

UINT2 lsig_process_exists	/* In process.c */
  (volatile const UINT4 ipid_a	/* Process ID */
   ) ;

UINT2 lsig_user_is_root(void) ;	/* In username.c */

void mapcache			/* In file mapio.c */
  ( UINT4 cachesz_a ) ;

void MapGetYmds			/* In file mapio.c */
  (struct ymds_time *pYmds_a,
   int iFlags_a );

void mapmmap			/* In file mapio.c */
  ( UINT2 luse_mmap_a ) ;

/* Simple array operations.  These are included to make the transition
 * from TI320 code a little easier, i.e., because the TI320 "int" and
 * "float" has a sizeof() one.  But they're handy for initializing any
 * non-byte array.
 */
void memset_f4( volatile  FLT4 f4Array_a[],  FLT4 f4Value_a, SINT4 iCount_a ) ;
void memset_f8( volatile  FLT8 f8Array_a[],  FLT8 f8Value_a, SINT4 iCount_a ) ;
void memset_s4( volatile SINT4 s4Array_a[], SINT4 s4Value_a, SINT4 iCount_a ) ;
void memset_s2( volatile SINT2 s2Array_a[], SINT2 s2Value_a, SINT4 iCount_a ) ;
void memset_u4( volatile UINT4 u4Array_a[], UINT4 u4Value_a, SINT4 iCount_a ) ;
void memset_u2( volatile UINT2 u2Array_a[], UINT2 u2Value_a, SINT4 iCount_a ) ;

  /* Translated message text will never exceed this size */
#define MESSAGE_SIZE (160)
char *message_to_text		/* File: error_report.c */
( MESSAGE istatus_a, UINT4 icount_a, const UINT4 iarg_list_a[] ) ;
char *message_to_text_r		/* File: error_report.c */
( MESSAGE istatus_a, UINT4 icount_a, const UINT4 iarg_list_a[],
  char sMessageRet_a[MESSAGE_SIZE] ) ;

#define MSG_TYPE_INFO      (1)	/* Informational, do not pop-up */
#define MSG_TYPE_NORMAL    (2)	/* Normal, pop-up */
#define MSG_TYPE_SAY       (3)	/* pop-up and speak */
#define MSG_TYPE_IGNORE    (4)	/* Ignore entirely */

#define MSG_FAULT_NONE     (1)	/* No fault implications */
#define MSG_FAULT_WARNING  (2)	/* Warning-level fault */
#define MSG_FAULT_CRITICAL (3)	/* Critical-level fault */

void mneg_2_to_1		/* File: byteops.c */
  ( const void* in_a, void* out_a, SINT4 icount_a );
void mneg_4_to_1		/* File: byteops.c */
  ( const void* input, void* output, SINT4 count );
void movb_2_to_1		/* File: byteops.c */
  ( const void* input, void* output, SINT4 count );
void movb_4_to_1		/* File: byteops.c */
  ( const void* input, void* output, SINT4 count );
void move_neg_words		/* File: byteops.c */
  ( const void* input, void* output, SINT4 count );

  void ServFromYmds             /* File: timesubs.c */
  (volatile struct serv_ymds_time *pYmdsOut_a,
   volatile const struct ymds_time *pYmdsIn_a );

/* Terminal I/O support
 */
struct ttyspec {		/* Specifications for opening a TTY */
  char sDevice[PATHNAME_SIZE] ;	/*   Pathname of device/FIFO file */
  SINT4 iBaud ;			/*   Baud rate of the interface */
  UINT1 iParity ;		/*   One of TTYPARITY_XXX */
  char  iIntrChar ;		/*   Interrupt character, if non-null */
  UINT1 lLockDevice ;		/*   Lock the device */
  UINT1 lPreserveAll ;		/*   Make no changes to the TTY settings */
  UINT1 lReadOnly, lWriteOnly ;	/*   We will only be doing reads/writes */
  char  iProtocol ;		/*   Serial protocol, one of TTYPROTO_XXX */
  char  pad171x1[1] ;
} ;

#ifdef INCLUDE_TERMIOS_PROTOTYPES
struct ttydev {			/* An open TTY device */
  UINT1 lOpenOkay ;		/*   The device is opened successfully */
  UINT1 lRealTTY ;		/*   Real TTY, versus FIFO pair */
  char pad2x2[2] ;
  struct ttyspec spec ;		/*   Settings used to open the device */
  FILE *Tx, *Rx ;		/*   Send/Receive stream pointers */
  int  fdOrig ;			/*   File descriptor for lock/attributes */
  struct termios tioOrig ;	/*   Original TTY settings */
#ifdef LINUX
  char pad245x3[3] ;
#endif /* #ifdef LINUX */
} ;
MESSAGE   openTTYDev( struct ttydev *tty_a, const struct ttyspec *spec_a ) ;
MESSAGE  closeTTYDev( struct ttydev *tty_a ) ;
void   drainTxTTYDev( const struct ttydev *tty_a ) ;
void   flushRxTTYDev( const struct ttydev *tty_a ) ;
UINT1 lGetInfoTTYDev( const struct ttydev *tty_a, struct ttyspec *spec_a ) ;

#endif /* #ifdef INCLUDE_TERMIOS_PROTOTYPES */

UINT1 lReadAvailFD( int iFD_a, SINT4 iTimeoutMS_a ) ;
UINT1 lWriteFreeFD( int iFD_a, SINT4 iTimeoutMS_a ) ;

/* Real time vector modeling package (RTVEC) definitions
 */
#define RTVECMAXDIM          16	/* Maximum vector dimension */
#define RTVECMAXSLOTS       500	/* Maximum number of history slots */
#define RTVECMAXTIMESPAN  60000	/* Maximum time span within the model */
struct rtvec_dhdr {		/* Data vector header */
  UINT4 iBtime ;		/*   Binary time of this data point */
  UINT1 lValidData ;		/*   This data point is valid */
  char  pad5x3[3] ;
} ;
struct rtvec_info {		/* Information/Control structure */
  UINT4 iSlotWidthMS ;		/*   Slot width in milliseconds */
  SINT4 nSlots ;		/*   Number of data history slots */
  SINT4 iVecDim ;		/*   Dimension of data vectors */
  SINT4 iTopSlot ;		/*   Index of most recent slot */
  UINT1 iModulos[RTVECMAXDIM] ;	/*   Modulous interval for the data */
#define RTVEC_MOD_NONE   0	/*     None (linear floating values) */
#define RTVEC_MOD_UBIN   1	/*     UnSigned binary angle (0,+360) */
#define RTVEC_MOD_SBIN   2	/*     Signed binary angle (-180,+180) */
#define RTVEC_MOD_EBIN   3	/*     Elevation binary angle (-90,+270) */
} ;
#define RTVECALLOCATE( NSLOTS, VECDIM ) struct {		\
  struct rtvec_info info ;					\
  struct { struct rtvec_dhdr dhdr ; FLT8 fData[ VECDIM ] ; }	\
    slots[ NSLOTS ] ; }

struct rtvec {			/* Per-process structure for model access */
  struct rtvec_info *info ;	/*   ==> The (public) model area */
  void *pLock, *pUnLock ;	/*   Optional Lock/Unlock routines */
} ;

void rtvecInitFirst		/* First time global inits */
( volatile struct rtvec_info *info_a, SINT4 nSlots_a, SINT4 iVecDim_a ) ;
void rtvecAttach		/* Per-process inits */
( struct rtvec *rtvec_a, volatile struct rtvec_info *info_a,
  void (*pLock_a)(void), void (*pUnLock_a)(void) ) ;
void rtvecInsertPoint
( struct rtvec *rtvec_a, UINT4 iBtime_a, const FLT8 fData_a[] ) ;
void rtvecJumpData
( struct rtvec *rtvec_a, const FLT8 fData_a[] );

#define RTVEC_LOOKUP_EMPTY        (0x0001)
#define RTVEC_LOOKUP_EXTRAPOLATE  (0x0002)
#define RTVEC_LOOKUP_SHORT        (0x0004)
UINT4 rtvecLookupPoint
( struct rtvec *rtvec_a, UINT4 iBtime_a, FLT8 fData_a[] ) ;

/* Message Queue routines
 */
MESSAGE msgQueCreate(UINT4 *imsgqid_a, const char *sname_a);
MESSAGE msgQueCreateFind(UINT4 *imsgqid_a, const char *sname_a);
MESSAGE msgQueFind(UINT4 *imsgqid_a, const char *sname_a);
MESSAGE msgQueSend(UINT4 imsgqid_a, const char *send_str);
MESSAGE msgQueRecv(UINT4 imsgqid_a, char **recv_str);
MESSAGE msgQueRemove(UINT4 imsgqid_a);
MESSAGE msgQueGetData(UINT4 imsgqid_a, UINT2 *inum_msgs_on_que_a);

char* nullify			/* File: str_subs.c */
  (volatile const char* input,
   volatile const SINT4 isize );

char* nullify_ncpy		/* File str_subs.c */
  (volatile char* output_a,
   volatile const char* input_a,
   const SINT4 isize_a );

#define SIGNAL_KILL_ON     (0x01)
#define SIGNAL_KILL_OFF    (0x00)
#define SIGNAL_RESTART_ON  (0x02)
#define SIGNAL_RESTART_OFF (0x00)
#define SIGNAL_MESSAGE_ON  (0x04)
#define SIGNAL_MESSAGE_OFF (0x00)
  void OverrideSignalResponse     /* File: error_report.c */
  (int iSignal_a,
   int iResponse_a );

  void PolarFromEarth              /* File: sphere.c */
  (volatile const struct latlon8 *Earth1_a,
   volatile const struct latlon8 *Earth2_a,
   volatile struct D2Polar8 *pPolar_a );
  
  /* Given a starting point on the earth and a range and direction,
   * return the ending point.
   */
  void polar_to_earth           /* File: sphere.c */
  (volatile const struct latlon8 *Start_a,
   FLT8 frange_a,
   FLT8 fangle_a,
   volatile struct latlon8 *End_a) ;

void PrintFromString            /* File: str_subs.c */
( char *sPrintableString_a,    /* Printable String output */
  const char *sRawString_a,    /* Raw String input */
  SINT4 iOutLength_a );	       /* Length of the output string */

UINT4 rain_file_to_long		/* File: data_types.c */
  (UINT2 iout );

void rain_long_to_file		/* File: data_types.c */
  (const UINT4 *input,
   SINT4 icount,
   UINT2 *iout );

UINT2 iPackIQFromFloatIQ_( FLT4 fIQVal_a, UINT4 iFlags_a ) ;
FLT4  fFloatIQFromPackIQ_( UINT2 iCode_a, UINT4 iFlags_a ) ;

void vecPackIQFromFloatIQ_
( volatile UINT2 iCodes_a[], volatile const FLT4 fIQVals_a[],
  SINT4 iCount_a, UINT4 iFlags_a ) ;
void vecFloatIQFromPackIQ_
( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[],
  SINT4 iCount_a, UINT4 iFlags_a ) ;
void vecFloatIQFromPackIQ_comp
( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[],
  SINT4 iCount_a, UINT4 iFlags_a ) ;

#define PACKIQ_HIGHSNR   0x0001	/* Use High-SNR packed format */
#define PACKIQ_BYTESWAP  0x0002	/* UINT2 data are byte-swapped */

void schedule_cbf               /* File: schedule.c */
( void (*callback_a)(int),
 SINT4 imilli_a );

void schedule_ef		/* File: schedule.c */
  (UINT4 ief_a,
   SINT4 imilli_a );

char* scan_name10               /* File: DataNames.c */
  ( UINT2 iscan_a );

char* sDataName3		/* File: DataNames.c */
  ( UINT4 idata_a );

char* sdata_name6		/* File: DataNames.c */
  ( UINT4 idata_a );

char* sdata_name6_ns            /* File: DataNames.c */
  ( UINT4 idtype_a );

char* sdata_name15		/* File: DataNames.c */
  ( UINT4 idata_a );

char* sdata_units6		/* File: DataNames.c */
  ( UINT2 idata_a,
    UINT4 iFlags_a );

char* sDataSpan12               /* File: DataNames.c */
( UINT2 idata_a,
  UINT4 iFlags_a );

#ifdef SIGMET_UF_H
char *sUfName2FromData          /* File: DataNames.c */
( char sText_a[3],                    /* Uf string stored here */
  UINT1 idata_a,
  const struct uf_data_map Map_a[] ); /* Optional mapping table here */
#endif  /* #ifdef SIGMET_UF_H */

const char *sMonth3( SINT4 iMonth_a ) ;

char* sddmonyyyy_r		/* File: TimeNames.c */
( SINT4 iyear_a,
  SINT4 imonth_a,
  SINT4 iday_a,
  char sBuffer_a[12] );

void set_say_on_error           /* File: error_report.c */
( UINT1 lsay_it_a );

MESSAGE SetTimeYmds		/* File: timesubs.c */
(volatile const struct ymds_time *pYmds_a);

char* sfilelist			/* File: fileops.c */
(const char *dir_a, const char* file_a, UINT2 lforward_a ) ;

#define TIMENAME_SIZE (25)
char* shhmmssmmm_ddmonyyyy      /* File: TimeNames.c */
( volatile const struct ymds_time *ymds_a,
  char sBuffer_a[TIMENAME_SIZE] );

char* shhmmssmmm_ddmonyyyy_serv /* File: TimeNames.c */
( volatile const struct serv_ymds_time *ymds_a,
  char sBuffer_a[TIMENAME_SIZE] );

char* shhmmssddmonyyyy_r	/* File: TimeNames.c */
(volatile const struct ymds_time *ymds,
 char sBuffer_a[TIMENAME_SIZE] );

char* shhmmssddmonyyyy_serv_r	/* File: TimeNames.c */
(volatile const struct serv_ymds_time *ymds,
 char sBuffer_a[TIMENAME_SIZE] );

char* shhmmddmonyyyy_r		/* File: TimeNames.c */
(volatile const struct ymds_time *ymds,
 char sBuffer_a[18] );

char* shhmmddmonyyyy_serv_r	/* File: TimeNames.c */
(volatile const struct serv_ymds_time *ymds,
 char sBuffer_a[18] );

char* shhmmssmmm_r		/* File: TimeNames.c */
( const SINT4 isec_a,
  UINT2 iMills_a,
  char sBuffer_a[13] );  /* Result is 12 chars + null */

char* shhmmss_r			/* File: TimeNames.c */
( const SINT4 isecs,
 char sBuffer_a[9] );  /* Result is 8 chars + null */

char* shhmm_r			/* File: TimeNames.c */
( const SINT4 isecs,
 char sBuffer_a[6] );  /* Result is 5 chars + null */

void shmsdmy_to_ymds		/* File: TimeNames.c */
  (volatile const char*,
   volatile struct ymds_time *ymds );

MESSAGE sig_copy_file		/* File: fileops.c */
  (const char *sdest_path_a,
   const char *source_path_a,
   UINT2 okay_to_clobber_a,
   UINT2 (*lstop_e)(void) ) ;

MESSAGE sig_copy_partial_file   /* File: fileops.c */
  (const char *sdest_path_a,
   const char *source_path_a,
   UINT2 okay_to_clobber_a,
   UINT4 isize_a );

UINT2 sig_clref			/* File: semaphore.c */
  ( UINT4 num_a ) ;

UINT2 sig_readef		/* File: semaphore.c */
  ( UINT4 num_a ) ;

UINT2 sig_setef			/* File: semaphore.c */
  ( UINT4 num_a ) ;

void sig_waitef			/* File: semaphore.c */
  ( UINT4 num_a ) ;

#ifdef INCLUDE_UNISTD_PROTOTYPES
int sig_system_int
(const char *command_a,           /* Shell command to run */
 void (*watchdog_a)(pid_t iPid_a) );   /* Function which can interrupt the call*/
#endif  /* #ifdef INCLUDE_UNISTD_PROTOTYPES */

MESSAGE sig_closedir		/* File: fileops.c */
  ( void );

/* This clears initial state as needed for a daemon process
 */
MESSAGE sig_daemon_init(void);  /* File: process.c */

MESSAGE sig_delete_file		/* File: fileops.c */
  (const char *spath_a );

MESSAGE sig_delete_file_wild	/* File: fileops.c */
  (const char *sdirectory_a,
   const char *smatch_a );

void sig_establish		/* File: error_report.c */
  (MESSAGE (*handler_a)(MESSAGE istatus, UINT4 icount, const UINT4 iargs[] ),
   const char *sprocess_a );
void sig_establish_opt
( MESSAGE (*handler_a)(MESSAGE istatus, UINT4 icount, const UINT4 iargs[] ),
  const char *sprocess_a, UINT1 lTrapSignals_a ) ;

MESSAGE sig_free		/* In sig_subs.c */
  (void* addr_a);

UINT2 sig_fnmatch		/* File: fileops.c */
  ( const char *wild_a, const char *name_a ) ;

MESSAGE sig_ftok		/* In shmem.c */
  (const char *sname_a, SINT4 *key_a ) ;

int sig_fork_execv              /* In process.c */
(const char *spath_a,         /* Path of file to execute */
 char * const sargv_a[],      /* Vector of arguments to process */
 MESSAGE *istatus_a );        /* Error status returned here */

int sig_fork_execv_pipe  /* In process.c */
( const char *spath_a,       /* Program to execv */
  char * const sargv_a[],    /* Args to execv */
  int *pwrite_pipe_a,        /* Write end of pipe returned here, NULL if not wanted */
  int *pread_pipe_a,         /* Read end of pipe returned here, NULL if not wanted */
  MESSAGE *istatus_a);       /* Error return info */

MESSAGE sig_fork_process      /* In process.c */
  (volatile UINT4 *ipid_a,    /* Process ID returned */
   const char *spath_a,	      /* Path of file to execute */
   const char *sname_a	      /* Process name, (supplied as second arg on UNIX platforms) */
   ) ;

MESSAGE sig_fork_daemon		/* Create a daemon (Parent = INIT) process */
( const char *spath_a,		/* Path of file to execute */
  const char *sname_a ) ;	/* Process name */

#ifdef INCLUDE_STDIO_PROTOTYPES
MESSAGE sig_init_serial         /* In sig_subs.c */
  (const char *sdevice_a,
   FILE **pstream_a,
   UINT2 lwrite_a      /* Set to true if you are going to write, else false*/
   );
#endif  /* #ifdef INCLUDE_STDIO_PROTOTYPES */

MESSAGE sig_kill_process	/* In process.c */
  (UINT4 ipid_a			/* Process ID to kill */
   ) ;

void sig_microSleep		/* File: schedule.c */
( SINT4 iMicroSeconds_a ) ;

MESSAGE sig_malloc		/* In sig_subs.c */
  (const UINT4 size_a,
   void**   addr_a) ;

MESSAGE sig_opendir		/* File: fileops.c */
  (const char *sdirectory_a,
   const char *smatch_a );

/* Returns an educated guess of the largest file descriptor which
 * can be open.
 */
int sig_open_max(void);		/* File: process.c */

UINT2 sig_os_byteswap(void) ;	/* sig_subs.c */
int   sig_rand(void) ;		/* sig_subs.c */

void sig_periodicSleep		/* File: schedule.c */
( SINT4 iMicroSeconds_a, FLT8 *fLastWakeupSec_a ) ;

MESSAGE sig_readdir		/* File: fileops.c */
  (char *spath_a,
   UINT4 path_len_a,
   UINT4* iname_a,
   UINT2* lvalid_a );

MESSAGE sig_run_as_operator(void) ;	/* In process.c */
MESSAGE sig_run_as_real    (void) ;    	/* In process.c */
MESSAGE sig_run_as_root    (void) ;	/* In process.c */

MESSAGE sig_setpriority( SINT4 ) ; /* In process.c */

MESSAGE sig_shmem_map		/* In shmem.c */
 (UINT4   dessize_a,
  const char* sname_a,
  UINT2   lwrite_a,
  void**  actaddr_a,
  SINT4*  actsize_a,
  SINT4*  iden_a,
  UINT2*  lcreated_a) ;

MESSAGE sig_shmem_unmap		/* In shmem.c */
  (volatile void*   addr_a,
   SINT4   size_a,
   SINT4   iden_a ) ;

void sig_signal                /* In error_report.c */
  ( const MESSAGE imessage ); 

/* Flag bits in icount_a below
 */
#define SIG_IGNORE_MESSAGE (0x00020000)
#define SIG_SAY_MESSAGE    (0x00040000)
#define SIG_INFO_MESSAGE   (0x00080000)
void sig_signal_va             /* In error_report.c */
  ( MESSAGE imessage,
    UINT4 icount_a, ...) ;

#ifdef INCLUDE_SYSUTSNAME_PROTOTYPES
MESSAGE sig_uname		/* File: sig_subs.c */
( struct utsname *pName_a ) ;
#endif /* INCLUDE_SYSUTSNAME_PROTOTYPES */

const char *sig_username	/* In username.c */
( void ) ;

char *sigtok                    /* File: str_subs.c */
( char *sline_a, 
  UINT4 *ioffset_a );		/* Modifies offset each time. */

  /* These are the 4 possible states which the socket connection can
   * cycle between.  SocketConnectNoBlock will signal when state
   * transitions are made.  Be sure to pass in the previous state.
   * For the first call, a good choice is to set it to OK.
   */
#define SIG_CONNECT_OK       (0)  /* Connected fine */
#define SIG_CONNECT_NOSERVER (1)  /* Host is reachable, but the server is off */
#define SIG_CONNECT_NOHOST   (2)  /* Host is not reachable */
#define SIG_CONNECT_ERROR    (3)  /* Error in socket call */
  int SocketConnectNoBlock      /* In SocketConnect.c  */
( int iFamily_a,                /* Address family to use */
  const char *sHost_a,          /* Host name to connect to */
  UINT2       iPort_a,          /* Port number to connect to */
  void (*IoHandler)(int),       /* Handler for sigio signals */
  volatile UINT4 *istat_a );

  void SocketDisconnect         /* File: SocketConnect.c */
( int iSocket_a );

#ifdef INCLUDE_SYSSOCKET_PROTOTYPES
  /* Function called each time the NoBlock listener wakes up.  If it returns
   * nonzero the listener will continue.
   */
  typedef int (*ContinueTestProc)( void );
  /* Function called each time an accept is called.  If it returns
   * a negative number, then the connection is rejected, and the
   * abs of the number is signaled as the max connection count.
   */
  typedef int (*ConnectionTestProc)( const struct sockaddr_in *cliaddr_a );

  typedef void (*ParentHandlerProc)( pid_t iChildPid_a, int iConnectNumber_a );

  typedef void (*ConnectionHandlerProc)
       ( int iSocket_a, const struct sockaddr_in *cliaddr_a, int iConnectNumber_a );

  void SocketListenBlock        /* File: SocketSupport.c */
  (UINT2 iPort_a,		/* Port number to listen on */
   ConnectionTestProc    ConnectionTester_a,
   ConnectionHandlerProc ConnectionHandler_a,
   ParentHandlerProc     ParentHandler_a
   );

  void SocketListenNoBlock	/* File: SocketSupport.c */
  (UINT2 iPort_a,		/* Port number to listen on */
   UINT4 iEventFlag_a,		/* Event flag to block on */
   ContinueTestProc      lContinueTester_a,  /* Function to detect if we should return */
   ConnectionTestProc    ConnectionTester_a,
   ConnectionHandlerProc ConnectionHandler_a,
   ParentHandlerProc     ParentHandler_a
   );
#endif  /* #ifdef INCLUDE_SYSSOCKET_PROTOTYPES */


  MESSAGE CheckSocketAck   /* File: SocketIo.c */
  ( const char *sBuffer_a,
    SINT4 iSize_a );

  char *ReadSocketPacketTcp   /* File: SocketIo.c */
  ( int iSocket_a,
    SINT4 *pBufferSize_a,
    UINT1 lVerbose_a,
    SINT4 iTimeoutMS_a,     /* Timeout in milliseconds, 0=no timeout */
    UINT1 *pTimedOut_a      /* Flag to indicate that timeout happened */
    );

  char *ReadSocketPacketUdp   /* File: SocketIo.c */
  ( int iSocket_a,
    SINT4 *pBufferSize_a,
    UINT1 lVerbose_a,
    UINT4 *pFromAddress_a,  /* From address returned in network byte order */
    SINT4 iTimeoutMS_a,     /* Timeout in milliseconds, 0=no timeout */
    UINT1 *pTimedOut_a      /* Flag to indicate that timeout happened */
    );

  MESSAGE ReadSocketAck       /* File: SocketIo.c */
  ( int iSocket_a,
    UINT1 lVerbose_a,
    SINT4 iTimeoutMS_a );     /* Timeout in milliseconds, 0=no timeout */
  
  MESSAGE WriteSocketData1   /* File: SocketIo.c */
  (int iSocket_a,
   const char *pData_a,
   SINT4 iSize_a,
   UINT1 lVerbose_a );
  
  MESSAGE WriteSocketString   /* File: SocketIo.c */
  (int iSocket_a,
   const char *sString_a,
   UINT1 lVerbose_a );
  
  MESSAGE WriteSocketData2    /* File: SocketIo.c */
  (int iSocket_a,
   const char *pData1_a,
   SINT4 iSize1_a,
   const char *pData2_a,
   SINT4 iSize2_a,
   UINT1 lVerbose_a );

#define UDP_FLG_MULTICAST (0x0001)  /* Use multicast addresses */
#define UDP_FLG_UNICAST   (0x0002)  /* Do not allow broadcast addresses */

/* Open a socket for UDP reading
 */
#ifdef INCLUDE_NETINET_IN_PROTOTYPES
int OpenUdpReader
(UINT2 iPort_a,            /* Port number to read from */
 const char *sAddress_a,   /* Address to read from (can be 0.0.0.0 for any) */
 const char *sInterface_a, /* Interface name needed for multicasting only */
 struct in_addr *pAddress_a,/* Returns the local address for those who might want it */  
 int   iFlags_a
 );
#endif  /* #ifdef INCLUDE_NETINET_IN_PROTOTYPES */
/* Open a socket for UDP writing
 */
int OpenUdpWriter
(UINT2 iPort_a,             /* Port number to write to */
 const char *sAddress_a,    /* Address to write to */
 const char *sInterface_a,  /* Interface name needed for multicasting only */
 int   iFlags_a
 );

#ifdef INCLUDE_STDIO_PROTOTYPES
struct sig_logging_spec
{
  UINT1 lLogToFile;
  UINT1 lLogToTerm;
  UINT1 lVerbose;		/* Log more details */
  UINT1 lAppendToLog;		/* Append to previous log file */
  FILE *iLogFile;		/* Open file descriptor to log file */
};

void LoggingPrintf		/* File: SocketSupport.c */
( const struct sig_logging_spec *pLogging_a, const char *sString_a );

#endif  /* #ifdef INCLUDE_STDIO_PROTOTYPES */

void spacify_ncpy		/* File: str_subs.c */
( volatile char* output,
  volatile const char* input,
  const SINT4 isize );

const char* sProjectionName16     /* File: ProjectionNames.c */
( UINT1 iProjectionType_a );

const char* sProjectionName3      /* File: ProjectionNames.c */
( UINT1 iprojection_type_a );

const char* sProjectionName32     /* File: ProjectionNames.c */
( UINT1 iProjectionType_a );

const char *sProj4Name            /* File: ProjectionNames.c */
(UINT1 iProjectionType_a);

/* Tell me if the projection uses the reference latitude. */
UINT1 lProjectionHasRefLat        /* File: ProjectionNames.c */
(UINT1 iProjectionType_a);

/* Tell me if the projection uses standard parallels */
UINT1 lProjectionHasStdPar        /* File: ProjectionNames.c */
(UINT1 iProjectionType_a);

/* Get name and value for one of the standard geodes.
 * Can produce up to IMAP_EARTH_GEODE_COUNT different values. */
  FLT8 fGetStandardGeode   /* Returns radius here */ /* File: ProjectionNames.c */
(char sName_a[16],       /* Returns name here */
 FLT8 *fFlattening_a,    /* Returns flattening here */
 UINT4 iIndex_a);

void StringFromPrint              /* File: str_subs.c */
( char *sRawString_a,              /* Raw output strint to fill in */
  const char *sPrintableString_a,  /* Printable input string */
  SINT4 iOutLength_a );		   /* Length of the output string */

  void strcpy_filesafe           /* File: strsubs.c */
  (char* sOutput_a,
   const char* sInput_a );
void strncpy_filter
  ( char* output_a, const char* input_a, SINT4 isize_a );
void strncpy_safe
  ( char* output_a, const char* input_a, SINT4 isize_a );
void strnpad
  ( char* output_a, const char* input_a, SINT4 isize_a );

  SINT2 iSwapS2(SINT2 iWord_a); /* File: swap.c */

  void SwapS2(SINT2 *pWord_a);
  void SwapU2(UINT2 *pWord_a);

  void SwapF4(FLT4  *pWord_a);
  void SwapS4(SINT4 *pWord_a);
  void SwapU4(UINT4 *pWord_a);

/* Copy and swap an array
 */
void SwapS2cpy
( SINT2 *pOut_a, const SINT2 *pIn_a, SINT4 iCount_a  ) ;

void SwapS2Array( SINT2 *pOut_a, SINT4 iCount_a ) ;
void SwapU2Array( UINT2 *pOut_a, SINT4 iCount_a ) ;

void TZNameBuild
( char sTZNameOut_a[TZNAME_SIZE],
  const char *sTZNameIn_a,	/* Local TZ name */
  SINT4 iMinutesWest_a,		/* Recorded TZ offset */
  UINT2 iFlags_a );             /* Flags indicating the type of TZ recording */

UINT4 iunique_sum		/* File: byteops.c */
( const void *array_a, SINT4 count_a ) ;

MESSAGE uncompress_cowords	/* File: compress.c */
 (void get_a( SINT2 *buf_a, SINT4 cnt_a), /* Input coroutine */
  SINT4 insize_a,               /* Maximum # usable words from input buffer */
  SINT4 *inlen_a,		/* Number of words read from INBUF */
  SINT2 ioutbuf_a[],		/* Output array to hold UnCompressed data */
  SINT4 ioutsize_a,             /* Maximum # usable words from output buffer */
  SINT4 *ioutlen_a) ;		/* Number of words written to IOUTBUF */

MESSAGE uncompress_cowords_skip	/* File: compress.c */
 (void get_a( SINT2 *buf_a, SINT4 cnt_a), /* Input coroutine */
  SINT4 insize_a,               /* Maximum # usable words from input buffer */
  SINT4 *inlen_a,		/* Number of words read from INBUF */
  SINT4 *ioutlen_a) ;		/* Number of words written to IOUTBUF */

MESSAGE uncompress_words	/* File: compress.c */
 (SINT2 inbuf_a[],		/* Input array of data to be uncompressed */
  SINT4 insize_a,               /* Maximum # usable words from input buffer */
  SINT4 *inlen_a,		/* Number of words read from INBUF */
  SINT2 ioutbuf_a[],		/* Output array to hold UnCompressed data */
  SINT4 ioutsize_a,             /* Maximum # usable words from output buffer */
  SINT4 *ioutlen_a) ;		/* Number of words written to IOUTBUF */

MESSAGE uncompress_words_skip	/* File: compress.c */
 (SINT2 inbuf_a[],		/* Input array of compressed data */
  SINT4 insize_a,               /* Maximum # usable words from input buffer */
  SINT4 *inlen_a,		/* Number of words read from INBUF */
  SINT4 *ioutlen_a) ;		/* Number of words of output that would have
				   resulted, had the data been UnCompressed. */

void unlevelize_4bit		/* In file unlevelize.c */
( const UINT1* pix_a,		/* Array of pixels */
  UINT4 pnum_a,			/* Number of pixels in PIX_A (may be odd or even) */
  const void* pmap_a,  		/* 16-element mapping table of bytes, words, or longs */
  UINT4 bwl_a,			/* 8, 16, or 32, conveying size of items in OUTPUT_A. */
  void* output_a		/* Resulting array of data */
  );

void unlevelize_8bit		/* In file unlevelize.c */
( const UINT1* byte_a,		/* Array of bytes */
  UINT4 pnum_a,			/* Number of pixels in PIX_A (may be odd or even) */
  const UINT4 pmap_a[256],     /* 256-element mapping table */
  UINT4 bwl_a,			/* 8, 16, or 32, conveying size of items in OUTPUT_A. */
  void* output_a		/* Resulting array of data */
  );

void unlock_sem			/* File: semaphore.c */
( UINT4 num_a ) ;

void unpacktime			/* File timesubs.c */
(UINT4           isec_a, 
 volatile UINT2* idays,
 volatile UINT2* ihours,
 volatile UINT2* iminutes,
 volatile UINT2* iseconds
 );

/* Convert SIGMET ymds time structure to UTC.  It may already be so.
 */
void UtcFromYmds               /* File: timesubs.c */
( struct ymds_time *pYmdsUtcOut_a,
  const struct ymds_time *pYmdsIn_a,
  SINT4 iMinutesWest_a );          /* Time offset to standard time */

#define HELP_MENU      0
#define HELP_CONTEXT   1
#define HELP_INDEX     2

#define MANUALS_IRIS    0
#define MANUALS_UTIL    1
#define MANUALS_PROG    2
#define MANUALS_INST    3
#define MANUALS_NOTE    4
#define MANUALS_RCP02   5
#define MANUALS_RVP7    6
#define MANUALS_RVP8    7
#define MANUALS_RCP8    8
#define MANUALS_IRISRAD 9
#define MANUALS_EXTRA   10

#define MANFILE_IRISRAD_RUNNING "2running.pdf"
#define MANFILE_IRISRAD_RST     "4radstat.pdf"
#define MANFILE_IRISRAD_MESSAGE "5errors.pdf"
#define MANFILE_IRISRAD_TCF     "6tskconf.pdf"
#define MANFILE_IRISRAD_TSC     "7tskschd.pdf"

#define MANFILE_IRIS_DPYOPT  "6prdout.pdf"
#define MANFILE_IRIS_ISM     "8ingest.pdf"
#define MANFILE_IRIS_OVR     "9overly.pdf"
#define MANFILE_IRIS_POM     "6prdout.pdf"
#define MANFILE_IRIS_PSC     "4prodsch.pdf"
#define MANFILE_IRIS_TAP     "7archiv.pdf"
#define MANFILE_IRIS_INTRO   "1intro.pdf"

#define MANFILE_IRIS_PCF_BASE    "22base.pdf"
#define MANFILE_IRIS_PCF_BEAM    "23beam.pdf"
#define MANFILE_IRIS_PCF_CAPPI   "24cappi.pdf"
#define MANFILE_IRIS_PCF_CATCH   "31catch.pdf"
#define MANFILE_IRIS_PCF_COMP    "32comp.pdf"
#define MANFILE_IRIS_PCF_DWELL   "33dwell.pdf"
#define MANFILE_IRIS_PCF_FCAST   "25fcast.pdf"
#define MANFILE_IRIS_PCF_HMAX    "26hmax.pdf"
#define MANFILE_IRIS_PCF_MAX     "27max.pdf"
#define MANFILE_IRIS_PCF_NDOP    "34ndop.pdf"
#define MANFILE_IRIS_PCF_PPI     "28ppi.pdf"
#define MANFILE_IRIS_PCF_RAIN1   "29rain1.pdf"
#define MANFILE_IRIS_PCF_RAINN   "210rainn.pdf"
#define MANFILE_IRIS_PCF_RAW     "211raw.pdf"
#define MANFILE_IRIS_PCF_RHI     "212rhi.pdf"
#define MANFILE_IRIS_PCF_RTI     "213rti.pdf"
#define MANFILE_IRIS_PCF_SHEAR   "35shear.pdf"
#define MANFILE_IRIS_PCF_SLINE   "36sline.pdf"
#define MANFILE_IRIS_PCF_SRI     "214sri.pdf"
#define MANFILE_IRIS_PCF_TOPS    "216tops.pdf"
#define MANFILE_IRIS_PCF_TRACK   "217track.pdf"
#define MANFILE_IRIS_PCF_VIL     "218vil.pdf"
#define MANFILE_IRIS_PCF_VVP     "219vvp.pdf"
#define MANFILE_IRIS_PCF_WARN    "220warn.pdf"
#define MANFILE_IRIS_PCF_WIND    "221wind.pdf"
#define MANFILE_IRIS_PCF_XSECT   "222xsect.pdf"

#define MANFILE_WIN_ANIM        "53qlanim.pdf" 
#define MANFILE_WIN_APPLSHELL   "5qlw.pdf" 
#define MANFILE_WIN_CATCH       "31catch.pdf"
#define MANFILE_WIN_CURS        "55qlcurs.pdf" 
#define MANFILE_WIN_DPYOPT      "52qldisp.pdf" 
#define MANFILE_WIN_DPYOPT_VVP  "59qlopt.pdf" 
#define MANFILE_WIN_DPYOPT_WND  "59qlopt.pdf" 
#define MANFILE_WIN_DPYOPT_COLR "51qlcol.pdf" 
#define MANFILE_WIN_EXPORT      "510qlexp.pdf" 
#define MANFILE_WIN_FCAST       "57qlforc.pdf"
#define MANFILE_WIN_MULTI       "5qlw.pdf" 
#define MANFILE_WIN_LIVE        "5qlw.pdf"
#define MANFILE_WIN_LONLAT      "5qlw.pdf" 
#define MANFILE_WIN_SLIDE       "54qlslid.pdf" 
#define MANFILE_WIN_TRACK       "56qltrak.pdf" 
#define MANFILE_WIN_XSECT       "58qlxsec.pdf"
#define MANFILE_WIN_SLIDE       "54qlslid.pdf"

  /* parameters for utility programs are listed here */
#define MANFILE_UTIL_ANTENNA          "2antenna.pdf"
#define MANFILE_UTIL_ASCOPE           "3ascope.pdf"
#define MANFILE_UTIL_BITEX            "4bitex.pdf"
#define MANFILE_UTIL_COLOR_SPECIAL    "5colors.pdf"
#define MANFILE_UTIL_COLOR_APPLSHELL  "5colors.pdf"
#define MANFILE_UTIL_COLOR_SET        "5colors.pdf"
#define MANFILE_IRISRAD_IRISNET       "3irisnet.pdf"
#define MANFILE_UTIL_PVIEW            "Unknown.pdf"
#define MANFILE_IRIS_RIB_SETUP        "etdwr.pdf"
#define MANFILE_UTIL_RTDISP           "8rtdisp.pdf"
#define MANFILE_IRIS_RUNWAYS          "etdwr.pdf"
#define MANFILE_SETUP_INTRO           "9setup.pdf"
#define MANFILE_SETUP_DSP             "92rvp.pdf"
#define MANFILE_SETUP_RCP             "93rcp.pdf"
#define MANFILE_SETUP_INPUT           "94in.pdf"
#define MANFILE_SETUP_IRIS            "95gen.pdf"
#define MANFILE_SETUP_LICENSE         "96lic.pdf"
#define MANFILE_SETUP_INGEST          "97ing.pdf"
#define MANFILE_SETUP_PRODUCT         "98pro.pdf"
#define MANFILE_SETUP_OUTPUT          "99out.pdf"
#define MANFILE_SETUP_WEB             "910web.pdf"
#define MANFILE_UTIL_UTILS            "1intro.pdf"
#define MANFILE_IRISRAD_SIGAUDIO      "2running.pdf"
#define MANFILE_INST_SIGPRINTER       "cprinter.pdf"
#define MANFILE_IRIS_VIR_RIB          "etdwr.pdf"
#define MANFILE_UTIL_ZAUTO7           "12zauto7.pdf"

#define MANFILE_INST_SIGBRU           "dsigbru.pdf"

#define MANFILE_RVP8_TSARCH           "ftsarch.pdf"

void helpStart(int imanual_a, const char *sfilename_a, int ioption_type_a);
void helpExit(void);


  /* Functions used to read/write VAX formats.  These deal with
   * byte swapping, byte alignment, and the floating point format.
   */
UINT2 GetVaxUint2( const UINT1 p[]);
  UINT4 GetVaxUint4( const UINT1 p[]);
FLT4 GetVaxFlt4( const UINT1 p[]);
void PutVaxUint2( UINT1 p[], UINT2 value);
void PutVaxUint4( UINT1 p[], UINT4 value);
void PutVaxFlt4( UINT1 p[], FLT4 number);

void YmdsAddMs                  /* In file: timesubs.c */
( SINT4 iMillSec_a, 
  volatile struct ymds_time *ymds_a );

void YmdsFromServ               /* In file: timesubs.c */
  (volatile struct ymds_time *pYmdsOut_a,
   volatile const struct serv_ymds_time *pYmdsIn_a );

#ifdef INCLUDE_SYSTIME_PROTOTYPES
  void YmdsFromTime_t            /* In file: timesubs.c */
  (struct ymds_time *pYmds_a,
   time_t iTime_a,
   UINT2  iMilliseconds_a, 
   int iFlags_a );

  time_t time_tFromYmds          /* In file: timesubs.c */
  (volatile const struct ymds_time *pYmds_a,
   MESSAGE *iStatus_a );
#endif  /* #ifdef INCLUDE_SYSTIME_PROTOTYPES */

void ymds_add_seconds		/* In file timesubs.c */
( SINT4 ioffset_a,
  volatile struct ymds_time* time_a );

/* --------------------------------------------------------------
 * ---------------- Custom Shared DSP Support -------------------
 * --------------------------------------------------------------
 */
#ifdef SIGMET_RVP8_H

#define BATCHOPARGLENGTH   11	/* Length of 16-bit encoding */
#define BATCHOPARGSET  0x2000 	/* Opcode bits for SET */
#define BATCHOPARGGET  0x1000 	/* Opcode bits for GET */

void rvp8DecodeBatchOpcode
( volatile struct rvp8BatchSetup *pUser_a,
  const UINT2 args_a[], SINT4 nArgs_a ) ;

SINT4 rvp8EncodeBatchOpcode
( volatile const struct rvp8BatchSetup *pUser_a,
  UINT2 args_a[ BATCHOPARGLENGTH ] ) ;

void rvp8DecodeSpecFilt
( volatile struct rvp8SpecFilt *pUser_a,
  volatile const struct rvp8SpecFiltIO *pCode_a ) ;

void rvp8EncodeSpecFilt
( volatile struct rvp8SpecFiltIO *pCode_a,
  volatile const struct rvp8SpecFilt *pUser_a ) ;

#endif /* #ifdef SIGMET_RVP8_H */

#ifdef __cplusplus
}
#endif
#endif /* #ifndef MESSAGE_ONLY */
#endif /* #ifndef SIGMET_USER_LIB_H */
