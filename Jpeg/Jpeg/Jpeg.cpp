#include "stdafx.h"


#include "stdafx.h"


#define M 8
#define N 8
#define PI 3.1415926

unsigned char* rgbBuf, *yBuf, *uBuf, *vBuf;
long Height, Width;

typedef struct  pitchElement
{
	double* tmp = new double[M * N];

}pitchElement;

void readRGB(FILE * pFile, BITMAPFILEHEADER & file_h, BITMAPINFOHEADER & info_h, unsigned char * rgbDataOut) {
	long  i, j, width, height, w, h;
	unsigned char mask, *Index_Data, *Data;

	if ((info_h.biWidth % 4) == 0)
		w = info_h.biWidth;
	else
		w = (info_h.biWidth*info_h.biBitCount + 31) / 32 * 4;
	if ((info_h.biHeight % 2) == 0)
		h = info_h.biHeight;
	else
		h = info_h.biHeight + 1;

	width = w / 8 * info_h.biBitCount;
	height = h;

	//����ǰ���ݻ�����  
	Index_Data = (unsigned char*)malloc(height*width);//buffer��СӦ����bmp����Ч���ݴ�С��ͬ  
													  //��������ݻ����������ڴ��bmp�е���Ч��  
	Data = (unsigned char*)malloc(height*width);//buffer��СӦ����bmp����Ч���ݴ�С��ͬ 

												//�ļ�ָ�붨λ����Ч������ʼ��,��ȡ��Ч����  
	fseek(pFile, file_h.bfOffBits, 0);

	if (fread(Index_Data, height*width, 1, pFile) != 1)
	{
		exit(0);
	}

	for (i = 0; i< height; i++)
		for (j = 0; j< width; j++)
		{
			Data[i*width + j] = Index_Data[(height - i - 1)*width + j];
		}
	//24λ��ֱ�Ӱѵ����Ļ��������ݸ��Ƹ����������
	//��Ϊ����Ƶ����ȡ�ؼ�֡��ʱ���Ѿ���bmp�ĸ�ʽ������Ϊ24���Ͳ������������	 
	if (info_h.biBitCount == 24)
	{
		memcpy(rgbDataOut, Data, height*width);
		if (Index_Data)
			free(Index_Data);
		if (Data)
			free(Data);

	}
}
float RGBYUV0257[256], RGBYUV0504[256], RGBYUV0098[256];
float RGBYUV0148[256], RGBYUV0291[256], RGBYUV0439[256];
float RGBYUV0368[256], RGBYUV0071[256];
void initLookupTable()
{
	for (int i = 0; i<256; i++)
	{
		RGBYUV0257[i] = (float)0.257 * i;
		RGBYUV0504[i] = (float)0.504 * i;
		RGBYUV0098[i] = (float)0.098 * i;
		RGBYUV0148[i] = (float)0.148 * i;
		RGBYUV0291[i] = (float)0.291 * i;
		RGBYUV0439[i] = (float)0.439 * i;
		RGBYUV0368[i] = (float)0.368 * i;
		RGBYUV0071[i] = (float)0.071 * i;

	}
}

void rgbToYUV(unsigned long w, unsigned long h, unsigned char * rgbData, unsigned char * y, unsigned char * u, unsigned char *v) {
	initLookupTable();//��ʼ�����ұ�  
	unsigned char*ytemp = NULL;
	unsigned char*utemp = NULL;
	unsigned char*vtemp = NULL;
	utemp = (unsigned char*)malloc(w*h);
	vtemp = (unsigned char*)malloc(w*h);

	unsigned long i, nr, ng, nb, nSize;
	//��ÿ�����ؽ��� rgb -> yuv��ת��  
	for (i = 0, nSize = 0; nSize < w * h * 3; nSize += 3)
	{
		nb = rgbData[nSize];
		ng = rgbData[nSize + 1];
		nr = rgbData[nSize + 2];
		y[i] = (unsigned char)(RGBYUV0257[nr] + RGBYUV0504[ng] + RGBYUV0098[nb] + 16);
		utemp[i] = (unsigned char)(-RGBYUV0148[nr] - RGBYUV0291[ng] + RGBYUV0439[nb] + 128);
		vtemp[i] = (unsigned char)(RGBYUV0439[nr] - RGBYUV0368[ng] - RGBYUV0071[nb] + 128);
		i++;
	}
	//��u�źż�v�źŽ��в�������Ϊ��4:2:0��ʽ������u��������y�����ݵ�1/4��v��������y�����ݵ�1/4  
	int k = 0;
	for (i = 0; i < h; i += 2)
		for (unsigned long j = 0; j < w; j += 2)
		{
			u[k] = (utemp[i*w + j] + utemp[(i + 1)*w + j] + utemp[i*w + j + 1] + utemp[(i + 1)

				*w + j + 1]) / 4;
			v[k] = (vtemp[i*w + j] + vtemp[(i + 1)*w + j] + vtemp[i*w + j + 1] + vtemp[(i + 1)

				*w + j + 1]) / 4;
			k++;
		}
	for (i = 0; i<w*h; i++)
	{
		if (y[i]<0)
			y[i] = 0;
		if (y[i]>255)
			y[i] = 255;
	}
	for (i = 0; i<h*w / 4; i++)
	{
		if (u[i]<0)
			u[i] = 0;
		if (v[i]<0)
			v[i] = 0;
		if (u[i]>255)
			u[i] = 255;
		if (v[i]>255)
			v[i] = 255;
	}

	if (utemp)
		free(utemp);
	if (vtemp)
		free(vtemp);
}

void writeYUV(unsigned char*Y, unsigned char*U, unsigned char*V, unsigned long size, FILE *outFile) {
	fwrite(Y, 1, size, outFile);
	fwrite(U, 1, size / 4, outFile);
	fwrite(V, 1, size / 4, outFile);
}

void bmpToYUV(FILE* bmpFile, FILE* yuvFile) {

	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;



	//��ȡλͼ�ļ�ͷ
	if (fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, bmpFile) != 1) {
		exit(0);
	}
	//�ж��ļ�����
	if (fileHeader.bfType != 0x4D42) {
		exit(0);
	}
	//��ȡλͼ��Ϣͷ
	if (fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, bmpFile) != 1) {
		exit(0);
	}
	long width, height;
	//��֤��4��������
	if (infoHeader.biWidth % 4 == 0) {
		width = infoHeader.biWidth;
	}
	else {
		width = (infoHeader.biWidth*infoHeader.biBitCount + 31) / 32 * (32 / 8);
	}
	//��֤������ż��
	if ((infoHeader.biHeight % 2) == 0)
		height = infoHeader.biHeight;
	else {
		height = infoHeader.biHeight + 1;
	}
	//��������
	rgbBuf = new unsigned char[height * width * 3];
	yBuf = new unsigned char[height * width];
	uBuf = new unsigned char[height * width / 4];
	vBuf = new unsigned char[height * width / 4];

	//��ȡrgb��Ϣ
	readRGB(bmpFile, fileHeader, infoHeader, rgbBuf);

	//��rgbת����yuv
	rgbToYUV(width, height, rgbBuf, yBuf, uBuf, vBuf);

	//��yuvд���ļ�
	writeYUV(yBuf, uBuf, vBuf, width * height, yuvFile);

	Height = height;
	Width = width;

	if (rgbBuf) {
		free(rgbBuf);
	}


}

double  sum(int u, int v, double* a) {
	double s = 0;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			s += (cos(((2 * i + 1)*u*PI) / (2 * M)))*(cos(((2 * j + 1)*v*PI) / (2 * N)))*a[i * N + j];
		}
	}
	return s;

}

double* DCT(double* a) {
	double C[M];
	for (int i = 0; i < M; i++) {
		if (i == 0) {
			C[i] = sqrt(2.0) / 2.0;
		}
		else {
			C[i] = 1.0;
		}
	}
	double* b = new double[M * N];
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			b[i * N + j] = ((2 * C[i] * C[j]) / sqrt(M * N)) * sum(i, j, a);
		}
	}
	return b;

}

//������
double yQ[M * N] = { 16, 11, 10, 16, 24, 40, 51, 61,
12, 12, 14, 19, 26, 58, 60, 55,
14, 13, 16, 24, 40, 57, 69, 56,
14, 17, 22, 29, 51, 87, 80, 62,
18, 22, 37, 56, 68, 109, 103, 77,
24, 35, 55, 64, 81, 104, 113, 92,
49, 64, 78, 87, 103, 121, 120, 101,
72, 92, 95, 98, 112, 100, 103, 99 };

double uvQ[M * N] = { 17, 18, 24, 47, 99, 99, 99, 99,
18, 21, 26, 66, 99, 99, 99, 99,
24, 26, 56, 99, 99, 99, 99, 99,
47, 66, 99, 99, 99, 99, 99, 99,
99, 99, 99, 99, 99, 99, 99, 99,
99, 99, 99, 99, 99, 99, 99, 99,
99, 99, 99, 99, 99, 99, 99, 99,
99, 99, 99, 99, 99, 99, 99, 99 };

double scale = 10; ///��������

double* Quantization(double* a, double* q) {
	double* b = new double[M * N];
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			b[i * N + j] = round(a[i * N + j] / (scale *q[i * N + j]));
		}
	}

	return b;
}

double sum2(int i, int j, double* a) {
	double C[M];
	for (int i = 0; i < M; i++) {
		if (i == 0) {
			C[i] = sqrt(2.0) / 2.0;
		}
		else {
			C[i] = 1.0;
		}
	}
	double s = 0;
	for (int u = 0; u < M; u++) {
		for (int v = 0; v < N; v++) {
			s += (C[u] * C[v]) / 4 * cos(((2 * i + 1) * u * PI) / 16) * cos(((2 * j + 1) * v * PI) / 16) * a[u * N + v];
		}
	}
	return s;
}
double* IDCT(double* a) {

	double* b = new double[M * N];
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			b[i * N + j] = sum2(i, j, a);
		}
	}
	return b;

}

double* IQuantization(double* a, double* q) {
	double* b = new double[M * N];
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			b[i * N + j] = a[i * N + j] * scale * q[i * N + j];
		}
	}

	return b;
}

void Encode_Decode(FILE* yuvFile) {

	///////////////��ͼ�����8*8���طֿ�
	pitchElement* yPitch, *uPitch, *vPitch;
	yPitch = new pitchElement[(Height / M) * (Width / N)];
	uPitch = new pitchElement[(Height / (M * 2)) * (Width / (N * 2))];
	vPitch = new pitchElement[(Height / (M * 2)) * (Width / (N * 2))];
	//y
	for (int m = 0; m < Height / M; m++) {
		for (int n = 0; n < Width / N; n++) {
			double* y = new double[M * N];
			for (int i = 0; i < M; i++) {
				for (int j = 0; j < N; j++) {
					y[i * N + j] = yBuf[m * Width * M + n * N + i * Width + j];


				}
			}
			yPitch[m * (Width / N) + n].tmp = y;
		}
	}
	//uv
	for (int m = 0; m < Height / (M * 2); m++) {
		for (int n = 0; n < Width / (N * 2); n++) {
			double* u = new double[M * N];
			double* v = new double[M * N];
			for (int i = 0; i < M; i++) {
				for (int j = 0; j < N; j++) {

					u[i * N + j] = uBuf[m * Width * M / 2 + n * N + i * Width / 2 + j];
					v[i * N + j] = vBuf[m * Width * M / 2 + n * N + i * Width / 2 + j];
				}
			}
			uPitch[m * (Width / (N * 2)) + n].tmp = u;
			vPitch[m * (Width / (N * 2)) + n].tmp = v;
		}
	}



	///////////��8*8���طֿ����DCT�仯
	pitchElement* dctY, *dctU, *dctV;
	dctY = new pitchElement[(Height / M) * (Width / N)];
	dctU = new pitchElement[(Height / (M * 2)) * (Width / (N * 2))];
	dctV = new pitchElement[(Height / (M * 2)) * (Width / (N * 2))];	
	for (int m = 0; m < Height / M; m++) {
		for (int n = 0; n < Width / N; n++) {
			dctY[m * (Width / N) + n].tmp = DCT(yPitch[m * (Width / N) + n].tmp);
		}
	}
	for (int m = 0; m < Height / (M * 2); m++) {
		for (int n = 0; n < Width / (N * 2); n++) {
			dctU[m * (Width / (N * 2)) + n].tmp = DCT(uPitch[m * (Width / (N * 2)) + n].tmp);
			dctV[m * (Width / (N * 2)) + n].tmp = DCT(vPitch[m * (Width / (N * 2)) + n].tmp);

		}
	}


	////////////��8*8���طֿ��������
	pitchElement* qY, *qU, *qV;
	qY = new pitchElement[(Height / M) * (Width / N)];
	qU = new pitchElement[(Height / (M * 2)) * (Width / (N * 2))];
	qV = new pitchElement[(Height / (M * 2)) * (Width / (N * 2))];
	for (int m = 0; m < Height / M; m++) {
		for (int n = 0; n < Width / N; n++) {
			qY[m * (Width / N) + n].tmp = Quantization(dctY[m * (Width / N) + n].tmp, yQ);
		}
	}
	for (int m = 0; m < Height / (M * 2); m++) {
		for (int n = 0; n < Width / (N * 2); n++) {
			qU[m * (Width / (N * 2)) + n].tmp = Quantization(dctU[m * (Width / (N * 2)) + n].tmp, uvQ);
			qV[m * (Width / (N * 2)) + n].tmp = Quantization(dctV[m * (Width / (N * 2)) + n].tmp, uvQ);
		}
	}



	///////////////////��8*8���طֿ���з�����
	pitchElement* iqY, *iqU, *iqV;
	iqY = new pitchElement[(Height / M) * (Width / N)];
	iqU = new pitchElement[(Height / (M * 2)) * (Width / (N * 2))];
	iqV = new pitchElement[(Height / (M * 2)) * (Width / (N * 2))];
	for (int m = 0; m < Height / M; m++) {
		for (int n = 0; n < Width / N; n++) {
			iqY[m * (Width / N) + n].tmp = IQuantization(qY[m * (Width / N) + n].tmp, yQ);
		}
	}
	for (int m = 0; m < Height / (M * 2); m++) {
		for (int n = 0; n < Width / (N * 2); n++) {

			iqU[m * (Width / (N * 2)) + n].tmp = IQuantization(qU[m * (Width / (N * 2)) + n].tmp, uvQ);
			iqV[m * (Width / (N * 2)) + n].tmp = IQuantization(qV[m * (Width / (N * 2)) + n].tmp, uvQ);
		}
	}

	////////////////////��8*8���طֿ����IDCT
	pitchElement* idctY, *idctU, *idctV;
	idctY = new pitchElement[(Height / M) * (Width / N)];
	idctU = new pitchElement[(Height / (M * 2)) * (Width / (N * 2))];
	idctV = new pitchElement[(Height / (M * 2)) * (Width / (N * 2))];	
	for (int m = 0; m < Height / M; m++) {
		for (int n = 0; n < Width / N; n++) {
			idctY[m * (Width / N) + n].tmp = IDCT(iqY[m * (Width / N) + n].tmp);
		}
	}
	for (int m = 0; m < Height / (M * 2); m++) {
		for (int n = 0; n < Width / (N * 2); n++) {
			idctU[m * (Width / (N * 2)) + n].tmp = IDCT(iqU[m * (Width / (N * 2)) + n].tmp);
			idctV[m * (Width / (N * 2)) + n].tmp = IDCT(iqV[m * (Width / (N * 2)) + n].tmp);

		}
	}

	
	
	/////////////////���yuv
	unsigned char* outY, *outU, *outV;
	//��doubleת��unsigned char
	outY = new unsigned char[Width * Height];
	outU = new unsigned char[Width * Height / 4];
	outV = new unsigned char[Width * Height / 4];
	//y
	for (int m = 0; m < Height / M; m++) {
		for (int n = 0; n < Width / N; n++) {
			for (int i = 0; i < M; i++) {
				for (int j = 0; j < N; j++) {

					outY[m * Width * M + n * N + i * Width + j] = (unsigned char)idctY[m * (Width / N) + n].tmp[i * N + j];

				}
			}
		}
	}
	//uv
	for (int m = 0; m < Height / (M * 2); m++) {
		for (int n = 0; n < Width / (N * 2); n++) {
			for (int i = 0; i < M; i++) {
				for (int j = 0; j < N; j++) {

					outU[m * Width * M / 2 + n * N + i * Width / 2 + j] = (unsigned char)idctU[m * (Width / (N * 2)) + n].tmp[i * N + j];
					outV[m * Width * M / 2 + n * N + i * Width / 2 + j] = (unsigned char)idctV[m * (Width / (N * 2)) + n].tmp[i * N + j];
				}
			}
		}
	}
	for (int i = 0; i< Width * Height; i++)
	{
		if (outY[i]<0)
			outY[i] = 0;
		if (outY[i]>255)
			outY[i] = 255;
	}
	for (int i = 0; i< Width * Height / 4; i++)
	{
		if (outU[i]<0)
			outU[i] = 0;
		if (outV[i]<0)
			outV[i] = 0;
		if (outU[i]>255)
			outU[i] = 255;
		if (outV[i]>255)
			outV[i] = 255;
	}

	long width, height;
	width = Width;
	height = Height;

	writeYUV(outY, outU, outV, width * height, yuvFile);

	if (yBuf) {
		free(yBuf);
	}
	if (uBuf) {
		free(uBuf);
	}
	if (vBuf) {
		free(vBuf);
	}
	if (outY) {
		free(outY);
	}
	if (outU) {
		free(outU);
	}
	if (outV) {
		free(outV);
	}

}
int main(int argc, char* argv[])
{

	getBMP();
	FILE* fp_BMP = fopen("KeyFrame1.bmp", "rb");
	FILE* fp_YUV = fopen("output1.yuv", "wb");
	FILE* fp_E_D = fopen("output2.yuv", "wb");
	bmpToYUV(fp_BMP, fp_YUV);
	Encode_Decode(fp_E_D);

	return 0;
}


