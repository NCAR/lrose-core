Attribute VB_Name = "Module2"
'*COMMON/SPHDAT/S1, S2, S3, A1, A2, A3, IER

    Dim S1 As Double, S2 As Double, S3 As Double, A1 As Double, A2 As Double, A3 As Double
   

      Sub MESOL(Myr, Md, Tgmt, Dglat, Dglon, ZEN, azi, arms)

' This is lifted from the original ERL program. Converted to VB
' J. Lutz 2/16/98


'        CALCULATE SUN'S POSITION BY THE METHOD PRESENTED
'        BY JEAN MEEUS IN ASTRONOMICAL FORMULAE FOR CALCULATORS
'        CHAPTERS--
'          18  SOLAR COORDINATES
'           7  SIDEREAL TIME
'          15  NUTATION
'          21  EQUATION OF TIME
'        ARE USED
'      --INPUT--
'        MYR   : 2 DIGIT YEAR
'        MD    : DAY OF YEAR
'        TGMT  : TIME GMT OF OBSERVATION
'        DGLAT : LATITUDE OF STATION (DEGREES)
'        DGLON : LONGITUDE OF STATION (DEGREES)
'      --OUTPUT--
' ZEN: ZENITH DISTANCE(DEGREES)
' AZI: ZAIMUTH (DEGREES)


' DEGREES - -RADIANS
'*      DATA DTR, RTD/.017453292519943, 57.295779513082/
'*      DATA PI, PI2, PIO2, PI32/3.1415926535898, 6.2831853071796,
'*     1  1.5707963267949, 4.7123889803847/
Const Dtr As Double = 0.017453292519943, Rtd As Double = 57.295779513082, Pi As Double = 3.1415926535898
Const Pi2 As Double = 2# * Pi, Pio2 As Double = Pi / 2#, Pi32 As Double = Pi * (3# / 2#)
Dim It As Long
Dim Ourlon As Double, Tloc As Double, T As Double, Tp As Double, Obl As Double, Ecs As Double, Amb As Double, Ambd1 As Double, Ambda As Double, Anom As Double, Ann As Double, Annom As Double, Amr As Double
Dim Anr As Double, Omeg As Double, Omega As Double, Sinom As Double, Sinm As Double, S2l As Double, C2l As Double, Snm As Double, Cent As Double
Dim Solong As Double, Sid0 As Double, Sid As Double, Sidloc As Double, Av As Double, Bv As Double, Cj As Double, Dl As Double, Ec As Double, Aav As Double, Bbv As Double, Ccj As Double, Ddl As Double, Eec As Double, Corlo As Double
Dim Solap As Double, Sidap As Double, Oblap As Double, Solr As Double, Oblr As Double, Rdec As Double, Ra As Double, Dec As Double, Sha As Double, Rsha As Double
Dim Alte As Double


' HOURS - -RADIANS
'*      DATA HTR, RTH/.261799387799, 3.8197186342/
    Const Htr As Double = 0.261799387799, Rth As Double = 3.8197186342
      
      
'        LONGITUDE IN HOURS
      S3 = (90# - Dglat) * Dtr
      Ourlon = Dglon / 15#
'        LOCAL TRUE TIME
      Tloc = Tgmt - Ourlon
'*      IF(TLOC .GE. 24.) TLOC = TLOC - 24.
'*      IF(TLOC .LT. 0.) TLOC = TLOC + 24.

    If (Tloc >= 24) Then Tloc = Tloc - 24#
    If (Tloc < 0) Then Tloc = Tloc + 24#
    





'******FOLLOWING MOD BY E. DUTTON 8NOV1988
'****** ACCOUNT FOR YEARS AFTER 1999
'*      IF(MYR.GT.99) MYR = MYR - 1900
    If (Myr > 99) Then Myr = Myr - 1900



'****** AFTER 1999 MUST ENTER FOUR DIGIT YEAR

      It = 365.25 * (Myr - 1) + 365 + Md
'        TIME IN CENTURIES FROM 1900 JANUARY 0.5 OF GREENWITCH
'        MIDNIGHT OF THE DAY OF THE OBSERVATION
      T = (It - 0.5) / 36525
'        TIME OF OBSERVATION IN CENTURIES
      Tp = T + Tgmt / 876600#
'        THE TIME DEPENDENT TERMS HAVE 2 FORMS GIVEN
'        THE LONG FORM IS THAT GIVEN BY MEEUS (USUALLY TAKEN FROM
'        THE AENA/AA)
'        THE SHORT FORM IS A LINEAR FIT AT 1985.0 (OR WAS IT 1982.0?)
'        OBLIQUITY OF THE ECLIPTIC
'CC OBL = 23.452294 - 0.0130135 * TP
      Obl = 23.452294 - (0.0130125 + (0.00000164 - 0.000000503 * Tp) * Tp) * Tp
'        ECCENTRICITY OF EARTH'S ORBIT
'CC ECS = 0.01675104 - 0.0000419 * TP
      Ecs = 0.01675104 - (0.0000418 + 0.000000126 * Tp) * Tp
'        MEAN GECOENTRIC LONGITUDE OF THE SUN
'CC AMB = 0.76917 * TP
      Amb = (0.76892 + 0.0003025 * Tp) * Tp
      Ambd1 = 279.69668 + Amb + 36000# * Tp
      Ambda = Amod(Ambd1, 360#)
'        MEAN ANNOMALLY OF THE SUN
'CC ANOM = -0.95037 * TP
      Anom = -(0.95025 + (0.00015 + 0.0000033 * Tp) * Tp) * Tp
      Ann = -1.52417 + Anom + 36000# * Tp
      Annom = Amod(Ann, 360#)
      Amr = Ambda * Dtr
      Anr = Annom * Dtr
'        LONGITUDE OF MOON'S ASCENDING NODE
'        MAKE THE SOLUTION POSITIVE FOR THE NEXT FEW CENTURIES
'CC OMEG = 2419.18 - 1934.14 * TP
      Omeg = 2419.1833 - (1934.142 - 0.002078 * Tp) * Tp
      Omeg = Amod(Omeg, 360#)
      Omega = Omeg * Dtr
'        A NUTATION TERM
      Sinom = 0.00479 * Sin(Omega)
      Sinm = Sin(Anr)
      S2l = Sin(2# * Amr)
      C2l = Cos(2# * Amr)
      Snm = Sin(Anr)
'        MORE NUTATION
      Sinom = Sinom + 0.000354 * S2l
'        EQUATION OF THE CENTER -- A SPECIALIZED FORM OF
' KEPPLER       'S EQUATION FOR THE SUN
'CC   CENT = (-.001172 * SINM ** 2 + (1.920339 - .0048 * TP)) * SINM
'CC 1 + 0.020012 * Sin(2# * ANR)
'*      CENT = (-.001172 * SINM ** 2 + (1.920339 -
 '*      (4.789E-3 + 1.4E-5 * TP) * TP)) * SINM +  (.020094 - 1.E-4 * TP) * SIN(2. * ANR)

      Cent = (-0.001172 * Sinm * Sinm + (1.920339 - _
      (0.004789 + 0.000014 * Tp) * Tp)) * Sinm + (0.020094 - 0.0001 * Tp) * Sin(2# * Anr)



' CENTER = CENT * DTR
      Solong = Ambda + Cent
'        SIDEREAL TIME
      Sid0 = (0.00002581 * T + 0.051262) * T + 6.6460656 + 2400# * T
      Sid = Amod(Sid0, 24#)
      Sidloc = Sid + 0.002737909 * Tgmt + Tloc
'*      IF(SIDLOC .GE.24.)SIDLOC = SIDLOC - 24.
    If (Sidloc >= 24) Then Sidloc = Sidloc - 24
'        THE MAJOR PLANETARY PERTURBATIONS ON THE EARTH
      Av = 153.23 + 22518.7541 * Tp
      Bv = 216.57 + 45037.5082 * Tp
      Cj = 312.69 + 32964.3577 * Tp
'CC DL = 267.113 * TP
      Dl = (267.1142 - 0.00144 * Tp) * Tp
      Dl = (Dl + 350.74) + 445000# * Tp
      Ec = 231.19 + 20.2 * Tp
      Aav = 0.00134 * Cos(Av * Dtr)
      Bbv = 0.00154 * Cos(Bv * Dtr)
      Ccj = 0.002 * Cos(Cj * Dtr)
      Ddl = 0.00179 * Sin(Dl * Dtr)
      Eec = 0.00178 * Sin(Ec * Dtr)
      Corlo = Aav + Bbv + Ccj + Ddl + Eec
'        APPARENT SOLAR LONGITUDE AND LOCAL SIDEREAL TIME
      Solap = Solong - 0.00569 - Sinom + Corlo
      Sidap = Sidloc - Sinom * 0.061165
'*      IF(SIDAP .GE. 24.) SIDAP = SIDAP - 24.
'*      IF(SIDAP .LT. 0.) SIDAP = SIDAP + 24.
    If (Sidap >= 24) Then Sidap = Sidap - 24
    If (Sidap < 0) Then Sidap = Sidap + 24
    


'        OBLIQUITY CORRECTED FOR NUTATION
      Oblap = Obl + 0.00256 * Cos(Omega)
      Solr = Solap * Dtr
' SIDR = sidap * Dtr
      Oblr = Oblap * Dtr
'        DECLINATION AND RIGHT ASCENSION
      Rdec = Asin(Sin(Oblr) * Sin(Solr))
      Ra = Atn(Cos(Oblr) * Tan(Solr))
      Ra = Ra * Rtd / 15#
      Dec = Rdec * Rtd
'*      IF(SOLR .GT. PIO2 .AND. SOLR .LT. PI32) RA = RA + 12.
'*      IF(RA .LT. 0) RA = RA + 24.
'*      IF(RA .GE. 24.) RA = RA - 24.

    If (Solr > Pio2 And Solr < Pi32) Then Ra = Ra + 12
    If (Ra < 0) Then Ra = Ra + 24
    If (Ra >= 24) Then Ra = Ra - 24





'        HOUR ANGLE FROM SIDEREAL TIME AND RA
      Sha = Sidap - Ra
'*      IF(SHA .GT. 12.) SHA = SHA - 24.
'*      IF(SHA .LE. -12.) SHA = SHA + 24.
    If (Sha > 12#) Then Sha = Sha - 24
    If (Sha <= -12#) Then Sha = Sha + 24
    
    
      
      
      Rsha = Abs(Sha) * Htr
      S1 = Pio2 - Rdec
      A2 = Rsha
      
      Call SAS
     
      ZEN = S2 * Rtd
      azi = A1
'*      IF(SHA .GT. 0.) AZI = PI2 - A1
    If (Sha > 0) Then azi = Pi2 - A1
    
      azi = azi * Rtd
'*      IF( ZEN.LT.89.99) GO TO 10
    If (ZEN < 89.99) Then GoTo 10
    
      arms = 88.888
'*      Return
Exit Sub

10    Alte = Pi / 2# - Pi * ZEN / 180#
'*      ARMS=1.0/(SIN(ALTE)+.15/(90.-ZEN+3.885)**1.253)
       arms = 1# / (Sin(Alte) + 0.15 / (90# - ZEN + 3.885) ^ 1.253)


'*      Return
Exit Sub

      End Sub
      
      
      Private Sub SAS()
'*      COMMON/SPHDAT/S1, S2, S3, A1, A2, A3, IER
'*      EQUIVALENCE (HPI, SMN)
'*      DATA PI, HPI, SMX/3.1415926535898,
 '*    1  1.5707963267949, 4.7123889803847/
 
    Const Hpi As Double = 1.5707963267949, Smx As Double = 4.7123889803847
    Dim Tnha2 As Double, Hds13 As Double, Hss13 As Double, Tnhda13 As Double, Tnhsa13 As Double, Hda13 As Double, Hsa13 As Double
    Dim Sns2 As Double, Css2 As Double
     Const Pi As Double = 3.1415926535898
    Dim I As Integer
      
      
      
      Tnha2 = 1# / Tan(0.5 * A2)
      Hds13 = 0.5 * (S1 - S3)
      Hss13 = 0.5 * (S1 + S3)
      Tnhda13 = Tnha2 * Sin(Hds13) / Sin(Hss13)
      Tnhsa13 = Tnha2 * Cos(Hds13) / Cos(Hss13)
      Hda13 = Atn(Tnhda13)
      Hsa13 = Atn(Tnhsa13)
'*      IF(HSA13 .LT. 0) HSA13 = HSA13 + PI
    If (Hsa13 < 0) Then Hsa13 = Hsa13 + Pi
    
      A1 = Hsa13 + Hda13
      A3 = Hsa13 - Hda13
      I = 1
'*      IF(ABS(HPI - A3) .LT. ABS(HPI - A1))I = 3
'*      IF(I .EQ. 1) SNS2 = SIN(A2) * SIN(S1) / SIN(A1)
'*      IF(I .EQ. 3) SNS2 = SIN(A2) * SIN(S3) / SIN(A3)

        If (Abs(Hpi - A3) < Abs(Hpi - A1)) Then I = 3
      If (I = 1) Then Sns2 = Sin(A2) * Sin(S1) / Sin(A1)
      If (I = 3) Then Sns2 = Sin(A2) * Sin(S3) / Sin(A3)


      Css2 = Cos(S1) * Cos(S3) + Sin(S1) * Sin(S3) * Cos(A2)
'*      IF(SNS2 .GT. .71) GO TO 10
    If (Sns2 > 71) Then GoTo 10
    
      S2 = Asin(Sns2)
'*      IF(CSS2 .LT. 0) S2 = PI - S2
    If (Css2 < 0) Then S2 = Pi - S2
    
      GoTo 20
10:
      S2 = Acos(Css2)
20:
 '*     Return
 Exit Sub
      End Sub
      
Private Function Amod(a, b)
If (b = 0) Then
Amod = 0#
Exit Function
End If
Amod = a - (b * Fix(a / b))
End Function

Private Function Asin(x)
Asin = Atn(x / Sqr(-x * x + 1))
End Function
Private Function Acos(x)
Acos = Atn(-x / Sqr(-x * x + 1)) + 2 * Atn(1)
End Function
