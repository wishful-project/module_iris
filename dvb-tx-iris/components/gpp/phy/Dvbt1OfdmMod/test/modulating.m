% launch this script before running the tests!!!
% Octave or MATLAB work the same way
clear all;

% number of OFDM symbols
nsymb = 5;
outpower = 10;;

% static lists
Nlist = [1512 3024 6048];
Tlist = [17 34 68];
Klist = [1705 3409 6817];
Mlist = [2048 4096 8192];
Dlist = [32 16 8 4];

% input data
inlen = nsymb * Klist(end);
data = 1/sqrt(2) .* (randn(inlen, 1) + 1i * randn(inlen, 1));
%data = (0:(inlen-1))';
%data = ones(size(data));

% save to file
fid = fopen('input.bin', 'wb');
fwrite(fid, [real(data) imag(data)].', 'float32');
fclose(fid);      

for mi = 1:length(Mlist),
  for di = 1:length(Dlist),

    N = Nlist(mi);
    T = Tlist(mi);
    K = Klist(mi);
    M = Mlist(mi);
    D = Dlist(di);

    % multiplicative factor
	  power = (1 * (N + T) ... % data + TPS carriers
		+ (16/9) * (K - N - T) ... % pilot carriers
        ) / M; 
    multFactor_ = sqrt((outpower / 100) / (power * M)) / 3;

    % reshape into symbols
    datain = reshape(data(1:(K*floor(inlen/K))), K, []).';
    ns = size(datain, 1);

    % prepare room for the output
    ofdms = zeros(ns, M + M/D);
    
    for l = 1:ns,
    
      fdata = [datain(l, :) zeros(1, M - K)];
      fdata = circshift(fdata.', -(K-1)/2).';
      tdata = ifft(fdata) * (multFactor_ * M);
      ofdms(l, :) = [tdata((end - M/D + 1):end) tdata];
             
    end;
    
    % save to file
    dataout = reshape(ofdms.', [], 1);
    fid = fopen(['output' int2str(M) int2str(D) '.bin'], 'wb');
    fwrite(fid, [real(dataout) imag(dataout)].', 'float32');
    fclose(fid);
  end;
  
end;
