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

#include "canvas.h"
#include "frame.h"

BEGIN_EVENT_TABLE(Canvas, wxWindow)
  EVT_PAINT(Canvas::OnPaint)
  EVT_MOTION(Canvas::OnMotion)
  EVT_LEAVE_WINDOW(Canvas::OnLeave)
  EVT_ENTER_WINDOW(Canvas::OnEnter)
  EVT_LEFT_DOWN(Canvas::OnLeftDown)
END_EVENT_TABLE()

void Canvas::OnLeftDown(wxMouseEvent &event)
{
  frame->OnLeftDown(event);
}

void Canvas::OnMotion(wxMouseEvent &event)
{
  if (!bitmap.IsOk())
    return;
  wxPoint point(event.GetPosition().x, event.GetPosition().y);
  
  wxString str;
  str.Printf(wxT("%d:%d"), point.x, point.y);
  frame->SetStatusText(str, StatusMouse);
}

void Canvas::OnLeave(wxMouseEvent &event)
{
  if (!bitmap.IsOk())
    return;
  frame->SetStatusText(wxEmptyString, StatusMouse);
}

void Canvas::OnEnter(wxMouseEvent &event)
{
  OnMotion(event);
}

void Canvas::OnPaint(wxPaintEvent &event)
{
  wxPaintDC dc(this);
  
  if (!bitmap.IsOk())
    return;
  
  wxMemoryDC memDC(bitmap);
  dc.Blit(0, 0, bitmap.GetWidth(), bitmap.GetHeight(), &memDC, 0, 0);
}

void Canvas::Resize()
{
  if (bitmap.IsOk())
    SetSize(bitmap.GetWidth(), bitmap.GetHeight());
}

Canvas::Canvas(wxWindow *parent, Frame *_frame)
    :wxWindow(parent, wxID_ANY),
    frame(_frame)
    , bitmap(_frame->pureBitmap)
{
  Resize();
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}
