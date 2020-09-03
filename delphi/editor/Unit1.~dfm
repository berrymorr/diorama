object Form1: TForm1
  Left = 188
  Top = 289
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'dioed'
  ClientHeight = 411
  ClientWidth = 801
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poDesktopCenter
  WindowState = wsMaximized
  OnClose = FormClose
  OnCreate = FormCreate
  OnKeyDown = FormKeyDown
  OnMouseWheel = FormMouseWheel
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object ebox: TPaintBox
    Left = 176
    Top = 0
    Width = 619
    Height = 361
    OnMouseDown = eboxMouseDown
    OnMouseMove = eboxMouseMove
    OnPaint = eboxPaint
  end
  object Bevel1: TBevel
    Left = 88
    Top = 152
    Width = 81
    Height = 41
    Shape = bsFrame
    Style = bsRaised
  end
  object Bevel2: TBevel
    Left = 0
    Top = 152
    Width = 89
    Height = 41
    Shape = bsFrame
    Style = bsRaised
  end
  object Bevel3: TBevel
    Left = 0
    Top = 192
    Width = 169
    Height = 41
    Shape = bsFrame
    Style = bsRaised
  end
  object cb_devs: TComboBox
    Left = 0
    Top = 0
    Width = 169
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 0
    TabStop = False
    OnCloseUp = cb_activeCloseUp
    OnEnter = cb_activeCloseUp
    OnSelect = cb_devsSelect
  end
  object Button1: TButton
    Left = 0
    Top = 32
    Width = 81
    Height = 25
    Caption = 'open img'
    TabOrder = 1
    TabStop = False
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 0
    Top = 64
    Width = 33
    Height = 25
    Caption = 'play'
    Enabled = False
    TabOrder = 2
    TabStop = False
    OnClick = Button2Click
  end
  object Panel1: TPanel
    Left = 176
    Top = 368
    Width = 617
    Height = 41
    TabOrder = 3
    object pb1: TProgressBar
      Left = 2
      Top = 8
      Width = 607
      Height = 10
      Min = 0
      Max = 100
      Smooth = True
      TabOrder = 0
      OnMouseDown = pb1MouseDown
      OnMouseMove = pb1MouseMove
    end
  end
  object se: TSpinEdit
    Left = 104
    Top = 387
    Width = 65
    Height = 22
    TabStop = False
    AutoSelect = False
    AutoSize = False
    MaxValue = 50
    MinValue = -10
    TabOrder = 4
    Value = 1
    OnChange = seChange
  end
  object cb_active: TComboBox
    Left = 0
    Top = 128
    Width = 169
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 5
    TabStop = False
    OnCloseUp = cb_activeCloseUp
    OnDropDown = cb_activeDropDown
    OnEnter = cb_activeCloseUp
    OnSelect = cb_activeSelect
  end
  object Button3: TButton
    Left = 32
    Top = 64
    Width = 33
    Height = 25
    Caption = 'stop'
    TabOrder = 6
    TabStop = False
    OnClick = Button3Click
  end
  object Button4: TButton
    Left = 88
    Top = 32
    Width = 83
    Height = 25
    Caption = 'save img'
    TabOrder = 7
    TabStop = False
    OnClick = Button4Click
  end
  object Button5: TButton
    Left = 8
    Top = 160
    Width = 33
    Height = 25
    Caption = '->0<-'
    TabOrder = 8
    TabStop = False
    OnClick = Button5Click
  end
  object Button6: TButton
    Left = 96
    Top = 160
    Width = 33
    Height = 25
    Caption = 'from'
    TabOrder = 9
    TabStop = False
    OnClick = Button6Click
  end
  object cp_cb: TComboBox
    Left = 128
    Top = 160
    Width = 33
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 10
    TabStop = False
  end
  object gainbar: TTrackBar
    Left = 8
    Top = 200
    Width = 121
    Height = 25
    Max = 200
    Orientation = trHorizontal
    PageSize = 10
    Frequency = 10
    Position = 100
    SelEnd = 0
    SelStart = 0
    TabOrder = 11
    TabStop = False
    TickMarks = tmBottomRight
    TickStyle = tsAuto
    OnChange = gainbarChange
  end
  object Button7: TButton
    Left = 128
    Top = 200
    Width = 33
    Height = 25
    Caption = 'set'
    TabOrder = 12
    TabStop = False
    OnClick = Button7Click
  end
  object Button8: TButton
    Left = 48
    Top = 160
    Width = 33
    Height = 25
    Caption = '->FF-<'
    TabOrder = 13
    TabStop = False
    OnClick = Button8Click
  end
  object od1: TOpenDialog
    DefaultExt = '*.img'
    Filter = 'IMG|*.img'
    Options = [ofFileMustExist, ofNoTestFileCreate, ofNoNetworkButton, ofDontAddToRecent, ofForceShowHidden]
    Left = 176
  end
  object sd1: TSaveDialog
    DefaultExt = '*.img'
    Filter = 'IMG|*.img'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofDontAddToRecent, ofForceShowHidden]
    Left = 208
  end
  object draw_timer: TTimer
    Enabled = False
    Interval = 40
    OnTimer = draw_timerTimer
    Left = 240
  end
end
