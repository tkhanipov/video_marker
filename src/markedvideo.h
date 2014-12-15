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

#include <string>
#include <vector>
#include <list>
#include <map>
#include "video_markup.h"

class VideoReader;

class MarkedVideo
{
public:
  MarkedVideo();
  ~MarkedVideo();

  bool loadVideo(const std::string &name, const char *markupName = 0);
  void closeVideo();
  std::string getVideoName() const
  {
    return _videoName;
  }

  const MinImg *getCurrentFrame();
  const MinImg *getNextFrame();
  bool goToFrame(int frameNumber);
  int getCurrentFrameNumber();
  int getTotalFrames();

  bool loadMarkup(const std::string &name);
  bool saveMarkup(const std::string &name) const;
  std::string getMarkupName() const;
  void setMarkupName(std::string name);

  static std::string guessMarkupName(const std::string &videoName);

  bool gotoInterval(int id);
  video_markup::Interval *getCurrentInterval();
  int getTotalIntervals();
  bool pushInterval(const video_markup::Interval *);
  bool deleteCurrentInterval();
  int getCurrentIntervalId();
  bool moveToNextInterval();
  bool moveToPrevInterval();
  bool moveToIntervalEnd();
  bool moveToIntervalStart();

  bool setStartFrame(int startFrame);
  bool setEndFrame(int endFrame);
  
  bool frameWithinBorders(int frame) const;

  bool dump(int startFrame, int endFrame, const char *format);

  void setAutoLoadMarkup(bool yesOrNo)
  {
    _autoLoadMarkup_flag = yesOrNo;
  }

  bool getAutoLoadMarkup() const
  {
    return _autoLoadMarkup_flag;
  }

private:
  MarkedVideo(const MarkedVideo &);
  MarkedVideo &operator= (const MarkedVideo &);

  bool _autoLoadMarkup_flag;
  int _frameNumber;
  VideoReader *_videoReader;
  video_markup::Markup _markup;
  std::string _videoName;
  std::string _markupName;
};
