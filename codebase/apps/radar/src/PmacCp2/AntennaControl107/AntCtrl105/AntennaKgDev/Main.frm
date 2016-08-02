VERSION 5.00
Object = "{648A5603-2C6E-101B-82B6-000000000014}#1.1#0"; "MSCOMM32.OCX"
Begin VB.Form frmMain 
   BackColor       =   &H00C0C0C0&
   Caption         =   "Main."
   ClientHeight    =   8790
   ClientLeft      =   165
   ClientTop       =   450
   ClientWidth     =   13365
   FillStyle       =   0  'Solid
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   18
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   FontTransparent =   0   'False
   ForeColor       =   &H000000FF&
   LinkTopic       =   "Form1"
   MousePointer    =   1  'Arrow
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   8790
   ScaleWidth      =   13365
   StartUpPosition =   2  'CenterScreen
   Begin VB.TextBox TxbHobbs 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   360
      Left            =   240
      TabIndex        =   81
      Text            =   "Text1"
      Top             =   1800
      Width           =   735
   End
   Begin VB.TextBox txtScanStat 
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   360
      Left            =   5520
      TabIndex        =   80
      Text            =   "Text1"
      Top             =   960
      Width           =   1935
   End
   Begin VB.TextBox TxtFake 
      Alignment       =   2  'Center
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   8400
      TabIndex        =   79
      Text            =   "Text1"
      Top             =   120
      Width           =   2055
   End
   Begin VB.Frame Frame6 
      BackColor       =   &H00008000&
      Caption         =   "Data Acquisition State Editor"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1212
      Left            =   5880
      TabIndex        =   74
      Top             =   7080
      Visible         =   0   'False
      Width           =   3252
      Begin VB.CommandButton CmdDaqD 
         Caption         =   "D"
         Height          =   432
         Left            =   2400
         TabIndex        =   78
         Top             =   480
         Width           =   372
      End
      Begin VB.CommandButton CmdDaqC 
         Caption         =   "C"
         Height          =   432
         Left            =   1680
         TabIndex        =   77
         Top             =   480
         Width           =   372
      End
      Begin VB.CommandButton CmdDaqB 
         Caption         =   "B"
         Height          =   432
         Left            =   960
         TabIndex        =   76
         Top             =   480
         Width           =   372
      End
      Begin VB.CommandButton CmdDaqA 
         Caption         =   "A"
         Height          =   432
         Left            =   360
         TabIndex        =   75
         Top             =   480
         Width           =   372
      End
   End
   Begin VB.CommandButton cmdBump 
      Appearance      =   0  'Flat
      Caption         =   "Bump to next Scan"
      BeginProperty Font 
         Name            =   "Arial"
         Size            =   11.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   11040
      Style           =   1  'Graphical
      TabIndex        =   73
      Top             =   1920
      Width           =   1455
   End
   Begin VB.TextBox txtEt 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00FFFF00&
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   345
      Left            =   8520
      TabIndex        =   72
      Text            =   "Text1"
      Top             =   1920
      Width           =   1335
   End
   Begin VB.TextBox txtElNow 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00C0FFFF&
      BeginProperty DataFormat 
         Type            =   0
         Format          =   "0.00"
         HaveTrueFalseNull=   0
         FirstDayOfWeek  =   0
         FirstWeekOfYear =   0
         LCID            =   1033
         SubFormatType   =   0
      EndProperty
      Enabled         =   0   'False
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000FFFF&
      Height          =   420
      Left            =   8880
      TabIndex        =   68
      Text            =   "Text1"
      Top             =   1080
      Width           =   975
   End
   Begin VB.TextBox txtAzNow 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00C0FFFF&
      BeginProperty DataFormat 
         Type            =   1
         Format          =   "0.00"
         HaveTrueFalseNull=   0
         FirstDayOfWeek  =   0
         FirstWeekOfYear =   0
         LCID            =   1033
         SubFormatType   =   1
      EndProperty
      Enabled         =   0   'False
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   420
      Left            =   8880
      TabIndex        =   67
      Text            =   "Text1"
      Top             =   480
      Width           =   975
   End
   Begin VB.TextBox txtST 
      Alignment       =   1  'Right Justify
      BackColor       =   &H00FFFF00&
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   396
      Left            =   5880
      TabIndex        =   56
      Top             =   1920
      Width           =   2535
   End
   Begin VB.Frame Frame5 
      BackColor       =   &H00808000&
      Caption         =   "Fixed Angle List  Editor"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   12
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1455
      Left            =   2760
      MousePointer    =   1  'Arrow
      TabIndex        =   54
      Top             =   6960
      Width           =   2772
      Begin VB.CommandButton Efa4 
         BackColor       =   &H0000C0C0&
         Caption         =   "4"
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   1680
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   66
         Top             =   360
         Width           =   375
      End
      Begin VB.CommandButton Efa2 
         BackColor       =   &H0000C0C0&
         Caption         =   "2"
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   720
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   65
         Top             =   360
         Width           =   375
      End
      Begin VB.CommandButton Efa3 
         BackColor       =   &H0000C0C0&
         Caption         =   "3"
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   1200
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   64
         Top             =   360
         Width           =   375
      End
      Begin VB.CommandButton Efa1 
         BackColor       =   &H0000C0C0&
         Caption         =   "1"
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   240
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   63
         Top             =   360
         Width           =   375
      End
      Begin VB.CommandButton Efa5 
         BackColor       =   &H0000C0C0&
         Caption         =   "5"
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   2160
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   62
         Top             =   360
         Width           =   375
      End
      Begin VB.CommandButton Efa6 
         BackColor       =   &H0000C0C0&
         Caption         =   "6"
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   240
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   61
         Top             =   840
         Width           =   375
      End
      Begin VB.CommandButton Efa7 
         BackColor       =   &H0000C0C0&
         Caption         =   "7"
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   720
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   60
         Top             =   840
         Width           =   375
      End
      Begin VB.CommandButton Efa8 
         BackColor       =   &H0000C0C0&
         Caption         =   "8"
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   1200
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   59
         Top             =   840
         Width           =   375
      End
      Begin VB.CommandButton Efa9 
         BackColor       =   &H0000C0C0&
         Caption         =   "9"
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   1680
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   58
         Top             =   840
         Width           =   375
      End
      Begin VB.CommandButton Efa10 
         BackColor       =   &H0000C0C0&
         Caption         =   "10"
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   375
         Left            =   2160
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   57
         Top             =   840
         Width           =   375
      End
   End
   Begin VB.TextBox RhiFa2 
      BeginProperty Font 
         Name            =   "Symbol"
         Size            =   12
         Charset         =   2
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   5640
      Locked          =   -1  'True
      MousePointer    =   2  'Cross
      TabIndex        =   50
      Text            =   "Text11"
      Top             =   3600
      Width           =   375
   End
   Begin VB.TextBox RhiFa1 
      BeginProperty Font 
         Name            =   "Symbol"
         Size            =   12
         Charset         =   2
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   315
      Left            =   5640
      Locked          =   -1  'True
      MousePointer    =   2  'Cross
      TabIndex        =   47
      Text            =   "Text5"
      Top             =   3000
      Width           =   375
   End
   Begin VB.CommandButton cmdrun 
      BackColor       =   &H0080FF80&
      Caption         =   " Run Scan"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   24
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1700
      Left            =   1680
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   42
      Top             =   720
      Width           =   3735
   End
   Begin VB.CommandButton cmdStop 
      Appearance      =   0  'Flat
      BackColor       =   &H000080FF&
      Caption         =   " RESET  PMAC"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1335
      Left            =   120
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   41
      Top             =   6960
      Width           =   2292
   End
   Begin VB.CommandButton cmdRunSeq 
      BackColor       =   &H00008080&
      Caption         =   "     Run       Sequence"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1700
      Left            =   10080
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   40
      Top             =   720
      Width           =   3135
   End
   Begin MSCommLib.MSComm MSComm1 
      Left            =   7200
      Top             =   360
      _ExtentX        =   1005
      _ExtentY        =   1005
      _Version        =   393216
      CommPort        =   2
      DTREnable       =   -1  'True
      RTSEnable       =   -1  'True
   End
   Begin VB.CommandButton cndSpecial 
      BackColor       =   &H00C000C0&
      Caption         =   "Special"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   510
      Left            =   9840
      MousePointer    =   10  'Up Arrow
      Style           =   1  'Graphical
      TabIndex        =   39
      Top             =   7800
      Width           =   1212
   End
   Begin VB.CommandButton cmdhlpseq 
      Caption         =   "?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   10200
      TabIndex        =   38
      Top             =   1560
      Width           =   200
   End
   Begin VB.CommandButton cmdhlprun 
      Caption         =   "?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   240
      Left            =   6000
      TabIndex        =   37
      Top             =   1440
      Width           =   200
   End
   Begin VB.CommandButton cmdSeqEdit 
      BackColor       =   &H00FFFF00&
      Caption         =   "Edit"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   12
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   500
      Left            =   10560
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   23
      Top             =   5520
      Width           =   800
   End
   Begin VB.Frame Frame4 
      BackColor       =   &H00C00000&
      Caption         =   "SCAN  SEQUENCES"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   3495
      Left            =   10440
      MousePointer    =   1  'Arrow
      TabIndex        =   20
      Top             =   2640
      Width           =   2715
      Begin VB.OptionButton optSeq3 
         BackColor       =   &H00FF0000&
         Caption         =   "Sequence 3"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   276
         Left            =   120
         MousePointer    =   2  'Cross
         TabIndex        =   28
         Top             =   1005
         Width           =   2000
      End
      Begin VB.OptionButton optseq6 
         BackColor       =   &H00FF0000&
         Caption         =   "Sequence 6"
         BeginProperty Font 
            Name            =   "System"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   840
         MousePointer    =   2  'Cross
         TabIndex        =   25
         Top             =   2130
         Width           =   2000
      End
      Begin VB.OptionButton optseq4 
         BackColor       =   &H00FF0000&
         Caption         =   "Sequence 4"
         BeginProperty Font 
            Name            =   "System"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   840
         MousePointer    =   2  'Cross
         TabIndex        =   27
         Top             =   1380
         Width           =   2000
      End
      Begin VB.OptionButton optseq2 
         BackColor       =   &H00FF0000&
         Caption         =   "Sequence 2"
         BeginProperty Font 
            Name            =   "System"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   840
         MousePointer    =   2  'Cross
         TabIndex        =   26
         Top             =   615
         Width           =   2000
      End
      Begin VB.OptionButton optSeq1 
         BackColor       =   &H00FF0000&
         Caption         =   "Sequence 1"
         BeginProperty Font 
            Name            =   "System"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   120
         MousePointer    =   2  'Cross
         TabIndex        =   24
         Top             =   240
         Width           =   2000
      End
      Begin VB.OptionButton optseq7 
         BackColor       =   &H00FF0000&
         Caption         =   "Sequence 7"
         BeginProperty Font 
            Name            =   "System"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   120
         MousePointer    =   2  'Cross
         TabIndex        =   22
         Top             =   2520
         Width           =   2000
      End
      Begin VB.OptionButton optseq5 
         BackColor       =   &H00FF0000&
         Caption         =   "Sequence 5"
         BeginProperty Font 
            Name            =   "System"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   285
         Left            =   120
         MousePointer    =   2  'Cross
         TabIndex        =   21
         Top             =   1755
         Width           =   2000
      End
   End
   Begin VB.Timer TmrClk 
      Interval        =   300
      Left            =   9600
      Top             =   3120
   End
   Begin VB.TextBox txtTime 
      Appearance      =   0  'Flat
      BackColor       =   &H00404080&
      BorderStyle     =   0  'None
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   12
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   285
      Left            =   6240
      Locked          =   -1  'True
      TabIndex        =   19
      Text            =   "Text4"
      Top             =   120
      Width           =   2000
   End
   Begin VB.TextBox txbSFile 
      BackColor       =   &H00C0C0C0&
      Enabled         =   0   'False
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000000FF&
      Height          =   396
      Left            =   6360
      MousePointer    =   1  'Arrow
      TabIndex        =   17
      Text            =   "AFILE"
      Top             =   6360
      Width           =   2175
   End
   Begin VB.CommandButton cmdFile 
      BackColor       =   &H0080C0FF&
      Caption         =   "Scan File Access"
      BeginProperty Font 
         Name            =   "System"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   240
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   16
      Top             =   6240
      Width           =   2895
   End
   Begin VB.CommandButton cmdStow 
      BackColor       =   &H00C000C0&
      Caption         =   "Stow"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   510
      Left            =   11880
      MousePointer    =   10  'Up Arrow
      Style           =   1  'Graphical
      TabIndex        =   14
      Top             =   6480
      Width           =   1212
   End
   Begin VB.CommandButton Command13 
      Caption         =   "Load"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   855
      Left            =   120
      TabIndex        =   12
      Top             =   720
      Visible         =   0   'False
      Width           =   1215
   End
   Begin VB.CommandButton CmdDump 
      BackColor       =   &H0080C0FF&
      Caption         =   "Save These Scans?"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1095
      Left            =   9480
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   11
      Top             =   6480
      Width           =   1935
   End
   Begin VB.CommandButton cmdquit 
      BackColor       =   &H00C000C0&
      Caption         =   "Quit"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   13.5
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      Height          =   510
      Left            =   11880
      MousePointer    =   2  'Cross
      Style           =   1  'Graphical
      TabIndex        =   10
      Top             =   7800
      Width           =   1212
   End
   Begin VB.Frame Frame3 
      BackColor       =   &H00C000C0&
      Caption         =   "SURVEILLANCE  SCANS"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   3255
      Left            =   6600
      MousePointer    =   1  'Arrow
      TabIndex        =   7
      Top             =   2640
      Width           =   2835
      Begin VB.TextBox SurFa3 
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   2040
         Locked          =   -1  'True
         MousePointer    =   2  'Cross
         TabIndex        =   52
         Text            =   "Text13"
         Top             =   1560
         Width           =   375
      End
      Begin VB.TextBox SurFa2 
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   2040
         Locked          =   -1  'True
         MousePointer    =   2  'Cross
         TabIndex        =   51
         Text            =   "Text12"
         Top             =   960
         Width           =   375
      End
      Begin VB.TextBox SurFa1 
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   2040
         Locked          =   -1  'True
         MousePointer    =   2  'Cross
         TabIndex        =   49
         Text            =   "Text10"
         Top             =   360
         Width           =   375
      End
      Begin VB.OptionButton optSur1 
         Caption         =   "Sur 1"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   240
         MousePointer    =   2  'Cross
         TabIndex        =   35
         Top             =   360
         Width           =   1600
      End
      Begin VB.OptionButton optSur3 
         BackColor       =   &H00C0C0C0&
         Caption         =   "Sur 3"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   240
         MousePointer    =   2  'Cross
         TabIndex        =   34
         Top             =   1560
         Width           =   1600
      End
      Begin VB.OptionButton optSur2 
         Caption         =   "Sur 2"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   240
         MousePointer    =   2  'Cross
         TabIndex        =   33
         Top             =   960
         Width           =   1600
      End
      Begin VB.CommandButton cmdSurView 
         BackColor       =   &H00808080&
         Caption         =   "View"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   500
         Left            =   240
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   9
         Top             =   2400
         Width           =   800
      End
      Begin VB.CommandButton cmdSurEdit 
         BackColor       =   &H00FFFF00&
         Caption         =   "Edit"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   500
         Left            =   1680
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   8
         Top             =   2400
         Width           =   800
      End
   End
   Begin VB.Frame Frame2 
      Appearance      =   0  'Flat
      BackColor       =   &H00008000&
      Caption         =   "RHI  SCANS"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H80000008&
      Height          =   3255
      Left            =   3600
      MousePointer    =   1  'Arrow
      TabIndex        =   3
      Top             =   2640
      Width           =   2715
      Begin VB.TextBox RhiFa3 
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   2040
         Locked          =   -1  'True
         MousePointer    =   2  'Cross
         TabIndex        =   48
         Text            =   "Text9"
         Top             =   1560
         Width           =   375
      End
      Begin VB.CommandButton cmdRhiEdit 
         BackColor       =   &H00FFFF00&
         Caption         =   "Edit"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   500
         Left            =   1560
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   32
         Top             =   2400
         Width           =   800
      End
      Begin VB.CommandButton cmdRhiView 
         BackColor       =   &H00808080&
         Caption         =   "View"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   500
         Left            =   240
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   31
         Top             =   2400
         Width           =   800
      End
      Begin VB.OptionButton optRhi3 
         Caption         =   "RHI 3"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   240
         MousePointer    =   2  'Cross
         TabIndex        =   6
         Top             =   1560
         Width           =   1600
      End
      Begin VB.OptionButton optRhi2 
         Caption         =   "RHI 2"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   240
         MousePointer    =   2  'Cross
         TabIndex        =   5
         Top             =   960
         Width           =   1600
      End
      Begin VB.OptionButton optRhi1 
         Caption         =   "RHI 1"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   300
         Left            =   240
         MousePointer    =   2  'Cross
         TabIndex        =   4
         Top             =   360
         Width           =   1600
      End
   End
   Begin VB.Frame Frame1 
      BackColor       =   &H00004080&
      Caption         =   "PPI  SCANS"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H00FFFFFF&
      Height          =   3255
      Left            =   720
      MousePointer    =   1  'Arrow
      TabIndex        =   0
      Top             =   2640
      Width           =   2715
      Begin VB.TextBox PpiFa1 
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   2040
         Locked          =   -1  'True
         MousePointer    =   2  'Cross
         TabIndex        =   53
         Text            =   "Text1"
         Top             =   360
         Width           =   375
      End
      Begin VB.TextBox PpiFa4 
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   2040
         Locked          =   -1  'True
         MousePointer    =   2  'Cross
         TabIndex        =   46
         Text            =   "Text1"
         Top             =   1800
         Width           =   375
      End
      Begin VB.TextBox PpiFa3 
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   2040
         Locked          =   -1  'True
         MousePointer    =   2  'Cross
         TabIndex        =   45
         Text            =   "Text1"
         Top             =   1320
         Width           =   375
      End
      Begin VB.TextBox PpiFa2 
         BeginProperty Font 
            Name            =   "Symbol"
            Size            =   12
            Charset         =   2
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   315
         Left            =   2040
         Locked          =   -1  'True
         MousePointer    =   2  'Cross
         TabIndex        =   44
         Text            =   "Text1"
         Top             =   840
         Width           =   375
      End
      Begin VB.OptionButton optPpi1 
         BackColor       =   &H00808080&
         Caption         =   "PPI 1"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   300
         Left            =   240
         MousePointer    =   2  'Cross
         TabIndex        =   43
         Top             =   360
         Width           =   1600
      End
      Begin VB.CommandButton cmdPPiEdit 
         BackColor       =   &H00C0C000&
         Caption         =   "Edit"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   500
         Left            =   1680
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   30
         Top             =   2400
         Width           =   800
      End
      Begin VB.CommandButton cmdPpiView 
         BackColor       =   &H00808080&
         Caption         =   "View"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   500
         Left            =   240
         MousePointer    =   2  'Cross
         Style           =   1  'Graphical
         TabIndex        =   29
         Top             =   2400
         Width           =   800
      End
      Begin VB.OptionButton optPpi4 
         BackColor       =   &H00808080&
         Caption         =   "PPI 4"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   300
         Left            =   240
         MousePointer    =   2  'Cross
         TabIndex        =   13
         Top             =   1800
         Width           =   1600
      End
      Begin VB.OptionButton optPpi3 
         BackColor       =   &H00808080&
         Caption         =   "PPI 3"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   300
         Left            =   240
         MousePointer    =   2  'Cross
         TabIndex        =   2
         Top             =   1320
         Width           =   1600
      End
      Begin VB.OptionButton optPpi2 
         BackColor       =   &H00808080&
         Caption         =   "PPI 2"
         BeginProperty Font 
            Name            =   "MS Sans Serif"
            Size            =   12
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         ForeColor       =   &H00FFFFFF&
         Height          =   300
         Left            =   240
         MousePointer    =   2  'Cross
         TabIndex        =   1
         Top             =   840
         Width           =   1600
      End
   End
   Begin VB.Label Label5 
      Caption         =   "Hobbs"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   240
      TabIndex        =   82
      Top             =   1560
      Width           =   1095
   End
   Begin VB.Label Label4 
      BackColor       =   &H00C0C0C0&
      Caption         =   "Elapsed Time"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000000FF&
      Height          =   255
      Left            =   8640
      TabIndex        =   71
      Top             =   1560
      Width           =   1575
   End
   Begin VB.Label Label3 
      BackColor       =   &H00C0C0C0&
      Caption         =   "Elevation"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   420
      Left            =   7800
      TabIndex        =   70
      Top             =   1080
      Width           =   1095
   End
   Begin VB.Label Label2 
      BackColor       =   &H00C0C0C0&
      Caption         =   "Azimuth"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   12
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   420
      Left            =   7800
      TabIndex        =   69
      Top             =   480
      Width           =   855
   End
   Begin VB.Label cmdRunSt 
      BackColor       =   &H00C0C0C0&
      Caption         =   "Start Time"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000000FF&
      Height          =   255
      Left            =   6840
      TabIndex        =   55
      Top             =   1560
      Width           =   975
   End
   Begin VB.Label Label1 
      BackColor       =   &H00C0C0C0&
      Caption         =   "4/04"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   165
      Left            =   6000
      TabIndex        =   36
      Top             =   480
      Width           =   735
   End
   Begin VB.Label lblSFile 
      BackColor       =   &H00C0C0C0&
      Caption         =   "Active Scan File ="
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   12
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000000FF&
      Height          =   375
      Left            =   4320
      MousePointer    =   1  'Arrow
      TabIndex        =   18
      Top             =   6360
      Width           =   1935
   End
   Begin VB.Label lblMain 
      BackColor       =   &H00C0C0C0&
      Caption         =   "NCAR Antenna Control Interface"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   18
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   372
      Left            =   600
      TabIndex        =   15
      Top             =   120
      Width           =   5292
   End
   Begin VB.Menu mnuSelSeq 
      Caption         =   "Select Seq"
      NegotiatePosition=   2  'Middle
      Begin VB.Menu mnuSeq1 
         Caption         =   "Seq1"
      End
      Begin VB.Menu mnuSeq2 
         Caption         =   "Seq 2"
      End
      Begin VB.Menu mnuSeq3 
         Caption         =   "Seq 3"
      End
      Begin VB.Menu mnuSeq4 
         Caption         =   "Seq  4"
      End
      Begin VB.Menu mnuSeq5 
         Caption         =   "Seq 5"
      End
      Begin VB.Menu mnuSeq6 
         Caption         =   "Seq 6"
      End
      Begin VB.Menu mnuSeq7 
         Caption         =   "Seq 7"
      End
      Begin VB.Menu mnuSeq8 
         Caption         =   "Seq 8"
      End
   End
   Begin VB.Menu mnuSelScan 
      Caption         =   "Select Scan"
      Begin VB.Menu mnuppi1 
         Caption         =   "PPI 1"
      End
      Begin VB.Menu mnuppi2 
         Caption         =   "PPI 2"
      End
      Begin VB.Menu mnuppi3 
         Caption         =   "PPi3"
      End
      Begin VB.Menu mnuppi4 
         Caption         =   "PPI 4"
      End
      Begin VB.Menu mnurhi1 
         Caption         =   "RHI1"
      End
      Begin VB.Menu mnurhi2 
         Caption         =   "RHI 2"
      End
      Begin VB.Menu mnurhi3 
         Caption         =   "RHI 3"
      End
      Begin VB.Menu mnusur1 
         Caption         =   "SUR 1"
      End
      Begin VB.Menu mnusur2 
         Caption         =   "SUR 2"
      End
      Begin VB.Menu mnuSur3 
         Caption         =   "SUR 3"
      End
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

 
Sub SaveToDsk()
Dim I, J, K As Integer
Dim x, Y, Z, W As Single



Open WKFILE For Output As #1

Write #1, "Fixed Angle Table"
For J = 0 To 29
    Write #1, FixedAng(J, 1), FixedAng(J, 2), FixedAng(J, 3), FixedAng(J, 4), FixedAng(J, 5), FixedAng(J, 6), FixedAng(J, 7), FixedAng(J, 8), FixedAng(J, 9), FixedAng(J, 10)
Next J

Write #1, "Ppi Definition Table"
For J = 0 To 15
    Write #1, PpiDef(J, 1), PpiDef(J, 2), PpiDef(J, 3), PpiDef(J, 4)
Next J

Write #1, "Rhi Definition Table"
For J = 0 To 15
    Write #1, RhiDef(J, 1), RhiDef(J, 2), RhiDef(J, 3), RhiDef(J, 4)
Next J
 
Write #1, "Survalence Definition Table"
For J = 0 To 15
    Write #1, SurDef(J, 1), SurDef(J, 2), SurDef(J, 3), SurDef(J, 4)
Next J
 
Write #1, "Sequence Definition Table"
For J = 0 To 9
    Write #1, SeqDef(J, 1), SeqDef(J, 2), SeqDef(J, 3), SeqDef(J, 4), SeqDef(J, 5), SeqDef(J, 6), SeqDef(J, 7), SeqDef(J, 8)
Next J


Write #1, "Sync Definition Table"
For J = 0 To 2
    Write #1, SynDef(J, 1), SynDef(J, 2), SynDef(J, 3), SynDef(J, 4), SynDef(J, 5), SynDef(J, 6), SynDef(J, 7)
Next J
 
Write #1, optPpi1.Caption
Write #1, optPpi2.Caption
Write #1, optPpi3.Caption
Write #1, optPpi4.Caption
Write #1, optRhi1.Caption
Write #1, optRhi2.Caption
Write #1, optRhi3.Caption
Write #1, optSur1.Caption
Write #1, optSur2.Caption
Write #1, optSur3.Caption

Write #1, optSeq1.Caption
Write #1, optseq2.Caption
Write #1, optSeq3.Caption
Write #1, optseq4.Caption
Write #1, optseq5.Caption
Write #1, optseq6.Caption
Write #1, optseq7.Caption

Close #1

Open "scansource" For Output As #1
Write #1, WKFILE
Write #1, AzStow, ElStow
Write #1, Hobbs
Close #1

retry1:
If ISAPMAC And Not SemTakePMAC(110) Then
    If PMACIoBusy <> 110 And Not PMACBlocked Then GoTo retry1
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "disable plc10" + Chr(13)
    PMACPORT.Output = "disable plc20" + Chr(13)
End If

Screen.MousePointer = vbHourglass
'frmMain.Enabled = False
frmPlWait.Show
frmPlWait.Refresh
EncodePpi
EncodeRhi
EncodeSur
EncodeFa
EncodeSeq
frmPlWait.Hide
'frmMain.Enabled = True
Screen.MousePointer = vbDefault


If ISAPMAC Then
    If Not PMACBlocked Then
        PMACPORT.Output = "enable plc10" + Chr(13)
        PMACPORT.Output = "enable plc20" + Chr(13)
    End If
    Do Until (SemGivePMAC(110) Or PMACIoBusy = 0)
    Loop
End If

End Sub
  
 
Sub EncodeFa()
Dim ID, I, P0, Fap As Integer
Dim Pstring As String * 20
Open "OutFat" For Output As #1


For ID = 1 To 10
    Select Case ID
      Case 1
        P0 = Afl1
      Case 2
        P0 = Afl2
      Case 3
        P0 = Afl3
      Case 4
        P0 = Afl4
      Case 5
        P0 = Afl5
      Case 6
        P0 = Afl6
      Case 7
        P0 = Afl7
      Case 8
        P0 = Afl8
      Case 9
        P0 = Afl9
      Case 10
        P0 = Afl10

    End Select

    For I = 0 To 32
        Pstring = Space(20)
        Pstring = "p" + Format(P0 + I) + "=" + Format(FixedAng(I, ID))
        If Debugum Then
            Print #1, Pstring
        End If
        If ISAPMAC Then
            PMACPORT.Output = Pstring + Chr(13)
            If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
        End If

        If (FixedAng(I, ID) = -999) Then
            GoTo xxxzzz     'bail at end of useful data
        End If

    Next I

xxxzzz: Next ID
Close #1
End Sub

'Write out all of the PPI Scan descriptors
Sub EncodePpi()
Dim ID, P0, Fap As Integer
Dim Pstring As String * 20
Close #1
Open "OutPPi" For Output As #1


For ID = 1 To 4
    Select Case ID
      Case 1
        P0 = Ppi1
      Case 2
        P0 = Ppi2
      Case 3
        P0 = Ppi3
      Case 4
        P0 = Ppi4
    End Select

    'Put in the Fixed Angle Pointers
    Select Case PpiDef(15, ID)
      Case 1
        Fap = Afl1
      Case 2
        Fap = Afl2
      Case 3
        Fap = Afl3
      Case 4
        Fap = Afl4
      Case 5
        Fap = Afl5
      Case 6
        Fap = Afl6
      Case 7
        Fap = Afl7
      Case 8
        Fap = Afl8
      Case 9
        Fap = Afl9
      Case 10
        Fap = Afl10
    End Select

    Pstring = Space(20)

    Pstring = "p" + Format(P0 + 1) + "=1" 'Scan Type
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 3) + "=" + Format(PpiDef(3, ID)) 'AZRPrint #1, "p" + Format(P0 + 3) + "=" + Format(PpiDef(3, ID)) 'AZR
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 4) + "=" + Format(PpiDef(4, ID)) 'AZL
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 7) + "=" + Format(PpiDef(7, ID)) 'RATE
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 10) + "=" + Format(PpiDef(10, ID)) 'SAMPLES
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 12) + "=" + Format(PpiDef(12, ID)) 'AZoff
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 13) + "=" + Format(PpiDef(13, ID)) 'Eloff
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 15) + "=" + Format(Fap) 'FA POINT
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

Next ID
Close #1

End Sub


Sub EncodeRhi()
'Write out all of the Rhi scan descriptors
Dim ID, P0, Fap As Integer
Dim Pstring As String * 20

Open "OutRhi" For Output As #1


For ID = 1 To 3
    Select Case ID
      Case 1
        P0 = Rhi1  'p0 is the Base P Variable address
      Case 2
        P0 = Rhi2
      Case 3
        P0 = Rhi3
    End Select

    'Put in the Fixed Angle Pointers
    Select Case RhiDef(15, ID)
      Case 1
        Fap = Afl1
      Case 2
        Fap = Afl2
      Case 3
        Fap = Afl3
      Case 4
        Fap = Afl4
      Case 5
        Fap = Afl5
      Case 6
        Fap = Afl6
      Case 7
        Fap = Afl7
      Case 8
        Fap = Afl8
      Case 9
        Fap = Afl9
      Case 10
        Fap = Afl10
    End Select

    Pstring = Space(20)

    Pstring = "p" + Format(P0 + 1) + "=2" 'Scan Type
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If
    Pstring = "p" + Format(P0 + 5) + "=" + Format(RhiDef(5, ID)) 'ELB
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If
    Pstring = "p" + Format(P0 + 6) + "=" + Format(RhiDef(6, ID)) 'ELT
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If
    Pstring = "p" + Format(P0 + 7) + "=" + Format(RhiDef(7, ID)) 'RATE
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If
    Pstring = "p" + Format(P0 + 10) + "=" + Format(RhiDef(10, ID)) 'SAMPLES
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If
    Pstring = "p" + Format(P0 + 12) + "=" + Format(RhiDef(12, ID)) 'AZoff
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 13) + "=" + Format(RhiDef(13, ID)) 'Eloff
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 15) + "=" + Format(Fap) 'FA POINT
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

Next ID
Close #1

End Sub


Sub EncodeSeq()
Dim ID As Integer, P0 As Integer, PT As Integer, D0 As Integer, J As Integer, KK As Integer
Dim Pstring As String * 20, Pstring1 As String * 20

Open "OutSeq" For Output As #1

'for each sequence
'download the sync dummy scans to the PMAC these really are not sequences but they are determined in the sequence window
For KK = 1 To 7
    Select Case KK
      Case 1
        P0 = Sync1      'address in the pmac memory
      Case 2
        P0 = Sync2
      Case 3
        P0 = Sync3
      Case 4
        P0 = Sync4
      Case 5
        P0 = Sync5
      Case 6
        P0 = Sync6
      Case 7
        P0 = Sync7
    End Select

    Pstring = Space(20)
    Pstring1 = Space(20)

    Pstring = "P" + Format(P0 + STYPE) + "=" + Format(SynDef(STYPE, KK))             'this should always be =5
    Pstring1 = "P" + Format(P0 + SModulo) + "=" + Format(SynDef(SModulo, KK))        'this is the modulo
    If Debugum Then
        Print #1, Pstring
        Print #1, Pstring1
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        PMACPORT.Output = Pstring1 + Chr(13)
        If (Twice) Then
            PMACPORT.Output = Pstring + Chr(13)
            PMACPORT.Output = Pstring1 + Chr(13)
        End If
    End If
Next KK

'for each sequence
For ID = 1 To 7
    Select Case ID
      Case 1
        P0 = Seq1
      Case 2
        P0 = Seq2
      Case 3
        P0 = Seq3
      Case 4
        P0 = Seq4
      Case 5
        P0 = Seq5
      Case 6
        P0 = Seq6
      Case 7
        P0 = Seq7
    End Select

    For J = 0 To 10
        PT = SeqDef(J, ID)  ' this is the scan# in the seq
        Select Case PT
          Case 1              'Ppi1
            D0 = Ppi1
          Case 2              'ppi2
            D0 = Ppi2
          Case 3              'ppi3
            D0 = Ppi3
          Case 4              'PPI4
            D0 = Ppi4
          Case 5              'RHI1
            D0 = Rhi1
          Case 6              'RHI2
            D0 = Rhi2
          Case 7              'RHI3
            D0 = Rhi3
          Case 8              'SUR1
            D0 = Sur1
          Case 9              'SUR2
            D0 = Sur2
          Case 10             'SUR3
            D0 = Sur3
          Case 11             'Sync
            Select Case ID
              Case 1
                D0 = Sync1
              Case 2
                D0 = Sync2
              Case 3
                D0 = Sync3
              Case 4
                D0 = Sync4
              Case 5
                D0 = Sync5
              Case 6
                D0 = Sync6
              Case 7
                D0 = Sync7
            End Select
          Case -2
            D0 = -2
        End Select

        Pstring = Space(20)
        Pstring = "P" + Format(P0 + J) + "=" + Format(D0)
        If Debugum Then
            Print #1, Pstring
        End If
        If ISAPMAC Then
            PMACPORT.Output = Pstring + Chr(13)
            If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
        End If
    Next J
Next ID

Close #1
End Sub

Sub EncodeSur()
'Write out all of the Surveillence scan descriptors
Dim ID, P0, Fap As Integer
Dim Pstring As String * 20

Open "OutSur" For Output As #1

For ID = 1 To 3
    Select Case ID
      Case 1
        P0 = Sur1    'p0 is the Base P Variable address
      Case 2
        P0 = Sur2
      Case 3
        P0 = Sur3
    End Select

    'Put in the Fixed Angle Pointers
    Select Case SurDef(15, ID)
      Case 1
        Fap = Afl1
      Case 2
        Fap = Afl2
      Case 3
        Fap = Afl3
      Case 4
        Fap = Afl4
      Case 5
        Fap = Afl5
      Case 6
        Fap = Afl6
      Case 7
        Fap = Afl7
      Case 8
        Fap = Afl8
      Case 9
        Fap = Afl9
      Case 10
        Fap = Afl10
    End Select
    
    Pstring = Space(20)

    Pstring = "p" + Format(P0 + 1) + "=3" 'Scan Type
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 3) + "=" + Format(SurDef(3, ID)) 'AZR
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 4) + "=" + Format(SurDef(4, ID)) 'AZL
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 7) + "=" + Format(SurDef(7, ID)) 'RATE
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 10) + "=" + Format(SurDef(10, ID)) 'SAMPLES
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 12) + "=" + Format(SurDef(12, ID)) 'Azoff
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 13) + "=" + Format(SurDef(13, ID)) 'Eloff
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If

    Pstring = "p" + Format(P0 + 15) + "=" + Format(Fap) 'FA POINT
    If Debugum Then
        Print #1, Pstring
    End If
    If ISAPMAC Then
        PMACPORT.Output = Pstring + Chr(13)
        If (Twice) Then PMACPORT.Output = Pstring + Chr(13)
    End If
    
Next ID
Close #1

End Sub


Public Sub LoadFrmDsk()
Dim I, J, K As Integer
Dim x, Y, Z, W As Single
Dim Temp, Name

On Error GoTo Erhand

Open "ScanSource" For Input As #1
Input #1, WKFILE
Input #1, AzStow, ElStow
Input #1, Z
Close #1

Open WKFILE For Input As #1

Input #1, Temp
For J = 0 To 29
    Input #1, FixedAng(J, 1), FixedAng(J, 2), FixedAng(J, 3), FixedAng(J, 4), FixedAng(J, 5), FixedAng(J, 6), FixedAng(J, 7), FixedAng(J, 8), FixedAng(J, 9), FixedAng(J, 10)
Next J

Input #1, Temp
For J = 0 To 15
    Input #1, PpiDef(J, 1), PpiDef(J, 2), PpiDef(J, 3), PpiDef(J, 4)
Next J

Input #1, Temp
For J = 0 To 15
    Input #1, RhiDef(J, 1), RhiDef(J, 2), RhiDef(J, 3), RhiDef(J, 4)
Next J
 
Input #1, Temp
For J = 0 To 15
    Input #1, SurDef(J, 1), SurDef(J, 2), SurDef(J, 3), SurDef(J, 4)
Next J
 
Input #1, Temp
For J = 0 To 9
    Input #1, SeqDef(J, 1), SeqDef(J, 2), SeqDef(J, 3), SeqDef(J, 4), SeqDef(J, 5), SeqDef(J, 6), SeqDef(J, 7), SeqDef(J, 8)
Next J

Input #1, Temp
For J = 0 To 2
    Input #1, SynDef(J, 1), SynDef(J, 2), SynDef(J, 3), SynDef(J, 4), SynDef(J, 5), SynDef(J, 6), SynDef(J, 7)
Next J

Input #1, Name
optPpi1.Caption = Name
Input #1, Name
optPpi2.Caption = Name
Input #1, Name
optPpi3.Caption = Name
Input #1, Name
optPpi4.Caption = Name
Input #1, Name
optRhi1.Caption = Name
Input #1, Name
optRhi2.Caption = Name
Input #1, Name
optRhi3.Caption = Name
Input #1, Name
optSur1.Caption = Name
Input #1, Name
optSur2.Caption = Name
Input #1, Name
optSur3.Caption = Name

Input #1, Name
optSeq1.Caption = Name
Input #1, Name
optseq2.Caption = Name
Input #1, Name
optSeq3.Caption = Name
Input #1, Name
optseq4.Caption = Name
Input #1, Name
optseq5.Caption = Name
Input #1, Name
optseq6.Caption = Name
Input #1, Name
optseq7.Caption = Name


PpiFa1.Text = PpiDef(FAPOINT, 1)
PpiFa2.Text = PpiDef(FAPOINT, 2)
PpiFa3.Text = PpiDef(FAPOINT, 3)
PpiFa4.Text = PpiDef(FAPOINT, 4)


SurFa1.Text = SurDef(FAPOINT, 1)
SurFa2.Text = SurDef(FAPOINT, 2)
SurFa3.Text = SurDef(FAPOINT, 3)


RhiFa1.Text = RhiDef(FAPOINT, 1)
RhiFa2.Text = RhiDef(FAPOINT, 2)
RhiFa3.Text = RhiDef(FAPOINT, 3)


Close #1
frmMain.txbSFile.Text = WKFILE
Exit Sub

Erhand:
Select Case Err.Number  ' Evaluate error number.
        Case 53 ' "File does not exist" error.
           frmMain.txbSFile.Text = "None"
           Beep
           MsgBox "File Does Not Exist", vbExclamation
           
        Case Else
            ' Handle other situations here...
           Beep
           MsgBox "File Open Error???", vbExclamation
End Select
Close #1    ' Close open file.
    

End Sub


Private Sub cmdBump_Click()
Dim J
If (ISAPMAC) Then
    J = MsgBox("Do You Really Want to Abort this Scan and Continue to the next?", vbOKCancel)
    If (J = vbOK) Then
        If PutPMACPVal(130, 1, 145) Then
            'all is good
            SemGivePMAC (145)
        End If
    End If
End If
End Sub

Private Sub CmdDump_Click()
Dim J

J = MsgBox("Overwrite Existing File?", vbYesNo)
If J = vbYes Then
    SaveToDsk
End If
CmdDump.Caption = "Scans Saved"

End Sub


Private Sub cmdFile_Click()
frmFiles.Show
End Sub

Private Sub cmdhlprun_Click()
MsgBox "Run one scan Forever"

End Sub

Private Sub cmdhlpseq_Click()
MsgBox "Run one Sequence Forever"

End Sub

Private Sub cmdPPiEdit_Click()
frmViewPPI.Hide
frmViewRhi.Hide
frmViewSur.Hide
frmPPI.Show
End Sub

Private Sub cmdPpiView_Click()
If frmPPI.Visible Then
    MsgBox "Close Edit Window First", vbExclamation
    frmPPI.Show
Else
    frmViewPPI.Show
    frmViewRhi.Hide
    frmViewSur.Hide
End If

End Sub

Private Sub cmdquit_Click()
Dim J As Integer
If (ISRUNNING) Then
    Beep
    MsgBox "Stop Antenna First", vbOKOnly
    Exit Sub
End If
J = MsgBox("Are You Sure You Want To Quit?", vbYesNo)
If J = vbNo Then
    Exit Sub
End If
J = MsgBox("Save and Overwrite Existing File?", vbYesNo)
If J = vbYes Then
    SaveToDsk
End If
End
End Sub

Private Sub cmdRhiEdit_Click()
frmViewPPI.Hide
frmViewRhi.Hide
frmViewSur.Hide
frmRhi.Show
End Sub

Private Sub cmdRhiView_Click()
If frmRhi.Visible Then
    MsgBox "Close Edit Window First", vbExclamation
Else
frmViewRhi.Show
frmViewPPI.Hide
frmViewSur.Hide
End If
End Sub

Private Sub cmdRun_Click()
 Unload frmSpecial
 Unload frmSolar
 Unload frmPoint

If ISRUNNING Then
    Beep
    MsgBox " Stop Antenna First", vbExclamation
    Exit Sub
End If

frmViewPPI.Hide
frmViewRhi.Hide
frmViewSur.Hide

mnuppi1.Caption = frmMain.optPpi1.Caption
mnuppi2.Caption = frmMain.optPpi2.Caption
mnuppi3.Caption = frmMain.optPpi3.Caption
mnuppi4.Caption = frmMain.optPpi4.Caption
mnurhi1.Caption = frmMain.optRhi1.Caption
mnurhi2.Caption = frmMain.optRhi2.Caption
mnurhi3.Caption = frmMain.optRhi3.Caption
mnusur1.Caption = frmMain.optSur1.Caption
mnusur2.Caption = frmMain.optSur2.Caption
mnuSur3.Caption = frmMain.optSur3.Caption

If ISAPMAC And Not SemTakePMAC(120) Then Exit Sub
'EncodePpi
'EncodeRhi
'EncodeSur
'EncodeFa
If ISAPMAC Then SemGivePMAC (120)

PopupMenu mnuSelScan
End Sub

Private Sub cmdRunSeq_Click()
Unload frmSpecial
Unload frmSolar
Unload frmPoint
 
If (ISRUNNING) Then
    Beep
    MsgBox " Stop Antenna First", vbExclamation
    Exit Sub
End If
frmViewPPI.Hide
frmViewRhi.Hide
frmViewSur.Hide

mnuSeq1.Caption = frmMain.optSeq1.Caption
mnuSeq2.Caption = frmMain.optseq2.Caption
mnuSeq3.Caption = frmMain.optSeq3.Caption
mnuSeq4.Caption = frmMain.optseq4.Caption
mnuSeq5.Caption = frmMain.optseq5.Caption
mnuSeq6.Caption = frmMain.optseq6.Caption
mnuSeq7.Caption = frmMain.optseq7.Caption

If ISAPMAC And Not SemTakePMAC(130) Then Exit Sub
'EncodePpi
'EncodeRhi
'EncodeSur
'EncodeFa
If ISAPMAC Then SemGivePMAC (130)

PopupMenu mnuSelSeq
cmdBump.Visible = True

End Sub

Private Sub cmdSeqEdit_Click()
'frmMain.Enabled = False
frmMain.Hide
frmSeq.Show
End Sub

Private Sub cmdstop_Click()
Dim J
Dim ier As Integer
Dim Instring As String * 20
NTimes = 0                  'for malfunction message
AntMalf = False

'this is here just to write out the Hobbs time
Open "ScanSource" For Output As #1
Write #1, WKFILE
Write #1, AzStow, ElStow
Write #1, Hobbs
Close #1
'
cmdstop.Caption = "TRYING"
TmrClk.Enabled = False
SDelay (1)
Instring = PMACPORT.Input   'clear out input buffer
frmMain.Enabled = False
'frmMain.Hide
txtST.Text = ""
If ISRUNNING Then
    cmdstop.Caption = "Reset  PMAC"
    cmdstop.BackColor = &H80FF&
    If ISAPMAC And SemTakePMAC(140) Then
        PMACPORT.Output = Chr(26)                        'this is ctrlZ
        PMACPORT.Output = "P50=0" + Chr(13)
        PMACPORT.Output = "P51=0" + Chr(13)
        PMACPORT.Output = "P52=0" + Chr(13)
        SemGivePMAC (140)
        SDelay (2)
    End If
    cmdrun.Caption = "Run"
    cmdrun.BackColor = &H80FF80
    cmdRunSeq.Caption = "Run Sequence"
    cmdBump.Visible = False
    cmdRunSeq.BackColor = &H80
    ISRUNNING = False
    'frmMain.Enabled = True
    'frmMain.Show

Else
    If Not ISAPMAC Then
        MsgBox "PMAC connection not enabled"
        frmMain.Enabled = True
        'frmMain.Show
        Exit Sub
    End If
    'Stop Everything
    If SemTakePMAC(141) Then
        PMACPORT.Output = Chr(26) 'this is ctrlZ

        PMACPORT.Output = "P50=0" + Chr(13)
        PMACPORT.Output = "P51=0" + Chr(13)
        PMACPORT.Output = "P52=0" + Chr(13)
        SDelay (0.25)  'This allows the PMAC to respond to the p50=0 command above

        J = GetPMACPVal(50, ier)
        If ier = 0 Then
            If J = -1 Then
                cmdstop.Caption = "PMAC OK"
                MalfEnb = True
            Else
                cmdstop.Caption = "PMAC NOT RUNNING"
            End If
        End If
        SemGivePMAC (141)
    Else
        MsgBox "PMAC Communications STALLED"
    End If
    SDelay (1)
    cmdstop.Caption = "RESET PMAC"
End If

frmMain.Enabled = True
TmrClk.Enabled = True

'frmMain.Show
End Sub

Private Sub cmdStow_Click()
Dim zzz As String
zzz = MsgBox("Do You Really Want To Stow the Antenna?", vbYesNo + vbQuestion)
If zzz = vbYes And ISAPMAC And SemTakePMAC(142) Then
    PMACPORT.Output = "P50=0" + Chr(13)
    SDelay (2)
    ISRUNNING = True
    PMACPORT.Output = "p51=" + Format(AzStow) + Chr(13) 'Az
    PMACPORT.Output = "p52=" + Format(ElStow) + Chr(13) 'El
    PMACPORT.Output = "p50=3" + Chr(13)
    SemGivePMAC (142)
    SDelay (20)
    ISRUNNING = False
End If
End Sub

Private Sub cmdSurEdit_Click()
frmViewPPI.Hide
frmViewRhi.Hide
frmViewSur.Hide
frmSur.Show
End Sub

Private Sub cmdSurView_Click()
If frmSur.Visible Then
    MsgBox "Close Edit Window First", vbExclamation
Else
    frmViewSur.Show
    frmViewPPI.Hide
    frmViewRhi.Hide
End If
End Sub

Private Sub Command11_Click()

End Sub

Private Sub Command12_Click()
Dim I, J, K As Integer
Dim x, Y, Z, W As Single

Open "Barf" For Output As #1

Write #1, "Fixed Angle Table"
For J = 0 To 29
    Write #1, FixedAng(J, 1), FixedAng(J, 2), FixedAng(J, 3), FixedAng(J, 4)
Next J

Write #1, "Ppi Definition Table"
For J = 0 To 15
    Write #1, PpiDef(J, 1), PpiDef(J, 2), PpiDef(J, 3), PpiDef(J, 4)
Next J

Write #1, "Rhi Definition Table"
For J = 0 To 15
    Write #1, RhiDef(J, 1), RhiDef(J, 2), RhiDef(J, 3), RhiDef(J, 4)
Next J
 
Write #1, "Survalence Definition Table"
For J = 0 To 15
    Write #1, SurDef(J, 1), SurDef(J, 2), SurDef(J, 3), SurDef(J, 4)
Next J
 

Write #1, optPpi1.Caption

Close #1
If ISAPMAC And Not SemTakePMAC(143) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "disable plc10" + Chr(13)
    PMACPORT.Output = "disable plc20" + Chr(13)
End If
Screen.MousePointer = vbHourglass
'frmMain.Enabled = False
frmPlWait.Show
frmPlWait.Refresh
EncodePpi
EncodeRhi
EncodeSur
EncodeFa
frmPlWait.Hide
'frmMain.Enabled = True
Screen.MousePointer = vbDefault
If ISAPMAC Then
    PMACPORT.Output = "enable plc10" + Chr(13)
    PMACPORT.Output = "enable plc20" + Chr(13)
    SemGivePMAC (143)
End If
End Sub

Private Sub Command10_Click()

End Sub

Private Sub cndSpecial_Click()
Dim zzz

If (ImaDow = 20) Then                    'password required for SPOL (NOT CP2 made 20)
    zzz = InputBox("Whats the Password?")
    If Val(zzz) = 5151 Then
       frmSpecial.Show
    Else
       Beep
      Exit Sub
    End If
Else
    frmSpecial.Show
End If

End Sub

Private Sub Command1_Click()

End Sub

Private Sub Command13_Click()
LoadFrmDsk
End Sub

Private Sub Command2_Click()
frmPPI.Show
End Sub

Private Sub Command3_Click()
frmRhi.Show
End Sub

Private Sub Command4_Click()
frmViewRhi.Show
End Sub

Private Sub Command5_Click()
frmSur.Show
End Sub

Private Sub Command6_Click()
frmViewSur.Show
End Sub

Private Sub Command7_Click()

End Sub

Private Sub Efa1_Click()
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "1"

End Sub

Private Sub Efa10_Click()
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "10"
End Sub

Private Sub Efa2_Click()
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "2"

End Sub

Private Sub Efa3_Click()
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "3"

End Sub

Private Sub Efa4_Click()
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "4"

End Sub

Private Sub Efa5_Click()
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "5"
End Sub

Private Sub Efa6_Click()
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "6"
End Sub

Private Sub Efa7_Click()
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "7"
End Sub

Private Sub Efa8_Click()
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "8"
End Sub

Private Sub Efa9_Click()
frmFang.lstAng.Clear
frmFang.Show
frmFang.txtFaid.Text = "9"
End Sub

Private Sub Form_Load()
Dim Instring
Dim jer As Integer

ISAPMAC = True

TmrClk.Enabled = False 'turn off the timer task until things settle down

cmdBump.Visible = False
'-------------------------------------------------------------------------------------
'Open the PMAC Port
If ISAPMAC Then
    Set PMACPORT = MSComm1 ' the one here doesn't matter
    PMACBlocked = False
    PMACBlockedAck = False
    PMACIoBusy = 0
    If Not SemTakePMAC(100, , True) Then Exit Sub

    PMACPORT.CommPort = PComPort   ' I think this is the port number
    PMACPORT.Settings = "38400,N,8,1"
    PMACPORT.InputLen = 0
    PMACPORT.DTREnable = True
    PMACPORT.PortOpen = True
    PMACPORT.Output = Chr(26)          'This is ctrlZ PMAC enable serial communications
    PMACPORT.Output = "i3=0" + Chr(13) 'This sets the communications mode in the PMAC
End If
'-------------------------------------------------------------------------------------
'Check for fake angles and also for proper serial communications with the PMAC
jer = 0
Call CheckFakeAngles(jer)
If ISAPMAC And jer <> 0 Then
    If (jer = 1) Then
        MsgBox "PMAC Serial Communications Failure"
        ISAPMAC = False
    ElseIf (jer = 2) Then
        MsgBox "PMAC Communications Inconsistant"
        End
    Else
        MsgBox "PMAC - Unknown Communications Error (" + Format(jer) + ")"
        End
    End If
End If
'-------------------------------------------------------------------------------------

'set the warning message
If Not ISAPMAC Then
    TxtFake.Text = "NO PMAC/Antenna"
    TxtFake.BackColor = RGB(255, 0, 0)
ElseIf FakeAngles Then
    TxtFake.Text = "Simulated Antenna"
    TxtFake.BackColor = RGB(255, 0, 0)
Else
    TxtFake.Text = "Real Antenna"
    TxtFake.BackColor = RGB(0, 255, 0)
End If
'--------------------------------------------------------------------------------------

If ISAPMAC Then
    PMACPORT.Output = "P51=0" + Chr(13)
    PMACPORT.Output = "P52=0" + Chr(13)


    PMACPORT.Output = "i5=3" + Chr(13)   'enable plc's
    PMACPORT.Output = "p168=0" + Chr(13) 'disable x10 azimuth expand
    PMACPORT.Output = "enable plc0" + Chr(13)
End If

'This is to read in Hobbs only when the program is started
'Other similar calls use a dummy in the Hobbs position

Open "ScanSource" For Input As #1
Input #1, WKFILE
Input #1, AzStow, ElStow
Input #1, Hobbs
Close #1
TxbHobbs.Text = Format(Val(Hobbs), "0000.0") 'write out hobbs


LoadFrmDsk

If ISAPMAC Then
    PMACPORT.Output = "disable plc10" + Chr(13)
    PMACPORT.Output = "disable plc20" + Chr(13)
End If
Screen.MousePointer = vbHourglass
'frmMain.Enabled = False
frmPlWait.Show
frmPlWait.Refresh
EncodePpi
EncodeRhi
EncodeSur
EncodeFa
EncodeSeq
frmPlWait.Hide
'frmMain.Enabled = True
Screen.MousePointer = vbDefault
If ISAPMAC Then
    PMACPORT.Output = "enable plc10" + Chr(13)
    PMACPORT.Output = "enable plc20" + Chr(13)

    'set the clock in the PMAC
    PMACPORT.Output = "P53=" + Format(Minute(Now) + Second(Now) / 60) + Chr(13)
    PMACPORT.Output = "P50=99" + Chr(13)

    SemGivePMAC (100)
End If

frmKill.Show

TmrClk.Enabled = True
End Sub

Sub CheckFakeAngles(jer)
Dim V As Single, ier As Integer

V = GetPMACPVal(49, ier)
'ier = 0
If ier = 0 Then
    If V = 1234 Then
        FakeAngles = True
    Else
        FakeAngles = False
    End If
Else
    If ier = 1 Then jer = 1
    If ier = 2 Then jer = 2
End If
End Sub

Private Sub Option7_Click()

End Sub


Private Sub mnuPpi1_Click()

If ISAPMAC And Not SemTakePMAC(151) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p630=400" + Chr(13)
    PMACPORT.Output = "p631=400" + Chr(13)
    PMACPORT.Output = "p632=400" + Chr(13)
    PMACPORT.Output = "p633=400" + Chr(13)
    PMACPORT.Output = "p634=-2" + Chr(13)
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (151)
End If
txtST.Text = Now

Etime = Timer
cmdrun.Caption = "Running " + frmMain.optPpi1.Caption
cmdrun.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuPpi2_Click()

If ISAPMAC And Not SemTakePMAC(152) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p630=416" + Chr(13)
    PMACPORT.Output = "p631=416" + Chr(13)
    PMACPORT.Output = "p632=416" + Chr(13)
    PMACPORT.Output = "p633=416" + Chr(13)
    PMACPORT.Output = "p634=-2" + Chr(13)
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (152)
End If
txtST.Text = Now
Etime = Timer
cmdrun.Caption = "Running " + frmMain.optPpi2.Caption
cmdrun.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True
End Sub

Private Sub mnuPPi3_Click()

If ISAPMAC And Not SemTakePMAC(153) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p630=432" + Chr(13)
    PMACPORT.Output = "p631=432" + Chr(13)
    PMACPORT.Output = "p632=432" + Chr(13)
    PMACPORT.Output = "p633=432" + Chr(13)
    PMACPORT.Output = "p634=-2" + Chr(13)
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (153)
End If
txtST.Text = Now
Etime = Timer
cmdrun.Caption = "Running " + frmMain.optPpi3.Caption
cmdrun.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True
End Sub

Private Sub mnuPpi4_Click()

If ISAPMAC And Not SemTakePMAC(154) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p630=448" + Chr(13)
    PMACPORT.Output = "p631=448" + Chr(13)
    PMACPORT.Output = "p632=448" + Chr(13)
    PMACPORT.Output = "p633=448" + Chr(13)
    PMACPORT.Output = "p634=-2" + Chr(13)
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (154)
End If
txtST.Text = Now
Etime = Timer
cmdrun.Caption = "Running " + frmMain.optPpi4.Caption
cmdrun.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True
End Sub

Private Sub mnuRhi1_Click()

If ISAPMAC And Not SemTakePMAC(161) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p630=464" + Chr(13)
    PMACPORT.Output = "p631=464" + Chr(13)
    PMACPORT.Output = "p632=464" + Chr(13)
    PMACPORT.Output = "p633=464" + Chr(13)
    PMACPORT.Output = "p634=-2" + Chr(13)
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (161)
End If
txtST.Text = Now
Etime = Timer
cmdrun.Caption = "Running " + frmMain.optRhi1.Caption
cmdrun.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuRhi2_Click()

If ISAPMAC And Not SemTakePMAC(162) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p630=480" + Chr(13)
    PMACPORT.Output = "p631=480" + Chr(13)
    PMACPORT.Output = "p632=480" + Chr(13)
    PMACPORT.Output = "p633=480" + Chr(13)
    PMACPORT.Output = "p634=-2" + Chr(13)
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (162)
End If
txtST.Text = Now
Etime = Timer
cmdrun.Caption = "Running " + frmMain.optRhi2.Caption
cmdrun.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuRhi3_Click()

If ISAPMAC And Not SemTakePMAC(163) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p630=496" + Chr(13)
    PMACPORT.Output = "p631=496" + Chr(13)
    PMACPORT.Output = "p632=496" + Chr(13)
    PMACPORT.Output = "p633=496" + Chr(13)
    PMACPORT.Output = "p634=-2" + Chr(13)
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (163)
End If
txtST.Text = Now
Etime = Timer
cmdrun.Caption = "Running " + frmMain.optRhi3.Caption
cmdrun.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuSur1_Click()
If ISAPMAC And Not SemTakePMAC(171) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p630=512" + Chr(13)
    PMACPORT.Output = "p631=512" + Chr(13)
    PMACPORT.Output = "p632=512" + Chr(13)
    PMACPORT.Output = "p633=512" + Chr(13)
    PMACPORT.Output = "p634=-2" + Chr(13)
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (171)
End If
txtST.Text = Now
Etime = Timer
cmdrun.Caption = "Running " + frmMain.optSur1.Caption
cmdrun.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuSur2_Click()

If ISAPMAC And Not SemTakePMAC(172) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p630=528" + Chr(13)
    PMACPORT.Output = "p631=528" + Chr(13)
    PMACPORT.Output = "p632=528" + Chr(13)
    PMACPORT.Output = "p633=528" + Chr(13)
    PMACPORT.Output = "p634=-2" + Chr(13)
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (172)
End If
txtST.Text = Now
Etime = Timer
cmdrun.Caption = "Running " + frmMain.optSur2.Caption
cmdrun.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True
End Sub

Private Sub mnuSur3_Click()

If ISAPMAC And Not SemTakePMAC(173) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p630=544" + Chr(13)
    PMACPORT.Output = "p631=544" + Chr(13)
    PMACPORT.Output = "p632=544" + Chr(13)
    PMACPORT.Output = "p633=544" + Chr(13)
    PMACPORT.Output = "p634=-2" + Chr(13)
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (173)
End If
txtST.Text = Now
Etime = Timer
cmdrun.Caption = "Running " + frmMain.optSur3.Caption
cmdrun.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub
Private Sub mnuSeq1_Click()

If ISAPMAC And Not SemTakePMAC(181) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p50=10" + Chr(13)
    SemGivePMAC (181)
End If
txtST.Text = Now
Etime = Timer
cmdRunSeq.Caption = "Running " + frmMain.optSeq1.Caption
cmdRunSeq.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuSeq2_Click()

If ISAPMAC And Not SemTakePMAC(182) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p50=11" + Chr(13)
    SemGivePMAC (182)
End If
txtST.Text = Now
Etime = Timer
cmdRunSeq.Caption = "Running " + frmMain.optseq2.Caption
cmdRunSeq.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuSeq3_Click()

If ISAPMAC And Not SemTakePMAC(183) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p50=12" + Chr(13)
    SemGivePMAC (183)
End If
txtST.Text = Now
Etime = Timer
cmdRunSeq.Caption = "Running " + frmMain.optSeq3.Caption
cmdRunSeq.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuSeq4_Click()

If ISAPMAC And Not SemTakePMAC(184) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p50=13" + Chr(13)
    SemGivePMAC (184)
End If
txtST.Text = Now
Etime = Timer
cmdRunSeq.Caption = "Running " + frmMain.optseq4.Caption
cmdRunSeq.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuSeq5_Click()

If ISAPMAC And Not SemTakePMAC(185) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p50=14" + Chr(13)
    SemGivePMAC (185)
End If
txtST.Text = Now
Etime = Timer
cmdRunSeq.Caption = "Running " + frmMain.optseq5.Caption
cmdRunSeq.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuSeq6_Click()

If ISAPMAC And Not SemTakePMAC(186) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p50=15" + Chr(13)
    SemGivePMAC (186)
End If
txtST.Text = Now
Etime = Timer
cmdRunSeq.Caption = "Running " + frmMain.optseq6.Caption
cmdRunSeq.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True

End Sub

Private Sub mnuSeq7_Click()

If ISAPMAC And Not SemTakePMAC(187) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p50=16" + Chr(13)
    SemGivePMAC (187)
End If
txtST.Text = Now
Etime = Timer
cmdRunSeq.Caption = "Running " + frmMain.optseq7.Caption
cmdRunSeq.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True
End Sub

Private Sub mnuSeq8_Click()
If ISAPMAC And Not SemTakePMAC(188) Then
    Exit Sub
ElseIf ISAPMAC Then
    PMACPORT.Output = "p50=17" + Chr(13)
    SemGivePMAC (188)
End If
txtST.Text = Now
Etime = Timer
'cmdRunSeq.Caption = "Running " + frmMain.optseq8.Caption
cmdRunSeq.BackColor = &H8000&
cmdstop.Caption = "STOP"
cmdstop.BackColor = &HFF&
ISRUNNING = True
End Sub



Private Sub Option2_Click()

End Sub

Private Sub Option9_Click()

End Sub

Private Sub Option1_Click()
End Sub

Private Sub optPpi1_DblClick()
Dim Name
Name = InputBox("Enter Scan Name")
If Name = "" Then
    Exit Sub
End If
optPpi1.Caption = Name
End Sub


Private Sub optPpi2_DblClick()
Dim Name
Name = InputBox("Enter Scan Name")
If Name = "" Then
    Exit Sub
End If
optPpi2.Caption = Name
End Sub

Private Sub optPpi3_DblClick()
Dim Name
Name = InputBox("Enter Scan Name")
If Name = "" Then
    Exit Sub
End If
optPpi3.Caption = Name
End Sub

Private Sub optPpi4_DblClick()
Dim Name
Name = InputBox("Enter Scan Name")
If Name = "" Then
    Exit Sub
End If
optPpi4.Caption = Name
End Sub


Private Sub optRhi1_DblClick()
Dim Name
Name = InputBox("Enter Scan Name")
If Name = "" Then
    Exit Sub
End If
optRhi1.Caption = Name
End Sub

Private Sub optRhi2_DblClick()
Dim Name
Name = InputBox("Enter Scan Name")
If Name = "" Then
    Exit Sub
End If
optRhi2.Caption = Name
End Sub

Private Sub optRhi3_DblClick()
Dim Name
Name = InputBox("Enter Scan Name")
If Name = "" Then
    Exit Sub
End If
optRhi3.Caption = Name
End Sub

Private Sub optSeq1_DblClick()
Dim Name
Name = InputBox("Enter Sequence Name")
If Name = "" Then
    Exit Sub
End If
optSeq1.Caption = Name
End Sub

Private Sub optseq2_DblClick()
Dim Name
Name = InputBox("Enter Sequence Name")
If Name = "" Then
    Exit Sub
End If
optseq2.Caption = Name
End Sub

Private Sub optseq3_DblClick()
Dim Name
Name = InputBox("Enter Sequence Name")
If Name = "" Then
    Exit Sub
End If
optSeq3.Caption = Name
End Sub

Private Sub optseq4_DblClick()
Dim Name
Name = InputBox("Enter Sequence Name")
If Name = "" Then
    Exit Sub
End If
optseq4.Caption = Name
End Sub

Private Sub optseq5_DblClick()
Dim Name
Name = InputBox("Enter Sequence Name")
If Name = "" Then
    Exit Sub
End If
optseq5.Caption = Name
End Sub

Private Sub optseq6_DblClick()
Dim Name
Name = InputBox("Enter Sequence Name")
If Name = "" Then
Exit Sub
End If
optseq6.Caption = Name
End Sub

Private Sub optseq7_DblClick()
Dim Name
Name = InputBox("Enter Sequence Name")
If Name = "" Then
    Exit Sub
End If
optseq7.Caption = Name
End Sub

Private Sub optseq8_DblClick()
Dim Name
Name = InputBox("Enter Sequence Name")
If Name = "" Then
    Exit Sub
End If
'optseq8.Caption = "Don't Use"   'Name
End Sub

Private Sub optSur1_DblClick()
Dim Name
Name = InputBox("Enter Scan Name")
If Name = "" Then
    Exit Sub
End If
optSur1.Caption = Name
End Sub

Private Sub optSur2_DblClick()
Dim Name
Name = InputBox("Enter Scan Name")
If Name = "" Then
    Exit Sub
End If
optSur2.Caption = Name
End Sub

Private Sub optSur3_DblClick()
Dim Name
Name = InputBox("Enter Scan Name")
If Name = "" Then
    Exit Sub
End If
optSur3.Caption = Name
End Sub


Private Sub PpiFa1_Click()
MsgBox "Fixed angle list used for this volume", vbOKOnly
End Sub

Private Sub PpiFa2_Click()
MsgBox "Fixed angle list used for this volume", vbOKOnly
End Sub

Private Sub PpiFa3_Click()
MsgBox "Fixed angle list used for this volume", vbOKOnly
End Sub

Private Sub PpiFa4_Click()
MsgBox "Fixed angle list used for this volume", vbOKOnly
End Sub

Private Sub RhiFa1_Click()
MsgBox "Fixed angle list used for this volume", vbOKOnly
End Sub

Private Sub RhiFa2_Click()
MsgBox "Fixed angle list used for this volume", vbOKOnly
End Sub

Private Sub RhiFa3_Click()
MsgBox "Fixed angle list used for this volume", vbOKOnly
End Sub

Private Sub SurFa1_Click()
MsgBox "Fixed angle list used for this volume", vbOKOnly
End Sub

Private Sub SurFa2_Click()
MsgBox "Fixed angle list used for this volume", vbOKOnly
End Sub

Private Sub SurFa3_Click()
MsgBox "Fixed angle list used for this volume", vbOKOnly
End Sub

Private Sub tmrclk_Timer()
Dim Instring As String * 20
Dim Zangle As Single
Static AzWas As Single
Static ElWas As Single
Static Lastm As Double
Static CCArmed As Boolean


Static AzNow As Single
Static ElNow As Single
Static Flasher As Single
Static WasRunning As Boolean
Dim NowMin, NowSec, ier As Integer
Dim ihr, imin, isec, MinPassed, HourPassed As Integer
Dim CountPerSec, ClockError As Single
Dim et, V, Sec2, Sec1, Secpassed As Double
Dim Counts As Single
Static time1, time2, time3 As String
Static putcount, tryput, noput As Integer
Static lastSec As Integer
Static lastEtime As Integer
Dim semOK As Boolean

'Grab the time now
NowMin = Minute(Now)
NowSec = Second(Now)

If NowSec <> lastSec Then
    txtTime.Text = Now  'write out current time
    lastSec = NowSec
End If

' Grab our PMAC access semaphore - exit if blocked
If ISAPMAC And Not SemTakePMAC(101) Then
    Exit Sub
End If

'Keep the PMAC clock sychronized to the host clock every 30 min
If ((NowMin Mod 20) = 0) Then   '1
    If (NowSec = 0) Then
        tryput = tryput + 1
        'set the clock in the PMAC to the correct minute
        PMACPORT.Output = "P53=" + Format(NowMin) + Chr(13)
        PMACPORT.Output = "P50=99" + Chr(13)
        
        'Determine counts/sec for the PMAC counter and update in the PMAC software clock
        ' The PMAC Counter is at M0, The counts/sec is in P57
        If CCArmed Then
            V = GetPMACMVal(0, ier)
            If ier <> 0 Then
                ier = ier
            End If

            time2 = Now
            If Lastm <> 0 Then
                Sec2 = Hour(time2) * 3600# + Minute(time2) * 60#
                Sec1 = Hour(time1) * 3600# + Minute(time1) * 60#
                Secpassed = Sec2 - Sec1
                If Secpassed < 0 Then
                    Secpassed = Secpassed + 24# * 3600#
                End If
           
                If Secpassed > 2000 Then
                    Secpassed = Secpassed
                End If
           
           
                CountPerSec = (V - Lastm) / Secpassed 'determine counts/sec for the PMAC counter
                If CountPerSec < 0 Then
                    'Here if counter rolled over, 24 Bits
                    CountPerSec = (V - Lastm + 16777216) / Secpassed
                
                End If
 
                Counts = GetPMACPVal(Clockrate, ier)
                If ier <> 0 Then
                    If ier = 2 Then GoTo 100  'bad char
                    ier = ier             'timeout
                End If
            
                ClockError = Counts - CountPerSec
            
                'throw out bad values due to program delays
                If (ClockError > Counts / 4) Or (ClockError < -Counts / 4) Then
                    Counts = Counts
                Else
                    Counts = Counts - ClockError / 10
                    putcount = putcount + 1
                    If Not PutPMACPVal(Clockrate, Counts) Then
                        MsgBox "Unable to Set PMAC Clock Rate"
                    End If
                End If
            
100             Lastm = V
                time1 = Now
                CCArmed = False
            End If
        End If
    Else
        CCArmed = True 'arm on non zero sec prevents multiple samples in 1 sec
    End If
    '-----------------------------------------------------------------------------------
    'Check to see if in Fake angles every 10 sec
    Static FATarmed As Boolean
    Static AntMoving As Boolean

    If (NowSec Mod 10) = 0 Then    '2
        If FATarmed Then     '3
        
            'Note Hobbs time is approximate

            If AntMoving Then
                Hobbs = Hobbs + (0.00277777) * 1.01 'inc hour counter by 1 sec +1% for algorithm design
                TxbHobbs.Text = Format(Val(Hobbs), "0000.0") 'write out hobbs
                AntMoving = False
            End If

            V = GetPMACPVal(RealorFake, ier)
            If ier = 0 Then  '4
                If (V = 1234) Then  'spcial value to turn on fake angles
                    FakeAngles = True
                Else
                    FakeAngles = False
                End If   '5
               
            Else
                'also make sure the PMAC is still on the serial line
                If ISAPMAC Then
                    If (ier = 1) Then
                        MsgBox "PMAC Serial Communications has FAILED"
                    End If
                    If (ier = 2) Then
                        MsgBox "PMAC Communications Corrupted"
                    End If
                End If
            End If '4
        End If  '3
        FATarmed = False
    Else        '2
        FATarmed = True
        'And send out the status message
        If FakeAngles Then
            TxtFake.Text = "Simulated Antenna"
            TxtFake.BackColor = RGB(255, 0, 0)
        Else
            TxtFake.Text = "Real Antenna"
            TxtFake.BackColor = RGB(0, 255, 0)
        End If
        'Find out which scan is running if any
        If GetPMACPVal(Running, ier) = 0 Then
            txtScanStat.Text = "Idle"
        Else
            V = GetPMACPVal(ScanID, ier)
        
            'Print out the name of the running scan. This is useful for sequences.
            Select Case V
              Case Ppi1
                txtScanStat.Text = frmMain.optPpi1.Caption
                AntMoving = True
              Case Ppi2
                txtScanStat.Text = frmMain.optPpi2.Caption
                AntMoving = True
              Case Ppi3
                txtScanStat.Text = frmMain.optPpi3.Caption
                AntMoving = True
              Case Ppi4
                txtScanStat.Text = frmMain.optPpi4.Caption
                AntMoving = True
              Case Rhi1
                txtScanStat.Text = frmMain.optRhi1.Caption
                AntMoving = True
              Case Rhi2
                txtScanStat.Text = frmMain.optRhi2.Caption
                AntMoving = True
              Case Rhi3
                txtScanStat.Text = frmMain.optRhi3.Caption
                AntMoving = True
              Case Sur1
                txtScanStat.Text = frmMain.optSur1.Caption
                AntMoving = True
              Case Sur2
                txtScanStat.Text = frmMain.optSur2.Caption
                AntMoving = True
              Case Sur3
                txtScanStat.Text = frmMain.optSur3.Caption
                AntMoving = True
              Case Sync1
                txtScanStat.Text = "Waiting for Sync"
              Case Sync2
                txtScanStat.Text = "Waiting for Sync"
              Case Sync3
                txtScanStat.Text = "Waiting for Sync"
              Case Sync4
                txtScanStat.Text = "Waiting for Sync"
              Case Sync5
                txtScanStat.Text = "Waiting for Sync"
              Case Sync6
                txtScanStat.Text = "Waiting for Sync"
              Case Sync7
                txtScanStat.Text = "Waiting for Sync"
            End Select
        End If
    End If  '2
End If  '1
'-----------------------------------------------------------------------------------



'Display the elapsd time from the last start command

If (ISRUNNING) Then
    et = Timer - Etime
    If et > (lastEtime + 1) Then
        ihr = Fix(et / 3600)
        imin = Fix(et / 60) - ihr * 60
        isec = Fix(et) - ihr * 3600 - imin * 60
       
        txtEt.Text = Format(ihr, "###0") + ":" + Format(imin, "00") + ":" + Format(isec, "00")
        lastEtime = et
    End If
Else
    txtEt.Text = "             "
End If


'read in azimuth
Zangle = GetPMACPVal(110, ier)
If (Zangle < 0) Then Zangle = 360 + Zangle
AzNow = Zangle
   
txtAzNow.Text = Format(Zangle, "###.00")

'read in Elevation
        
Zangle = GetPMACPVal(139, ier)
If (Zangle < 0) Then Zangle = 360 + Zangle
ElNow = Zangle
   
txtElNow.Text = Format(Zangle, "###.00")
    
SemGivePMAC (101)        ' Make sure we return that Semaphore

End Sub

