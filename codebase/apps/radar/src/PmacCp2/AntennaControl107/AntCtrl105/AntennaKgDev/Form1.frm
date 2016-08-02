VERSION 4.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   5940
   ClientLeft      =   1140
   ClientTop       =   1515
   ClientWidth     =   6690
   Height          =   6345
   Left            =   1080
   LinkTopic       =   "Form1"
   ScaleHeight     =   5940
   ScaleWidth      =   6690
   Top             =   1170
   Width           =   6810
   Begin VB.CommandButton Command1 
      Caption         =   "Ugg"
      Height          =   495
      Left            =   2760
      TabIndex        =   0
      Top             =   2760
      Width           =   1215
   End
End
Attribute VB_Name = "Form1"
Attribute VB_Creatable = False
Attribute VB_Exposed = False
Option Explicit

 
 
  
 




Private Sub Command1_Click()
frmPPI.Show
End Sub


Private Sub Form_Load()
FixedAng(0, 1) = -999
FixedAng(0, 2) = -999
FixedAng(0, 3) = -999
FixedAng(0, 4) = -999
End Sub


