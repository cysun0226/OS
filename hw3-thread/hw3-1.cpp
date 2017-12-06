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

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock6 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock7 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock8 = PTHREAD_MUTEX_INITIALIZER;

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
  pthread_mutex_t lock;
  int j_up;
	int j_low;
	int color_ptr;
} Parameter;

void *convertGrey(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	pthread_mutex_lock(&arg->lock);
	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_grey[j*imgWidth + i] = RGB2grey(i, j);
		}
	}
	pthread_mutex_unlock(&arg->lock);
}

void *applyBlur(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;

	while (pthread_mutex_trylock(&arg->lock)) {
	}
	// pthread_mutex_lock(&arg->lock);

	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_blur[j*imgWidth + i] = GaussianFilter(i, j);
		}
	}

	pthread_mutex_unlock(&arg->lock);
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
		arg1.j_low = 0; arg1.j_up = imgHeight/8; arg1.lock = lock1;
		arg2.j_low = imgHeight/8; arg2.j_up = imgHeight/4; arg2.lock = lock2;
		arg3.j_low = imgHeight/4; arg3.j_up = (imgHeight/8)*3; arg3.lock = lock3;
		arg4.j_low = (imgHeight/8)*3; arg4.j_up = imgHeight/2; arg4.lock = lock4;
		arg5.j_low = imgHeight/2; arg5.j_up = (imgHeight/8)*5; arg5.lock = lock5;
		arg6.j_low = (imgHeight/8)*5; arg6.j_up = (imgHeight/8)*6; arg6.lock = lock6;
		arg7.j_low = (imgHeight/8)*6; arg7.j_up = (imgHeight/8)*7; arg7.lock = lock7;
		arg8.j_low = (imgHeight/8)*7; arg8.j_up = imgHeight; arg8.lock = lock8;


		pthread_create(&grey_thread1, NULL, convertGrey, &arg1);
		pthread_create(&grey_thread2, NULL, convertGrey, &arg2);
		pthread_create(&grey_thread3, NULL, convertGrey, &arg3);
		pthread_create(&grey_thread4, NULL, convertGrey, &arg4);
		pthread_create(&grey_thread5, NULL, convertGrey, &arg5);
		pthread_create(&grey_thread6, NULL, convertGrey, &arg6);
		pthread_create(&grey_thread7, NULL, convertGrey, &arg7);
		pthread_create(&grey_thread8, NULL, convertGrey, &arg8);

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



		pthread_create(&blur_thread1, NULL, applyBlur, &arg1);
		pthread_create(&blur_thread2, NULL, applyBlur, &arg2);
		pthread_create(&blur_thread3, NULL, applyBlur, &arg3);
		pthread_create(&blur_thread4, NULL, applyBlur, &arg4);
		pthread_create(&blur_thread5, NULL, applyBlur, &arg5);
		pthread_create(&blur_thread6, NULL, applyBlur, &arg6);
		pthread_create(&blur_thread7, NULL, applyBlur, &arg7);
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
		Parameter ext_arg1, ext_arg2, ext_arg3, ext_arg4;
		Parameter ext_arg5, ext_arg6, ext_arg7, ext_arg8;

		ext_arg1.j_low = 0; ext_arg1.j_up = imgHeight/4;
		ext_arg2.j_low = imgHeight/4; ext_arg2.j_up = imgHeight/2;
		ext_arg3.j_low = imgHeight/2; ext_arg3.j_up = (imgHeight/4)*3;
		ext_arg4.j_low = (imgHeight/4)*3; ext_arg4.j_up = imgHeight;

		pthread_t ext_r_thread1, ext_r_thread2, ext_r_thread3, ext_r_thread4;
		pthread_t ext_r_thread5, ext_r_thread6, ext_r_thread7, ext_r_thread8;
		pthread_t ext_g_thread1, ext_g_thread2, ext_g_thread3, ext_g_thread4;
		pthread_t ext_g_thread5, ext_g_thread6, ext_g_thread7, ext_g_thread8;
		pthread_t ext_b_thread1, ext_b_thread2, ext_b_thread3, ext_b_thread4;
		pthread_t ext_b_thread5, ext_b_thread6, ext_b_thread7, ext_b_thread8;

		pthread_join(blur_thread1, NULL);
		pthread_join(blur_thread2, NULL);
		pthread_create(&ext_r_thread1, NULL, extend_r, &ext_arg1);
		pthread_create(&ext_g_thread1, NULL, extend_g, &ext_arg1);
		pthread_create(&ext_b_thread1, NULL, extend_b, &ext_arg1);

		pthread_join(blur_thread3, NULL);
		pthread_join(blur_thread4, NULL);
		pthread_create(&ext_r_thread2, NULL, extend_r, &ext_arg2);
		pthread_create(&ext_g_thread2, NULL, extend_g, &ext_arg2);
		pthread_create(&ext_b_thread2, NULL, extend_b, &ext_arg2);

		pthread_join(blur_thread5, NULL);
		pthread_join(blur_thread6, NULL);
		pthread_create(&ext_r_thread3, NULL, extend_r, &ext_arg3);
		pthread_create(&ext_g_thread3, NULL, extend_g, &ext_arg3);
		pthread_create(&ext_b_thread3, NULL, extend_b, &ext_arg3);

		pthread_join(blur_thread7, NULL);
		pthread_join(blur_thread8, NULL);
		pthread_create(&ext_r_thread4, NULL, extend_r, &ext_arg4);
		pthread_create(&ext_g_thread4, NULL, extend_g, &ext_arg4);
		pthread_create(&ext_b_thread4, NULL, extend_b, &ext_arg4);

		pthread_join( ext_r_thread1, NULL);
		pthread_join( ext_r_thread2, NULL);
		pthread_join( ext_r_thread3, NULL);
		pthread_join( ext_r_thread4, NULL);

		pthread_join( ext_g_thread1, NULL);
		pthread_join( ext_g_thread2, NULL);
		pthread_join( ext_g_thread3, NULL);
		pthread_join( ext_g_thread4, NULL);

		pthread_join( ext_b_thread1, NULL);
		pthread_join( ext_b_thread2, NULL);
		pthread_join( ext_b_thread3, NULL);
		pthread_join( ext_b_thread4, NULL);

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
