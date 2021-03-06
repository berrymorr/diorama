unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls, bass, Spin, ComCtrls, math;

type
  TForm1 = class(TForm)
    cb_devs: TComboBox;
    od1: TOpenDialog;
    sd1: TSaveDialog;
    Button1: TButton;
    ebox: TPaintBox;
    Button2: TButton;
    draw_timer: TTimer;
    Panel1: TPanel;
    se: TSpinEdit;
    pb1: TProgressBar;
    cb_active: TComboBox;
    Button3: TButton;
    Button4: TButton;
    Button5: TButton;
    Bevel1: TBevel;
    Button6: TButton;
    cp_cb: TComboBox;
    gainbar: TTrackBar;
    Button7: TButton;
    Bevel2: TBevel;
    Bevel3: TBevel;
    Button8: TButton;
    procedure draw_grid();
    procedure draw_cursor();
    procedure draw_sound();
    procedure draw_light();
    procedure draw_inact_light();
    procedure NCLDBLClick(var Msg: TMessage); message WM_NCLBUTTONDBLCLK;
    procedure FormCreate(Sender: TObject);
    procedure cb_devsSelect(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure seChange(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure eboxPaint(Sender: TObject);
    procedure draw_timerTimer(Sender: TObject);
    procedure eboxMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure eboxMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure Button3Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
    procedure pb1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure FormKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure cb_activeCloseUp(Sender: TObject);
    procedure FormMouseWheel(Sender: TObject; Shift: TShiftState;
      WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);
    procedure cb_activeSelect(Sender: TObject);
    procedure cb_inactiveSelect(Sender: TObject);
    procedure cb_activeDropDown(Sender: TObject);
    procedure Button5Click(Sender: TObject);
    procedure Button8Click(Sender: TObject);
    procedure Button6Click(Sender: TObject);
    procedure gainbarChange(Sender: TObject);
    procedure Button7Click(Sender: TObject);
    procedure pb1MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
  private

  public
  end;

var Form1: TForm1;


const frame_head:dword=$CCAA5533; //normal head, reverse byte order 0x3355AACC
const frame_tail:word=$AACC;      //normal tail, reverse byte order CCAA
const eos_head:dword=$FFFFFFFF;   //end of stream (the last) head


type
  TScenarioFrame=packed record
  head:dword;
  sound:array[0..399]of byte;
  light:array[0..63]of byte;
  smoke:array[0..7]of byte;
  pins:byte;//PD3on & 0x01; PB0on & 0x02; PB1off & 0x04
  reserved:array[0..32]of byte;
  tail:word;
  end;

type Tch_arr=record
  ch:byte;
  flat:array of byte;
  bmp:TBitmap;
  cb:TComboBox;
end;



  var bs:cardinal;
  scenario:array of TScenarioFrame;
  ch_arr:array of Tch_arr;
  flat,flat_l:array of byte;
  flat_minimax:array of word;
  s_pos,offset:integer;

  //add pin images here

  var bm,sound_img,light_img:tbitmap;
  var last_point:TPoint;
  var zoom_prev:integer;//������. ��-� ���-����, ��� ��������� ����� 1 � -2
  var czoom:integer;//��������� �� se.Value

implementation


{$R *.dfm}

//������ y �� x ����� �� ������, ��������
//����� ������� �������
function ykxb(x,x1,y1,x2,y2:integer):integer;
var k,b:double;
begin
k:=(y2-y1)/(x2-x1);
b:=(x2*y1-y2*x1)/(x2-x1);
result:=round(k*x+b);
end;


//����� ������� � �������� �� �������� ����������
//������ �������
function minimax_arr(parr:pbyte;len:integer):word;
var q:integer;
ptr:pbyte;
res:word;
begin
ptr:=parr;
q:=0; res:=$00FF;//00 to maxval, FF to minval
while q<len do
  begin
  if ptr^>hi(res) then res:=(ptr^ shl 8)or lo(res);
  if ptr^<lo(res) then res:=(hi(res) shl 8)or ptr^;
  inc(ptr);
  inc(q);
  end;
result:=res;
end;

//����� ������� � �������� �� �������� ����������
//������� ��������� � ����������
function zoom_s_arr(parr:pword;len:integer):word;
var q:integer;
ptr:pword;
res:word;
begin
ptr:=parr;
q:=0; res:=$00FF;//00 to maxval, FF to minval
while q<len do
  begin
  if hi(ptr^)>hi(res) then res:=(hi(ptr^) shl 8)or lo(res);
  if lo(ptr^)<lo(res) then res:=(hi(res) shl 8)or lo(ptr^);
  inc(ptr);
  inc(q);
  end;
result:=res;
end;


//����� �������� �� �������� ���������� ������ �������
function zoom_l_arr(parr:pbyte;len:integer):byte;
var q:integer;
ptr:pbyte;
res:byte;
begin
ptr:=parr;
q:=0; res:=$00;
while q<len do
  begin
  if ptr^>res then res:=ptr^;
  inc(ptr);
  inc(q);
  end;
result:=res;
end;



//���������� �������� ������
procedure draw_l_window(bmp:tbitmap;parr:pbyte;zoom,start_x,max_elem:integer);
var q:integer;
ptr:pbyte;
cmax:byte;
y_scale:double;
begin
if zoom=0 then exit;

y_scale:=bmp.Height/256;
ptr:=parr;

//clear canvas
PatBlt(bmp.Canvas.Handle,start_x,0,bmp.Width-start_x,bmp.Height,WHITENESS);

if zoom>0 then //zoom out
  begin
  //��������� 1-�� ��������
  cmax:=zoom_l_arr(ptr,zoom);
  bmp.Canvas.MoveTo(start_x,bmp.Height-round(cmax*y_scale));
  inc(ptr,zoom);

  for q:=start_x+1 to bmp.Width-1 do
    begin
    if (q+1)*zoom<max_elem then
      begin
      cmax:=zoom_l_arr(ptr,zoom);
      bmp.Canvas.LineTo(q,bmp.Height-round(cmax*y_scale));
      inc(ptr,zoom);
      end

    else//��������� �� �����, ���� ������ �������� ������ �������
      bmp.Canvas.LineTo(q,bmp.Height-1);
    end;
  end
else //zoom in
  begin
  bmp.Canvas.MoveTo(start_x,bmp.Height-round(ptr^*y_scale));//0th elem
  q:=start_x+1;
  inc(ptr);
  while (q*-zoom<bmp.Width-1)and(q<max_elem) do
    begin
    bmp.Canvas.LineTo(q*-zoom,bmp.Height-round(ptr^*y_scale));
    inc(ptr);
    inc(q);
    end;
  end;
end;


procedure draw_s_window(bmp:tbitmap;parr:pword;zoom,max_elem:integer);
var q:integer;
ptr:pword;
cmin,cmax:byte;
y_scale:double;
begin
if zoom=0 then exit;

y_scale:=bmp.Height/256;
ptr:=parr;

//clear canvas
PatBlt(bmp.Canvas.Handle,0,0,bmp.Width,bmp.Height,WHITENESS);

if zoom>0 then //zoom out
  begin
  //��������� 1-�� ��������
  cmin:=lo(zoom_s_arr(ptr,zoom));
  cmax:=hi(zoom_s_arr(ptr,zoom));
  bmp.Canvas.MoveTo(0,bmp.Height-round(cmin*y_scale));
  bmp.Canvas.LineTo(0,bmp.Height-round(cmax*y_scale));
  bmp.Canvas.Pixels[0,bmp.Height-round(cmax*y_scale)]:=bmp.Canvas.Pen.Color;
  inc(ptr,zoom);

  for q:=1 to bmp.Width-1 do
    begin
    if (q+1)*zoom<max_elem then
      begin
      cmin:=lo(zoom_s_arr(ptr,zoom));
      cmax:=hi(zoom_s_arr(ptr,zoom));
      if cmin<>cmax then bmp.Canvas.MoveTo(q,bmp.Height-round(cmin*y_scale));
      bmp.Canvas.LineTo(q,bmp.Height-round(cmax*y_scale));
      bmp.Canvas.Pixels[q,bmp.Height-round(cmax*y_scale)]:=bmp.Canvas.Pen.Color;
      inc(ptr,zoom);
      end

    else//��������� �� �����, ���� ������ �������� ������ �������
      begin
      bmp.Canvas.LineTo(q,bmp.Height div 2);
      bmp.Canvas.Pixels[q,bmp.Height div 2]:=bmp.Canvas.Pen.Color;
      end;
    end;
  end
else //zoom in
  begin
  bmp.Canvas.MoveTo(0,bmp.Height-round(ptr^*y_scale));//0th elem
  q:=1;
  inc(ptr);
  while (q*-zoom<bmp.Width-1)and(q<max_elem) do
    begin
    bmp.Canvas.LineTo(q*-zoom,bmp.Height-round(ptr^*y_scale));
    inc(ptr);
    inc(q);
    end;
  end;
end;

//������ �����
procedure TForm1.draw_grid();
var q:integer;
begin
//����� ��� ���������� �������
bm.Canvas.Pen.Color:=clblue;
bm.Canvas.Brush.Style:=bsclear;
for q:=0 to length(ch_arr)-1 do
bm.Canvas.Rectangle(0,q*64+128+256,bm.Width-1,(q+1)*64+128+256);

//����� ��� ��������� ������
bm.Canvas.Pen.Color:=clgreen;
bm.Canvas.Brush.Style:=bsclear;
bm.Canvas.Rectangle(0,0,bm.Width-1,128);

//����� ��� ��������� ������
bm.Canvas.Pen.Color:=clblue;
bm.Canvas.Brush.Style:=bsclear;
bm.Canvas.Rectangle(0,128,bm.Width-1,384);
end;


//������ �����
procedure TForm1.draw_cursor();
begin
if czoom>0 then begin //zoom out

bm.Canvas.Pen.Color:=clred;
if (s_pos div czoom)-(bm.Width div 2)<0 then
  begin
  //���� ������ ����� �� ���� � ������
  bm.Canvas.MoveTo(s_pos div czoom,0);
  bm.Canvas.LineTo(s_pos div czoom,bm.Height);
  end
  else if (s_pos div czoom)+(bm.Width div 2)>
       (length(scenario) div czoom)-1 then
         begin
         //���� ������ �� ������ � ����� ������
         bm.Canvas.MoveTo(bm.Width +((s_pos-length(scenario)-1)div czoom),0);
         bm.Canvas.LineTo(bm.Width +((s_pos-length(scenario)-1)div czoom),bm.Height);
         end
       else
         begin
         //����� ����������, ���� ��������
         bm.Canvas.MoveTo(bm.Width div 2,0);
         bm.Canvas.LineTo(bm.Width div 2,bm.Height);
         end;

end

else begin //zoom in

if (s_pos * -czoom)-(bm.Width div 2)<0 then
  begin
  //���� ������ ����� �� ���� � ������
  bm.Canvas.MoveTo(s_pos * -czoom,0);
  bm.Canvas.LineTo(s_pos * -czoom,bm.Height);
  end
  else if (s_pos * -czoom)+(bm.Width div 2)>
       (length(scenario) * -czoom)-1 then
         begin
         //���� ������ �� ������ � ����� ������
         bm.Canvas.MoveTo(bm.Width +((s_pos-length(scenario)-1)* -czoom),0);
         bm.Canvas.LineTo(bm.Width +((s_pos-length(scenario)-1)* -czoom),bm.Height);
         end
       else
         begin
         //����� ����������, ���� ��������
         bm.Canvas.MoveTo(bm.Width div 2,0);
         bm.Canvas.LineTo(bm.Width div 2,bm.Height);
         end;

end;
end;

//������ �������� ������� �� �������� ������
procedure TForm1.draw_sound();
begin

//zoom out
if czoom>0 then begin

if (s_pos div czoom)-(bm.Width div 2)<0 then
  //���� ������ ����� �� ���� � ������
  bitblt(bm.Canvas.Handle,0,0,bm.Width,bm.Height,
  sound_img.Canvas.Handle,0,0,SRCCOPY)
else if (s_pos div czoom)+(bm.Width div 2)>(length(scenario) div czoom)-1 then
     bitblt(bm.Canvas.Handle,0,0,bm.Width,bm.Height,
     sound_img.Canvas.Handle,sound_img.Width-bm.Width,0,SRCCOPY)
     else
     //����� ����������, ���� ��������
     bitblt(bm.Canvas.Handle,0,0,bm.Width,bm.Height,
     sound_img.Canvas.Handle,(s_pos div czoom)-(bm.Width div 2),0,SRCCOPY);

end

//zoom in
else begin

if (s_pos * -czoom)-(bm.Width div 2)<0 then
  //���� ������ ����� �� ���� � ������
  bitblt(bm.Canvas.Handle,0,0,bm.Width,bm.Height,
  sound_img.Canvas.Handle,0,0,SRCCOPY)
  else if (s_pos * -czoom)+(bm.Width div 2)>(length(scenario) * -czoom)-1 then
    //���� ������ �� ������ � ����� ������
    bitblt(bm.Canvas.Handle,0,0,bm.Width,bm.Height,
    sound_img.Canvas.Handle,sound_img.Width-bm.Width,0,SRCCOPY)
    else
    //����� ����������, ���� ��������
    bitblt(bm.Canvas.Handle,0,0,bm.Width,bm.Height,
    sound_img.Canvas.Handle,(s_pos * -czoom)-(bm.Width div 2),0,SRCCOPY);

end;

end;



//������ �������� �������� ������� �� �������� ������
procedure TForm1.draw_light();
begin

//zoom out
if czoom>0 then begin

if (s_pos div czoom)-(bm.Width div 2)<0 then
  //���� ������ ����� �� ���� � ������
  bitblt(bm.Canvas.Handle,0,128,bm.Width,bm.Height,
  light_img.Canvas.Handle,0,0,SRCCOPY)
else if (s_pos div czoom)+(bm.Width div 2)>(length(scenario) div czoom)-1 then
     bitblt(bm.Canvas.Handle,0,128,bm.Width,bm.Height,
     light_img.Canvas.Handle,light_img.Width-bm.Width,0,SRCCOPY)
     else
     //����� ����������, ���� ��������
     bitblt(bm.Canvas.Handle,0,128,bm.Width,bm.Height,
     light_img.Canvas.Handle,(s_pos div czoom)-(bm.Width div 2),0,SRCCOPY);

end

//zoom in
else begin

if (s_pos * -czoom)-(bm.Width div 2)<0 then
  //���� ������ ����� �� ���� � ������
  bitblt(bm.Canvas.Handle,0,128,bm.Width,bm.Height,
  light_img.Canvas.Handle,0,0,SRCCOPY)
  else if (s_pos * -czoom)+(bm.Width div 2)>(length(scenario) * -czoom)-1 then
    //���� ������ �� ������ � ����� ������
    bitblt(bm.Canvas.Handle,0,128,bm.Width,bm.Height,
    light_img.Canvas.Handle,light_img.Width-bm.Width,0,SRCCOPY)
    else
    //����� ����������, ���� ��������
    bitblt(bm.Canvas.Handle,0,128,bm.Width,bm.Height,
    light_img.Canvas.Handle,(s_pos * -czoom)-(bm.Width div 2),0,SRCCOPY);

end;

end;


//������ ��� ���������� �������� ������� �� �������� ������
procedure TForm1.draw_inact_light();
var q:integer;
begin
for q:=0 to length(ch_arr)-1 do
//zoom out
if czoom>0 then begin

if (s_pos div czoom)-(bm.Width div 2)<0 then
  //���� ������ ����� �� ���� � ������
  bitblt(bm.Canvas.Handle,0,384+q*64,bm.Width,bm.Height,
  ch_arr[q].bmp.Canvas.Handle,0,0,SRCCOPY)
else if (s_pos div czoom)+(bm.Width div 2)>(length(scenario) div czoom)-1 then
     bitblt(bm.Canvas.Handle,0,384+q*64,bm.Width,bm.Height,
     ch_arr[q].bmp.Canvas.Handle,ch_arr[q].bmp.Width-bm.Width,0,SRCCOPY)
     else
     //����� ����������, ���� ��������
     bitblt(bm.Canvas.Handle,0,384+q*64,bm.Width,bm.Height,
     ch_arr[q].bmp.Canvas.Handle,(s_pos div czoom)-(bm.Width div 2),0,SRCCOPY);

end

//zoom in
else begin

if (s_pos * -czoom)-(bm.Width div 2)<0 then
  //���� ������ ����� �� ���� � ������
  bitblt(bm.Canvas.Handle,0,384+q*64,bm.Width,bm.Height,
  ch_arr[q].bmp.Canvas.Handle,0,0,SRCCOPY)
  else if (s_pos * -czoom)+(bm.Width div 2)>(length(scenario) * -czoom)-1 then
    //���� ������ �� ������ � ����� ������
    bitblt(bm.Canvas.Handle,0,384+q*64,bm.Width,bm.Height,
    ch_arr[q].bmp.Canvas.Handle,ch_arr[q].bmp.Width-bm.Width,0,SRCCOPY)
    else
    //����� ����������, ���� ��������
    bitblt(bm.Canvas.Handle,0,384+q*64,bm.Width,bm.Height,
    ch_arr[q].bmp.Canvas.Handle,(s_pos * -czoom)-(bm.Width div 2),0,SRCCOPY);

end;

end;


//block resize on dblclick on form's header
procedure TForm1.NCLDBLClick(var Msg: TMessage);
begin
//��� �����
if msg.wParam=HTCAPTION then msg.Result:=0;
end;

procedure TForm1.FormCreate(Sender: TObject);
var q:integer;
dev:BASS_DEVICEINFO;
begin
//fill editing light channel combobox
for q:=0 to 63 do cb_active.Items.Add('['+inttostr(q)+']');
cb_active.ItemIndex:=0;

//fill copy-from-combobox
for q:=0 to 63 do cp_cb.Items.Add('['+inttostr(q)+']');
cp_cb.ItemIndex:=1;

last_point.X:=0;
last_point.Y:=0;


q:=0;
while BASS_GetDeviceInfo(q,dev) do
  begin
  cb_devs.AddItem(dev.name,nil);
  inc(q);
  end;

if cb_devs.Items.Count>1 then
  begin
  cb_devs.ItemIndex:=1;
  cb_devsSelect(nil);
  end;
end;

//����� �����������
procedure TForm1.cb_devsSelect(Sender: TObject);
var dev:BASS_DEVICEINFO;
begin
if cb_devs.Items.Count<1 then exit;
BASS_Free();
if not BASS_Init(cb_devs.ItemIndex,48000,BASS_DEVICE_CPSPEAKERS+BASS_DEVICE_LATENCY+BASS_DEVICE_FREQ,application.Handle,nil)
then showmessage('���� �������� ���������');

BASS_GetDeviceInfo(BASS_GetDevice(),dev);
form1.Caption:=dev.name+'@'+dev.driver;

BASS_Start();

end;


//������� ����
procedure TForm1.Button1Click(Sender: TObject);
//load image to memory
var f:file of TScenarioFrame;
b:TScenarioFrame;
corrupted:boolean;
q:integer;
begin
form1.SetFocus;
if not od1.Execute then
  begin
  form1.SetFocus;
  exit;
  end;
form1.SetFocus;

button2.Enabled:=false;
s_pos:=0;

corrupted:=false;
scenario:=nil;//free old scenario
flat:=nil;
flat_minimax:=nil;

assignfile(f,od1.FileName);
{$I-}
reset(f);
{$I+}
if ioresult<>0 then
  begin
  showmessage('can not open file '+od1.FileName);
  exit;
  end;


repeat
{$I-}
read(f,b);
{$I+}
if ioresult<>0 then //we get ioerror before eos_head
  begin
  corrupted:=true;
  break;
  end;

if (b.head=frame_head)and(b.tail=frame_tail) then
//it's regular frame
  begin
  //load it into scenario
  setlength(scenario,length(scenario)+1);
  scenario[length(scenario)-1]:=b;
  //also copy to flat
  setlength(flat,length(flat)+400);
  move(scenario[length(scenario)-1].sound[0],flat[(length(scenario)-1)*400],400);
  //also make /400 array of min & max values
  setlength(flat_minimax,length(flat_minimax)+1);
  flat_minimax[length(flat_minimax)-1]:=minimax_arr(@scenario[length(scenario)-1].sound[0],400);
  end
else
  begin
  if b.head<>eos_head then corrupted:=true else
  //it's da last frame, end of stream
    begin
    setlength(scenario,length(scenario)+1);
    //fill with $FF
    fillchar(scenario[length(scenario)-1],sizeof(scenario[length(scenario)-1]),#255);
    end;
  break;//stop reading image file
  end;
until eof(f);
closefile(f);
if corrupted then form1.Caption:='*corrupted* ['+od1.FileName+']'
else form1.Caption:='['+od1.FileName+']';
form1.Caption:=form1.Caption+#32+inttostr(length(scenario) div 3307)+':'+
inttostr((length(scenario) mod 3307)div 55)+','+
inttostr((length(scenario) mod 3307)mod 55);

if length(scenario)>1 then
  begin
  button2.Enabled:=true;
  pb1.Max:=length(scenario)-1;
  s_pos:=0;offset:=0;

  //��������� flat-������� ��� �����
  cb_activeSelect(cb_active);
  for q:=0 to length(ch_arr)-1 do cb_inactiveSelect(ch_arr[q].cb);

  //��������� ��� �, =>, ��������
  seChange(nil);

  bs:=BASS_StreamCreate(22050,1,BASS_SAMPLE_8BITS,STREAMPROC_PUSH,nil);
  BASS_StreamPutData(bs,@flat[0],length(flat)-1);
  end;
end;

//PLAY/PAUSE
procedure TForm1.Button2Click(Sender: TObject);
begin
if not length(scenario)>1 then exit;

if BASS_ChannelIsActive(bs)<>BASS_ACTIVE_PLAYING then
  begin
  if BASS_ChannelPlay(bs,false) then
    begin
    draw_timer.Enabled:=true;
    button2.Caption:='pause';
    end;
  end

else
  begin
  if BASS_ChannelPause(bs) then
    begin
    draw_timer.Enabled:=false;
    button2.Caption:='play';
    end;
  end;

cb_devs.Enabled:=not draw_timer.Enabled;
end;

procedure TForm1.FormClose(Sender: TObject; var Action: TCloseAction);
begin
BASS_Free();
end;


//��� ��� ���
procedure TForm1.seChange(Sender: TObject);
var q:integer;
begin
if (se.Value=-1)or(se.Value=0) then
if zoom_prev=-2 then se.Value:=1 else se.Value:=-2;
zoom_prev:=se.Value;
czoom:=zoom_prev;

if length(flat_minimax)=0 then exit;

//������������ ������� �� ������
//PatBlt(sound_img.Canvas.Handle,0,0,sound_img.Width,sound_img.Height,WHITENESS);
if czoom>0 then sound_img.Width:=length(flat_minimax) div czoom
else sound_img.Width:=length(flat_minimax) * -czoom;
draw_s_window(sound_img,@flat_minimax[0],czoom,length(flat_minimax)-1);

//������������ ������� � �������� ������
if length(flat_l)>0 then
  begin
  //PatBlt(light_img.Canvas.Handle,0,0,light_img.Width,light_img.Height,WHITENESS);
  if czoom>0 then light_img.Width:=length(flat_l) div czoom
  else light_img.Width:=length(flat_l) * -czoom;
  draw_l_window(light_img,@flat_l[0],czoom,0,length(flat_l)-1);
  end;

//������������ ������� � ���������� ������
for q:=0 to length(ch_arr)-1 do
if length(ch_arr)>0 then
if length(ch_arr[q].flat)>0 then
  begin
  //PatBlt(ch_arr[q].bmp.Canvas.Handle,0,0,ch_arr[q].bmp.Width,ch_arr[q].bmp.Height,WHITENESS);
  if czoom>0 then ch_arr[q].bmp.Width:=length(ch_arr[q].flat) div czoom
  else ch_arr[q].bmp.Width:=length(ch_arr[q].flat) * -czoom;
  draw_l_window(ch_arr[q].bmp,@ch_arr[q].flat[0],czoom,0,length(ch_arr[q].flat)-1);
  end;

//������������ ������
if BASS_ChannelIsActive(bs)<>BASS_ACTIVE_PLAYING then draw_timerTimer(nil);
end;


//������ ��������-������� ��� ��������
procedure TForm1.FormResize(Sender: TObject);
var w,h,q,l,i:integer;
begin
form1.SetFocus;
if not form1.Active then exit;

czoom:=se.Value;

//prevent components borders sticking
w:=form1.ClientWidth-3;
h:=form1.ClientHeight-1;

//bottom panel
panel1.Top:=h-panel1.Height;
panel1.Width:=w-panel1.Left;

//progressbar
pb1.Width:=panel1.Width-4;

//draw box
ebox.ClientWidth:=w-ebox.Left;
ebox.ClientHeight:=h-panel1.Height;

//zoom control
se.Top:=h-se.Height;

//bitmaps init
//sound
sound_img:=tbitmap.Create;
sound_img.PixelFormat:=pf32bit;
sound_img.Height:=128;
sound_img.Canvas.Pen.Color:=clgreen;
//active light
light_img:=tbitmap.Create;
light_img.PixelFormat:=pf32bit;
light_img.Height:=256;
//buffer
bm:=tbitmap.Create;
bm.PixelFormat:=pf32bit;
bm.Width:=ebox.ClientWidth;
bm.Height:=ebox.ClientHeight;


//���������� ������
//����������� ���������� ������� ��������� �������
//�� ������� ��������� � 1 ��������� - �������
//(bm.Height-128(����)-256(��������)) div 64(������ ����������)
for q:=0 to ((bm.Height-128-256) div 64)-1 do
  begin
  l:=length(ch_arr)+1;
  setlength(ch_arr,l);
  ch_arr[l-1].ch:=$FF;//unassigned
  ch_arr[l-1].bmp:=TBitmap.Create;
  ch_arr[l-1].bmp.PixelFormat:=pf32bit;
  ch_arr[l-1].bmp.Height:=64;
  ch_arr[l-1].cb:=TComboBox.Create(form1);
  ch_arr[l-1].cb.Parent:=form1;
  ch_arr[l-1].cb.Width:=169;
  ch_arr[l-1].cb.Left:=0;
  ch_arr[l-1].cb.Top:=q*64+128+256;
  ch_arr[l-1].cb.Style:=csDropDownList;
  ch_arr[l-1].cb.TabStop:=false;
  ch_arr[l-1].cb.Tag:=l-1;
  ch_arr[l-1].cb.OnEnter:=cb_activeCloseUp;
  ch_arr[l-1].cb.OnCloseUp:=cb_activeCloseUp;
  ch_arr[l-1].cb.OnSelect:=cb_inactiveSelect;
  for i:=0 to 63 do ch_arr[l-1].cb.Items.Add('['+inttostr(i)+']');
  ch_arr[l-1].cb.ItemIndex:=l;

  bm.Canvas.Brush.Color:=clblue;
  bm.Canvas.Brush.Style:=bsbdiagonal;
  bm.Canvas.Rectangle(0,q*64+128+256,bm.Width-1,(q+1)*64+128+256);
  end;

//��������� ������
for q:=0 to form1.ControlCount-1 do
if form1.Controls[q].ClassName<>'TComboBox' then
form1.Controls[q].ControlState:=[csFocusing];
end;

procedure TForm1.eboxPaint(Sender: TObject);
begin
draw_timerTimer(nil);
end;

procedure TForm1.draw_timerTimer(Sender: TObject);
begin
//��������� ������� �������
//if BASS_ChannelIsActive(bs)=BASS_ACTIVE_PLAYING then
s_pos:=offset+(BASS_ChannelGetPosition(bs,BASS_POS_BYTE) div 400)+1;
pb1.Position:=s_pos;

form1.Caption:='['+od1.FileName+']'+#32+inttostr(length(scenario) div 3307)+':'+
inttostr((length(scenario) mod 3307)div 55)+','+
inttostr((length(scenario) mod 3307)mod 55);

case BASS_ChannelIsActive(bs) of
BASS_ACTIVE_PLAYING: form1.Caption:=form1.Caption+#32+'PLAY';
BASS_ACTIVE_STOPPED: form1.Caption:=form1.Caption+#32+'STOPPED';
BASS_ACTIVE_PAUSED: form1.Caption:=form1.Caption+#32+'PAUSED';
BASS_ACTIVE_STALLED: form1.Caption:=form1.Caption+#32+'STALLED';
end;

form1.Caption:=form1.Caption+#32+inttostr(s_pos div 3307)+':'+
inttostr((s_pos mod 3307)div 55)+','+
inttostr((s_pos mod 3307)mod 55);


//��������� �����
PatBlt(bm.Canvas.Handle,0,0,bm.Width,bm.Height,WHITENESS);

draw_sound();//����

draw_light();//����

draw_inact_light();//���������� ����

draw_grid();//�����

draw_cursor();//�����

//����� �� ����� ����!
bitblt(ebox.Canvas.Handle,0,0,bm.Width,bm.Height,bm.Canvas.Handle,0,0,SRCCOPY);

//���������� ��? 
//application.ProcessMessages;
end;

procedure TForm1.eboxMouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
var q,screen_offset:integer;
begin
if BASS_ChannelIsActive(bs)=BASS_ACTIVE_PLAYING then exit;
if length(flat_l)<2 then exit;
if not ((ssleft in shift) and //��� ������
        ((y>128)and(y<384)and(x>0)and(x<ebox.Width))//� �������� ������
        ) then exit;


//��������� screen_offset - �������������� ������ ����
//�������� ������, �� �������
if (s_pos div czoom)-(bm.Width div 2)<0
  then screen_offset:=0 //����� ����� ����� � ������
  else if (s_pos div czoom)+(bm.Width div 2)>(length(scenario)div czoom)-1
         then screen_offset:=length(scenario)-1-bm.Width*czoom //����� ����� � �����
         else screen_offset:=s_pos-(bm.Width*czoom)div 2;//����� ����������


//������ ������
//������ ����� � ������ �� �� ���� ����� ������������ ����
for q:=x*czoom-(czoom div 2) to x*czoom-(czoom div 2)+czoom-1 do
flat_l[q+screen_offset]:=384-y;
//�������� ������������ ��� ����������� ������� ���������
for q:=min(x*czoom+screen_offset,last_point.X*czoom+screen_offset)+1
to max(x*czoom+screen_offset,last_point.X*czoom+screen_offset)-1 do
flat_l[q]:=ykxb(q,x*czoom+screen_offset,384-y,
last_point.X*czoom+screen_offset,384-last_point.Y);

//������ ����� ������
//������ ��� �������� ������ ��-�� ������ ����������
//draw_l_window(light_img,@flat_l[screen_offset],czoom,round(screen_offset/czoom),length(flat_l)-1);
//������� �� ���� ���������� �� ������ �������� ���� ������. ��� 10-20 �����.
draw_l_window(light_img,@flat_l[0],czoom,0,length(flat_l)-1);

last_point.X:=x;
last_point.Y:=y;


draw_timerTimer(draw_timer);
//showmessage('from '+inttostr(x*czoom-(czoom div 2))+' to '+inttostr(x*czoom-(czoom div 2)+czoom-1));
{//���� ����� ��������
bm.Canvas.LineTo(x,y);
flat[x]:=384-y;//relative
flat[last_point.X]:=384-last_point.Y;//relative

for q:=min(x,last_point.X)+1 to max(x,last_point.X)-1 do
flat[q]:=ykxb(q,x,384-y,last_point.X,384-last_point.Y);


last_point.X:=x;
last_point.Y:=y;

button4click(nil);}
end;

procedure TForm1.eboxMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
//if (last_point.X=0)and(last_point.Y=0)then


  begin
  last_point.X:=x;
  last_point.Y:=y;
//  bm.Canvas.MoveTo(x,y);
  end;

end;

//STOP
procedure TForm1.Button3Click(Sender: TObject);
begin
BASS_ChannelStop(bs);
draw_timer.Enabled:=false;
cb_devs.Enabled:=not draw_timer.Enabled;
s_pos:=0; offset:=0;
button2.Caption:='play';
BASS_ChannelSetPosition(bs,0,BASS_POS_BYTE);
BASS_StreamPutData(bs,@flat[0],length(flat)-1);
//BASS_ChannelPause(bs);
draw_timerTimer(nil);
end;

//��������!
procedure TForm1.Button4Click(Sender: TObject);
var q:integer;
f:file of TScenarioFrame;
begin
form1.SetFocus;
if not sd1.Execute then exit;
form1.SetFocus;
//�������� ����������������� �������� ������� �� flat_l � ��������
for q:=0 to length(flat_l)-1 do
scenario[q].light[cb_active.ItemIndex]:=flat_l[q];

//������� � ����...
assignfile(f,sd1.FileName);
{$I-}
rewrite(f);
{$I+}
if ioresult<>0 then
  begin
  showmessage('can not open/create file '+sd1.FileName);
  exit;
  end;


for q:=0 to length(scenario)-1 do
  begin
  {$I-}
  write(f,scenario[q]);
  {$I+}
  if ioresult<>0 then //we get ioerror before eos_head
    begin
    showmessage('can not write to file '+sd1.FileName);
    break;
    end;
  end;
closefile(f);
od1.FileName:=sd1.FileName;
end;


//��������� �� ����� �� ������������
procedure TForm1.pb1MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
BASS_ChannelSetPosition(bs,0,BASS_POS_BYTE);
offset:=round((length(scenario)*x)/pb1.ClientWidth);
BASS_StreamPutData(bs,@flat[offset*400],length(flat)-1-offset*400);

if BASS_ChannelIsActive(bs)<>BASS_ACTIVE_PLAYING then draw_timerTimer(nil);
end;

procedure TForm1.FormKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
var step:byte;
begin
form1.SetFocus;
if (ssCtrl in shift) then step:=100 else step:=20;
if czoom>0 then step:=step*czoom else step:=step div -czoom;

case key of
  //��������� �����
  vk_left: begin
  offset:=s_pos-step;
  if offset<0 then offset:=0;
  BASS_ChannelSetPosition(bs,0,BASS_POS_BYTE);
  BASS_StreamPutData(bs,@flat[offset*400],length(flat)-1-offset*400);
  end;

  //��������� ������
  vk_right: begin
  offset:=s_pos+step;
  if offset>length(scenario)-1 then offset:=length(scenario)-2;
  BASS_ChannelSetPosition(bs,0,BASS_POS_BYTE);
  BASS_StreamPutData(bs,@flat[offset*400],length(flat)-2-offset*400);
  end;

  //��������� �� ������
  vk_home: begin
  offset:=0;
  BASS_ChannelSetPosition(bs,0,BASS_POS_BYTE);
  BASS_StreamPutData(bs,@flat[offset*400],length(flat)-1-offset*400);
  end;

  //��������� �� �����
  vk_end: begin
  offset:=length(scenario)-2;
  BASS_ChannelSetPosition(bs,0,BASS_POS_BYTE);
  BASS_StreamPutData(bs,@flat[offset*400],length(flat)-2-offset*400);
  end;

  //zoom-out
  vk_up: begin
  se.Value:=se.Value+1;
  if se.Value>se.MaxValue then se.Value:=se.MaxValue;
  seChange(nil);
  end;

  //zoom-in
  vk_down: begin
  se.Value:=se.Value-1;
  if se.Value<se.MinValue then se.Value:=se.MinValue;
  seChange(nil);
  end;

  //�����-�����
  vk_space: button2click(nil);
  //����������!
  vk_escape: button3click(nil);
  //ctrl+o
  79: if (ssCtrl in shift) then button1click(nil);

end;

key:=0;//��� �����
if BASS_ChannelIsActive(bs)<>BASS_ACTIVE_PLAYING then draw_timerTimer(nil);
end;

//���������� - ��� �����
procedure TForm1.cb_activeCloseUp(Sender: TObject);
begin
button1.SetFocus;
end;

//��� ��������
procedure TForm1.FormMouseWheel(Sender: TObject; Shift: TShiftState;
  WheelDelta: Integer; MousePos: TPoint; var Handled: Boolean);
begin
if (mousepos.X>ebox.Left)and(mousepos.Y>ebox.Top)and
   (mousepos.X<ebox.Left+ebox.Width)and(mousepos.Y<ebox.Top+ebox.Height)then
   begin
   if wheeldelta>0 then se.Value:=se.Value+1
   else se.Value:=se.Value-1;
   if se.Value<se.MinValue then se.Value:=se.MinValue;
   if se.Value>se.MaxValue then se.Value:=se.MaxValue;
   seChange(nil);
   end;
end;

//�������� light_img � ������� �������� ������
procedure TForm1.cb_activeSelect(Sender: TObject);
var q:integer;
begin
if cb_active.ItemIndex<0 then exit;

flat_l:=nil;
setlength(flat_l,length(scenario));
cb_active.Enabled:=false;
for q:=0 to length(scenario)-1 do //fill flat active light
flat_l[q]:=scenario[q].light[cb_active.ItemIndex];
cb_active.Enabled:=true;

seChange(nil);
end;

//����� ���. ������� ��� ���������
procedure TForm1.cb_inactiveSelect(Sender: TObject);
var q:integer;
begin
{showmessage('cb#'+inttostr((sender as tcombobox).Tag)+
' selected ch#'+inttostr((sender as tcombobox).ItemIndex));}

if (sender as tcombobox).ItemIndex<0 then exit;

ch_arr[(sender as tcombobox).Tag].flat:=nil;
setlength(ch_arr[(sender as tcombobox).Tag].flat,length(scenario));
(sender as tcombobox).Enabled:=false;
for q:=0 to length(scenario)-1 do //fill flat light
ch_arr[(sender as tcombobox).Tag].flat[q]:=
scenario[q].light[(sender as tcombobox).ItemIndex];
(sender as tcombobox).Enabled:=true;

seChange(nil);

end;

procedure TForm1.cb_activeDropDown(Sender: TObject);
var q:integer;
begin
if cb_active.ItemIndex<0 then exit;
for q:=0 to length(flat_l)-1 do
scenario[q].light[cb_active.ItemIndex]:=flat_l[q];
end;

//�������� �����
procedure TForm1.Button5Click(Sender: TObject);
var q:integer;
begin
if length(flat_l)<>length(scenario) then exit;

cb_active.Enabled:=false;
for q:=0 to length(scenario)-1 do flat_l[q]:=0;
cb_active.Enabled:=true;

seChange(nil);
end;

procedure TForm1.Button8Click(Sender: TObject);
var q:integer;
begin
if length(flat_l)<>length(scenario) then exit;

cb_active.Enabled:=false;
for q:=0 to length(scenario)-1 do flat_l[q]:=$FF;
cb_active.Enabled:=true;

seChange(nil);
end;

procedure TForm1.Button6Click(Sender: TObject);
var q:integer;
begin
if (length(flat_l)<>length(scenario))or(cp_cb.ItemIndex<0) then exit;

cb_active.Enabled:=false;
cp_cb.Enabled:=false;
for q:=0 to length(scenario)-1 do flat_l[q]:=scenario[q].light[cp_cb.ItemIndex];
cb_active.Enabled:=true;
cp_cb.Enabled:=true;

seChange(nil);
end;

procedure TForm1.gainbarChange(Sender: TObject);
begin
button7.Caption:=inttostr(gainbar.Position)+'%';
end;

procedure TForm1.Button7Click(Sender: TObject);
var q,i,lx:integer;
ly:byte;
begin
if length(flat_l)<>length(scenario) then exit;
if not length(scenario)>2 then exit;

cb_active.Enabled:=false;
for q:=0 to length(scenario)-1 do
if ((flat_l[q]*gainbar.Position)div 100)>=256 then flat_l[q]:=255
else flat_l[q]:=(flat_l[q]*gainbar.Position)div 100;
cb_active.Enabled:=true;

//������������ ��� �����>1
if gainbar.Position>100 then
  begin
  lx:=0; ly:=flat_l[0];
  for q:=1 to length(flat_l)-1 do
  if abs(ly-flat_l[q])>1 then
    begin
    for i:=lx+1 to q-1 do flat_l[i]:=ykxb(i,lx,ly,q,flat_l[q]);
    lx:=q;
    ly:=flat_l[q];
    end;
  end;

gainbar.Position:=100;
button7.Caption:='set';

seChange(nil);
end;

procedure TForm1.pb1MouseMove(Sender: TObject; Shift: TShiftState; X,
  Y: Integer);
begin
if (ssleft in shift) then pb1MouseDown(sender,mbleft,shift,x,y);
end;

end.
