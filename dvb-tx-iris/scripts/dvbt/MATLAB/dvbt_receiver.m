function [dataout, rs_dec_output_data, rs_dec_input_data, viterbi_output_data, ...
          viterbi_input_data, punctured_soft_data, bitinterleaved_llr_data, ...
          llr_data, noisy_symbols, fd_data, est_chan] ...
          = dvbt_receiver(received_data, mode, cp_ratio, M, fec, outpower, ce_method, hf)
% The function DVBT_RECEIVER decodes the OFDM symbols sent by a DVB-T transmitter.
% The function is called as
%
% [dataout, scrambled_data, rsencoded_data, convinterleaved_data,...
%  convencoded_data, punctured_data, bitinterleaved_data,...
%  llr_data, noisy_symbols, fd_data] ...
%  = dvbt_receiver(ofdm_data, mode, cp_ratio, M, fec, outpower, ce_type, hf)
%
% where the input parameters
% received_data is the vector of received data,
% mode          is the DVB-T mode (equal to '2k', '4k', or '8k'),
% cp_ratio      is the cyclic prefix ratio (equal to '1/32', '1/16', '1/8', or '1/4'),
% M             is the QAM size (equal to 4, 16, or 64),
% fec           is the FEC ratio (equal to '1/2', '2/3', '3/4', '5/6', or '7/8'),
% outpower      is the percentage of output power (can be larger than 100),
% ce_method     is the channel estimation method, which can be equal to:
%               'perf' for perfect channel knowledge,
%               'LI'   for linear interpolation,
%               'LIF'  for linear interpolation with time-domain filtering,
%               'DFT'  for DFT interpolation with noise reduction,
%               'DFTF' for DFT interpolation with noise reduction and filtering,
% hf            is the channel frequency response (optional parameter used only
%               in case of perfect channel knowledge),
%
% while the output parameters
% dataout                 are the output data (a matrix of bytes with 188 rows),
% rs_dec_output_data      are the Reed-Solomon decoded data bytes,
% rs_dec_input_data       are the convolutionally deinterleaved data bytes,
% viterbi_output_data     are the convolutionally decoded data bytes,
% viterbi_input_data      are the LLR's produced by the depuncturer,
% punctured_soft_data     are the LLR's produced by the bit deinterleaver,
% bitinterleaved_llr_data are the LLR's produced by the symbol deinterleaver,
% llr_data                are the LLR's produced by the soft-output demapper,
% noisy_symbols           are the noisy received QAM symbols,
% fd_data                 are the estimated frequency-domain active carrier data cells,
% est_chan                is the frequency-domain estimated channel matrix.

% (c) 2016 The DVB-TX-IRIS team, University of Perugia
 
% number of total carriers, data carriers, active carriers, pilot carriers, TPS carriers, and interleaving parameters
if strcmp(mode,'2k'),
    Mmax = 2048; Nmax = 1512; Kmax = 1705; Pmax =  45; Tmax = 17; R1mask = [1 0 0 1 0 0 0 0 0 0]; r1perm = [4 3 9 6 2 8 1 5 7 0];
elseif strcmp(mode,'4k'),
    Mmax = 4096; Nmax = 3024; Kmax = 3409; Pmax =  89; Tmax = 34; R1mask = [1 0 1 0 0 0 0 0 0 0 0]; r1perm = [6 3 0 9 4 2 1 8 5 10 7];
elseif strcmp(mode,'8k'),
    Mmax = 8192; Nmax = 6048; Kmax = 6817; Pmax = 177; Tmax = 68; R1mask = [1 1 0 0 1 0 1 0 0 0 0 0]; r1perm = [7 1 4 2 9 6 8 10 0 3 11 5];
else
    error('Invalid DVB-T mode.')
end

% cyclic prefix of OFDM
if strcmp(cp_ratio,'1/32'),
    Dmax = 32;
elseif strcmp(cp_ratio,'1/16'),
    Dmax = 16;
elseif strcmp(cp_ratio,'1/8'),
    Dmax =  8;
elseif strcmp(cp_ratio,'1/4'),
    Dmax = 4;
else
    error('Invalid cyclic prefix.')
end

% demultiplexing indexes
if M == 4,
    dmx = [1 2];
elseif M == 16,
    dmx = [1 3 2 4];
elseif M == 64,
    dmx = [1 4 2 5 3 6];
else
    error('Invalid QAM size.')
end

% puncturing table
if strcmp(fec,'1/2'),
    pu_table = [1 1]';
elseif strcmp(fec,'2/3'),
    pu_table = [1 1; 0 1]';
elseif strcmp(fec,'3/4'),
    pu_table = [1 1; 0 1; 1 0]';
elseif strcmp(fec,'5/6'),
    pu_table = [1 1; 0 1; 1 0; 0 1; 1 0]';
elseif strcmp(fec,'7/8'),
    pu_table = [1 1; 0 1; 0 1; 0 1; 1 0; 0 1; 1 0]';
else
    error('Invalid FEC type.')
end

% truncate the input data to an integer number of OFDM symbols
nrows = size(received_data,1);
ex_len = mod(nrows,Mmax+Mmax/Dmax); % excess length
received_data = received_data(1:(nrows-ex_len),1);

% -------------------------
% REMOVE CP AND PERFORM FFT
% -------------------------
f = 3; % map -3sigma...+3sigma into -1...+1
multFactor_ = Mmax * sqrt(outpower/(50 * f^2 * (1 * (Nmax + Tmax) + (16/9) * (Kmax - Nmax - Tmax))));
ofdms = reshape(received_data, Mmax + Mmax/Dmax, []).';
ns = size(ofdms, 1); % number of OFDM symbols
fd_data = zeros(ns, Kmax); % memory allocation
for l = 1:ns,
    tdata = ofdms(l, (Mmax/Dmax + 1):end); % remove cyclic prefix
    fdata = fft(tdata) / multFactor_; % FFT
    fdata = circshift(fdata.', (Kmax-1)/2).'; % carrier reordering
    fd_data(l, :) = fdata(1:Kmax); % remove null (virtual) carriers
end;
% fd_data is an ns x Kmax matrix of frequency-domain symbols

% -------------------------
% PILOT AND DATA SEPARATION
% -------------------------
cont_pil_list = [   0   48   54   87  141  156  192  201  255  279  282  333  432  450  483 ...
    525  531  618  636  714  759  765  780  804  873  888  918  939  942  969  984 ...
    1050 1101 1107 1110 1137 1140 1146 1206 1269 1323 1377 1491 ...
    1683 1704 1752 1758 1791 1845 1860 1896 1905 1959 1983 1986 ...
    2037 2136 2154 2187 2229 2235 2322 2340 2418 2463 2469 2484 ...
    2508 2577 2592 2622 2643 2646 2673 2688 2754 2805 2811 2814 2841 2844 2850 2910 2973 ...
    3027 3081 3195 3387 3408 3456 3462 3495 ...
    3549 3564 3600 3609 3663 3687 3690 3741 3840 3858 3891 3933 3939 ...
    4026 4044 4122 4167 4173 4188 4212 4281 4296 4326 4347 4350 4377 4392 4458 ...
    4509 4515 4518 4545 4548 4554 4614 4677 4731 4785 4899 ...
    5091 5112 5160 5166 5199 5253 5268 5304 5313 5367 5391 5394 5445 ...
    5544 5562 5595 5637 5643 5730 5748 5826 5871 5877 5892 5916 5985 ...
    6000 6030 6051 6054 6081 6096 6162 6213 6219 6222 6249 6252 6258 6318 6381 6435 6489 ...
    6603 6795 6816]; % location of continual pilots
tps_pil_list  = [  34   50  209  346  413  569  595  688  790  901 ...
    1073 1219 1262 1286 1469 1594 1687 1738 1754 1913 ...
    2050 2117 2273 2299 2392 2494 2605 2777 2923 2966 2990 ...
    3173 3298 3391 3442 3458 3617 3754 3821 3977 ...
    4003 4096 4198 4309 4481 4627 4670 4694 4877 ...
    5002 5095 5146 5162 5321 5458 5525 5681 5707 5800 5902 ...
    6013 6185 6331 6374 6398 6581 6706 6799]; % location of TPS carriers
cont_pil_list = cont_pil_list(1:Pmax); % set pilot locations
tps_pil_list  =  tps_pil_list(1:Tmax); % set TPS locations
reg = zeros(Kmax, 11); % reference PRBS
reg(1, :) = 1;
for ss = 2:size(reg, 1),
    reg(ss, 2:11) = reg(ss - 1, 1:10);
    reg(ss, 1) = mod(reg(ss - 1, 9) + reg(ss - 1, 11), 2);
end;
w_prbs = reg(:, 11);
pil_amplitude = 4/3; pil_distance = 12; pil_mod = 3; % pilot parameters
chan = NaN(size(fd_data)); % initialize channel estimate
noisy_symbols = zeros(Nmax, ns); % memory allocation
sc = 0; % symbol counter
fc = 0; % frame counter
for l = 0:(ns - 1),
    pilpos = (0:pil_distance:(Kmax-1)) + mod(sc*pil_mod, pil_distance); % location of the scattered pilots
    if pilpos(end) > Kmax - 1,
        pilpos(end) = [];
    end;
    chan(l + 1, pilpos + 1) = fd_data(l + 1, pilpos + 1) ./ (pil_amplitude .* 2 .* (0.5 - w_prbs(pilpos + 1)')); % channel coefficients on the scattered pilots
    chan(l + 1, cont_pil_list + 1) = fd_data(l + 1, cont_pil_list + 1) ./ (pil_amplitude .* 2 .* (0.5 - w_prbs(cont_pil_list + 1)')); % channel coefficients on the continual pilots
    chan(l + 1, tps_pil_list + 1) = Inf;   % set to Inf the channel estimate at the TPS locations
    noisy_symbols(:, l + 1) = fd_data(l + 1, isnan(chan(l + 1, :))).'; % extract the data by removing pilot and TPS
    sc = mod(sc + 1, 68); % update symbol counter
    fc = mod(fc + 1,  4); % update  frame counter
end;
% noisy_symbols is an Nmax x ns matrix of noisy QAM symbols

% ------------------
% CHANNEL ESTIMATION
% ------------------
if strcmp(ce_method,'perf'),
    est_chan = repmat(hf.',ns,1); % set perfect channel estimation
else
    % frequency-domain interpolation
    chani = zeros(size(chan)); % memory allocation
    if or(strcmp(ce_method,'LI'),strcmp(ce_method,'LIF')), % linear interpolation
        for l = 1:ns,
            chanpos = find(isfinite(chan(l,:))); % location of the pilot carriers
            chani(l,:) = interp1(chanpos-1, chan(l,chanpos), 0:(Kmax-1)); % interpolate along frequency
        end;
    elseif or(strcmp(ce_method,'DFT'),strcmp(ce_method,'DFTF')), % DFT interpolation
            active_carrier_index = [(Mmax-((Kmax-3)/2)):Mmax, 1:((Kmax+1)/2)].'; % indexes of the active carriers
            cir_len  = 50; % length of the channel impulse response (set 50 for P1 or F1 with standard sample rate)
            chanpos1 = isfinite(chan(1,:)); % location of the pilot carriers of the first OFDM block
            chanpos2 = isfinite(chan(2,:)); % location of the pilot carriers of the second OFDM block
            chanpos3 = isfinite(chan(3,:)); % location of the pilot carriers of the third OFDM block
            chanpos4 = isfinite(chan(4,:)); % location of the pilot carriers of the fourth OFDM block
            F_big    = fft(eye(Mmax)); % FFT matrix
            F_reord  = F_big(active_carrier_index,1:cir_len); % reordered and shortened FFT matrix  
            F1_red   = F_reord(chanpos1,1:cir_len); % reduced FFT matrix for the first OFDM block
            F2_red   = F_reord(chanpos2,1:cir_len); % reduced FFT matrix for the second OFDM block
            F3_red   = F_reord(chanpos3,1:cir_len); % reduced FFT matrix for the third OFDM block
            F4_red   = F_reord(chanpos4,1:cir_len); % reduced FFT matrix for the fourth OFDM block
            ce_op    = zeros(Kmax,Kmax-Nmax-Tmax,4); % memory allocation
            ce_op(1:Kmax,1:(Kmax-Nmax-Tmax),1) = F_reord*pinv(F1_red); % channel estimation matrix for the first OFDM block
            ce_op(1:Kmax,1:(Kmax-Nmax-Tmax),2) = F_reord*pinv(F2_red); % channel estimation matrix for the second OFDM block
            ce_op(1:Kmax,1:(Kmax-Nmax-Tmax),3) = F_reord*pinv(F3_red); % channel estimation matrix for the third OFDM block
            ce_op(1:Kmax,1:(Kmax-Nmax-Tmax),4) = F_reord*pinv(F4_red); % channel estimation matrix for the fourth OFDM block
            for l = 1:ns,
                ind4 = mod(l,4)+4*(mod(l,4)==0); % counter index of scattered pilots
                chanpos = isfinite(chan(l,:)); % location of the pilot carriers
                chani(l,:) = chan(l,chanpos)*((squeeze(ce_op(1:Kmax,1:(Kmax-Nmax-Tmax),ind4))).'); % interpolation
            end;
    else
        error('Invalid channel estimation method.');
    end
    % time-domain filtering
    if or(strcmp(ce_method,'LIF'),strcmp(ce_method,'DFTF')), % time-domain filtering
        est_chan = zeros(size(chani)); % memory allocation
        Nfilt = 5; % number of OFDM symbols for time-domain filtering
        for l = 1:ns,
            lm = max(l-Nfilt+1,1);
            est_chan(l,1:Kmax) = mean(chani(lm:l,1:Kmax),1); % average over the last Nfilt OFDM symbols
        end;
    elseif or(strcmp(ce_method,'LI'),strcmp(ce_method,'DFT')), % no time-domain filtering
        est_chan = chani;
    else
        error('Invalid channel estimation method.');
    end
end

% --------------------
% CHANNEL EQUALIZATION
% --------------------
datapos = isnan(chan); % location of the data carriers
for l = 1:ns,
    noisy_symbols(:, l) = noisy_symbols(:, l) ./ est_chan(l,datapos(l,:)).';
end;

% --------------
% SOFT DEMAPPING
% --------------
D = zeros([size(noisy_symbols) log2(M)]); % memory allocation
switch M,
    case 4,
        D(:,:,1) = real(noisy_symbols*sqrt(2)); 
        D(:,:,2) = imag(noisy_symbols*sqrt(2)); 
    case 16,
        dd = zeros(size(noisy_symbols));
        dd(:) = interp1([-100 -2 +2 +100], [-198 -2 +2 +198], real(noisy_symbols(:)*sqrt(10)), 'linear', 'extrap'); % LLR function for the first bit
        D(:,:,1) = dd;
        dd(:) = interp1([-100 -2 +2 +100], [-198 -2 +2 +198], imag(noisy_symbols(:)*sqrt(10)), 'linear', 'extrap'); % LLR function for the second bit
        D(:,:,2) = dd;
        dd(:) = interp1([-100 0 +100], [+98 -2 +98], real(noisy_symbols(:)*sqrt(10)), 'linear', 'extrap'); % LLR function for the third bit
        D(:,:,3) = dd;
        dd(:) = interp1([-100 0 +100], [+98 -2 +98], imag(noisy_symbols(:)*sqrt(10)), 'linear', 'extrap'); % LLR function for the fourth bit
        D(:,:,4) = dd;
    case 64,
        dd = zeros(size(noisy_symbols));
        dd(:) = interp1([-100 -6 -4 -2 +2 +4 +6 +100], [-388 -12 -6 -2 +2 +6 +12 +388], real(noisy_symbols(:)*sqrt(42)), 'linear', 'extrap'); % LLR function for the first bit
        D(:,:,1) = dd;
        dd(:) = interp1([-100 -6 -4 -2 +2 +4 +6 +100], [-388 -12 -6 -2 +2 +6 +12 +388], imag(noisy_symbols(:)*sqrt(42)), 'linear', 'extrap'); % LLR function for the second bit
        D(:,:,2) = dd;
        dd(:) = interp1([-100 -6 -2 0 +2 +6 +100], [+190 +2 -2 -6 -2 +2 +190], real(noisy_symbols(:)*sqrt(42)), 'linear', 'extrap'); % LLR function for the third bit
        D(:,:,3) = dd;
        dd(:) = interp1([-100 -6 -2 0 +2 +6 +100], [+190 +2 -2 -6 -2 +2 +190], imag(noisy_symbols(:)*sqrt(42)), 'linear', 'extrap'); % LLR function for the fourth bit
        D(:,:,4) = dd;
        dd(:) = interp1([-100 -4 0 +4 +100], [+94 -2 +2 -2 +94], real(noisy_symbols(:)*sqrt(42)), 'linear', 'extrap'); % LLR function for the fifth bit
        D(:,:,5) = dd;
        dd(:) = interp1([-100 -4 0 +4 +100], [+94 -2 +2 -2 +94], imag(noisy_symbols(:)*sqrt(42)), 'linear', 'extrap'); % LLR function for the sixth bit
        D(:,:,6) = dd;
end;
llr_data = zeros(size(D)); % memory allocation
for l = 1:ns,
    llr_data(:, l, :) = D(:, l, :) .* abs(repmat(est_chan(l,datapos(l,:)).', [1 1 log2(M)])).^2;
end;
% llr_data is an Nmax x ns x log2(M) array of LLRs

% ---------------------
% SYMBOL DEINTERLEAVING
% ---------------------
Nr = log2(Mmax);
R1 = zeros(Mmax, Nr - 1);
R1(1, :) = zeros(1, Nr - 1);
R1(2, :) = zeros(1, Nr - 1);
R1(3, :) = [1 zeros(1, Nr - 2)];
for ii = 4:Mmax,
    R1(ii, 1:(Nr - 2)) = R1(ii - 1, 2:(Nr - 1));
    R1(ii, Nr - 1) = mod(sum(R1(ii - 1, :) .* R1mask), 2);
end;
R = zeros(size(R1));
R(:, r1perm + 1) = R1;
Hall = bi2de(R) + pow2(Nr - 1) .* mod(0:(Mmax - 1), 2)';
Hlist = Hall(Hall < Nmax);
data = zeros(size(llr_data)); % memory allocation
data(:, 1:2:size(data, 2), :) = llr_data(Hlist + 1, 1:2:size(data, 2), :); % apply interleaving on even symbols: q -> H(q)
data(Hlist + 1, 2:2:size(data, 2), :) = llr_data(:, 2:2:size(data, 2), :); % apply interleaving on odd symbols: H(q) -> q
bitinterleaved_llr_data = reshape(data, [], log2(M)); % Nmax*ns x log2(M) soft bits 

% ------------------
% BIT DEINTERLEAVING
% ------------------
Isize = 126; % number of bits for each RAM of the bit interleaver 
woff  = [0 63 105 42 21 84]; % column offsets of the six RAMs
a = bitinterleaved_llr_data';
b = zeros(size(a)); % memory allocation
w = 0:(size(b, 2) - 1); % column index
for e = 1:log2(M),
    wi = Isize .* floor(w / Isize) + mod(w + woff(e), Isize); % interleaved column index
    b(e,wi+1) = a(e,:); % deinterleave bits
end;
xr(dmx,:) = b; % demultiplexing
punctured_soft_data = reshape(xr, [], 1); % Nmax*ns*log2(M) x 1 soft bits

% ------------
% DEPUNCTURING
% ------------
pw = size(pu_table, 2); % puncturing period
tdata = zeros(2, pw*floor(length(punctured_soft_data)/sum(pu_table(:)))); % add zeros for unknown LLR of bits discarded by the puncturer
pm = logical(repmat(pu_table, 1, size(tdata,2)/pw )); % selection matrix
tdata(pm) = punctured_soft_data;
viterbi_input_data = tdata(:); % 2*fec*Nmax*ns*log2(M) x 1 soft bits

% ----------------
% VITERBI DECODING
% ----------------
clen       = 7; % constraint length of the convolutional encoder
ce_genpoly = [171 133]; % generator polynomial of the convolutional encoder
tblen      = 128; % traceback length
trellis    = poly2trellis(clen, ce_genpoly); % convolutional code trellis
decoded    = vitdec(viterbi_input_data, trellis, tblen, 'trunc', 'unquant'); % apply soft-input hard-output Viterbi decoder
viterbi_output_data = bi2de(reshape(decoded, 8, [])', 'left-msb'); % fec*Nmax*ns*log2(M)/8 x 1 bytes

% ----------------------------
% CONVOLUTIONAL DEINTERLEAVING
% ----------------------------
I_ci = 12; % number of delay paths of the interleaver
M_ci = 17; % number of bytes in each memory cell of each delay path
N   = 204; % number of bytes at the input of the shortened RS decoder
numI = floor(length(viterbi_output_data)/I_ci);
numII = numI - M_ci*(I_ci - 1);
idata = reshape(viterbi_output_data(1:(I_ci*numI)),I_ci,numI); 
tdata = zeros(I_ci, numII);
for ii = 0:(I_ci-1),
    tdata(ii+1,1:numII) = idata(ii+1,(0:(numII-1))+ii*M_ci+1); % useful data
end;
tlen = N*floor(numel(tdata)/N);
rs_dec_input_data = reshape(tdata(1:tlen),N,[]); % N x floor(12*(floor(fec*Nmax*ns*log2(M)/96)-187)/N) bytes

% -----------
% RS DECODING
% -----------
Kfull = 239; % number of bytes at the input of the RS encoder, including 51 zeros
Nfull = 255; % number of bytes at the output of the RS encoder, including 51 zeros
K     = 188; % number of bytes in each decoded packet
rs_genpoly = rsgenpoly(Nfull,Kfull,285,0); % primitive polynomial is 285
npack = size(rs_dec_input_data, 2); % npack = floor(12*(floor(fec*Nmax*ns*log2(M)/96)-187)/N)
cw  = gf([zeros(npack, Nfull - N) rs_dec_input_data.'], 8);
msg = rsdec(cw, Nfull, Kfull, rs_genpoly);
msg = double(msg.x);
rs_dec_output_data = msg(:,(Kfull-K+1):end).'; % K x npack bytes

% ------------
% DESCRAMBLING
% ------------
grouplen = K*8*8;
ngroups  = floor(npack/8);
bout = de2bi(rs_dec_output_data(:), 8, 'left-msb')';
bout = reshape(bout(1:(grouplen*ngroups)), grouplen, ngroups);
bin = zeros(grouplen,ngroups);
bin(1:8,:) = bout(1:8,1:ngroups); % pass inverted SYNC byte
register = [1 0 0 1 0 1 0 1 0 0 0 0 0 0 0]; % register load
for ii = 8:(grouplen-1),
    tmpxor = xor(register(14),register(15)); % register output
    register = [tmpxor,register(1:14)]; % shift register
    if mod(ii,K*8) < 8,
        tmpxor = 0; % pass SYNC byte
    end;
    bin(ii+1,:) = xor(tmpxor*ones(1,ngroups),bout(ii+1,:)); % xor
end;
bin = bi2de(reshape(bin, 8, [])', 'left-msb');
dataout = reshape(bin, K, []);
dataout(1,1:8:end) = 71; % direct SYNC is 71
% dataout is a K x npack matrix of bytes

