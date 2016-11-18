% This script runs the simulations for drawing the performance figures

% (c) 2016 The DVB-TX-IRIS team, University of Perugia
 
clear all;

% common parameters
mode     = '8k';   % mode     can be   '2k',   '4k',  '8k'
cp_ratio = '1/4';  % cp_ratio can be '1/32', '1/16', '1/8', '1/4'
outpower = 100;    % percentage of output power, can be larger than 100
npack    = 555;    % number of packets
channel_type = 'AWGN';   % type of channel, can be 'AWGN', 'P1', 'F1'
ce_method    = 'perf'; % channel estimation method, can be:
% 'perf' for perfect channel knowledge
% 'LI'   for linear interpolation
% 'LIF'  for linear interpolation with time-domain filtering
% 'DFT'  for DFT interpolation with noise reduction
% 'DFTF' for DFT interpolation with noise reduction and filtering
num_errs = 200;  % required number of bit errors (after the Viterbi decoder) for each carrier-to-noise ratio
num_sims = 10;   % required number of simulation cycles for each carrier-to-noise ratio
time     = 5;    % required number of minutes for each carrier-to-noise ratio

% the file name where results are saved 
savename = [mfilename '_' mode '_' cp_ratio(1) cp_ratio(3) '_' channel_type '_' ce_method];

% 4-QAM 1/2
[cndb_4_12, berv_4_12] = dvbt_simulation_function(mode, cp_ratio, 4, '1/2', outpower, npack, channel_type, ce_method, ...
    0, 4.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_4_12', 'berv_4_12');

hnd = figure(20);
clf;
semilogy(cndb_4_12, berv_4_12, 'b-o', 'DisplayName', '4-QAM 1/2');
xlabel('\itC\rm/\itN\rm (dB)');
ylabel('BER');
xlim([0 25]);
ylim([0.00000999 0.1]);
title(['DVB-T ' mode ' ' cp_ratio ' Viterbi Soft ' channel_type ' ' ce_method]);
lh = legend('-DynamicLegend');
grid on;
set(lh, 'FontSize', 6, 'Location', 'northeast');
hold off;
saveas(hnd, savename, 'fig');

% 4-QAM 2/3
[cndb_4_23, berv_4_23] = dvbt_simulation_function(mode, cp_ratio, 4, '2/3', outpower, npack, channel_type, ce_method, ...
    2, 6.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_4_23', 'berv_4_23', '-append');

figure(20);
hold on;
semilogy(cndb_4_23, berv_4_23, 'b-*', 'DisplayName', '4-QAM 2/3');
hold off;
saveas(hnd, savename, 'fig');

% 4-QAM 3/4
[cndb_4_34, berv_4_34] = dvbt_simulation_function(mode, cp_ratio, 4, '3/4', outpower, npack, channel_type, ce_method, ...
    3, 7.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_4_34', 'berv_4_34', '-append');

figure(20);
hold on;
semilogy(cndb_4_34, berv_4_34, 'b-s', 'DisplayName', '4-QAM 3/4');
hold off;
saveas(hnd, savename, 'fig');

% 4-QAM 5/6
[cndb_4_56, berv_4_56] = dvbt_simulation_function(mode, cp_ratio, 4, '5/6', outpower, npack, channel_type, ce_method, ...
    4, 8.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_4_56', 'berv_4_56', '-append');

figure(20);
hold on;
semilogy(cndb_4_56, berv_4_56, 'b-d', 'DisplayName', '4-QAM 5/6');
hold off;
saveas(hnd, savename, 'fig');

% 4-QAM 7/8
[cndb_4_78, berv_4_78] = dvbt_simulation_function(mode, cp_ratio, 4, '7/8', outpower, npack, channel_type, ce_method, ...
    5, 9, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_4_78', 'berv_4_78', '-append');

figure(20);
hold on;
semilogy(cndb_4_78, berv_4_78, 'b-v', 'DisplayName', '4-QAM 7/8');
hold off;
saveas(hnd, savename, 'fig');

% 16-QAM 1/2
[cndb_16_12, berv_16_12] = dvbt_simulation_function(mode, cp_ratio, 16, '1/2', outpower, npack, channel_type, ce_method, ...
    6, 10.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_16_12', 'berv_16_12', '-append');

figure(20);
hold on;
semilogy(cndb_16_12, berv_16_12, 'g-o', 'DisplayName', '16-QAM 1/2');
hold off;
saveas(hnd, savename, 'fig');

% 16-QAM 2/3
[cndb_16_23, berv_16_23] = dvbt_simulation_function(mode, cp_ratio, 16, '2/3', outpower, npack, channel_type, ce_method, ...
    8.5, 12.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_16_23', 'berv_16_23', '-append');

figure(20);
hold on;
semilogy(cndb_16_23, berv_16_23, 'g-*', 'DisplayName', '16-QAM 2/3');
hold off;
saveas(hnd, savename, 'fig');

% 16-QAM 3/4
[cndb_16_34, berv_16_34] = dvbt_simulation_function(mode, cp_ratio, 16, '3/4', outpower, npack, channel_type, ce_method, ...
    9.5, 13.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_16_34', 'berv_16_34', '-append');

figure(20);
hold on;
semilogy(cndb_16_34, berv_16_34, 'g-s', 'DisplayName', '16-QAM 3/4');
hold off;
saveas(hnd, savename, 'fig');

% 16-QAM 5/6
[cndb_16_56, berv_16_56] = dvbt_simulation_function(mode, cp_ratio, 16, '5/6', outpower, npack, channel_type, ce_method, ...
    10.5, 15.0, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_16_56', 'berv_16_56', '-append');

figure(20);
hold on;
semilogy(cndb_16_56, berv_16_56, 'g-d', 'DisplayName', '16-QAM 5/6');
hold off;
saveas(hnd, savename, 'fig');

% 16-QAM 7/8
[cndb_16_78, berv_16_78] = dvbt_simulation_function(mode, cp_ratio, 16, '7/8', outpower, npack, channel_type, ce_method, ...
    11.5, 15.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_16_78', 'berv_16_78', '-append');

figure(20);
hold on;
semilogy(cndb_16_78, berv_16_78, 'g-v', 'DisplayName', '16-QAM 7/8');
hold off;
saveas(hnd, savename, 'fig');

% 64-QAM 1/2
[cndb_64_12, berv_64_12] = dvbt_simulation_function(mode, cp_ratio, 64, '1/2', outpower, npack, channel_type, ce_method, ...
    10.5, 15, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_64_12', 'berv_64_12', '-append');

figure(20);
hold on;
semilogy(cndb_64_12, berv_64_12, 'g-o', 'DisplayName', '64-QAM 1/2');
hold off;
saveas(hnd, savename, 'fig');

% 64-QAM 2/3
[cndb_64_23, berv_64_23] = dvbt_simulation_function(mode, cp_ratio, 64, '2/3', outpower, npack, channel_type, ce_method, ...
    13.5, 18, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_64_23', 'berv_64_23', '-append');

figure(20);
hold on;
semilogy(cndb_64_23, berv_64_23, 'g-*', 'DisplayName', '64-QAM 2/3');
hold off;
saveas(hnd, savename, 'fig');

% 64-QAM 3/4
[cndb_64_34, berv_64_34] = dvbt_simulation_function(mode, cp_ratio, 64, '3/4', outpower, npack, channel_type, ce_method, ...
    15, 19.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_64_34', 'berv_64_34', '-append');

figure(20);
hold on;
semilogy(cndb_64_34, berv_64_34, 'g-s', 'DisplayName', '64-QAM 3/4');
hold off;
saveas(hnd, savename, 'fig');

% 64-QAM 5/6
[cndb_64_56, berv_64_56] = dvbt_simulation_function(mode, cp_ratio, 64, '5/6', outpower, npack, channel_type, ce_method, ...
    15.5, 20.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_64_56', 'berv_64_56', '-append');

figure(20);
hold on;
semilogy(cndb_64_56, berv_64_56, 'g-d', 'DisplayName', '64-QAM 5/6');
hold off;
saveas(hnd, savename, 'fig');

% 64-QAM 7/8
[cndb_64_78, berv_64_78] = dvbt_simulation_function(mode, cp_ratio, 64, '7/8', outpower, npack, channel_type, ce_method, ...
    17, 21.5, 0.5, num_errs, num_sims, time, 50, 0);
save(savename, 'cndb_64_78', 'berv_64_78', '-append');

figure(20);
hold on;
semilogy(cndb_64_78, berv_64_78, 'g-v', 'DisplayName', '64-QAM 7/8');
hold off;
saveas(hnd, savename, 'fig');

