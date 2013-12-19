#pragma once

#include <GL/glew.h>
#include <GL/wglew.h>
//#include <GL/glxew.h>

//#include <GL/freeglut.h>

#ifdef _WIN32
#   include <winsock2.h>
#   include <windows.h>
#   pragma comment( lib, "user32.lib" )
#   pragma comment( lib, "advapi32.lib" )
#   pragma comment( lib, "gdi32.lib" )
#   pragma comment( lib, "winmm.lib" )
#   pragma comment( lib, "opengl32.lib" )
#   pragma comment( lib, "glu32.lib" )
#endif

#include <GL/gl.h>
#include <GL/glu.h>
