object Form1: TForm1
  Left = 403
  Top = 502
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsToolWindow
  Caption = 'rawpcm2img'
  ClientHeight = 129
  ClientWidth = 392
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 0
    Top = 0
    Width = 385
    Height = 33
    AutoSize = False
    Caption = 'select raw pcm'
    WordWrap = True
  end
  object Label2: TLabel
    Left = 0
    Top = 64
    Width = 385
    Height = 33
    AutoSize = False
    Caption = 'select output img'
    WordWrap = True
  end
  object Button1: TButton
    Left = 0
    Top = 40
    Width = 75
    Height = 25
    Caption = 'open raw pcm'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 0
    Top = 104
    Width = 75
    Height = 25
    Caption = 'save img'
    Enabled = False
    TabOrder = 1
    OnClick = Button2Click
  end
  object od: TOpenDialog
    Filter = 'RAW|*.raw'
    Options = [ofFileMustExist, ofNoTestFileCreate, ofNoNetworkButton, ofDontAddToRecent, ofForceShowHidden]
    Left = 80
  end
  object sd: TSaveDialog
    DefaultExt = '*.img'
    Filter = 'IMG|*.img'
    Options = [ofOverwritePrompt, ofPathMustExist, ofDontAddToRecent, ofForceShowHidden]
    Left = 112
  end
end
