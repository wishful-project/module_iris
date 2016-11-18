% launch this script before running the tests!!!
% Octave or MATLAB work the same way
clear all;

% data len
inlen = 64;

% input data
%data = randi([0 5], inlen, 1);
data = (0:(inlen - 1))';

% save to file
fid = fopen('input.bin', 'wb');
fwrite(fid, data, 'uint8');
fclose(fid);
    
% list of modulations
Mlist = [4 16 64];

% Mappings with MSB first
QAM{1} = [ 
    +1+1i; %0
    +1-1i; %1
    -1+1i; %2
    -1-1i  %3
    ] / sqrt(2);

QAM{2} = [ 
    +3+3i; %0
    +3+1i; %1
    +1+3i; %2
    +1+1i; %3
    +3-3i; %4
    +3-1i; %5
    +1-3i; %6
    +1-1i; %7
    -3+3i; %8
    -3+1i; %9
    -1+3i; %10
    -1+1i; %11
    -3-3i; %12
    -3-1i; %13
    -1-3i; %14
    -1-1i  %15
    ] / sqrt(10);

QAM{3} = [ 
    +7+7i; %0
    +7+5i; %1
    +5+7i; %2
    +5+5i; %3
    +7+1i; %4
    +7+3i; %5
    +5+1i; %6
    +5+3i; %7
    +1+7i; %8
    +1+5i; %9
    +3+7i; %10
    +3+5i; %11
    +1+1i; %12
    +1+3i; %13
    +3+1i; %14
    +3+3i; %15
    +7-7i; %16
    +7-5i; %17
    +5-7i; %18
    +5-5i; %19
    +7-1i; %20
    +7-3i; %21
    +5-1i; %22
    +5-3i; %23
    +1-7i; %24
    +1-5i; %25
    +3-7i; %26
    +3-5i; %27
    +1-1i; %28
    +1-3i; %29
    +3-1i; %30
    +3-3i; %31
    -7+7i; %32
    -7+5i; %33
    -5+7i; %34
    -5+5i; %35
    -7+1i; %36
    -7+3i; %37
    -5+1i; %38
    -5+3i; %39
    -1+7i; %40
    -1+5i; %41
    -3+7i; %42
    -3+5i; %43
    -1+1i; %44
    -1+3i; %45
    -3+1i; %46
    -3+3i; %47
    -7-7i; %48
    -7-5i; %49
    -5-7i; %50
    -5-5i; %51
    -7-1i; %52
    -7-3i; %53
    -5-1i; %54
    -5-3i; %55
    -1-7i; %56
    -1-5i; %57
    -3-7i; %58
    -3-5i; %59
    -1-1i; %60
    -1-3i; %61
    -3-1i; %62
    -3-3i; %63
    ] / sqrt(42);

for mi = 1:length(Mlist),
    
    M = Mlist(mi);
    
    maps = QAM{mi}(bitand(data, M - 1) + 1);
    
    % save to file
    fid = fopen(['output' int2str(M) '.bin'], 'wb');
    fwrite(fid, [real(maps) imag(maps)].', 'float32');
    fclose(fid);
    
end;

