C    *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
C    ** Copyright UCAR (c) 1992 - 2010 
C    ** University Corporation for Atmospheric Research(UCAR) 
C    ** National Center for Atmospheric Research(NCAR) 
C    ** Research Applications Laboratory(RAL) 
C    ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
C    ** 2010/10/7 23:12:29 
C    *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
C
C +------------------------------------------------------------------+
C          DATA SET UTILITY    AT LEVEL 003 AS OF 02/25/92
      SUBROUTINE GBYTE(IN,IOUT,ISKIP,NBYTE)
C
C THIS PROGRAM WRITTEN BY.....
C             DR. ROBERT C. GAMMILL, CONSULTANT
C             NATIONAL CENTER FOR ATMOSPHERIC RESEARCH
C             MAY 1972
C
C             CHANGES FOR CRAY Y-MP8/832
C             CRAY CFT77 FORTRAN 
C             JULY 1992, RUSSELL E. JONES
C             NATIONAL WEATHER SERVICE
C
C THIS IS THE FORTRAN VERSION OF GBYTE
C
      INTEGER    IN(*)
      INTEGER    IOUT
#ifdef BIT64
      INTEGER    MASKS(64)
C
      DATA  NBITSW/64/
C     DATA  MASKS /Z'00000001',Z'00000003',Z'00000007',Z'0000000F',
C    &             Z'0000001F',Z'0000003F',Z'0000007F',Z'000000FF',
C    &             Z'000001FF',Z'000003FF',Z'000007FF',Z'00000FFF',
C    &             Z'00001FFF',Z'00003FFF',Z'00007FFF',Z'0000FFFF',
C    &             Z'0001FFFF',Z'0003FFFF',Z'0007FFFF',Z'000FFFFF',
C    &             Z'001FFFFF',Z'003FFFFF',Z'007FFFFF',Z'00FFFFFF',
C    &             Z'01FFFFFF',Z'03FFFFFF',Z'07FFFFFF',Z'0FFFFFFF',
C    &             Z'1FFFFFFF',Z'3FFFFFFF',Z'7FFFFFFF',Z'FFFFFFFF',
C    &   Z'1FFFFFFFF',  Z'3FFFFFFFF',  Z'7FFFFFFFF',  Z'FFFFFFFFF',            
C    &  Z'1FFFFFFFFF', Z'3FFFFFFFFF', Z'7FFFFFFFFF', Z'FFFFFFFFFF',            
C    & Z'1FFFFFFFFFF',Z'3FFFFFFFFFF',Z'7FFFFFFFFFF',Z'FFFFFFFFFFF',            
C    & Z'1FFFFFFFFFFF',Z'3FFFFFFFFFFF',Z'7FFFFFFFFFFF',
C    & Z'FFFFFFFFFFFF',Z'1FFFFFFFFFFFF',Z'3FFFFFFFFFFFF', 
C    & Z'7FFFFFFFFFFFF,Z'FFFFFFFFFFFFF',Z'1FFFFFFFFFFFFF',
C    & Z'3FFFFFFFFFFFFF',Z'7FFFFFFFFFFFFF',Z'FFFFFFFFFFFFFF',
C    & Z'1FFFFFFFFFFFFFF',Z'3FFFFFFFFFFFFFF,Z'7FFFFFFFFFFFFFF',
C    & Z'FFFFFFFFFFFFFFF',Z'1FFFFFFFFFFFFFFF',Z'3FFFFFFFFFFFFFFF',
C    & Z'7FFFFFFFFFFFFFFF',Z'FFFFFFFFFFFFFFFF'/           
C
C     MASKS TABLE PUT IN DECIMAL SO IT WILL COMPILE ON ANY 64 BIT 
C     COMPUTER 
C
      DATA  MASKS / 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047,
     & 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287,
     & 1048575, 2097151, 4194303, 8388607, 16777215, 33554431,
     & 67108863, 134217727, 268435455, 536870911, 1073741823,
     & 2147483647, 4294967295, 8589934591, 17179869183,
     & 34359738367, 68719476735, 137438953471, 274877906943,
     & 549755813887, 1099511627775, 2199023255551, 4398046511103,
     & 8796093022207, 17592186044415, 35184372088831,
     & 70368744177663, 140737488355327, 281474976710655,
     & 562949953421311, 1125899906842623, 2251799813685247,
     & 4503599627370495, 9007199254740991, 18014398509481983,
     & 36028797018963967, 72057594037927935, 144115188075855871,
     & 288230376151711743, 576460752303423487, 1152921504606846975,
     & 2305843009213693951, 4611686018427387903, 9223372036854775807,
     & -1/
#else
      INTEGER    MASKS(32)
C
      DATA  NBITSW/32/
C     DATA  MASKS /Z'00000001',Z'00000003',Z'00000007',Z'0000000F',
C    &             Z'0000001F',Z'0000003F',Z'0000007F',Z'000000FF',
C    &             Z'000001FF',Z'000003FF',Z'0000007FF',Z'00000FFF',
C    &             Z'00001FFF',Z'00003FFF',Z'00007FFF',Z'0000FFFF',
C    &             Z'0001FFFF',Z'0003FFFF',Z'0007FFFF',Z'000FFFFF',
C    &             Z'001FFFFF',Z'003FFFFF',Z'007FFFFF',Z'00FFFFFF',
C    &             Z'01FFFFFF',Z'03FFFFFF',Z'07FFFFFF',Z'0FFFFFFF',
C    &             Z'1FFFFFFF',Z'3FFFFFFF',Z'7FFFFFFF',Z'FFFFFFFF',
C
C     MASKS TABLE PUT IN DECIMAL SO IT WILL COMPILE ON ANY 32 BIT 
C     COMPUTER 
C
      DATA  MASKS / 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047,
     & 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287,
     & 1048575, 2097151, 4194303, 8388607, 16777215, 33554431,
     & 67108863, 134217727, 268435455, 536870911, 1073741823,
     & 2147483647, -1/
#endif
C
C NBYTE MUST BE LESS THAN OR EQUAL TO NBITSW                                    
C
      ICON   = NBITSW - NBYTE
      IF (ICON.LT.0) RETURN
      MASK   = MASKS(NBYTE)
C
C INDEX TELLS HOW MANY WORDS INTO THE ARRAY 'IN' THE NEXT BYTE APPEARS.         
C
      INDEX  = ISKIP / NBITSW
C
C II TELLS HOW MANY BITS THE BYTE IS FROM THE LEFT SIDE OF THE WORD.
C
      II     = MOD(ISKIP,NBITSW)
C
C MOVER SPECIFIES HOW FAR TO THE RIGHT NBYTE MUST BE MOVED IN ORDER            
C
C    TO BE RIGHT ADJUSTED.                                                 
C
      MOVER = ICON - II
C
      IF (MOVER.GT.0) THEN
        IOUT  = IAND(ISHFT(IN(INDEX+1),-MOVER),MASK)
C
C THE BYTE IS SPLIT ACROSS A WORD BREAK.                 
C
      ELSE IF (MOVER.LT.0) THEN                      
        MOVEL = - MOVER
        MOVER = NBITSW - MOVEL
        IOUT  = IAND(IOR(ISHFT(IN(INDEX+1),MOVEL),
     &          ISHFT(IN(INDEX+2),-MOVER)),MASK)
C
C THE BYTE IS ALREADY RIGHT ADJUSTED.
C  
      ELSE                              
        IOUT  = IAND(IN(INDEX+1),MASK) 
      ENDIF
C   
      RETURN
      END
C
C +------------------------------------------------------------------+
      SUBROUTINE GBYTES(IN,IOUT,ISKIP,NBYTE,NSKIP,N)
CDIR$ INTEGER=64
C
C THIS PROGRAM WRITTEN BY.....
C             DR. ROBERT C. GAMMILL, CONSULTANT
C             NATIONAL CENTER FOR ATMOSPHERIC RESEARCH
C             MAY 1972
C
C             CHANGES FOR CRAY Y-MP8/832
C             CRAY CFT77 FORTRAN 
C             JULY 1992, RUSSELL E. JONES
C             NATIONAL WEATHER SERVICE
C
C THIS IS THE FORTRAN VERSION OF GBYTES.
C
      INTEGER    IN(*)
      INTEGER    IOUT(*)
#ifdef BIT64
      INTEGER    MASKS(64)
C
      DATA  NBITSW/64/
C     DATA  MASKS /Z'00000001',Z'00000003',Z'00000007',Z'0000000F',
C    &             Z'0000001F',Z'0000003F',Z'0000007F',Z'000000FF',
C    &             Z'000001FF',Z'000003FF',Z'000007FF',Z'00000FFF',
C    &             Z'00001FFF',Z'00003FFF',Z'00007FFF',Z'0000FFFF',
C    &             Z'0001FFFF',Z'0003FFFF',Z'0007FFFF',Z'000FFFFF',
C    &             Z'001FFFFF',Z'003FFFFF',Z'007FFFFF',Z'00FFFFFF',
C    &             Z'01FFFFFF',Z'03FFFFFF',Z'07FFFFFF',Z'0FFFFFFF',
C    &             Z'1FFFFFFF',Z'3FFFFFFF',Z'7FFFFFFF',Z'FFFFFFFF',
C    &   Z'1FFFFFFFF',  Z'3FFFFFFFF',  Z'7FFFFFFFF',  Z'FFFFFFFFF',            
C    &  Z'1FFFFFFFFF', Z'3FFFFFFFFF', Z'7FFFFFFFFF', Z'FFFFFFFFFF',            
C    & Z'1FFFFFFFFFF',Z'3FFFFFFFFFF',Z'7FFFFFFFFFF',Z'FFFFFFFFFFF',            
C    & Z'1FFFFFFFFFFF',Z'3FFFFFFFFFFF',Z'7FFFFFFFFFFF',
C    & Z'FFFFFFFFFFFF',Z'1FFFFFFFFFFFF',Z'3FFFFFFFFFFFF', 
C    & Z'7FFFFFFFFFFFF,Z'FFFFFFFFFFFFF',Z'1FFFFFFFFFFFFF',
C    & Z'3FFFFFFFFFFFFF',Z'7FFFFFFFFFFFFF',Z'FFFFFFFFFFFFFF',
C    & Z'1FFFFFFFFFFFFFF',Z'3FFFFFFFFFFFFFF,Z'7FFFFFFFFFFFFFF',
C    & Z'FFFFFFFFFFFFFFF',Z'1FFFFFFFFFFFFFFF',Z'3FFFFFFFFFFFFFFF',
C    & Z'7FFFFFFFFFFFFFFF',Z'FFFFFFFFFFFFFFFF'/           
C
C     MASKS TABLE PUT IN DECIMAL SO IT WILL COMPILE ON ANY 64 BIT 
C     COMPUTER 
C
      DATA  MASKS / 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047,
     & 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287,
     & 1048575, 2097151, 4194303, 8388607, 16777215, 33554431,
     & 67108863, 134217727, 268435455, 536870911, 1073741823,
     & 2147483647, 4294967295, 8589934591, 17179869183,
     & 34359738367, 68719476735, 137438953471, 274877906943,
     & 549755813887, 1099511627775, 2199023255551, 4398046511103,
     & 8796093022207, 17592186044415, 35184372088831,
     & 70368744177663, 140737488355327, 281474976710655,
     & 562949953421311, 1125899906842623, 2251799813685247,
     & 4503599627370495, 9007199254740991, 18014398509481983,
     & 36028797018963967, 72057594037927935, 144115188075855871,
     & 288230376151711743, 576460752303423487, 1152921504606846975,
     & 2305843009213693951, 4611686018427387903, 9223372036854775807,
     & -1/
#else
      INTEGER    MASKS(32)
C
      DATA  NBITSW/32/
C     DATA  MASKS /Z'00000001',Z'00000003',Z'00000007',Z'0000000F',
C    &             Z'0000001F',Z'0000003F',Z'0000007F',Z'000000FF',
C    &             Z'000001FF',Z'000003FF',Z'000007FF',Z'00000FFF',
C    &             Z'00001FFF',Z'00003FFF',Z'00007FFF',Z'0000FFFF',
C    &             Z'0001FFFF',Z'0003FFFF',Z'0007FFFF',Z'000FFFFF',
C    &             Z'001FFFFF',Z'003FFFFF',Z'007FFFFF',Z'00FFFFFF',
C    &             Z'01FFFFFF',Z'03FFFFFF',Z'07FFFFFF',Z'0FFFFFFF',
C    &             Z'1FFFFFFF',Z'3FFFFFFF',Z'7FFFFFFF',Z'FFFFFFFF',
C
C     MASKS TABLE PUT IN DECIMAL SO IT WILL COMPILE ON ANY 32 BIT 
C     COMPUTER 
C
      DATA  MASKS / 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047,
     & 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287,
     & 1048575, 2097151, 4194303, 8388607, 16777215, 33554431,
     & 67108863, 134217727, 268435455, 536870911, 1073741823,
     & 2147483647, -1/
#endif
C
C NBYTE MUST BE LESS THAN OR EQUAL TO NBITSW                                    
C
      ICON   = NBITSW - NBYTE
      IF (ICON.LT.0) RETURN
      MASK   = MASKS(NBYTE)
C
C INDEX TELLS HOW MANY WORDS INTO THE ARRAY 'IN' THE NEXT BYTE APPEARS.         
C
      INDEX  = ISKIP / NBITSW
C
C II TELLS HOW MANY BITS THE BYTE IS FROM THE LEFT SIDE OF THE WORD.
C
      II     = MOD(ISKIP,NBITSW)
C
C ISTEP IS THE DISTANCE IN BITS FROM THE START OF ONE BYTE TO THE NEXT.
C
      ISTEP  = NBYTE + NSKIP      
C
C IWORDS TELLS HOW MANY WORDS TO SKIP FROM ONE BYTE TO THE NEXT.                
C
      IWORDS = ISTEP / NBITSW    
C
C IBITS TELLS HOW MANY BITS TO SKIP AFTER SKIPPING IWORDS.                      
C
      IBITS  = MOD(ISTEP,NBITSW) 
C
      DO 10 I = 1,N
C
C MOVER SPECIFIES HOW FAR TO THE RIGHT A BYTE MUST BE MOVED IN ORDER            
C
C    TO BE RIGHT ADJUSTED.                                                      
C    TO BE RIGHT ADJUSTED.
C
      MOVER = ICON - II
C
C THE BYTE IS SPLIT ACROSS A WORD BREAK.                 
C                       
      IF (MOVER.LT.0) THEN
        MOVEL   = - MOVER
        MOVER   = NBITSW - MOVEL
        IOUT(I) = IAND(IOR(ISHFT(IN(INDEX+1),MOVEL),
     &            ISHFT(IN(INDEX+2),-MOVER)),MASK)
C
C RIGHT ADJUST THE BYTE.
C
      ELSE IF (MOVER.GT.0) THEN
        IOUT(I) = IAND(ISHFT(IN(INDEX+1),-MOVER),MASK)
C                                             
C THE BYTE IS ALREADY RIGHT ADJUSTED.
C
      ELSE
        IOUT(I) = IAND(IN(INDEX+1),MASK)
      ENDIF
C
C INCREMENT II AND INDEX.
C
        II    = II + IBITS
        INDEX = INDEX + IWORDS
        IF (II.GE.NBITSW) THEN
          II    = II - NBITSW
          INDEX = INDEX + 1
        ENDIF
C
   10 CONTINUE
        RETURN
      END
C
C +------------------------------------------------------------------+
      SUBROUTINE SBYTE(IOUT,IN,ISKIP,NBYTE)
C THIS PROGRAM WRITTEN BY.....
C             DR. ROBERT C. GAMMILL, CONSULTANT
C             NATIONAL CENTER FOR ATMOSPHERIC RESEARCH
C             JULY 1972
C THIS IS THE FORTRAN VERSIONS OF SBYTE.
C             FORTRAN 90
C             AUGUST 1990  RUSSELL E. JONES
C             NATIONAL WEATHER SERVICE
C
C USAGE:    CALL SBYTE (PCKD,UNPK,INOFST,NBIT)
C
C   INPUT ARGUMENT LIST:
C     UNPK     -  NBITS OF THE RIGHT SIDE OF UNPK IS MOVED TO
C                 ARRAY PCKD. INOFST BITS ARE SKIPPED OVER BEFORE
C                 THE DATA IS MOVED, NBITS ARE STORED.
C    INOFST    -  A FULLWORD INTEGER SPECIFYING THE INITAL OFFSET
C                 IN BITS OF THE FIRST BYTE, COUNTED FROM THE
C                 LEFTMOST BIT IN PCKD.
C    NBITS     -  A FULLWORD INTEGER SPECIFYING THE NUMBER OF BITS
C                 IN EACH BYTE TO BE PACKED.  LEGAL BYTE WIDTHS
C                 ARE IN THE RANGE 1 - 32.
C   OUTPUT ARGUMENT LIST:
C    PCKD      -  THE FULLWORD IN MEMORY TO WHICH PACKING IS TO
C                 BEGIN STARTING AT BIT INOFST. THE INOSTAT BITS
C                 ARE NOT ALTERED.
C
      INTEGER    IN
      INTEGER    IOUT(*)
#ifdef BIT64
      INTEGER    MASKS(64)
C
      DATA  NBITSW/64/
C     DATA  MASKS /Z'00000001',Z'00000003',Z'00000007',Z'0000000F',
C    &             Z'0000001F',Z'0000003F',Z'0000007F',Z'000000FF',
C    &             Z'000001FF',Z'000003FF',Z'000007FF',Z'00000FFF',
C    &             Z'00001FFF',Z'00003FFF',Z'00007FFF',Z'0000FFFF',
C    &             Z'0001FFFF',Z'0003FFFF',Z'0007FFFF',Z'000FFFFF',
C    &             Z'001FFFFF',Z'003FFFFF',Z'007FFFFF',Z'00FFFFFF',
C    &             Z'01FFFFFF',Z'03FFFFFF',Z'07FFFFFF',Z'0FFFFFFF',
C    &             Z'1FFFFFFF',Z'3FFFFFFF',Z'7FFFFFFF',Z'FFFFFFFF',
C    &   Z'1FFFFFFFF',  Z'3FFFFFFFF',  Z'7FFFFFFFF',  Z'FFFFFFFFF',            
C    &  Z'1FFFFFFFFF', Z'3FFFFFFFFF', Z'7FFFFFFFFF', Z'FFFFFFFFFF',            
C    & Z'1FFFFFFFFFF',Z'3FFFFFFFFFF',Z'7FFFFFFFFFF',Z'FFFFFFFFFFF',            
C    & Z'1FFFFFFFFFFF',Z'3FFFFFFFFFFF',Z'7FFFFFFFFFFF',
C    & Z'FFFFFFFFFFFF',Z'1FFFFFFFFFFFF',Z'3FFFFFFFFFFFF', 
C    & Z'7FFFFFFFFFFFF,Z'FFFFFFFFFFFFF',Z'1FFFFFFFFFFFFF',
C    & Z'3FFFFFFFFFFFFF',Z'7FFFFFFFFFFFFF',Z'FFFFFFFFFFFFFF',
C    & Z'1FFFFFFFFFFFFFF',Z'3FFFFFFFFFFFFFF,Z'7FFFFFFFFFFFFFF',
C    & Z'FFFFFFFFFFFFFFF',Z'1FFFFFFFFFFFFFFF',Z'3FFFFFFFFFFFFFFF',
C    & Z'7FFFFFFFFFFFFFFF',Z'FFFFFFFFFFFFFFFF'/           
C
C     MASKS TABLE PUT IN DECIMAL SO IT WILL COMPILE ON ANY 64 BIT 
C     COMPUTER 
C
      DATA  MASKS / 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047,
     & 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287,
     & 1048575, 2097151, 4194303, 8388607, 16777215, 33554431,
     & 67108863, 134217727, 268435455, 536870911, 1073741823,
     & 2147483647, 4294967295, 8589934591, 17179869183,
     & 34359738367, 68719476735, 137438953471, 274877906943,
     & 549755813887, 1099511627775, 2199023255551, 4398046511103,
     & 8796093022207, 17592186044415, 35184372088831,
     & 70368744177663, 140737488355327, 281474976710655,
     & 562949953421311, 1125899906842623, 2251799813685247,
     & 4503599627370495, 9007199254740991, 18014398509481983,
     & 36028797018963967, 72057594037927935, 144115188075855871,
     & 288230376151711743, 576460752303423487, 1152921504606846975,
     & 2305843009213693951, 4611686018427387903, 9223372036854775807,
     & -1/
#else
      INTEGER    MASKS(32)
C
      DATA  NBITSW/32/
C     DATA  MASKS /Z'00000001',Z'00000003',Z'00000007',Z'0000000F',
C    &             Z'0000001F',Z'0000003F',Z'0000007F',Z'000000FF',
C    &             Z'000001FF',Z'000003FF',Z'000007FF',Z'00000FFF',
C    &             Z'00001FFF',Z'00003FFF',Z'00007FFF',Z'0000FFFF',
C    &             Z'0001FFFF',Z'0003FFFF',Z'0007FFFF',Z'000FFFFF',
C    &             Z'001FFFFF',Z'003FFFFF',Z'007FFFFF',Z'00FFFFFF',
C    &             Z'01FFFFFF',Z'03FFFFFF',Z'07FFFFFF',Z'0FFFFFFF',
C    &             Z'1FFFFFFF',Z'3FFFFFFF',Z'7FFFFFFF',Z'FFFFFFFF',
C
C     MASKS TABLE PUT IN DECIMAL SO IT WILL COMPILE ON ANY 32 BIT 
C     COMPUTER 
C
      DATA  MASKS / 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047,
     & 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287,
     & 1048575, 2097151, 4194303, 8388607, 16777215, 33554431,
     & 67108863, 134217727, 268435455, 536870911, 1073741823,
     & 2147483647, -1/
#endif
C
C NBYTE MUST BE LESS THAN OR EQUAL TO NBITSW
C
        ICON  = NBITSW - NBYTE
        IF (ICON.LT.0) RETURN
        MASK  = MASKS(NBYTE)
C
C INDEX TELLS HOW MANY WORDS INTO IOUT THE NEXT BYTE IS TO BE STORED.
C
        INDEX = ISKIP / NBITSW
C
C II TELLS HOW MANY BITS IN FROM THE LEFT SIDE OF THE WORD TO STORE IT.
C
        II    = MOD(ISKIP,NBITSW)
C
        J     = IAND(MASK,IN)
        MOVEL = ICON - II
C
C BYTE IS TO BE STORED IN MIDDLE OF WORD.  SHIFT LEFT.
C
        IF (MOVEL.GT.0) THEN
          MSK           = ISHFT(MASK,MOVEL)
          IOUT(INDEX+1) = IOR(IAND(NOT(MSK),IOUT(INDEX+1)),
     &    ISHFT(J,MOVEL))
C
C THE BYTE IS TO BE SPLIT ACROSS A WORD BREAK.
C
        ELSE IF (MOVEL.LT.0) THEN
          MSK           = MASKS(NBYTE+MOVEL)
          IOUT(INDEX+1) = IOR(IAND(NOT(MSK),IOUT(INDEX+1)),
     &    ISHFT(J,MOVEL))
          ITEMP         = IAND(MASKS(NBITSW+MOVEL),IOUT(INDEX+2))
          IOUT(INDEX+2) = IOR(ITEMP,ISHFT(J,NBITSW+MOVEL))
C
C BYTE IS TO BE STORED RIGHT-ADJUSTED.
C
        ELSE
          IOUT(INDEX+1) = IOR(IAND(NOT(MASK),IOUT(INDEX+1)),J)
        ENDIF
C
      RETURN
      END
C
C +------------------------------------------------------------------+
      SUBROUTINE SBYTES(IOUT,IN,ISKIP,NBYTE,NSKIP,N)
C THIS PROGRAM WRITTEN BY.....
C             DR. ROBERT C. GAMMILL, CONSULTANT
C             NATIONAL CENTER FOR ATMOSPHERIC RESEARCH
C             JULY 1972
C THIS IS THE FORTRAN VERSIONS OF SBYTES.
C
C             FORTRAN 90
C             AUGUST 1990  RUSSELL E. JONES
C             NATIONAL WEATHER SERVICE
C
C USAGE:    CALL SBYTES (PCKD,UNPK,INOFST,NBIT, NSKIP,ITER)
C
C   INPUT ARGUMENT LIST:
C     UNPK     -  NBITS OF THE RIGHT SIDE OF EACH WORD OF ARRAY
C                 UNPK IS MOVED TO ARRAY PCKD. INOFST BITS ARE
C                 SKIPPED OVER BEFORE THE 1ST DATA IS MOVED, NBITS
C                 ARE STORED, NSKIP BITS ARE SKIPPED OVER, THE NEXT
C                 NBITS ARE MOVED,  BIT ARE SKIPPED OVER, ETC. UNTIL
C                 ITER GROUPS OF BITS ARE PACKED.
C    INOFST    -  A FULLWORD INTEGER SPECIFYING THE INITAL OFFSET
C                 IN BITS OF THE FIRST BYTE, COUNTED FROM THE
C                 LEFTMOST BIT IN PCKD.
C    NBITS     -  A FULLWORD INTEGER SPECIFYING THE NUMBER OF BITS
C                 IN EACH BYTE TO BE PACKED.  LEGAL BYTE WIDTHS
C                 ARE IN THE RANGE 1 - 32.
C    NSKIP     -  A FULLWORD INTEGER SPECIFYING THE NUMBER OF BITS
C                 TO SKIP BETWEEN SUCCESSIVE BYTES.  ALL NON-NEGATIVE
C                 SKIP COUNTS ARE LEGAL.
C    ITER      -  A FULLWORD INTEGER SPECIFYING THE TOTAL NUMBER OF
C                 BYTES TO BE PACKED, AS CONTROLLED BY INOFST,
C                 NBIT AND NSKIP ABOVE.   ALL NON-NEGATIVE ITERATION
C                 COUNTS ARE LEGAL.
C
C   OUTPUT ARGUMENT LIST:
C    PCKD      -  THE FULLWORD IN MEMORY TO WHICH PACKING IS TO
C                 BEGIN STARTING AT BIT INOFST. THE INOSTAT BITS
C                 ARE NOT ALTERED. NSKIP BITS ARE NOT ALTERED.
C
      INTEGER    IN(*)
      INTEGER    IOUT(*)
#ifdef BIT64
      INTEGER    MASKS(64)
C
      DATA  NBITSW/64/
C     DATA  MASKS /Z'00000001',Z'00000003',Z'00000007',Z'0000000F',
C    &             Z'0000001F',Z'0000003F',Z'0000007F',Z'000000FF',
C    &             Z'000001FF',Z'000003FF',Z'000007FF',Z'00000FFF',
C    &             Z'00001FFF',Z'00003FFF',Z'00007FFF',Z'0000FFFF',
C    &             Z'0001FFFF',Z'0003FFFF',Z'0007FFFF',Z'000FFFFF',
C    &             Z'001FFFFF',Z'003FFFFF',Z'007FFFFF',Z'00FFFFFF',
C    &             Z'01FFFFFF',Z'03FFFFFF',Z'07FFFFFF',Z'0FFFFFFF',
C    &             Z'1FFFFFFF',Z'3FFFFFFF',Z'7FFFFFFF',Z'FFFFFFFF',
C    &   Z'1FFFFFFFF',  Z'3FFFFFFFF',  Z'7FFFFFFFF',  Z'FFFFFFFFF',            
C    &  Z'1FFFFFFFFF', Z'3FFFFFFFFF', Z'7FFFFFFFFF', Z'FFFFFFFFFF',            
C    & Z'1FFFFFFFFFF',Z'3FFFFFFFFFF',Z'7FFFFFFFFFF',Z'FFFFFFFFFFF',            
C    & Z'1FFFFFFFFFFF',Z'3FFFFFFFFFFF',Z'7FFFFFFFFFFF',
C    & Z'FFFFFFFFFFFF',Z'1FFFFFFFFFFFF',Z'3FFFFFFFFFFFF', 
C    & Z'7FFFFFFFFFFFF,Z'FFFFFFFFFFFFF',Z'1FFFFFFFFFFFFF',
C    & Z'3FFFFFFFFFFFFF',Z'7FFFFFFFFFFFFF',Z'FFFFFFFFFFFFFF',
C    & Z'1FFFFFFFFFFFFFF',Z'3FFFFFFFFFFFFFF,Z'7FFFFFFFFFFFFFF',
C    & Z'FFFFFFFFFFFFFFF',Z'1FFFFFFFFFFFFFFF',Z'3FFFFFFFFFFFFFFF',
C    & Z'7FFFFFFFFFFFFFFF',Z'FFFFFFFFFFFFFFFF'/           
C
C     MASKS TABLE PUT IN DECIMAL SO IT WILL COMPILE ON ANY 64 BIT 
C     COMPUTER 
C
      DATA  MASKS / 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047,
     & 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287,
     & 1048575, 2097151, 4194303, 8388607, 16777215, 33554431,
     & 67108863, 134217727, 268435455, 536870911, 1073741823,
     & 2147483647, 4294967295, 8589934591, 17179869183,
     & 34359738367, 68719476735, 137438953471, 274877906943,
     & 549755813887, 1099511627775, 2199023255551, 4398046511103,
     & 8796093022207, 17592186044415, 35184372088831,
     & 70368744177663, 140737488355327, 281474976710655,
     & 562949953421311, 1125899906842623, 2251799813685247,
     & 4503599627370495, 9007199254740991, 18014398509481983,
     & 36028797018963967, 72057594037927935, 144115188075855871,
     & 288230376151711743, 576460752303423487, 1152921504606846975,
     & 2305843009213693951, 4611686018427387903, 9223372036854775807,
     & -1/
#else
      INTEGER    MASKS(32)
C
      DATA  NBITSW/32/
C     DATA  MASKS /Z'00000001',Z'00000003',Z'00000007',Z'0000000F',
C    &             Z'0000001F',Z'0000003F',Z'0000007F',Z'000000FF',
C    &             Z'000001FF',Z'000003FF',Z'000007FF',Z'00000FFF',
C    &             Z'00001FFF',Z'00003FFF',Z'00007FFF',Z'0000FFFF',
C    &             Z'0001FFFF',Z'0003FFFF',Z'0007FFFF',Z'000FFFFF',
C    &             Z'001FFFFF',Z'003FFFFF',Z'007FFFFF',Z'00FFFFFF',
C    &             Z'01FFFFFF',Z'03FFFFFF',Z'07FFFFFF',Z'0FFFFFFF',
C    &             Z'1FFFFFFF',Z'3FFFFFFF',Z'7FFFFFFF',Z'FFFFFFFF',
C
C     MASKS TABLE PUT IN DECIMAL SO IT WILL COMPILE ON ANY 32 BIT 
C     COMPUTER 
C
      DATA  MASKS / 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047,
     & 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287,
     & 1048575, 2097151, 4194303, 8388607, 16777215, 33554431,
     & 67108863, 134217727, 268435455, 536870911, 1073741823,
     & 2147483647, -1/
#endif
C
C NBYTE MUST BE LESS THAN OR EQUAL TO NBITSW
C
      ICON = NBITSW - NBYTE
      IF (ICON.LT.0) RETURN
      MASK   = MASKS(NBYTE)
C
C INDEX TELLS HOW MANY WORDS INTO IOUT THE NEXT BYTE IS TO BE STORED.
C
      INDEX  = ISKIP / NBITSW
C
C II TELLS HOW MANY BITS IN FROM THE LEFT SIDE OF THE WORD TO STORE IT.
C
      II     = MOD(ISKIP,NBITSW)
C
C ISTEP IS THE DISTANCE IN BITS FROM ONE BYTE POSITION TO THE NEXT.
C
      ISTEP  = NBYTE + NSKIP
C
C IWORDS TELLS HOW MANY WORDS TO SKIP FROM ONE BYTE TO THE NEXT.
C
      IWORDS = ISTEP / NBITSW
C
C IBITS TELLS HOW MANY BITS TO SKIP AFTER SKIPPING IWORDS.
C
      IBITS  = MOD(ISTEP,NBITSW)
C
      DO 10 I = 1,N
        J     = IAND(MASK,IN(I))
        MOVEL = ICON - II
C
C BYTE IS TO BE STORED IN MIDDLE OF WORD.  SHIFT LEFT.
C
        IF (MOVEL.GT.0) THEN
          MSK           = ISHFT(MASK,MOVEL)
          IOUT(INDEX+1) = IOR(IAND(NOT(MSK),IOUT(INDEX+1)),
     &    ISHFT(J,MOVEL))
C
C THE BYTE IS TO BE SPLIT ACROSS A WORD BREAK.
C
        ELSE IF (MOVEL.LT.0) THEN
          MSK           = MASKS(NBYTE+MOVEL)
          IOUT(INDEX+1) = IOR(IAND(NOT(MSK),IOUT(INDEX+1)),
     &    ISHFT(J,MOVEL))
          ITEMP         = IAND(MASKS(NBITSW+MOVEL),IOUT(INDEX+2))
          IOUT(INDEX+2) = IOR(ITEMP,ISHFT(J,NBITSW+MOVEL))
C
C BYTE IS TO BE STORED RIGHT-ADJUSTED.
C
        ELSE
          IOUT(INDEX+1) = IOR(IAND(NOT(MASK),IOUT(INDEX+1)),J)
        ENDIF
C
        II    = II + IBITS
        INDEX = INDEX + IWORDS
        IF (II.GE.NBITSW) THEN
          II    = II - NBITSW
          INDEX = INDEX + 1
        ENDIF
C
10    CONTINUE
C
      RETURN
      END
C
C +------------------------------------------------------------------+
      FUNCTION INTRSHFT(IWORD,NSHFT)
C
C       This function shifts IWORD to the right NSHFT bits.
C
#ifdef CRAY
      INTRSHFT=SHIFTR(IWORD,NSHFT)
#else
      INTRSHFT=ISHFT(IWORD,-NSHFT)
#endif
      RETURN
      END
C +------------------------------------------------------------------+
      FUNCTION INTLSHFT(IWORD,NSHFT)
C
C       This function shifts IWORD to the left NSHFT bits in a
C         circular manner.
C
#ifdef CRAY
      INTLSHFT=SHIFT(IWORD,NSHFT)
#else
      INTLSHFT=ISHFT(IWORD,NSHFT)
#endif
      RETURN
      END
C +------------------------------------------------------------------+
      FUNCTION INTAND(IWORD1,IWORD2)
C
C       This function performs a bit-by-bit AND between IWORD1 and
C         IWORD2.
C
#ifdef CRAY
      INTAND=AND(IWORD1,IWORD2)
#else
      INTAND=IAND(IWORD1,IWORD2)
#endif
      RETURN
      END
C +------------------------------------------------------------------+
      FUNCTION INTOR(IWORD1,IWORD2)
C
C       This function performs a bit-by-bit OR between IWORD1 and
C         IWORD2.
C
#ifdef CRAY
      INTOR=OR(IWORD1,IWORD2)
#else
      INTOR=IOR(IWORD1,IWORD2)
#endif
      RETURN
      END
C +------------------------------------------------------------------+
      FUNCTION INTXOR(IWORD1,IWORD2)
C
C       This function performs an exclusive bit-by-bit OR between IWORD1 and
C         IWORD2.
C
      INTXOR=XOR(IWORD1,IWORD2)
C
      RETURN
      END
C +------------------------------------------------------------------+
      FUNCTION INTLMSK(NBITS)
C
C       This function sets NBITS bits to 1 on the left side of the word.
C
#ifdef CRAY
      INTLMSK=MASK(NBITS)
#elif defined(IBM) || defined(HP) || defined(VAX) || defined(TITAN)
      MSK=NOT(0)
      INTR=0
      CALL MVBITS(MSK,0,NBITS,INTR,31-NBITS)
      INTLMSK=INTR
#else
      INTLMSK=ishft(2**nbits-1, 32-nbits)
#endif
 
      RETURN
      END
C +------------------------------------------------------------------+
      FUNCTION INTRMSK(NBITS)
C
C       This function sets NBITS bits to 1 on the right side of the word
C
#ifdef CRAY
      INTRMSK=SHIFT(MASK(NBITS),NBITS)
#elif defined(IBM) || defined(HP) || defined(VAX) || defined(TITAN)
      MSK=NOT(0)
      INTR=0
      CALL MVBITS(MSK,0,NBITS,INTR,0)
      INTRMSK=INTR
#else
      INTRMSK=2**nbits-1
#endif
      RETURN
      END
C
C+---+-----------------------------------------------------------------+
C
      subroutine swap_bytes(msg,nbytes)
C
      implicit none
      integer nbytes, n
      character*1 msg(nbytes), temp
 
#ifdef BIT64
      do n = 1,nbytes,8
         temp=msg(n)
         msg(n)=msg(n+7)
         msg(n+7)=temp
         temp=msg(n+1)
         msg(n+1)=msg(n+6)
         msg(n+6)=temp
         temp=msg(n+2)
         msg(n+2)=msg(n+5)
         msg(n+5)=temp
         temp=msg(n+3)
         msg(n+3)=msg(n+4)
         msg(n+4)=temp
      enddo
#else
      do n = 1,nbytes,4
         temp=msg(n)
         msg(n)=msg(n+3)
         msg(n+3)=temp
         temp=msg(n+1)
         msg(n+1)=msg(n+2)
         msg(n+2)=temp
      enddo
#endif
C
      return
      end
C
C +------------------------------------------------------------------+
C
C ... The following routines came from NCEP via Clinton Wallace at
C ... the Aviation Weather Center.  Apparently, these versions are
C ... needed to make the bit map section work correctly on little
C ... endian machines (Linux).  The only calls made to these routines
C ... can be found in SUBROUTINE W3FI73.
C
      SUBROUTINE GBYTEC(IN,IOUT,ISKIP,NBYTE)
      character*1 in(*)
      integer iout(*)
      CALL GBYTESC(IN,IOUT,ISKIP,NBYTE,0,1)
      RETURN
      END

      SUBROUTINE SBYTEC(OUT,IN,ISKIP,NBYTE)
      character*1 out(*)
      integer in(*)
      CALL SBYTESC(OUT,IN,ISKIP,NBYTE,0,1)
      RETURN
      END

      SUBROUTINE GBYTESC(IN,IOUT,ISKIP,NBYTE,NSKIP,N)
C          Get bytes - unpack bits:  Extract arbitrary size values from a
C          packed bit string, right justifying each value in the unpacked
C          array.
C            IN    = character*1 array input
C            IOUT  = unpacked array output
C            ISKIP = initial number of bits to skip
C            NBYTE = number of bits to take
C            NSKIP = additional number of bits to skip on each iteration
C            N     = number of iterations
C v1.1
C
      character*1 in(*)
      integer iout(*)
      integer ones(8), tbit, bitcnt
      save ones
      data ones/1,3,7,15,31,63,127,255/

c     nbit is the start position of the field in bits
      nbit = iskip
      do i = 1, n
         bitcnt = nbyte
         index=nbit/8+1
         ibit=mod(nbit,8)
         nbit = nbit + nbyte + nskip

c        first byte
         tbit = min(bitcnt,8-ibit)
         itmp = iand(ichar(in(index)),ones(8-ibit))
         if (tbit.ne.8-ibit) itmp = ishft(itmp,tbit-8+ibit)
         index = index + 1
         bitcnt = bitcnt - tbit

c        now transfer whole bytes
         do while (bitcnt.ge.8)
             itmp = ior(ishft(itmp,8),ichar(in(index)))
             bitcnt = bitcnt - 8
             index = index + 1
         enddo

c        get data from last byte
         if (bitcnt.gt.0) then
             itmp = ior(ishft(itmp,bitcnt),iand(ishft(ichar(in(index)),
     1          -(8-bitcnt)),ones(bitcnt)))
         endif

         iout(i) = itmp
      enddo

      RETURN
      END

      SUBROUTINE SBYTESC(OUT,IN,ISKIP,NBYTE,NSKIP,N)
C          Store bytes - pack bits:  Put arbitrary size values into a
C          packed bit string, taking the low order bits from each value
C          in the unpacked array.
C            IOUT  = packed array output
C            IN    = unpacked array input
C            ISKIP = initial number of bits to skip
C            NBYTE = number of bits to pack
C            NSKIP = additional number of bits to skip on each iteration
C            N     = number of iterations
C v1.1
C
      character*1 out(*)
      integer in(N), bitcnt, ones(8), tbit
      save ones
      data ones/    1,  3,  7, 15, 31, 63,127,255/

c     number bits from zero to ...
c     nbit is the last bit of the field to be filled

      nbit = iskip + nbyte - 1
      do i = 1, n
         itmp = in(i)
         bitcnt = nbyte
         index=nbit/8+1
         ibit=mod(nbit,8)
         nbit = nbit + nbyte + nskip

c        make byte aligned 
         if (ibit.ne.7) then
             tbit = min(bitcnt,ibit+1)
             imask = ishft(ones(tbit),7-ibit)
             itmp2 = iand(ishft(itmp,7-ibit),imask)
             itmp3 = iand(ichar(out(index)), 255-imask)
             out(index) = char(ior(itmp2,itmp3))
             bitcnt = bitcnt - tbit
             itmp = ishft(itmp, -tbit)
             index = index - 1
         endif

c        now byte aligned

c        do by bytes
         do while (bitcnt.ge.8)
             out(index) = char(iand(itmp,255))
             itmp = ishft(itmp,-8)
             bitcnt = bitcnt - 8
             index = index - 1
         enddo

c        do last byte

         if (bitcnt.gt.0) then
             itmp2 = iand(itmp,ones(bitcnt))
             itmp3 = iand(ichar(out(index)), 255-ones(bitcnt))
             out(index) = char(ior(itmp2,itmp3))
         endif
      enddo

      return
      end
