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

#include <cstdio>
#include <cmath>
#include <iostream>
#include <sstream>
#include <wx/numdlg.h>
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/clipbrd.h>
#include <wx/filename.h>

#include "video_markup.h"
#include "frame.h"
#include "canvas_holder.h"
#include "interval_panel.h"
#include "frame_id.h"

const int IntervalDumpMargin = 20;

// settings' names
const char *seDefaultLogDir     = "defaultLogDir";
const char *seDefaultVideoDir   = "defaultVideoDir";
const char *seDefaultDumpDir    = "defaultDumpDir";
const char *seDefaultShotsDir   = "shotsDir";
const char *seMoveSize          = "moveSize";
const char *seLittleMoveSize    = "littleMoveSize";
const char *seFastPlayFps       = "fastPlayFps";
const char *seSlowPlayFps       = "slowPlayFps";
const char *seAutoLoadMarkup    = "autoLoadMarkup";

wxTextCtrl *Frame::logPanel = 0;

using video_markup::Interval;

void Frame::OnIntervalLabel(wxCommandEvent &event)
{
  Interval *interval = markedVideo.getCurrentInterval();
  if (!interval)
    return;
  std::string label;
  switch (event.GetId())
  {
  case ID_INTERVAL_LABEL_FUNNY:        label = "funny"; break;
  case ID_INTERVAL_LABEL_INTERESTING:  label = "interesting"; break;
  case ID_INTERVAL_LABEL_BAD:          label = "bad"; break;
  default:
    LOG_ERROR("Internal error: unknown command id");
    return;
  }

  std::set<std::string>::iterator it = interval->labels.find(label);
  if (it != interval->labels.end())
    interval->labels.erase(it);
  else
    interval->labels.insert(label);
  markupChanged = true;
  Synchronize(true);
}

void Frame::OnSetMovieStartFrame(wxCommandEvent &)
{
  markedVideo.setStartFrame( markedVideo.getCurrentFrameNumber() );
  markupChanged = true;
  Synchronize();
}

void Frame::OnSetMovieEndFrame(wxCommandEvent &)
{
  markedVideo.setEndFrame( markedVideo.getCurrentFrameNumber() );
  markupChanged = true;
  Synchronize();
}

void Frame::OnUnsetMovieStartFrame(wxCommandEvent &)
{
  markedVideo.setStartFrame(-1);
  markupChanged = true;
  Synchronize();
}

void Frame::OnUnsetMovieEndFrame(wxCommandEvent &)
{
  markedVideo.setEndFrame(-1);
  markupChanged = true;
  Synchronize();
}

void Frame::OnGoToFrame(wxCommandEvent &)
{
  wxNumberEntryDialog dialog(this, wxEmptyString, wxT("Enter frame number:"), wxT("Go to frame"), 0, 0, markedVideo.getTotalFrames() - 1);
  if (dialog.ShowModal() == wxID_OK)
  {
    markedVideo.goToFrame(dialog.GetValue());
    Synchronize();
  }
}

void Frame::OnGoToMovieStart(wxCommandEvent &)
{
  markedVideo.goToFrame(0);
  Synchronize();
}

void Frame::OnGoToMovieEnd(wxCommandEvent &)
{
  markedVideo.goToFrame( markedVideo.getTotalFrames()-1 );
  Synchronize();
}

void Frame::OnCopyShortMovieName(wxCommandEvent &)
{
  wxString name, ext;
  wxSplitPath(markedVideo.getVideoName().c_str(), 0, &name, &ext);
  if (wxTheClipboard->Open())
  {
    wxTheClipboard->SetData(new wxTextDataObject(name + ext));
    wxTheClipboard->Close();
  }
}

void Frame::setPlaybackState(Frame::PlaybackState newState)
{
  if (m_playbackState == newState)
    return;
  if (newState == playbackStopped)
  {
    m_playbackTimer.Stop();
    m_playbackState = playbackStopped;
    SetStatusText(wxT("Stopped"), StatusPlayback);
  }
  else if (newState == playbackFast)
  {
    if (m_playbackState != playbackStopped)
      m_playbackTimer.Stop();
    m_currentPlaybackStep = m_fastPlaybackStep;
    m_playbackTimer.Start(m_fastPlaybackPeriod);
    m_playbackState = playbackFast;
    SetStatusText(wxT("Fast"), StatusPlayback);
  }
  else if (newState == playbackSlow)
  {
    if (m_playbackState != playbackStopped)
      m_playbackTimer.Stop();
    m_currentPlaybackStep = m_slowPlaybackStep;
    m_playbackTimer.Start(m_slowPlaybackPeriod);
    m_playbackState = playbackSlow;
    SetStatusText(wxT("Slow"), StatusPlayback);
  }
  else
  {
    LOG_ERROR("Internal error: wrong playback state");
  }
}

void CalculatePlaybackParams(double fps, int *period, int *step)
{
  *period = int( 1000.0 / fps );
  // 40 ms <--> 25 fps. Human eye cannot see faster sequences
  if (*period < 40)
  {
    *period = 40;
    *step = int( 0.04 * fps );
  }
  else
    *step = 1;
}

void Frame::OnSetFastPlayFps(wxCommandEvent &)
{
  setPlaybackState(playbackStopped);
  wxNumberEntryDialog dialog(this, wxEmptyString, wxT("Fast playback speed:"), wxT("Fast playback speed (low values are equal to fps)"), wxConfigBase::Get()->Read(seFastPlayFps, 100), 1, 10000);
  if (dialog.ShowModal() == wxID_OK)
  {
    CalculatePlaybackParams( dialog.GetValue(), &m_fastPlaybackPeriod, &m_fastPlaybackStep );
    wxConfigBase::Get()->Write(seFastPlayFps, dialog.GetValue());
  }
}

void Frame::OnSetSlowPlayFps(wxCommandEvent &)
{
  setPlaybackState(playbackStopped);
  wxNumberEntryDialog dialog(this, wxEmptyString, wxT("Slow playback speed:"), wxT("Slow playback speed (low values are equal to fps)"), wxConfigBase::Get()->Read(seSlowPlayFps, 40), 1, 10000);
  if (dialog.ShowModal() == wxID_OK)
  {
    CalculatePlaybackParams( dialog.GetValue(), &m_slowPlaybackPeriod, &m_slowPlaybackStep );
    wxConfigBase::Get()->Write(seSlowPlayFps, dialog.GetValue());
  }
}

void Frame::OnPlaybackTimer(wxTimerEvent &)
{
  int nextFrame = markedVideo.getCurrentFrameNumber() + m_currentPlaybackStep;
  if (markedVideo.getTotalFrames() >= 0 && nextFrame >= markedVideo.getTotalFrames()
      || !markedVideo.goToFrame( nextFrame ))
  {
    setPlaybackState(playbackStopped);
  }
  Synchronize();
}

void Frame::OnFastPlay(wxCommandEvent &)
{
  if (m_playbackState == playbackFast)
    setPlaybackState(playbackStopped);
  else
    setPlaybackState(playbackFast);
}

void Frame::OnSlowPlay(wxCommandEvent &)
{
  if (m_playbackState == playbackSlow)
    setPlaybackState(playbackStopped);
  else
    setPlaybackState(playbackSlow);
}

void Frame::OnCalcChecksum(wxCommandEvent &)
{
  const MinImg *frame = markedVideo.getCurrentFrame();
  if (!frame)
    return;
  unsigned sum = 0;
  for (int i = 0; i < frame->height; i++)
    for (int j = 0; j < frame->width * frame->channels * frame->channelDepth; j++)
      sum += frame->pScan0[ i * frame->stride + j ];
  LOG_INFO("Checksum of frame #" << markedVideo.getCurrentFrameNumber() << " is " << sum);
}

void Frame::OnDumpIntervalTo(wxCommandEvent &)
{
  wxDirDialog dialog(this, wxT("Choose directory for interval dump"), wxConfigBase::Get()->Read(seDefaultDumpDir));
  if (dialog.ShowModal() == wxID_OK)
  {
    wxConfigBase::Get()->Write(seDefaultDumpDir, dialog.GetPath());
    LOG_INFO("Chosen dir " << dialog.GetPath());
    OnDumpInterval(wxCommandEvent());
  }
}

void Frame::OnDumpInterval(wxCommandEvent &)
{
  std::string dumpDir = wxConfigBase::Get()->Read(seDefaultDumpDir);
  if (dumpDir == "")
  {
    OnDumpIntervalTo(wxCommandEvent());
    return;
  }
  const Interval *interval = markedVideo.getCurrentInterval();
  if (!interval)
  {
    LOG_ERROR("No current interval!");
    return;
  }

  wxString videoName = wxFileName::FileName(markedVideo.getVideoName()).GetName();

  if (markedVideo.dump(interval->start - IntervalDumpMargin, interval->end + IntervalDumpMargin,
    (dumpDir + "/" + videoName.To8BitData() + "_%08d.jpg").c_str()))
  {
    LOG_INFO("Dumped successfully to " << dumpDir);
  }
  else
  {
    LOG_ERROR("Dump failed");
  }
}

void Frame::OnDumpAllIntervals(wxCommandEvent &)
{
  std::string dumpDir = wxConfigBase::Get()->Read(seDefaultDumpDir);
  if (dumpDir == "")
  {
    OnDumpAllIntervalsTo(wxCommandEvent());
    return;
  }
  int currentFrame = markedVideo.getCurrentFrameNumber();
  for (int i = 0; i < markedVideo.getTotalIntervals(); ++i)
  {
    markedVideo.gotoInterval(i);
    Synchronize();
    LOG_INFO("Dumping interval " << i);
    OnDumpInterval(wxCommandEvent());
  }
  markedVideo.goToFrame(currentFrame);
  Synchronize();
}

void Frame::OnDumpAllIntervalsTo(wxCommandEvent &)
{
  wxDirDialog dialog(this, wxT("Choose directory for intervals dump"), wxConfigBase::Get()->Read(seDefaultDumpDir));
  if (dialog.ShowModal() == wxID_OK)
  {
    wxConfigBase::Get()->Write(seDefaultDumpDir, dialog.GetPath());
    LOG_INFO("Chosen dir " << dialog.GetPath());
    OnDumpAllIntervals(wxCommandEvent());
  }
}

void Frame::OnSetComment(wxCommandEvent &)
{
  Interval *interval = markedVideo.getCurrentInterval();
  if (!interval)
    return;
  wxString str = interval->comment;
  wxTextEntryDialog dialog(this, wxT("Interval comment"), wxT("Input comment:"), str);
  if (dialog.ShowModal() == wxID_OK)
  {
    std::string value = dialog.GetValue();
    for (unsigned i = 0; i < value.length(); i++)
      if (!isalnum((unsigned char) value[i]) && !isspace((unsigned char) value[i]))
      {
        wxMessageBox(wxT("Illegal characters in comment"));
        return;
      }
    interval->comment = dialog.GetValue();
    markupChanged = true;
    intervalPanel->OnUpdateInterval(true);
    Synchronize();
  }
}

void Frame::OnSetGamma(wxCommandEvent &)
{
  wxString str;
  str.Printf(wxT("%.1lf"), gamma);
  wxTextEntryDialog dialog(this, wxT("Gamma correction"), wxT("Input gamma:"), str);
  if (dialog.ShowModal() == wxID_OK)
  {
    str = dialog.GetValue();
    double newGamma;
    if (!str.ToDouble(&newGamma))
    {
      wxMessageBox(wxT("Wrong format!"));
      return;
    }
    gamma = newGamma;
    for (int i = 0; i < 256; i++)
      gammaMatrix[i] = unsigned char(255.0 * pow(i / 255.0, gamma));
    Synchronize();
  }
}

void Frame::OnToggleAutoLoadMarkup(wxCommandEvent &event)
{
  markedVideo.setAutoLoadMarkup(event.IsChecked());
  wxConfigBase::Get()->Write(seAutoLoadMarkup, event.IsChecked());
  Synchronize();
}

void Frame::OnSetType(wxCommandEvent &)
{
  Interval *interval = markedVideo.getCurrentInterval();
  if (!interval)
    return;

  wxNumberEntryDialog dialog(this, wxEmptyString, wxT("Type of interval:"), wxT("Type of interval"), (long) interval->type, 0, 500);
  if (dialog.ShowModal() == wxID_OK)
  {
    int type = interval->type = dialog.GetValue();
    if (type < 1 || type > 4)
    {
      wxMessageBox(wxT("Invalid interval type. Possible types are 1, 2, 3 and 4"), wxT("Invalid interval type"), wxICON_ERROR);
      return;
    }
    markupChanged = true;
  }
  Synchronize();
}

void Frame::OnDeleteInterval(wxCommandEvent &)
{
  if (markedVideo.getCurrentInterval())
  {
    markedVideo.deleteCurrentInterval();
    markupChanged = true;
    Synchronize();
  }
}

void Frame::OnPrevInterval(wxCommandEvent &)
{
  markedVideo.moveToPrevInterval();
  Synchronize(true);
}

void Frame::OnNextInterval(wxCommandEvent &)
{
  markedVideo.moveToNextInterval();
  Synchronize(true);
}

void Frame::OnGotoIntervalStart(wxCommandEvent &)
{
  markedVideo.moveToIntervalStart();
  Synchronize();
}

void Frame::OnGotoIntervalEnd(wxCommandEvent &)
{
  markedVideo.moveToIntervalEnd();
  Synchronize();
}

void Frame::OnGotoInterval(wxCommandEvent &)
{

  wxNumberEntryDialog dialog(this, wxEmptyString, wxT("Enter interval id:"), wxT("Goto interval"), (long) markedVideo.getCurrentIntervalId(), 0, markedVideo.getTotalIntervals() - 1);
  if (dialog.ShowModal() == wxID_OK)
    if (!markedVideo.gotoInterval(dialog.GetValue()))
    {
      LOG_ERROR("Failed to go to interval!");
    }
  Synchronize(true);
}

bool Frame::canResetMarkup() const
{
  return !markupChanged || (wxYES == wxMessageBox(wxT("You have made changes to the markup without saving. Continue?"), wxT("Markup changed"), wxCENTRE | wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION));
}

void Frame::OnLoadMarkup(wxCommandEvent &)
{
  if (!canResetMarkup())
    return;

  wxString videoFileName;
  wxString videoFileExt;
  wxSplitPath(markedVideo.getVideoName().c_str(), 0, &videoFileName, &videoFileExt);
  wxString defaultLogName = videoFileName + wxT(".") + videoFileExt + wxT(".xml");

  wxFileDialog dialog(this, wxT("Load markup"), wxConfigBase::Get()->Read(seDefaultLogDir), defaultLogName, wxT("*.xml"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (dialog.ShowModal() == wxID_OK)
  {
    if (markedVideo.loadMarkup(dialog.GetPath().c_str()))
    {
      markupChanged = false;
      wxConfigBase::Get()->Write(seDefaultLogDir, dialog.GetDirectory());
      Synchronize(true);
    }
    else
    {
      wxMessageBox(wxT("Failed to load markup"), wxT("Error"), wxICON_ERROR);
    }
  }
}

void Frame::OnSaveMarkup(wxCommandEvent &)
{
  if (markedVideo.getMarkupName() == "")
    OnSaveMarkupAs(wxCommandEvent());
  else
  {
    if (markedVideo.saveMarkup( markedVideo.getMarkupName() ))
    {
      markupChanged = false;
      LOG_INFO("Markup saved to " << markedVideo.getMarkupName());
    }
    else
      wxMessageBox(wxT("Failed to save markup"), wxT("Error"), wxICON_ERROR);
  }
}

void Frame::OnSaveMarkupAs(wxCommandEvent &)
{
  wxString videoFileName;
  wxString videoFileExt;
  wxSplitPath(markedVideo.getVideoName().c_str(), 0, &videoFileName, &videoFileExt);
  wxString defaultLogName = videoFileName + wxT(".") + videoFileExt + wxT(".xml");
  wxFileDialog dialog(this, wxT("Save markup As..."), wxConfigBase::Get()->Read(seDefaultLogDir), defaultLogName, wxT("*.xml"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK)
  {
    if (markedVideo.saveMarkup(dialog.GetPath().c_str()))
    {
      markupChanged = false;
      markedVideo.setMarkupName(dialog.GetPath().c_str());
      wxConfigBase::Get()->Write(seDefaultLogDir, dialog.GetDirectory());
      LOG_INFO("Markup saved to " << dialog.GetPath());
    }
    else
      wxMessageBox(wxT("Failed to save markup"), wxT("Error"), wxICON_ERROR);
  }
}

void Frame::OnMark(wxCommandEvent &)
{
  if (markedVideo.getCurrentInterval())
    return;
  if (intervalStartFrame >= 0)
  {
    Interval interval;
    interval.start = intervalStartFrame;
    interval.end = currentFrameNumber();
    interval.type = 1;
    markedVideo.pushInterval(&interval);

    markupChanged = true;
    SetStatusText(wxEmptyString);
    intervalStartFrame = -1;
  }
  else
  {
    wxString status;
    status.sprintf(wxT("Enter @%d..."), currentFrameNumber());
    SetStatusText(status);
    intervalStartFrame = currentFrameNumber();
  }
  Synchronize();
}

void Frame::OnMarkNewEnd(wxCommandEvent &)
{
  Interval *interval = markedVideo.getCurrentInterval();
  int frame = markedVideo.getCurrentFrameNumber();
  if (interval && frame > interval->start)
  {
    interval->end = frame;
    markupChanged = true;
    LOG_INFO("New interval end was set to " << frame);
    Synchronize();
  }
  else
  {
    if (!markedVideo.moveToPrevInterval())
      return;
    Interval *interval = markedVideo.getCurrentInterval();
    interval->end = frame;
    markedVideo.goToFrame(frame);
    int id = markedVideo.getCurrentIntervalId();
    LOG_INFO("Interval " << id << ": new interval end was set to " << frame);
    Synchronize();
  }
}

void Frame::OnMarkNewStart(wxCommandEvent &)
{
  Interval *interval = markedVideo.getCurrentInterval();
  int frame = markedVideo.getCurrentFrameNumber();
  if (interval)
  {
    interval->start = frame;
    markupChanged = true;
    LOG_INFO("New start frame was set to " << frame);
    Synchronize();
  }
  else
  {
    if (!markedVideo.moveToNextInterval())
      return;
    Interval *interval = markedVideo.getCurrentInterval();
    interval->start = frame;
    markedVideo.goToFrame(frame);
    int id = markedVideo.getCurrentIntervalId();
    LOG_INFO("Interval " << id << ": new start frame was set to " << frame);
    Synchronize();
  }
}

void Frame::OnSetMoveSize(wxCommandEvent &)
{
  wxNumberEntryDialog dialog(this, wxEmptyString, wxT("Set move size:"), wxT("Move size"), (long) moveSize, 1, 100000);
  if (dialog.ShowModal() == wxID_OK)
  {
    moveSize = dialog.GetValue();
    wxConfigBase::Get()->Write(seMoveSize, moveSize);
  }
}

void Frame::OnSetLittleMoveSize(wxCommandEvent &)
{
  wxNumberEntryDialog dialog(this, wxEmptyString, wxT("Set little move size:"), wxT("Little move size"), (long) littlemoveSize, 1, 100000);
  if (dialog.ShowModal() == wxID_OK)
  {
    littlemoveSize = dialog.GetValue();
    wxConfigBase::Get()->Write(seLittleMoveSize, littlemoveSize);
  }
}

int Frame::currentFrameNumber()
{
  return markedVideo.getCurrentFrameNumber();
}

void Frame::DrawVerticalBorders(wxMemoryDC &dc, const Interval *interval)
{
  int maxX = pureBitmap.GetWidth() - 1;
  dc.SetPen(wxPen(wxColour(0, 255, 255)));
}

void Frame::DrawIntervalAttributes(wxMemoryDC &dc, const Interval *interval)
{
  if (interval)
  {
    wxString sIntervalTypes;
    sIntervalTypes.sprintf(wxT("T: %d"), interval->type);

    dc.SetTextForeground(wxColour(0, 255, 0));
    dc.SetFont(wxFont(20, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    int x, y;
    canvasHolder->GetViewStart(&x, &y);
    dc.DrawText(sIntervalTypes, 5, 5 + y);

    if (interval->labels.count("inverse_direction"))
    {
      dc.SetTextForeground(wxColour(255, 0, 0));
      dc.DrawText("<<<", 350, 15);
    }
  }
}

void Frame::UpdateStatusLine(const Interval *interval)
{
  // frame number
  wxString s;
  int totalFrames;
  if ((totalFrames = markedVideo.getTotalFrames()) > 0)
    s.Printf(wxT("%d/%d"), currentFrameNumber(), totalFrames - 1);
  else
    s.Printf(wxT("%d/-"), currentFrameNumber());
  SetStatusText(s, StatusFrame);

  if (interval)
  {
    // interval number
    s.Printf(wxT("Interval %d/%d"), markedVideo.getCurrentIntervalId()+1, markedVideo.getTotalIntervals());
    SetStatusText(s, StatusInterval);
  }
  else
  {
    s.Printf(wxT("Interval -/%d"), markedVideo.getTotalIntervals());
    SetStatusText(s, StatusInterval);
  }
}

bool Frame::Synchronize(bool force)
{
  // temporary way to check if the video is opened
  if (!markedVideo.getCurrentFrame())
    return false;
  OpenImage();

  if (pureBitmap.GetWidth() != oldWidth || pureBitmap.GetHeight() != oldHeight)
  {
    oldWidth = pureBitmap.GetWidth();
    oldHeight = pureBitmap.GetHeight();
    canvasHolder->OnBitmapUpdate(true);
  }

  wxMemoryDC dc(pureBitmap);

  const Interval *interval = markedVideo.getCurrentInterval();

  DrawCenterLine(dc, interval);
  DrawHorizLine(dc, cutTop, wxColour(250, 200 , 200));

  DrawIntervalAttributes(dc, interval);
  UpdateStatusLine(interval);

  canvasHolder->OnBitmapUpdate();
  intervalPanel->OnUpdateInterval(force);

  frameSlider->SetValue(markedVideo.getCurrentFrameNumber());

  return true;
}

void Frame::OnLeftDown(wxMouseEvent &event)
{
  Interval *interval = markedVideo.getCurrentInterval();
  if (!interval)
    return;
  interval->y_border = event.GetPosition().y;

  markupChanged = true;
  Synchronize();
}

void Frame::OnDeleteHeights(wxCommandEvent &)
{
  Interval *interval = markedVideo.getCurrentInterval();
  if (interval)
  {
    interval->y_border = -1;
    Synchronize();
  }
}

void Frame::OnStepForward(wxCommandEvent &)
{
  markedVideo.getNextFrame();
  Synchronize();
}

void Frame::OnStepBackward(wxCommandEvent &)
{
  markedVideo.goToFrame( markedVideo.getCurrentFrameNumber() - 1 );
  Synchronize();
}

void Frame::OnLittleMoveForward(wxCommandEvent &)
{
  markedVideo.goToFrame( markedVideo.getCurrentFrameNumber() + littlemoveSize );
  Synchronize();
}

void Frame::OnLittleMoveBackward(wxCommandEvent &)
{
  markedVideo.goToFrame( markedVideo.getCurrentFrameNumber() - littlemoveSize );
  Synchronize();
}

void Frame::OnMoveForward(wxCommandEvent &)
{
  markedVideo.goToFrame( markedVideo.getCurrentFrameNumber() + moveSize );
  Synchronize();
}

void Frame::OnMoveBackward(wxCommandEvent &)
{
  markedVideo.goToFrame( markedVideo.getCurrentFrameNumber() - moveSize );
  Synchronize();
}

void Frame::OnMakeScreenshot(wxCommandEvent &)
{
  if (pureBitmap.SaveFile(wxT("screenshot.bmp"), wxBITMAP_TYPE_BMP))
    LOG_INFO("Screenshot saved to screenshot.bmp")
  else
    LOG_ERROR("Cannot save screenshot");
}

void Frame::OnSaveScreenshotAs(wxCommandEvent &)
{
  wxString videoFileName;
  wxString videoFileExt;
  wxSplitPath(markedVideo.getVideoName().c_str(), 0, &videoFileName, &videoFileExt);
  wxString frameStr;
  frameStr.Printf(wxT("-%d"), markedVideo.getCurrentFrameNumber());
  wxString defaultShotName = videoFileName + wxT(".") + videoFileExt + frameStr + wxT(".bmp");
  wxFileDialog dialog(this, wxT("Save screenshot as..."), wxConfigBase::Get()->Read(seDefaultShotsDir), defaultShotName, wxT(".bmp"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK)
  {
    if (pureBitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_BMP))
    {
      LOG_INFO("Screenshot saved to " << dialog.GetPath().c_str());
      wxConfigBase::Get()->Write(seDefaultShotsDir, dialog.GetDirectory());
    }
    else
    {
      LOG_ERROR("Cannot save screenshot");
    }
  }
}

void Frame::OnAbout(wxCommandEvent &)
{
  wxMessageBox(wxT("Program for linear interval mark-up in video"), wxT("About"), wxOK | wxICON_INFORMATION, this);
}

Frame::~Frame()
{
  Logger::SetLogFunction(0);
}

void Frame::logToPanelOld(const std::string &buf, Logger::MessageLevel level)
{
#ifdef _UNICODE
  std::wstring wstr(buf.begin(), buf.end());
  (*logPanel) << wstr;
#else
  switch (level)
  {
    case Logger::error:     logPanel->SetDefaultStyle(wxTextAttr(*wxRED));                  break;
    case Logger::warning:   logPanel->SetDefaultStyle(wxTextAttr(wxColor(250, 200, 0)));    break;
    case Logger::debug:     logPanel->SetDefaultStyle(wxTextAttr(*wxGREEN));                break;
    case Logger::trace:     logPanel->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY));           break;
    default:                logPanel->SetDefaultStyle(wxTextAttr(*wxBLACK));                break;
  }
  (*logPanel) << buf;
#endif
  logPanel->ScrollLines(1);
}

void Frame::DrawHorizLine(wxMemoryDC &dc, int y, const wxColour &colour)
{
  if (y < 0 || y >= pureBitmap.GetHeight())
    return;

  dc.SetPen(wxPen(colour));
  dc.DrawLine(0, y, pureBitmap.GetWidth(), y);
}

void Frame::DrawCenterLine(wxMemoryDC &dc, const Interval *interval)
{
  wxColour colour;
  if (interval)
  {
    colour = wxColour(255, 0, 0);
  }
  else   // no interval in sight
  {
    if (intervalStartFrame != -1)
      colour = wxColour(255, 0, 0);
    else if (markedVideo.frameWithinBorders( markedVideo.getCurrentFrameNumber() ))
      colour = wxColour(0, 255, 0);
    else
      colour = wxColour(100, 100, 100);
  }

  int xcenter = pureBitmap.GetWidth() / 2;

  dc.SetPen(wxPen(colour));
  dc.DrawLine(xcenter, 0, xcenter, pureBitmap.GetHeight());
}

void Frame::ApplyGammaCorrection()
{
  wxImage img = pureBitmap.ConvertToImage();
  int w = img.GetWidth();
  int h = img.GetHeight();

  int bytesPerPix = 3;
  int stride = w * bytesPerPix;
  uint8_t *pLine, *pix;
  int x, y;

  for (y = 0, pLine = img.GetData(); y < h; y++, pLine += stride)
    for (x = 0, pix = pLine; x < w; x++, pix += bytesPerPix)
    {
      pix[0] = gammaMatrix[ pix[0] ];
      pix[1] = gammaMatrix[ pix[1] ];
      pix[2] = gammaMatrix[ pix[2] ];
    }
  pureBitmap = wxBitmap(img);
}

void Frame::OpenImage()
{
  const MinImg *minimg = markedVideo.getCurrentFrame();
  assert(minimg);
  assert(minimg->channels == 3 && minimg->channelDepth == 1 && minimg->format == FMT_UINT);

  //FIXME: copying each line may cause speed issues
  wxImage image(minimg->width, minimg->height, false);
  uint8_t *imageData = image.GetData();
  int imageDataStride = minimg->width * 3;
  for (int i = 0; i < minimg->height; i++)
    memcpy(imageData + i * imageDataStride, minimg->pScan0 + i * minimg->stride, imageDataStride);
  pureBitmap = wxBitmap(image);

  ApplyGammaCorrection();
}

bool Frame::OpenVideo(const char *videoFileName, const char *markupName, int startFrame)
{
  setPlaybackState(playbackStopped);
  LOG_INFO("Opening video. This may take some time...");

  frameSlider->SetMax(0);   // resetting frameSlider
  if (!markedVideo.loadVideo(videoFileName, markupName))
  {
    LOG_ERROR("Failed to open video " << videoFileName);
    Raise();      // to ensure that the app is an active windows app
    return false;
  }
  markupChanged = false;
  LOG_INFO("Successfully opened video " << markedVideo.getVideoName());

  if (startFrame > 0)
  {
    if (!markedVideo.goToFrame(startFrame))
    {
      LOG_ERROR("Seek failed");
    }
  }

  intervalStartFrame = -1;
  oldWidth = oldHeight = -1;
  cutTop = -1;

  SetTitle(videoFileName);
  SetStatusText(wxEmptyString);
  frameSlider->SetMax(markedVideo.getTotalFrames());

  Synchronize(true);
  Raise();      // to ensure that the app is an active windows app
  return true;
}

void Frame::OnOpen(wxCommandEvent &)
{
  if (!canResetMarkup())
    return;
  wxFileDialog dialog(this, wxT("Load video"), wxConfigBase::Get()->Read(seDefaultVideoDir), wxEmptyString, wxT("*.avi"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (dialog.ShowModal() == wxID_OK)
  {
    if (OpenVideo(dialog.GetPath().c_str()))
      wxConfigBase::Get()->Write(seDefaultVideoDir, dialog.GetDirectory());
  }
}

void Frame::OnClose(wxCloseEvent &event)
{
  if (canResetMarkup())
    Destroy();
}

void Frame::OnQuit(wxCommandEvent &)
{
  Close();
}

void Frame::OnSliderUpdate(wxScrollEvent &e)
{
  int currentFrameNumber = markedVideo.getCurrentFrameNumber();
  int newValue = frameSlider->GetValue();
  if (currentFrameNumber >= 0 && newValue != currentFrameNumber)
  {
    markedVideo.goToFrame(newValue);
    Synchronize();
  }
}

Frame::Frame(const CmdLineArguments &cmdLineArguments)
  : wxFrame(0, wxID_ANY, wxT("Marker"))
  , intervalStartFrame(-1)
  , cutTop(-1)
  , oldWidth(-1)
  , oldHeight(-1)
  , pureBitmap(wxBitmap(640, 480))
  , gamma(1.0)
  , topHeightLineColour(wxColour(255, 0, 0))
  , m_playbackTimer(this, PLAYBACK_TIMER_ID)
  , m_playbackState(playbackStopped)
  , markupChanged(false)
{
  // initializing some parameters from the config
  wxConfigBase::Create();
  moveSize = wxConfigBase::Get()->Read(seMoveSize, 40);

  bool autoLoadMarkup_flag;
  wxConfigBase::Get()->Read(seAutoLoadMarkup, &autoLoadMarkup_flag, true);
  markedVideo.setAutoLoadMarkup(autoLoadMarkup_flag);

  littlemoveSize = wxConfigBase::Get()->Read(seLittleMoveSize, 5);
  CalculatePlaybackParams( wxConfigBase::Get()->Read(seFastPlayFps, 100), &m_fastPlaybackPeriod, &m_fastPlaybackStep);
  CalculatePlaybackParams( wxConfigBase::Get()->Read(seSlowPlayFps, 40), &m_slowPlaybackPeriod, &m_slowPlaybackStep);
  m_currentPlaybackStep = m_slowPlaybackStep;

  for (int i = 0; i < 256; i++)
    gammaMatrix[i] = i;
  InitMenu();

  // initializing status bar
  // BE CAREFUL AND MODIFY ALL THESE THREE LINES TOGETHER
  wxStatusBar *statusBar = CreateStatusBar(5);
  int widths[] = {-1, 100, 100, 80, 70};
  statusBar->SetStatusWidths(5, widths);

  canvasHolder = new CanvasHolder(this);
  intervalPanel = new IntervalPanel(this);

  // window layout
  wxBoxSizer *vertSizer = new wxBoxSizer(wxVERTICAL);
  vertSizer->Add(canvasHolder, 8, wxRIGHT | wxEXPAND, 5);

  Logger::SetLogLevel(Logger::trace);
  Logger::Format() = Logger::showTime;

  frameSlider = new wxSlider(this, wxID_ANY, 0, 0, 0);
  frameSlider->SetPageSize(100);
  Connect(frameSlider->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxScrollEventHandler(Frame::OnSliderUpdate));
  vertSizer->Add(frameSlider, 0, wxEXPAND, 2);

  if (!logPanel)
  {
    logPanel = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxHSCROLL | wxTE_RICH);
    vertSizer->Add(logPanel, 1, wxEXPAND | wxTOP, 5);

    Logger::SetLogFunction(Frame::logToPanelOld);
  }
  else
    LOG_WARNING("Probably more than one instance of the Frame class exists");


  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  topSizer->Add(vertSizer, 5, wxEXPAND);
  topSizer->Add(intervalPanel, 1);

  SetSizerAndFit(topSizer);
  SetFocus();

  // parsing command line
  if (cmdLineArguments.videoFile != wxEmptyString)
  {
    if (cmdLineArguments.markup != wxEmptyString)
      OpenVideo(cmdLineArguments.videoFile.c_str(), cmdLineArguments.markup.c_str(), cmdLineArguments.startFrame);
    else
      OpenVideo(cmdLineArguments.videoFile.c_str(), 0, cmdLineArguments.startFrame);
  }
}

void Frame::InitMenu()
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_OPEN, wxT("&Open video\tCtrl+O"), wxT("Open video"));
  fileMenu->Append(ID_LOAD_MARKUP, wxT("&Load markup\tCtrl+L"), wxT("Load markup"));
  fileMenu->Append(ID_SAVE_MARKUP, wxT("&Save markup\tCtrl+S"), wxT("Save markup"));
  fileMenu->Append(ID_SAVE_MARKUP_AS, wxT("Save markup &As..."), wxT("Save markup As..."));
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, wxT("E&xit"), wxT("Quit the program"));

  wxMenu *navigateMenu = new wxMenu;
  navigateMenu->Append(ID_STEP_FORWARD, wxT("Next frame\tRight"), wxT("Step to the next frame"));
  navigateMenu->Append(ID_STEP_BACKWARD, wxT("Previous frame\tLeft"), wxT("Step back one frame"));
  navigateMenu->Append(ID_LITTLEMOVE_FORWARD, wxT("Little move forward\tAlt+Right"), wxT("Jump a few frames forward (adjustable)"));
  navigateMenu->Append(ID_LITTLEMOVE_BACKWARD, wxT("Little move backward\tAlt+Left"), wxT("Jump a few frames backward (adjustable)"));
  navigateMenu->Append(ID_MOVE_FORWARD, wxT("Move forward\tShift+Right"), wxT("Move several more frames forward (adjustable)"));
  navigateMenu->Append(ID_MOVE_BACKWARD, wxT("Move backward\tShift+Left"), wxT("Move several more frames backward (adjustable)"));
  navigateMenu->AppendSeparator();
  navigateMenu->Append(ID_GOTO_FRAME, wxT("Go to &frame"));
  navigateMenu->Append(ID_GOTO_MOVIE_START, wxT("Go to movie &start"));
  navigateMenu->Append(ID_GOTO_MOVIE_END, wxT("Go to movie &end"));
  navigateMenu->AppendSeparator();
  navigateMenu->Append(ID_NEXT_INTERVAL, wxT("Next interval\tCtrl+Right"), wxT("Next interval"));
  navigateMenu->Append(ID_PREV_INTERVAL, wxT("Previous interval\tCtrl+Left"), wxT("Previous interval"));
  navigateMenu->Append(ID_GOTO_INTERVAL, wxT("Go to interval\tCtrl+F"), wxT("Go to a given interval"));
  navigateMenu->Append(ID_GOTO_INTERVAL_END, wxT("Interval end\tEnd"), wxT("Go to interval end"));
  navigateMenu->Append(ID_GOTO_INTERVAL_START, wxT("Interval start\tHome"), wxT("Go to interval start"));

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(ID_SET_GAMMA, wxT("&Gamma correction\tCtrl+G"), wxT("Set gamma correction parameter"));

  wxMenu *miscMenu = new wxMenu;
  miscMenu->Append(ID_MAKE_SCREENSHOT, wxT("Make &screenshot"), wxT("Make a screenshot"));
  miscMenu->Append(ID_SAVE_SCREENSHOT_AS, wxT("Save screenshot &as...\tCtrl+R"), wxT("Make a screenshot and save it to a given file"));
  miscMenu->Append(ID_CALC_CHECKSUM, wxT("&Checksum"), wxT("Calculate frame checksum"));

  wxMenu *playMenu = new wxMenu;
  playMenu->Append(ID_FAST_PLAY, wxT("&Fast play\tS"), wxT("Start or stop fast playback"));
  playMenu->Append(ID_SLOW_PLAY, wxT("S&low play\tC"), wxT("Start or stop slow playback"));
  playMenu->Append(ID_SET_FAST_PLAY_FPS, wxT("Set f&ast playback speed"), wxT("Set fast playback speed"));
  playMenu->Append(ID_SET_SLOW_PLAY_FPS, wxT("Set slo&w playback speed"), wxT("Set slow playback speed"));

  wxMenu *intervalMenu = new wxMenu;
  intervalMenu->Append(ID_MARK, wxT("&Mark enter/leave\tSpace"), wxT("Mark enter/leave"));
  intervalMenu->Append(ID_MARK_NEW_START, wxT("Mark new &start\tX"), wxT("Set interval start frame to the current frame"));
  intervalMenu->Append(ID_MARK_NEW_END, wxT("Mark new &end\tZ"), wxT("Set interval end frame to the current frame"));
  intervalMenu->Append(ID_DELETE_INTERVAL, wxT("&Delete\tDelete"), wxT("Delete current interval"));
  intervalMenu->Append(ID_SET_TYPE, wxT("Type\tT"), wxT("Set interval type"));
  intervalMenu->Append(ID_SET_COMMENT, wxT("Ñomment\tAlt+C"), wxT("Set interval comment"));
  intervalMenu->AppendSeparator();
  intervalMenu->Append(ID_DUMP_INTERVAL, wxT("D&ump\tCtrl+D"), wxT("Dump current interval's frames"));
  intervalMenu->Append(ID_DUMP_INTERVAL_TO, wxT("Dump t&o..."), wxT("Dump current interval's frames to a given directory"));
     wxMenu *labelMenu = new wxMenu;
     labelMenu->Append(ID_INTERVAL_LABEL_FUNNY, wxT("&Funny"), wxT("Funny element label"));
     labelMenu->Append(ID_INTERVAL_LABEL_INTERESTING, wxT("&Interesting"), wxT("Interesting element label"));
     labelMenu->Append(ID_INTERVAL_LABEL_BAD, wxT("&Bad"), wxT("Bad element label"));
  intervalMenu->AppendSubMenu(labelMenu, wxT("&Label"));

  wxMenu *movieMenu = new wxMenu;
  movieMenu->Append(ID_SET_MOVIE_START_FRAME, wxT("&Set start frame"), wxT("Set current frame as start frame"));
  movieMenu->Append(ID_SET_MOVIE_END_FRAME, wxT("Set &end frame"), wxT("Set current frame as end frame"));
  movieMenu->Append(ID_UNSET_MOVIE_START_FRAME, wxT("&Unset start frame"), wxT("Unset start frame"));
  movieMenu->Append(ID_UNSET_MOVIE_END_FRAME, wxT("Unset e&nd frame"), wxT("Unset end frame"));
  movieMenu->Append(ID_DUMP_ALL_INTERVALS, wxT("Dump all intervals"), wxT("Dump all intervals to the previously selected directory"));
  movieMenu->Append(ID_DUMP_ALL_INTERVALS_TO, wxT("Dump all intervals to..."), wxT("Dump all intervals to a given directory"));
  movieMenu->AppendSeparator();
  movieMenu->Append(ID_COPY_SHORT_MOVIE_NAME, wxT("Copy short name\tCtrl+Insert"), wxT("Copy short movie name to clipboard"));

  wxMenu *settingsMenu = new wxMenu;
  settingsMenu->Append(ID_SET_MOVESIZE, wxT("&Move size\tCtrl+M"), wxT("Set on how many frames move forward and backward"));
  settingsMenu->Append(ID_SET_LITTLEMOVESIZE, wxT("L&ittle move size\tCtrl+I"), wxT("Set on how many frames move forward and backward"));
  settingsMenu->AppendSeparator();
  settingsMenu->AppendCheckItem(ID_TOGGLE_AUTO_LOAD_MARKUP, wxT("&Auto load markup"), wxT("Try to load markup automatically"));
  settingsMenu->Check(ID_TOGGLE_AUTO_LOAD_MARKUP, markedVideo.getAutoLoadMarkup());


  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(ID_ABOUT, wxT("&About\tF1"), wxT("Show program information"));

  menuBar = new wxMenuBar();
  menuBar->Append(fileMenu, wxT("&File"));
  menuBar->Append(navigateMenu, wxT("&Navigate"));
  menuBar->Append(viewMenu, wxT("&View"));
  menuBar->Append(intervalMenu, wxT("&Interval"));
  menuBar->Append(movieMenu, wxT("&Movie"));
  menuBar->Append(playMenu, wxT("&Play"));
  menuBar->Append(miscMenu, wxT("Mis&c"));
  menuBar->Append(settingsMenu, wxT("&Settings"));
  menuBar->Append(helpMenu, wxT("&Help"));
  SetMenuBar(menuBar);
}
