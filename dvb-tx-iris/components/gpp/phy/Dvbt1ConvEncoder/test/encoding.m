% launch this script before running the tests!!!
% Octave or MATLAB work the same way
clear all;

N = 204; % packet length
npack = 100;

% input byte length
inlen = npack * N;

% convolutional code trellis
trellis = poly2trellis(7, [171 133]); % as per DVB spec

% message
uncoded8 = randi([0 255], inlen, 1);

% encode (C version wants byte in input and emits bit at the outpout)
uncoded = de2bi(uncoded8, 8, 'left-msb')';
coded = convenc(uncoded(:), trellis);

% save to file
fid = fopen('input.bin', 'wb');
fwrite(fid, uncoded8, 'uint8');
fclose(fid);

fid = fopen('output.bin', 'wb');
fwrite(fid, coded, 'uint8');
fclose(fid);
