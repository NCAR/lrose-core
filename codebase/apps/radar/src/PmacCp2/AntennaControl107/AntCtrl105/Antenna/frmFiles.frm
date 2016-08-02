VERSION 5.00
Begin VB.Form frmFiles 
   BackColor       =   &H00C0E0FF&
   Caption         =   "Change Scan Files"
   ClientHeight    =   4260
   ClientLeft      =   4065
   ClientTop       =   8640
   ClientWidth     =   6045
   LinkTopic       =   "Form1"
   ScaleHeight     =   4260
   ScaleWidth      =   6045
   StartUpPosition =   1  'CenterOwner
   Begin VB.ListBox LstScanFile 
      Height          =   3570
      Left            =   4680
      Sorted          =   -1  'True
      TabIndex        =   3
      Top             =   600
      Width           =   1215
   End
   Begin VB.CommandButton CmdBye 
      Caption         =   "Bye"
      Height          =   375
      Left            =   3360
      TabIndex        =   2
      Top             =   2640
      Width           =   975
   End
   Begin VB.CommandButton cmdOpnFile 
      Caption         =   "Open  Existing Scan File"
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
      Left            =   360
      TabIndex        =   1
      Top             =   1560
      Width           =   2775
   End
   Begin VB.CommandButton cmdNewFile 
      Caption         =   "Create and Open A New Scan File"
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
      Left            =   360
      TabIndex        =   0
      Top             =   480
      Width           =   2655
   End
   Begin VB.Label Label1 
      BackColor       =   &H00C0E0FF&
      Caption         =   "*.scn  Files"
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
      Left            =   4440
      TabIndex        =   4
      Top             =   120
      Width           =   1575
   End
End
Attribute VB_Name = "frmFiles"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Private Sub cmdBye_Click()
frmFiles.Hide
'frmMain.Enabled = True
frmMain.Show
End Sub

Private Sub cmdNewFile_Click()
Dim Name As String
Dim J As Single
Name = MsgBox("Loading a New File Will Corrupt Running Scans", vbOKCancel, "Be Careful!")
If Name = 2 Then
Exit Sub
End If
If (ISRUNNING) Then
Beep
MsgBox "Stop Antenna First", vbOKOnly
Exit Sub
End If
Name = InputBox("Enter New Scan Data File name: use .scn extension")
If Name = "" Then
Exit Sub
End If

J = MsgBox("Save and Overwrite Currently Open  File?", vbYesNo)
If J = vbYes Then
frmMain.SaveToDsk
End If



WKFILE = Name
On Error GoTo oops
Open WKFILE For Input As #1  'test if file already exists
MsgBox "File Already Exists", vbExclamation  'no error means it exists
Close #1
Exit Sub
oops:
'Force a save of the existing file
frmMain.SaveToDsk

Open WKFILE For Output As #1   'create it
frmMain.txbSFile.Text = WKFILE
Write #1, "Fixed Angle Table"
For J = 0 To 29
FixedAng(J, 1) = 0
FixedAng(J, 2) = 0
FixedAng(J, 3) = 0
FixedAng(J, 4) = 0
FixedAng(J, 5) = 0
FixedAng(J, 6) = 0
FixedAng(J, 7) = 0
FixedAng(J, 8) = 0
FixedAng(J, 9) = 0
FixedAng(J, 10) = 0

FixedAng(2, 1) = -1
FixedAng(2, 2) = -1
FixedAng(2, 3) = -1
FixedAng(2, 4) = -1
FixedAng(2, 5) = -1
FixedAng(2, 6) = -1
FixedAng(2, 7) = -1
FixedAng(2, 8) = -1
FixedAng(2, 9) = -1
FixedAng(2, 10) = -1

Write #1, FixedAng(J, 1), FixedAng(J, 2), FixedAng(J, 3), FixedAng(J, 4), FixedAng(J, 5), FixedAng(J, 6), FixedAng(J, 7), FixedAng(J, 8), FixedAng(J, 9), FixedAng(J, 10)

Next J

Write #1, "Ppi Definition Table"
For J = 0 To 15
PpiDef(J, 1) = 0
PpiDef(J, 2) = 0
PpiDef(J, 3) = 0
PpiDef(J, 4) = 0

PpiDef(FAPOINT, 1) = 1
PpiDef(FAPOINT, 2) = 2
PpiDef(FAPOINT, 3) = 3
PpiDef(FAPOINT, 4) = 4

Write #1, PpiDef(J, 1), PpiDef(J, 2), PpiDef(J, 3), PpiDef(J, 4)
Next J

Write #1, "Rhi Definition Table"
For J = 0 To 15
RhiDef(J, 1) = 0
RhiDef(J, 2) = 0
RhiDef(J, 3) = 0

RhiDef(FAPOINT, 1) = 5
RhiDef(FAPOINT, 2) = 6
RhiDef(FAPOINT, 3) = 7

Write #1, RhiDef(J, 1), RhiDef(J, 2), RhiDef(J, 3), RhiDef(J, 4)
Next J
 
Write #1, "Survalence Definition Table"
For J = 0 To 15
SurDef(J, 1) = 0
SurDef(J, 2) = 0
SurDef(J, 3) = 0

SurDef(FAPOINT, 1) = 8
SurDef(FAPOINT, 2) = 9
SurDef(FAPOINT, 3) = 10

Write #1, SurDef(J, 1), SurDef(J, 2), SurDef(J, 3), SurDef(J, 4)
Next J
 
SeqDef(0, 1) = -2
SeqDef(0, 2) = -2
SeqDef(0, 3) = -2
SeqDef(0, 4) = -2
SeqDef(0, 5) = -2
SeqDef(0, 6) = -2
SeqDef(0, 7) = -2
SeqDef(0, 8) = -2

Write #1, "Sequence Definition Table"
For J = 0 To 9
Write #1, SeqDef(J, 1), SeqDef(J, 2), SeqDef(J, 3), SeqDef(J, 4), SeqDef(J, 5), SeqDef(J, 6), SeqDef(J, 7), SeqDef(J, 8)
Next J

Write #1, frmMain.optPpi1.Caption
Write #1, frmMain.optPpi2.Caption
Write #1, frmMain.optPpi3.Caption
Write #1, frmMain.optPpi4.Caption
Write #1, frmMain.optRhi1.Caption
Write #1, frmMain.optRhi2.Caption
Write #1, frmMain.optRhi3.Caption
Write #1, frmMain.optSur1.Caption
Write #1, frmMain.optSur2.Caption
Write #1, frmMain.optSur3.Caption

Write #1, frmMain.optSeq1.Caption
Write #1, frmMain.optseq2.Caption
Write #1, frmMain.optSeq3.Caption
Write #1, frmMain.optseq4.Caption
Write #1, frmMain.optseq5.Caption
Write #1, frmMain.optseq6.Caption
Write #1, frmMain.optseq7.Caption
'Write #1, frmMain.optseq8.Caption

frmMain.PpiFa1.Text = PpiDef(FAPOINT, 1)
frmMain.PpiFa2.Text = PpiDef(FAPOINT, 2)
frmMain.PpiFa3.Text = PpiDef(FAPOINT, 3)
frmMain.PpiFa4.Text = PpiDef(FAPOINT, 4)


frmMain.SurFa1.Text = SurDef(FAPOINT, 1)
frmMain.SurFa2.Text = SurDef(FAPOINT, 2)
frmMain.SurFa3.Text = SurDef(FAPOINT, 3)



frmMain.RhiFa1.Text = RhiDef(FAPOINT, 1)
frmMain.RhiFa2.Text = RhiDef(FAPOINT, 2)
frmMain.RhiFa3.Text = RhiDef(FAPOINT, 3)

Close #1


Open "ScanSource" For Output As #1
Write #1, WKFILE
Write #1, AzStow, ElStow
Write #1, Hobbs
Close #1

frmMain.txbSFile.Text = WKFILE
frmMain.LoadFrmDsk
If ISAPMAC Then
PMACPORT.Output = "disable plc10" + Chr(13)
PMACPORT.Output = "disable plc20" + Chr(13)
End If
Screen.MousePointer = vbHourglass
'frmMain.Enabled = False
frmPlWait.Show
frmPlWait.Refresh
frmMain.EncodePpi
frmMain.EncodeRhi
frmMain.EncodeSur
frmMain.EncodeFa
frmMain.EncodeSeq
frmPlWait.Hide
'frmMain.Enabled = True
Screen.MousePointer = vbDefault
If ISAPMAC Then
PMACPORT.Output = "enable plc10" + Chr(13)
PMACPORT.Output = "enable plc20" + Chr(13)
End If

MsgBox " Don't forget your file name", vbExclamation
'frmMain.Enabled = True
frmMain.Show


 Unload frmFiles


End Sub

Private Sub cmdOpnFile_Click()
Dim Name As String
Dim J As Single


Name = MsgBox("Loading a New File Will Corrupt Running Scans", vbOKCancel, "Be Careful!")
If Name = 2 Then
Exit Sub
End If
If (ISRUNNING) Then
Beep
MsgBox "Stop Antenna First", vbOKOnly
Exit Sub
End If
Name = InputBox("Enter Scan File Name")

If Name = "" Then
Exit Sub
End If

J = MsgBox("Save and Overwrite Currently Open File?", vbYesNo)
If J = vbYes Then
frmMain.SaveToDsk
End If

WKFILE = Name
frmMain.txbSFile.Text = WKFILE
Open "ScanSource" For Output As #1
Write #1, WKFILE
Write #1, AzStow, ElStow
Write #1, Hobbs
Close #1
frmMain.Hide
frmMain.LoadFrmDsk
If ISAPMAC Then
PMACPORT.Output = "disable plc10" + Chr(13)
PMACPORT.Output = "disable plc20" + Chr(13)
End If
Screen.MousePointer = vbHourglass
'frmMain.Enabled = False
frmPlWait.Show
frmPlWait.Refresh
frmMain.EncodePpi
frmMain.EncodeRhi
frmMain.EncodeSur
frmMain.EncodeFa
frmMain.EncodeSeq
frmPlWait.Hide
'frmMain.Enabled = True
Screen.MousePointer = vbDefault
If ISAPMAC Then
PMACPORT.Output = "enable plc10" + Chr(13)
PMACPORT.Output = "enable plc20" + Chr(13)
End If

frmMain.Show
'frmMain.Enabled = True
frmMain.Show
Unload frmFiles
 End Sub

Private Sub Form_Load()
Dim Name As String
'frmMain.Enabled = False
LstScanFile.Clear
frmMain.Hide
Name = Dir("*.scn")
LstScanFile.AddItem Name
While (Name <> "")
Name = Dir
If (Name <> "") Then LstScanFile.AddItem Name
Wend
End Sub

Private Sub LstScanFile_DblClick()
Dim Name As String
Dim M As Integer
Dim qqq As String

M = LstScanFile.ListIndex
If (M >= 0) Then
Name = (LstScanFile.List(M))
qqq = MsgBox("Loading a New File Will Corrupt Running Scans", vbOKCancel, "Be Careful!")
If qqq = 2 Then
Exit Sub
End If

If (ISRUNNING) Then
Beep
MsgBox "Stop Antenna First", vbOKOnly
Exit Sub
End If

If Name = "" Then
Exit Sub
End If
M = MsgBox("Save and Overwrite Currently Open File?", vbYesNo)
If M = vbYes Then
frmMain.SaveToDsk
End If

WKFILE = Name
frmMain.txbSFile.Text = WKFILE
Open "ScanSource" For Output As #1
Write #1, WKFILE
Write #1, AzStow, ElStow
Write #1, Hobbs
Close #1
frmMain.Hide
frmMain.LoadFrmDsk
If ISAPMAC Then
PMACPORT.Output = "disable plc10" + Chr(13)
PMACPORT.Output = "disable plc20" + Chr(13)
End If
Screen.MousePointer = vbHourglass
'frmMain.Enabled = False
frmPlWait.Show
frmPlWait.Refresh
frmMain.EncodePpi
frmMain.EncodeRhi
frmMain.EncodeSur
frmMain.EncodeFa
frmMain.EncodeSeq
frmPlWait.Hide
'frmMain.Enabled = True
Screen.MousePointer = vbDefault
If ISAPMAC Then
PMACPORT.Output = "enable plc10" + Chr(13)
PMACPORT.Output = "enable plc20" + Chr(13)
End If

frmMain.Show
'frmMain.Enabled = True
frmMain.Show
Unload frmFiles
End If

End Sub
