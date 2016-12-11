#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mtat.h"
#include "bmp.h"
typedef struct bwimage{
	BMPImage* image;
	BMPImage* bmp;
	int index;
	int count;
	int radius;
}BWimage;

void* _bwimage(void* arg);
bool _check(BMPHeader* header);
int _index(BMPImage* bmp, int x, int y);
void _binarize(BMPImage* input, BMPImage* output, int radius);
int _intensity(BMPImage* bmp, int x, int y);
char* _strdup(const char* src);
int _max(int x, int y);
int _min(int x, int y);

bool _check(BMPHeader* header){
	if(header->type != 0x4d42){
		return false;
	}
	if(header->offset != BMP_HEADER_SIZE){
		return false;
	}
	if(header->dib_header_size != DIB_HEADER_SIZE){
		return false;
	}
	if(header->num_planes != 1){
		return false;
	}
	if(header->compression != 0){
		return false;
	}
	if(header->num_colors != 0 || header->important_colors != 0){
		return false;
	}
	if(header->bits_per_pixel != 16 && header->bits_per_pixel != 24){
		return false;
	}
	return true;
}

BMPImage* binarize(BMPImage* image, int radius, int num_threads, char** error){
	bool check = _check(&(image->header));
	if(check == false){
		if(*error == NULL){
			char* message = "unable to open the image because the header info is wrong.";
			*error = _strdup(message);
		}
		return NULL;
	}
	if(radius < 0){
		if(*error == NULL){
			char* message = "unable to binarize the image because the radius is negative.";
			*error = _strdup(message);
		}
		return NULL;
	}
	int height = image->header.height_px;
	int width = image->header.width_px;
	BMPImage* bmp = malloc(sizeof(*bmp));
	bmp->header = image->header;
	bmp->data = malloc(sizeof(char)*image->header.image_size_bytes);
	for(int i = 0; i < image->header.image_size_bytes; i++){
		bmp->data[i] = image->data[i];
	}
	//bmp = crop_bmp(image, 0, 0, width, height, error);
	BWimage* bwimage = malloc(sizeof(*bwimage)*num_threads);
	pthread_t* thread = malloc(sizeof(*thread)*num_threads);
	//BMPImage* bmp = crop_bmp(image, 0, 0, width, height, error);
	int num_pixel = (image->header.width_px)*(image->header.height_px);
	if(num_pixel/num_threads < 1){
		num_threads = num_pixel;
	}
	int process = num_pixel/num_threads;
	int remainder = num_pixel%num_threads;
	int i = 0;
	for(i = 0; i < num_threads; i++){
		bwimage[i].image = image;
		bwimage[i].bmp = bmp;
		bwimage[i].radius = radius;
		bwimage[i].count = process;
		if(i == 0){
			bwimage[i].index = 0;
		}
		else{
			bwimage[i].index = bwimage[i - 1].index + bwimage[i - 1].count;
		}
	}
	bwimage[i].count = process + remainder;
	for(int j = 0; j < num_threads; j++){
		check = pthread_create(&thread[j], NULL, _bwimage, &bwimage[j]);
		if(check != 0){
			free(thread);
			if(*error == NULL){
				char* message = "The thread cannot be created.";
				*error = _strdup(message);
			}
			return NULL;
		}
	}
	for(int k = 0; k < num_threads; k++){
		pthread_join(thread[k], NULL);
	}
	free(thread);
	return bmp;
}

void* _bwimage(void* arg){
	BWimage* bwimage = (BWimage*)arg;
	//int i = bwimage->index;
	//for(int j = 0; j < bwimage->count; j++){
		_binarize(bwimage->image, bwimage->bmp,  bwimage->radius);
	//}
	return NULL;
}

void _binarize(BMPImage* input, BMPImage* output, int radius){
int size=input->header.image_size_bytes;
int height =input->header.height_px;
int width = input->header.width_px;

int pixel = input -> header.bits_per_pixel / 8;
int pixel_row = pixel * input -> header.width_px;
int row_lenght = (width*pixel + 3)/4*4;    //add padding

int padding = row_lenght - pixel_row;
 int x=0;
 int y=0;
 int i=0;

 for (y=0;y<height;y++) {
 for (x=0;x<width;x++){
 int up = radius;
 int down = radius;
 int left = radius;
 int right = radius;

   if(x - radius < 0){
     left = x;
	        	 }
				 if(x + radius >width-1){
				 right = width - x-1;
				 }
				 if(y - radius <0){
				 up = y;
				 }
				 if(y + radius > (height-1)){
				 down = height - y-1;
				 }
				 int sum_neighb=0;
				 int h=0;
				 int w=0;
				 int count=0;
				 for (h=y-up;h<(y+down+1);h++){
				 for (w=x-left	;w<x+right+1; w++){
				 sum_neighb += input->data[h*row_lenght+3*w];
				 sum_neighb += input->data[h*row_lenght+3*w+1];
				 sum_neighb += input->data[h*row_lenght+3*w+2];
				 count++;
				 }
				 }
				 int sum_px=input->data[y*row_lenght+3*x]+input->data[y*row_lenght+3*x+1]+input->data[y*row_lenght+3*x+2];

				 if(sum_px * count <= sum_neighb	){
				 for(i = 0; i < pixel; i++){
				 output->data[y*row_lenght+3*x + i] = 0;
				 }
				 }
				 else{
				 for(i = 0; i < pixel; i++){
				 output->data[y*row_lenght+3*x + i] = 255;
				 }
				 }
				 }
				 for (i=0;i<padding;i++)	{
				 output->data[y*row_lenght+3*x+3+i]=0;
				 }	
				 }
				 return;
				 }


int _intensity(BMPImage* bmp, int x, int y){
	int sum = 0;
	int pixel = bmp->header.bits_per_pixel/8;
	int index = _index(bmp, x, y);
	for(int i =0; i < pixel; i++){
		sum += bmp->data[index + i];
	}
	return sum;
}

int _index(BMPImage* image, int x, int y){
	int row = (image->header.image_size_bytes)/(image->header.height_px);
	int pixel = (image->header.bits_per_pixel)/8;
	int index = pixel*x + (image->header.height_px - y - 1)*row;
	return index;
}

char* _strdup(const char* src){
	return strcpy(malloc((strlen(src) + 1)*sizeof(*src)), src);
}

int _max(int x, int y){
	if(x > y){
		return x;
	}
	return y;
}

int _min(int x, int y){
	if(x > y){
		return y;
	}
	return x;
}
	

