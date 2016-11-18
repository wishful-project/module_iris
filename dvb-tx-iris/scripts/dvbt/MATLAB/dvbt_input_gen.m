function datain = dvbt_input_gen(npack,filename)
% The function DVBT_INPUT_GEN generates the input data
% of a DVB-T transmitter, either randomly or by loading
% the data from a file. The function is called as
%
% datain = dvbt_input_gen(npack,filename)
%
% where npack is the number of packets (an integer multiple of 8),
% filename is an optional parameter that contains the name of 
% the file, and datain is the generated data (a 188 x npack
% matrix of bytes, expressed by integer numbers from 0 to 255).

% (c) 2016 The DVB-TX-IRIS team, University of Perugia
 
if or(npack==0,mod(npack,8)~=0)
    error('The number of packets must be a multiple of 8.')
end

if nargin == 1,
    datain          = zeros(188,npack); % memory allocation
    datain(1,:)     = repmat(71,1,npack); % SYNC is 71
    datain(2:188,:) = randi([0 255], 187, npack); % random data (bytes)
elseif nargin == 2,
    fid = fopen(filename,'rb');
    if fid<0,
        error('Invalid name of the file.')
    else
        datain = double(fread(fid,[188,npack],'uint8'));
        fclose(fid);
    end
else
    error('The number of input parameters cannot be greater than two.')
end
