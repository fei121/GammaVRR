#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gammavrr.h"
#include "regMgr.h"
#include <vector>
#include "stdint.h"
#include "HV7607B_InterFace_GammaVrr.h"

static bool Read_Config(FILE* f_config, char *InImgName, char InLUTName[3][1024], char *regName, char *GammaResultName)
{
	fscanf(f_config, "%s", InImgName);
	fscanf(f_config, "%*[^\n]");
	fscanf(f_config, "%s", *InLUTName++);
	fscanf(f_config, "%*[^\n]");
	fscanf(f_config, "%s", *InLUTName++);
	fscanf(f_config, "%*[^\n]");
	fscanf(f_config, "%s", *InLUTName);
	fscanf(f_config, "%*[^\n]");
	fscanf(f_config, "%s", regName);
	fscanf(f_config, "%*[^\n]");
	fscanf(f_config, "%s", GammaResultName);
	fscanf(f_config, "%*[^\n]");

	return true;
}

//int main(int argc, char* argv[])
int main()
{

	char CfgName[1024];
	char InImgName[1024];
	char InLUTName[3][1024];
	char RegName[1024];
	char OutPutFolderPath[1024];


	FILE* f_config = NULL;

	////////////////////////////////////////////
	////0. prepare 
	////////////////////////////////////////////

	printf("****************************************************************\n");
	printf("GammaVRR\n");
	printf("Date: 2025-04-02\n");
	printf("****************************************************************\n");


	/*if (argc < 2)
	{
		printf("    1: Config file (path\\name)\n");
		sprintf(CfgName, "./input/Config.txt");
		if ((f_config = fopen(CfgName, "r")) != NULL)
		{
			Read_Config(f_config, InImgName, InLUTName, RegName, OutPutFolderPath);
		}
		else
		{
			printf("can not find config file (path\\name)\n");
		}
	}
	else if (argc == 2)
	{
		strcpy(CfgName, argv[1]);
		if ((f_config = fopen(CfgName, "r")) != NULL)
		{
			Read_Config(f_config, InImgName, InLUTName, RegName, OutPutFolderPath);
		}
		else
		{
			printf("can not find config file (path\\name)\n");
		}

		fclose(f_config);
	}
	else if (argc >= 6)
	{
		strcpy(InImgName, argv[2]);
		strcpy(RegName, argv[4]);
		strcpy(OutPutFolderPath, argv[5]);
	}
	else
	{
		printf("ERROR: Not find config file or No enough input!\n");
		return 0;
	}*/
	sprintf(CfgName, "./input/Config.txt");
	f_config = fopen(CfgName, "r");
	Read_Config(f_config, InImgName, InLUTName, RegName, OutPutFolderPath);
	//read register
	bool show_cfg_en = 1;
	int ImgWidth, ImgHeight;
	HV7607B_LoadAlgParaGammaVrr(RegName, show_cfg_en, &ImgWidth, &ImgHeight);


	printf("Input Image:  %s\n", InImgName);
	printf("Input LUT R: %s\n", InLUTName[0]);
	printf("Input LUT G: %s\n", InLUTName[1]);
	printf("Input LUT B: %s\n", InLUTName[2]);
	printf("Gamma result: %s\n", OutPutFolderPath);
	int SubPixelNum = 3;

	unsigned int*  InputImage = NULL;
	unsigned int*  OutputImage = NULL;

	PicStruct* imageIn = (PicStruct*)calloc(1, sizeof(PicStruct));
	PicStruct* imageOut = (PicStruct*)calloc(1, sizeof(PicStruct));
	getPPMInfo(InImgName, imageIn);
	if (ImgWidth != imageIn->w || ImgHeight != imageIn->h) {
		printf("The width or height in CFG is different from that in PPM\n");
		printf("cfg_w=%d, cfg_h=%d, ppm_w=%d, ppm_h=%d\n", ImgWidth, ImgHeight, imageIn->w, imageIn->h);
		getchar();
	}

	imageIn->dataBuffer = (unsigned int*)calloc(ImgWidth*ImgHeight*SubPixelNum, sizeof(unsigned int));
	imageOut->dataBuffer = (unsigned int*)calloc(ImgWidth*ImgHeight*SubPixelNum, sizeof(unsigned int));
	readPPMToBuffer(InImgName, imageIn);

	HV7607B_RunGammaVrr(InLUTName, imageIn, imageOut, OutPutFolderPath);
	char finalDumpPath[255];
	sprintf(finalDumpPath, "%s/%s", OutPutFolderPath, "HV7607B_OUT_GammaVrr.ppm");
	writeBufferToPPM(imageOut, finalDumpPath);

	free(imageIn->dataBuffer);
	free(imageOut->dataBuffer);
	free(imageIn);
	free(imageOut);
	//getchar();
	//system("pause");
	return 0;
}
