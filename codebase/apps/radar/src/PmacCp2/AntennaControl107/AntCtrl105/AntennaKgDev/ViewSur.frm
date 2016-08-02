VERSION 5.00
Begin VB.Form frmViewSur 
   BackColor       =   &H00C000C0&
   Caption         =   "View Surveillance  Scan"
   ClientHeight    =   4065
   ClientLeft      =   4905
   ClientTop       =   7065
   ClientWidth     =   3795
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   4065
   ScaleWidth      =   3795
   Begin VB.TextBox txtBw 
      Enabled         =   0   'False
      Height          =   285
      Left            =   1080
      TabIndex        =   11
      Text            =   "0.00"
      Top             =   2880
      Width           =   495
   End
   Begin VB.TextBox txtTime 
      Enabled         =   0   'False
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
      Left            =   960
      TabIndex        =   7
      Text            =   "0000"
      Top             =   2160
      Width           =   612
   End
   Begin VB.CommandButton Command1 
      Caption         =   "ok"
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
      Left            =   2400
      TabIndex        =   5
      Top             =   3360
      Width           =   1215
   End
   Begin VB.TextBox txtAzRate 
      Enabled         =   0   'False
      Height          =   300
      Left            =   1080
      TabIndex        =   4
      Text            =   "*"
      Top             =   840
      Width           =   800
   End
   Begin VB.TextBox txtSamp 
      Enabled         =   0   'False
      Height          =   300
      Left            =   1080
      TabIndex        =   3
      Text            =   "*"
      Top             =   1440
      Width           =   800
   End
   Begin VB.ListBox lstViewFa 
      Enabled         =   0   'False
      Height          =   2595
      ItemData        =   "ViewSur.frx":0000
      Left            =   2640
      List            =   "ViewSur.frx":0002
      TabIndex        =   2
      Top             =   120
      Width           =   735
   End
   Begin VB.Label Label6 
      BackColor       =   &H00C000C0&
      Caption         =   "Beamwidth"
      Height          =   255
      Left            =   120
      TabIndex        =   10
      Top             =   2880
      Width           =   855
   End
   Begin VB.Label Label2 
      BackColor       =   &H00C000C0&
      Caption         =   "Sec."
      Height          =   252
      Left            =   1920
      TabIndex        =   9
      Top             =   2280
      Width           =   492
   End
   Begin VB.Label Label1 
      BackColor       =   &H00C000C0&
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
      Height          =   615
      Left            =   120
      TabIndex        =   8
      Top             =   2160
      Width           =   735
   End
   Begin VB.Label lblSurId 
      BackColor       =   &H00C000C0&
      Caption         =   "Sur Scan # ?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   375
      Left            =   120
      TabIndex        =   6
      Top             =   120
      Width           =   1695
   End
   Begin VB.Label Label4 
      BackColor       =   &H00C000C0&
      Caption         =   "Az Rate"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   300
      Left            =   0
      TabIndex        =   1
      Top             =   840
      Width           =   1215
   End
   Begin VB.Label Label3 
      BackColor       =   &H00C000C0&
      Caption         =   "Samples"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   300
      Left            =   0
      TabIndex        =   0
      Top             =   1440
      Width           =   1095
   End
End
Attribute VB_Name = "frmViewSur"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private Sub Command1_Click()
frmViewSur.Hide
End Sub

Private Sub Form_Activate()
Dim J As Integer, I As Integer, Count As Integer, M As Integer
Dim Time As Single
J = 0
If frmMain.optSur1.Value = True Then
lblSurId.Caption = frmMain.optSur1.Caption
J = 1
End If

If frmMain.optSur2.Value = True Then
lblSurId.Caption = frmMain.optSur2.Caption
J = 2
End If

If frmMain.optSur3.Value = True Then
lblSurId.Caption = frmMain.optSur3.Caption
J = 3
End If

If J = 0 Then
lblSurId.Caption = "Sur Scan # ?"
Beep
MsgBox "Must Select One Sur Volume", vbExclamation
frmViewSur.Hide
Exit Sub
End If



txtAzRate = SurDef(RATE, J)
txtSamp = SurDef(SAMPLES, J)
'beamwidth display
If (SurDef(SAMPLES, J) > 0) Then
txtBw.Text = SurDef(RATE, J) / (960 / SurDef(SAMPLES, J))
Else
txtBw = 0#
End If

M = SurDef(FAPOINT, J)

If M = 0 Then
lstViewFa.Clear
txtTime.Text = "0000"
MsgBox "No Fixed Angle List Selected", vbExclamation
frmViewSur.Hide
Exit Sub
End If

lstViewFa.Clear
Count = 0
For I = 0 To 29
If FixedAng(I, M) <> -999 Then
lstViewFa.AddItem Str(FixedAng(I, M))
Count = Count + 1
Else
lstViewFa.AddItem "end"
GoTo Out
End If
Next I
Out:
'volume time
If (SurDef(RATE, J) > 0) Then
Time = 5 + Count * ((360 / SurDef(RATE, J)) + 1) 'sweep time +1 sec transition+5
txtTime.Text = Time
End If


End Sub

