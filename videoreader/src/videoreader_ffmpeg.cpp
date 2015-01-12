/*
 * Written by Timur Khanipov and released to the public domain,
 * as explained at http://creativecommons.org/publicdomain/zero/1.0/
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
