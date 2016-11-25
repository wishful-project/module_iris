% launch this script before running the tests!!!
% Octave or MATLAB work the same way
clear all;

K = 188; % packet size in bytes
G = 8; % size of a group of packets (every G packets the SYNC must be inverted)

npack = 100;

% packets
datain = randi([0 255], K, npack);
datain(1,:) = 71; % 0x47 -- SYNC
bin = reshape(de2bi(datain(:),8,'left-msb')', [], 1);

% simulate PRBS
register = zeros(1, 15);
bout = zeros(size(bin));
binlen = length(bin);
for ii = 0:(binlen-1),
    jj = mod(ii, G * K * 8);
    if jj == 0, % first bit in group
        register = [1 0 0 1 0 1 0 1 0 0 0 0 0 0 0]; % register initialize
    end;
    if jj >= 1 * 8, % after the first byte in group
      tmpxor = xor(register(14), register(15)); % register output
      register = [tmpxor register(1:14)]; % shift register
      if mod(jj, K * 8) < 8,
        tmpxor = 0; % pass SYNC byte
      end;
    else
      tmpxor = 1; % invert SYNC byte
    end;
    bout(ii + 1) = xor(tmpxor, bin(ii + 1)); % xor the input
end;

dataout = reshape(bout, 8, [])';
dataout = bi2de(dataout,'left-msb');

fid = fopen('input.bin', 'wb');
fwrite(fid, datain, 'uint8');
fclose(fid);

fid = fopen('output.bin', 'wb');
fwrite(fid, dataout, 'uint8');
fclose(fid);

