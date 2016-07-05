# Introduce
A simple HTTP Live Stream Server. It's quite simple for now.  
To play video, you must create the playlist index file (.m3u8) and the transport stream files (.ts) for the video first.  
And the client (browser or video player which supports HLS) should know the link of the .m3u8.

# Create .m3u8 and .ts files
$ ffmpeg -i video.rmvb -acodec libfaac -vcodec libx264 -map 0 -f segment -segment_time 10 -segment_list video.m3u8 -segment_format mpegts -vbsf h264_mp4toannexb -flags -global_header viedo-stream%05d.ts  

NOTE: If you meet an error: `unknown encoder libfaac` or `unknown encoder libx264`, you should recompile ffmpeg with `--enable-libfaac` and `--enable-libx264`, these two features in ffmpeg are default to NO.

# Installation
$ cd lmvs/src  
$ make  
$ make install  

# Run
$ cd lmvs/bin  
$ ./run.sh  

# License

[MIT](http://opensource.org/licenses/MIT)

Copyright (c) 2016 Lampman Yao

