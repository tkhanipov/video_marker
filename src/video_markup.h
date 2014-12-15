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

#include <vector>
#include <string>
#include <cstring>
#include <set>
#include <string>

namespace video_markup {

struct Interval
{
  Interval()
  : start(-1),
    end(-1),
    type(-1),
    y_border(-1)
  { }

  int start;
  int end;
  int type;

  int y_border;
  std::string comment;

  std::set<std::string> labels;

  bool hasLabel(std::string const& label) const { return labels.find(label) != labels.end(); }

  bool contains(int frame) const
    { return frame >= start && frame <= end; }

};

class Markup: private std::vector<Interval>
{
private:
  /* If we need to process only a part of the movie, we may use startFrame and endFrame.
     They mark the first and the last frames of the part to be processed respectively.
     If negative, they are ignored (which is equal to startFrame = 0, endFrame = {index of
     the last frame}
  */
  int         startFrame;     ///< first frame of the video chunk to be processed or <0 of not specified
  int         endFrame;       ///< last  frame of the video chunk to be processed or <0 if not specified

public:
  using std::vector<Interval>::size;
  using std::vector<Interval>::size_type;
  using std::vector<Interval>::operator[];
  using std::vector<Interval>::empty;
  using std::vector<Interval>::at;
  using std::vector<Interval>::push_back;
  using std::vector<Interval>::pop_back;

  Markup()
  : startFrame(-1),
    endFrame(-1)
  {
  }

  void clear();

  bool save(const char *fileName) const;
  bool load(const char *fileName);

  int push(const Interval &interval);
  int find(int frame) const;

  int findNextClosest(int frame) const;
  int findPrevClosest(int frame) const;
  int findClosest(int frame) const;

  bool isIntervalFrame(int frame) const
  {
    int closestIndex = findClosest(frame);
    if (closestIndex >= 0)
      return at(closestIndex).contains(frame);
    return false;
  }


  bool erase(int id);

  int getStartFrame() const
  {
    return startFrame;
  }

  int getEndFrame() const
  {
    return endFrame;
  }

  /** @return true Success
    * @return false Failure
    */
  bool setStartFrame(int _startFrame);

  /** @return true Success
    * @return false Failure
    */
  bool setEndFrame(int _endFrame);
};

} // namespace video_markup
