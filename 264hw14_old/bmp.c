#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "bmp.h"


BMPImage* read_bmp(FILE* fp, char** error){
	FILE* file = fp;
	if(file == NULL){
		char* message = "Cannot open the file, because the file doesn't exist";
		*error = malloc(sizeof(*error)*(strlen(message)+1));
		strcpy(*error, message);
		return NULL;
	}
	fseek(file, 0, SEEK_SET);
	BMPImage* image = malloc(sizeof(*image));
	int check = fread(&(image->header), sizeof(BMPHeader), 1, file);
	if(check != 1){
		free(image);
		char* message = "Cannot open the image beacuse it failed to read";
		*error = malloc(sizeof(*error)*(strlen(message) + 1));
		strcpy(*error, message);
		return NULL;
	}
	bool check1 = check_bmp_header(&(image->header), file);
	if(check1 == false){
		free(image);
		if(*error == NULL){
			char* message = "bmp header is corrupt";
			(*error) = malloc((strlen(message)+1)*sizeof(*error));
			strcpy(*error, message);
			printf("%s", *error);
			return NULL;
		}
		return NULL;
	}
	else{
		image->data = malloc(image->header.image_size_bytes);
		int pos = ftell(file);
		fseek(file, 54L, SEEK_SET);
		int check2 = fread(image->data, 1, (image->header.image_size_bytes), file);
		fseek(file, pos, SEEK_SET);
		if(check2 != image->header.image_size_bytes){
			free(image->data);
			free(image);
			char* message = "Cannot open the image because of the false information";
			*error = malloc(sizeof(*error)*(strlen(message) + 1));
			strcpy(*error, message);
			return NULL;
		}
		return image;
	}
	return image;
}

bool check_bmp_header(BMPHeader* bmp_hdr, FILE* fp){
	if(fp == NULL){
		return false;
	}
	fseek(fp, 0, SEEK_END);
    int fileSize = ftell(fp);
	if(bmp_hdr->type != 0x4d42){
		return false;
	}
	if(bmp_hdr->offset != BMP_HEADER_SIZE){
		return false;
	}
	if(bmp_hdr->dib_header_size != DIB_HEADER_SIZE){
		return false;
	}
    if(bmp_hdr->num_planes != 1){
		return false;
	}
	if(bmp_hdr->compression !=0){
		return false;
	}
	if(bmp_hdr->num_colors != 0 || bmp_hdr->important_colors != 0){
		return false;
	}
	if(bmp_hdr->bits_per_pixel != 16 && bmp_hdr->bits_per_pixel != 24){
		return false;
	}
	if(bmp_hdr->size != fileSize) {
	  return false;
	}
	int width = bmp_hdr->width_px;
	int height = bmp_hdr->height_px;
	int bppixel = bmp_hdr->bits_per_pixel;
	int linebyte = (width*bppixel/8 + 3)/4*4;
	int imagesize = height*linebyte;
	int bmpsize = imagesize + 54;
	int pos = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	if(bmpsize != size){
		return false;
	}
	else{
		fseek(fp, pos, SEEK_SET);
		return true;
	}
}

bool write_bmp(FILE* fp, BMPImage* image, char** error){
	if(image == NULL){
		if(*error == NULL){
			char* message = "reading image failed";
			*error = (char*)malloc((strlen(message)+1)*sizeof(*error));
			strcpy(*error, message);
			printf("%s", *error);
			return false;
		}
		return false;
	}
	if(fwrite(&(image->header), sizeof(image->header), 1, fp) != 1){
		if(*error == NULL){
			char* message = "reading image failed";
			*error = (char*)malloc((strlen(message)+1)*sizeof(*error));
			strcpy(*error, message);
			printf("%s", *error);
			return false;
		}
		return false;
	}
	if(fwrite(image->data, image->header.image_size_bytes, 1, fp) != 1){
		if(*error == NULL){
			char* message = "reading image failed";
			*error = (char*)malloc((strlen(message)+1)*sizeof(*error));
			strcpy(*error, message);
			printf("%s", *error);
			return false;
		}
		return false;
	}
	return true;
}

void free_bmp(BMPImage* image){
	if(image != NULL){
	if(image->data != NULL){
		free(image->data);
		
	}
	free(image);
}
}

BMPImage* crop_bmp(BMPImage* image, int x, int y, int w, int h, char** error){
	BMPImage* bmp = malloc(sizeof(*bmp));
	if(image == NULL){
		return NULL;
	}
		else if(w > image->header.width_px || h > image->header.height_px){
		if(*error == NULL){
			char* message = "Unable to crop the image";
			*error = malloc(sizeof(*error)*strlen(message) + 1);
			strcpy(*error, message);
			return NULL;
		}
		return NULL;
	}
	else if(h == 0 || w == 0){
		if(*error == NULL){
			char* message = "Unable to crop the image";
			*error = malloc(sizeof(*error)*strlen(message) + 1);
			strcpy(*error, message);
			return NULL;
		}
		return NULL;
	}
	else{
		bmp->header = image->header;
		bmp->header.width_px = w;
		bmp->header.height_px = h;
		bmp->header.image_size_bytes = w*h*image->header.bits_per_pixel/8;
		bmp->header.size = 54 + bmp->header.image_size_bytes;
		int width = image->header.width_px;
		int height = image->header.height_px;
		int bppixel = image->header.bits_per_pixel;
		width = image->header.width_px;
		height = image->header.height_px;
		bppixel = image->header.bits_per_pixel;
		int linebyte = (width*bppixel/8 + 3)/4*4;
		bmp->header.image_size_bytes = h*linebyte;
		bmp->header.size = 54 + bmp->header.image_size_bytes;
		int crop_linebyte = (w*bppixel/8 + 3)/4*4;
		int crop_padding = crop_linebyte - w*bppixel/8;
		int startpoint = x*bppixel/8 + linebyte*(height - h - y);
		bmp->data = malloc(sizeof(unsigned char)*h*crop_linebyte);
		int index = 0;
		int index2 = 0;
		while(index2 < h){
			int index3 = 0;
			while(index3 < bppixel*w/8){
				bmp->data[index] = image->data[startpoint + index3];
				index++;
				index3++;
			}
			if(crop_padding > 0){
				for(int index4 = 0; index4 < crop_padding; index4++){
					bmp->data[index + index4] = 0;
				}
				index += crop_padding;
			}
			startpoint = startpoint + bppixel*width/8;
			index2++;
		}
		return bmp;
	}
}


