VERSION 5.00
Begin VB.Form frmEqHeight 
   BackColor       =   &H00008000&
   Caption         =   "Form1"
   ClientHeight    =   6780
   ClientLeft      =   60
   ClientTop       =   405
   ClientWidth     =   7470
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   18
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   LinkTopic       =   "Form1"
   ScaleHeight     =   6780
   ScaleWidth      =   7470
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox TxtRangeMin 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   3480
      TabIndex        =   12
      Text            =   "Text1"
      Top             =   1920
      Width           =   2055
   End
   Begin VB.TextBox txtBot 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   3480
      TabIndex        =   10
      Text            =   "Text1"
      Top             =   3600
      Width           =   2055
   End
   Begin VB.CommandButton Command2 
      BackColor       =   &H000000FF&
      Caption         =   "Cancel"
      Height          =   615
      Left            =   4320
      Style           =   1  'Graphical
      TabIndex        =   4
      Top             =   5400
      Width           =   1455
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H0000FF00&
      Caption         =   "Fill"
      Height          =   615
      Left            =   960
      Style           =   1  'Graphical
      TabIndex        =   3
      Top             =   5760
      Width           =   1695
   End
   Begin VB.TextBox TxtStep 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   3480
      TabIndex        =   2
      Text            =   "Text3"
      Top             =   4440
      Width           =   2055
   End
   Begin VB.TextBox TxtTop 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   3480
      TabIndex        =   1
      Text            =   "Text2"
      Top             =   2760
      Width           =   2055
   End
   Begin VB.TextBox TxtRange 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   24
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   3480
      TabIndex        =   0
      Text            =   "Text1"
      Top             =   1080
      Width           =   2055
   End
   Begin VB.Label Label6 
      BackColor       =   &H00C0FFC0&
      Caption         =   "Range Min (km)"
      Height          =   615
      Left            =   360
      TabIndex        =   11
      Top             =   1920
      Width           =   2655
   End
   Begin VB.Label Label5 
      BackColor       =   &H00C0FFC0&
      Caption         =   "Bottom Angle"
      Height          =   615
      Left            =   360
      TabIndex        =   9
      Top             =   3600
      Width           =   2655
   End
   Begin VB.Label Label4 
      BackColor       =   &H00E0E0E0&
      Caption         =   "Fixed Angle Equal Resolution Step Fill"
      Enabled         =   0   'False
      Height          =   375
      Left            =   120
      TabIndex        =   8
      Top             =   360
      Width           =   6615
   End
   Begin VB.Label Label3 
      BackColor       =   &H00C0FFC0&
      Caption         =   "Resolution(m)"
      Height          =   615
      Left            =   360
      TabIndex        =   7
      Top             =   4440
      Width           =   2655
   End
   Begin VB.Label Label2 
      BackColor       =   &H00C0FFC0&
      Caption         =   "Top (km) agl"
      Height          =   615
      Left            =   360
      TabIndex        =   6
      Top             =   2760
      Width           =   2655
   End
   Begin VB.Label Label1 
      BackColor       =   &H00C0FFC0&
      Caption         =   "Range Max (km)"
      Height          =   615
      Left            =   360
      TabIndex        =   5
      Top             =   1080
      Width           =   2655
   End
End
Attribute VB_Name = "frmEqHeight"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim Range As Single
Dim RangeM As Single
Dim TopE As Single
Dim Bot As Single
Dim Step As Single
Dim Temp As String

Private Sub Command1_Click()
Dim I As Integer
Dim J As Integer
Dim R As Single
Dim Angle As Single
Dim Switch As Single
Dim Height As Single
Dim Rad As Single
Dim AStep As Single
Dim EndA As Single
I = MsgBox("Erase Existing Fixed Angle List?", vbOKCancel)
TopE = TopE - 0.0000058845 * Range * Range
Switch = Atn(TopE / Range)            'angle to far corner
EndA = Atn(TopE / RangeM)             'angle to near corner
Angle = Bot * (2 * 3.14159 / 360#)
If I = 1 Then
J = Val(frmFang.txtFaid)
I = 0
FixedAng(I, J) = Int((Angle * 360 / (2 * 3.14159)) * 100) / 100# 'bottom angle
I = 1
While Angle < EndA

If (Angle < Switch) Then
    Rad = Range / Cos(Angle) 'range to far edge of box
    AStep = Atn(Step / Rad)
    Angle = Angle + AStep
    FixedAng(I, J) = Int((Angle * 360 / (2 * 3.14159)) * 100) / 100#
    I = I + 1
        If (I > 30) Then
        MsgBox "Maximum of 30 Steps", vbOKOnly + vbExclamation
        FixedAng(I, J) = -999
        Exit Sub
        End If
Else
Rad = TopE / Sin(Angle) 'range to top of box
AStep = Atn(Step / Rad)
Angle = Angle + AStep
FixedAng(I, J) = Int((Angle * 360 / (2 * 3.14159)) * 100) / 100#
I = I + 1
        If (I > 30) Then
        MsgBox "Maximum of 30 Steps", vbOKOnly + vbExclamation
        FixedAng(I, J) = -999
         Exit Sub
        End If
       
End If

Wend






'While Height < TopE
'Height = Height + Step
'R = Height / Range
'Angle = (Atn(R)) * 360 / (2 * 3.14159)
'If (Angle < 0) Then Angle = 360# + Angle
'FixedAng(I, J) = Int(Angle * 100) / 100#

''I = I + 1
'If (I > 30) Then
'MsgBox "Maximum of 30 Steps", vbOKOnly + vbExclamation
'FixedAng(I, J) = -999
'Exit Sub
''End If
'Wend
FixedAng(I, J) = -999
End If
frmEqHeight.Hide
End Sub

Private Sub Command2_Click()
frmEqHeight.Hide
End Sub

Private Sub Label1_Click()
Temp = InputBox("Enter Range Max in km")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Be Positive"
Exit Sub

End If
TxtRange.Text = Str(Temp)
Range = Val(Temp)
End Sub

Private Sub Label2_Click()
Temp = InputBox("Enter Top km")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Be Positive"
Exit Sub

End If
TxtTop.Text = Str(Temp)
TopE = Val(Temp)

End Sub

Private Sub Label3_Click()
Temp = InputBox("Enter Resolution (m)")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Be Positive"
Exit Sub

End If
TxtStep.Text = Str(Temp)
Step = Val(Temp) / 1000
End Sub




Private Sub Label5_Click()
Temp = InputBox("Enter Bottom  Angle  (deg)")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Be Positive"
Exit Sub

End If
txtBot.Text = Str(Temp)
Bot = Val(Temp)

End Sub

Private Sub Label6_Click()
Temp = InputBox("Enter Range Min in km")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Be Positive"
Exit Sub

End If
TxtRangeMin.Text = Str(Temp)
RangeM = Val(Temp)
End Sub

Private Sub txtBot_Click()
Temp = InputBox("Enter Bottom Angle (deg)")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Be Positive"
Exit Sub

End If
txtBot.Text = Str(Temp)
Bot = Val(Temp)

End Sub



Private Sub TxtRange_Click()
Temp = InputBox("Enter Range Max in km")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Be Positive"
Exit Sub

End If
TxtRange.Text = Str(Temp)
Range = Val(Temp)
End Sub



Private Sub TxtRangeMin_Click()
Temp = InputBox("Enter Range Min in km")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Be Positive"
Exit Sub

End If
TxtRangeMin.Text = Str(Temp)
RangeM = Val(Temp)
End Sub


Private Sub TxtStep_Click()
Temp = InputBox("Enter Resolution (m)")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Be Positive"
Exit Sub

End If
TxtStep.Text = Str(Temp)
Step = Val(Temp) / 1000
End Sub



Private Sub TxtTop_Click()
Temp = InputBox("Enter Top km")
If Temp = "" Then
Exit Sub
End If

If Not IsNumeric(Temp) Then
MsgBox "Invalid Number"
Exit Sub
End If

If Temp < 0 Then
MsgBox "Must Be Positive"
Exit Sub

End If
TxtTop.Text = Str(Temp)
TopE = Val(Temp)

End Sub

