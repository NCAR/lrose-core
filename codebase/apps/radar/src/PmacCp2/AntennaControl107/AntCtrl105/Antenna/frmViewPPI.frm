VERSION 5.00
Begin VB.Form frmViewPPI 
   BackColor       =   &H000000FF&
   Caption         =   "View PPI"
   ClientHeight    =   3585
   ClientLeft      =   9255
   ClientTop       =   2685
   ClientWidth     =   5040
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3585
   ScaleWidth      =   5040
   Begin VB.TextBox txtBw 
      Height          =   285
      Left            =   1080
      TabIndex        =   15
      Text            =   "0.00"
      Top             =   3240
      Width           =   495
   End
   Begin VB.TextBox txtTime 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   372
      Left            =   840
      TabIndex        =   12
      Text            =   "0000"
      Top             =   2520
      Width           =   612
   End
   Begin VB.CommandButton Command1 
      Caption         =   "OK"
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
      Left            =   2520
      TabIndex        =   9
      Top             =   2880
      Width           =   1215
   End
   Begin VB.TextBox txtAzLeft 
      Height          =   300
      Left            =   1080
      TabIndex        =   8
      Text            =   "*"
      Top             =   600
      Width           =   800
   End
   Begin VB.TextBox txtAzRate 
      Height          =   300
      Left            =   1080
      TabIndex        =   7
      Text            =   "*"
      Top             =   1320
      Width           =   800
   End
   Begin VB.TextBox txtSamp 
      Height          =   300
      Left            =   1080
      TabIndex        =   6
      Text            =   "*"
      Top             =   1680
      Width           =   800
   End
   Begin VB.TextBox txtAzRight 
      Height          =   300
      Left            =   1080
      TabIndex        =   5
      Text            =   "*"
      Top             =   960
      Width           =   800
   End
   Begin VB.ListBox lstViewFa 
      Height          =   2595
      ItemData        =   "frmViewPPI.frx":0000
      Left            =   3840
      List            =   "frmViewPPI.frx":0002
      TabIndex        =   4
      Top             =   120
      Width           =   735
   End
   Begin VB.Label Label7 
      BackColor       =   &H000000FF&
      Caption         =   "Beamwidth"
      Height          =   255
      Left            =   120
      TabIndex        =   14
      Top             =   3240
      Width           =   855
   End
   Begin VB.Label Label6 
      BackColor       =   &H000000FF&
      Caption         =   "Sec."
      Height          =   252
      Left            =   1560
      TabIndex        =   13
      Top             =   2640
      Width           =   492
   End
   Begin VB.Label Label5 
      BackColor       =   &H000000FF&
      Caption         =   "Approx. Time"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   612
      Left            =   0
      TabIndex        =   11
      Top             =   2400
      Width           =   732
   End
   Begin VB.Label lblPpiId 
      Caption         =   "PPI SCan # ?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   0
      TabIndex        =   10
      Top             =   120
      Width           =   1575
   End
   Begin VB.Label Label4 
      Caption         =   "Az Rate"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   0
      TabIndex        =   3
      Top             =   1320
      Width           =   1215
   End
   Begin VB.Label Label3 
      Caption         =   "Samples"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   0
      TabIndex        =   2
      Top             =   1680
      Width           =   1095
   End
   Begin VB.Label Label2 
      Caption         =   "Az Right"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   0
      TabIndex        =   1
      Top             =   960
      Width           =   1095
   End
   Begin VB.Label Label1 
      Caption         =   "Az Left"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   300
      Left            =   0
      TabIndex        =   0
      Top             =   600
      Width           =   855
   End
End
Attribute VB_Name = "frmViewPPI"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private Sub Command1_Click()
frmViewPPI.Hide
End Sub

Private Sub Form_Activate()
Dim J, I, K As Integer
Dim Time As Single, Count As Integer, Swww As Single
J = 0
If frmMain.optPpi1.Value = True Then
J = 1
lblPpiId.Caption = frmMain.optPpi1.Caption
End If

If frmMain.optPpi2.Value = True Then
J = 2
lblPpiId.Caption = frmMain.optPpi2.Caption
End If

If frmMain.optPpi3.Value = True Then
J = 3
lblPpiId.Caption = frmMain.optPpi3.Caption
End If

If frmMain.optPpi4.Value = True Then
J = 4
lblPpiId.Caption = frmMain.optPpi4.Caption
End If

If J = 0 Then
lblPpiId.Caption = "PPi Scan # ?"
Beep
MsgBox "Must Select One Ppi Volume", vbExclamation
frmViewPPI.Hide
Exit Sub
End If



txtAzRight.Text = PpiDef(AZR, J)
txtAzLeft.Text = PpiDef(AZL, J)
txtAzRate.Text = PpiDef(RATE, J)
txtSamp.Text = PpiDef(SAMPLES, J)

'beamwidth display
If (PpiDef(SAMPLES, J) > 0) Then
txtBw.Text = PpiDef(RATE, J) / (960 / PpiDef(SAMPLES, J))
Else
txtBw.Text = 0#
End If
K = PpiDef(FAPOINT, J)

If K = 0 Then
lstViewFa.Clear
txtTime.Text = "0000"
MsgBox "No Fixed Angle List Selected", vbExclamation
frmViewPPI.Hide
Exit Sub
End If
Count = 0
If K <> 0 Then
lstViewFa.Clear
For I = 0 To 29
If FixedAng(I, K) > -998 Then
lstViewFa.AddItem Str(FixedAng(I, K))
Count = Count + 1
Else
lstViewFa.AddItem "end"
GoTo Out
End If
Next I
End If
Out:

'volume time
'turnaround
If (PpiDef(RATE, J) > 0) Then
Swww = PpiDef(AZR, J) - PpiDef(AZL, J)
If (Swww < 0) Then
Swww = 360# + Swww
End If
Time = Count * PpiDef(RATE, J) / 5# 'time/turnaround
'add sweep time
Time = Time + Count * (Swww / PpiDef(RATE, J))
' add startup
Time = Time + 5#
txtTime.Text = Time
Else
txtTime.Text = "Inf"
End If
End Sub

