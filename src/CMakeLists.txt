project(video_marker)

option(wxWidgets_ROOT_DIR "Path where wxWidgets 2.8 is installed" "")
find_package(wxWidgets 2.8 EXACT COMPONENTS core base)
if(NOT wxWidgets_FOUND)
  message(FATAL_ERROR "Cannot find wxWidgets. Try to set wxWidgets_ROOT_DIR cmake option manually")
endif()
include(${wxWidgets_USE_FILE})

add_executable(video_marker WIN32 
  app.cpp
  app.h
  canvas.cpp
  canvas.h
  canvas_holder.cpp
  canvas_holder.h
  frame.cpp
  frame.h
  frame_id.h
  logger.cpp
  logger.h
  main.cpp
  markedvideo.cpp
  markedvideo.h
  interval_panel.cpp
  interval_panel.h
  video_markup.h
  video_markup.cpp
  ffmpegvideo.cpp
  ffmpegvideo.h
  videoreader.cpp
  videoreader.h
  videoreader_ffmpeg.cpp
  videoreader_ffmpeg.h
)

target_link_libraries(video_marker
  avcodec
  avcore
  avdevice
  avfilter
  avformat
  avutil
  swscale

  ${wxWidgets_LIBRARIES}
  pugixml
)
