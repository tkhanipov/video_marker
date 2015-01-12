project(video_marker)

if(WIN32)
  set(wxWidgets_CONFIGURATION msw)
  option(wxWidgets_ROOT_DIR "Path where wxWidgets 2.8 is installed" "")
  if(wxWidgets_ROOT_DIR)
    message(STATUS "Searching for wxWidgets in directory ${wxWidgets_ROOT_DIR}")
  endif()
else()
  # For some reason these settings don't work so they are set manually in wxWidgets_CONFIG_OPTIONS.
  # See wx-config -h for the full set.
  # set(wxWidgets_USE_DEBUG OFF)
  # set(wxWidgets_USE_UNICODE OFF)
  # set(wxWidgets_USE_UNIVERSAL OFF)
  # set(wxWidgets_USE_STATIC ON)
  set(wxWidgets_CONFIG_OPTIONS --unicode=no --universal=no --debug=no)
endif()


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
)

target_link_libraries(video_marker
  videoreader
  ${wxWidgets_LIBRARIES}
  pugixml
)
