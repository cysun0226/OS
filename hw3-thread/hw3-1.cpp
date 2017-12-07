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

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock6 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock7 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock8 = PTHREAD_MUTEX_INITIALIZER;

int gStart_1 = 0;
int gStart_2 = 0;
int gStart_3 = 0;
int gStart_4 = 0;
int gStart_5 = 0;
int gStart_6 = 0;
int gStart_7 = 0;
int gStart_8 = 0;

int g1_pre = 1;
int g2_pre = 0;
int g3_pre = 0;
int g4_pre = 0;
int g5_pre = 0;
int g6_pre = 0;
int g7_pre = 0;
int g8_pre = 0;

int g1_next = 1;
int g2_next = 0;
int g3_next = 0;
int g4_next = 0;
int g5_next = 0;
int g6_next = 0;
int g7_next = 0;
int g8_next = 0;

enum { GREY, BLUR };

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

unsigned char unbnd_GaussianFilter(int w, int h)
{
	int tmp = 0;
	int a, b;
	for (int j = 0; j<::ws; j++)
	for (int i = 0; i<::ws; i++)
	{
		a = w + i - (::ws / 2);
		b = h + j - (::ws / 2);

		// detect for borders of the image

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
  pthread_mutex_t* lock;
  int j_up;
	int j_low;
	int color_ptr;
	int type;
	int* start;
	int* pre;
	int* cur;
	int* next;
	int part;
} Parameter;

int try_sync(int num)
{
	switch (num)
	{
		case 1:
		if(gStart_1 && gStart_2) return 1;
		else return 0;

		case 2:
		if(gStart_1 && gStart_2 && gStart_3) return 1;
		else return 0;

		case 3:
		if(gStart_2 && gStart_3 && gStart_4) return 1;
		else return 0;

		case 4:
		if(gStart_3 && gStart_4 && gStart_5) return 1;
		else return 0;

		case 5:
		if(gStart_4 && gStart_5 && gStart_6) return 1;
		else return 0;

		case 6:
		if(gStart_5 && gStart_6 && gStart_7) return 1;
		else return 0;

		case 7:
		if(gStart_6 && gStart_7 && gStart_8) return 1;
		else return 0;

		case 8:
		if(gStart_7 && gStart_8) return 1;
		else return 0;
	}
}


void *convertGrey(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	pthread_mutex_lock(arg->lock);
	// cout << "grey lock " << arg->lock << endl;

	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_grey[j*imgWidth + i] = RGB2grey(i, j);
		}
	}

	*arg->start = 1;
	// *arg->pre = 1;
	pthread_mutex_unlock(arg->lock);
	// cout << "grey unlock " << arg->lock << endl;
}

void *applyBlur(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	// cout << "blur lock " << arg->lock << endl;
	pthread_mutex_lock(arg->lock);
	while (try_sync(arg->part) != 1) {
		;
		// cout << "waiting" << endl; /* wait */
	}

	if (arg->j_low == 0 || arg->j_up == imgHeight) {
		for (int j = arg->j_low; j<arg->j_low+::ws; j++) {
			for (int i = 0; i<::ws; i++) {
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
			for (int i = ::ws; i<imgWidth-::ws; i++) {
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
			for (int i = imgWidth-::ws; i<imgWidth; i++) {
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
		}
		for (int j = arg->j_low+::ws; j<arg->j_up-::ws; j++) {
			for (int i = 0; i<::ws; i++) {
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
			for (int i = ::ws; i<imgWidth-::ws; i++) {
				pic_blur[j*imgWidth + i] = unbnd_GaussianFilter(i, j);
			}
			for (int i = imgWidth-::ws; i<imgWidth; i++) {
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
		}
		for (int j = arg->j_up-::ws; j<arg->j_up; j++) {
			for (int i = 0; i<::ws; i++) {
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
			for (int i = ::ws; i<imgWidth-::ws; i++) {
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
			for (int i = imgWidth-::ws; i<imgWidth; i++) {
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
		}
	}
	else {
		for (int j = arg->j_low; j<arg->j_up; j++) {
			for (int i = 0; i<::ws; i++) {
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
			for (int i = ::ws; i<imgWidth-::ws; i++) {
				pic_blur[j*imgWidth + i] = unbnd_GaussianFilter(i, j);
			}
			for (int i = imgWidth-::ws; i<imgWidth; i++) {
				pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
			}
		}
	}

	pthread_mutex_unlock(arg->lock);
	// cout << "blur unlock " << arg->lock << endl;
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

void *extend_color(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	int color = arg->color_ptr;
	// unsigned char* pic_blur = (unsigned char*) arg->blur_ptr;

	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_final[3 * (j*imgWidth + i) + color] = pic_blur[j*imgWidth + i];
		}
	}

}

void *extend_r(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;

	for (int j = arg->j_low; j<arg->j_up; j++) {
		// pthread_mutex_lock(&mutex);
		for (int i = 0; i<imgWidth; i++) {
			pic_final[3 * (j*imgWidth + i) + MYRED] = pic_blur[j*imgWidth + i];
		}
		// pthread_mutex_unlock(&mutex);
	}
}

void *extend_g(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;

	for (int j = arg->j_low; j<arg->j_up; j++) {
		// pthread_mutex_lock(&mutex);
		for (int i = 0; i<imgWidth; i++) {
			pic_final[3 * (j*imgWidth + i) + MYGREEN] = pic_blur[j*imgWidth + i];
		}
		// pthread_mutex_unlock(&mutex);
	}
}

void *extend_b(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;

	for (int j = arg->j_low; j<arg->j_up; j++) {
		// pthread_mutex_lock(&mutex);
		for (int i = 0; i<imgWidth; i++) {
			pic_final[3 * (j*imgWidth + i) + MYBLUE] = pic_blur[j*imgWidth + i];
		}
		// pthread_mutex_unlock(&mutex);
	}
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

		Parameter arg1, arg2, arg3, arg4;
		Parameter arg5, arg6, arg7, arg8;
		arg1.j_low = 0; arg1.j_up = imgHeight/8; arg1.lock = &lock1; arg1.start = &gStart_1; arg1.pre = &gStart_1; arg1.next = &gStart_2; arg1.cur = &gStart_1; arg1.part = 1;
		arg2.j_low = imgHeight/8; arg2.j_up = imgHeight/4; arg2.lock = &lock2; arg2.start = &gStart_2; arg2.pre = &gStart_1; arg2.next = &gStart_3; arg2.cur = &gStart_2; arg2.part = 2;
		arg3.j_low = imgHeight/4; arg3.j_up = (imgHeight/8)*3; arg3.lock = &lock3; arg3.start = &gStart_3; arg3.pre = &gStart_2; arg3.next = &gStart_4; arg3.cur = &gStart_3; arg3.part = 3;
		arg4.j_low = (imgHeight/8)*3; arg4.j_up = imgHeight/2; arg4.lock = &lock4; arg4.start = &gStart_4; arg4.pre = &gStart_3; arg4.next = &gStart_5; arg4.cur = &gStart_4; arg4.part = 4;
		arg5.j_low = imgHeight/2; arg5.j_up = (imgHeight/8)*5; arg5.lock = &lock5; arg5.start = &gStart_5; arg5.pre = &gStart_4; arg5.next = &gStart_6; arg5.cur = &gStart_5; arg5.part = 5;
		arg6.j_low = (imgHeight/8)*5; arg6.j_up = (imgHeight/8)*6; arg6.lock = &lock6; arg6.start = &gStart_6; arg6.pre = &gStart_5; arg6.next = &gStart_7; arg6.cur = &gStart_6; arg6.part = 6;
		arg7.j_low = (imgHeight/8)*6; arg7.j_up = (imgHeight/8)*7; arg7.lock = &lock7; arg7.start = &gStart_7; arg7.pre = &gStart_6; arg7.next = &gStart_8; arg7.cur = &gStart_7; arg7.part = 7;
		arg8.j_low = (imgHeight/8)*7; arg8.j_up = imgHeight; arg8.lock = &lock8; arg8.start = &gStart_8; arg8.pre = &gStart_7; arg8.next = &gStart_8; arg8.cur = &gStart_8; arg8.part = 8;

		Parameter grey_arg1 = arg1, grey_arg2 = arg2, grey_arg3 = arg3, grey_arg4 = arg4;
		Parameter grey_arg5 = arg5, grey_arg6 = arg6, grey_arg7 = arg7, grey_arg8 = arg8;
		grey_arg1.type = GREY; grey_arg2.type = GREY; grey_arg3.type = GREY; grey_arg4.type = GREY;
		grey_arg5.type = GREY; grey_arg6.type = GREY; grey_arg7.type = GREY; grey_arg8.type = GREY;


		pthread_create(&grey_thread1, NULL, convertGrey, &arg1);
		pthread_create(&grey_thread2, NULL, convertGrey, &arg2);
		pthread_create(&grey_thread3, NULL, convertGrey, &arg3);
		pthread_create(&grey_thread4, NULL, convertGrey, &arg4);
		pthread_create(&grey_thread5, NULL, convertGrey, &arg5);
		pthread_create(&grey_thread6, NULL, convertGrey, &arg6);
		pthread_create(&grey_thread7, NULL, convertGrey, &arg7);
		pthread_create(&grey_thread8, NULL, convertGrey, &arg8);

		// pthread_join(grey_thread1, NULL);
		// pthread_join(grey_thread2, NULL);
		// pthread_join(grey_thread3, NULL);
		// pthread_join(grey_thread4, NULL);
		// pthread_join(grey_thread5, NULL);
		// pthread_join(grey_thread6, NULL);
		// pthread_join(grey_thread7, NULL);
		// pthread_join(grey_thread8, NULL);


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

		Parameter blur_arg1 = arg1, blur_arg2 = arg2, blur_arg3 = arg3, blur_arg4 = arg4;
		Parameter blur_arg5 = arg5, blur_arg6 = arg6, blur_arg7 = arg7, blur_arg8 = arg8;
		blur_arg1.type = BLUR; blur_arg2.type = BLUR; blur_arg3.type = BLUR; blur_arg4.type = BLUR;
		blur_arg5.type = BLUR; blur_arg6.type = BLUR; blur_arg7.type = BLUR; blur_arg8.type = BLUR;

		pthread_join(grey_thread1, NULL);
		pthread_create(&blur_thread1, NULL, applyBlur, &arg1);
		pthread_join(grey_thread2, NULL);
		pthread_create(&blur_thread2, NULL, applyBlur, &arg2);
		pthread_join(grey_thread3, NULL);
		pthread_create(&blur_thread3, NULL, applyBlur, &arg3);
		pthread_join(grey_thread4, NULL);
		pthread_create(&blur_thread4, NULL, applyBlur, &arg4);
		pthread_join(grey_thread5, NULL);
		pthread_create(&blur_thread5, NULL, applyBlur, &arg5);
		pthread_join(grey_thread6, NULL);
		pthread_create(&blur_thread6, NULL, applyBlur, &arg6);
		pthread_join(grey_thread7, NULL);
		pthread_create(&blur_thread7, NULL, applyBlur, &arg7);
		pthread_join(grey_thread8, NULL);
		pthread_create(&blur_thread8, NULL, applyBlur, &arg8);

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

		//extend the size form WxHx1 to WxHx3
		pthread_t ext_thread1, ext_thread2, ext_thread3, ext_thread4;
		pthread_t ext_thread5, ext_thread6, ext_thread7, ext_thread8;

		pthread_join(blur_thread1, NULL);
		pthread_create(&ext_thread1, NULL, extend, &arg1);
		pthread_join(blur_thread2, NULL);
		pthread_create(&ext_thread2, NULL, extend, &arg2);

		pthread_join(blur_thread3, NULL);
		pthread_create(&ext_thread3, NULL, extend, &arg3);
		pthread_join(blur_thread4, NULL);
		pthread_create(&ext_thread4, NULL, extend, &arg4);

		pthread_join(blur_thread5, NULL);
		pthread_create(&ext_thread5, NULL, extend, &arg5);
		pthread_join(blur_thread6, NULL);
		pthread_create(&ext_thread6, NULL, extend, &arg6);

		pthread_join(blur_thread7, NULL);
		pthread_create(&ext_thread7, NULL, extend, &arg7);
		pthread_join(blur_thread8, NULL);
		pthread_create(&ext_thread8, NULL, extend, &arg8);

		pthread_join( ext_thread1, NULL);
		pthread_join( ext_thread2, NULL);
		pthread_join( ext_thread3, NULL);
		pthread_join( ext_thread4, NULL);
		pthread_join( ext_thread5, NULL);
		pthread_join( ext_thread6, NULL);
		pthread_join( ext_thread7, NULL);
		pthread_join( ext_thread8, NULL);

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

		gStart_1 = 0;	gStart_2 = 0;	gStart_3 = 0;	gStart_4 = 0;
		gStart_5 = 0;	gStart_6 = 0;	gStart_7 = 0;	gStart_8 = 0;

		g1_pre = 1;	g2_pre = 0;	g3_pre = 0;	g4_pre = 0;
		g5_pre = 0;	g6_pre = 0;	g7_pre = 0;	g8_pre = 0;
	}

	return 0;
}
