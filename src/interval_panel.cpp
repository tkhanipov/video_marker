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

#include "interval_panel.h"
#include <wx/wx.h>
#include <ctime>

IntervalPanel::IntervalPanel(Frame *parent)
  : wxPanel(parent)
  , frame(parent)
  , interval(0)
{
  wxBoxSizer *vertSizer = new wxBoxSizer(wxVERTICAL);
  vertSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Labels")), 1, wxALIGN_CENTER);
  labels = new wxListBox(this, wxID_ANY);
  vertSizer->Add(labels, 15, wxEXPAND);
  vertSizer->AddSpacer(20);
  vertSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Comment")), 1, wxALIGN_CENTER);
  comment = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);
  vertSizer->Add(comment, 15, wxEXPAND);

  SetSizerAndFit(vertSizer); 
  OnUpdateInterval(true);
}

void IntervalPanel::OnUpdateInterval(bool force)
{
  const video_markup::Interval *currentInterval = frame->markedVideo.getCurrentInterval();
  if (currentInterval != interval || force)
  {
    interval = currentInterval;
    labels->Clear();
    comment->Clear();
    if (interval)
    {
      comment->SetValue(interval->comment);
      int i = 0;
      for (std::set<std::string>::const_iterator it = interval->labels.begin(); it != interval->labels.end(); ++it, ++i)
        labels->Insert(*it, i);
    }
  }
  Enable(interval != 0);
  Update();
}
