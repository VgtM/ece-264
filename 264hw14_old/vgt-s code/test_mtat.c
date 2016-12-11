#include <assert.h>
#include "math.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bmp.h"
#include "mtat.h"

int main(int argc, const char *argv[])
{
	char* error = NULL;
	FILE* fp = fopen("img2_384x510_gray.bmp", "r");
	BMPImage* tmp = read_bmp(fp, &error);
	FILE* fp1 = fopen("newimage_radius3_thread1.bmp", "w");
	BMPImage* bin = binarize(tmp, 3, 3, &error);
	write_bmp(fp1, bin, &error);
	fp1 = fopen("newimage_radius10_thread3.bmp", "w");
	bin = binarize(tmp, 10, 5, &error);
	write_bmp(fp1, bin, &error);
	/*fp1 = fopen("newimage_radius10_thread5.bmp", "w");
	bin = binarize(tmp, 10, 10, &error);
	write_bmp(fp1, bin, &error);
	fp1 = fopen("newimage_radius20_thread3.bmp", "w");
	bin = binarize(tmp, 20, 10, &error);
	write_bmp(fp1, bin, &error);
	fp1 = fopen("newimage_radius20_thread5.bmp", "w");
	bin = binarize(tmp, 20, 10, &error);
	write_bmp(fp1, bin, &error);
	fp1 = fopen("newimage_radius2_thread3.bmp", "w");
	bin = binarize(tmp, 2, 10, &error);
	write_bmp(fp1, bin, &error);*/
	fp = fopen("img1_6x6_color.bmp", "r");
	tmp = read_bmp(fp, &error);
	fp1 = fopen("img1_6x6_bw_r_05.bmp", "w");
	bin = binarize(tmp, 5, 10, &error);
	write_bmp(fp1, bin, &error);
	/*FILE* fp2 = fopen("img1_6x6_color.bmp", "r");
	tmp = read_bmp(fp, &error);
	FILE* fp3 = fopen("img1_6x6_bwcrop.bmp", "w");
	BMPImage* crop = crop_bmp(tmp, 0, 0, tmp->header.width_px, tmp->header.height_px, &error);
	write_bmp(fp3,crop, &error);*/

	fclose(fp);
	fclose(fp1);
	free_bmp(tmp);
	free_bmp(bin);
	free(error);
	return 0;
}
