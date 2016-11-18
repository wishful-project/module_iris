% this M-script loads a logo and generates a poerloading configuration for 
% an OFDM transmistter

% (c) 2016 The DVB-TX-IRIS team, University of Perugia
 
clear all;

% graphics_toolkit ("gnuplot")

% open logo
logo = double(imread("logo.png"))/255;
figure(1);
imshow(logo);

% go to grayscale
logo_gs = mean(logo, 3);
figure(2);
imshow(logo_gs);

% binarize
threshold = 0.9;
logo_bw = zeros(size(logo_gs));
logo_bw(logo_gs < threshold) = 1;
logo_bw(end, :) = 1;
figure(3);
imshow(logo_bw);

% original profile
prof = zeros(1, size(logo_gs, 2));

% scan down the columns and find first "0"
for c = 1:size(logo_bw, 2),
  prof(c) = find(logo_bw(:, c), 1);
end;

% plot the profile
excursus = 6; % dB
prof = max(prof) - prof;
prof = excursus * (prof / max(prof)) - excursus / 2;
figure(4);
plot(prof);

% adjust to carriers
M = 8192;
N = 6817;
profi = [zeros(1, floor((M - N) / 2)) ...
  interp1(linspace(0, 1, size(logo_bw, 2)), prof, linspace(0, 1, N)) ...
  zeros(1, ceil((M - N) / 2))];
figure(5);
plot(profi);

% save profile
fp = fopen("../../../examples/dvbt/logo_profile.txt", "wt");
fprintf(fp, "%f\n", profi);
fclose(fp);


