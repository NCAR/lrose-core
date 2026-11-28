C
C Program that demonstrates how to write MDV data from FORTRAN.
C See comments in produce_mdv.cc.
C
C Niles Oien March 2005.
C

      PROGRAM produce_Mdv_demo

      IMPLICIT NONE

      INTEGER DEBUG
      PARAMETER (DEBUG = 3) ! Verbose debugging.

      INTEGER NX
      PARAMETER (NX = 100)
      INTEGER NY
      PARAMETER (NY = 150)
      INTEGER NZ
      PARAMETER (NZ = 5)    ! Grid size.

      REAL DX
      PARAMETER (DX = 1.0)
      REAL DY
      PARAMETER (DY = 1.0)  ! Grid resoulution.

      REAL MINX
      PARAMETER (MINX = -49.5)
      REAL MINY
      PARAMETER (MINY = -49.5) ! Grid navigation.

      REAL ORIGIN_LAT
      PARAMETER (ORIGIN_LAT = 40)

      REAL ORIGIN_LON
      PARAMETER (ORIGIN_LON = -90) ! Grid navigation.

      CHARACTER*128 FIELDNAMES
      CHARACTER*128 URL
      CHARACTER*128 UNITS

      INTEGER VLEVELTYPE
      PARAMETER (VLEVELTYPE = 4) ! 4 => vertical levels in kilometers.

      REAL VLEVELS
      DIMENSION VLEVELS(NZ) ! Actual heights of planes, in Km.

      INTEGER GRIDTYPE
      PARAMETER (GRIDTYPE = 2) ! 2 => Flat earth projection.

      REAL ROTATION
      PARAMETER (ROTATION = 30.0)

      REAL LAMBERTLAT1
      PARAMETER (LAMBERTLAT1 = 40.0)

      REAL LAMBERTLAT2
      PARAMETER (LAMBERTLAT2 = 40.0)


      REAL BADVAL
      PARAMETER (BADVAL = -999.0) ! Value to use for bad/missing data.

      INTEGER YEAR, MONTH, DAY, HOUR, MINUTE, SECOND
      PARAMETER (YEAR = 2005)
      PARAMETER (MONTH = 2)
      PARAMETER (DAY = 12)
      PARAMETER (HOUR = 1)
      PARAMETER (MINUTE = 30)
      PARAMETER (SECOND = 0)

      INTEGER UNIXTIME
      PARAMETER (UNIXTIME = 0) ! Generation time of data.

      INTEGER EXPIRY
      PARAMETER (EXPIRY = 3600) ! How long data are valid, seconds.

      INTEGER NUMFIELDS
      PARAMETER (NUMFIELDS=3) ! Number of fields we are writing.

      INTEGER ENCODING_TYPE

      INTEGER ARRAYSIZE
      PARAMETER (ARRAYSIZE = NX * NY * NZ * NUMFIELDS) 

      REAL FIELD_DATA
      DIMENSION FIELD_DATA( ARRAYSIZE ) ! Actual data to write.

      INTEGER FORECASTLEADTIME
      INTEGER WRITEASFORECAST
      INTEGER WENTOK

      INTEGER IZ, IY, IX, INDEX, IFIELD

C
C Set up the strings, terminated with a carat.
C
      WRITE (FIELDNAMES, 90) 'U^V^Temp^'
      WRITE (URL, 91) 'mdvp:://localhost::./mdv/non_fcst^'
      WRITE (UNITS, 92) 'm/s^m/s^Celcius^'

 90   FORMAT (A9)
 91   FORMAT (A34)
 92   FORMAT (A16)

      ENCODING_TYPE = 2 ! Store as scaled 16 bit integers.

C
C Set up the data. Of the five planes, the lowest and
C highest are set to missing. The others are set to the plane number
C multiplied by the field number.
C
      DO IFIELD = 1, NUMFIELDS
         DO IZ = 1, NZ
            DO IY = 1, NY
               DO IX = 1,NX

                  INDEX = (IFIELD-1) *NX*NY*NZ + 
     1                     (IZ-1)*NX*NY + (IY-1)*NX + (IX-1) + 1
C     
                  IF ((IZ . EQ. 1) .OR. (IZ .EQ. NZ)) THEN
                     FIELD_DATA(INDEX) = BADVAL
                  ELSE 
                     FIELD_DATA(INDEX) = IZ * IFIELD
                  END IF
C     
               END DO
            END DO
         END DO
      END DO

C
C Set up the heights of the planes. For the vlevel type we are
C using, these are in Km above sea level.
C
      DO IZ = 1, NZ
         VLEVELS(IZ) = 0.5 + 0.25*(IZ-1)
      END DO

C
C Write as non-forecat (ie. measured) data.
C
      FORECASTLEADTIME = 0
      WRITEASFORECAST = 0

      CALL PRODUCE_MDV( FIELDNAMES, URL, UNITS, DEBUG, NX, 
     1 NY, NZ, DX, DY, MINX, MINY,
     2 ORIGIN_LAT, ORIGIN_LON, VLEVELTYPE,
     3 VLEVELS, GRIDTYPE, ROTATION, LAMBERTLAT1, LAMBERTLAT2, BADVAL,
     4 YEAR, MONTH, DAY, HOUR, MINUTE, SECOND, UNIXTIME,
     5 EXPIRY, FIELD_DATA, FORECASTLEADTIME, WRITEASFORECAST,
     6 NUMFIELDS, ENCODING_TYPE, WENTOK)

      IF (WENTOK .NE. 1) THEN
         STOP 'Failed on first write'
      END IF

C
C Now write the same thing to a different URL. Pretend
C it is forecast data with a lead time of one hour (3600 seconds).
C Also increase the precision to FLOAT32.

       FORECASTLEADTIME = 3600
       WRITEASFORECAST = 1
       ENCODING_TYPE = 3

       WRITE (URL, 91) 'mdvp:://localhost::./mdv/pls_fcst^'

      CALL PRODUCE_MDV( FIELDNAMES, URL, UNITS, DEBUG, NX, 
     1 NY, NZ, DX, DY, MINX, MINY,
     2 ORIGIN_LAT, ORIGIN_LON, VLEVELTYPE,
     3 VLEVELS, GRIDTYPE, ROTATION, LAMBERTLAT1, LAMBERTLAT2, BADVAL,
     4 YEAR, MONTH, DAY, HOUR, MINUTE, SECOND, UNIXTIME,
     5 EXPIRY, FIELD_DATA, FORECASTLEADTIME, WRITEASFORECAST,
     6 NUMFIELDS, ENCODING_TYPE, WENTOK)

      IF (WENTOK .NE. 1) THEN
         STOP 'Failed on second write'
      END IF

      STOP
      END




