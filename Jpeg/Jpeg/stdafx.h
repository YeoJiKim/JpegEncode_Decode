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
#include "libavcodec/avcodec.h"   //����루����Ҫ�Ŀ⣩
#include "libavformat/avformat.h"  //��װ��ʽ����
#include "libswscale/swscale.h" //��Ƶ�������ݸ�ʽת��
#include "libswresample/swresample.h"

#include "libavutil/avutil.h"   //���߿�
};
// TODO: reference additional headers your program requires here

#define MAXSIZE 1024
#define __STDC_CONSTANT_MACROS
#define MAX_AUDIO_FRAME_SIZE 192000