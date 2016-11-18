% launch this script before running the tests!!!
% Octave or MATLAB work the same way
clear all;

% puncturing matrices
pu_table{1} = [1 1]'; % 1/2
pu_table{2} = [1 1; 0 1]'; % 2/3
pu_table{3} = [1 1; 0 1; 1 0]'; % 3/4
pu_table{4} = [1 1; 0 1; 1 0; 0 1; 1 0]'; % 5/6
pu_table{5} = [1 1; 0 1; 0 1; 0 1; 1 0; 0 1; 1 0]'; % 7/8

% input sizes
npack = 100;
N = 204 * 8 * 2;
datalen = 420 * floor(N * npack / 420);

% generate input data
data = randi([0 1], datalen, 1);

% puncture at all rates
for pin = 1:length(pu_table),
    pw = size(pu_table{pin}, 2); % puncturing period
    tdata = reshape(data, 2, []); % data in puncturing format
    pm = logical(repmat(pu_table{pin}, 1, size(tdata, 2) / pw)); % selection matrix
    pdata = tdata(pm); % punctured data
    
    % save to file
    [n, d] = rat(0.5*numel(pu_table{pin})/sum(pu_table{pin}(:)));
    fid = fopen(['output' int2str(n) int2str(d) '.bin'], 'wb');
    fwrite(fid, pdata, 'uint8');
    fclose(fid);
end;

% save to file
fid = fopen('input.bin', 'wb');
fwrite(fid, data, 'uint8');
fclose(fid);
