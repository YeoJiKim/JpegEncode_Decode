#include "stdafx.h"


static int av_create_bmp(char* filename, AVFrame *pRGBBuffer, int width, int height, int bpp)
{
	BITMAPFILEHEADER bmpheader;
	BITMAPINFO bmpinfo;
	FILE *fp;

	fp = fopen(filename, "wb");
	if (!fp)return -1;

	bmpheader.bfType = ('M' << 8) | 'B';
	bmpheader.bfReserved1 = 0;
	bmpheader.bfReserved2 = 0;
	bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpheader.bfSize = bmpheader.bfOffBits + width*height*bpp / 8;
	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biWidth = width;
	bmpinfo.bmiHeader.biHeight = height;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biBitCount = 24;
	bmpinfo.bmiHeader.biCompression = BI_RGB;
	bmpinfo.bmiHeader.biSizeImage = 0;
	bmpinfo.bmiHeader.biXPelsPerMeter = 100;
	bmpinfo.bmiHeader.biYPelsPerMeter = 100;
	bmpinfo.bmiHeader.biClrUsed = 0;
	bmpinfo.bmiHeader.biClrImportant = 0;
	fwrite(&bmpheader, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite(&bmpinfo.bmiHeader, sizeof(BITMAPINFOHEADER), 1, fp);
	for (int y = height; y > 0; y --)
		fwrite(pRGBBuffer->data[0] + y*pRGBBuffer->linesize[0], 1, width * 3, fp);
	fclose(fp);
	return 0;
}

int getBMP() {
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;

	AVFrame	*pFrame, *pFrameRGB;
	uint8_t *out_buffer;
	AVPacket *packet;

	int ret, got_picture;
	struct SwsContext *img_convert_ctx;
	char filepath[] = "Titanic.ts";

	int frame_cnt;

	av_register_all(); //注册所有组件
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	//打开输入视频文件
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return -1;
	}
	//获取视频文件信息
	if (avformat_find_stream_info(pFormatCtx, NULL)<0) {
		printf("Couldn't find stream information.\n");
		return -1;
	}

	videoindex = -1;

	//遍历输入视频
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		////获取视频流
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			continue;
		}
	}

	if (videoindex == -1) {
		printf("Didn't find a video stream.\n");
		return -1;
	}

	pCodecCtx = pFormatCtx->streams[videoindex]->codec;

	//查找解码器
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if (pCodec == NULL) {
		printf("Codec not found.\n");
		return -1;
	}

	//打开解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
		printf("Could not open codec.\n");
		return -1;
	}

	//为每一帧图像分配内存
	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	if (pFrameRGB == NULL)
		return -1;

	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height));//设置像素格式和图像宽、高
	avpicture_fill((AVPicture *)pFrameRGB, out_buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	//设置图像转换上下文
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	av_init_packet(packet);
	//开始解码，解码图像并且保存
	frame_cnt = 0;
	//从输入文件读取一帧压缩数据
	while (av_read_frame(pFormatCtx, packet) >= 0) {
		if (packet->stream_index == videoindex) {
			//解码开始
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0) {
				printf("Decode Error.\n");
				return -1;
			}
			if (got_picture > 0 && pFrame->key_frame == 1) {
				//解码一帧成功  
				sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
				// 保存到磁盘   
				char pic[200];
				sprintf(pic, "KeyFrame%d.bmp", frame_cnt + 1);
				frame_cnt++;
				av_create_bmp(pic, pFrameRGB, pCodecCtx->width, pCodecCtx->height, 24);
				break;
			}

		}
		
		av_free_packet(packet);
	
	}
	
	sws_freeContext(img_convert_ctx);

	av_frame_free(&pFrameRGB);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);   //关闭解码器
	avformat_close_input(&pFormatCtx);  //关闭输入视频文件
	
	return 0;
}