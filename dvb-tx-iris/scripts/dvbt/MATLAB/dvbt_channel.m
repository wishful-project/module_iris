function [received_data, h, hf] = dvbt_channel(transmitted_data, mode, fs, channel_type, CNdB)
% The function DVBT_CHANNEL modifies the DVB-T transmitted
% signal using a multipath channel and white Gaussian noise.

% The function is called as
%
%   received_data = dvbt_channel(transmitted_data, mode, fs, channel_type, CNdB)
%
% where the input parameters
% transmitted_data is the DVB-T transmitted signal,
% mode             is the DVB-T mode (equal to '2k', '4k', or '8k'),
% fs               is the sample rate,
% channel_type     is the type of channel (equal to 'AWGN', 'P1', or 'F1'),
% CNdB             is the carrier-to-noise ratio (in dB),
%
% while the output parameters
% received_data    is the DVB-T received signal,
% h                is the channel impulse response,
% hf               is the channel frequency response (on the active carriers).

% (c) 2016 The DVB-TX-IRIS team, University of Perugia
 
% check mode
if strcmp(mode,'2k'),
    Mmax = 2048; Kmax = 1705;
elseif strcmp(mode,'4k'),
    Mmax = 4096; Kmax = 3409;
elseif strcmp(mode,'8k'),
    Mmax = 8192; Kmax = 6817;
else
    error('Invalid DVB-T mode.')
end
active_carrier_index = [(Mmax-((Kmax-3)/2)):Mmax, 1:((Kmax+1)/2)].'; % indexes of the active carriers

% multipath channel
if or(strcmp(channel_type,'P1'),strcmp(channel_type,'F1')),
    h = zeros(Mmax,1); % memory allocation
    delay = [1.003019  5.422091  0.518650  2.751772  0.602895 ...
             1.016585  0.143556  0.153832  3.324866  1.935570 ...
             0.429948  3.228872  0.848831  0.073883  0.203952 ...
             0.194207  0.924450  1.381320  0.640512  1.368671].';
    amplitude = [0.057662*exp(-(1i)*4.855121)  0.176809*exp(-(1i)*3.419109) ...
                 0.407163*exp(-(1i)*5.864470)  0.303585*exp(-(1i)*2.215894) ...
                 0.258782*exp(-(1i)*3.758058)  0.061831*exp(-(1i)*5.430202) ...
                 0.150340*exp(-(1i)*3.952093)  0.051534*exp(-(1i)*1.093586) ...
                 0.185074*exp(-(1i)*5.775198)  0.400967*exp(-(1i)*0.154459) ...
                 0.295723*exp(-(1i)*5.928383)  0.350825*exp(-(1i)*3.053023) ...
                 0.262909*exp(-(1i)*0.628578)  0.225894*exp(-(1i)*2.128544) ...
                 0.170996*exp(-(1i)*1.099463)  0.149723*exp(-(1i)*3.462951) ...
                 0.240140*exp(-(1i)*3.664773)  0.116587*exp(-(1i)*2.833799) ...
                 0.221155*exp(-(1i)*3.334290)  0.259730*exp(-(1i)*0.393889)].';
    for tap_index = 1:length(delay),
        p = ceil(delay(tap_index)*fs); h(p) = h(p) + amplitude(tap_index);
    end   
    if strcmp(channel_type,'F1'),
        h(1) = sqrt(10*sum(abs(h(2:end)).^2)); % add LOS component with Rice factor K = 10 dB 
    end
    h  = h(find(h,1,'first'):find(h,1,'last')); % trim head and tail
    h  = h.*((-1).^(0:(length(h)-1))'); % modulate to center at 32/7
    h  = h ./ sqrt(sum(abs(h).^2)); % normalize impulse response
    hf = fft(h,Mmax); % frequency-domain channel
    hf = hf(active_carrier_index); % remove null (virtual) carriers
elseif strcmp(channel_type,'AWGN'),
    h  = 1;
    hf = ones(length(active_carrier_index),1);
else
    error('Invalid channel type.')
end
multipath_data = conv(transmitted_data,h);

% add noise
P_avg          = (sum(abs(multipath_data).^2))/(length(transmitted_data)); % estimate the signal power
sigma2         = 10^(-CNdB/10) * P_avg * (Mmax/Kmax); % noise variance, includes noise bandwidth correction
sigma          = sqrt(sigma2/2); % standard deviation of each I/Q component
noise          = sigma * (randn(size(multipath_data)) + (1i)*randn(size(multipath_data))); % AWGN generation
received_data  = multipath_data + noise; % noise addition
     
