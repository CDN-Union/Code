# 源码：
ffmpeg-3.0.2_flv265.tar.gz ---> flv封装H.265源代码，基于ffmpeg-3.0.2 <br>
#搜索"_FLV265"宏可见修改代码
<br>
<br>
# 编译：
$ ./configure --enable-gpl --enable-libx265 --extra-cflags=-D_FLV265 <br>
$ make & make install
<br>
<br>
