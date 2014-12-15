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

#include <minimg.h>

// interface abstract class
class VideoReader
{
public:
  enum Type
  {
    AbstractReader,       ///< never used in a real VideoReader
    FFMpegReader,         ///< movie reader which uses FFMpeg libav* libraries
    DirReader             ///< reader for working with picture files in a directory (NOT IMPLEMENTED)
  };

  VideoReader();
  virtual ~VideoReader() = 0;
  
  /** Open video file
    * @param[in] sourceName name of the video source to be opened
    * @return true Success
    * @return false Failure
    */
  virtual bool open(const char *sourceName) = 0;

  /** Close video file
    * @return true Success
    * @return false Failure
    */
  virtual bool close() = 0;

  /** Move on to the next frame
    * @return pointer to the frame which has been read
    * @return NULL reached the end or an error happened
    * TODO: add a mechanism for distinguishing error and EOF
    */
  virtual const MinImg *readNextFrame() = 0;

  /** Get current frame, i.e. the frame which has been read by the last
    * readNextFrame() call
    * @return pointer to the current frame
    * @return NULL No frames have been read so far
    */
  virtual const MinImg *getCurrentFrame() = 0;

  /** Seek to a given position
    * @param[in] pos Frame number to seek to (frame numbers start from zero)
    * @return true Success
    * @return false Failure
    */
  virtual bool seek(int pos) = 0;

  /** Get current position which is current frame number
    * @return non-negative Current frame number
    * @return negative Failure
  */
  virtual int getPos() = 0;

  virtual bool isOpened() = 0;

  /** Get number of frames in the video
    * @return Total number of frames
    * @return -1 if information is not available
    */
  virtual int getTotalFrames() = 0;

  virtual int getWidth() = 0;
  virtual int getHeight() = 0;

  VideoReader::Type getType()
  {
    return _type;
  }

protected:
  VideoReader::Type _type;

private:
  VideoReader(const VideoReader &)
  {
  }
};

/** Creates an instance of a videoreader of the given type.
  * Should be destroyed with a call to deleteVideoReader() after
  * the instance is no more needed
  * @return NULL Failure
  */
VideoReader *createVideoReader(VideoReader::Type type);

/** Deletes the VideoReader instance previously created by createVideoReader
  * @param[in] videoReader pointer to the instance (if NULL nothing happens)
  */
void deleteVideoReader(VideoReader *videoReader);
