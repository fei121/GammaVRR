#ifndef HISENSE_TOOL_H
#define HISENSE_TOOL_H

typedef struct PicStruct {
	unsigned int* dataBuffer;
	unsigned int w;
	unsigned int h;
	unsigned int maxValue;
	unsigned int bitDepth;
	unsigned int ppmType;
}PicStruct;

/*
 * Load the dynamic link library.
 * Parameters:
 *   libraryPath	- The file path of dynamic link library.
 * Returns:
 *   A handle that has been loaded with a dynamic link library.
 */
void* loadLibrary(const char* libraryName);

/*
 * Load a function with the specified name from the dynamic link library.
 * Parameters:
 *   libraryHandle	- The handle that has been loaded with a dynamic link library.
  *   functionName	- Specified function name.
 * Returns:
 *   A pointer to a specified function.
 */
void* getFunction(void* libraryHandle, const char* functionName);

/*
 * Free the dynamic link library has already loaded.
 * Parameters:
 *   libraryHandle	- The handle that has been loaded with a dynamic link library.
 * Returns:
 *   NULL.
 */
void unloadLibrary(void* libraryHandle);

/*
 * Copy image info from one image struct to another.
 * Image info include: w, h, maxValue, bitDepth, ppmType.
 * Parameters:
 *   file_path	- Src image struct.
 *   image		- Dst image struct.
 * Returns:
 *   Operation flag. 0:success; -1:failed;
 */
int copyPPMInfo(PicStruct* image_src, PicStruct* image_dst);

/*
 * Copy image data from one image struct to another.
 * Parameters:
 *   file_path	- Src image struct.
 *   image		- Dst image struct.
 * Returns:
 *   Operation flag. 0:success; -1:failed;
 */
int copyPPMData(PicStruct* image_src, PicStruct* image_dst);

/*
 * Get image info from ppm file.
 * Image info include: w, h, maxValue, bitDepth, ppmType.
 * Parameters:
 *   file_path	- The path to a PPM file.
 *   image		- The struct of image.
 * Returns:
 *   Operation flag. 0:success; -1:failed;
 */
int getPPMInfo(const char *file_path, PicStruct* image);

/*
 * Read the contents of the ppm file into a buffer(unsigned int*).
 * Parameters:
 *   file_path	- The path to a PPM file.
 *   image		- The struct of image.
 * Returns:
 *   Operation flag. 0:success; -1:failed;
 */
int readPPMToBuffer(const char *file_path, PicStruct* image);

/*
 * Write the data in the buffer to a PPM file.
 * Parameters:
 *   image		- The struct of image.
 *   file_path	- The path to a PPM file.
 * Returns:
 *   Operation flag. 0:success; -1:failed;
 */
int writeBufferToPPM(PicStruct* image, const char *file_path);

#endif // HISENSE_TOOL_H

