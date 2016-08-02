VERSION 5.00
Begin VB.Form frmFang 
   BackColor       =   &H0080FF80&
   Caption         =   "Fixed Angle Editor"
   ClientHeight    =   8220
   ClientLeft      =   1950
   ClientTop       =   2055
   ClientWidth     =   5250
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   13.5
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   8220
   ScaleWidth      =   5250
   StartUpPosition =   2  'CenterScreen
   Begin VB.CommandButton CmdEqHeight 
      BackColor       =   &H00FF00FF&
      Caption         =   "Clear and Fill with Equal Resolution Steps "
      Height          =   1215
      Left            =   2040
      Style           =   1  'Graphical
      TabIndex        =   14
      Top             =   4680
      Width           =   3015
   End
   Begin VB.CommandButton CmdChange 
      BackColor       =   &H000080FF&
      Caption         =   " Change Angle"
      Height          =   492
      Left            =   2040
      Style           =   1  'Graphical
      TabIndex        =   12
      Top             =   2280
      Width           =   2412
   End
   Begin VB.CommandButton cmdDelete 
      BackColor       =   &H0000C0C0&
      Caption         =   "Delete Angle"
      Height          =   492
      Left            =   2040
      Style           =   1  'Graphical
      TabIndex        =   11
      Top             =   1440
      Width           =   2532
   End
   Begin VB.CommandButton Command2 
      Caption         =   "?"
      Height          =   360
      Left            =   3600
      TabIndex        =   10
      Top             =   6360
      Width           =   375
   End
   Begin VB.CommandButton cmdRotate 
      BackColor       =   &H0000C0C0&
      Caption         =   "Rotate"
      Height          =   495
      Left            =   2040
      Style           =   1  'Graphical
      TabIndex        =   9
      Top             =   6240
      Width           =   1335
   End
   Begin VB.CommandButton cmdInsert 
      BackColor       =   &H00FF0000&
      Caption         =   "Insert  Angle"
      Height          =   375
      Left            =   2040
      MaskColor       =   &H000080FF&
      Style           =   1  'Graphical
      TabIndex        =   8
      Top             =   3120
      Width           =   2895
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H00C0FFFF&
      Caption         =   "?"
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
      Left            =   4560
      Style           =   1  'Graphical
      TabIndex        =   7
      Top             =   240
      Width           =   495
   End
   Begin VB.CommandButton cmdCancel 
      BackColor       =   &H000000FF&
      Caption         =   "Cancel"
      Height          =   495
      Left            =   3360
      Style           =   1  'Graphical
      TabIndex        =   6
      Top             =   7320
      Width           =   1215
   End
   Begin VB.CommandButton cmdClear 
      BackColor       =   &H00FF00FF&
      Caption         =   "Clear List"
      Height          =   495
      Left            =   2040
      Style           =   1  'Graphical
      TabIndex        =   5
      Top             =   600
      Width           =   1335
   End
   Begin VB.CommandButton cmdFill 
      BackColor       =   &H00C0C000&
      Caption         =   "Clear and Fill with Equal Steps"
      Height          =   735
      Left            =   2040
      Style           =   1  'Graphical
      TabIndex        =   4
      Top             =   3840
      Width           =   3015
   End
   Begin VB.TextBox txtFaid 
      Appearance      =   0  'Flat
      BackColor       =   &H0000FF00&
      BorderStyle     =   0  'None
      ForeColor       =   &H000000C0&
      Height          =   360
      Left            =   2280
      TabIndex        =   3
      Text            =   "Text1"
      Top             =   120
      Width           =   735
   End
   Begin VB.CommandButton frmExit 
      BackColor       =   &H0000C000&
      Caption         =   "Save Changes "
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   735
      Left            =   840
      Style           =   1  'Graphical
      TabIndex        =   2
      Top             =   7200
      Width           =   1455
   End
   Begin VB.ListBox lstAng 
      BeginProperty DataFormat 
         Type            =   1
         Format          =   "0.00E+00"
         HaveTrueFalseNull=   0
         FirstDayOfWeek  =   0
         FirstWeekOfYear =   0
         LCID            =   1033
         SubFormatType   =   6
      EndProperty
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FF0000&
      Height          =   6060
      ItemData        =   "Fixed_Angle.frx":0000
      Left            =   840
      List            =   "Fixed_Angle.frx":0002
      TabIndex        =   0
      Top             =   720
      Width           =   1095
   End
   Begin VB.Label Label1 
      Caption         =   "Click on Item to Select"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1092
      Left            =   120
      TabIndex        =   13
      Top             =   3000
      Width           =   612
   End
   Begin VB.Label lblfixanglist 
      AutoSize        =   -1  'True
      BackColor       =   &H0080FF80&
      Caption         =   "Fixed Angle List"
      ForeColor       =   &H000000C0&
      Height          =   360
      Left            =   120
      TabIndex        =   1
      Top             =   120
      Width           =   2025
   End
End
Attribute VB_Name = "frmFang"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim M, K As Integer
Dim Message, Title, Inval

Private Sub CmdChange_Click()
Dim J As Integer
Dim zzz As String

J = lstAng.ListIndex
If (J >= 0) Then
    If lstAng.List(J) <> "end" Then
        zzz = InputBox("Enter New Value For This Location", "Change")
        If (zzz = "") Then Exit Sub
            If IsNumeric(zzz) Then
                lstAng.List(J) = zzz
            Else
                MsgBox "Invalid entry", vbExclamation
            End If
     End If
Else
    MsgBox "Select Item First", vbExclamation
End If
End Sub

Private Sub cmdDelete_Click()
Dim J As Integer
J = lstAng.ListIndex
If (J >= 0) Then
    If lstAng.List(J) <> "end" Then
        lstAng.RemoveItem J
    End If
Else
    MsgBox "Select Item First", vbExclamation
End If
End Sub

Private Sub CmdEqHeight_Click()
lstAng.Clear
lstAng.AddItem "end"
frmEqHeight.Show

End Sub

Private Sub cmdInsert_Click()
Dim J As Integer
Dim zzz As String
J = lstAng.ListIndex
If (J >= 0) Then
    'If lstAng.List(J) <> "end" Then
    zzz = InputBox("Enter Fixed Angle ", "Insert")
    If (zzz = "") Then Exit Sub
    If (IsNumeric(zzz)) Then
        lstAng.AddItem zzz, J
    Else
        MsgBox "Invalid Entry", vbExclamation
    End If
    'Else

   'End If

Else
    MsgBox "Select Item First", vbExclamation
End If
End Sub

Private Sub cmdRotate_Click()
Dim M As Integer
Dim K As Integer
Dim zzz As String
Dim qqq As Single
Dim off As Single


M = lstAng.ListIndex
If (M >= 0) Then
    zzz = InputBox("Enter New Value For This Location", "Rotate")
If (zzz = "") Then Exit Sub
    If IsNumeric(zzz) Then
        qqq = Val(lstAng.List(M))
        off = Val(zzz) - qqq
        For K = 0 To 29
            If lstAng.List(K) <> "end" Then
                qqq = Val(lstAng.List(K)) + off
                If (qqq < 0) Then
                    qqq = 360 + qqq
                End If

                If (qqq >= 360) Then
                    qqq = qqq - 360
                End If

                lstAng.List(K) = Int(qqq * 1000) / 1000#

            Else
                GoTo out1
            End If
        Next K
    Else
        MsgBox "Invalid Entry", vbOK
    End If
out1:
Else
    MsgBox "Select Item First", vbExclamation
End If
End Sub

Private Sub Command1_Click()
MsgBox "To Change Value Double Click on it" + Chr(13) + Chr(12) + " To Delete Selected Entry; Enter a Blank" + Chr(13) + Chr(12) + " To Append an Angle; Double Click on End" + Chr(13) + Chr(12) + " To Insert;Double Click on Location First, Then click on Insert" + Chr(13) + Chr(12) + " Rotate adds a constant to the list, Select Location then Enter New Value" + Chr(13) + Chr(12)
     


End Sub


Private Sub cmdCancel_Click()
frmFang.Hide
'frmPPI.optFang1.BackColor = &HC0C0C0
'frmPPI.optFang2.BackColor = &HC0C0C0
'frmPPI.optFang3.BackColor = &HC0C0C0
'frmPPI.optFang4.BackColor = &HC0C0C0
'frmMain.Enabled = True
frmMain.Show
End Sub

Private Sub cmdClear_Click()
lstAng.Clear
lstAng.AddItem "end"
End Sub

Private Sub cmdFill_Click()
lstAng.Clear
lstAng.AddItem "end"
frmFiller.Show
End Sub

Private Sub cmsClear_Click()
lstAng.Clear
lstAng.AddItem "end"
End Sub

Private Sub Command2_Click()
MsgBox "Change any angle keeping all others relative", vbOKOnly
End Sub

Private Sub Command3_Click()

End Sub

Private Sub Form_Activate()
Dim I, J As Integer
frmViewPPI.Hide
frmViewRhi.Hide
frmViewSur.Hide
'frmMain.Enabled = False
frmMain.Hide
J = Val(txtFaid)
lstAng.Clear
For I = 0 To 29
    If FixedAng(I, J) > -998 Then
        lstAng.AddItem Str(FixedAng(I, J))
    Else
        lstAng.AddItem "end"
        GoTo Out
    End If
Next I
Out:

End Sub

Private Sub frmExit_Click()
Dim I, J As Integer
Dim zzz
zzz = MsgBox("This may change the Fixed Angles for other Scans", vbOKCancel)
If zzz = vbOK Then
    'copy the Fixed Angle List into the Array
    J = Val(txtFaid)
    For I = 0 To lstAng.ListCount - 2
        FixedAng(I, J) = Val(lstAng.List(I))
    Next I
    FixedAng(I, J) = -999#
End If
If ISAPMAC And Not SemTakePMAC(900) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "disable plc10" + Chr(13)
    PMACPORT.Output = "disable plc20" + Chr(13)
End If
frmFang.Hide
Screen.MousePointer = vbHourglass
'frmMain.Enabled = False
frmPlWait.Show
frmPlWait.Refresh
frmMain.EncodeFa
frmPlWait.Hide
'frmMain.Enabled = True
Screen.MousePointer = vbDefault
If ISAPMAC Then
    PMACPORT.Output = "enable plc10" + Chr(13)
    PMACPORT.Output = "enable plc20" + Chr(13)
    SemGivePMAC (900)
End If

frmPPI.optFang1.BackColor = &HC0C0C0
frmPPI.optFang2.BackColor = &HC0C0C0
frmPPI.optFang3.BackColor = &HC0C0C0
frmPPI.optFang4.BackColor = &HC0C0C0

'frmMain.Enabled = True
frmMain.Show
frmMain.Refresh
frmMain.CmdDump.Caption = "Save Scans?"
End Sub

Private Sub List1_Click()

M = lstAng.ListIndex

Message = "Enter a Fixed Angle "
Title = "SPOL"

' Display message, title, and default value.


'was the end value clicked?
'if so push the new value onto the bottom of the list

If M = lstAng.ListCount - 1 Then
    If lstAng.ListCount >= 30 Then
        MsgBox "Maximum List Length=30 angles", vbOKOnly + vbInformation
    Else
        Inval = InputBox(Message, Title)
        If IsNumeric(Inval) Then
            lstAng.RemoveItem M
            lstAng.AddItem Inval
            lstAng.AddItem "end"
        End If
    End If
    GoTo Out
End If
Inval = InputBox(Message, Title)
'blank entry into the list deletes the entry
If Inval = "" Then
    lstAng.RemoveItem M
End If


'good value into the list changes the value
'an end inserted erases the rest of the list
If IsNumeric(Inval) Then
    lstAng.RemoveItem M
    lstAng.AddItem Inval, M
Else
    If Inval = "end" Then
        lstAng.RemoveItem M
        lstAng.AddItem Inval, M
        For K = M To lstAng.ListCount - 2
            lstAng.RemoveItem (M + 1)
        Next K
    End If
End If











Out:
End Sub


Private Sub txbFAID_Change()

End Sub


Private Sub lstAng_DblClick()
M = lstAng.ListIndex

Message = "Enter a Fixed Angle "
Title = "CP2"

' Display message, title, and default value.


'was the end value clicked?
'if so push the new value onto the bottom of the list

If M = lstAng.ListCount - 1 Then
    If lstAng.ListCount >= 30 Then
        MsgBox "Maximum List Length=30 angles", vbOKOnly + vbInformation
    Else
        Inval = InputBox(Message, Title)
        If IsNumeric(Inval) Then
            lstAng.RemoveItem M
            lstAng.AddItem Inval
            lstAng.AddItem "end"
        End If
    End If
    GoTo Out
End If
Inval = InputBox(Message, Title)
'blank entry into the list deletes the entry
If Inval = "" Then
    lstAng.RemoveItem M
End If


'good value into the list changes the value
'an end inserted erases the rest of the list
If IsNumeric(Inval) Then
    lstAng.RemoveItem M
    lstAng.AddItem Inval, M
Else
    If Inval = "end" Then
        lstAng.RemoveItem M
        lstAng.AddItem Inval, M
        For K = M To lstAng.ListCount - 2
            lstAng.RemoveItem (M + 1)
        Next K
    End If
End If
Out:


End Sub
