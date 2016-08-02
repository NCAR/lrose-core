C
C Program to read and print netCDF lidar data.
C Niles Oien, oien@ucar.edu, February 2004.
C
C
      PROGRAM readWindtracer

      IMPLICIT NONE
C
C Declare some arrays and variables.
C
      INTEGER ITIME
      INTEGER IGATE
      INTEGER INDEX

      INTEGER NUMTIMES
      INTEGER MAXNUMTIMES
      PARAMETER (MAXNUMTIMES = 700)

      INTEGER NUMGATES
      INTEGER MAXNUMGATES
      PARAMETER (MAXNUMGATES = 150)

      REAL AZ
      DIMENSION AZ(MAXNUMTIMES)

      REAL EL
      DIMENSION EL(MAXNUMTIMES)

      REAL YEAR, MONTH, DAY, HOUR, MIN, SEC, MSEC
      DIMENSION YEAR(MAXNUMTIMES)
      DIMENSION MONTH(MAXNUMTIMES)
      DIMENSION DAY(MAXNUMTIMES)
      DIMENSION HOUR(MAXNUMTIMES)
      DIMENSION MIN(MAXNUMTIMES)
      DIMENSION SEC(MAXNUMTIMES)
      DIMENSION MSEC(MAXNUMTIMES)

      REAL VEL
      DIMENSION VEL(MAXNUMTIMES*MAXNUMGATES)

      REAL BACKSCAT
      DIMENSION BACKSCAT(MAXNUMTIMES*MAXNUMGATES)

      INTEGER OK
C
C Set debug to 1 to get output from the C subroutines.
C
      INTEGER DEBUG
      PARAMETER (DEBUG = 0)

      REAL FIRST_RANGE
      REAL DELTA_RANGE
      REAL R_NGATES
      REAL R_NTIMES

C
C Procedural part of program starts. Open the netcdf file.
C
      CALL openNetCDF('20020722_231017_base.prd.nc', OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to open file.'
      END IF
C
C Read some of the attributes - start range, range spacing,
C number of gates, number of times. Exit if we have too many
C elements to cope with.
C
      CALL getNetCdfAtt('firstRange', FIRST_RANGE, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get firstRange attribute.'
      END IF

      CALL getNetCdfAtt('deltaRange', DELTA_RANGE, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get deltaRange attribute.'
      END IF

      CALL getNetCdfAtt('nTimes', R_NTIMES, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get nTimes attribute.'
      END IF
      NUMTIMES = R_NTIMES

      CALL getNetCdfAtt('nGates', R_NGATES, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get nGates attribute.'
      END IF
      NUMGATES = R_NGATES


      PRINT *, 'firstRange = ', FIRST_RANGE
      PRINT *, 'deltaRange = ', DELTA_RANGE
      PRINT *, NUMGATES, ' Gates.'
      PRINT *, NUMTIMES, ' Times.'

      IF (NUMTIMES .GT. MAXNUMTIMES) THEN
         STOP 'Too many times - increase MAXNUMTIMES and recompile.'
      END IF

      IF (NUMGATES .GT. MAXNUMGATES) THEN
         STOP 'Too many gates - increase MAXNUMGATES and recompile.'
      END IF

C
C Read the elevation and azimuth one dimensional arrays.
C Also read the year, month, day, hour, min, sec and msec
C arrays.
C
      CALL getNetCdfVar('azimuth', AZ, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get azimuth variable.'
      END IF

      CALL getNetCdfVar('elevation', EL, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get elevation variable.'
      END IF

      CALL getNetCdfVar('year', YEAR, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get year variable.'
      END IF

      CALL getNetCdfVar('month', MONTH, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get month variable.'
      END IF

      CALL getNetCdfVar('day', DAY, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get day variable.'
      END IF

      CALL getNetCdfVar('hour', HOUR, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get hour variable.'
      END IF

      CALL getNetCdfVar('min', MIN, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get min variable.'
      END IF

      CALL getNetCdfVar('sec', SEC, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get sec variable.'
      END IF

      CALL getNetCdfVar('msec', MSEC, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get msec variable.'
      END IF

C
C Read the two dimensional velocity variable.
C Other fields may include SNR, CFAR, SW,
C BACKSCAT, QUALITY, MCFAR, FVEL and FSNR.
C
C The command 'ncdump -h <filename>' can be used
C to determine what fields are available in a file.
C
      CALL getNetCdfVar('VEL', VEL, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get velocity variable.'
      END IF
C
C Read backscatter as well.
C
      CALL getNetCdfVar('BACKSCAT', BACKSCAT, OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to get backscatter variable.'
      END IF
C
C Close the netcdf file.
C 
      CALL closeNetCDF(OK, DEBUG)
      IF (OK .NE. 1) THEN
         STOP 'Failed to close file.'
      END IF
C
C Print these data out.
C
      DO ITIME = 1, NUMTIMES
         PRINT *,  'Time number ', ITIME
         PRINT *, ' Year, mon, day, hour, min, sec, millisec : '
         PRINT *,  YEAR(ITIME), MONTH(ITIME), DAY(ITIME),
     1             HOUR(ITIME), MIN(ITIME), SEC(ITIME), MSEC(ITIME)
         PRINT *,  ' AZ = ', AZ(ITIME)
         PRINT *,  ' EL = ', EL(ITIME)
C
         DO IGATE =1, NUMGATES
            INDEX = (ITIME-1)*NUMGATES + IGATE
            PRINT *,"Velocity at gate ", IGATE,
     1       " is ", VEL(INDEX), " backscatter is ",
     2       BACKSCAT(INDEX)
         END DO
      END DO
C
C All done.
C
      STOP 'Normal termination.'

      END
