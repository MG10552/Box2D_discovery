//#include <SDKDDKVer.h>
#include <tchar.h>
#include <cstdio>

#include <Windows.h>
#include <gl/GL.h>
#include "misc.h"
#include "game.h"

HWND              hwin;
HINSTANCE         hinst;
static ContextGL  gl;
static Input      in;
Game*             game = NULL;

static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  int ret_val = 0;
  
  switch(msg)
  {
    case WM_CLOSE:
      PostQuitMessage(0);
      break;
    case WM_ERASEBKGND:
      ret_val = 1;
      break;
    case WM_LBUTTONDOWN:
      in.ms[0] = Input::Down;
      break;
    case WM_LBUTTONUP:
      in.ms[0] = Input::Up;
      break;
    case WM_RBUTTONDOWN:
      in.ms[1] = Input::Down;
      break;
    case WM_RBUTTONUP:
      in.ms[1] = Input::Up;
      break;
    default:
      ret_val = DefWindowProc(hwnd, msg, wparam, lparam);
      break;
  }
 
  return ret_val;
}
static Input grabInput()
{
  RECT rect;
  GetClientRect(hwin, &rect);

  POINT pt;
  GetCursorPos(&pt);
  ScreenToClient(hwin, &pt);

  in.mx =  pt.x - (rect.right - rect.left)/2;
  in.my = -pt.y + (rect.bottom - rect.top)/2;

  Input inp = in;

  char txt[64];
  sprintf(txt, "Win [%d, %d]", inp.mx, inp.my);
  SetWindowText(hwin, txt);

  //input
  for(int q = 0; q<2; ++q)
  {
    if(in.ms[q] == Input::Down)
      in.ms[q] = Input::Hold;
    else if(in.ms[q] == Input::Up)
      in.ms[q] = Input::Idle;
  }

  BYTE ks[256];
  GetKeyboardState(ks);
  for(int k = 0; k<256; ++k)
  {
    if(in.kb[k] == Input::Idle)
    {
      if(ks[k] & 0x80)
        in.kb[k] = Input::Down;
    }
    else if(in.kb[k] == Input::Down)
    {
      if(ks[k] & 0x80)
        in.kb[k] = Input::Hold;
      else
        in.kb[k] = Input::Up;
    }
    else if(in.kb[k] == Input::Hold)
    {
      if((ks[k] & 0x80) == 0)
        in.kb[k] = Input::Up;
    }
    else if(in.kb[k] == Input::Up)
    {
      in.kb[k] = Input::Idle;
    }
  };

  return inp;
}
int main(int argc, char* argv[])
{
  //create window - 800x600 
  hinst = GetModuleHandle(NULL);
  WNDCLASSEX wc = { sizeof(WNDCLASSEX), NULL, &WinProc, 0, 0, hinst, LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW), NULL, NULL, "Win", NULL };
  RegisterClassEx(&wc);
  RECT rect = { 0, 0, 800, 600 };
  AdjustWindowRectEx(&rect, WS_OVERLAPPED | WS_CAPTION, FALSE, 0);
  hwin = CreateWindowEx(0, "Win", "Window", WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hinst, NULL);
  GetClientRect(hwin, &rect);

  //gl context
  gl.create(hwin);

  //create game object 
  game = new Game();
  game->create(rect.right - rect.left, rect.bottom - rect.top);

  //message loop
  MSG msg;
  memset(&msg, 0, sizeof(MSG));

  while(msg.message != WM_QUIT)
  {
    if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      //input
      Input inp = grabInput();

      //your game logic
      game->process(inp);

      //swap gl buffers
      gl.swap();
    }
  }

  //clean up game
  game->destroy();
  delete game;

  //clean up gl context
  gl.destroy();

	return 1;
}


