VERSION 5.00
Begin VB.Form frmSolar 
   BackColor       =   &H0000FFFF&
   Caption         =   "Be Careful"
   ClientHeight    =   6090
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   9450
   LinkTopic       =   "Form1"
   ScaleHeight     =   6090
   ScaleWidth      =   9450
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdFSun 
      BackColor       =   &H00FF00FF&
      Caption         =   "Fast Sun Finder"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   5880
      Style           =   1  'Graphical
      TabIndex        =   33
      Top             =   4080
      Width           =   2775
   End
   Begin VB.CommandButton CmdFlip 
      BackColor       =   &H00FF80FF&
      Caption         =   "Flip"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   3000
      Style           =   1  'Graphical
      TabIndex        =   32
      Top             =   4080
      Width           =   1215
   End
   Begin VB.CommandButton CmdRunOff 
      BackColor       =   &H000000C0&
      Caption         =   "Solar Offset"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   18
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   855
      Left            =   480
      Style           =   1  'Graphical
      TabIndex        =   31
      Top             =   5040
      Width           =   1935
   End
   Begin VB.CommandButton cmdExpand 
      BackColor       =   &H00FFFF00&
      Caption         =   "Az Expand"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   975
      Left            =   3000
      Style           =   1  'Graphical
      TabIndex        =   30
      Top             =   4920
      Width           =   1215
   End
   Begin VB.TextBox txtAzOff 
      Height          =   375
      Left            =   6840
      Locked          =   -1  'True
      TabIndex        =   29
      Text            =   "Text1"
      Top             =   5040
      Width           =   615
   End
   Begin VB.TextBox txtElOff 
      Height          =   375
      Left            =   7680
      Locked          =   -1  'True
      TabIndex        =   28
      Text            =   "Text1"
      Top             =   3360
      Width           =   735
   End
   Begin VB.TextBox txtHits 
      Height          =   375
      Left            =   7680
      Locked          =   -1  'True
      TabIndex        =   27
      Text            =   "Text2"
      Top             =   1800
      Width           =   735
   End
   Begin VB.CommandButton cmdHits 
      BackColor       =   &H000080FF&
      Caption         =   "Hits"
      Height          =   375
      Left            =   7560
      Style           =   1  'Graphical
      TabIndex        =   26
      Top             =   1320
      Width           =   855
   End
   Begin VB.CommandButton CmdOuttaHere 
      BackColor       =   &H0000FF00&
      Caption         =   "Exit Solar"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   4800
      Style           =   1  'Graphical
      TabIndex        =   23
      Top             =   4680
      Width           =   975
   End
   Begin VB.VScrollBar ElScroll 
      Height          =   3375
      Left            =   8760
      Max             =   20
      Min             =   -20
      MousePointer    =   2  'Cross
      TabIndex        =   22
      Top             =   1920
      Width           =   375
   End
   Begin VB.HScrollBar AzScroll 
      Height          =   375
      Left            =   5760
      Max             =   20
      Min             =   -20
      MousePointer    =   2  'Cross
      TabIndex        =   21
      Top             =   5520
      Width           =   3375
   End
   Begin VB.CommandButton CmdRunScan 
      BackColor       =   &H000000FF&
      Caption         =   "Run Calibration Scan"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   855
      Left            =   480
      Style           =   1  'Graphical
      TabIndex        =   20
      Top             =   3960
      Width           =   2175
   End
   Begin VB.TextBox TxtLM 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   480
      Left            =   5400
      Locked          =   -1  'True
      TabIndex        =   19
      Text            =   "Text1"
      Top             =   2040
      Width           =   975
   End
   Begin VB.TextBox TxtLD 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   480
      Left            =   4200
      Locked          =   -1  'True
      TabIndex        =   18
      Text            =   "Text1"
      Top             =   2040
      Width           =   855
   End
   Begin VB.TextBox Txtgmtoff 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   480
      Left            =   4200
      Locked          =   -1  'True
      TabIndex        =   6
      Text            =   "Text1"
      Top             =   720
      Width           =   615
   End
   Begin VB.TextBox TxtLocal 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   450
      Left            =   4200
      Locked          =   -1  'True
      TabIndex        =   5
      Text            =   "Text1"
      Top             =   120
      Width           =   3015
   End
   Begin VB.TextBox TxtGmt 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   480
      Left            =   4200
      Locked          =   -1  'True
      TabIndex        =   4
      Text            =   "Text1"
      Top             =   1320
      Width           =   3015
   End
   Begin VB.TextBox TxtLongDeg 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   480
      Left            =   4200
      Locked          =   -1  'True
      TabIndex        =   3
      Text            =   "Text2"
      Top             =   2760
      Width           =   855
   End
   Begin VB.TextBox TxtLongMIn 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   480
      Left            =   5400
      Locked          =   -1  'True
      TabIndex        =   2
      Text            =   "Text2"
      Top             =   2760
      Width           =   975
   End
   Begin VB.TextBox TxtAz 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   4200
      Locked          =   -1  'True
      TabIndex        =   1
      Text            =   "Text1"
      Top             =   3480
      Width           =   975
   End
   Begin VB.TextBox TxtEl 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   6240
      Locked          =   -1  'True
      TabIndex        =   0
      Text            =   "Text1"
      Top             =   3480
      Width           =   975
   End
   Begin VB.Timer TmrSolar 
      Interval        =   900
      Left            =   8040
      Top             =   120
   End
   Begin VB.Label Label2 
      BackColor       =   &H0000FFFF&
      Caption         =   "Align Azimuth"
      Height          =   255
      Left            =   6480
      TabIndex        =   25
      Top             =   4680
      Width           =   1215
   End
   Begin VB.Label Label1 
      BackColor       =   &H0000FFFF&
      Caption         =   "Align Elevation"
      Height          =   255
      Left            =   7560
      TabIndex        =   24
      Top             =   3120
      Width           =   1095
   End
   Begin VB.Label LblLat 
      BackColor       =   &H0000FFFF&
      Caption         =   "Latitude (Deg,Min)"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   360
      TabIndex        =   17
      Top             =   2040
      Width           =   2775
   End
   Begin VB.Label LblHlat 
      BackColor       =   &H0000FFFF&
      Caption         =   "?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   6720
      TabIndex        =   16
      Top             =   2040
      Width           =   375
   End
   Begin VB.Label LblLocalTime 
      BackColor       =   &H0000FFFF&
      Caption         =   "Local Date and Time"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   360
      TabIndex        =   15
      Top             =   120
      Width           =   3135
   End
   Begin VB.Label LblGmtOff 
      BackColor       =   &H0000FFFF&
      Caption         =   "GMT Difference (Hrs)"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   360
      TabIndex        =   14
      Top             =   720
      Width           =   2895
   End
   Begin VB.Label LblGmtTime 
      BackColor       =   &H0000FFFF&
      Caption         =   "GMT date and Time"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   360
      TabIndex        =   13
      Top             =   1320
      Width           =   2895
   End
   Begin VB.Label LblDate 
      BackColor       =   &H0000FFFF&
      Caption         =   "?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   7440
      TabIndex        =   12
      Top             =   240
      Width           =   375
   End
   Begin VB.Label LblLong 
      BackColor       =   &H0000FFFF&
      Caption         =   "Longitude (Deg,Min)"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   360
      TabIndex        =   11
      Top             =   2760
      Width           =   3135
   End
   Begin VB.Label LblSunLOc 
      BackColor       =   &H0000FFFF&
      Caption         =   "Sun Location ="
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   360
      TabIndex        =   10
      Top             =   3480
      Width           =   2415
   End
   Begin VB.Label LblAz 
      BackColor       =   &H0000FFFF&
      Caption         =   "Az"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   3360
      TabIndex        =   9
      Top             =   3480
      Width           =   615
   End
   Begin VB.Label LblEl 
      BackColor       =   &H0000FFFF&
      Caption         =   "El"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   5400
      TabIndex        =   8
      Top             =   3480
      Width           =   615
   End
   Begin VB.Label LblHlong 
      BackColor       =   &H0000FFFF&
      Caption         =   "?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   6720
      TabIndex        =   7
      Top             =   2760
      Width           =   375
   End
End
Attribute VB_Name = "frmSolar"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
  Dim Tloc, Ourlon, Dglon, Dglat, Myr, It, T, Obl, Ecs, Amb As Double
  Dim GmtOff As Double, LatDeg As Double, LonDeg As Double, LatMin As Double, LonMin As Double
  Dim Temp
  Dim GmtDate
  Dim ElOff As Single, AzOff As Single, Azv As Single, Azw As Single, Hits As Integer
  Dim Pointsun As Boolean
  
  Dim SunPlot As Boolean

  
Private Sub AzScroll_Change()
AzOff = AzScroll.Value / 10#
txtAzOff.Text = AzOff
End Sub

Private Sub cmdExpand_Click()
If cmdExpand.Caption = "Expand Azimuth" Then
If (ISAPMAC) Then
PMACPORT.Output = "disable plc10" + Chr(13)
PMACPORT.Output = "p51=" + Format(Az + AzOff) + Chr(13) 'center of  scan

PMACPORT.Output = "p168=1" + Chr(13)
End If
cmdExpand.Caption = "Normal Azimuth"
Else
If (ISAPMAC) Then
PMACPORT.Output = "p168=0" + Chr(13)
PMACPORT.Output = "enable plc10" + Chr(13)
PMACPORT.Output = "p51=0" + Chr(13)
End If
cmdExpand.Caption = "Expand Azimuth"
End If
End Sub

Private Sub CmdFlip_Click()
If Flip Then
Flip = Not (Flip)
CmdFlip.Caption = "Flip"
Else
Flip = Not (Flip)
CmdFlip.Caption = "Flipped"
End If
End Sub

Private Sub cmdFSun_Click()
frmFSun.Show
End Sub

Private Sub cmdHits_Click()
Dim zzz, Hits
zzz = InputBox("Enter Hits")
If IsNumeric(zzz) Then
Hits = zzz
txtHits.Text = zzz
PMACPORT.Output = "m207=" + Format(Hits) + Chr(13) 'Hits only
End If
End Sub

Private Sub CmdOuttaHere_Click()
If Pointsun Then
MsgBox "Stop sun scan first"
Else
CmdRunScan.Caption = "Pointing Away From The Sun"
sdelay (2)
CmdRunScan.Caption = "Run Calibration Scan"
Unload frmSolar
If (ISAPMAC) Then
PMACPORT.Output = "i5=3" + Chr(13)
sdelay (1)
PMACPORT.Output = "P50=0" + Chr(13)
sdelay (1)
PMACPORT.Output = "P51=0" + Chr(13)
PMACPORT.Output = "P52=0" + Chr(13)
sdelay (1)
End If
frmSpecial.Show
End If
End Sub

Private Sub CmdRunOff_Click()
Pointsun = False


If CmdRunOff.Caption = "Solar Offset" Then
zzz = MsgBox("Do not continue unless a reasonable sunscan is currently running", vbOKCancel, "Extream Danger")
If (zzz = vbCancel) Or (ISRUNNING = False) Then
Exit Sub
End If
    If (ISAPMAC) Then
    
    CmdRunOff.Caption = "Stop Solar Offset"
    CmdRunOff.BackColor = &HC000&
    sdelay (4)
    SunPlot = True
    Pointsun = True
    End If
Else

CmdRunOff.Caption = "Solar Offset"
CmdRunOff.BackColor = &HC0&
Pointsun = False


End If
End Sub

Private Sub CmdRunScan_Click()
SunPlot = False
Pointsun = False

If CmdRunScan.Caption = "Run Calibration Scan" Then


    If ISRUNNING Then
    frmMain.cmdstop.Caption = "Reset  PMAC"
    frmMain.cmdstop.BackColor = &H80FF&
    frmMain.cmdrun.Caption = "Run"
    frmMain.cmdrun.BackColor = &H80FF80
    frmMain.cmdRunSeq.Caption = "Run Sequence"
    frmMain.cmdRunSeq.BackColor = &H80
    ISRUNNING = False
    End If



    If Not ISAPMAC Then
    MsgBox "PMAC connection not enabled"
    End If
    
    If (ISAPMAC) Then
    PMACPORT.Output = "P50=0" + Chr(13)
    sdelay (2)

' az and el center, velocity and width for solar scan
    Azv = 4
    Azw = 4

        If (El <= 60) Then

' az endpoints
        PMACPORT.Output = "p126=" + Format(Azv) + Chr(13) 'Azv
        PMACPORT.Output = "p106=" + Format(Az - Azw) + Chr(13)
        PMACPORT.Output = "p114=" + Format(Az + Azw) + Chr(13)

        PMACPORT.Output = "p50=4"                        'start it
        PMACPORT.Output = "m207=" + Format(Hits) + Chr(13)
        ISRUNNING = True
        Pointsun = True
        End If


End If
CmdRunScan.Caption = "Stop Running Solar Scan"
CmdRunScan.BackColor = &HC000&
Else
'Stop the Solar Scan and Point Away From The Sun
Pointsun = False
CmdRunScan.Caption = "Pointing Away From The Sun"
sdelay (2)
CmdRunScan.Caption = "Run Calibration Scan"
CmdRunScan.BackColor = 255
End If

End Sub



Private Sub Command1_Click()

End Sub

Private Sub ElScroll_Change()
ElOff = -ElScroll.Value / 10#
txtElOff.Text = ElOff
End Sub

Private Sub Form_Load()

Flip = False
frmMain.Hide
frmSpecial.Hide
cmdExpand.Caption = "Expand Azimuth"
Hits = 32
txtHits.Text = Hits
ElScroll.Value = 0
AzScroll.Value = 0
ElOff = -ElScroll.Value / 10#
txtElOff.Text = ElOff
AzOff = AzScroll.Value / 10#
txtAzOff.Text = AzOff
TmrSolar.Enabled = False
On Error GoTo oops
Open "LatLong" For Input As #2
Input #2, LatDeg, LatMin, LonDeg, LonMin, GmtOff
Close #2
Txtgmtoff.Text = GmtOff
TxtLD.Text = LatDeg
TxtLM.Text = LatMin
TxtLongMIn.Text = LonMin
TxtLongDeg.Text = LonDeg
TmrSolar.Enabled = True
Exit Sub
oops:
MsgBox ("LatLong File Open Error")

TxtLM.Text = LatMin
TxtLongMIn.Text = LonMin
TxtLongDeg.Text = LonDeg
Close #1
End Sub


Private Sub LblDate_Click()
MsgBox "Set Date and Time from Windows", vbOK

End Sub

Private Sub LblHlat_Click()
MsgBox "Deg + Decimal Min", vbOK
End Sub

Private Sub LblHlong_Click()
MsgBox "Deg + Decimal Min", vbOK

End Sub



Private Sub TmrSolar_Timer()
Dim Myr As Double, Mmo As Double, Md As Double, Dglat As Double, Dglon As Double
Dim Tgmt As Double
Dim Az As Double, ZEN As Double, El As Double
Dim arms As Double

    

TxtLocal.Text = Now
GmtDate = DateAdd("h", GmtOff, Now)
TxtGmt.Text = GmtDate
Dglat = LatDeg + LatMin / 60#
Dglon = LonDeg + LonMin / 60#
Myr = Year(GmtDate)
Mmo = Month(GmtDate)
Md = Day(GmtDate)
Tgmt = Hour(GmtDate) + Minute(GmtDate) / 60# + Second(GmtDate) / 3600#




'if((myr%4)==0 && mon>2) md++;

'for testing
'Myr = 1998
'Mmo = 5
'Md = 23
'Tgmt = 17 + 1 / 60 + 1 / 3600




'get day of year
Select Case Mmo
Case 1
Md = Md + 0
Case 2
Md = Md + 31
Case 3
Md = Md + 59
Case 4
Md = Md + 90
Case 5
Md = Md + 120
Case 6
Md = Md + 151
Case 7
Md = Md + 181
Case 8
Md = Md + 212
Case 9
Md = Md + 243
Case 10
Md = Md + 273
Case 11
Md = Md + 304
Case 12
Md = Md + 334
End Select

'leap year?
If (((Myr Mod 4) = 0) And Mmo > 2) Then
Md = Md + 1
End If

Call MESOL(Myr, Md, Tgmt, Dglat, Dglon, ZEN, Az, arms)

El = 90 - ZEN
TxtAz.Text = Az
TxtEl.Text = El
'fast sun finder
If (FSF) Then

PMACPORT.Output = "p52=" + Format(El + ELO) + Chr(13)
Else
If (Pointsun) Then
    If (SunPlot) Then
        If (ISAPMAC) Then
            If Flip Then
            PMACPORT.Output = "p51=" + Format(Az + 180 - AzOff) + Chr(13) 'center of  scan
            PMACPORT.Output = "p52=" + Format(180 - El - ElOff) + Chr(13) 'El
             Else
            PMACPORT.Output = "p51=" + Format(Az + AzOff) + Chr(13) 'center of  scan
            PMACPORT.Output = "p52=" + Format(El + ElOff) + Chr(13) 'El
            End If
        End If
    Else
        If (El > 45) Then   'wider scans for high elevations
        Azw = 6
        Azv = 2
        Else: Azw = 4
        Azv = 1.5
        End If
    
        If (El > 60) Then
        Pointsun = False
        MsgBox "Cannot do solar scans if elevation is > 60 deg"
        CmdRunScan.Caption = "Pointing Away From The Sun"
        sdelay (2)
        CmdRunScan.Caption = "Run Scan"
        Else
If (ISAPMAC) Then
    PMACPORT.Output = "p126=" + Format(Azv) + Chr(13) 'Azv
    If Flip Then
    PMACPORT.Output = "p51=" + Format(Az + 180 - AzOff) + Chr(13) 'center of  scan
    PMACPORT.Output = "p52=" + Format(180 - El - ElOff) + Chr(13) 'El
    ' az endpoints
    PMACPORT.Output = "p106=" + Format(Az + 180 - AzOff - Azw) + Chr(13)
    PMACPORT.Output = "p114=" + Format(Az + 180 - AzOff + Azw) + Chr(13)
    Else
    PMACPORT.Output = "p51=" + Format(Az + AzOff) + Chr(13) 'center of  scan
    PMACPORT.Output = "p52=" + Format(El + ElOff) + Chr(13) 'El
    ' az endpoints
    PMACPORT.Output = "p106=" + Format(Az + AzOff - Azw) + Chr(13)
    PMACPORT.Output = "p114=" + Format(Az + AzOff + Azw) + Chr(13)
    End If
End If
End If
End If
Else
If (ISAPMAC) Then
' az and el center
PMACPORT.Output = "p52=90" + Chr(13)  'El
PMACPORT.Output = "p106=-10" + Chr(13)  'Az
PMACPORT.Output = "p114=10" + Chr(13)  'El
End If
End If
End If
Beep
End Sub

Private Sub Txtgmtoff_DblClick()
Temp = InputBox("Enter Gmt offset")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If
GmtOff = Temp
Txtgmtoff.Text = GmtOff
Open "LatLong" For Output As #1
Write #1, LatDeg, LatMin, LonDeg, LonMin, GmtOff
Close #1
End Sub








Private Sub TxtLD_DblClick()
Temp = InputBox("Enter Latitude Degrees")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If
LatDeg = Temp
TxtLD.Text = LatDeg
Open "LatLong" For Output As #1
Write #1, LatDeg, LatMin, LonDeg, LonMin, GmtOff
Close #1
End Sub

Private Sub TxtLM_DblClick()
Temp = InputBox("Enter Longitude Minutes")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If
LatMin = Temp
TxtLM.Text = LatMin
Open "LatLong" For Output As #1
Write #1, LatDeg, LatMin, LonDeg, LonMin, GmtOff
Close #1
End Sub

Private Sub TxtLongDeg_DblClick()
Temp = InputBox("Enter Longitude Degrees")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If
LonDeg = Temp

TxtLongDeg.Text = LonDeg
Open "LatLong" For Output As #1
Write #1, LatDeg, LatMin, LonDeg, LonMin, GmtOff
Close #1
End Sub

Private Sub TxtLongMIn_DblClick()
Temp = InputBox("Enter Longitude Minutes")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If
LonMin = Temp
TxtLongMIn.Text = LonMin
Open "LatLong" For Output As #1
Write #1, LatDeg, LatMin, LonDeg, LonMin, GmtOff
Close #1
End Sub
