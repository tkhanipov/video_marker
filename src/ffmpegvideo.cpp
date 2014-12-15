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
#include <cstdarg>
#include <cassert>

#include "ffmpegvideo.h"

#ifdef VIDEOREADER_THREAD_SAFE
# include <boost/thread.hpp>
static boost::recursive_mutex g_criticalSection;
# define CRITICAL_SECTION   boost::lock_guard<boost::recursive_mutex> lock(g_criticalSection);
#else
# define CRITICAL_SECTION
#endif

namespace {
  // AV_Initializer is needed to call av_register_all() prior to usage of libav*-functions.
  class AV_Initializer
  {
  public:
    AV_Initializer()
    {
      av_register_all();
  #ifdef NDEBUG
      av_log_set_level(AV_LOG_QUIET);
  #endif
    }
  };

  static AV_Initializer g_avInitializer;
}


#ifdef NDEBUG
  FFMpegVideoFile::LogLevel FFMpegVideoFile::g_LogLevel = FFMpegVideoFile::LOG_QUIET;
#else
  FFMpegVideoFile::LogLevel FFMpegVideoFile::g_LogLevel = FFMpegVideoFile::LOG_DEBUG;
#endif

FFMpegVideoFile::FFMpegVideoFile()
{
  _init();
}

FFMpegVideoFile::~FFMpegVideoFile()
{
  if (isOpened())
    close();
}

void FFMpegVideoFile::_init()
{
  _pFormatContext = 0;
  _pCodecContext = 0;
  _pFrame = 0;
  _pConverter2RGB = 0;
  _streamId = -1;
  _isOpened = false;
  _currentFrame = -1;
  _totalFrames = -1;

  _pFrameRGB = 0;
  _frameBufferSizeRGB = 0;
  _frameBufferRGB = 0;
}

void FFMpegVideoFile::_free()
{
  CRITICAL_SECTION
  _keyIndexTable.clear();
  if (_pCodecContext)
    avcodec_close(_pCodecContext);
  if (_pFormatContext)
    av_close_input_file(_pFormatContext);
  if (_pFrame)
    av_free(_pFrame);
  if (_frameBufferRGB)
    av_free(_frameBufferRGB);
  if (_pFrameRGB)
    av_free(_pFrameRGB);

  sws_freeContext(_pConverter2RGB);

  _init();
}

bool FFMpegVideoFile::open(const char *videoFileName)
{
  CRITICAL_SECTION
  assert(videoFileName);

  try
  {
    if (isOpened())
      throw "Already opened";
    if (av_open_input_file( &_pFormatContext, videoFileName, 0, 0, 0 ) != 0)
      throw "Cannot open file";
    if (av_find_stream_info(_pFormatContext) < 0)
      throw "av_find_stream_info() failed";

    _streamId = -1;
    for (int i = 0; i < (int) _pFormatContext->nb_streams; i++)
      if (_pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
      {
        _streamId = i;
        break;
      }
    if (_streamId == -1)
      throw "no video streams found";
    
    _pCodecContext = _pFormatContext->streams[ _streamId ]->codec;
    AVCodec *pCodec = avcodec_find_decoder(_pCodecContext->codec_id);
    if (!pCodec)
      throw "unknown codec";
    if (avcodec_open( _pCodecContext, pCodec ) < 0)
      throw "cannot open codec";

    _pFrame = avcodec_alloc_frame();
    if (!_pFrame)
      throw "out of memory";

    _pFrameRGB = avcodec_alloc_frame();
    if (!_pFrameRGB)
      throw "out of memory";
    _frameBufferSizeRGB = avpicture_get_size(PIX_FMT_RGB24, _pCodecContext->width, _pCodecContext->height);
    if (_frameBufferSizeRGB <= 0)
      throw "avpicture_get_size() failed";
    _frameBufferRGB = (uint8_t *) av_malloc(_frameBufferSizeRGB);
    if (!_frameBufferRGB)
      throw "out of memory";
    avpicture_fill((AVPicture *) _pFrameRGB, _frameBufferRGB, PIX_FMT_RGB24, _pCodecContext->width, _pCodecContext->height);

    if (!_buildIndexTable())
      throw "failed to build index table";

    _isOpened = true;
    _currentFrame = 0;
  }
  catch (const char *errorMsg)
  {
    assert(errorMsg);
    _log(LOG_ERROR, errorMsg);
    _free();
    return false;
  }
  return true;
}

bool FFMpegVideoFile::close()
{
  if (!isOpened())
    return false;
  _free();
  return true;
}

const AVFrame *FFMpegVideoFile::readNextFrame()
{
  if (!isOpened())
    return 0;

  AVPacket packet;
  while (true)
  {
    if (av_read_frame(_pFormatContext, &packet) < 0)
      return 0;
    if (packet.stream_index == _streamId)
      break;
    av_free_packet(&packet);
  }

  if (_currentFrame == 0 && !(packet.flags & AV_PKT_FLAG_KEY))
  {
    _log(LOG_ERROR, "frame #0 is not key frame");
    close();
    return 0;
  }

  int frameFinished = 0;
  int res = avcodec_decode_video2(_pCodecContext, _pFrame, &frameFinished, &packet);
  av_free_packet(&packet);
  if (res < 0)
  {
    _log(LOG_ERROR, "avcodec_decode_video2() failed");
    close();
    return 0;
  }
  if (!frameFinished)
  {
    _log(LOG_ERROR, "incomplete frame detected");
    close();
    return 0;
  }
  _currentFrame++;
  return _pFrame;
}

int FFMpegVideoFile::getPos()
{
  if (isOpened())
    return _currentFrame;
  else
    return -1;
}

bool FFMpegVideoFile::_buildIndexTable()
{
  AVStream *stream = _pFormatContext->streams[_streamId];
  if (stream->nb_index_entries > 0)
  {
    if (!(stream->index_entries[0].flags & AVINDEX_KEYFRAME))
    {
      _log(LOG_ERROR, "First frame is not key frame");
      return false;
    }
    for (int i = 0; i < stream->nb_index_entries; i++)
    {
      if (stream->index_entries[i].flags & AVINDEX_KEYFRAME)
        _keyIndexTable.push_back(i);
    }
    return true;
  }

  _log(LOG_DEBUG, "Building index manually...\n");
  int i = 0;
  AVPacket packet;
  while (av_read_frame(_pFormatContext, &packet) >= 0)
  {
    if (packet.stream_index != _streamId)
      continue;
    bool isKey = packet.flags & AV_PKT_FLAG_KEY;
    av_free_packet(&packet);

    if (i == 0 && !isKey)
    {
      _log(LOG_ERROR, "First frame is not key frame");
      return false;
    }
    if (isKey)
      _keyIndexTable.push_back(i);
    i++;
  }
  _totalFrames = i;

  _log(LOG_DEBUG, "Key frame index has been built (%d elements)", _keyIndexTable.size());
  if (!_keyIndexTable.size())
  {
    _log(LOG_ERROR, "No key frames found.");
    return false;
  }
  if (av_seek_frame(_pFormatContext, _streamId, 0, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD) < 0)
  {
    _log(LOG_ERROR, "av_seek_frame() failed");
    return false;
  }
  avcodec_flush_buffers(_pCodecContext);
  return true;
}

bool FFMpegVideoFile::seek(int pos)
{
  if (!isOpened() || pos < 0 || (_totalFrames >= 0 && pos >= _totalFrames))
    return false;

  if (pos == _currentFrame)
    return true;

  int keyFrameIndex = _findKeyIndex(pos);
  int keyFrame = _keyIndexTable[ keyFrameIndex ];
  int dir = (keyFrame > _currentFrame) ? 0 : AVSEEK_FLAG_BACKWARD;

  _log(LOG_DEBUG, "seeking %s to keyframe %d", dir & AVSEEK_FLAG_BACKWARD ? "backward" : "forward", keyFrame);
  if (av_seek_frame(_pFormatContext, _streamId, keyFrame, AVSEEK_FLAG_FRAME | dir) < 0)
  {
    _log(LOG_ERROR, "av_seek_frame() failed");
    return false;
  }
  avcodec_flush_buffers(_pCodecContext);
  _currentFrame = keyFrame;

  _log(LOG_DEBUG, "finally seeking to position %d", pos);
  while (_currentFrame < pos)
  {
    if (!readNextFrame())
      break;
  }  
  return true;
}

int FFMpegVideoFile::_findKeyIndex(int pos) const
{
  assert(pos >= 0);
  if (!isOpened() || !_keyIndexTable.size())
    return -1;

  // FIXME: use binary search
  int keyIndex = -1;
  for (int i = 0; i < (int) _keyIndexTable.size(); i++)
  {
    if (_keyIndexTable[i] > pos)
      break;
    keyIndex = i;
  }
  return keyIndex;
}

FFMpegVideoFile::LogLevel FFMpegVideoFile::setLogLevel(FFMpegVideoFile::LogLevel newLevel)
{
  CRITICAL_SECTION
  LogLevel oldLevel = g_LogLevel;
  g_LogLevel = newLevel;
  return oldLevel;
}

void FFMpegVideoFile::_log(FFMpegVideoFile::LogLevel level, const char *fmt, ...)
{
  CRITICAL_SECTION
  if (level > g_LogLevel)
    return;
  const char *levelName = 0;
  switch (level)
  {
    case LOG_ERROR: levelName = "error";  break;
    case LOG_DEBUG: levelName = "debug";  break;
    default:        levelName = "???";    break;
  }
  fprintf(stderr, "%s: ", levelName);

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n");
}

int FFMpegVideoFile::getWidth()
{
  if (isOpened())
    return _pCodecContext->width;
  else
    return -1;
}

int FFMpegVideoFile::getHeight()
{
  if (isOpened())
    return _pCodecContext->height;
  else
    return -1;
}

const AVCodecContext *FFMpegVideoFile::getCodecContext()
{
  return _pCodecContext;
}

const AVFrame *FFMpegVideoFile::convertToRGB(const AVFrame *pNativeFrame)
{
  if (!isOpened() || !pNativeFrame)
    return 0;
  if (!_pConverter2RGB)
  {
    _pConverter2RGB = sws_getContext( getWidth(), getHeight(), getCodecContext()->pix_fmt,
                                      getWidth(), getHeight(), PIX_FMT_RGB24, SWS_BICUBIC, 0, 0, 0);
    if (!_pConverter2RGB)
    {
      _log(LOG_ERROR, "sws_getContext() failed");
      return 0;
    }
  }
  sws_scale(_pConverter2RGB, pNativeFrame->data, pNativeFrame->linesize, 0, _pCodecContext->height,
            _pFrameRGB->data, _pFrameRGB->linesize);
  return _pFrameRGB;
}

int FFMpegVideoFile::getTotalFrames()
{
  if (!isOpened())
    return -1;
  int totalFrames;
  if ((totalFrames = (int) _pFormatContext->streams[_streamId]->nb_frames) > 0)
    return totalFrames;
  else
    return -1;
}

int FFMpegVideoFile::findKeyFrame(int pos) const
{
  if (!isOpened())
    return -1;
  int idx = _findKeyIndex(pos);
  if (idx < 0)
    return -1;
  else
    return _keyIndexTable[ idx ];
}
