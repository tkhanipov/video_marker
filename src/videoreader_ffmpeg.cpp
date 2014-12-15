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

#include "ffmpegvideo.h"
#include "videoreader_ffmpeg.h"

#include <cstring>

VideoReaderFFMpeg::VideoReaderFFMpeg()
{
  _type = FFMpegReader;
  _pFFMpegVideoFile = new FFMpegVideoFile;
  memset(&_minimg, 0, sizeof(_minimg));
}

VideoReaderFFMpeg::~VideoReaderFFMpeg()
{
  delete _pFFMpegVideoFile;
}

bool VideoReaderFFMpeg::open(const char *sourceName)
{
  if (!_pFFMpegVideoFile->open(sourceName))
    return false;
  _minimg.width = _pFFMpegVideoFile->getWidth();
  _minimg.height = _pFFMpegVideoFile->getHeight();
  _minimg.format = FMT_UINT;
  _minimg.channels = 3;
  _minimg.channelDepth = 1;
  return true;
}

bool VideoReaderFFMpeg::close()
{
  memset(&_minimg, 0, sizeof(_minimg));
  return _pFFMpegVideoFile->close();
}

const MinImg *VideoReaderFFMpeg::readNextFrame()
{
  const AVFrame *pRawFrame = _pFFMpegVideoFile->readNextFrame();
  if (!pRawFrame)
    return 0;
  const AVFrame *pFrameRGB = _pFFMpegVideoFile->convertToRGB(pRawFrame);
  if (!pFrameRGB)
    return 0;
  _minimg.stride = pFrameRGB->linesize[0];
  _minimg.pScan0 = pFrameRGB->data[0];
  return &_minimg;
}

const MinImg *VideoReaderFFMpeg::getCurrentFrame()
{
  if (_minimg.pScan0)
    return &_minimg;
  else
    return 0;
}

bool VideoReaderFFMpeg::seek(int pos)
{
  return _pFFMpegVideoFile->seek(pos);
}

int VideoReaderFFMpeg::getPos()
{
  return _pFFMpegVideoFile->getPos();
}

bool VideoReaderFFMpeg::isOpened()
{
  return _pFFMpegVideoFile->isOpened();
}

int VideoReaderFFMpeg::getTotalFrames()
{
  return _pFFMpegVideoFile->getTotalFrames();
}

int VideoReaderFFMpeg::getWidth()
{
  return _pFFMpegVideoFile->getWidth();
}

int VideoReaderFFMpeg::getHeight()
{
  return _pFFMpegVideoFile->getHeight();
}
