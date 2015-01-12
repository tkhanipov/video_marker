/*
 * Written by Timur Khanipov and released to the public domain,
 * as explained at http://creativecommons.org/publicdomain/zero/1.0/
 */


// this is needed to make stdint.h define the UINT64_C type which
// is used in FFMpeg headers
#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
# define CLEAN__STDC_CONSTANT_MACROS
#endif

#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable:4244 )
#endif

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#ifdef _MSC_VER
# pragma warning( pop )
#endif

#ifdef CLEAN__STDC_CONSTANT_MACROS
# undef __STDC_CONSTANT_MACROS
# undef CLEAN__STDC_CONSTANT_MACROS
#endif

#include <vector>

class FFMpegVideoFile
{
public:
  FFMpegVideoFile();
  ~FFMpegVideoFile();

  /** Open video file
    * @param[in] videoFileName name of the video file to be opened
    * @return true Success
    * @return false Failure
    */
  bool open(const char *videoFileName);

  /** Close video file
    * @return true Success
    * @return false Failure
    */
  bool close();

  /** Move on to the next frame
    * @return pointer to the frame which has been read
    * @return NULL reached the end of file or an error happened
    * TODO: add a mechanism for distinguishing error and EOF
    */
  const AVFrame *readNextFrame();

  /** Converts a frame obtained by readNextFrame() to the internally
    * stored RGB frame.
    * @param[in] pNativeFrame Raw video frame obtained by readNextFrame()
    * @return Pointer to RGB frame
    * @return NULL Failure
    * @see readNextFrame()
    */
  const AVFrame *convertToRGB(const AVFrame *pNativeFrame);

  /** Seek to a given position
    * @param[in] pos Frame number to seek to (frame numbers start from zero)
    * @return true Success
    * @return false Failure
    */
  bool seek(int pos);

  /** Get current position which is current frame number
    * @return non-negative Current frame number
    * @return negative Failure
  */
  int getPos();

  int getWidth();
  int getHeight();

  bool isOpened() const 
  {
    return _isOpened;
  }

  /** Get number of frames in the video
    * @return Total number of frames
    * @return -1 if information is not available
    */
  int getTotalFrames();

  /** Finds the latest key frame which precedes or is equal to frame #pos
    * @return -1 Failed to find key frame
    */
  int findKeyFrame(int pos) const;

  const AVCodecContext *getCodecContext();

  enum LogLevel
  {
    LOG_QUIET = 0,
    LOG_ERROR,
    LOG_DEBUG
  };

  static LogLevel setLogLevel(LogLevel newLogLevel);

private:
  AVFormatContext *_pFormatContext;
  AVCodecContext  *_pCodecContext;
  AVFrame *_pFrame;
  struct SwsContext *_pConverter2RGB;
  int _streamId;
  bool _isOpened;
  int _currentFrame;
  int _totalFrames;

  AVFrame *_pFrameRGB;
  uint8_t *_frameBufferRGB;
  int _frameBufferSizeRGB;

  std::vector<int> _keyIndexTable;

  void _init();
  void _free();
  bool _buildIndexTable();
  int _findKeyIndex(int pos) const;

  static void _log(LogLevel level, const char *fmt, ...);
  static LogLevel g_LogLevel;
};
