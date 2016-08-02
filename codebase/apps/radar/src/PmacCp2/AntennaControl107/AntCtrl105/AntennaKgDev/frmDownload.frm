VERSION 5.00
Begin VB.Form frmDownload 
   Caption         =   "Download"
   ClientHeight    =   3195
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   3195
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command1 
      Caption         =   "Bye"
      Height          =   375
      Left            =   3600
      TabIndex        =   4
      Top             =   2640
      Width           =   615
   End
   Begin VB.TextBox txtrunning 
      Height          =   495
      Left            =   360
      TabIndex        =   2
      Text            =   "Text1"
      Top             =   720
      Width           =   1455
   End
   Begin VB.CommandButton cmdDownGo 
      Caption         =   "Start Download"
      Height          =   735
      Left            =   2760
      TabIndex        =   1
      Top             =   480
      Width           =   1575
   End
   Begin VB.TextBox txtDownload 
      Height          =   375
      Left            =   600
      TabIndex        =   0
      Text            =   "Text1"
      Top             =   1800
      Width           =   3135
   End
   Begin VB.Label Label1 
      Caption         =   "Label1"
      Height          =   495
      Left            =   960
      TabIndex        =   3
      Top             =   120
      Width           =   1215
   End
End
Attribute VB_Name = "frmDownload"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub cmdDownGo_Click()

Dim Astring As String * 50
On Error GoTo oops
'Unload frmMain
txtrunning.Text = "Running"
txtrunning.Refresh
Open "precip98.cfg" For Input As #3
If ISAPMAC And Not SemTakePMAC(9000) Then
    GoTo skipPmac
ElseIf ISAPMAC Then
    PMACPORT.Output = "i5=0" + Chr(13)
    SDelay (2)
End If

Do While 2 > 1
    Input #3, Astring
    txtDownload.Text = Astring
    txtDownload.Refresh
   If (ISAPMAC) Then PMACPORT.Output = Astring + Chr(13)

Loop
oops:
If (Err.Number = 62) Then



txtrunning.Text = "Done"
Else
txtrunning.Text = Err.Number
End If

Err.Clear
Close #3

If ISAPMAC Then
    If Not PMACBlocked Then
        PMACPORT.Output = "i5=3" + Chr(13)
        PMACPORT.Output = "enable plc0" + Chr(13)
        PMACPORT.Output = "enable plc10" + Chr(13)
        PMACPORT.Output = "enable plc20" + Chr(13)
    End If
    SemGivePMAC (9000)
End If
skipPmac:
frmMain.Show
End Sub





Private Sub Command1_Click()
frmDownload.Hide
End Sub

Private Sub Form_Load()
txtrunning.Text = "Ready"
End Sub

