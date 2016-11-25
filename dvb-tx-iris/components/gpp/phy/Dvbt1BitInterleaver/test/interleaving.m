% launch this script before running the tests!!!
% Octave or MATLAB work the same way
clear all;

% data len
N = 204;
npack = 100;
Isize = 126; % don't modify
binlen = 12 * 126 * floor(npack * N * 8 / (126 * 12));

% input data
x = randi([0 1], binlen, 1);

% lists
hlist = {'nh'};
Mlist = [4 16 64];
dmxlist{1} = [1 2];
dmxlist{2} = [1 3 2 4];
dmxlist{3} = [1 4 2 5 3 6];

for hi = 1:length(hlist),
    for mi = 1:length(Mlist),
        
        % skip hyerarchical QPSK
        if mi == 1 && hi == 2,
            continue;
        end;
        
        % modulation
        M = Mlist(mi);
        
        % demultiplexing addresses
        dmx = dmxlist{mi};
        
        % reshape for demultiplexing
        xr = reshape(x, log2(M), []);
        
        % demultiplex
        b = xr(dmx, :);
        
        % column index
        w = 0:(size(b, 2) - 1);
        
        % column offsets
        woff = [0 63 105 42 21 84];
        
        % interleave bits
        a = zeros(size(b));
        for e = 1:log2(M),
            % interleaved column index
            wi = Isize .* floor(w / Isize) + mod(w + woff(e), Isize);
            a(e,:) = b(e,wi+1);
        end;
        
        % form symbol
        y = bi2de(a', 'left-msb');
        
        % save to file
        fid = fopen(['output' int2str(M) hlist{hi} '.bin'], 'wb');
        fwrite(fid, y, 'uint8');
        fclose(fid);
        
    end;
end;

% save to file
fid = fopen('input.bin', 'wb');
fwrite(fid, x, 'uint8');
fclose(fid);
