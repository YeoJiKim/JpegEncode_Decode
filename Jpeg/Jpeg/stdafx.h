// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "getBMP.h"

#include <stdio.h>
#include <tchar.h>
#include <math.h>
#include <string>
#include <windows.h>
#include <iostream>
using namespace std;

extern "C"
{
#include "libavcodec/avcodec.h"   //编解码（最重要的库）
#include "libavformat/avformat.h"  //封装格式处理
#include "libswscale/swscale.h" //视频像素数据格式转换
#include "libswresample/swresample.h"

#include "libavutil/avutil.h"   //工具库
};
// TODO: reference additional headers your program requires here

#define MAXSIZE 1024
#define __STDC_CONSTANT_MACROS
#define MAX_AUDIO_FRAME_SIZE 192000