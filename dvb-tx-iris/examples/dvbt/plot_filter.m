% load and display an impulse response
clear all;

% parameters
coename = 'currentcoe.txt';
fs = 12.5e6;
fd = 64e6/7;
N = 6817;
M = 8192;
fftsize = 2048;
SB = 4e6;
ATT = 25;

% load impulse response
h = load(coename);
N = length(h);
t = (0:(N-1))/fs;

figure(1);
clf;
plot(t, h);

% frequency response
[H, F1] = freqz(h, 1, fftsize, 'whole', fs);
F = F1 - fs/2;

figure(2);
clf;
plot(F, 20*log10(abs(fftshift(H))));
set(gca, 'xlim', [-5e6 5e6]);
grid on;
hold on;
plot([F(1) -SB], [-ATT -ATT], 'r');
plot([SB F(end)], [-ATT -ATT], 'r');
hold off;

figure(3);
clf;
plot(0:(fftsize-1), 20*log10(abs(fftshift(H))));
set(gca, 'xlim', [0 (fftsize-1)]);
set(gca, 'ylim', [-100 0]);

