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

#include <wx/cmdline.h>
#include "app.h"
#include "frame.h"

static const wxCmdLineEntryDesc g_cmdLineDesc[] =
{
  { wxCMD_LINE_PARAM, 0, 0, wxT("video file to open"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
  { wxCMD_LINE_OPTION, 0, wxT("markup"), wxT("Custom markup to be loaded"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
  { wxCMD_LINE_OPTION, 0, wxT("frame"), wxT("Seek to frame"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL },
  { wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("show help") },
  { wxCMD_LINE_NONE }
};

bool App::OnInit()
{
  if (!wxApp::OnInit())
    return false;

  ::wxInitAllImageHandlers();
  Frame *frame = new Frame(m_cmdLineArguments);
  frame->Show(true);
  SetTopWindow(frame);
  frame->Maximize();
  return true;
}

void App::OnInitCmdLine(wxCmdLineParser &parser)
{
  parser.SetDesc(g_cmdLineDesc);
  parser.SetSwitchChars(wxT("-"));
}

bool App::OnCmdLineParsed(wxCmdLineParser &parser)
{
  if (parser.Found(wxT("h")))
  {
    parser.Usage();
    return true;
  }
  if (parser.GetParamCount() != 0)
    m_cmdLineArguments.videoFile = parser.GetParam(0);
  parser.Found(wxT("markup"), &m_cmdLineArguments.markup);
  parser.Found(wxT("frame"), &m_cmdLineArguments.startFrame);
  return true;
}
