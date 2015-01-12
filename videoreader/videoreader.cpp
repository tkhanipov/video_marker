/*
 * Written by Timur Khanipov and released to the public domain,
 * as explained at http://creativecommons.org/publicdomain/zero/1.0/
*/

#include "videoreader.h"
#include "videoreader_ffmpeg.h"

VideoReader::VideoReader()
: _type(AbstractReader)
{
}

VideoReader::~VideoReader()
{
}

VideoReader *createVideoReader(VideoReader::Type type)
{
  switch (type)
  {
    case VideoReader::FFMpegReader:
      return new VideoReaderFFMpeg;
    default:
      return 0;
  }
  return 0;
}

void deleteVideoReader(VideoReader *videoReader)
{
  if (videoReader)
    delete videoReader;
}
