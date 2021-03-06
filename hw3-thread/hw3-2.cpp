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
int ws;
int FILTER_SIZE;
int FILTER_SCALE;
int *filter_x;
int *filter_y;

sem_t sem1, sem2, sem3, sem4, sem5, sem6, sem7, sem8, sem;

const char *inputfile_name[5] = {
	"input1.bmp",
	"input2.bmp",
	"input3.bmp",
	"input4.bmp",
	"input5.bmp"
};

const char *outputSobel_name[5] = {
	"Sobel1.bmp",
	"Sobel2.bmp",
	"Sobel3.bmp",
	"Sobel4.bmp",
	"Sobel5.bmp"
};

int gStart_1 = 0;
int gStart_2 = 0;
int gStart_3 = 0;
int gStart_4 = 0;
int gStart_5 = 0;
int gStart_6 = 0;
int gStart_7 = 0;
int gStart_8 = 0;

unsigned char *pic_in, *pic_grey, *image_x, *image_y, *image, *pic_blur, *pic_final;

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

unsigned char applyFilter(int w, int h, int* filter)
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

		tmp += filter[j*::ws + i] * pic_grey[b*imgWidth + a];
	};
	// tmp /= FILTER_SCALE;
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char unbnd_applyFilter(int w, int h, int* filter)
{
	int tmp = 0;
	int a, b;

	for (int j = 0; j<::ws; j++)
	for (int i = 0; i<::ws; i++)
	{
		a = w + i - (::ws / 2);
		b = h + j - (::ws / 2);

		// detect for borders of the image

		tmp += filter[j*::ws + i] * pic_grey[b*imgWidth + a];
	};
	// tmp /= FILTER_SCALE;
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char sobel(int i,int j, unsigned char* image_x, unsigned char* image_y)
{
	unsigned int tmp = 0;
	tmp = image_x[j*imgWidth + i]*image_x[j*imgWidth + i] + image_y[j*imgWidth + i]*image_y[j*imgWidth + i];
	tmp = sqrt(tmp);
	if (tmp > 255) tmp = 255;
	if (tmp < 0) tmp = 0;
	return (unsigned char)tmp;
}

// thread functions
typedef struct
{
  int j_up;
	int j_low;
	int color_ptr;
	sem_t* sem;
	int part;
	int* start;
	int num;
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

	// sem_wait(arg->sem);
	sem_wait(&sem);
	// unsigned char* pic_grey = (unsigned char*) arg->pic_ptr;
	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			pic_grey[j*imgWidth + i] = RGB2grey(i, j);
		}
	}
	*arg->start = 1;
	// sem_post(arg->sem);
	sem_post(&sem);
}

void *applyX(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	while (try_sync(arg->num) != 1) {
		;/* waiting */
	}
	sem_wait(&sem);


	// sem_wait(arg->sem);
	// unsigned char* pic_blur = (unsigned char*) arg->pic_ptr;

	if (arg->j_low == 0 || arg->j_up == imgHeight) {
	  for (int j = arg->j_low; j<arg->j_low+::ws; j++) {
	    for (int i = 0; i<::ws; i++) {
	      image_x[j*imgWidth + i] = applyFilter(i, j, filter_x);
	    }
	    for (int i = ::ws; i<imgWidth-::ws; i++) {
	      image_x[j*imgWidth + i] = applyFilter(i, j, filter_x);
	    }
	    for (int i = imgWidth-::ws; i<imgWidth; i++) {
	      image_x[j*imgWidth + i] = applyFilter(i, j, filter_x);
	    }
	  }
	  for (int j = arg->j_low+::ws; j<arg->j_up-::ws; j++) {
	    for (int i = 0; i<::ws; i++) {
	      image_x[j*imgWidth + i] = applyFilter(i, j, filter_x);
	    }
	    for (int i = ::ws; i<imgWidth-::ws; i++) {
	      image_x[j*imgWidth + i] = unbnd_applyFilter(i, j, filter_x);
	    }
	    for (int i = imgWidth-::ws; i<imgWidth; i++) {
	      image_x[j*imgWidth + i] = applyFilter(i, j, filter_x);
	    }
	  }
	  for (int j = arg->j_up-::ws; j<arg->j_up; j++) {
	    for (int i = 0; i<::ws; i++) {
	      image_x[j*imgWidth + i] = applyFilter(i, j, filter_x);
	    }
	    for (int i = ::ws; i<imgWidth-::ws; i++) {
	      image_x[j*imgWidth + i] = applyFilter(i, j, filter_x);
	    }
	    for (int i = imgWidth-::ws; i<imgWidth; i++) {
	      image_x[j*imgWidth + i] = applyFilter(i, j, filter_x);
	    }
	  }
	}
	else {
	  for (int j = arg->j_low; j<arg->j_up; j++) {
	    for (int i = 0; i<::ws; i++) {
	      image_x[j*imgWidth + i] = applyFilter(i, j, filter_x);
	    }
	    for (int i = ::ws; i<imgWidth-::ws; i++) {
	      image_x[j*imgWidth + i] = unbnd_applyFilter(i, j, filter_x);
	    }
	    for (int i = imgWidth-::ws; i<imgWidth; i++) {
	      image_x[j*imgWidth + i] = applyFilter(i, j, filter_x);
	    }
	  }
	}


	sem_post(&sem);
}

void *applyY(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	while (try_sync(arg->num) != 1) {
		;/* waiting */
	}
	sem_wait(arg->sem);
	// unsigned char* pic_blur = (unsigned char*) arg->pic_ptr;
	if (arg->j_low == 0 || arg->j_up == imgHeight) {
	  for (int j = arg->j_low; j<arg->j_low+::ws; j++) {
	    for (int i = 0; i<::ws; i++) {
	      image_y[j*imgWidth + i] = applyFilter(i, j, filter_y);
	    }
	    for (int i = ::ws; i<imgWidth-::ws; i++) {
	      image_y[j*imgWidth + i] = applyFilter(i, j, filter_y);
	    }
	    for (int i = imgWidth-::ws; i<imgWidth; i++) {
	      image_y[j*imgWidth + i] = applyFilter(i, j, filter_y);
	    }
	  }
	  for (int j = arg->j_low+::ws; j<arg->j_up-::ws; j++) {
	    for (int i = 0; i<::ws; i++) {
	      image_y[j*imgWidth + i] = applyFilter(i, j, filter_y);
	    }
	    for (int i = ::ws; i<imgWidth-::ws; i++) {
	      image_y[j*imgWidth + i] = unbnd_applyFilter(i, j, filter_y);
	    }
	    for (int i = imgWidth-::ws; i<imgWidth; i++) {
	      image_y[j*imgWidth + i] = applyFilter(i, j, filter_y);
	    }
	  }
	  for (int j = arg->j_up-::ws; j<arg->j_up; j++) {
	    for (int i = 0; i<::ws; i++) {
	      image_y[j*imgWidth + i] = applyFilter(i, j, filter_y);
	    }
	    for (int i = ::ws; i<imgWidth-::ws; i++) {
	      image_y[j*imgWidth + i] = applyFilter(i, j, filter_y);
	    }
	    for (int i = imgWidth-::ws; i<imgWidth; i++) {
	      image_y[j*imgWidth + i] = applyFilter(i, j, filter_y);
	    }
	  }
	}
	else {
	  for (int j = arg->j_low; j<arg->j_up; j++) {
	    for (int i = 0; i<::ws; i++) {
	      image_y[j*imgWidth + i] = applyFilter(i, j, filter_y);
	    }
	    for (int i = ::ws; i<imgWidth-::ws; i++) {
	      image_y[j*imgWidth + i] = unbnd_applyFilter(i, j, filter_y);
	    }
	    for (int i = imgWidth-::ws; i<imgWidth; i++) {
	      image_y[j*imgWidth + i] = applyFilter(i, j, filter_y);
	    }
	  }
	}
	sem_post(arg->sem);
}

void *computeImg(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;
	// unsigned char* pic_blur = (unsigned char*) arg->pic_ptr;
	for (int j = arg->j_low; j<arg->j_up; j++) {
		for (int i = 0; i<imgWidth; i++) {
			image[j*imgWidth + i] = sobel(i, j, image_x, image_y);
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
			pic_final[3 * (j*imgWidth + i) + MYRED] = image[j*imgWidth + i];
			pic_final[3 * (j*imgWidth + i) + MYGREEN] = image[j*imgWidth + i];
			pic_final[3 * (j*imgWidth + i) + MYBLUE] = image[j*imgWidth + i];
		}
	}
	// pthread_mutex_unlock(&mutex);
}

void *extend_r(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;

	for (int j = arg->j_low; j<arg->j_up; j++) {
		// sem_wait(&sem);
		for (int i = 0; i<imgWidth; i++) {
			pic_final[3 * (j*imgWidth + i) + MYRED] = image[j*imgWidth + i];
		}
		// sem_post(&sem);
	}
}

void *extend_g(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;

	for (int j = arg->j_low; j<arg->j_up; j++) {
		// sem_wait(&sem);
		for (int i = 0; i<imgWidth; i++) {
			pic_final[3 * (j*imgWidth + i) + MYGREEN] = image[j*imgWidth + i];
		}
		// sem_post(&sem);
	}
}

void *extend_b(void* arg_ptr)
{
	Parameter* arg = (Parameter*) arg_ptr;

	for (int j = arg->j_low; j<arg->j_up; j++) {
		// sem_wait(&sem);
		for (int i = 0; i<imgWidth; i++) {
			pic_final[3 * (j*imgWidth + i) + MYBLUE] = image[j*imgWidth + i];
		}
		// sem_post(&sem);
	}
}


int main()
{
	// initialize semaphore
	sem_init(&sem, 0, 8);
	sem_init(&sem1, 0, 1);
	sem_init(&sem2, 0, 1);
	sem_init(&sem3, 0, 1);
	sem_init(&sem4, 0, 1);
	sem_init(&sem5, 0, 1);
	sem_init(&sem6, 0, 1);
	sem_init(&sem7, 0, 1);
	sem_init(&sem8, 0, 1);

	// read mask file
	FILE* mask;
	mask = fopen("mask_Sobel.txt", "r");
	fscanf(mask, "%d", &FILTER_SIZE);
	::ws = (int)sqrt((float)FILTER_SIZE);
	//fscanf(mask, "%d", &FILTER_SCALE);

	filter_x = new int[FILTER_SIZE];
	filter_y = new int[FILTER_SIZE];

	// read filter x
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_x[i]);

	// read filter y
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_y[i]);

	fclose(mask);

	BmpReader* bmpReader = new BmpReader();
	for (int k = 0; k<5; k++){
		// read input BMP file
		pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth, &imgHeight);
		// allocate space for output image
		pic_grey = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		// pic_blur = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		image_x = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		image_y = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		image = (unsigned char*)malloc(imgWidth*imgHeight*sizeof(unsigned char));
		pic_final = (unsigned char*)malloc(3 * imgWidth*imgHeight*sizeof(unsigned char));

		//convert RGB image to grey image

		pthread_t grey_thread1, grey_thread2, grey_thread3, grey_thread4;
		pthread_t grey_thread5, grey_thread6, grey_thread7, grey_thread8;
		void *grey1_fin, *grey2_fin, *grey3_fin, *grey4_fin;
		Parameter arg1, arg2, arg3, arg4;
		Parameter arg5, arg6, arg7, arg8;

		arg1.j_low = 0; arg1.j_up = imgHeight/8; arg1.sem = &sem1; arg1.num = 1; arg1.start = &gStart_1;
		arg2.j_low = imgHeight/8; arg2.j_up = imgHeight/4; arg2.sem = &sem2; arg2.num = 2; arg2.start = &gStart_2;
		arg3.j_low = imgHeight/4; arg3.j_up = (imgHeight/8)*3; arg3.sem = &sem3; arg3.num = 3; arg3.start = &gStart_3;
		arg4.j_low = (imgHeight/8)*3; arg4.j_up = imgHeight/2; arg4.sem = &sem4; arg4.num = 4; arg4.start = &gStart_4;
		arg5.j_low = imgHeight/2; arg5.j_up = (imgHeight/8)*5; arg5.sem = &sem5; arg5.num = 5; arg5.start = &gStart_5;
		arg6.j_low = (imgHeight/8)*5; arg6.j_up = (imgHeight/8)*6; arg6.sem = &sem6; arg6.num = 6; arg6.start = &gStart_6;
		arg7.j_low = (imgHeight/8)*6; arg7.j_up = (imgHeight/8)*7; arg7.sem = &sem7; arg7.num = 7; arg7.start = &gStart_7;
		arg8.j_low = (imgHeight/8)*7; arg8.j_up = imgHeight; arg8.sem = &sem8; arg8.num = 8; arg8.start = &gStart_8;

		pthread_create(&grey_thread1, NULL, convertGrey, &arg1);
		pthread_create(&grey_thread2, NULL, convertGrey, &arg2);
		pthread_create(&grey_thread3, NULL, convertGrey, &arg3);
		pthread_create(&grey_thread4, NULL, convertGrey, &arg4);

		pthread_create(&grey_thread5, NULL, convertGrey, &arg5);
		pthread_create(&grey_thread6, NULL, convertGrey, &arg6);
		pthread_create(&grey_thread7, NULL, convertGrey, &arg7);
		pthread_create(&grey_thread8, NULL, convertGrey, &arg8);

		// apply x-filter to image
		// apply y-filter to image
		pthread_t x_thread1, x_thread2, x_thread3, x_thread4;
		pthread_t x_thread5, x_thread6, x_thread7, x_thread8;

		pthread_t y_thread1, y_thread2, y_thread3, y_thread4;
		pthread_t y_thread5, y_thread6, y_thread7, y_thread8;

		pthread_join( grey_thread1, NULL);
		pthread_join( grey_thread2, NULL);



		pthread_create(&x_thread1, NULL, applyX, &arg1);
		pthread_create(&y_thread1, NULL, applyY, &arg1);

		pthread_join( grey_thread3, NULL);
		pthread_create(&x_thread2, NULL, applyX, &arg2);
		pthread_create(&y_thread2, NULL, applyY, &arg2);

		pthread_join( grey_thread4, NULL);
		pthread_create(&x_thread3, NULL, applyX, &arg3);
		pthread_create(&y_thread3, NULL, applyY, &arg3);

		pthread_join( grey_thread5, NULL);
		pthread_create(&x_thread4, NULL, applyX, &arg4);
		pthread_create(&y_thread4, NULL, applyY, &arg4);

		pthread_join( grey_thread6, NULL);
		pthread_create(&x_thread5, NULL, applyX, &arg5);
		pthread_create(&y_thread5, NULL, applyY, &arg5);

		pthread_join( grey_thread7, NULL);
		pthread_create(&x_thread6, NULL, applyX, &arg6);
		pthread_create(&y_thread6, NULL, applyY, &arg6);

		pthread_join( grey_thread8, NULL);
		pthread_create(&x_thread7, NULL, applyX, &arg7);
		pthread_create(&y_thread7, NULL, applyY, &arg7);

		pthread_create(&x_thread8, NULL, applyX, &arg8);
		pthread_create(&y_thread8, NULL, applyY, &arg8);


		// compute image
		pthread_t img_thread1, img_thread2, img_thread3, img_thread4;
		pthread_t img_thread5, img_thread6, img_thread7, img_thread8;

		pthread_join( x_thread1, NULL);
		pthread_join( y_thread1, NULL);

		pthread_join( x_thread2, NULL);
		pthread_join( y_thread2, NULL);
		pthread_create(&img_thread1, NULL, computeImg, &arg1);

		pthread_join( x_thread3, NULL);
		pthread_join( y_thread3, NULL);
		pthread_create(&img_thread2, NULL, computeImg, &arg2);

		pthread_join( x_thread4, NULL);
		pthread_join( y_thread4, NULL);
		pthread_create(&img_thread3, NULL, computeImg, &arg3);

		pthread_join( x_thread5, NULL);
		pthread_join( y_thread5, NULL);
		pthread_create(&img_thread4, NULL, computeImg, &arg4);

		pthread_join( x_thread6, NULL);
		pthread_join( y_thread6, NULL);
		pthread_create(&img_thread5, NULL, computeImg, &arg5);

		pthread_join( x_thread7, NULL);
		pthread_join( y_thread7, NULL);
		pthread_create(&img_thread6, NULL, computeImg, &arg6);

		pthread_join( x_thread8, NULL);
		pthread_join( y_thread8, NULL);
		pthread_create(&img_thread7, NULL, computeImg, &arg7);
		pthread_create(&img_thread8, NULL, computeImg, &arg8);

		//extend the size form WxHx1 to WxHx3
		pthread_t ext_thread1, ext_thread2, ext_thread3, ext_thread4;
		pthread_t ext_thread5, ext_thread6, ext_thread7, ext_thread8;

		pthread_join(img_thread1, NULL);
		pthread_create(&ext_thread1, NULL, extend, &arg1);
		pthread_join(img_thread2, NULL);
		pthread_create(&ext_thread2, NULL, extend, &arg2);

		pthread_join(img_thread3, NULL);
		pthread_create(&ext_thread3, NULL, extend, &arg3);
		pthread_join(img_thread4, NULL);
		pthread_create(&ext_thread4, NULL, extend, &arg4);

		pthread_join(img_thread5, NULL);
		pthread_create(&ext_thread5, NULL, extend, &arg5);
		pthread_join(img_thread6, NULL);
		pthread_create(&ext_thread6, NULL, extend, &arg6);

		pthread_join(img_thread7, NULL);
		pthread_create(&ext_thread7, NULL, extend, &arg7);
		pthread_join(img_thread8, NULL);
		pthread_create(&ext_thread8, NULL, extend, &arg8);

		pthread_join( ext_thread1, NULL);
		pthread_join( ext_thread2, NULL);
		pthread_join( ext_thread3, NULL);
		pthread_join( ext_thread4, NULL);
		pthread_join( ext_thread5, NULL);
		pthread_join( ext_thread6, NULL);
		pthread_join( ext_thread7, NULL);
		pthread_join( ext_thread8, NULL);

		// write output BMP file
		bmpReader->WriteBMP(outputSobel_name[k], imgWidth, imgHeight, pic_final);

		//free memory space
		free(pic_in);
		free(pic_grey);
		free(image_x);
		free(image_y);
		free(image);
		free(pic_final);

		gStart_1 = 0;
		gStart_2 = 0;
		gStart_3 = 0;
		gStart_4 = 0;
		gStart_5 = 0;
		gStart_6 = 0;
		gStart_7 = 0;
		gStart_8 = 0;
	}

	return 0;
}
