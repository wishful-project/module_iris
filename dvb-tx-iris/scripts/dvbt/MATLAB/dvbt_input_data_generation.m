function datain = dvbt_input_data_generation(npack)
% The function DVBT_INPUT_DATA_GENERATOR randomly generates the
% input data of a DVB-T transmitter. The function is called as
%
% [datain] = dvbt_input_data_generation(npack)
%
% where the input parameter
% npack  is the number of packets to be generated,
%
% while the output parameters
% datain is the generated data (a 188 x npack8 matrix of bytes,
%        expressed by integer numbers from 0 to 255, where
%        npack8 = 8*ceil(npack/8) is the number of generated
%        packets, multiple of 8).

% (c) 2016 The DVB-TX-IRIS team, University of Perugia
 
npack8 = round(npack);   % set npack8 to be integer
npack8 = max(npack8,8);  % if npack8 < 8, set npack8 = 8
if mod(npack8,8) ~= 0,
    npack8 = npack8 + (8 - mod(npack8,8)); % set npack8 a multiple of 8
end
datain          = zeros(188,npack8); % memory allocation
datain(1,:)     = repmat(71,1,npack8); % SYNC is 71
datain(2:188,:) = randi([0 255], 187, npack8); % random data (bytes)

