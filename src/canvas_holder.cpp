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

#include "canvas_holder.h"
#include <wx/config.h>

BEGIN_EVENT_TABLE(CanvasHolder, wxScrolledWindow)
  EVT_SCROLLWIN_THUMBRELEASE(CanvasHolder::OnScrollEnd)
END_EVENT_TABLE()

void CanvasHolder::OnBitmapUpdate(bool fullUpdate)
{
  if (fullUpdate)
  {
    Scroll(0, 0);
    SetVirtualSize(canvas->bitmap.GetWidth(), canvas->bitmap.GetHeight());
    canvas->Resize();
    SetScrollRate(1, 1);
  }
  Refresh();
}

CanvasHolder::CanvasHolder(Frame *ownerFrame)
:wxScrolledWindow(ownerFrame, wxID_ANY, wxDefaultPosition, wxSize(600, 400), wxHSCROLL | wxVSCROLL)
{
  wxColour bgColour = wxConfigBase::Get()->Read(wxT("videoBackgroundColour"), wxColour(0, 0, 0).GetAsString());
  wxConfigBase::Get()->Write(wxT("videoBackgroundColour"), bgColour.GetAsString(wxC2S_HTML_SYNTAX));

  owner = ownerFrame;
  SetBackgroundColour(bgColour);
  canvas = new Canvas(this, owner);
  SetMinSize(wxSize(600, 400));
}

void CanvasHolder::OnScrollEnd(wxScrollWinEvent  &event)
{
  wxScrolledWindow::OnScroll(event);
  owner->Synchronize();
}
