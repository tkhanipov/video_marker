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

#pragma once

#include "logger.h"

#include <wx/wx.h>

#include "videoreader.h"
#include "markedvideo.h"

class IntervalPanel;
class CanvasHolder;

struct CmdLineArguments
{
  wxString videoFile;
  wxString markup;
  long startFrame;

  CmdLineArguments()
  {
    videoFile = markup = wxT("");
    startFrame = 0;
  }
};

class Frame: public wxFrame
{
  public:
    Frame(const CmdLineArguments &);
    ~Frame();

    // event handlers
    void OnOpen(wxCommandEvent &);
    void OnSaveMarkup(wxCommandEvent &);
    void OnSaveMarkupAs(wxCommandEvent &);
    void OnLoadMarkup(wxCommandEvent &);
    void OnQuit(wxCommandEvent &);
    void OnSetGamma(wxCommandEvent &);
    void OnAbout(wxCommandEvent &);
    void OnMakeScreenshot(wxCommandEvent &);
    void OnSaveScreenshotAs(wxCommandEvent &);
    void OnCalcChecksum(wxCommandEvent &);

    void OnMark(wxCommandEvent &);
    void OnMarkNewStart(wxCommandEvent &);
    void OnMarkNewEnd(wxCommandEvent &);
    void OnSetType(wxCommandEvent &);
    void OnSetComment(wxCommandEvent &);
    void OnStepForward(wxCommandEvent &);
    void OnStepBackward(wxCommandEvent &);
    void OnLittleMoveForward(wxCommandEvent &);
    void OnLittleMoveBackward(wxCommandEvent &);
    void OnMoveForward(wxCommandEvent &);
    void OnMoveBackward(wxCommandEvent &);
    void OnNextInterval(wxCommandEvent &);
    void OnPrevInterval(wxCommandEvent &);
    void OnGotoInterval(wxCommandEvent &);
    void OnGotoIntervalEnd(wxCommandEvent &);
    void OnGotoIntervalStart(wxCommandEvent &);
    void OnDeleteInterval(wxCommandEvent &);
    void OnSetMoveSize(wxCommandEvent &);
    void OnSetLittleMoveSize(wxCommandEvent &);
    
    void OnToggleAutoLoadMarkup(wxCommandEvent &);

    void OnDumpInterval(wxCommandEvent &);
    void OnDumpIntervalTo(wxCommandEvent &);
    void OnDumpAllIntervals(wxCommandEvent &);
    void OnDumpAllIntervalsTo(wxCommandEvent &);
    void OnDeleteHeights(wxCommandEvent &);
    void OnCopyShortMovieName(wxCommandEvent &);

    void OnGoToFrame(wxCommandEvent &);
    void OnGoToMovieStart(wxCommandEvent &);
    void OnGoToMovieEnd(wxCommandEvent &);

    void OnSetMovieStartFrame(wxCommandEvent &);
    void OnSetMovieEndFrame(wxCommandEvent &);
    void OnUnsetMovieStartFrame(wxCommandEvent &);
    void OnUnsetMovieEndFrame(wxCommandEvent &);

    void OnIntervalLabel(wxCommandEvent &);

    void OnSetFastPlayFps(wxCommandEvent &);
    void OnSetSlowPlayFps(wxCommandEvent &);
    void OnSlowPlay(wxCommandEvent &);
    void OnFastPlay(wxCommandEvent &);
    void OnPlaybackTimer(wxTimerEvent &);

    void OnClose(wxCloseEvent &);

    void OnLeftDown(wxMouseEvent &);

    void OnSliderUpdate(wxScrollEvent &);

    void OpenImage();
    bool OpenVideo(const char *videoFileName, const char *markupName = 0, int startFrame = 0);

    void DrawHorizLine(wxMemoryDC &dc, int y, const wxColour &colour);
    int cutBottom, cutAxis, cutTop;

    wxTimer m_playbackTimer;
    int m_fastPlaybackStep;
    int m_fastPlaybackPeriod;
    int m_slowPlaybackStep;
    int m_slowPlaybackPeriod;
    int m_currentPlaybackStep;
    enum PlaybackState {playbackFast, playbackSlow, playbackStopped} m_playbackState;
    void setPlaybackState(enum PlaybackState);

    int currentFrameNumber();

    bool Synchronize(bool force = false);
    void DrawVerticalBorders(wxMemoryDC &dc, const video_markup::Interval *interval);
    void DrawIntervalAttributes(wxMemoryDC &dc, const video_markup::Interval *interval);
    void DrawCenterLine(wxMemoryDC &dc, const video_markup::Interval *interval);
    void UpdateStatusLine(const video_markup::Interval *interval);

    int oldWidth, oldHeight;

    CanvasHolder *canvasHolder;
    IntervalPanel *intervalPanel;
    wxSlider *frameSlider;

    wxBitmap pureBitmap;

    bool markupChanged;
    bool canResetMarkup() const;

    wxColour topHeightLineColour;

    MarkedVideo markedVideo;

    int moveSize;
    int littlemoveSize;
    double gamma;

    unsigned char gammaMatrix[256];
    void ApplyGammaCorrection();

    int intervalStartFrame;

  private:
    void InitMenu();

    wxMenuBar *menuBar;
    static wxTextCtrl *logPanel;
    static void logToPanelOld(const std::string &buf, Logger::MessageLevel);

    DECLARE_EVENT_TABLE()
};

const int StatusGeneral = 0;    // id for SetStatusText
const int StatusInterval = 1;
const int StatusFrame = 2;
const int StatusMouse = 3;
const int StatusPlayback = 4;
