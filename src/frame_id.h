/*

Copyright (c) 2014 Timur M. Khanipov <khanipov@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

/** Widget identifiers and their linkage to methods for the Frame class
  */

#pragma once

enum {
  ID_ABOUT = wxID_HIGHEST + 1,
  ID_MAKE_SCREENSHOT,
  ID_SAVE_SCREENSHOT_AS,
  ID_STEP_FORWARD,
  ID_STEP_BACKWARD,
  ID_SET_MOVESIZE,
  ID_SET_LITTLEMOVESIZE,
  ID_MOVE_FORWARD,
  ID_MOVE_BACKWARD,
  ID_MARK,
  ID_LOAD_MARKUP,
  ID_SAVE_MARKUP,
  ID_SAVE_MARKUP_AS,
  ID_PREV_INTERVAL,
  ID_NEXT_INTERVAL,
  ID_DELETE_INTERVAL,
  ID_SET_TYPE,
  ID_LITTLEMOVE_FORWARD,
  ID_LITTLEMOVE_BACKWARD,
  ID_SET_GAMMA,
  ID_COPY_SHORT_MOVIE_NAME,
  ID_DUMP_INTERVAL,
  ID_DUMP_INTERVAL_TO,
  ID_DUMP_ALL_INTERVALS,
  ID_DUMP_ALL_INTERVALS_TO,
  ID_GOTO_INTERVAL,
  ID_CALC_CHECKSUM,
  ID_GOTO_INTERVAL_END,
  ID_GOTO_INTERVAL_START,
  ID_MARK_NEW_START,
  ID_MARK_NEW_END,
  ID_CALC_HEIGHT,
  ID_SLOW_PLAY,
  ID_FAST_PLAY,
  ID_SET_FAST_PLAY_FPS,
  ID_SET_SLOW_PLAY_FPS,
  ID_SET_COMMENT,
  ID_TOGGLE_AUTO_LOAD_MARKUP,
  ID_GOTO_FRAME,
  ID_GOTO_MOVIE_START,
  ID_GOTO_MOVIE_END,
  ID_SET_MOVIE_START_FRAME,
  ID_SET_MOVIE_END_FRAME,
  ID_UNSET_MOVIE_START_FRAME,
  ID_UNSET_MOVIE_END_FRAME,
  
  ID_INTERVAL_LABEL_FUNNY,
  ID_INTERVAL_LABEL_INTERESTING,
  ID_INTERVAL_LABEL_BAD
  };

#define PLAYBACK_TIMER_ID   10000

BEGIN_EVENT_TABLE(Frame, wxFrame)
  EVT_MENU(   wxID_OPEN,                         Frame::OnOpen                      )
  EVT_MENU(   ID_SAVE_MARKUP,                    Frame::OnSaveMarkup                )
  EVT_MENU(   ID_SAVE_MARKUP_AS,                 Frame::OnSaveMarkupAs              )
  EVT_MENU(   wxID_EXIT,                         Frame::OnQuit                      )
  EVT_MENU(   ID_ABOUT,                          Frame::OnAbout                     )
  EVT_MENU(   ID_MAKE_SCREENSHOT,                Frame::OnMakeScreenshot            )
  EVT_MENU(   ID_SAVE_SCREENSHOT_AS,             Frame::OnSaveScreenshotAs          )
  EVT_MENU(   ID_STEP_FORWARD,                   Frame::OnStepForward               )
  EVT_MENU(   ID_STEP_BACKWARD,                  Frame::OnStepBackward              )
  EVT_MENU(   ID_LITTLEMOVE_FORWARD,             Frame::OnLittleMoveForward         )
  EVT_MENU(   ID_LITTLEMOVE_BACKWARD,            Frame::OnLittleMoveBackward        )
  EVT_MENU(   ID_MOVE_FORWARD,                   Frame::OnMoveForward               )
  EVT_MENU(   ID_MOVE_BACKWARD,                  Frame::OnMoveBackward              )
  EVT_MENU(   ID_SET_MOVESIZE,                   Frame::OnSetMoveSize               )
  EVT_MENU(   ID_SET_LITTLEMOVESIZE,             Frame::OnSetLittleMoveSize         )
  EVT_MENU(   ID_MARK,                           Frame::OnMark                      )
  EVT_MENU(   ID_LOAD_MARKUP,                    Frame::OnLoadMarkup                )
  EVT_MENU(   ID_PREV_INTERVAL,                  Frame::OnPrevInterval              )
  EVT_MENU(   ID_NEXT_INTERVAL,                  Frame::OnNextInterval              )
  EVT_MENU(   ID_DELETE_INTERVAL,                Frame::OnDeleteInterval            )
  EVT_MENU(   ID_SET_TYPE,                       Frame::OnSetType                   )
  EVT_MENU(   ID_SET_COMMENT,                    Frame::OnSetComment                )
  EVT_MENU(   ID_SET_GAMMA,                      Frame::OnSetGamma                  )
  EVT_MENU(   ID_GOTO_FRAME,                     Frame::OnGoToFrame                 )
  EVT_MENU(   ID_GOTO_MOVIE_START,               Frame::OnGoToMovieStart            )
  EVT_MENU(   ID_GOTO_MOVIE_END,                 Frame::OnGoToMovieEnd              )
  EVT_MENU(   ID_COPY_SHORT_MOVIE_NAME,          Frame::OnCopyShortMovieName        )
  EVT_MENU(   ID_DUMP_INTERVAL,                  Frame::OnDumpInterval              )
  EVT_MENU(   ID_DUMP_INTERVAL_TO,               Frame::OnDumpIntervalTo            )
  EVT_MENU(   ID_DUMP_ALL_INTERVALS,             Frame::OnDumpAllIntervals          )
  EVT_MENU(   ID_DUMP_ALL_INTERVALS_TO,          Frame::OnDumpAllIntervalsTo        )
  EVT_MENU(   ID_GOTO_INTERVAL,                  Frame::OnGotoInterval              )
  EVT_MENU(   ID_CALC_CHECKSUM,                  Frame::OnCalcChecksum              )
  EVT_MENU(   ID_GOTO_INTERVAL_END,              Frame::OnGotoIntervalEnd           )
  EVT_MENU(   ID_GOTO_INTERVAL_START,            Frame::OnGotoIntervalStart         )
  EVT_MENU(   ID_MARK_NEW_START,                 Frame::OnMarkNewStart              )
  EVT_MENU(   ID_MARK_NEW_END,                   Frame::OnMarkNewEnd                )
  EVT_MENU(   ID_FAST_PLAY,                      Frame::OnFastPlay                  )
  EVT_MENU(   ID_SLOW_PLAY,                      Frame::OnSlowPlay                  )
  EVT_MENU(   ID_SET_SLOW_PLAY_FPS,              Frame::OnSetSlowPlayFps            )
  EVT_MENU(   ID_SET_FAST_PLAY_FPS,              Frame::OnSetFastPlayFps            )
  EVT_MENU(   ID_TOGGLE_AUTO_LOAD_MARKUP,        Frame::OnToggleAutoLoadMarkup      )
  EVT_MENU(   ID_SET_MOVIE_START_FRAME,          Frame::OnSetMovieStartFrame        )
  EVT_MENU(   ID_SET_MOVIE_END_FRAME,            Frame::OnSetMovieEndFrame          )
  EVT_MENU(   ID_UNSET_MOVIE_START_FRAME,        Frame::OnUnsetMovieStartFrame      )
  EVT_MENU(   ID_UNSET_MOVIE_END_FRAME,          Frame::OnUnsetMovieEndFrame        )

  EVT_MENU(   ID_INTERVAL_LABEL_FUNNY,           Frame::OnIntervalLabel             )
  EVT_MENU(   ID_INTERVAL_LABEL_INTERESTING,     Frame::OnIntervalLabel             )
  EVT_MENU(   ID_INTERVAL_LABEL_BAD,             Frame::OnIntervalLabel             )

  EVT_CLOSE(  Frame::OnClose  )

  EVT_TIMER(  PLAYBACK_TIMER_ID,                 Frame::OnPlaybackTimer             )
END_EVENT_TABLE()
