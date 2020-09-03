unit Unit2;

interface

uses
  Classes,windows,mmsystem,dialogs,messages;

type
  mythread = class(TThread)
  private

  protected
    procedure Execute; override;
  end;

implementation

uses Unit1;

{ Important: Methods and properties of objects in VCL or CLX can only be used
  in a method called using Synchronize, for example,

      Synchronize(UpdateCaption);

  and UpdateCaption could look like,

    procedure mythread.UpdateCaption;
    begin
      Form1.Caption := 'Updated in a thread';
    end; }

{ mythread }

procedure mythread.Execute;
begin
while not terminated do begin

sleep(10);

end;
end;


end.
