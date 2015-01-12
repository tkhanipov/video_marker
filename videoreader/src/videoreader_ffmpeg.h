/*
 * Written by Timur Khanipov and released to the public domain,
 * as explained at http://creativecommons.org/publicdomain/zero/1.0/
*/


#pragma once
#include "videoreader.h"

class FFMpegVideoFile;

class VideoReaderFFMpeg: public VideoReader
{
public:
  VideoReaderFFMpeg();
  virtual ~VideoReaderFFMpeg();
  virtual bool open(const char *sourceName);
  virtual bool close();
  virtual const MinImg *readNextFrame();
  virtual const MinImg *getCurrentFrame();
  virtual bool seek(int pos);
  virtual int getPos();
  virtual bool isOpened();
  virtual int getTotalFrames();
  virtual int getWidth();
  virtual int getHeight();
private:
  FFMpegVideoFile *_pFFMpegVideoFile;
  MinImg _minimg;
};
