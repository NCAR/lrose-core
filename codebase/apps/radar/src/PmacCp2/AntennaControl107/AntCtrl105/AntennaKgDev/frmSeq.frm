VERSION 5.00
Begin VB.Form frmSeq 
   BackColor       =   &H00000080&
   Caption         =   "Sequence Editor"
   ClientHeight    =   4020
   ClientLeft      =   7590
   ClientTop       =   6690
   ClientWidth     =   7680
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   12
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   LinkTopic       =   "Form1"
   ScaleHeight     =   4020
   ScaleWidth      =   7680
   Begin VB.CommandButton SynMod 
      Caption         =   "Command1"
      Height          =   615
      Left            =   600
      TabIndex        =   11
      Top             =   2160
      Width           =   855
   End
   Begin VB.CommandButton cmdCancel 
      BackColor       =   &H000000FF&
      Caption         =   "Cancel"
      Height          =   492
      Left            =   3960
      Style           =   1  'Graphical
      TabIndex        =   9
      Top             =   3360
      Width           =   1215
   End
   Begin VB.CommandButton cmdInsert 
      BackColor       =   &H00FFFF00&
      Caption         =   "INSERT"
      Height          =   495
      Left            =   3840
      Style           =   1  'Graphical
      TabIndex        =   8
      Top             =   1440
      Width           =   1455
   End
   Begin VB.CommandButton cmdDelete 
      BackColor       =   &H00FF00FF&
      Caption         =   "DELETE"
      Height          =   495
      Left            =   3840
      Style           =   1  'Graphical
      TabIndex        =   7
      Top             =   2160
      Width           =   1455
   End
   Begin VB.CommandButton cmdAdd 
      BackColor       =   &H0000FFFF&
      Caption         =   "MODIFY"
      Height          =   495
      Left            =   3840
      Style           =   1  'Graphical
      TabIndex        =   6
      Top             =   720
      Width           =   1455
   End
   Begin VB.ListBox lstSelect 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   2205
      Left            =   5400
      TabIndex        =   5
      Top             =   720
      Width           =   300
   End
   Begin VB.CommandButton cmdDone 
      BackColor       =   &H0000C000&
      Caption         =   "Enter"
      Height          =   495
      Left            =   6120
      Style           =   1  'Graphical
      TabIndex        =   4
      Top             =   3360
      Width           =   975
   End
   Begin VB.ListBox lstseq 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   2595
      Left            =   6000
      TabIndex        =   2
      Top             =   600
      Width           =   1575
   End
   Begin VB.ListBox lstScans 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   2700
      Left            =   2280
      TabIndex        =   0
      Top             =   480
      Width           =   1455
   End
   Begin VB.Label SMod 
      Caption         =   "Sychronization Modulo    (Min Past The Hour)"
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
      TabIndex        =   10
      Top             =   1200
      Width           =   2055
   End
   Begin VB.Label lblSeqedit1 
      Caption         =   "Sequence 1"
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
      Left            =   5640
      TabIndex        =   3
      Top             =   120
      Width           =   1935
   End
   Begin VB.Label Label1 
      Caption         =   "Available Scans"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   2040
      TabIndex        =   1
      Top             =   120
      Width           =   1815
   End
End
Attribute VB_Name = "frmSeq"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub cmdAdd_Click()
Dim M, J, K As Integer
Dim Name
M = lstSelect.ListIndex
If M < 0 Then
    MsgBox "Must Select a Sequence Entry", vbExclamation
    Exit Sub
End If

If M > lstseq.ListCount Then
    M = lstseq.ListCount
End If

J = lstScans.ListIndex
If J < 0 Then
    MsgBox "Must Select a Scan Entry", vbExclamation
    Exit Sub
End If

'Modify the Entry
lstseq.List(M) = lstScans.List(J)

'Remove all the "loop"s
For M = 0 To lstseq.ListCount
If lstseq.List(M) = "LOOP" Then
    lstseq.RemoveItem M
End If
If lstseq.List(M) = "-" Then
    lstseq.RemoveItem M
End If
Next M

'put in a "loop" at the bottom
M = lstseq.ListCount
Do While M >= 1
M = M - 1
If lstseq.List(M) <> "-" Then
    If lstseq.List(M) <> "LOOP" Then
        lstseq.List(M + 1) = "LOOP"
        Exit Do
    End If
End If
Loop
End Sub

Private Sub cmdCancel_Click()
frmSeq.Hide
frmMain.Show
'frmMain.Enabled = True

End Sub

Private Sub cmdDelete_Click()
Dim M
M = lstSelect.ListIndex
If M < 0 Then
    MsgBox "Must Select a Sequence Entry", vbExclamation
    Exit Sub
End If
If M <= lstseq.ListCount Then
    If lstseq.List(M) <> "LOOP" Then
        lstseq.RemoveItem M
    End If
End If
End Sub

Private Sub cmdDone_Click()
Dim I, J, K As Integer

If frmMain.optSeq1 Then
    lblSeqedit1.Caption = frmMain.optSeq1.Caption
    K = 1
End If

If frmMain.optseq2 Then
    lblSeqedit1.Caption = frmMain.optseq2.Caption
    K = 2
End If

If frmMain.optSeq3 Then
    lblSeqedit1.Caption = frmMain.optSeq3.Caption
    K = 3
End If

If frmMain.optseq4 Then
    lblSeqedit1.Caption = frmMain.optseq4.Caption
    K = 4
End If

If frmMain.optseq5 Then
    lblSeqedit1.Caption = frmMain.optseq5.Caption
    K = 5
End If

If frmMain.optseq6 Then
    lblSeqedit1.Caption = frmMain.optseq6.Caption
    K = 6
End If

If frmMain.optseq7 Then
    lblSeqedit1.Caption = frmMain.optseq7.Caption
    K = 7
End If

'If frmMain.optseq8 Then
'   lblSeqedit1.Caption = frmMain.optseq8.Caption
'   K = 1
'End If


'put sync modulo  into the array
SynDef(STYPE, K) = 5
SynDef(SModulo, K) = Val(SynMod.Caption)


'Match the names and encode seqdef
For I = 0 To lstseq.ListCount - 1
    For J = 0 To 11
        If lstseq.List(I) = lstScans.List(J) Then
            SeqDef(I, K) = J + 1
        End If
        If lstseq.List(I) = "LOOP" Then
            SeqDef(I, K) = -2
        End If
    Next J
Next I

If ISAPMAC And Not SemTakePMAC(800) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "disable plc10" + Chr(13)
    PMACPORT.Output = "disable plc20" + Chr(13)
End If
frmMain.EncodeSeq
If ISAPMAC Then
    PMACPORT.Output = "enable plc10" + Chr(13)
    PMACPORT.Output = "enable plc20" + Chr(13)
    SemGivePMAC (800)
End If
frmMain.Show
'frmMain.Enabled = True
frmSeq.Hide
frmMain.CmdDump.Caption = "Save Scans?"
End Sub

Private Sub cmdInsert_Click()
Dim M, J, K As Integer
Dim Name
M = lstSelect.ListIndex
If M < 0 Then
    MsgBox "Must Select a Sequence Entry", vbExclamation
    Exit Sub
End If
J = lstScans.ListIndex
If J < 0 Then
    MsgBox "Must Select a Scan Entry", vbExclamation
    Exit Sub
End If

'insert the item
If M <= lstseq.ListCount - 1 Then
    If lstseq.ListCount <= 9 Then
        lstseq.AddItem (lstScans.List(J)), M
    End If
End If

'get rid of any "-" 's
For M = 0 To 10
    If lstseq.List(M) = "-" Then
        lstseq.RemoveItem M
    End If
Next M



'put in a "loop" at the bottom
M = lstseq.ListCount
Do While M >= 1
    M = M - 1
    If lstseq.List(M) <> "-" Then
        If lstseq.List(M) <> "LOOP" Then
            lstseq.List(M + 1) = "LOOP"
            Exit Do
        End If
    End If
Loop


End Sub

Private Sub cmdLoop_Click()
Dim M As Integer
M = lstSelect.ListIndex
If M < 0 Then
    MsgBox "Must Select a Sequence Entry", vbExclamation
    Exit Sub
End If
lstseq.List(M) = "LOOP"

End Sub

Private Sub Form_Activate()
Dim I, J, K, L As Integer
frmMain.Hide
lstScans.Clear
lstseq.Clear
lstSelect.Clear

lstScans.AddItem frmMain.optPpi1.Caption
lstScans.AddItem frmMain.optPpi2.Caption
lstScans.AddItem frmMain.optPpi3.Caption
lstScans.AddItem frmMain.optPpi4.Caption
lstScans.AddItem frmMain.optRhi1.Caption
lstScans.AddItem frmMain.optRhi2.Caption
lstScans.AddItem frmMain.optRhi3.Caption
lstScans.AddItem frmMain.optSur1.Caption
lstScans.AddItem frmMain.optSur2.Caption
lstScans.AddItem frmMain.optSur3.Caption
lstScans.AddItem "Sync"


For I = 1 To 10
    lstSelect.AddItem "*"
    lstseq.AddItem "-"
Next I
K = 0

If frmMain.optSeq1 Then
    lblSeqedit1.Caption = frmMain.optSeq1.Caption
    K = 1
End If

If frmMain.optseq2 Then
    lblSeqedit1.Caption = frmMain.optseq2.Caption
    K = 2
End If

If frmMain.optSeq3 Then
    lblSeqedit1.Caption = frmMain.optSeq3.Caption
K = 3
End If

If frmMain.optseq4 Then
    lblSeqedit1.Caption = frmMain.optseq4.Caption
    K = 4
End If

If frmMain.optseq5 Then
    lblSeqedit1.Caption = frmMain.optseq5.Caption
    K = 5
End If

If frmMain.optseq6 Then
    lblSeqedit1.Caption = frmMain.optseq6.Caption
    K = 6
End If

If frmMain.optseq7 Then
    lblSeqedit1.Caption = frmMain.optseq7.Caption
    K = 7
End If

'If frmMain.optseq8 Then
'   lblSeqedit1.Caption = frmMain.optseq8.Caption
'   K = 1
'End If

If K = 0 Then
    MsgBox "Must Select a Sequence", vbExclamation
    frmSeq.Hide
    frmMain.Show
    'frmMain.Enabled = True
    Exit Sub
End If

'load the Seq list from the data
For J = 0 To 9
    L = SeqDef(J, K) - 1
    If L < 0 Then
        lstseq.List(J) = "LOOP"
        Exit For
    End If
    lstseq.List(J) = lstScans.List(L)
Next J
'load the sync modulo
SynMod.Caption = SynDef(SModulo, K)

End Sub

Private Sub Text1_Change()

End Sub

Private Sub TxtSynMod_Change()

End Sub

Private Sub SynMod_Click()
Dim MinMod


MinMod = InputBox("Enter Modulo Minutes Past The Hour for Synchronization")

If MinMod = "" Then
    Exit Sub
End If

If Not IsNumeric(MinMod) Then
    MsgBox "Invalid Number"
    Exit Sub
End If

If (MinMod > 59) Or (MinMod < 1) Then
    MsgBox "Must Be Between 1 and 59"
    Exit Sub
End If
SynMod.Caption = MinMod


End Sub
