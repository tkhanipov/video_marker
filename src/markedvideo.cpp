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

#include <cmath>
#include <sstream>
#include <algorithm>

#include <videoreader.h>
#include <wx/image.h>

#include "markedvideo.h"
#include "logger.h"

using video_markup::Interval;

// if we jump to less than this many frames forward, use multiple readNextFrame()
// instead of seeking - this increases speed
#define GOTOFRAME_SEEK_THRES   200

MarkedVideo::MarkedVideo()
: _autoLoadMarkup_flag(true)
{
  _videoReader = createVideoReader(VideoReader::FFMpegReader);
}

MarkedVideo::~MarkedVideo()
{
  closeVideo();
  deleteVideoReader(_videoReader);
  _videoReader = 0;
}

void MarkedVideo::closeVideo()
{
  _videoReader->close();
  _videoName = "";

  _markup.clear();
}

bool MarkedVideo::loadVideo(const std::string &name, const char *markupName)
{
  closeVideo();

  if (!_videoReader->open(name.c_str()))
    return false;
  if (!_videoReader->readNextFrame())
    return false;
  _videoName = name;

  std::string markupPath;
  if (markupName)
  {
    markupPath = markupName;
    LOG_INFO("Using custom markup '" << markupPath << "'...");
    loadMarkup(markupPath);
  }
  else if (_autoLoadMarkup_flag)
  {
    markupPath = guessMarkupName(name);
    LOG_INFO("Trying to load markup automatically from '" << markupPath << "'...");
    loadMarkup(markupPath);
  }
  return true;
}

bool MarkedVideo::loadMarkup(const std::string &name)
{
  _markup.clear();
  _markupName = "";
  if (!_videoReader->isOpened())
  {
    LOG_ERROR("Video must be loaded prior to markup");
    return false;
  }
  if (!_markup.load(name.c_str()))
  {
    LOG_ERROR("Failed to load markup from '" << name << "'");
    return false;
  }

  int totalFrames = _videoReader->getTotalFrames();
  if (totalFrames >= 0 && _markup.size() > 0 && _markup.at(_markup.size() - 1).end >= totalFrames)
  {
    LOG_WARNING("Last markup interval is outside of the video");
  }

  int comments = 0;
  for (unsigned i = 0; i < _markup.size(); i++)
    if (_markup.at(i).comment != "")
      comments++;

  std::ostringstream oss;
  oss << _markup.size() << " interval(s) loaded";
  if (comments)
    oss << " and " << comments << " comment(s)";

  LOG_INFO(oss.str());
  _markupName = name;
  return true;
}

int MarkedVideo::getTotalFrames()
{
  return _videoReader->getTotalFrames();
}

Interval *MarkedVideo::getCurrentInterval()
{
  int frame = getCurrentFrameNumber();
  if (frame < 0)
    return 0;
  int id = _markup.find(frame);
  if (id < 0)
    return 0;
  else
    return &_markup.at(id);
}

int MarkedVideo::getTotalIntervals()
{
  return _markup.size();
}

bool MarkedVideo::pushInterval(const Interval *interval)
{
  return _markup.push(*interval) >= 0;
}

bool MarkedVideo::deleteCurrentInterval()
{
  int frame = getCurrentFrameNumber();
  if (frame < 0)
    return false;
  int id = _markup.find(frame);
  if (id < 0)
    return false;
  else
    return _markup.erase(id);
}

bool MarkedVideo::moveToIntervalEnd()
{
  const Interval *interval = getCurrentInterval();
  if (interval)
    return goToFrame(interval->end);
  else
    return false;
}

bool MarkedVideo::moveToIntervalStart()
{
  const Interval *interval = getCurrentInterval();
  if (interval)
    return goToFrame(interval->start);
  else
    return false;
}

bool MarkedVideo::moveToNextInterval()
{
  int interval = _markup.findNextClosest(getCurrentFrameNumber());
  if (interval < 0)
    return false;
  goToFrame(_markup.at(interval).start);
  return true;
}

bool MarkedVideo::moveToPrevInterval()
{
  int interval = _markup.findPrevClosest(getCurrentFrameNumber());
  if (interval < 0)
    return false;
  goToFrame(_markup.at(interval).start);
  return true;
}

const MinImg *MarkedVideo::getNextFrame()
{
  return _videoReader->readNextFrame();
}

const MinImg *MarkedVideo::getCurrentFrame()
{
  return _videoReader->getCurrentFrame();
}

int MarkedVideo::getCurrentIntervalId()
{
  int frame = getCurrentFrameNumber();
  if (frame < 0)
    return -1;
  return _markup.find(frame);
}

int MarkedVideo::getCurrentFrameNumber()
{
  int pos = _videoReader->getPos();
  if (pos < 0)
    return -1;
  else
    return pos - 1;
}

bool MarkedVideo::saveMarkup(const std::string &name) const
{
  return _markup.save(name.c_str());
}

bool MarkedVideo::gotoInterval(int id)
{
  if (!_videoReader->isOpened())
    return false;
  if (id < 0 || id >= (int) _markup.size())
    return false;
  if (!_videoReader->seek(_markup.at(id).start))
  {
    LOG_ERROR("VideoReader::seek() failed");
    return false;
  }
  if (!_videoReader->readNextFrame())
    return false;
  return true;
}

bool MarkedVideo::frameWithinBorders(int frame) const
{
  int start = _markup.getStartFrame();
  int end   = _markup.getEndFrame();
  if (start >= 0 && frame < start)
    return false;
  else if (end >= 0 && frame > end)
    return false;
  else
    return true;
}

bool MarkedVideo::setStartFrame(int startFrame)
{
  if (!_markup.setStartFrame(startFrame))
  {
    LOG_ERROR("Failed to set start frame. Check if intervals before the frame exist");
    return false;
  }
  else
    return true;
}

bool MarkedVideo::setEndFrame(int endFrame)
{
  if (!_markup.setEndFrame(endFrame))
  {
    LOG_ERROR("Failed to set end frame. Check if intervals after the frame exist");
    return false;
  }
  else
    return true;
}

bool MarkedVideo::goToFrame(int frameNumber)
{
  if (!_videoReader->isOpened())
    return false;
  if (frameNumber < 0)
    frameNumber = 0;
  if (getTotalFrames() > 0 && frameNumber >= getTotalFrames())
    frameNumber = getTotalFrames() - 1;

  int diff = frameNumber - getCurrentFrameNumber();
  if (diff >= 0 && diff < GOTOFRAME_SEEK_THRES)
  {
    for (int i = 0; i < diff; i++)
      if (!_videoReader->readNextFrame())
      {
        LOG_ERROR("VideoReader::seek() failed");
        return false;
      }
  }
  else
  {
    if (!_videoReader->seek(frameNumber))
    {
      LOG_ERROR("VideoReader::seek() failed");
      return false;
    }
    if (!_videoReader->readNextFrame())
    {
      LOG_ERROR("Failed to read frame " << frameNumber);
      return false;
    }
  }
  return true;
}

std::string MarkedVideo::getMarkupName() const
{
  return _markupName;
}

void MarkedVideo::setMarkupName(std::string name)
{
  _markupName = name;
}

bool MarkedVideo::dump(int startFrame, int endFrame, const char *format)
{
  int pos = getCurrentFrameNumber();
  if (!goToFrame(startFrame))
    return false;
  bool fError = false;
  for (int i = startFrame; i <= endFrame; i++)
  {
    char buf[4096];
    sprintf(buf, format, i);

    const MinImg *minimg = getCurrentFrame();
    // we use wxImage to save image. It seems that wxImage does not support stride, so manual line copy is needed for 100% compatibility
    wxImage image(minimg->width, minimg->height, false);
    uint8_t *imageData = image.GetData();
    int imageDataStride = minimg->width * 3;
    for (int i = 0; i < minimg->height; i++)
      memcpy(imageData + i * imageDataStride, minimg->pScan0 + i * minimg->stride, imageDataStride);
    if (!image.SaveFile(buf))
    {
      fError = true;
      break;
    }

    if (!getNextFrame())
      break;
  }
  goToFrame(pos);
  return !fError;
}

std::string MarkedVideo::guessMarkupName(const std::string &videoName)
{
  return videoName + ".xml";
}
