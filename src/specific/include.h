#ifndef INCLUDE_H
#define INCLUDE_H

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <string.h>

#define Zero(thing) memset(&(thing), 0, sizeof(thing))
#define ArraySize(a) (sizeof(a) / sizeof((a)[0]))

extern IDirect3DDevice2 *G_Direct3DDevice;

#endif
