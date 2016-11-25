% launch this script before running the tests!!!
% Octave or MATLAB work the same way
clear all;

% data len
N = 204;
npack = 25;
binlen = 6 * 48 * 126 * floor(npack * N * 8 / (126 * 48 * 6));

% input data
x = randi([0 5], binlen, 1);

% lists
Nlist = [1512 3024 6048];
Mlist = [2048 4096 8192];

% AND mask for symbol interleaver
R1mask{1} = [1 0 0 1 0 0 0 0 0 0];
R1mask{2} = [1 0 1 0 0 0 0 0 0 0 0];
R1mask{3} = [1 1 0 0 1 0 1 0 0 0 0 0];

% permutation mask for symbol interleaver
R1perm{1} = [4 3 9 6 2 8 1 5 7 0];
R1perm{2} = [6 3 0 9 4 2 1 8 5 10 7];
R1perm{3} = [7 1 4 2 9 6 8 10 0 3 11 5];

% H sequence for symbol interleaver
Hlist = cell(1, 3);
for ff = 1:length(Nlist), % fft size
    Mmax = Mlist(ff);
    Nmax = Nlist(ff);
    Nr = log2(Mmax);
    R1 = zeros(Mmax, Nr - 1);
    R1(1, :) = zeros(1, Nr - 1);
    R1(2, :) = zeros(1, Nr - 1);
    R1(3, :) = [1 zeros(1, Nr - 2)];
    for ii = 4:Mmax,
        R1(ii, 1:(Nr - 2)) = R1(ii - 1, 2:(Nr - 1));
        R1(ii, Nr - 1) = mod(sum(R1(ii - 1, :) .* R1mask{ff}), 2);
    end;
    r1perm = R1perm{ff};
    R = zeros(size(R1));
    R(:, r1perm + 1) = R1;
    Hall = bi2de(R) + pow2(Nr - 1) .* mod(0:(Mmax - 1), 2)';
    Hlist{ff} = Hall(Hall < Nmax);
end;


for ni = 1:length(Nlist),
        
    % max active carriers
    Nmax = Nlist(ni);
    
    % form OFDM symbols
    data = reshape(x, Nmax, []);
    idata = zeros(size(data));
    
    % apply interleaving on even symbols: q -> H(q)
    idata(Hlist{ni} + 1, 1:2:size(data, 2)) = data(:, 1:2:size(data, 2));  
        
    % apply interleaving on odd symbols: H(q) -> q
    idata(:, 2:2:size(data, 2)) = data(Hlist{ni} + 1, 2:2:size(data, 2)); 
        
    % save to file
    fid = fopen(['output' int2str(Nmax) '.bin'], 'wb');
    fwrite(fid, idata, 'uint8');
    fclose(fid);
        
end;

% save to file
fid = fopen('input.bin', 'wb');
fwrite(fid, data, 'uint8');
fclose(fid);
