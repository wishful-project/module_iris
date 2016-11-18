% prepare a power loading file
pl=5*sin(linspace(0,12*pi,8192));
pl=pl-mean(pl);fid=fopen('powerload.txt', 'wt');fprintf(fid, '%f\n',pl);fclose(fid);