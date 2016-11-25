rem **********************************************************************************
rem Batch script used for generation of the WiSHFUL typical HD TS mux @ 22.394118 Mbps
rem **********************************************************************************

rem (c) 2016 The DVB-TX-IRIS team, University of Perugia

rem use preset ultrafast for preview and placebo for production

rem Set a proven or the last ffmpeg version
rem set FFMPEG=..\..\testlocal\ffmpeg-20130611-git-0f88a98-win64-static\bin\ffmpeg
set FFMPEG=ffmpeg

rem Use proven opencaster tools
set OCP=..\..\msw\Debug

rem prepare billboards
%FFMPEG% -y -loop 1 -i "cartello.png" -i "silenzio.mp3" -t 185.83 -pix_fmt yuv420p -r 24 -vf "drawtext=fontfile=DejaVuSansMono-Bold.ttf:text='NEXT SHOW BEGINS IN %%{eif\:187-t\:d} SECONDS':fontcolor=#223344:fontsize=60:x=450:y=900" -preset placebo -c:v libx264 -c:a copy cartello1.mp4
%FFMPEG% -y -loop 1 -i "cartello.png" -i "silenzio.mp3" -t 31.97 -pix_fmt yuv420p -r 24 -vf "drawtext=fontfile=DejaVuSansMono-Bold.ttf:text='NEXT SHOW BEGINS IN %%{eif\:33-t\:d} SECONDS':fontcolor=#223344:fontsize=60:x=450:y=900" -preset placebo -c:v libx264 -c:a copy cartello2.mp4
%FFMPEG% -y -loop 1 -i "cartello.png" -i "silenzio.mp3" -t 323.54 -pix_fmt yuv420p -r 24 -vf "drawtext=fontfile=DejaVuSansMono-Bold.ttf:text='NEXT SHOW BEGINS IN %%{eif\:325-t\:d} SECONDS':fontcolor=#223344:fontsize=60:x=450:y=900" -preset placebo -c:v libx264 -c:a copy cartello3.mp4

rem Create video bitstream with impressed logo

%FFMPEG% -y -fflags "+genpts" -i "ToS-4k-1920.mov" -i cartello1.mp4 -i "wishful1hd.png" -r 24 -filter_complex "[0:v] pad=width=1920:height=1080:x=0:y=140:color=black [padded], [padded] [1:v:0] concat=n=2:v=1:a=0 [mv], [2:v] scale = -1:85 [ovrl], [mv][ovrl] overlay = (main_w - overlay_w - 66):46, setdar = 16/9, setpts = 0.96 * PTS, colormatrix = bt709:bt601 [vv]" -map "[vv]" -r 25 -an -f yuv4mpegpipe - | %OCP%\x264 --preset placebo --tune psnr --stdin y4m --level 4.0 --nal-hrd cbr --vbv-bufsize 2000 --bitrate 6700 --vbv-maxrate 6700 --keyint 12 --vbv-init 0 -o video1.pes -

%FFMPEG% -y -fflags "+genpts" -i "Sintel.2010.1080p.ogg" -i cartello2.mp4 -i "wishful2hd.png" -r 24 -filter_complex "[0:v] pad=width=1920:height=1080:x=0:y=131:color=black [padded], [padded] [1:v:0] concat=n=2:v=1:a=0 [mv], [2:v] scale = -1:85 [ovrl], [mv][ovrl] overlay = (main_w - overlay_w - 66):46, setdar = 16/9, setpts = 0.96 * PTS, colormatrix = bt709:bt601 [vv]" -map "[vv]" -r 25 -an -f yuv4mpegpipe - | %OCP%\x264 --preset placebo --tune psnr --stdin y4m --level 4.0 --nal-hrd cbr --vbv-bufsize 2000 --bitrate 6700 --vbv-maxrate 6700 --keyint 12 --vbv-init 0 -o video2.pes -

%FFMPEG% -y -fflags "+genpts" -i "big_buck_bunny_1080p_h264.mov" -i cartello3.mp4 -i "wishful3hd.png" -r 24 -filter_complex "[0:v:0] [1:v:0] concat=n=2:v=1:a=0 [mv],[2:v] scale = -1:85 [ovrl], [mv][ovrl] overlay = (main_w - overlay_w - 66):46, setdar = 16/9, setpts = 0.96 * PTS, colormatrix = bt709:bt601 [vv]" -map "[vv]" -r 25 -an -f yuv4mpegpipe - | %OCP%\x264 --preset placebo --tune psnr --stdin y4m --level 4.0 --nal-hrd cbr --vbv-bufsize 2000 --bitrate 6700 --vbv-maxrate 6700 --keyint 12 --vbv-init 0 -o video3.pes -

rem Create video TS
%OCP%\pesvideo2ts 2064 25:38 b6700000 7100000 0 video1.pes > video1.ts
%OCP%\pesvideo2ts 2074 25:38 b6700000 7100000 0 video2.pes > video2.ts
%OCP%\pesvideo2ts 2084 25:38 b6700000 7100000 0 video3.pes > video3.ts

rem Create audio bitstream

%FFMPEG% -y -i "ToS-4k-1920.mov" -i cartello1.mp4 -r 24 -filter_complex "[0:a:0] [1:a:0] concat=n=2:v=0:a=1, atempo=1.0417, volume=0.5 [ma]" -map "[ma]" -r 25 -ac 2 -vn -acodec mp2 -f mp2 -b:a 128000 -ar 48000 audio1.mp2

%FFMPEG% -y -i "Sintel.2010.1080p.ogg" -i cartello2.mp4 -r 24 -filter_complex "[0:a:0] [1:a:0] concat=n=2:v=0:a=1, atempo=1.0417 [ma]" -map "[ma]" -r 25 -ac 2 -vn -acodec mp2 -f mp2 -b:a 128000 -ar 48000 audio2.mp2

%FFMPEG% -y -i "big_buck_bunny_1080p_h264.mov" -i cartello3.mp4 -r 24 -filter_complex "[0:a:0] [1:a:0] concat=n=2:v=0:a=1, atempo=1.0417 [ma]" -map "[ma]" -r 25 -ac 2 -vn -acodec mp2 -f mp2 -b:a 128000 -ar 48000 audio3.mp2

rem %OCP%\esaudioinfo audio1.mp2

rem Create audio program elementary stream
%OCP%\esaudio2pes audio1.mp2 1152 48000 384 -1 3600 > audio1.pes
%OCP%\esaudio2pes audio2.mp2 1152 48000 384 -1 3600 > audio2.pes
%OCP%\esaudio2pes audio3.mp2 1152 48000 384 -1 3600 > audio3.pes
rem %OCP%\pesinfo audio1.pes

rem Create audio TS
%OCP%\pesaudio2ts 2068 1152 48000 384 0 audio1.pes > audio1.ts
%OCP%\pesaudio2ts 2078 1152 48000 384 0 audio2.pes > audio2.ts
%OCP%\pesaudio2ts 2088 1152 48000 384 0 audio3.pes > audio3.ts

rem Mux into the final TS
%OCP%\tscbrmuxer b:7100000 video1.ts b:188000 audio1.ts      b:7100000 video2.ts b:188000 audio2.ts      b:7100000 video3.ts b:188000 audio3.ts       b:1400 nit.ts b:3008 pat.ts b:1500 sdt.ts b:2000 tdt.ts       b:3008 pmt1.ts b:3008 pmt2.ts b:3008 pmt3.ts      b:2000 eit1.ts b:2000 eit2.ts b:2000 eit3.ts      b:507186 null.ts > HD3_64Q3414_22394118bps.ts

