C    *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
C    ** Copyright UCAR (c) 1992 - 2010 
C    ** University Corporation for Atmospheric Research(UCAR) 
C    ** National Center for Atmospheric Research(NCAR) 
C    ** Research Applications Laboratory(RAL) 
C    ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
C    ** 2010/10/7 23:12:29 
C    *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
      SUBROUTINE W3FI82 (IFLD,FVAL1,FDIFF1,NPTS)
C$$$  SUBPROGRAM DOCUMENTATION BLOCK
C                .      .    .                                       .
C SUBPROGRAM:  W3FI82        CONVERT TO SECOND DIFF ARRAY
C   PRGMMR: CAVANAUGH        ORG: NMC421      DATE:93-08-18
C
C ABSTRACT: ACCEPT AN INPUT ARRAY, CONVERT TO ARRAY OF SECOND
C   DIFFERENCES.  RETURN THE ORIGINAL FIRST VALUE AND THE FIRST
C   FIRST-DIFFERENCE AS SEPARATE VALUES.
C
C PROGRAM HISTORY LOG:
C   93-07-14  CAVANAUGH
C   YY-MM-DD  MODIFIER1   DESCRIPTION OF CHANGE
C
C USAGE:    CALL W3FI82 (IFLD,FVAL1,FDIFF1,NPTS)
C   INPUT ARGUMENT LIST:
C     IFLD     - INTEGER INPUT ARRAY
C     NPTS     - NUMBER OF POINTS IN ARRAY
C
C   OUTPUT ARGUMENT LIST:
C     IFLD     - SECOND DIFFERENCED FIELD
C     FVAL1    - FLOATING POINT ORIGINAL FIRST VALUE
C     FDIFF1   -     "      "   FIRST FIRST-DIFFERENCE
C
C REMARKS: LIST CAVEATS, OTHER HELPFUL HINTS OR INFORMATION
C
C ATTRIBUTES:
C   LANGUAGE: FORTRAN 77
C   MACHINE:  NAS, CYBER
C
C$$$
      REAL        FVAL1,FDIFF1
      INTEGER     IFLD(*),NPTS
C
C  ---------------------------------------------
          DO 4000 I = NPTS, 2, -1
              IFLD(I)  = IFLD(I) - IFLD(I-1)
 4000     CONTINUE
          DO 5000 I = NPTS, 3, -1
              IFLD(I)  = IFLD(I) - IFLD(I-1)
 5000     CONTINUE
C         PRINT *,'IFLD(1) =',IFLD(1),'  IFLD(2) =',IFLD(2)
C
C                      SPECIAL FOR GRIB
C                         FLOAT OUTPUT OF FIRST POINTS TO ANTICIPATE
C                         GRIB FLOATING POINT OUTPUT
C
          FVAL1    = IFLD(1)
          FDIFF1   = IFLD(2)
C
C       SET FIRST TWO POINTS TO SECOND DIFF VALUE FOR BETTER PACKING
C
          IFLD(1)  = IFLD(3)
          IFLD(2)  = IFLD(3)
C  -----------------------------------------------------------
      RETURN
      END
