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
Public ISAPMAC As Boolean
 
Public PMACIoBusy As Integer
Public PMACBlocked As Boolean       '20071209kg Added - used with the above to indicate semaphore block
Public PMACBlockedAck As Boolean    'ditto - Indicates that user has ack'd the blockage
 
Public Const Debugum = True         '20071209kg set true
 
Public Const ImaDow = 1             '=1 SPOL(or CP2), =2 Dow2, =3 Dow3, =4 Xpol
Public Const SpolMaxr = 15
Public Const Dow2Maxr = 50          'dow 2 max rate =50
Public Const Dow3Maxr = 50          'Dow3 max rate=50
Public Const Dow4Maxr = 25          'Xpol max rate =25

Public PMACPORT As Object
Public Const PComPort = 1           'this is the port the PMAC is connected to
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

Public Function SemTakePMAC(busyVal, Optional waitMs As Integer = 20, Optional FORCE As Boolean = False) As Boolean

Dim waitCount As Integer
Dim result As VbMsgBoxResult
Static blocktime, btnow As Single
Static lastBusyVal As Integer

waitCount = 0

SemTakePMAC = Not PMACBlocked
If busyVal = 0 Then Exit Function   ' Skip Semaphore if '0' passed

If PMACBlocked Then
    If FORCE Then       ' We can try and force clear a blockage
        PMACPORT.OutBufferCount = 0
        PMACPORT.Output = Chr(13)   'send a <CR>
        Sleep (500) ' add a 1/2 second wait then
        PMACPORT.InBufferCount = 0  'Flush the in buffer
    ElseIf PMACIoBusy <> 0 Then
        If Not PMACBlockedAck Or busyVal <> lastBusyVal Then
            If MsgBox("PMAC is Blocked - try RESET PMAC", vbExclamation) = vbOK Then
                PMACBlockedAck = True
                blocktime = Timer
            End If
            lastBusyVal = busyVal
        Else
            btnow = Timer
            If btnow < blocktime Then btnow = btnow + 24 * 3600
            If Timer > blocktime + 20 Then pmacblockack = False
        End If
        Exit Function   ' We exit still blocked - user must reset PMAC
    End If
    PMACIoBusy = 0
    PMACBlocked = False
    PMACBlockedAck = False
End If

SemTakePMAC = True
Do While PMACIoBusy And PMACIoBusy <> busyVal
    DoEvents
    Sleep (waitMs) 'let the system get in, This prevents a hang.
    waitCount = waitCount + 1
    ' Detect Blocked if held up for > 5 secs
    If waitCount > (5000 / waitMs) Then
        SemTakePMAC = False
        PMACBlocked = True
        DoEvents
        If Not FORCE Then
            result = MsgBox("PMAC access is BLOCKED (" + Format(busyVal) + ")", vbAbortRetryIgnore + vbExclamation + vbDefaultButton3)
            Select Case result
              Case vbAbort
                End
              Case vbRetry
                PMACBlocked = False
                PMACBlockedAck = False
                Exit Function
              Case vbIgnore
            End Select
        End If
        SemTakePMAC = True
        PMACIoBusy = 0
        PMACBlocked = False
        PMACBlockedAck = False
    End If
Loop

PMACIoBusy = busyVal  'lock out everybody else
lastBusyVal = busyVal

End Function

Public Function SemGivePMAC(busyVal, Optional FORCE As Boolean = False) As Boolean

Dim result As VbMsgBoxResult

SemGivePMAC = True
If busyVal = 0 Then Exit Function   ' Skip Semaphore if '0' passed

If busyVal <> PMACIoBusy Then
    SemGivePMAC = False
    If Not FORCE Then
        result = MsgBox("DEBUG: Out of order SemGivePMAC (" + Format(busyVal) + "<>" + Format(PMACIoBusy) + ")", vbAbortRetryIgnore + vbExclamation + vbDefaultButton3)
        Select Case result
          Case vbAbort
            End
          Case vbRetry
            Sleep (1000)
            Exit Function
          Case vbIgnore
        End Select
    SemGivePMAC = True
    End If
End If
PMACBlocked = False
PMACBlockedAck = False
PMACIoBusy = 0 'Clear the lock out

End Function

'Get the value of a PMAC P variable
'ppoint= pointer to P vatiable
'UseSem = Optional Semaphore value to use: 0 = none (ie skip semaphore)
'value is returned by the function

Public Function GetPMACPVal(ppoint, ier, Optional UseSem As Integer = 0)
Dim Instring As String * 30
'Dim Timestart As Double
Dim Ntries, zzz As Integer
Ntries = 0
Dim waitCount As Integer

If SemTakePMAC(UseSem) = False Then Exit Function

100 PMACPORT.InBufferCount = 0  'Flush the in buffer
' While (PMACPORT.InBufferCount <> 0) 'Clear any pending inputs
'    DoEvents
'    Instring = PMACPORT.Input
'Wend

Instring = ""
Instring = "P" + Format(ppoint) + Chr(13)
PMACPORT.Output = Instring
'Timestart = Timer
buffer$ = ""
waitCount = 0
Do
    '20071206kg New Wait Method
    While (PMACPORT.InBufferCount = 0)
        DoEvents
        Sleep (20)
        waitCount = waitCount + 1
        If waitCount > (0.5 / 0.02) Then   'wait .5 sec
            ier = 1
            SemGivePMAC (UseSem)
            Exit Function
        End If
    Wend
    buffer$ = buffer$ & PMACPORT.Input
Loop Until InStr(buffer$, Chr(13))

If IsNumeric(buffer$) Then  'It should be a number
    GetPMACPVal = Val(buffer$)
Else
    Ntries = Ntries + 1  'bad character?
    If Ntries < 3 Then
        GoTo 100            'retry from top?
    Else
        ier = 2             'Give up, exit with error
    End If

End If

SemGivePMAC (UseSem)

End Function

Public Function PutPMACPVal(ppoint, V, Optional UseSem As Integer = 0) As Boolean

Dim Outstring As String * 30

PutPMACVal = False
If SemTakePMAC(UseSem) = False Then Exit Function

Outstring = "p" + Format(ppoint) + "=" + Format(V) + Chr(13)
PMACPORT.Output = Outstring

PutPMACVal = SemGivePMAC(UseSem)

End Function

'Get the value of a PMAC M variable
'ppoint= pointer to M vatiable
'V=returned value
'Ier=0 OK Ier=1 timeout Ier=2 bad characters
'UseSem = Optional Semaphore value to use: 0 = none (ie skip semaphore)

Public Function GetPMACMVal(ppoint, ier, Optional UseSem As Integer = 0)
Dim Instring As String * 30
'Dim Timestart As Double
Dim Ntries As Integer
Dim waitCount As Integer

Ntries = 0

If SemTakePMAC(UseSem) = False Then Exit Function

100 PMACPORT.InBufferCount = 0  'Flush the in buffer
'100 While (PMACPORT.InBufferCount <> 0) 'Clear any pending inputs
'    DoEvents
'    Instring = PMACPORT.Input
'Wend

Instring = ""

Instring = "M" + Format(ppoint) + Chr(13)
PMACPORT.Output = Instring
'Timestart = Timer
buffer$ = ""
waitCount = 0
Do
    '20071206kg New Wait Method
    While (PMACPORT.InBufferCount = 0)
        DoEvents
        Sleep (20)
        waitCount = waitCount + 1
        If waitCount > (0.5 / 0.02) Then   'wait .5 sec
            ier = 1
            SemGivePMAC (UseSem)
            Exit Function
        End If
    Wend
    buffer$ = buffer$ & PMACPORT.Input
Loop Until InStr(buffer$, Chr(13))

If IsNumeric(buffer$) Then  'It should be a number
    GetPMACMVal = Val(buffer$)
Else
    Ntries = Ntries + 1  'bad character?
    If Ntries < 3 Then
        GoTo 100            'retry from top?
    Else
        ier = 2             'Give up, exit with error
    End If
End If

SemGivePMAC (UseSem)

End Function

