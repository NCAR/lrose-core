VERSION 5.00
Begin VB.Form frmFiller 
   BackColor       =   &H000040C0&
   Caption         =   "Fixed Angle Filler"
   ClientHeight    =   3795
   ClientLeft      =   3255
   ClientTop       =   3030
   ClientWidth     =   5490
   LinkTopic       =   "Form1"
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3795
   ScaleWidth      =   5490
   Begin VB.TextBox txtWidth 
      Height          =   372
      Left            =   3840
      TabIndex        =   12
      Text            =   "***"
      Top             =   2040
      Width           =   972
   End
   Begin VB.TextBox txtCenter 
      Height          =   408
      Left            =   3840
      TabIndex        =   11
      Text            =   "***"
      Top             =   1320
      Width           =   972
   End
   Begin VB.TextBox txtStart 
      Height          =   375
      Left            =   1320
      Locked          =   -1  'True
      TabIndex        =   5
      Text            =   "***"
      Top             =   1080
      Width           =   800
   End
   Begin VB.TextBox txtEnd 
      Height          =   375
      Left            =   1320
      Locked          =   -1  'True
      TabIndex        =   4
      Text            =   "***"
      Top             =   2400
      Width           =   800
   End
   Begin VB.TextBox txtStep 
      Height          =   375
      Left            =   1320
      Locked          =   -1  'True
      TabIndex        =   3
      Text            =   "***"
      Top             =   1680
      Width           =   800
   End
   Begin VB.CommandButton cmdCancel 
      BackColor       =   &H000000FF&
      Caption         =   "Cancel"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   18
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   2640
      Style           =   1  'Graphical
      TabIndex        =   1
      Top             =   3000
      Width           =   1455
   End
   Begin VB.CommandButton Command1 
      BackColor       =   &H0000C000&
      Caption         =   "Fill "
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   18
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   360
      Style           =   1  'Graphical
      TabIndex        =   2
      Top             =   3000
      Width           =   1455
   End
   Begin VB.Label Label6 
      BackColor       =   &H00FF8080&
      Caption         =   "Width"
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
      Left            =   2640
      TabIndex        =   10
      Top             =   2040
      Width           =   732
   End
   Begin VB.Label Label5 
      BackColor       =   &H00C0C000&
      Caption         =   "Center"
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
      Left            =   2640
      TabIndex        =   9
      Top             =   1320
      Width           =   1092
   End
   Begin VB.Label Label2 
      BackColor       =   &H000000FF&
      Caption         =   "Start"
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
      Left            =   360
      TabIndex        =   8
      Top             =   1080
      Width           =   852
   End
   Begin VB.Label Label3 
      BackColor       =   &H0000FFFF&
      Caption         =   "End"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   372
      Left            =   360
      TabIndex        =   7
      Top             =   2400
      Width           =   852
   End
   Begin VB.Label Label4 
      BackColor       =   &H0000FF00&
      Caption         =   "Step"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   372
      Left            =   360
      TabIndex        =   6
      Top             =   1680
      Width           =   852
   End
   Begin VB.Label Label1 
      BackColor       =   &H0000FFFF&
      Caption         =   "Fixed Angle Constant Step Fill "
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   360
      TabIndex        =   0
      Top             =   360
      Width           =   3975
   End
End
Attribute VB_Name = "frmFiller"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim Start As Single, Step As Single, Fend As Single
Dim Center As Single, Fwidth As Single
Dim J As Integer

Private Sub cmdCancel_Click()
frmFiller.Hide
End Sub


Private Sub Command1_Click()
Dim Ang As Single
Dim I, J As Integer
Dim Prec As Single

If Step = 0 Then
MsgBox "Need a non zero step size"
Exit Sub
End If
If (Start > Fend) Then
MsgBox "End Must be Greater than Start", vbOKOnly
Exit Sub
End If
Prec = 100
If (Step < 0.1) Then
Prec = 1000
End If

I = 1 + Abs(Start - Fend) / Abs(Step)
If I <= 30 Then

I = MsgBox("Erase Existing Fixed Angle List?", vbOKCancel)

'Fill the Fixed Angle List with Angles
If I = 1 Then

J = Val(frmFang.txtFaid)
I = 0
Ang = Start
While Ang <= Fend
FixedAng(I, J) = Int(Ang * Prec + 0.5) / Prec

If (FixedAng(I, J) > 360) Then
FixedAng(I, J) = FixedAng(I, J) - 360
End If

If (FixedAng(I, J) < 0) Then
FixedAng(I, J) = FixedAng(I, J) + 360
End If

Ang = Ang + Step
I = I + 1
Wend
FixedAng(I, J) = -999
End If

frmFiller.Hide



Else
MsgBox "Maximum of 30 Steps", vbOKOnly + vbExclamation
End If
End Sub

Private Sub Command2_Click()

End Sub

Private Sub Form_Load()
J = Val(frmFang.txtFaid)
End Sub

Private Sub txtCenter_Click()
Dim I As String
I = InputBox("Enter Center")
If IsNumeric(I) Then
Center = I
Start = Center - Fwidth / 2
Fend = Center + Fwidth / 2
txtCenter.Text = Center
txtStart.Text = Start
txtEnd.Text = Fend
Else
MsgBox "Invalid Value", vbExclamation

End If
End Sub

Private Sub txtEnd_Click()
Dim I
I = InputBox("Enter Final Fixed Angle")
If IsNumeric(I) Then
If (I > Start) Then
Fend = I
Else
Fend = I
'MsgBox "End must be > Start", vbExclamation
End If
txtEnd.Text = Str(Fend)
Fwidth = Fend - Start
txtWidth.Text = Fwidth
Center = (Start + Fend) / 2
txtCenter.Text = Center
End If

End Sub



Private Sub txtStart_Click()
Dim I
I = InputBox("Enter Initial Fixed Angle")
If IsNumeric(I) Then
If (I < Fend) Then
Start = I
Else
Start = I
'MsgBox "Start must be < End", vbExclamation
End If
txtStart.Text = Str(Start)
Fwidth = Fend - Start
txtWidth.Text = Fwidth
Center = (Start + Fend) / 2
txtCenter.Text = Center
End If
End Sub


Private Sub TxtStep_Click()
Dim I
I = InputBox("Enter Step Size")
If IsNumeric(I) Then
Step = I
TxtStep.Text = Str(Step)
End If
End Sub


Private Sub txtWidth_Click()
Dim I As String
I = InputBox("Enter Width")
If IsNumeric(I) Then
Fwidth = I
Start = Center - Fwidth / 2
Fend = Center + Fwidth / 2
txtWidth.Text = Fwidth
txtStart.Text = Start
txtEnd.Text = Fend
Else
MsgBox "Invalid Value", vbExclamation

End If
End Sub
