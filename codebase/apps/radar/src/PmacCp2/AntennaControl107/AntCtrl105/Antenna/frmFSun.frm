VERSION 5.00
Begin VB.Form frmFSun 
   BackColor       =   &H0000FF00&
   Caption         =   "Find The Sun"
   ClientHeight    =   3120
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6915
   LinkTopic       =   "Form1"
   ScaleHeight     =   3120
   ScaleWidth      =   6915
   Begin VB.TextBox txtElo 
      Height          =   285
      Left            =   5400
      TabIndex        =   11
      Text            =   "Text1"
      Top             =   2280
      Width           =   615
   End
   Begin VB.CommandButton cmddone 
      Caption         =   "Done"
      Height          =   495
      Left            =   2160
      TabIndex        =   9
      Top             =   2520
      Width           =   855
   End
   Begin VB.CommandButton cmdstop 
      Caption         =   "Stop"
      Height          =   495
      Left            =   1080
      TabIndex        =   8
      Top             =   2520
      Width           =   855
   End
   Begin VB.CommandButton cmdgoo 
      Caption         =   "GO"
      Height          =   495
      Left            =   120
      TabIndex        =   7
      Top             =   2520
      Width           =   735
   End
   Begin VB.TextBox txtWidth 
      Height          =   285
      Left            =   2880
      TabIndex        =   6
      Text            =   "Text1"
      Top             =   1320
      Width           =   735
   End
   Begin VB.TextBox txtCent 
      Height          =   285
      Left            =   2880
      TabIndex        =   5
      Text            =   "Text1"
      Top             =   480
      Width           =   735
   End
   Begin VB.VScrollBar VScrollElO 
      Height          =   2775
      Left            =   6240
      Max             =   100
      TabIndex        =   2
      Top             =   120
      Width           =   375
   End
   Begin VB.HScrollBar HScrollWd 
      Height          =   255
      Left            =   600
      Max             =   45
      Min             =   10
      TabIndex        =   1
      Top             =   960
      Value           =   10
      Width           =   5175
   End
   Begin VB.HScrollBar HScrollCn 
      Height          =   255
      Left            =   600
      Max             =   359
      TabIndex        =   0
      Top             =   120
      Value           =   45
      Width           =   5175
   End
   Begin VB.Label Label3 
      Caption         =   "El Offset"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   4080
      TabIndex        =   10
      Top             =   2280
      Width           =   1095
   End
   Begin VB.Label Label2 
      Caption         =   "Width"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   1680
      TabIndex        =   4
      Top             =   1320
      Width           =   855
   End
   Begin VB.Label Label1 
      Caption         =   "Center"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   1800
      TabIndex        =   3
      Top             =   480
      Width           =   855
   End
End
Attribute VB_Name = "frmFSun"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim Cn As Single, Wd As Single
Dim Azl As Single, Azr As Single
Dim Azv As Single





Private Sub cmdDone_Click()
If FSF Then
MsgBox "Stop First"
Exit Sub
End If
frmSolar.CmdRunScan.Visible = True
frmSolar.CmdRunOff.Visible = True
frmSolar.CmdFlip.Visible = True
frmSolar.CmdOuttaHere.Visible = True
frmSolar.cmdExpand.Visible = True
Unload frmFSun

End Sub

Private Sub cmdgoo_Click()
 PMACPORT.Output = "p50=4"                        'start it
 FSF = True
End Sub

Private Sub cmdstop_Click()
FSF = False
End Sub

Private Sub Form_Load()
PMACPORT.Output = "p50=0=" + Chr(13)
FSF = False
frmSolar.CmdRunScan.Visible = False
frmSolar.CmdRunOff.Visible = False
frmSolar.CmdFlip.Visible = False
frmSolar.CmdOuttaHere.Visible = False
frmSolar.cmdExpand.Visible = False
HScrollCn.Value = 180
Cn = HScrollCn.Value
txtCent.Text = Cn
HScrollWd.Value = 45
Wd = HScrollWd.Value
txtWidth.Text = Wd
VScrollElO.Value = 50
ELO = (50 - VScrollElO.Value) / 10
txtElo.Text = ELO
Azl = Cn + Wd / 2
Azr = Cn - Wd / 2
sdelay (2)
PMACPORT.Output = "p106=" + Format(Azl) + Chr(13)
PMACPORT.Output = "p114=" + Format(Azr) + Chr(13)
Azv = 15
 PMACPORT.Output = "p126=" + Format(Azv) + Chr(13)
 End Sub

Private Sub HScrollcn_Change()

Cn = HScrollCn.Value
txtCent.Text = Cn
Azl = Cn + Wd / 2
Azr = Cn - Wd / 2
PMACPORT.Output = "p106=" + Format(Azl) + Chr(13)
PMACPORT.Output = "p114=" + Format(Azr) + Chr(13)
End Sub

Private Sub HScrollWd_Change()
Wd = HScrollWd.Value
txtWidth.Text = Wd
Azl = Cn + Wd / 2
Azr = Cn - Wd / 2
PMACPORT.Output = "p106=" + Format(Azl) + Chr(13)
PMACPORT.Output = "p114=" + Format(Azr) + Chr(13)
Azv = 4
If (Wd > 15) Then
Azv = 6
End If
If (Wd > 25) Then
Azv = 10
End If
If (Wd > 35) Then
Azv = 14
End If
 PMACPORT.Output = "p126=" + Format(Azv) + Chr(13)
 End Sub

Private Sub VScrollElO_Change()
ELO = (50 - VScrollElO.Value) / 10
txtElo.Text = ELO
End Sub
