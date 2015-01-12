add_library(videoreader
  ffmpegvideo.cpp
  ffmpegvideo.h
  videoreader.cpp
  videoreader.h
  videoreader_ffmpeg.cpp
  videoreader_ffmpeg.h
)

target_link_libraries(video_reader
  avcodec
  avcore
  avdevice
  avfilter
  avformat
  avutil
  swscale
)
