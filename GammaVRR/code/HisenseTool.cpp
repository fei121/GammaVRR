#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include "HisenseTool.h"
/*
 * Load the dynamic link library.
 * Parameters:
 *   libraryPath	- The file path of dynamic link library.
 * Returns:
 *   A handle that has been loaded with a dynamic link library.
 */
void* loadLibrary(const char* libraryPath) {
#ifdef _WIN32
	HMODULE handle = LoadLibraryA(libraryPath);
	if (!handle) {
		printf("Failed to load library: %s\n", libraryPath);
		getchar();
	}
	return (void*)handle;
#else
	void* handle = dlopen(libraryPath, RTLD_LAZY);
	if (!handle) {
		printf("Failed to load library: %s\n", dlerror());
		getchar();
	}
	return handle;
#endif
}

/*
 * Load a function with the specified name from the dynamic link library.
 * Parameters:
 *   libraryHandle	- The handle that has been loaded with a dynamic link library.
  *   functionName	- Specified function name.
 * Returns:
 *   A pointer to a specified function.
 */
void* getFunction(void* libraryHandle, const char* functionName) {
#ifdef _WIN32
	void* func = GetProcAddress((HMODULE)libraryHandle, functionName);
	if (!func) {
		printf("Failed to find function: %s\n", functionName);
		getchar();
	}
	return func;
#else
	dlerror(); 
	void* func = dlsym(libraryHandle, functionName);
	char* dlsymError = dlerror();
	if (dlsymError) {
		printf("Failed to find function: %s\n", dlsymError);
		getchar();
		return NULL;
	}
	return func;
#endif
}

/*
 * Free the dynamic link library has already loaded.
 * Parameters:
 *   libraryHandle	- The handle that has been loaded with a dynamic link library.
 * Returns:
 *   NULL.
 */
void unloadLibrary(void* libraryHandle) {
#ifdef _WIN32
	if (libraryHandle) {
		FreeLibrary((HMODULE)libraryHandle);
	}
#else
	if (libraryHandle) {
		dlclose(libraryHandle);
	}
#endif
}

/*
 * Copy image info from one image struct to another.
 * Image info include: w, h, maxValue, bitDepth, ppmType.
 * Parameters:
 *   file_path	- Src image struct.
 *   image		- Dst image struct.
 * Returns:
 *   Operation flag. 0:success; -1:failed;
 */
int copyPPMInfo(PicStruct* image_src, PicStruct* image_dst) {
	image_dst->w = image_src->w;
	image_dst->h = image_src->h;
	image_dst->maxValue = image_src->maxValue;
	image_dst->bitDepth = image_src->bitDepth;
	image_dst->ppmType = image_src->ppmType;
	return 0;
}

/*
 * Copy image data from one image struct to another.
 * Parameters:
 *   file_path	- Src image struct.
 *   image		- Dst image struct.
 * Returns:
 *   Operation flag. 0:success; -1:failed;
 */
int copyPPMData(PicStruct* image_src, PicStruct* image_dst) {
	int cpt_num = 3;
	int pix_num = image_src->w * image_src->h * cpt_num;
	memcpy(image_dst->dataBuffer, image_src->dataBuffer, pix_num * sizeof(unsigned int));
	return 0;
}

/*
 * Get image info from ppm file. 
 * Image info include: w, h, maxValue, bitDepth, ppmType.
 * Parameters:
 *   file_path	- The path to a PPM file.
 *   image		- The struct of image.
 * Returns:
 *   Operation flag. 0:success; -1:failed;
 */
int getPPMInfo(const char *file_path, PicStruct* image) {
	FILE *f_p = fopen(file_path, "rb");
	if (!f_p) {
		printf("can not open file_path: %s\n", file_path);
		getchar();
		return -1;
	}
	char format[3];
	if (!fgets(format, sizeof(format), f_p)) {
		perror("Failed to read the file format\n");
		fclose(f_p);
		getchar();
		return -1;
	}

	if (strcmp(format, "P3") != 0 && strcmp(format, "P6") != 0) {
		printf("PPM format is not supported: %s\n", format);
		fclose(f_p);
		getchar();
		return -1;
	}

	if (fscanf(f_p, "%d %d %d", &(image->w), &(image->h), &(image->maxValue)) != 3) {
		perror("Failed to read image size or maximum color value\n");
		fclose(f_p);
		getchar();
		return -1;
	}

	char tmpStr[2] = { format[1] , '\0' };
	image->ppmType = atoi(tmpStr);

	image->bitDepth = 0;
	while (image->maxValue >= (1 << image->bitDepth))
	{
		image->bitDepth++;
	}
	fclose(f_p);
	return 0;
}

/*
 * Read the contents of the ppm file into a buffer(unsigned int*).
 * Parameters:
 *   file_path	- The path to a PPM file.
 *   image		- The struct of image.
 * Returns:
 *   Operation flag. 0:success; -1:failed;
 */
int readPPMToBuffer(const char *file_path, PicStruct* image) {
	int cpt_num = 3;
	FILE *f_p = fopen(file_path, "rb");
	if (!f_p) {
		printf("can not open file_path: %s\n", file_path);
		getchar();
		return -1;
	}

	char format[3];
	if (!fgets(format, sizeof(format), f_p)) {
		perror("Failed to read the file format\n");
		fclose(f_p);
		getchar();
		return -1;
	}

	if (strcmp(format, "P3") != 0 && strcmp(format, "P6") != 0) {
		printf("PPM format is not supported: %s\n", format);
		fclose(f_p);
		getchar();
		return -1;
	}

	if (fscanf(f_p, "%d %d %d", &(image->w), &(image->h), &(image->maxValue)) != 3) {
		perror("Failed to read image size or maximum color value\n");
		fclose(f_p);
		getchar();
		return -1;
	}

	fgetc(f_p);

	if (strcmp(format, "P3") == 0) {
		for (int i = 0; i < (image->w * image->h); i++) {
			int r, g, b;
			if (fscanf(f_p, "%d %d %d", &r, &g, &b) != 3) {
				perror("Failed to read pixel data\n");
				fclose(f_p);
				getchar();
				return -1;
			}
			image->dataBuffer[i * cpt_num + 0] = r;
			image->dataBuffer[i * cpt_num + 1] = g;
			image->dataBuffer[i * cpt_num + 2] = b;
		}
	}
	else if (strcmp(format, "P6") == 0) {
		for (int i = 0; i < (image->w * image->h); i++) {
			unsigned char rgb[3];
			if (fread(rgb, sizeof(unsigned char), 3, f_p) != 3) {
				perror("Failed to read pixel data\n");
				fclose(f_p);
				getchar();
				return -1;
			}
			image->dataBuffer[i * cpt_num + 0] = rgb[0];
			image->dataBuffer[i * cpt_num + 1] = rgb[1];
			image->dataBuffer[i * cpt_num + 2] = rgb[2];
		}
	}

	fclose(f_p);
	printf("Read PPM success: %s\n", file_path);
	return 0;
}

/*
 * Write the data in the buffer to a PPM file.
 * Parameters:
 *   image		- The struct of image.
 *   file_path	- The path to a PPM file.
 * Returns:
 *   Operation flag. 0:success; -1:failed;
 */
int writeBufferToPPM(PicStruct* image, const char *file_path) {
	int cpt_num = 3;
	if (image->ppmType != 3 && image->ppmType != 6) {
		printf("Unsupported formats: P%d\n", image->ppmType);
		getchar();
		return -1;
	}

	if (image->ppmType == 3) {
		FILE *f_p = fopen(file_path, "wt");
		if (!f_p) {
			printf("can not open file_path: %s\n", file_path);
			getchar();
			getchar();
			return -1;
		}
		fprintf(f_p, "P%d\n%d %d\n%d\n", image->ppmType, image->w, image->h, image->maxValue);
		for (int i = 0; i < (image->w * image->h); i++) {
			if (image->maxValue < (1 << 8)) {
				unsigned char rgb[3] = { 0 };
				rgb[0] = (unsigned char)(image->dataBuffer[i * cpt_num + 0]);
				rgb[1] = (unsigned char)(image->dataBuffer[i * cpt_num + 1]);
				rgb[2] = (unsigned char)(image->dataBuffer[i * cpt_num + 2]);
				fprintf(f_p, "%d %d %d\n", rgb[0], rgb[1], rgb[2]);
			}
			else if (image->maxValue < (1 << 16)) {
				unsigned short rgb[3] = { 0 };
				rgb[0] = (unsigned short)(image->dataBuffer[i * cpt_num + 0]);
				rgb[1] = (unsigned short)(image->dataBuffer[i * cpt_num + 1]);
				rgb[2] = (unsigned short)(image->dataBuffer[i * cpt_num + 2]);
				fprintf(f_p, "%d %d %d\n", rgb[0], rgb[1], rgb[2]);
			}
			else
			{
				unsigned int rgb[3] = { 0 };
				rgb[0] = (unsigned int)(image->dataBuffer[i * cpt_num + 0]);
				rgb[1] = (unsigned int)(image->dataBuffer[i * cpt_num + 1]);
				rgb[2] = (unsigned int)(image->dataBuffer[i * cpt_num + 2]);
				fprintf(f_p, "%d %d %d\n", rgb[0], rgb[1], rgb[2]);
			}
		}
		fclose(f_p);
	}
	else
	{
		FILE *f_p = fopen(file_path, "wb");
		if (!f_p) {
			printf("can not open file_path: %s\n", file_path);
			getchar();
			getchar();
			return -1;
		}
		fprintf(f_p, "P%d\n%d %d\n%d\n", image->ppmType, image->w, image->h, image->maxValue);
		for (int i = 0; i < (image->w * image->h); i++) {
			if (image->maxValue < (1 << 8)) {
				unsigned char rgb[3] = { 0 };
				rgb[0] = (unsigned char)(image->dataBuffer[i * cpt_num + 0]);
				rgb[1] = (unsigned char)(image->dataBuffer[i * cpt_num + 1]);
				rgb[2] = (unsigned char)(image->dataBuffer[i * cpt_num + 2]);
				fwrite(rgb, sizeof(unsigned char), 3, f_p);
			}
			else if (image->maxValue < (1 << 16)) {
				unsigned short rgb[3] = { 0 };
				rgb[0] = (unsigned short)(image->dataBuffer[i * cpt_num + 0]);
				rgb[1] = (unsigned short)(image->dataBuffer[i * cpt_num + 1]);
				rgb[2] = (unsigned short)(image->dataBuffer[i * cpt_num + 2]);
				fwrite(rgb, sizeof(unsigned short), 3, f_p);
			}
			else
			{
				unsigned int rgb[3] = { 0 };
				rgb[0] = (unsigned int)(image->dataBuffer[i * cpt_num + 0]);
				rgb[1] = (unsigned int)(image->dataBuffer[i * cpt_num + 1]);
				rgb[2] = (unsigned int)(image->dataBuffer[i * cpt_num + 2]);
				fwrite(rgb, sizeof(unsigned int), 3, f_p);
			}
		}
		fclose(f_p);
	}

	printf("Write PPM success: %s\n", file_path);
	return 0;
}