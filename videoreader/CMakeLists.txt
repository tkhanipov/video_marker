add_library(videoreader
  videoreader.h
  src/ffmpegvideo.cpp
  src/ffmpegvideo.h
  src/videoreader.cpp
  src/videoreader_ffmpeg.cpp
  src/videoreader_ffmpeg.h
)

target_link_libraries(videoreader
  avcodec
  avcore
  avdevice
  avfilter
  avformat
  avutil
  swscale
)
