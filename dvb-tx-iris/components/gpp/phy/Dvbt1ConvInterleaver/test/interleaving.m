% launch this script before running the tests!!!
% Octave or MATLAB work the same way
clear all;

N = 204; % packet length
npack = 100;

% interleaver parameters
I = 12;
M = N / I;

% input data
data = randi([0 255], N, npack);
data(1,:) = 71; % SYNC
data(1,1:8:end) = 184; % inverted SYNC

% interleave
tdata = reshape(data, I, []);
numI = size(tdata, 2);
idata = zeros(size(tdata));
for ii = 0:(I - 1),
    idata(ii + 1, (0:(numI - 1)) + ii * M + 1) = tdata(ii + 1, :);
end;
idata = idata(:, 1:numI);

% save to file
fid = fopen('input.bin', 'wb');
fwrite(fid, data, 'uint8');
fclose(fid);

fid = fopen('output.bin', 'wb');
fwrite(fid, idata, 'uint8');
fclose(fid);
