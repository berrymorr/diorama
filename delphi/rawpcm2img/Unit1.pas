unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TForm1 = class(TForm)
    od: TOpenDialog;
    sd: TSaveDialog;
    Button1: TButton;
    Label1: TLabel;
    Label2: TLabel;
    Button2: TButton;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

procedure TForm1.Button1Click(Sender: TObject);
begin
if od.Execute then
        begin
        label1.Caption:=od.FileName;
        button2.Enabled:=true;
        end;
end;

procedure TForm1.Button2Click(Sender: TObject);
var fraw,fimg:file of byte;
buf,z:byte;
q,i,k:integer;
i_up,k_up:boolean;
begin
if sd.Execute then
        begin
        i_up:=true;
        k_up:=true;
        k:=-1;
        i:=0;
        z:=0;
        button1.Enabled:=false;
        label2.Caption:=sd.FileName;
        assignfile(fraw,od.FileName);
        assignfile(fimg,sd.FileName);
        reset(fraw);
        rewrite(fimg);
        while not eof(fraw) do
                begin
                //frame head
                buf:=$33; write(fimg,buf);
                buf:=$55; write(fimg,buf);
                buf:=$AA; write(fimg,buf);
                buf:=$CC; write(fimg,buf);
                //sound
                for q:=0 to 400-1 do
                        begin
                        if not eof(fraw) then read(fraw,buf)
                        else buf:=$80;//set DAC at middle
                        write(fimg,buf);
                        end;
                //light
                write(fimg,buf);//write DAC to 0 channel
                //write
                if (k=256) then k_up:=false;
                if (k=-1) then k_up:=true;
                if k_up then inc(k) else dec(k);
                write(fimg,k);
                //write othr 61
                if (i>220) then i_up:=false;
                if (i=0) then i_up:=true;
                if i_up then inc(i) else dec(i);
                for q:=1 to 59 do
                        begin
                        buf:=q*i;
                        write(fimg,buf);
                        end;
                //fixed
                buf:=132;
                write(fimg,buf);
                //fixed
                buf:=$55;
                write(fimg,buf);
                //slow byte
                write(fimg,z);
                inc(z);
                //smoke
                for q:=1 to 8 do
                        begin
                        buf:=q;
                        write(fimg,buf);
                        end;
                //reserved
                buf:=$FF;
                for q:=0 to 34-1 do
                        begin
                        write(fimg,buf);
                        end;
                //frame tail
                buf:=$CC; write(fimg,buf);
                buf:=$AA; write(fimg,buf);
                end;
        //write sequence terminator
        //whole of sector - some raw writers don't
        //write last particular sector
        buf:=$FF; for q:=0 to 512-1 do write(fimg,buf);
        closefile(fraw);
        closefile(fimg);
        button1.Enabled:=true;
        button2.Enabled:=false;
        label2.Caption:='completed.';
        end;
end;

end.
