% launch this script before running the tests!!!
% Octave or MATLAB work the same way
clear all;
Kfull = 239;
K = 188;
Nfull = 255;
N = 204;
m = 8;
rs_genpoly = rsgenpoly(Nfull, Kfull, 285, 0); % 0 is as per DVB spec.
npack=100;

% real encode
data = uint8(randi([0 255], K, npack));
data(1,:) = 71; % 0x47 -- SYNC
data(1,1:8:end) = 184; % 0xB8 -- inverted SYNC
msg = gf([zeros(Kfull - K, npack); data]', m); % shorten
cw = rsenc(msg, Nfull, Kfull, rs_genpoly);
cw8 = uint8(cw.x);
outdata = cw8(:, (Kfull - K + 1):end)';

fid = fopen('input.bin', 'wb');
fwrite(fid, data(:), 'uint8');
fclose(fid);

fid = fopen('output.bin', 'wb');
fwrite(fid, outdata(:), 'uint8');
fclose(fid);

