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
C
      SUBROUTINE W3FI01(LW)
C$$$  SUBPROGRAM DOCUMENTATION BLOCK
C                .      .    .                                       .
C SUBPROGRAM:    W3FI01      DETERMINES MACHINE WORD LENGTH IN BYTES
C   PRGMMR: KEYSER           ORG: W/NMC22    DATE: 06-29-92
C
C ABSTRACT: DETERMINES THE NUMBER OF BYTES IN A FULL WORD FOR THE
C   PARTICULAR MACHINE (IBM OR CRAY).
C
C PROGRAM HISTORY LOG:
C   92-01-10  R. KISTLER (W/NMC23)
C   92-05-22  D. A. KEYSER -- DOCBLOCKED/COMMENTED
C
C USAGE:    CALL W3FI01(LW)
C   OUTPUT ARGUMENT LIST:      (INCLUDING WORK ARRAYS)
C     LW       - MACHINE WORD LENGTH IN BYTES
C
C   OUTPUT FILES:
C     FT06F001 - PRINTOUT
C
C REMARKS: NONE.
C
C ATTRIBUTES:
C   LANGUAGE: CRAY CFT77 FORTRAN
C   MACHINE:  CRAY Y-MP8/832
C
C$$$
C
      CHARACTER*8  CTEST1,CTEST2
C
      INTEGER      ITEST1,ITEST2
C
      EQUIVALENCE  (CTEST1,ITEST1),(CTEST2,ITEST2)
C
      SAVE
C
      DATA  CTEST1/'12345678'/
C
      ITEST2 = ITEST1
CCCCC PRINT *,' CTEST1 = ',CTEST1,' CTEST2 = ',CTEST2
      IF (CTEST1 .EQ. CTEST2) THEN
CCCCC   PRINT*,' MACHINE WORD LENGTH IS 8 BYTES'
        LW = 8
      ELSE
CCCCC   PRINT*,' MACHINE WORD LENGTH IS 4 BYTES'
        LW = 4
      END IF
      RETURN
      END
