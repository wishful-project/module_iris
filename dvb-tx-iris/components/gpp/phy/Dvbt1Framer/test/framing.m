% launch this script before running the tests!!!
% Octave or MATLAB work the same way
clear all;

% number of OFDM symbols
nsymb = 5;

% static tables
Nlist = [1512 3024 6048];
Klist = [1705 3409 6817];
Mlist = [2048 4096 8192];

% input data
inlen = nsymb * Nlist(end);
data = randn(inlen, 1) + 1i * randn(inlen, 1);
data = (1:inlen)';

% save to file
fid = fopen('input.bin', 'wb');
fwrite(fid, [real(data) imag(data)].', 'float32');
fclose(fid);      

% reference PRBS
reg = zeros(Klist(end), 11);
reg(1, :) = 1;
for ss = 2:size(reg, 1),
    reg(ss, 2:11) = reg(ss - 1, 1:10);
    reg(ss, 1) = mod(reg(ss - 1, 9) + reg(ss - 1, 11), 2);
end;
w_prbs = reg(:, 11);

% pilots
pil_amplitude = 4/3;
pil_distance = 12;
pil_mod = 3;
cont_pil_list{1}   = [0 48 54 87 141 156 192 ...
                    201 255 279 282 333 432 450 ...
                    483 525 531 618 636 714 759 ...
                    765 780 804 873 888 918 939 ...
                    942 969 984 1050 1101 1107 1110 ...
                    1137 1140 1146 1206 1269 1323 1377 ...
                    1491 1683 1704];

cont_pil_list{2}   = [0 48 54 87 141 156 192 ...
                    201 255 279 282 333 432 450 ...
                    483 525 531 618 636 714 759 ...
                    765 780 804 873 888 918 939 ...
                    942 969 984 1050 1101 1107 1110 ...
                    1137 1140 1146 1206 1269 1323 1377 ...
                    1491 1683 1704 1752 1758 1791 1845 ...
                    1860 1896 1905 1959 1983 1986 2037 ...
                    2136 2154 2187 2229 2235 2322 2340 ...
                    2418 2463 2469 2484 2508 2577 2592 ...
                    2622 2643 2646 2673 2688 2754 2805 ...
                    2811 2814 2841 2844 2850 2910 2973 ...
                    3027 3081 3195 3387 3408];

cont_pil_list{3}   = [0 48 54 87 141 156 192 ...
                    201 255 279 282 333 432 450 ...
                    483 525 531 618 636 714 759 ...
                    765 780 804 873 888 918 939 ...
                    942 969 984 1050 1101 1107 1110 ...
                    1137 1140 1146 1206 1269 1323 1377 ...
                    1491 1683 1704 1752 1758 1791 1845 ...
                    1860 1896 1905 1959 1983 1986 2037 ...
                    2136 2154 2187 2229 2235 2322 2340 ...
                    2418 2463 2469 2484 2508 2577 2592 ...
                    2622 2643 2646 2673 2688 2754 2805 ...
                    2811 2814 2841 2844 2850 2910 2973 ...
                    3027 3081 3195 3387 3408 3456 3462 ...
                    3495 3549 3564 3600 3609 3663 3687 ...
                    3690 3741 3840 3858 3891 3933 3939 ...
                    4026 4044 4122 4167 4173 4188 4212 ...
                    4281 4296 4326 4347 4350 4377 4392 ...
                    4458 4509 4515 4518 4545 4548 4554 ...
                    4614 4677 4731 4785 4899 5091 5112 ...
                    5160 5166 5199 5253 5268 5304 5313 ...
                    5367 5391 5394 5445 5544 5562 5595 ...
                    5637 5643 5730 5748 5826 5871 5877 ...
                    5892 5916 5985 6000 6030 6051 6054 ...
                    6081 6096 6162 6213 6219 6222 6249 ...
                    6252 6258 6318 6381 6435 6489 6603 ...
                    6795 6816];

% TPS pilots
tps_amplitude = 1;
tps_pil_list{1}    = [34 50 209 346 413 569 595 688 ...
                    790 901 1073 1219 1262 1286 1469 1594 ...
                    1687];

tps_pil_list{2}    = [34 50 209 346 413 569 595 688 ...
                    790 901 1073 1219 1262 1286 1469 1594 ...
                    1687 1738 1754 1913 2050 2117 2273 2299 ...
                    2392 2494 2605 2777 2923 2966 2990 3173 ...
                    3298 3391];

tps_pil_list{3}    = [34 50 209 346 413 569 595 688 ...
                    790 901 1073 1219 1262 1286 1469 1594 ...
                    1687 1738 1754 1913 2050 2117 2273 2299 ...
                    2392 2494 2605 2777 2923 2966 2990 3173 ...
                    3298 3391 3442 3458 3617 3754 3821 3977 ...
                    4003 4096 4198 4309 4481 4627 4670 4694 ...
                    4877 5002 5095 5146 5162 5321 5458 5525 ...
                    5681 5707 5800 5902 6013 6185 6331 6374 ...
                    6398 6581 6706 6799];

% BCH code for TPS
gpoly = [1 0 0 0 0 1 1 0 1 1 1 0 1 1 1];

for ni = 1:length(Nlist),

  N = Nlist(ni);
  K = Klist(ni);
  M = Mlist(ni);

  % reshape into symbols
  datain = reshape(data, N, []).';
  ns = size(datain, 1);

  % prepare room for the output
  maps = zeros(ns, K);
  
  % prepare tps
  tps = zeros(68, 1);
  tps(0+1) = 0; % don't care
  tps((1:16)+1) =  [0 0 1 1 0 1 0 1 1 1 1 0 1 1 1 0]; % synchro word 1-3
%  tps((1:16)+1) = [1 1 0 0 1 0 1 0 0 0 0 1 0 0 0 1]; % synchro word 2-4
  tps((17:22)+1) = [0 1 0 1 1 1]; % length indicator
  tps((23:24)+1) = [0 0]; % frame counter
%  tps((25:26)+1) = [0 0]; % QPSK
  tps((25:26)+1) = [0 1]; % 16-QAM
%  tps((25:26)+1) = [1 0]; % 64-QAM
  tps((27:29)+1) = [0 0 0]; % NH
%  tps((27:29)+1) = [0 0 1]; % alpha=1
%  tps((27:29)+1) = [0 1 0]; % alpha=2
%  tps((27:29)+1) = [0 1 1]; % alpha=4
%  tps((30:32)+1) = [0 0 0]; % HP 1/2
%  tps((30:32)+1) = [0 0 1]; % HP 2/3
  tps((30:32)+1) = [0 1 0]; % HP 3/4
%  tps((30:32)+1) = [0 1 1]; % HP 5/6
%  tps((30:32)+1) = [1 0 0]; % HP 7/8
  tps((33:35)+1) = [0 0 0]; % LP 1/2 or NH
%  tps((33:35)+1) = [0 0 1]; % LP 2/3
%  tps((33:35)+1) = [0 1 0]; % LP 3/4
%  tps((33:35)+1) = [0 1 1]; % LP 5/6
%  tps((33:35)+1) = [1 0 0]; % LP 7/8
  tps((36:37)+1) = [0 0]; % 1/32
%  tps((36:37)+1) = [0 1]; % 1/16
%  tps((36:37)+1) = [1 0]; % 1/8
%  tps((36:37)+1) = [1 1]; % 1/4
  if ni == 1,
    tps((38:39)+1) = [0 0]; % 2K
  elseif ni == 2,
    tps((38:39)+1) = [1 0]; % 4K
  else,
    tps((38:39)+1) = [0 1]; % 8K
  end;
  tps((40:47)+1) = [0 0 0 0 0 0 0 0]; % cellid
  tps((48:53)+1) = [0 0 0 0 0 0]; % Annex F

  sc = 0; % symbol counter
  fc = 0; % frame counter
  for l = 0:(ns - 1),
      
     % place the scattered pilots
     pilpos = (0:pil_distance:(K-1)) + mod(sc*pil_mod, pil_distance);
     if pilpos(end) > K - 1,
       pilpos(end) = [];
     end;
     maps(l + 1, pilpos + 1) = pil_amplitude .* 2 .* (0.5 - w_prbs(pilpos + 1)');
     
     % place the continual pilots
     maps(l + 1, cont_pil_list{ni} + 1) = pil_amplitude.* 2 .* (0.5 - w_prbs(cont_pil_list{ni} + 1)');
          
     % update TPS for this frame
     if sc == 0,
     
       if fc == 0 || fc == 2,
         tps((1:16)+1) =  [0 0 1 1 0 1 0 1 1 1 1 0 1 1 1 0]; % synchro word 1-3
       else
         tps((1:16)+1) = [1 1 0 0 1 0 1 0 0 0 0 1 0 0 0 1]; % synchro word 2-4
       end;
       tps((23:24)+1) = de2bi(fc,2,'left-msb'); % frame counter
     
       % find parity
       msg = [zeros(1, 60) tps(2:54)'];
       cw = fliplr(bchenco(fliplr(msg), 127, 113));
       tps((54:67) + 1) = cw(114:127);

       fprintf(1, '%d: ', fc);
       fprintf(1, '%d', tps);
       fprintf(1, '\n');
       
       % absolute modulation
       maps(l + 1, tps_pil_list{ni} + 1) = 2 .* (0.5 - w_prbs(tps_pil_list{ni} + 1)'); % ref
     
     else
     
       % differential modulation
       if tps(sc + 1) == 0,
          maps(l + 1, tps_pil_list{ni} + 1) = maps((l - 1) + 1, tps_pil_list{ni} + 1);
       else,
          maps(l + 1, tps_pil_list{ni} + 1) = -maps((l - 1) + 1, tps_pil_list{ni} + 1);
       end;
       
     end;
          
     % fill remaining cells with data
     maps(l + 1, maps(l + 1, :) == 0) = datain(l + 1, :);
     
     % update
     sc = mod(sc + 1, 68);
     fc = mod(fc + 1, 4);     
     
  end;
  
  % save to file
  dataout = reshape(maps.', [], 1);
  fid = fopen(['output' int2str(M) '.bin'], 'wb');
  fwrite(fid, [real(dataout) imag(dataout)].', 'float32');
  fclose(fid);

end;


