VERSION 5.00
Begin VB.Form frmSur 
   BackColor       =   &H00C000C0&
   Caption         =   "Edit Surveillance Scan"
   ClientHeight    =   5370
   ClientLeft      =   3285
   ClientTop       =   3015
   ClientWidth     =   6705
   LinkTopic       =   "Form1"
   MousePointer    =   1  'Arrow
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5370
   ScaleWidth      =   6705
   StartUpPosition =   1  'CenterOwner
   Begin VB.OptionButton optFang9 
      BackColor       =   &H00FFFFC0&
      Caption         =   "Fixed Angle List 9"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   4440
      MousePointer    =   2  'Cross
      TabIndex        =   18
      Top             =   3450
      Width           =   2025
   End
   Begin VB.OptionButton optFang10 
      BackColor       =   &H00FFFFC0&
      Caption         =   "Fixed Angle List 10"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   4440
      MousePointer    =   2  'Cross
      TabIndex        =   17
      Top             =   3840
      Width           =   2025
   End
   Begin VB.OptionButton optFang5 
      BackColor       =   &H00FFFFC0&
      Caption         =   "Fixed Angle List 5"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   4440
      MousePointer    =   2  'Cross
      TabIndex        =   16
      Top             =   1905
      Width           =   2025
   End
   Begin VB.OptionButton optFang6 
      BackColor       =   &H00FFFFC0&
      Caption         =   "Fixed Angle List 6"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   4440
      MousePointer    =   2  'Cross
      TabIndex        =   15
      Top             =   2295
      Width           =   2025
   End
   Begin VB.OptionButton optFang7 
      BackColor       =   &H00FFFFC0&
      Caption         =   "Fixed Angle List 7"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   4440
      MousePointer    =   2  'Cross
      TabIndex        =   14
      Top             =   2670
      Width           =   2025
   End
   Begin VB.OptionButton optFang8 
      BackColor       =   &H00FFFFC0&
      Caption         =   "Fixed Angle List 8"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   4440
      MousePointer    =   2  'Cross
      TabIndex        =   13
      Top             =   3060
      Width           =   2025
   End
   Begin VB.CommandButton Command2 
      Caption         =   "?"
      Height          =   255
      Left            =   3720
      TabIndex        =   12
      Top             =   960
      Width           =   375
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H000000FF&
      Caption         =   "Cancel"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   360
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   11
      Top             =   2520
      Width           =   1335
   End
   Begin VB.CommandButton cmdReturn 
      BackColor       =   &H0000C000&
      Caption         =   "Enter"
      Default         =   -1  'True
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   2040
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   9
      Top             =   2520
      Width           =   1575
   End
   Begin VB.TextBox txtAzRate 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   405
      Left            =   2280
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   8
      Text            =   "Text1"
      Top             =   840
      Width           =   900
   End
   Begin VB.OptionButton optFang4 
      BackColor       =   &H00FFFFC0&
      Caption         =   "Fixed Angle List 4"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   4440
      MousePointer    =   2  'Cross
      TabIndex        =   6
      Top             =   1515
      Width           =   2025
   End
   Begin VB.OptionButton optFang3 
      BackColor       =   &H00FFFFC0&
      Caption         =   "Fixed Angle List 3"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   4440
      MousePointer    =   2  'Cross
      TabIndex        =   5
      Top             =   1125
      Width           =   2025
   End
   Begin VB.OptionButton optFang2 
      BackColor       =   &H00FFFFC0&
      Caption         =   "Fixed Angle List 2"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   4440
      MousePointer    =   2  'Cross
      TabIndex        =   4
      Top             =   750
      Width           =   2025
   End
   Begin VB.OptionButton optFang1 
      BackColor       =   &H00FFFFC0&
      Caption         =   "Fixed Angle List 1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   4440
      MousePointer    =   2  'Cross
      TabIndex        =   3
      Top             =   360
      Width           =   2025
   End
   Begin VB.TextBox txtSamples 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   405
      Left            =   2280
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   2
      Text            =   "Text4"
      Top             =   1440
      Width           =   900
   End
   Begin VB.Label lblSurId 
      BackColor       =   &H00C000C0&
      Caption         =   "Now Editing Sur Scan # ?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000FF00&
      Height          =   375
      Left            =   1080
      TabIndex        =   10
      Top             =   120
      Width           =   3135
   End
   Begin VB.Label Label5 
      BackColor       =   &H00C000C0&
      Caption         =   "Use:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   16.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   3480
      TabIndex        =   7
      Top             =   2040
      Width           =   735
   End
   Begin VB.Label Label4 
      BackColor       =   &H00C000C0&
      Caption         =   "Samples"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   405
      Left            =   360
      TabIndex        =   1
      Top             =   1440
      Width           =   1095
   End
   Begin VB.Label Label3 
      BackColor       =   &H00C000C0&
      Caption         =   "Azimuth Rate"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   405
      Left            =   360
      TabIndex        =   0
      Top             =   840
      Width           =   1815
   End
End
Attribute VB_Name = "frmSur"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim Azrate, pSamp  As Single
Dim J, K As Integer
Dim Temp


Function WriteOut()
Dim J As Integer
J = 0
If frmMain.optSur1 = True Then
J = 1
End If

If frmMain.optSur2 = True Then
J = 2
End If

If frmMain.optSur3 = True Then
J = 3
End If


SurDef(RATE, J) = Azrate
SurDef(SAMPLES, J) = pSamp
SurDef(12, J) = 0   'azoff
SurDef(13, J) = 0    'eloff


K = 0
If optFang1 Then
SurDef(FAPOINT, J) = 1
K = 1
End If
If optFang2 Then
SurDef(FAPOINT, J) = 2
K = 2
End If
If optFang3 Then
SurDef(FAPOINT, J) = 3
K = 3
End If
If optFang4 Then
SurDef(FAPOINT, J) = 4
K = 4
End If
If optFang5 Then
SurDef(FAPOINT, J) = 5
K = 5
End If
If optFang6 Then
SurDef(FAPOINT, J) = 6
K = 6
End If
If optFang7 Then
SurDef(FAPOINT, J) = 7
K = 7
End If
If optFang8 Then
SurDef(FAPOINT, J) = 8
K = 8
End If
If optFang9 Then
SurDef(FAPOINT, J) = 9
K = 9
End If
If optFang10 Then
SurDef(FAPOINT, J) = 10
K = 10
End If


If frmMain.optSur1 = True Then
frmMain.SurFa1.Text = K
End If

If frmMain.optSur2 = True Then
frmMain.SurFa2.Text = K
End If

If frmMain.optSur3 = True Then
frmMain.SurFa3.Text = K
End If

If ISAPMAC Then
PMACPORT.Output = "disable plc10" + Chr(13)
PMACPORT.Output = "disable plc20" + Chr(13)
End If
Screen.MousePointer = vbHourglass
'frmMain.Enabled = False
frmPlWait.Show
frmPlWait.Refresh
frmMain.EncodeSur
frmMain.EncodeFa
frmPlWait.Hide
'frmMain.Enabled = True
Screen.MousePointer = vbDefault
If ISAPMAC Then
PMACPORT.Output = "enable plc10" + Chr(13)
PMACPORT.Output = "enable plc20" + Chr(13)
End If
WriteOut = True

If K = 0 Then
MsgBox "Must Select a Fixed Angle List", vbExclamation
WriteOut = False
Exit Function
End If

If pSamp < 32 Then
MsgBox "Must Have At Least 32 Samples", vbExclamation
WriteOut = False
Exit Function
End If

End Function


Private Sub cmdReturn_Click()
frmFang.Hide
frmMain.Show
'frmMain.Enabled = True
frmMain.Show
frmMain.Refresh
If WriteOut Then
frmSur.Hide
End If
frmMain.CmdDump.Caption = "Save Scans?"
End Sub

Private Sub Command1_Click()
frmFang.Hide
frmSur.Hide
frmMain.Show
'frmMain.Enabled = True
frmMain.Show
frmMain.Refresh
End Sub

Private Sub Command2_Click()
MsgBox "Double Click on Button to edit Fixed Angle List", vbInformation
End Sub

Private Sub Form_Activate()
frmMain.Hide
J = 0

If frmMain.optSur1.Value = True Then
lblSurId.Caption = "Now Editing  " + frmMain.optSur1.Caption
J = 1
End If

If frmMain.optSur2.Value = True Then
lblSurId.Caption = "Now Editing  " + frmMain.optSur2.Caption
J = 2
End If

If frmMain.optSur3.Value = True Then
lblSurId.Caption = "Now Editing  " + frmMain.optSur3.Caption
J = 3
End If

If J = 0 Then
lblSurId.Caption = "Now Editing Sur Scan #?"
Beep
MsgBox "Must Select One Sur Volume", vbOKOnly + vbExclamation
frmMain.Show
frmSur.Hide
Exit Sub
End If



txtAzRate.Text = SurDef(RATE, J)
txtSamples.Text = SurDef(SAMPLES, J)
pSamp = SurDef(SAMPLES, J)
Azrate = SurDef(RATE, J)

optFang1 = False
optFang2 = False
optFang3 = False
optFang4 = False


If SurDef(FAPOINT, J) = 1 Then
optFang1 = True
End If
If SurDef(FAPOINT, J) = 2 Then
optFang2 = True
End If
If SurDef(FAPOINT, J) = 3 Then
optFang3 = True
End If
If SurDef(FAPOINT, J) = 4 Then
optFang4 = True
End If
If SurDef(FAPOINT, J) = 5 Then
optFang5 = True
End If
If SurDef(FAPOINT, J) = 6 Then
optFang6 = True
End If
If SurDef(FAPOINT, J) = 7 Then
optFang7 = True
End If
If SurDef(FAPOINT, J) = 8 Then
optFang8 = True
End If
If SurDef(FAPOINT, J) = 9 Then
optFang9 = True
End If
If SurDef(FAPOINT, J) = 10 Then
optFang10 = True
End If

End Sub

Private Sub Label1_Click()

End Sub

Private Sub optFAng1_DblClick()
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "1"

End Sub


Private Sub optFang10_DblClick()
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "10"

End Sub

Private Sub optFAng2_DblClick()
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "2"

End Sub


Private Sub optFAng3_DblClick()
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "3"

End Sub


Private Sub optFang4_DblClick()
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "4"

End Sub




Private Sub txtEleRate_Click()
Temp = InputBox("Enter Azimuth Rate")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp > 15 Then
MsgBox "Must Be Between Zero and 15"
Exit Sub
End If

Azrate = Temp
txtAzRate.Text = Str(Azrate)

End Sub




Private Sub optFang5_DblClick()
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "5"
End Sub

Private Sub optFang6_DblClick()
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "6"

End Sub


Private Sub optFang7_DblClick()
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "7"
End Sub

Private Sub optFang8_DblClick()
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "8"

End Sub

Private Sub optFang9_DblClick()
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "9"

End Sub



Private Sub txtAzRate_Click()

Temp = InputBox("Enter Azimuth Rate")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If
' different rates for different radars
If (ImaDow = 1) Then
If (Temp > SpolMaxr) Then
MsgBox "Must Be Between Zero and   ", Format(SpolMaxr)
Exit Sub
End If
End If


If (ImaDow = 2) Then
If (Temp > Dow2Maxr) Then
MsgBox "Must Be Between Zero and   ", Format(Dow2Maxr)
Exit Sub
End If
End If


If (ImaDow = 3) Then
If (Temp > Dow3Maxr) Then
MsgBox "Must Be Between Zero and   " + Format(Dow3Maxr)
Exit Sub
End If
End If



If (ImaDow = 4) Then
If (Temp > Dow4Maxr) Then
MsgBox "Must Be Between Zero and   ", Format(Dow4Maxr)
Exit Sub
End If
End If


Azrate = Temp
txtAzRate.Text = Str(Azrate)


End Sub


Private Sub txtSamp_Click()
Dim Temp

Temp = InputBox("Enter Samples")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If (Temp Mod 2) <> 0 Then
MsgBox "Must Be Even"
Exit Sub
End If

If Temp < 16 Then
MsgBox "Must Be Greater then 16"
Exit Sub
End If
pSamp = Temp
txtSamples.Text = Str(pSamp)

End Sub


Private Sub txtSamples_Click()
Temp = InputBox("Enter Samples")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If (Temp Mod 2) <> 0 Then
MsgBox "Must Be Even"
Exit Sub
End If


If Temp < 32 Then
MsgBox "Must Be At Least 32"
Exit Sub
End If

pSamp = Temp
txtSamples.Text = Str(pSamp)


End Sub


