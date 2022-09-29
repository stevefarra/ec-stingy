% Params
DATA_FILE = 'ecg.txt';
Fs = 256;

table = readtable(DATA_FILE);
t = table.Var1;
v = table.Var2;

hpf1 = tf([1,0],[1,2*pi*8])
hpf2 = tf([1,0,0],[1,32*pi,256*pi.^2])
% lpf = tf(1,[1,-2*pi*20]);
% bpf = tf([640*pi.^2,0],[1,56*pi,640*pi.^2])

h = bodeplot(hpf2);
setoptions(h,'FreqUnits','Hz','PhaseVisible','off');
datacursormode on

% lsim(hpf1,v,t)

% filter([640*pi.^2,0],[1,56*pi,640*pi.^2],v)