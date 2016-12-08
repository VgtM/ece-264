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
void _binarize(BWimage* bwimage, int x, int y);
int _intensity(BMPImage* bmp, int x, int y);
char* _strdup(const char* src);

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
	//BMPImage* bmp = malloc(sizeof(*bmp));
	//bmp->header = image->header;
	//bmp->data = malloc(sizeof(bmp->data[0])*height*width);
	//bmp = crop_bmp(image, 0, 0, width, height, error);
	BWimage* bwimage = malloc(sizeof(*bwimage)*num_threads);
	pthread_t* thread = malloc(sizeof(*thread)*num_threads);
	BMPImage* bmp = crop_bmp(image, 0, 0, width, height, error);
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
	int i = bwimage->index;
	for(int j = 0; j < bwimage->count; j++){
		int x = i%(bwimage->image->header.width_px);
		int y = i/(bwimage->image->header.width_px);
		_binarize(bwimage, x, y);
		i++;
	}
	return NULL;
}

void _binarize(BWimage* bwimage, int x, int y){
	int radius = bwimage->radius;
	int height = bwimage->image->header.height_px;
	int width = bwimage->image->header.width_px;
	int pixel = bwimage->image->header.bits_per_pixel/8;
	int up = radius;
	int down = radius;
	int left = radius;
	int right = radius;
	int sum = 0;
	if(x - radius <= 0){
		left = x;
	}
	if(x + radius >= width){
		right = width - x;
	}
	if(y - radius <= 0){
		up = y;
	}
	if(y + radius >= height){
		down = height - y;
	}
	int i = 0;
	int index = _index(bwimage->image, x, y);
	int target = _intensity(bwimage->image, x, y);
	int ymin = y - up;
	int ymax = y + down;
	assert(ymin >= 0 && ymax <= height);
	while(ymin < ymax){
		//if(ymin >= 0 && ymax <= height){
			int xmax = x + right;
			int xmin = x - left;
			assert(xmin >= 0 && xmax <= width);
			while(xmin < xmax){
				//if(xmin >= 0 && xmax < width){
					int sum1  = _intensity(bwimage->image, xmin, ymin);
					sum += sum1;
					i++;
					xmin++;
				}
				//else if(xmin < 0){
				//	xmin++;
				//}
				//else{
				//	xmax--;
				//}
			//}
	//	}
		//else if(ymin < 0){
		//	ymin++;
		//}
		//else{
		//	ymax--;
		//}
		ymin++;
	}
	if(target <= sum/i){
		for(i = 0; i < pixel; i++){
			bwimage->bmp->data[index + i] = 0;
		}
	}
	else{
		for(i = 0; i < pixel; i++){
			bwimage->bmp->data[index + i] = 255;
		}
	}
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


	

