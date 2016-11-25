function [CNdBlist, berlist_vit, berlist_rs, toterrs_vit, toterrs_rs, totbits_vit, totbits_rs] ...
    = dvbt_simulation_function(mode, cp_ratio, M, fec, outpower, npack, channel_type, ce_method, ...
    CNdBmin, CNdBmax, CNdBstep, num_errs, num_sims, time, plotfig, savesim)
% The function DVBT_EXAMPLE performs a simulation of
% transmission and reception of a DVB-T signal over the
% channel.
%
% Number of packets per superframe for 2k:
%
% Code rate   QPSK   16-QAM   64-QAM
%    1/2       252     504      756
%    2/3       336     672     1008
%    3/4       378     756     1134
%    5/6       420     840     1260
%    7/8       441     882     1323
%
% Number of packets per superframe for 8k:
%
% Code rate   QPSK   16-QAM   64-QAM
%    1/2      1008    2016     3024
%    2/3      1344    2688     4032
%    3/4      1512    3024     4536
%    5/6      1680    3360     5040
%    7/8      1764    3528     5292
%
% The parameters can be changed according to the comments below.
%
% DVB-T parameters
% mode     can be   '2k',   '4k',  '8k'
% cp_ratio can be '1/32', '1/16', '1/8', '1/4'
% M        can be      4,     16,    64
% fec      can be  '1/2',  '2/3', '3/4', '5/6', '7/8'
% outpower is the percentage of output power, can be larger than 100
% npack    is the number of packets
%
% channel parameters
% channel_type is the type of channel, can be 'AWGN', 'P1', 'F1'
% ce_method is the channel estimation method, can be:
% 'perf' for perfect channel knowledge
% 'LI'   for linear interpolation
% 'LIF'  for linear interpolation with time-domain filtering
% 'DFT'  for DFT interpolation with noise reduction
% 'DFTF' for DFT interpolation with noise reduction and filtering
%
% simulation parameters
% CNdBmin is the minimum carrier-to-noise ratio in dB
% CNdBmax is the maximum carrier-to-noise ratio in dB
% CNdBstep is the step of carrier-to-noise ratio in dB
% num_errs is the required number of bit errors (after the Viterbi decoder) for each carrier-to-noise ratio
% num_sims is the required number of simulation cycles for each carrier-to-noise ratio
% time is the required number of minutes for each carrier-to-noise ratio
% plotfig plots a BER figure (0 is off, else is the figure number)
% savesim saves simulation data to a file (true or false)


% (c) 2016 The DVB-TX-IRIS team, University of Perugia
 
% ----------------
% SIMULATION CYCLE
% ----------------

CNdBlist = CNdBmin:CNdBstep:CNdBmax;  % vector of carrier-to-noise ratios in dB
berlist_rs  = nan(size(CNdBlist)); % memory allocation for the BER after the RS decoder
berlist_vit = nan(size(CNdBlist)); % memory allocation for the BER after the Viterbi decoder

for CNdB = CNdBlist,
    
    totbits_rs  = 0; % initialization of the number of bits after the RS decoder
    totbits_vit = 0; % initialization of the number of bits after the Viterbi decoder
    toterrs_rs  = 0; % initialization of the number of bit errors after the RS decoder
    toterrs_vit = 0; % initialization of the number of bit errors after the Viterbi decoder
    nsims       = 0; % initialization of the number of simulation cycles
    tic;             % start of the time counter
    
    while and( or(toterrs_vit < num_errs , nsims < num_sims) , toc < time*60),
        
        % --------------
        % GENERATE INPUT
        % --------------
        datain = dvbt_input_data_generation(npack); % generation of the input data
        npack8 = size(datain,2); % number of generated packets (is a multiple of 8)
        
        % ---------------------
        % GENERATE DVB-T SIGNAL
        % ---------------------
        [ofdm_data, ~, ~, ~, ~, ~, ~, convinterleaved_data, ~, scrambled_data] ...
            = dvbt_signal_generation(datain, mode, cp_ratio, M, fec, outpower); % generation of the DVB-T signal
        % This function returns also the intermediate data produced within the transmission chain.
        % Anyway, in the rest of this script, only the following output variables are used:
        % - ofdm_data (this variable contains the transmitted signal)
        % - convinterleaved_data (this variable contains the bits before the convolutional encoder)
        
        % -------------
        % DVB-T CHANNEL
        % -------------
        fs                     = 64/7;   % basic sample rate, in MHz
        [received_data, ~, hf] = dvbt_channel(ofdm_data, mode, fs, channel_type, CNdB); % add multipath and AWGN
        
        % --------------------
        % RECEIVE DVB-T SIGNAL
        % --------------------
        [dataout, ~, ~, convinterleaved_data_out, ~, ~, ~, ~, ~, ~, hf_est] ...
            = dvbt_receiver(received_data, mode, cp_ratio, M, fec, outpower, ce_method, hf); % reception of the DVB-T signal
        % This function returns also the intermediate data produced within the receiver chain.
        % Anyway, in the rest of this script, only the following output variables are used:
        % - dataout (this variable contains the estimate of the original data bytes)
        % - convinterleaved_data_out (this variable contains the estimate of the data bits before the convolutional encoder)
        % Note that the input variable hf is exploited only in case of perfect channel estimation (ce_method = 'perf')
        
        % --------------
        % BER ESTIMATION
        % --------------
        mindatalen_rs  = min(numel(datain), numel(dataout)); % set the number of compared bytes of the transport stream
        mindatalen_vit = min(length(convinterleaved_data), length(convinterleaved_data_out)); % set the number of compared bytes at the output of the Viterbi decoder
        [numerr_rs,  ber_rs ] = biterr(datain(1:mindatalen_rs), dataout(1:mindatalen_rs),8); % bit errors on the transport stream
        [numerr_vit, ber_vit] = biterr(convinterleaved_data(1:mindatalen_vit), convinterleaved_data_out(1:mindatalen_vit),8); % bit errors at the output of the Viterbi decoder
        toterrs_rs  = toterrs_rs  + numerr_rs;  % error accumulation (transport stream)
        toterrs_vit = toterrs_vit + numerr_vit; % error accumulation (output of the Viterbi decoder)
        totbits_rs  = totbits_rs  + mindatalen_rs*8;  % number of bits (transport stream)
        totbits_vit = totbits_vit + mindatalen_vit*8; % number of bits (output of the Viterbi decoder)
        totber_rs  = toterrs_rs  / totbits_rs;  % BER update (transport stream)
        totber_vit = toterrs_vit / totbits_vit; % BER update (output of the Viterbi decoder)
        nsims = nsims + 1; % update the number of simulation cycles
        
        % display BER results
        disp(['#' int2str(nsims) ': C/N = ' num2str(CNdB, '%.1f') ', BER(VIT) = ' num2str(totber_vit, '%.1e') ...
            ' (' int2str(toterrs_vit) '/' int2str(totbits_vit) ')' ...
            ', BER(RS) = ' num2str(totber_rs, '%.1e') ' (' int2str(toterrs_rs) '/' int2str(totbits_rs) ')']);
        
        % record BER results values
        berlist_rs(CNdBlist == CNdB)  = totber_rs;
        berlist_vit(CNdBlist == CNdB) = totber_vit;
        
        % plot the actual BER
        if plotfig > 0,
            hnd = figure(plotfig);
            clf;
            semilogy(CNdBlist, berlist_vit, 'r+-', CNdBlist, berlist_rs, 'bo-');
            grid on;
            xlabel('C/N (dB)');
            ylabel('Bit Error Rate (BER)');
            legend('Viterbi', 'RS');
            title([int2str(M) '-QAM ' fec ' ' mode ' ' cp_ratio ' Viterbi Soft']);
        end;
        
        % save results to file and to a figure
        if savesim,
            savename = [mfilename '_' mode '_' cp_ratio(1) cp_ratio(3) '_' int2str(M) '_' fec(1) fec(3) ...
                '_' channel_type '_' ce_method];
            save(savename, 'CNdBlist', 'berlist_rs', 'toterrs_rs', 'totbits_rs', ...
                'berlist_vit', 'toterrs_vit', 'totbits_vit');
            if plotfig > 0,
                saveas(hnd, savename, 'fig');
            end;
        end;
        
    end;
    
end;
