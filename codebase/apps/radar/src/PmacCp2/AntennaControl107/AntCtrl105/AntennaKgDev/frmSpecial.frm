VERSION 5.00
Begin VB.Form frmSpecial 
   BackColor       =   &H00FF00FF&
   Caption         =   "Special"
   ClientHeight    =   4935
   ClientLeft      =   11655
   ClientTop       =   7650
   ClientWidth     =   5910
   LinkTopic       =   "Form1"
   ScaleHeight     =   4935
   ScaleWidth      =   5910
   StartUpPosition =   2  'CenterScreen
   Begin VB.CommandButton Command1 
      BackColor       =   &H000000FF&
      Caption         =   "PMAC Commands"
      Height          =   1095
      Left            =   3000
      Style           =   1  'Graphical
      TabIndex        =   7
      Top             =   840
      Width           =   2175
   End
   Begin VB.CommandButton cmdDownload 
      Caption         =   "Download PMAC"
      Height          =   855
      Left            =   3960
      TabIndex        =   6
      Top             =   2040
      Width           =   1215
   End
   Begin VB.CommandButton cmdExpand 
      BackColor       =   &H00C0C000&
      Caption         =   "Expand Azimuth"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1095
      Left            =   120
      Style           =   1  'Graphical
      TabIndex        =   4
      Top             =   3600
      Width           =   1695
   End
   Begin VB.CommandButton Commcmdstowloc 
      BackColor       =   &H00FF0000&
      Caption         =   "Stow Locations"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   120
      MaskColor       =   &H00404040&
      Style           =   1  'Graphical
      TabIndex        =   3
      Top             =   1320
      Width           =   2535
   End
   Begin VB.CommandButton cmdSolar 
      BackColor       =   &H000000FF&
      Caption         =   "Solar"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   120
      Style           =   1  'Graphical
      TabIndex        =   2
      Top             =   2880
      Width           =   975
   End
   Begin VB.CommandButton cmdBye 
      Caption         =   "Bye"
      Height          =   252
      Left            =   4560
      TabIndex        =   1
      Top             =   4440
      Width           =   612
   End
   Begin VB.CommandButton cmdPoint 
      BackColor       =   &H0000FF00&
      Caption         =   "Point Antenna"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   492
      Left            =   120
      Style           =   1  'Graphical
      TabIndex        =   0
      Top             =   2160
      Width           =   2415
   End
   Begin VB.Label Label1 
      BackColor       =   &H00FF00FF&
      Caption         =   "Special Commands"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   18
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   960
      TabIndex        =   5
      Top             =   360
      Width           =   3495
   End
End
Attribute VB_Name = "frmSpecial"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Private Sub cmdBye_Click()
frmMain.Show
Unload frmSpecial
End Sub

Private Sub cmdDownload_Click()
frmDownload.Show

End Sub

Private Sub cmdExpand_Click()
If cmdExpand.Caption = "Expand Azimuth" Then
If (ISAPMAC) Then
PMACPORT.Output = "disable plc10" + Chr(13)
PMACPORT.Output = "p51=p107" + Chr(13)     'center of ppi scan

PMACPORT.Output = "p168=1" + Chr(13)
End If
cmdExpand.Caption = "Normal Azimuth"
Else
If (ISAPMAC) Then
PMACPORT.Output = "p168=0" + Chr(13)
PMACPORT.Output = "enable plc10" + Chr(13)
End If
cmdExpand.Caption = "Expand Azimuth"
End If
End Sub

Private Sub cmdPoint_Click()
frmPoint.Show
End Sub

Private Sub cmdSolar_Click()
frmSolar.Show

End Sub


Private Sub Command1_Click()
Dim Instring As String * 100
Dim Temp As String
Dim Inval
Dim Timestart As Single



Timestart = Timer
Do While PMACIoBusy > 0 'wait for busy to go down
If (Timer - Timestart) > 0.25 Then GoTo 50
zzz = PMACIoBusy
DoEvents
Call SDelay(0.1) 'let the system get in, This prevents a hang.
Loop


50 PMACIoBusy = 50 'lock out somebody else

While (PMACPORT.InBufferCount <> 0) 'Clear any pending inputs
DoEvents
Instring = PMACPORT.Input
Wend




Temp = InputBox("Enter PMAC Command")
If Temp = "" Then
PMACIoBusy = False
Exit Sub
End If
If (ISAPMAC) Then
 PMACPORT.Output = Temp + Chr(13)

Timestart = Timer
buffer$ = ""
Do
DoEvents
buffer$ = buffer$ & PMACPORT.Input
    If (Timer - Timestart) > 0.5 Then  'give it .5 sec
    PMACIoBusy = 0
    MsgBox "No response"
    GoTo 50
    
    End If

Loop Until InStr(buffer$, Chr(13))

PMACIoBusy = 0
MsgBox buffer$
End If
GoTo 50
End Sub

 

Private Sub Commcmdstowloc_Click()
Dim Temp
zop:
Temp = InputBox("Enter Azimuth Stow Position")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If
AzStow = Temp
Temp = InputBox("Enter Elevation Stow Position")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If
ElStow = Temp
Temp = "Az Stow= " + Format(AzStow) + " El Stow= " + Format(ElStow)
Temp = MsgBox(Temp, vbOK, "Stow Positions")
If (Temp = 2) Then
GoTo zop
Else
Open "scansource" For Output As #1

Write #1, WKFILE
Write #1, AzStow, ElStow
Write #1, Hobbs
Close #1
End If
End Sub

Private Sub Form_Load()
'frmMain.Hide
End Sub

