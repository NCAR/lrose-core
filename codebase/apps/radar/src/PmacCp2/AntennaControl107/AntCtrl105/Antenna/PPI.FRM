VERSION 5.00
Begin VB.Form frmPPI 
   BackColor       =   &H000040C0&
   Caption         =   "PPI Volume Specification"
   ClientHeight    =   4575
   ClientLeft      =   4050
   ClientTop       =   3105
   ClientWidth     =   6420
   FillColor       =   &H00FFFFFF&
   FillStyle       =   6  'Cross
   ForeColor       =   &H0000C000&
   LinkTopic       =   "Form1"
   MousePointer    =   1  'Arrow
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   4575
   ScaleWidth      =   6420
   StartUpPosition =   2  'CenterScreen
   Begin VB.OptionButton optFang10 
      Caption         =   "Fixed Angle List 10"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   300
      Left            =   3360
      MousePointer    =   5  'Size
      TabIndex        =   27
      Top             =   3840
      Width           =   2325
   End
   Begin VB.OptionButton optFang9 
      Caption         =   "Fixed Angle List 9"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   300
      Left            =   3360
      MousePointer    =   5  'Size
      TabIndex        =   26
      Top             =   3480
      Width           =   2325
   End
   Begin VB.OptionButton optFang8 
      Caption         =   "Fixed Angle List 8"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   300
      Left            =   3360
      MousePointer    =   5  'Size
      TabIndex        =   25
      Top             =   3120
      Width           =   2325
   End
   Begin VB.OptionButton optFang7 
      Caption         =   "Fixed Angle List 7"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   300
      Left            =   3360
      MousePointer    =   5  'Size
      TabIndex        =   24
      Top             =   2760
      Width           =   2325
   End
   Begin VB.OptionButton optFang6 
      Caption         =   "Fixed Angle List 6"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   300
      Left            =   3360
      MousePointer    =   5  'Size
      TabIndex        =   23
      Top             =   2400
      Width           =   2325
   End
   Begin VB.OptionButton optFang5 
      Caption         =   "Fixed Angle List 5"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   300
      Left            =   3360
      MousePointer    =   5  'Size
      TabIndex        =   22
      Top             =   2040
      Width           =   2325
   End
   Begin VB.CommandButton Command2 
      BackColor       =   &H00E0E0E0&
      Caption         =   "?"
      Height          =   255
      Left            =   2520
      MousePointer    =   5  'Size
      Style           =   1  'Graphical
      TabIndex        =   21
      Top             =   3360
      Width           =   375
   End
   Begin VB.CommandButton Command3 
      Appearance      =   0  'Flat
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
      Height          =   375
      Left            =   480
      MousePointer    =   5  'Size
      Style           =   1  'Graphical
      TabIndex        =   20
      Top             =   3840
      Width           =   1095
   End
   Begin VB.CommandButton cmdReturn 
      Appearance      =   0  'Flat
      BackColor       =   &H0000C000&
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
      Left            =   1800
      MousePointer    =   5  'Size
      Style           =   1  'Graphical
      TabIndex        =   18
      Top             =   3840
      Width           =   1215
   End
   Begin VB.OptionButton optFang4 
      Caption         =   "Fixed Angle List 4"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   300
      Left            =   3360
      MousePointer    =   5  'Size
      TabIndex        =   16
      Top             =   1680
      Width           =   2325
   End
   Begin VB.OptionButton optFAng3 
      Caption         =   "Fixed Angle List 3"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   300
      Left            =   3360
      MousePointer    =   5  'Size
      TabIndex        =   15
      Top             =   1320
      Width           =   2325
   End
   Begin VB.OptionButton optFAng2 
      Caption         =   "Fixed Angle List 2"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   300
      Left            =   3360
      MousePointer    =   5  'Size
      TabIndex        =   14
      Top             =   960
      Width           =   2325
   End
   Begin VB.OptionButton optFAng1 
      BackColor       =   &H00C0C0C0&
      Caption         =   "Fixed Angle List 1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   300
      Left            =   3360
      MousePointer    =   5  'Size
      TabIndex        =   13
      Top             =   600
      Width           =   2325
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H00FF00FF&
      Caption         =   "Command1"
      Height          =   495
      Left            =   3840
      TabIndex        =   12
      Top             =   4560
      Width           =   1215
   End
   Begin VB.TextBox txtSamples 
      BackColor       =   &H00FFFFFF&
      BorderStyle     =   0  'None
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   480
      Left            =   1560
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   11
      Text            =   "****"
      Top             =   3120
      Width           =   735
   End
   Begin VB.TextBox txtAzRate 
      BackColor       =   &H00FFFFFF&
      BorderStyle     =   0  'None
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   480
      Left            =   1560
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   10
      Text            =   "****"
      Top             =   2520
      Width           =   735
   End
   Begin VB.TextBox txtAzLeft 
      BackColor       =   &H00FFFFFF&
      BorderStyle     =   0  'None
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   480
      Left            =   1560
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   9
      Text            =   "****"
      Top             =   1920
      Width           =   735
   End
   Begin VB.TextBox txtAzRight 
      BackColor       =   &H00FFFFFF&
      BorderStyle     =   0  'None
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   480
      Left            =   1560
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   8
      Text            =   "****"
      Top             =   1320
      Width           =   735
   End
   Begin VB.TextBox txtCenter 
      Appearance      =   0  'Flat
      BackColor       =   &H00FFFFFF&
      BorderStyle     =   0  'None
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   480
      Left            =   1560
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   7
      Text            =   "***"
      Top             =   120
      Width           =   735
   End
   Begin VB.TextBox txtWidth 
      BackColor       =   &H00FFFFFF&
      BorderStyle     =   0  'None
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00000000&
      Height          =   480
      Left            =   1560
      Locked          =   -1  'True
      MousePointer    =   3  'I-Beam
      TabIndex        =   6
      Text            =   "****"
      Top             =   720
      Width           =   735
   End
   Begin VB.Label lblPpiId 
      BackColor       =   &H000040C0&
      Caption         =   "Now Editing PPI Scan # ?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000FF00&
      Height          =   375
      Left            =   2520
      TabIndex        =   19
      Top             =   0
      Width           =   3255
   End
   Begin VB.Label Label7 
      BackColor       =   &H000040C0&
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
      ForeColor       =   &H0080FFFF&
      Height          =   375
      Left            =   2520
      TabIndex        =   17
      Top             =   2160
      Width           =   735
   End
   Begin VB.Label Label6 
      BackColor       =   &H000040C0&
      Caption         =   "Width"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   480
      Left            =   120
      TabIndex        =   5
      Top             =   720
      Width           =   1215
   End
   Begin VB.Label Label5 
      BackColor       =   &H000040C0&
      Caption         =   "Center"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   480
      Left            =   120
      TabIndex        =   4
      Top             =   120
      Width           =   1215
   End
   Begin VB.Label Label4 
      BackColor       =   &H000040C0&
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
      ForeColor       =   &H00FFFFFF&
      Height          =   480
      Left            =   120
      TabIndex        =   3
      Top             =   3120
      Width           =   1215
   End
   Begin VB.Label Label3 
      BackColor       =   &H000040C0&
      Caption         =   "Rate"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   480
      Left            =   120
      TabIndex        =   2
      Top             =   2520
      Width           =   1215
   End
   Begin VB.Label Label2 
      BackColor       =   &H000040C0&
      Caption         =   "Az Left"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   480
      Left            =   120
      TabIndex        =   1
      Top             =   1920
      Width           =   1215
   End
   Begin VB.Label Label1 
      BackColor       =   &H000040C0&
      Caption         =   "Az Right"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   480
      Left            =   120
      TabIndex        =   0
      Top             =   1320
      Width           =   1215
   End
End
Attribute VB_Name = "frmPPI"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim J As Integer
Dim AzRight, Rotate, Cwidth, pSamp As Single
Dim AzLeft, Azrate, Center, Swidth As Single
Dim Temp

Function WriteOut()
Dim J, K As Integer
J = 0
If frmMain.optPpi1 = True Then
J = 1
End If

If frmMain.optPpi2 = True Then
J = 2
End If

If frmMain.optPpi3 = True Then
J = 3
End If

If frmMain.optPpi4 = True Then
J = 4
End If

PpiDef(AZR, J) = AzRight
PpiDef(AZL, J) = AzLeft
PpiDef(RATE, J) = Azrate
PpiDef(SAMPLES, J) = pSamp
PpiDef(12, J) = 0  'azoff
PpiDef(13, J) = 0   'eloff

K = 0
If optFang1 Then
PpiDef(FAPOINT, J) = 1
K = 1
End If
If optFang2 Then
PpiDef(FAPOINT, J) = 2
K = 2
End If
If optFang3 Then
PpiDef(FAPOINT, J) = 3
K = 3
End If
If optFang4 Then
PpiDef(FAPOINT, J) = 4
K = 4
End If

If optFang5 Then
PpiDef(FAPOINT, J) = 5
K = 5
End If
If optFang6 Then
PpiDef(FAPOINT, J) = 6
K = 6
End If
If optFang7 Then
PpiDef(FAPOINT, J) = 7
K = 7
End If
If optFang8 Then
PpiDef(FAPOINT, J) = 8
K = 8
End If
If optFang9 Then
PpiDef(FAPOINT, J) = 9
K = 9
End If
If optFang10 Then
PpiDef(FAPOINT, J) = 10
K = 10
End If

If frmMain.optPpi1 = True Then
frmMain.PpiFa1.Text = K
End If

If frmMain.optPpi2 = True Then
frmMain.PpiFa2.Text = K
End If

If frmMain.optPpi3 = True Then
frmMain.PpiFa3.Text = K
End If

If frmMain.optPpi4 = True Then
frmMain.PpiFa4.Text = K
End If





If ISAPMAC Then
PMACPORT.Output = "disable plc10" + Chr(13)
PMACPORT.Output = "disable plc20" + Chr(13)
End If
Screen.MousePointer = vbHourglass
'frmMain.Enabled = False
frmPlWait.Show
frmPlWait.Refresh
frmMain.EncodePpi
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

If pSamp < 16 Then
MsgBox "Must Have At Least 16 Samples", vbExclamation
WriteOut = False
Exit Function
End If

End Function

Private Sub cmdReturn_Click()
If (Swidth > 270) Then
MsgBox "Sweep width must be less then 270 Deg.", vbOK
Exit Sub
End If
frmFang.Hide
frmMain.Show
'frmMain.Enabled = True
frmMain.Show
frmMain.Refresh
If WriteOut Then
frmPPI.Hide
End If

frmMain.CmdDump.Caption = "Save Scans?"

End Sub

Private Sub Command1_Click()
End
End Sub

Private Sub Text4_Change()

End Sub


Private Sub Option1_DblClick()

End Sub


Private Sub Option2_DblClick()
End Sub


Private Sub Option3_Click()

End Sub

Private Sub Option3_DblClick()

End Sub


Private Sub Option4_DblClick()

End Sub






Private Sub Command2_Click()
MsgBox "Double Click on Button to edit Fixed Angle List", vbInformation
End Sub

Private Sub Command3_Click()
frmFang.Hide
frmPPI.Hide
frmMain.Show
frmMain.Refresh
'frmMain.Enabled = True
frmMain.Show
End Sub

Private Sub Form_Activate()
frmMain.Hide
J = 0
If frmMain.optPpi1 = True Then
lblPpiId.Caption = "Now Editing  " + frmMain.optPpi1.Caption
J = 1
End If

If frmMain.optPpi2 = True Then
lblPpiId.Caption = "Now Editing  " + frmMain.optPpi2.Caption
J = 2
End If

If frmMain.optPpi3 = True Then
lblPpiId.Caption = "Now Editing  " + frmMain.optPpi3.Caption
J = 3
End If

If frmMain.optPpi4 = True Then
lblPpiId.Caption = "Now Editing  " + frmMain.optPpi4.Caption
J = 4
End If

If J = 0 Then
lblPpiId.Caption = "Now Editing PPi Scan # ?"
Beep
MsgBox "Must Select One Ppi Volume", vbOKOnly + vbExclamation
frmPPI.Hide
frmMain.Show
Exit Sub
End If

txtAzRight.Text = PpiDef(AZR, J)
txtAzLeft.Text = PpiDef(AZL, J)
txtAzRate.Text = PpiDef(RATE, J)
txtSamples.Text = PpiDef(SAMPLES, J)


'Bring in the Old Values from the Array
AzRight = PpiDef(AZR, J)
AzLeft = PpiDef(AZL, J)
Azrate = PpiDef(RATE, J)
pSamp = PpiDef(SAMPLES, J)


If PpiDef(FAPOINT, J) = 1 Then
optFang1 = True
End If
If PpiDef(FAPOINT, J) = 2 Then
optFang2 = True
End If
If PpiDef(FAPOINT, J) = 3 Then
optFang3 = True
End If
If PpiDef(FAPOINT, J) = 4 Then
optFang4 = True
End If
If PpiDef(FAPOINT, J) = 5 Then
optFang5 = True
End If
If PpiDef(FAPOINT, J) = 6 Then
optFang6 = True
End If
If PpiDef(FAPOINT, J) = 7 Then
optFang7 = True
End If
If PpiDef(FAPOINT, J) = 8 Then
optFang8 = True
End If
If PpiDef(FAPOINT, J) = 9 Then
optFang9 = True
End If
If PpiDef(FAPOINT, J) = 10 Then
optFang10 = True
End If

Temp = AzRight - AzLeft
If Temp < 0 Then
Temp = 360 + Temp
End If
Swidth = Temp


Temp = AzLeft + Swidth / 2
If Temp > 360 Then
Temp = Temp - 360
End If

While Temp >= 360
Temp = Temp - 360
Wend

Center = Temp
txtCenter.Text = Str(Center)
txtWidth.Text = Str(Swidth)
End Sub





Private Sub optFAng1_DblClick()
'optFang1.BackColor = &HFF00&
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "1"

End Sub


Private Sub optFang10_DblClick()
'optFang1.BackColor = &HFF00&
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "10"

End Sub


Private Sub optFAng2_DblClick()
'optFang2.BackColor = &HFF00&
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "2"

End Sub




Private Sub optFAng3_DblClick()
'optFang3.BackColor = &HFF00&
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "3"

End Sub



Private Sub optFang4_DblClick()
'optFang1.BackColor = &HFF00&
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "4"

End Sub


Private Sub optFang5_DblClick()
'optFang1.BackColor = &HFF00&
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "5"

End Sub

Private Sub optFang6_DblClick()
'optFang1.BackColor = &HFF00&
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "6"

End Sub

Private Sub optFang7_DblClick()
'optFang1.BackColor = &HFF00&
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "7"

End Sub

Private Sub optFang8_DblClick()
'optFang1.BackColor = &HFF00&
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "8"

End Sub

Private Sub optFang9_DblClick()
'optFang1.BackColor = &HFF00&
'WriteOut
'Bring up the Fixed Angle Display
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "9"

End Sub

Private Sub txtAzLeft_Click()
Temp = InputBox("Enter Azimuth Left")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp > 360 Then
MsgBox "Must Be Between Zero and 360"
Exit Sub
End If

AzLeft = Temp
txtAzLeft.Text = Str(AzLeft)

Temp = AzRight - AzLeft
If Temp < 0 Then
Temp = 360 + Temp
End If
Swidth = Temp


Temp = AzLeft + Swidth / 2
If Temp > 360 Then
Temp = Temp - 360
End If

While Temp >= 360
Temp = Temp - 360
Wend

Center = Temp
txtCenter.Text = Str(Center)
txtWidth.Text = Str(Swidth)


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


Private Sub txtAzRight_Click()
Temp = InputBox("Enter Azimuth Right")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp > 360 Then
MsgBox "Must Be Between Zero and 360"
Exit Sub
End If

AzRight = Temp
txtAzRight.Text = Str(AzRight)
Temp = AzRight - AzLeft
If Temp < 0 Then
Temp = 360 + Temp
End If
Swidth = Temp


Temp = AzLeft + Swidth / 2
If Temp > 360 Then
Temp = Temp - 360
End If

While Temp >= 360
Temp = Temp - 360
Wend

Center = Temp



txtWidth.Text = Str(Swidth)
txtCenter.Text = Str(Center)


End Sub


Private Sub txtCenter_Click()
Temp = InputBox("Enter Center")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp > 360 Then
MsgBox "Must Be Between Zero and 360"
Exit Sub
End If
Rotate = Temp - Center
Center = Temp
txtCenter.Text = Str(Center)

AzRight = AzRight + Rotate
AzLeft = AzLeft + Rotate

If AzRight >= 360 Then
AzRight = AzRight - 360
End If

If AzLeft < 0 Then
AzLeft = AzLeft + 360
End If

If AzRight < 0 Then
AzRight = AzRight + 360
End If

If AzLeft >= 360 Then
AzLeft = AzLeft - 360
End If

txtAzRight.Text = Str(AzRight)
txtAzLeft.Text = Str(AzLeft)

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


If Temp < 16 Then
MsgBox "Must Be At Least 16"
Exit Sub
End If

pSamp = Temp
txtSamples.Text = Str(pSamp)


End Sub


Private Sub txtWidth_Click()
Temp = InputBox("Enter Width")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp > 360 Then
MsgBox "Must Be Between Zero and 360"
Exit Sub
End If

Cwidth = Temp - Swidth
Swidth = Temp

txtWidth.Text = Str(Swidth)

AzRight = AzRight + Cwidth / 2
AzLeft = AzLeft - Cwidth / 2

If AzRight >= 360 Then
AzRight = AzRight - 360
End If

If AzLeft < 0 Then
AzLeft = AzLeft + 360
End If

If AzRight < 0 Then
AzRight = AzRight + 360
End If

If AzLeft >= 360 Then
AzLeft = AzLeft - 360
End If


txtAzRight.Text = Str(AzRight)
txtAzLeft.Text = Str(AzLeft)



End Sub


