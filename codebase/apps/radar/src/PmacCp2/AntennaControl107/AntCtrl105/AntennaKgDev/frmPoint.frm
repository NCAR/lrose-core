VERSION 5.00
Begin VB.Form frmPoint 
   BackColor       =   &H000080FF&
   Caption         =   "Point and Stop"
   ClientHeight    =   7200
   ClientLeft      =   12870
   ClientTop       =   4185
   ClientWidth     =   11445
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   13.5
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   LinkTopic       =   "Form1"
   ScaleHeight     =   7200
   ScaleWidth      =   11445
   StartUpPosition =   2  'CenterScreen
   Begin VB.CommandButton CmdTrack 
      Caption         =   "Track"
      Height          =   975
      Left            =   4080
      TabIndex        =   11
      Top             =   3840
      Width           =   3015
   End
   Begin VB.TextBox txtHits 
      Height          =   492
      Left            =   6120
      TabIndex        =   10
      Text            =   "128"
      Top             =   6240
      Width           =   732
   End
   Begin VB.CommandButton cmdHits 
      Caption         =   "Hits"
      Height          =   612
      Left            =   4080
      TabIndex        =   9
      Top             =   6240
      Width           =   1812
   End
   Begin VB.CommandButton Command1 
      Caption         =   "?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   840
      TabIndex        =   8
      Top             =   2880
      Width           =   615
   End
   Begin VB.VScrollBar VScrollEl 
      Height          =   6972
      LargeChange     =   20
      Left            =   10560
      Max             =   3600
      MousePointer    =   2  'Cross
      SmallChange     =   2
      TabIndex        =   7
      Top             =   120
      Width           =   495
   End
   Begin VB.HScrollBar HScrollAz 
      Height          =   495
      LargeChange     =   20
      Left            =   240
      Max             =   7200
      MousePointer    =   2  'Cross
      SmallChange     =   2
      TabIndex        =   6
      Top             =   120
      Width           =   10212
   End
   Begin VB.CommandButton frmcanc 
      Caption         =   "Bye"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   492
      Left            =   480
      TabIndex        =   5
      Top             =   6480
      Width           =   972
   End
   Begin VB.CommandButton cmdpnt 
      BackColor       =   &H00008000&
      Caption         =   "Point Now"
      Height          =   1095
      Left            =   4080
      MaskColor       =   &H0000C000&
      TabIndex        =   4
      Top             =   2400
      Width           =   2415
   End
   Begin VB.TextBox txtEl 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   372
      Left            =   9120
      TabIndex        =   1
      Text            =   "Text2"
      Top             =   3600
      Width           =   1212
   End
   Begin VB.TextBox txtAz 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   372
      Left            =   4680
      TabIndex        =   0
      Text            =   "Text1"
      Top             =   840
      Width           =   1212
   End
   Begin VB.Label Label2 
      BackColor       =   &H000080FF&
      Caption         =   "EL"
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
      Left            =   8400
      TabIndex        =   3
      Top             =   3600
      Width           =   615
   End
   Begin VB.Label Label1 
      BackColor       =   &H000080FF&
      Caption         =   "AZ"
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
      Left            =   4200
      TabIndex        =   2
      Top             =   840
      Width           =   615
   End
End
Attribute VB_Name = "frmPoint"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim Az As Single
Dim El As Single
Dim Enable As Boolean


Private Sub cmdHits_Click()
Dim zzz, Hits
zzz = InputBox("Enter Hits: Check Dropped Beams")
If IsNumeric(zzz) Then
Hits = zzz
txtHits.Text = zzz
PMACPORT.Output = "m207=" + Format(Hits) + Chr(13) 'Hits only
End If
End Sub

Private Sub cmdpnt_Click()
If (Enable) Then
Enable = False
cmdpnt.Caption = "Press Here to Point"
Else
Enable = True
cmdpnt.Caption = "Continuously Pointing"
MalfEnb = False
If (ISAPMAC) Then
PMACPORT.Output = "P50=0" + Chr(13)
sdelay (2)
PMACPORT.Output = "p51=" + Format(Az) + Chr(13) 'Az
PMACPORT.Output = "p52=" + Format(El) + Chr(13) 'El
PMACPORT.Output = "p50=3" + Chr(13)
Beep
End If
End If
End Sub

Private Sub CmdTrack_Click()
If (Enable) Then
Enable = False
CmdTrack.Caption = "Press Here to Track"

If (ISAPMAC) Then
PMACPORT.Output = "P50=0" + Chr(13)
sdelay (2)
End If

Else
Enable = True
CmdTrack.Caption = "Tracking: Running Last Loaded Scan"
If (ISAPMAC) Then
PMACPORT.Output = "P50=0" + Chr(13)
sdelay (2)
PMACPORT.Output = "p51=" + Format(Az) + Chr(13) 'Az
PMACPORT.Output = "p52=" + Format(El) + Chr(13) 'El
PMACPORT.Output = "p50=17" + Chr(13)
Beep
End If
End If
End Sub

Private Sub Command1_Click()
MsgBox "Use slider for big changes " + Chr(13) + "Click in slider for 1 deg. changes" + Chr(13) + "Click small arrows for .1 degree changes.", vbOKOnly, "Point and Stop"


End Sub

Private Sub Form_Load()

ISRUNNING = True
Enable = False
cmdpnt.Caption = "Press Here to Point"
Az = 45
El = 45
TxtAz.Text = Az
TxtEl.Text = El
VScrollEl.Value = (180 - El) * 20
HScrollAz.Value = Az * 20
If (ISAPMAC) Then
PMACPORT.Output = "m207=" + Format(128) + Chr(13) 'Hits only
End If
End Sub

Private Sub frmcanc_Click()
Enable = False
ISRUNNING = False
Unload frmPoint
End Sub

Private Sub HScrollAz_Change()
Az = HScrollAz.Value / 20#
TxtAz.Text = Az
If (Enable) Then
If (ISAPMAC) Then
'PMACPORT.Output = "P50=0" + Chr(13)
'sdelay (2)
PMACPORT.Output = "p51=" + Format(Az) + Chr(13) 'Az
PMACPORT.Output = "p52=" + Format(El) + Chr(13) 'El
'PMACPORT.Output = "p50=3" + Chr(13)
End If
Beep
End If
End Sub

Private Sub HScrollAz_Scroll()
Az = HScrollAz.Value / 20#
TxtAz.Text = Az
End Sub



Private Sub txtAz_Click()
Dim zzz
zzz = InputBox("Enter Azimuth")
If IsNumeric(zzz) Then
Az = zzz
TxtAz.Text = zzz
HScrollAz.Value = Az * 20
If (Enable) Then
If (ISAPMAC) Then
'PMACPORT.Output = "P50=0" + Chr(13)
'sdelay (2)
PMACPORT.Output = "p51=" + Format(Az) + Chr(13) 'Az
PMACPORT.Output = "p52=" + Format(El) + Chr(13) 'El
'PMACPORT.Output = "p50=3" + Chr(13)

End If
End If
End If

End Sub

Private Sub txtEl_Click()
Dim zzz
zzz = InputBox("Enter Elevation")
If IsNumeric(zzz) Then
El = zzz
VScrollEl.Value = (90 - El) * 20
TxtEl.Text = zzz
If (Enable) Then
If (ISAPMAC) Then
'PMACPORT.Output = "P50=0" + Chr(13)
'sdelay (2)
PMACPORT.Output = "p51=" + Format(Az) + Chr(13) 'Az
PMACPORT.Output = "p52=" + Format(El) + Chr(13) 'El
'PMACPORT.Output = "p50=3" + Chr(13)

End If
End If
End If

End Sub

Private Sub VScrollEl_Change()
El = 180 - VScrollEl.Value / 20#
TxtEl.Text = El
If (Enable) Then
If (ISAPMAC) Then
'PMACPORT.Output = "P50=0" + Chr(13)
'sdelay (2)
PMACPORT.Output = "p51=" + Format(Az) + Chr(13) 'Az
PMACPORT.Output = "p52=" + Format(El) + Chr(13) 'El
'PMACPORT.Output = "p50=3" + Chr(13)

End If
End If

End Sub

Private Sub VScrollEl_Scroll()
El = 180 - VScrollEl / 20#
TxtEl.Text = El
End Sub
