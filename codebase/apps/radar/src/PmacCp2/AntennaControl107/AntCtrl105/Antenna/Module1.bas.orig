Attribute VB_Name = "Module1"
'Fixed angle Lists
Public FixedAng(0 To 32, 1 To 10) As Single

'Pmac "P" addresses for fixed angle lists
'addresses must match those defined in spoldefm.h
Public Const Afl1 = 640
Public Const Afl2 = 673
Public Const Afl3 = 706
Public Const Afl4 = 739
Public Const Afl5 = 772
Public Const Afl6 = 805
Public Const Afl7 = 838
Public Const Afl8 = 871
Public Const Afl9 = 904
Public Const Afl10 = 937

'Volume Definition Offsets
Public Const STYPE = 1, Azr = 3, Azl = 4, RATE = 7, FAPOINT = 15
Public Const SAMPLES = 10, ELT = 6, ELB = 5
Public Const SModulo = 2          'for sync volume

'Sync volume definitions

Public SynDef(0 To 2, 1 To 7) As Single

'addresses must match those defined in spoldefm.h
'one for each sequence
Public Const Sync1 = 970
Public Const Sync2 = 973
Public Const Sync3 = 976
Public Const Sync4 = 979
Public Const Sync5 = 982
Public Const Sync6 = 985
Public Const Sync7 = 988


'PPI Definitions
Public PpiDef(0 To 15, 1 To 4) As Single
'addresses must match those defined in spoldefm.h
Public Const Ppi1 = 400
Public Const Ppi2 = 416
Public Const Ppi3 = 432
Public Const Ppi4 = 448

'RHI Definitions
Public RhiDef(0 To 15, 1 To 4) As Single
'addresses must match those defined in spoldefm.h
Public Const Rhi1 = 464
Public Const Rhi2 = 480
Public Const Rhi3 = 496

'Sur Definitions
Public SurDef(0 To 15, 1 To 4) As Single
'addresses must match those defined in spoldefm.h
Public Const Sur1 = 512
Public Const Sur2 = 528
Public Const Sur3 = 544

'Seq Definitions
Public SeqDef(0 To 10, 1 To 8) As Single
'addresses must match those defined in spoldefm.h
Public Const Seq1 = 560
Public Const Seq2 = 570
Public Const Seq3 = 580
Public Const Seq4 = 590
Public Const Seq5 = 600
Public Const Seq6 = 610
Public Const Seq7 = 620
Public Const Seq10 = 630
'the above address only is hardwired into menuppi1,menuppi2 etc.

'Special PMAC locations
Public Const Clockrate = 57   'p address pmac clock counts/sec
Public Const RealorFake = 49  'p address =1234 for simulated angles, otherwise real angles
Public Const Running = 181    'p address for running flag in PMAC
Public Const ScanID = 180     'p address for Scan ID location in PMAC

'Control Parameters
 Public FakeAngles As Boolean
 Public Const ISAPMAC = True
 
 Public PMACIoBusy As Integer
 
 Public Const Debugum = False
 Public Const ImaDow = 3          '=1 SPOL =2 Dow2 =3 Dow3 =4 Xpol
 Public Const SpolMaxr = 15
 Public Const Dow2Maxr = 50       'dow 2 max rate =50
 Public Const Dow3Maxr = 50       'Dow3 max rate=50
 Public Const Dow4Maxr = 25       'Xpol max rate =25
 Public PMACPORT As Object
 Public Const PComPort = 1       'this is the port the PMAC is connected to
 Public WKFILE As String
 Public ISRUNNING As Boolean
 Public AzStow As Single
 Public ElStow As Single
 Public Counter As Single
 Public AntMalf As Boolean
 Public MalfEnb As Boolean
 Public Flip As Boolean
 Public NTimes As Single
 Public Etime As Single
  Public FSF As Boolean
  Public ELO As Single
  Public Hobbs As Single
  
  
  Public Const Twice = False      'true to send data to PMAC twice, false saves time
 
 
 
 
 
 

Declare Sub Sleep Lib "kernel32" (ByVal dwmilleseconds As Long)
Public Sub SDelay(D)
'Delay for about D sec, fractions to .001 OK
Dim J As Long
J = D * 1000
Sleep (J)
End Sub


'Get the value of a PMAC P variable
'ppoint= pointer to P vatiable
'v=returned value

Public Sub GetPMACPVal(ppoint, V, ier)
Dim Instring As String * 30
Dim Timestart As Double
Dim Ntries, zzz As Integer
Ntries = 0
If (1 > 2) Then
Timestart = Timer
Do While PMACIoBusy > 0 'wait for busy to go down
If (Timer - Timestart) > 0.25 Then GoTo 50
zzz = PMACIoBusy
DoEvents
Call SDelay(0.1) 'let the system get in, This prevents a hang.
Loop

50 PMACIoBusy = 10 'lock out everybody else

End If

100 While (PMACPORT.InBufferCount <> 0) 'Clear any pending inputs
'DoEvents
Instring = PMACPORT.Input
Wend

Instring = ""
Instring = "P" + Format(ppoint) + Chr(13)
PMACPORT.Output = Instring
Timestart = Timer
buffer$ = ""
Do
'DoEvents
buffer$ = buffer$ & PMACPORT.Input
    If (Timer - Timestart) > 0.5 Then 'give it .5 sec
    ier = 1
    PMACIoBusy = 0
    Exit Sub
    End If

Loop Until InStr(buffer$, Chr(13))

If IsNumeric(buffer$) Then  'It should be a number
V = Val(buffer$)
Else
Ntries = Ntries + 1  'bad character?
If Ntries < 3 Then
GoTo 100            'retry from top?
Else
 ier = 2             'Give up, exit with error
End If

End If

PMACIoBusy = 0

End Sub
Public Sub PutPMACPVal(ppoint, V)
Dim Outstring As String * 30
'Do While PMACIoBusy
'DoEvents
'Call SDelay(0.01) 'let the system get in, This prevents a hang.
'Loop
PMACIoBusy = 20
Outstring = "p" + Format(ppoint) + "=" + Format(V) + Chr(13)
PMACPORT.Output = Outstring
PMACIoBusy = 0
End Sub

'Get the value of a PMAC M variable
'ppoint= pointer to M vatiable
'V=returned value
'Ier=0 OK Ier=1 timeout Ier=2 bad characters



Public Sub GetPMACMVal(ppoint, V, ier)
Dim Instring As String * 30
Dim Timestart As Double
Dim Ntries As Integer
Ntries = 0
If (1 > 2) Then
Do While PMACIoBusy > 0 'wait for busy to go down
DoEvents
Call SDelay(0.01) 'let the system get in, This prevents a hang.
Loop

PMACIoBusy = 30 'lock out somebody else
End If

100 While (PMACPORT.InBufferCount <> 0) 'Clear any pending inputs
'DoEvents
Instring = PMACPORT.Input
Wend

Instring = ""

Instring = "M" + Format(ppoint) + Chr(13)
PMACPORT.Output = Instring
Timestart = Timer
buffer$ = ""
Do
'DoEvents
buffer$ = buffer$ & PMACPORT.Input
If (Timer - Timestart) > 0.5 Then  'give it .5 sec
ier = 1
PMACIoBusy = 0
Return
End If

Loop Until InStr(buffer$, Chr(13))

If IsNumeric(buffer$) Then  'It should be a number
V = Val(buffer$)
Else
Ntries = Ntries + 1  'bad character?
If Ntries < 3 Then
GoTo 100            'retry from top?
Else
 ier = 2             'Give up, exit with error
End If

End If


PMACIoBusy = 0
End Sub


