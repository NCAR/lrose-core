VERSION 5.00
Begin VB.Form frmViewRhi 
   BackColor       =   &H0000C000&
   Caption         =   "View Rhi"
   ClientHeight    =   3930
   ClientLeft      =   375
   ClientTop       =   6030
   ClientWidth     =   4425
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3930
   ScaleWidth      =   4425
   Begin VB.TextBox txtBw 
      Height          =   285
      Left            =   1200
      TabIndex        =   15
      Text            =   "0.00"
      Top             =   2880
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
      Left            =   1080
      TabIndex        =   12
      Text            =   "0000"
      Top             =   2280
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
      Left            =   1920
      TabIndex        =   9
      Top             =   3240
      Width           =   1215
   End
   Begin VB.TextBox txtElTop 
      Height          =   300
      Left            =   1080
      TabIndex        =   8
      Text            =   "*"
      Top             =   600
      Width           =   800
   End
   Begin VB.TextBox txtElRate 
      Height          =   300
      Left            =   1080
      TabIndex        =   7
      Text            =   "*"
      Top             =   1320
      Width           =   800
   End
   Begin VB.TextBox txtSamples 
      Height          =   300
      Left            =   1080
      TabIndex        =   6
      Text            =   "*"
      Top             =   1800
      Width           =   800
   End
   Begin VB.TextBox txtElbot 
      Height          =   300
      Left            =   1080
      TabIndex        =   5
      Text            =   "*"
      Top             =   960
      Width           =   800
   End
   Begin VB.ListBox lstViewFa 
      Height          =   3375
      ItemData        =   "ViewRhi.frx":0000
      Left            =   3480
      List            =   "ViewRhi.frx":0002
      TabIndex        =   4
      Top             =   240
      Width           =   735
   End
   Begin VB.Label Label7 
      BackColor       =   &H0000C000&
      Caption         =   "Beamwidth"
      Height          =   255
      Left            =   240
      TabIndex        =   14
      Top             =   2880
      Width           =   855
   End
   Begin VB.Label Label6 
      BackColor       =   &H0000C000&
      Caption         =   "Sec."
      Height          =   252
      Left            =   1920
      TabIndex        =   13
      Top             =   2400
      Width           =   492
   End
   Begin VB.Label Label5 
      BackColor       =   &H0000C000&
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
      Left            =   240
      TabIndex        =   11
      Top             =   2160
      Width           =   732
   End
   Begin VB.Label lblRhiId 
      BackColor       =   &H0000C000&
      Caption         =   "RHI Scan # ?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FF0000&
      Height          =   255
      Left            =   120
      TabIndex        =   10
      Top             =   120
      Width           =   1695
   End
   Begin VB.Label Label4 
      BackColor       =   &H0000C000&
      Caption         =   "El Rate"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FF0000&
      Height          =   300
      Left            =   0
      TabIndex        =   3
      Top             =   1320
      Width           =   1215
   End
   Begin VB.Label Label3 
      BackColor       =   &H0000C000&
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
      ForeColor       =   &H00FF0000&
      Height          =   300
      Left            =   0
      TabIndex        =   2
      Top             =   1680
      Width           =   1095
   End
   Begin VB.Label Label2 
      BackColor       =   &H0000C000&
      Caption         =   "El Bot"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FF0000&
      Height          =   300
      Left            =   0
      TabIndex        =   1
      Top             =   960
      Width           =   1095
   End
   Begin VB.Label Label1 
      BackColor       =   &H0000C000&
      Caption         =   "El Top"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FF0000&
      Height          =   300
      Left            =   0
      TabIndex        =   0
      Top             =   600
      Width           =   855
   End
End
Attribute VB_Name = "frmViewRhi"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private Sub Command1_Click()
frmViewRhi.Hide
End Sub

Private Sub Form_Activate()
Dim J As Integer, I As Integer, M As Integer
Dim Time As Single, Count As Integer
J = 0

If frmMain.optRhi1.Value = True Then
lblRhiId.Caption = frmMain.optRhi1.Caption
J = 1
End If

If frmMain.optRhi2.Value = True Then
lblRhiId.Caption = frmMain.optRhi2.Caption
J = 2
End If

If frmMain.optRhi3.Value = True Then
lblRhiId.Caption = frmMain.optRhi3.Caption
J = 3
End If

If J = 0 Then
lblRhiId.Caption = "RHI Scan # ?"
Beep
MsgBox "Must Select One RHI Volume", vbExclamation
frmViewRhi.Hide
Exit Sub
End If




txtElTop = RhiDef(ELT, J)
txtElbot = RhiDef(ELB, J)
txtElRate = RhiDef(RATE, J)
txtSamples = RhiDef(SAMPLES, J)
'beamwidth display
If (RhiDef(SAMPLES, J) > 0) Then
txtBw.Text = RhiDef(RATE, J) / (840 / RhiDef(SAMPLES, J)) 'assumes fixed PRF
Else
txtBw.Text = 0#
End If

M = RhiDef(FAPOINT, J)

If M = 0 Then
lstViewFa.Clear
txtTime.Text = "0000"
MsgBox "No Fixed Angle List Selected", vbExclamation
frmViewRhi.Hide
Exit Sub
End If
Count = 0
lstViewFa.Clear
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
'turnaround
If (RhiDef(RATE, J) > 0) Then
Time = Count * RhiDef(RATE, J) / 5# 'time/turnaround
'add sweep time
Time = Time + Count * ((RhiDef(ELT, J) - RhiDef(ELB, J)) / RhiDef(RATE, J))
' add startup
Time = Time + 5#
txtTime.Text = Time
Else
txtTime.Text = "Inf"
End If
End Sub

