VERSION 5.00
Begin VB.Form frmRhi 
   BackColor       =   &H0000FF00&
   Caption         =   "Edit Rhi"
   ClientHeight    =   5895
   ClientLeft      =   1890
   ClientTop       =   4065
   ClientWidth     =   6690
   LinkTopic       =   "Form1"
   MousePointer    =   1  'Arrow
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   5895
   ScaleWidth      =   6690
   StartUpPosition =   1  'CenterOwner
   Begin VB.OptionButton optFang8 
      BackColor       =   &H00FF00FF&
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
      Height          =   285
      Left            =   4560
      MousePointer    =   2  'Cross
      TabIndex        =   22
      Top             =   2880
      Width           =   2025
   End
   Begin VB.OptionButton optFang9 
      BackColor       =   &H00FF00FF&
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
      Height          =   285
      Left            =   4560
      MousePointer    =   2  'Cross
      TabIndex        =   21
      Top             =   3240
      Width           =   2025
   End
   Begin VB.OptionButton optFang10 
      BackColor       =   &H00FF00FF&
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
      Height          =   285
      Left            =   4560
      MousePointer    =   2  'Cross
      TabIndex        =   20
      Top             =   3600
      Width           =   2025
   End
   Begin VB.OptionButton optFang5 
      BackColor       =   &H00FF00FF&
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
      Height          =   285
      Left            =   4560
      MousePointer    =   2  'Cross
      TabIndex        =   19
      Top             =   1800
      Width           =   2025
   End
   Begin VB.OptionButton optFang6 
      BackColor       =   &H00FF00FF&
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
      Height          =   285
      Left            =   4560
      MousePointer    =   2  'Cross
      TabIndex        =   18
      Top             =   2160
      Width           =   2025
   End
   Begin VB.OptionButton optFang7 
      BackColor       =   &H00FF00FF&
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
      Height          =   285
      Left            =   4560
      MousePointer    =   2  'Cross
      TabIndex        =   17
      Top             =   2520
      Width           =   2025
   End
   Begin VB.CommandButton Command2 
      Caption         =   "?"
      Height          =   255
      Left            =   3120
      MousePointer    =   2  'Cross
      TabIndex        =   16
      Top             =   3240
      Width           =   255
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
      Left            =   240
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   15
      Top             =   3120
      Width           =   975
   End
   Begin VB.TextBox txtElRate 
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
      Left            =   2500
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   14
      Text            =   "Text1"
      Top             =   1560
      Width           =   900
   End
   Begin VB.CommandButton cmdReturn 
      BackColor       =   &H00008000&
      Caption         =   "Enter"
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
      Left            =   1440
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   12
      Top             =   3120
      Width           =   1215
   End
   Begin VB.OptionButton optFang4 
      BackColor       =   &H00FF00FF&
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
      Height          =   285
      Left            =   4560
      MousePointer    =   2  'Cross
      TabIndex        =   10
      Top             =   1440
      Width           =   2025
   End
   Begin VB.OptionButton optFang3 
      BackColor       =   &H00FF00FF&
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
      Height          =   285
      Left            =   4560
      MousePointer    =   2  'Cross
      TabIndex        =   9
      Top             =   1080
      Width           =   2025
   End
   Begin VB.OptionButton optFang2 
      BackColor       =   &H00FF00FF&
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
      Height          =   285
      Left            =   4560
      MousePointer    =   2  'Cross
      TabIndex        =   8
      Top             =   720
      Width           =   2025
   End
   Begin VB.OptionButton optFang1 
      BackColor       =   &H00FF00FF&
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
      Height          =   285
      Left            =   4560
      MousePointer    =   2  'Cross
      TabIndex        =   7
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
      Left            =   2500
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   6
      Text            =   "Text4"
      Top             =   2040
      Width           =   900
   End
   Begin VB.TextBox txtElTop 
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
      Left            =   2500
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   5
      Text            =   "Text2"
      Top             =   1080
      Width           =   900
   End
   Begin VB.TextBox txtElBot 
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
      Left            =   2500
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   4
      Text            =   "Text1"
      Top             =   480
      Width           =   900
   End
   Begin VB.Label lblRhiId 
      BackColor       =   &H0000FF00&
      Caption         =   "Now Editing RHI Scan # ?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000080FF&
      Height          =   375
      Left            =   1320
      TabIndex        =   13
      Top             =   0
      Width           =   3255
   End
   Begin VB.Label Label5 
      BackColor       =   &H0000FF00&
      Caption         =   "Use:"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   18
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   3600
      TabIndex        =   11
      Top             =   1800
      Width           =   735
   End
   Begin VB.Label Label4 
      BackColor       =   &H0000FF00&
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
      ForeColor       =   &H000080FF&
      Height          =   405
      Left            =   120
      TabIndex        =   3
      Top             =   2040
      Width           =   1095
   End
   Begin VB.Label Label3 
      BackColor       =   &H0000FF00&
      Caption         =   "Elevation Rate"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000080FF&
      Height          =   405
      Left            =   120
      TabIndex        =   2
      Top             =   1520
      Width           =   2295
   End
   Begin VB.Label Label2 
      BackColor       =   &H0000FF00&
      Caption         =   "Elevation Top"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000080FF&
      Height          =   405
      Left            =   120
      TabIndex        =   1
      Top             =   1000
      Width           =   2295
   End
   Begin VB.Label Label1 
      BackColor       =   &H0000FF00&
      Caption         =   "Elevation Bottom"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000080FF&
      Height          =   405
      Left            =   120
      TabIndex        =   0
      Top             =   480
      Width           =   2295
   End
End
Attribute VB_Name = "frmRhi"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim ElBot, ElTop, pSamp, ElRate As Single
Dim J, K As Integer
Dim Temp
Function WriteOut()
J = 0
If frmMain.optRhi1 = True Then
J = 1
End If

If frmMain.optRhi2 = True Then
J = 2
End If

If frmMain.optRhi3 = True Then
J = 3
End If


RhiDef(ELT, J) = ElTop
RhiDef(ELB, J) = ElBot
RhiDef(RATE, J) = ElRate
RhiDef(SAMPLES, J) = pSamp
RhiDef(12, J) = 0 'azoff
RhiDef(13, J) = 0  'eloff




K = 0
If optFang1 Then
RhiDef(FAPOINT, J) = 1
K = 1
End If
If optFang2 Then
RhiDef(FAPOINT, J) = 2
K = 2
End If
If optFang3 Then
RhiDef(FAPOINT, J) = 3
K = 3
End If
If optFang4 Then
RhiDef(FAPOINT, J) = 4
K = 4
End If
If optFang5 Then
RhiDef(FAPOINT, J) = 5
K = 5
End If
If optFang6 Then
RhiDef(FAPOINT, J) = 6
K = 6
End If
If optFang7 Then
RhiDef(FAPOINT, J) = 7
K = 7
End If
If optFang8 Then
RhiDef(FAPOINT, J) = 8
K = 8
End If
If optFang9 Then
RhiDef(FAPOINT, J) = 9
K = 9
End If
If optFang10 Then
RhiDef(FAPOINT, J) = 10
K = 10
End If

If frmMain.optRhi1 = True Then
frmMain.RhiFa1.Text = K
End If

If frmMain.optRhi2 = True Then
frmMain.RhiFa2.Text = K
End If

If frmMain.optRhi3 = True Then
frmMain.RhiFa3.Text = K
End If





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

If ISAPMAC Then
PMACPORT.Output = "disable plc10" + Chr(13)
PMACPORT.Output = "disable plc20" + Chr(13)
End If
Screen.MousePointer = vbHourglass
'frmMain.Enabled = False
frmPlWait.Show
frmPlWait.Refresh
frmMain.EncodeRhi
frmMain.EncodeFa
frmPlWait.Hide
'frmMain.Enabled = True
Screen.MousePointer = vbDefault
If ISAPMAC Then
PMACPORT.Output = "enable plc10" + Chr(13)
PMACPORT.Output = "enable plc20" + Chr(13)
End If
WriteOut = True




End Function

Private Sub Command1_Click()
frmFang.Hide
frmRhi.Hide
frmMain.Show
'frmMain.Enabled = True
frmMain.Show
frmMain.Refresh
End Sub

Private Sub cmdReturn_Click()
frmFang.Hide
frmMain.Show
'frmMain.Enabled = True
frmMain.Show
frmMain.Refresh
If WriteOut Then
frmRhi.Hide

End If
frmMain.CmdDump.Caption = "Save Scans?"
End Sub

Private Sub Command2_Click()
MsgBox "Double Click on Button to edit Fixed Angle List", vbInformation
End Sub

Private Sub Form_Activate()
frmMain.Hide
J = 0
If frmMain.optRhi1.Value = True Then
lblRhiId.Caption = "Now Editing  " + frmMain.optRhi1.Caption
J = 1
End If

If frmMain.optRhi2.Value = True Then
lblRhiId.Caption = "Now Editing  " + frmMain.optRhi2.Caption
J = 2
End If

If frmMain.optRhi3.Value = True Then
lblRhiId.Caption = "Now Editing  " + frmMain.optRhi3.Caption
J = 3
End If

If J = 0 Then
lblRhiId.Caption = "Now Editing RHI Scan # ?"
Beep
MsgBox "Must Select One RHI Volume", vbOKOnly + vbExclamation

frmRhi.Hide
frmMain.Show
Exit Sub
End If

txtElTop = RhiDef(ELT, J)
txtElBot = RhiDef(ELB, J)
txtElRate = RhiDef(RATE, J)
txtSamples = RhiDef(SAMPLES, J)

'Bring in the Old Values from the Array
ElTop = RhiDef(ELT, J)
ElBot = RhiDef(ELB, J)
ElRate = RhiDef(RATE, J)
pSamp = RhiDef(SAMPLES, J)
optFang1 = False
optFang2 = False
optFang3 = False
optFang4 = False

If RhiDef(FAPOINT, J) = 1 Then
optFang1 = True
End If
If RhiDef(FAPOINT, J) = 2 Then
optFang2 = True
End If
If RhiDef(FAPOINT, J) = 3 Then
optFang3 = True
End If
If RhiDef(FAPOINT, J) = 4 Then
optFang4 = True
End If
If RhiDef(FAPOINT, J) = 5 Then
optFang5 = True
End If
If RhiDef(FAPOINT, J) = 6 Then
optFang6 = True
End If
If RhiDef(FAPOINT, J) = 7 Then
optFang7 = True
End If
If RhiDef(FAPOINT, J) = 8 Then
optFang8 = True
End If
If RhiDef(FAPOINT, J) = 9 Then
optFang9 = True
End If
If RhiDef(FAPOINT, J) = 10 Then
optFang10 = True
End If


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

Private Sub txtElBot_Click()
Temp = InputBox("Enter Elevation Bottom")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Not be Less Than Zero"
Exit Sub
End If
ElBot = Temp
txtElBot.Text = Str(ElBot)
End Sub


Private Sub txtEleRate_Click()
Temp = InputBox("Enter Elevation Rate")
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

ElRate = Temp
txtElRate.Text = Str(ElRate)

End Sub


Private Sub txtElRate_Click()
Temp = InputBox("Enter Elevation Rate")
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

ElRate = Temp
txtElRate.Text = Str(ElRate)

End Sub


Private Sub txtElTop_Click()
Temp = InputBox("Enter Elevation Top")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp > 180 Then
MsgBox "Must Be Between Zero and 180"
Exit Sub
End If

ElTop = Temp
txtElTop.Text = Str(ElTop)

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


