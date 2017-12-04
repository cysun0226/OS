// Student ID: 0416045
// Name      : cysun
// Date      : 2017.11.03

#include "bmpReader.h"
#include "bmpReader.cpp"
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

#define MYRED	2
#define MYGREEN 1
#define MYBLUE	0

int imgWidth, imgHeight;
int FILTER_SIZE;
int FILTER_SCALE;
int *filter_G;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

const char *inputfile_name[5] = {
	"input1.bmp",
	"input2.bmp",
	"input3.bmp",
	"input4.bmp",
	"input5.bmp"
};
const char *outputBlur_name[5] = {
	"Blur1.bmp",
	"Blur2.bmp",
	"Blur3.bmp",
	"Blur4.bmp",
	"Blur5.bmp"
};

unsigned char *pic_in, *pic_grey, *pic_blur, *pic_final;

unsigned char RGB2grey(int w, int h)
{
	int tmp = (
		pic_in[3 * (h*imgWidth + w) + MYRED] +
		pic_in[3 * (h*imgWidth + w) + MYGREEN] +
		pic_in[3 * (h*imgWidth + w) + MYBLUE] )/3;

	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char GaussianFilter(int w, int h)
{
	int tmp = 0;
	int a, b;
	int ws = (int)sqrt((float)FILTER_SIZE);
	for (int j = 0; j<ws; j++)
	for (int i = 0; i<ws; i++)
	{
		a = w + i - (ws / 2);
		b = h + j - (ws / 2);

		// detect for borders of the image
		if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;

		tmp += filter_G[j*ws + i] * pic_grey[b*imgWidth + a];
	};
	tmp /= FILTER_SCALE;
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

// thread functions
typedef struct
{
  unsigned char* pic_ptr;
  int j_up;
	int j_low;
} Parameter;

void *convertGrey(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	unsigned char* pic_grey = (unsigned char*) arg->pic_ptr;
	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_grey[j*imgWidth + i] = RGB2grey(i, j);
		}
	}
}

void *applyBlur(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	unsigned char* pic_blur = (unsigned char*) arg->pic_ptr;
	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
		}
	}
}

void *extend(void* color_ptr)
{
	int color = *((int *) color_ptr);
	pthread_mutex_lock(&mutex);
	for (int j = 0; j<imgHeight; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_final[3 * (j*imgWidth + i) + color] = pic_blur[j*imgWidth + i];
		}
	}
	pthread_mutex_unlock(&mutex);
}

/* main */
int main()
{
	// read mask file
	FILE* mask;
	mask = fopen("mask_Gaussian.txt", "r");
	fscanf(mask, "%d", &FILTER_SIZE);
	fscanf(mask, "%d", &FILTER_SCALE);

	filter_G = new int[FILTER_SIZE];
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_G[i]);
	fclose(mask);


	BmpReader* bmpReader = new BmpReader();
	for (int k = 0; k<5; k++){
		// read input BMP file
		pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth, &imgHeight);
		// allocate space for output image
		pic_grey = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_blur = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_final = (unsigned char*)malloc(3 * imgWidth*imgHeight*sizeof(unsigned char));

		//convert RGB image to grey image
		pthread_t grey_thread1, grey_thread2, grey_thread3, grey_thread4;
		void *grey1_fin, *grey2_fin, *grey3_fin, *grey4_fin;

		Parameter grey_arg1, grey_arg2, grey_arg3, grey_arg4;
		grey_arg1.pic_ptr = pic_grey;
		grey_arg1.j_low = 0; grey_arg1.j_up = imgHeight/4;
		pthread_create(&grey_thread1, NULL, convertGrey, &grey_arg1);
		grey_arg2.pic_ptr = pic_grey;
		grey_arg2.j_low = imgHeight/4; grey_arg2.j_up = imgHeight/2;
		pthread_create(&grey_thread2, NULL, convertGrey, &grey_arg2);
		grey_arg3.pic_ptr = pic_grey;
		grey_arg3.j_low = imgHeight/2; grey_arg3.j_up = (imgHeight/4)*3;
		pthread_create(&grey_thread3, NULL, convertGrey, &grey_arg3);
		grey_arg4.pic_ptr = pic_grey;
		grey_arg4.j_low = (imgHeight/4)*3; grey_arg4.j_up = imgHeight;
		pthread_create(&grey_thread4, NULL, convertGrey, &grey_arg4);

		pthread_join( grey_thread1, NULL);
		pthread_join( grey_thread2, NULL);
		pthread_join( grey_thread3, NULL);
		pthread_join( grey_thread4, NULL);

		//apply the Gaussian filter to the image
		for (int j = 0; j<imgHeight; j++) {
			for (int i = 0; i<imgWidth; i++){
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
		}

		//extend the size form WxHx1 to WxHx3
		int r = MYRED, g = MYGREEN, b = MYBLUE;
		pthread_t red_thread, green_thread, blue_thread;
		pthread_create(&red_thread, NULL, extend, &r);
		pthread_create(&green_thread, NULL, extend, &g);
		pthread_create(&blue_thread, NULL, extend, &b);

		pthread_join( red_thread, NULL);
		pthread_join( green_thread, NULL);
		pthread_join( blue_thread, NULL);

		// for (int j = 0; j<imgHeight; j++) {
		// 	for (int i = 0; i<imgWidth; i++) {
		// 		pic_final[3 * (j*imgWidth + i) + MYRED] = pic_blur[j*imgWidth + i];
		// 		pic_final[3 * (j*imgWidth + i) + MYGREEN] = pic_blur[j*imgWidth + i];
		// 		pic_final[3 * (j*imgWidth + i) + MYBLUE] = pic_blur[j*imgWidth + i];
		// 	}
		// }

		// write output BMP file
		bmpReader->WriteBMP(outputBlur_name[k], imgWidth, imgHeight, pic_final);

		//free memory space
		free(pic_in);
		free(pic_grey);
		free(pic_blur);
		free(pic_final);
	}

	return 0;
}
