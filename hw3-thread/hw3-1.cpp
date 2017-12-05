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

// #define PRINT_TIME

#ifdef PRINT_TIME
#include "time.h"
#endif

using namespace std;

#define MYRED	2
#define MYGREEN 1
#define MYBLUE	0

int imgWidth, imgHeight;
int ws;
// #define ws 3
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
	for (int j = 0; j<::ws; j++)
	for (int i = 0; i<::ws; i++)
	{
		a = w + i - (::ws / 2);
		b = h + j - (::ws / 2);

		// detect for borders of the image
		if (a<0 || b<0 || a>=imgWidth || b>=imgHeight) continue;

		tmp += filter_G[j*::ws + i] * pic_grey[b*imgWidth + a];
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
	unsigned char* blur_ptr;
  int j_up;
	int j_low;
} Parameter;

void *convertGrey(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	// unsigned char* pic_grey = (unsigned char*) arg->pic_ptr;
	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_grey[j*imgWidth + i] = RGB2grey(i, j);
		}
	}
}

void *applyBlur(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	// unsigned char* pic_blur = (unsigned char*) arg->pic_ptr;
	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
		}
	}
}

void *extend(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	// unsigned char* pic_blur = (unsigned char*) arg->blur_ptr;
	// pthread_mutex_lock(&mutex);
	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_final[3 * (j*imgWidth + i) + MYRED] = pic_blur[j*imgWidth + i];
			pic_final[3 * (j*imgWidth + i) + MYGREEN] = pic_blur[j*imgWidth + i];
			pic_final[3 * (j*imgWidth + i) + MYBLUE] = pic_blur[j*imgWidth + i];
		}
	}
	// pthread_mutex_unlock(&mutex);
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
	::ws = (int)sqrt((float)FILTER_SIZE);
	// ::ws = 3;

	#ifdef PRINT_TIME
	double START,END;
	cout << "Start read mask file." << endl;
	START = clock();
	#endif
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_G[i]);
	fclose(mask);
	#ifdef PRINT_TIME
	END = clock();
	cout << "Read file time：" << (END - START) / CLOCKS_PER_SEC << " s" << endl;
	cout << endl;
	#endif


	BmpReader* bmpReader = new BmpReader();
	for (int k = 0; k<5; k++){

		#ifdef PRINT_TIME
		cout << "--- image " << k << "---\n" << endl;
		#endif

		// read input BMP file
		pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth, &imgHeight);
		// allocate space for output image
		pic_grey = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_blur = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_final = (unsigned char*)malloc(3 * imgWidth*imgHeight*sizeof(unsigned char));

		//convert RGB image to grey image


		#ifdef PRINT_TIME
		cout << "Start grey." << endl;
		START = clock();
		#endif

		pthread_t grey_thread1, grey_thread2, grey_thread3, grey_thread4;
		pthread_t grey_thread5, grey_thread6, grey_thread7, grey_thread8;
		void *grey1_fin, *grey2_fin, *grey3_fin, *grey4_fin;
		Parameter grey_arg1, grey_arg2, grey_arg3, grey_arg4;
		Parameter grey_arg5, grey_arg6, grey_arg7, grey_arg8;

		grey_arg1.j_low = 0; grey_arg1.j_up = imgHeight/8;
		pthread_create(&grey_thread1, NULL, convertGrey, &grey_arg1);
		grey_arg2.j_low = imgHeight/8; grey_arg2.j_up = imgHeight/4;
		pthread_create(&grey_thread2, NULL, convertGrey, &grey_arg2);


		grey_arg3.j_low = imgHeight/4; grey_arg3.j_up = (imgHeight/8)*3;
		pthread_create(&grey_thread3, NULL, convertGrey, &grey_arg3);
		grey_arg4.j_low = (imgHeight/8)*3; grey_arg4.j_up = imgHeight/2;
		pthread_create(&grey_thread4, NULL, convertGrey, &grey_arg4);

		grey_arg5.j_low = imgHeight/2; grey_arg5.j_up = (imgHeight/8)*5;
		pthread_create(&grey_thread5, NULL, convertGrey, &grey_arg5);
		grey_arg6.j_low = (imgHeight/8)*5; grey_arg6.j_up = (imgHeight/8)*6;
		pthread_create(&grey_thread6, NULL, convertGrey, &grey_arg6);


		grey_arg7.j_low = (imgHeight/8)*6; grey_arg7.j_up = (imgHeight/8)*7;
		pthread_create(&grey_thread7, NULL, convertGrey, &grey_arg7);
		grey_arg8.j_low = (imgHeight/8)*7; grey_arg8.j_up = imgHeight;
		pthread_create(&grey_thread8, NULL, convertGrey, &grey_arg8);

		// pthread_join( grey_thread1, NULL);
		// pthread_join( grey_thread2, NULL);
		// pthread_join( grey_thread3, NULL);
		// pthread_join( grey_thread4, NULL);

		#ifdef PRINT_TIME
		END = clock();
		cout << "Grey time：" << (END - START) << " ms" << endl;
		cout << endl;
		#endif

		//apply the Gaussian filter to the image
		#ifdef PRINT_TIME
		cout << "Start filter." << endl;
		START = clock();
		#endif

		pthread_t blur_thread1, blur_thread2, blur_thread3, blur_thread4;
		pthread_t blur_thread5, blur_thread6, blur_thread7, blur_thread8;
		Parameter blur_arg1, blur_arg2, blur_arg3, blur_arg4;
		Parameter blur_arg5, blur_arg6, blur_arg7, blur_arg8;

		blur_arg1.pic_ptr = pic_blur;
		blur_arg1.j_low = 0; blur_arg1.j_up = imgHeight/8;
		pthread_join( grey_thread1, NULL);
		pthread_create(&blur_thread1, NULL, applyBlur, &blur_arg1);
		blur_arg2.pic_ptr = pic_blur;
		blur_arg2.j_low = imgHeight/8; blur_arg2.j_up = imgHeight/4;
		pthread_join( grey_thread2, NULL);
		pthread_create(&blur_thread2, NULL, applyBlur, &blur_arg2);

		blur_arg3.pic_ptr = pic_blur;
		blur_arg3.j_low = imgHeight/4; blur_arg3.j_up = (imgHeight/8)*3;
		pthread_join( grey_thread3, NULL);
		pthread_create(&blur_thread3, NULL, applyBlur, &blur_arg3);
		blur_arg4.pic_ptr = pic_blur;
		blur_arg4.j_low = (imgHeight/8)*3; blur_arg4.j_up = imgHeight/2;
		pthread_join( grey_thread4, NULL);
		pthread_create(&blur_thread4, NULL, applyBlur, &blur_arg4);

		blur_arg5.pic_ptr = pic_blur;
		blur_arg5.j_low = imgHeight/2; blur_arg5.j_up = (imgHeight/8)*5;
		pthread_join( grey_thread5, NULL);
		pthread_create(&blur_thread5, NULL, applyBlur, &blur_arg5);
		blur_arg6.pic_ptr = pic_blur;
		blur_arg6.j_low = (imgHeight/8)*5; blur_arg6.j_up = (imgHeight/8)*6;
		pthread_join( grey_thread6, NULL);
		pthread_create(&blur_thread6, NULL, applyBlur, &blur_arg6);

		blur_arg7.pic_ptr = pic_blur;
		blur_arg7.j_low = (imgHeight/8)*6; blur_arg7.j_up = (imgHeight/8)*7;
		pthread_join( grey_thread7, NULL);
		pthread_create(&blur_thread7, NULL, applyBlur, &blur_arg7);
		blur_arg8.pic_ptr = pic_blur;
		blur_arg8.j_low = (imgHeight/8)*7; blur_arg8.j_up = imgHeight;
		pthread_join( grey_thread8, NULL);
		pthread_create(&blur_thread8, NULL, applyBlur, &blur_arg8);

		#ifdef PRINT_TIME
		END = clock();
		cout << "Filter time：" << (END - START) << " ms" << endl;
		cout << endl;
		#endif

		//extend the size form WxHx1 to WxHx3
		#ifdef PRINT_TIME
		cout << "Start extend." << endl;
		START = clock();
		#endif

		int r = MYRED, g = MYGREEN, b = MYBLUE;

		pthread_t fin_thread1, fin_thread2, fin_thread3, fin_thread4;
		pthread_t fin_thread5, fin_thread6, fin_thread7, fin_thread8;
		Parameter fin_arg1, fin_arg2, fin_arg3, fin_arg4;
		Parameter fin_arg5, fin_arg6, fin_arg7, fin_arg8;

		fin_arg1.j_low = 0; fin_arg1.j_up = (imgHeight/8);
		pthread_join( blur_thread1, NULL);
		pthread_create(&fin_thread1, NULL, extend, &fin_arg1);
		fin_arg2.j_low = (imgHeight/8); fin_arg2.j_up = (imgHeight/4);
		pthread_join( blur_thread2, NULL);
		pthread_create(&fin_thread2, NULL, extend, &fin_arg2);

		fin_arg3.j_low = imgHeight/4; fin_arg3.j_up = (imgHeight/8)*3;
		pthread_join( blur_thread3, NULL);
		pthread_create(&fin_thread3, NULL, extend, &fin_arg3);
		fin_arg4.j_low = (imgHeight/8)*3; fin_arg4.j_up = imgHeight/2;
		pthread_join( blur_thread4, NULL);
		pthread_create(&fin_thread4, NULL, extend, &fin_arg4);

		fin_arg5.j_low = imgHeight/2; fin_arg5.j_up = (imgHeight/8)*5;
		pthread_join( blur_thread5, NULL);
		pthread_create(&fin_thread5, NULL, extend, &fin_arg5);
		fin_arg6.j_low = (imgHeight/8)*5; fin_arg6.j_up = (imgHeight/8)*6;
		pthread_join( blur_thread6, NULL);
		pthread_create(&fin_thread6, NULL, extend, &fin_arg6);

		fin_arg7.j_low = (imgHeight/8)*6; fin_arg7.j_up = (imgHeight/8)*7;
		pthread_join( blur_thread7, NULL);
		pthread_create(&fin_thread7, NULL, extend, &fin_arg7);

		fin_arg8.j_low = (imgHeight/8)*7; fin_arg8.j_up = imgHeight;
		pthread_join( blur_thread8, NULL);
		pthread_create(&fin_thread8, NULL, extend, &fin_arg8);

		pthread_join( fin_thread1, NULL);
		pthread_join( fin_thread2, NULL);
		pthread_join( fin_thread3, NULL);
		pthread_join( fin_thread4, NULL);
		pthread_join( fin_thread5, NULL);
		pthread_join( fin_thread6, NULL);
		pthread_join( fin_thread7, NULL);
		pthread_join( fin_thread8, NULL);

		#ifdef PRINT_TIME
		END = clock();
		cout << "Extend time：" << (END - START) << " ms" << endl;
		cout << endl;
		#endif

		// write output BMP file
		#ifdef PRINT_TIME
		cout << "Start write." << endl;
		START = clock();
		#endif

		bmpReader->WriteBMP(outputBlur_name[k], imgWidth, imgHeight, pic_final);

		#ifdef PRINT_TIME
		END = clock();
		cout << "Write time：" << (END - START) << " ms" << endl;
		cout << endl;
		#endif

		//free memory space
		free(pic_in);
		free(pic_grey);
		free(pic_blur);
		free(pic_final);
	}

	return 0;
}
