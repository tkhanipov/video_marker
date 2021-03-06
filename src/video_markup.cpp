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

#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <pugixml.hpp>
#include "video_markup.h"

namespace video_markup {

void Markup::clear()
{
  startFrame = endFrame = -1;
  std::vector<Interval>::clear();
}

bool Markup::save(const char *fileName) const
{
  pugi::xml_document doc;
  pugi::xml_node root = doc.append_child("video_markup");
  if (startFrame > 0)
    root.append_attribute("start_frame") = startFrame;
  if (endFrame > 0)
    root.append_attribute("end_frame") = endFrame;
  for (const_iterator it = begin(); it != end(); ++it)
  {
    pugi::xml_node i = root.append_child("interval");
    i.append_attribute("start") = it->start;
    i.append_attribute("end") = it->end;
    i.append_attribute("type") = it->type;
    i.append_attribute("y_border") = it->y_border;
    if (!it->comment.empty())
      i.append_child("comment").text() = it->comment.c_str();
    for (Interval::Labels::const_iterator label = it->labels.begin(); label != it->labels.end(); ++label)
      i.append_child("label").text() = label->c_str();
  }
  return doc.save_file(fileName);
}

bool Markup::load(const char *fileName)
{
  clear();
  startFrame = endFrame = -1;
  pugi::xml_document doc;
  pugi::xml_parse_result res = doc.load_file(fileName);
  if (!res)
    return false;
  pugi::xml_node root = doc.document_element();
  if (strcmp(root.name(), "video_markup"))
    return false;
  startFrame = root.attribute("start_frame").as_int(-1);
  endFrame = root.attribute("end_frame").as_int(-1);
  for (pugi::xml_node i = root.child("interval"); i; i = i.next_sibling("interval"))
  {
    if (!i.attribute("start") || !i.attribute("end") || !i.attribute("type") || !i.attribute("y_border"))
      goto failure;
    Interval interval;
    interval.start    = i.attribute("start").as_int();
    interval.end      = i.attribute("end").as_int();
    interval.type     = i.attribute("type").as_int();
    interval.y_border = i.attribute("y_border").as_int();
    interval.comment = i.child("comment").text().as_string();
    for (pugi::xml_node l = i.child("label"); l; l = l.next_sibling("label"))
      interval.labels.insert(l.text().as_string());
    if (push(interval) < 0)
      goto failure;
  }
  return true;
failure:
  clear();
  startFrame = endFrame = -1;
  return false;
}

int Markup::find(int frame) const
{
  for (Markup::const_iterator it = begin(); it != end(); ++it)
    if (it->contains(frame))
      return it - begin();
  return -1;
}

int Markup::findNextClosest(int frame) const
{
  int interval = find(frame);
  if (interval >= 0)
    return interval < (int)size()-1 ?  interval+1   : -1;    // is it the last interval? if so, return -1

  for (Markup::const_iterator it = begin(); it != end(); ++it)
    if (it->start > frame)
      return it - begin();
  return -1;
}

int Markup::findPrevClosest(int frame) const
{
  int interval = find(frame);
  if (interval >= 0)
    return interval>0 ?  interval-1   : -1;    // is it the first interval? if so, return -1

  for (int i = size()-1; i >= 0; i--)
    if (at(i).start < frame)
      return i;
  return -1;
}

int Markup::findClosest(int frame) const
{
  int i = find(frame);
  if (i >= 0)
    return i;

  int iNext = findNextClosest(frame);
  int iPrev = findPrevClosest(frame);
  if (iNext < 0)
    return iPrev;
  if (iPrev < 0)
    return iNext;
  if (at(iNext).start - frame < frame - at(iPrev).end)
    return iNext;
  else
    return iPrev;
}

bool Markup::setStartFrame(int _startFrame)
{
  // we must check here that no existing interval will become an outsider
  if (_startFrame < 0)
  {
    startFrame = -1;
    return true;
  }
  else
  {
    for (Markup::const_iterator it = begin(); it != end(); ++it)
      if (it->start < _startFrame)
        return false;
    startFrame = _startFrame;
    return true;
  }
}

bool Markup::setEndFrame(int _endFrame)
{
  // we must check here that no existing interval will become an outsider
  if (_endFrame < 0)
  {
    endFrame = -1;
    return true;
  }
  else
  {
    for (Markup::const_iterator it = begin(); it != end(); ++it)
      if (it->end > _endFrame)
        return false;
    endFrame = _endFrame;
    return true;
  }
}

int Markup::push(const Interval &interval)
{
  if (interval.start >= interval.end)  // perverted interval
    return -1;

  // interval not within movie borders
  if (interval.start < startFrame || (endFrame >= 0 && interval.end > endFrame))
    return -1;

  Markup::iterator it;
  for (it = begin(); it != end(); ++it)
    if (interval.start < it->start)
      break;

  if (it != end() && interval.end >= it->start)   // overlapping intervals
    return -1;

  if (it != begin() && interval.start <= (it-1)->end)   // overlapping intervals
    return -1;

  it = insert(it, interval);
  return it - begin();
}

bool Markup::erase(int id)
{
  if (id >= 0 && id < (int) size())
  {
    std::vector<Interval>::erase(std::vector<Interval>::begin() + id);
    return true;
  }
  else
    return false;
}

} // namespace video_markup
