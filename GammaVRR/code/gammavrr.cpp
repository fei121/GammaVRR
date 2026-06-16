
#include <stdio.h>
#include <string.h>
#include <cmath>
#include "gammavrr.h"
#include "HV7607B_InterFace_GammaVrr.h"

CRegParam_Encode	reg("reg_");
_GAMMA_DATA			m_Data;


bool GammaVrrProc(_GAMMA_DATA * m_Data, CRegParam_Encode reg);
bool GammaInitial(_GAMMA_DATA * m_Data);
bool GammaRelease(_GAMMA_DATA * m_Data);
bool GammaOutGen(_GAMMA_DATA * m_Data);

static bool ReadGammaLUT(char InLUTName[3][1024], _GAMMA_DATA * m_Data);
static bool Read_ppm_Input(unsigned int** InputImage, FILE* in_fname, int * Width, int * Height, int * MaxValue);
static bool PPMDump(unsigned int *OutputImage, unsigned int *lutInfo, FILE* RGBPPMFile, int ImgWidth, int ImgHeight, int SubPixelNum, int MaxValue, int Mode);


#if 1
CRegParam_Encode::CRegParam_Encode(string prefix)
	: reg_para_test(prefix + "para_test", 5),
	reg_para_file_dump(prefix + "para_file_dump"),
	reg_para_Img_Width(prefix + "para_Img_Width"),
	reg_para_Img_Height(prefix + "para_Img_Height"),
	reg_para_gamma_vrr_en(prefix + "para_gamma_vrr_en"),
	reg_para_gamma_vrr_zero_setting(prefix + "para_gamma_vrr_zero_setting"),
    regr_para_frame_rate_level_0(prefix + "para_frame_rate_level_0"),
	reg_para_gamma_vrr_sw_en(prefix + "para_gamma_vrr_sw_en"),
    reg_para_gamma_vrr_frame_level(prefix + "para_gamma_vrr_frame_level")
{
}

CRegParam_Encode :: ~CRegParam_Encode()
{

}
#endif



bool GammaVrrProc(_GAMMA_DATA * m_Data, CRegParam_Encode reg,int MaxValue)
{
	unsigned  int i = 0, j = 0, k = 0, Idx = 0;
	unsigned  int ImgH = m_Data->InImgH;
	unsigned  int ImgW = m_Data->InImgW;
	unsigned  int subpixelNum = m_Data->SubPixelNum;
	unsigned  int Temp;
	unsigned  int TempResult;
	unsigned  int ImageIndex;
	unsigned  int GrayIndex;
	unsigned  int Detal;
	int16_t L0 = 0;
	int16_t L1 = 0;
	int16_t R0 = 0;
	int16_t R1 = 0;
	int16_t interp_frame[256];
	unsigned int idx1 = 0, idx2 = 0, idx3 = 0,idx4 = 0;
	int offset, coef;

	//unsigned int InterpStep = m_Data->GammaLUTbit - m_Data->GammaStep;
	//unsigned int InterpDistane = 1 << (m_Data->GammaLUTbit - m_Data->GammaStep);
	//unsigned int index = 0;
	int FrameLevel = reg.reg_para_gamma_vrr_sw_en.mValue == 0 ? reg.regr_para_frame_rate_level_0.mValue : reg.reg_para_gamma_vrr_frame_level.mValue;

	for (i = 0; i < ImgH; i++)
	{
		for (j = 0; j < ImgW; j++)
		{
			for (k = 0; k < subpixelNum; k++)
			{ 
				ImageIndex = (i*ImgW*subpixelNum + j * subpixelNum) + k;
				Temp = m_Data->InputImg[ImageIndex];
				if (MaxValue == 4095) 
				{
					GrayIndex = (Temp >> 4) & 0xFF;
					Detal = Temp & 0xF;
					idx1 = CLIP((FrameLevel >> 4), 0, 7);
					idx2 = CLIP((idx1 + 1), 0, 7);
					idx3 = CLIP(GrayIndex, 0, 255);
					idx4 = CLIP((idx3 + 1), 0, 255);
					coef = FrameLevel & 0xF;
				}
				
				if (k == 0)
				{
					L0 = m_Data->GammaLUT_R[idx1][idx3];
					R0 = m_Data->GammaLUT_R[idx1][idx4];
					L1 = m_Data->GammaLUT_R[idx2][idx3];
					R1 = m_Data->GammaLUT_R[idx2][idx4];
					interp_frame[idx1] = round((double)(L0 * (16 - Detal) + R0 * Detal) / 16);
					interp_frame[idx2] = round((double)(L1 * (16 - Detal) + R1 * Detal) / 16);
					offset = round((double)(interp_frame[idx1] * (16 - coef) + interp_frame[idx2] * coef) /16) - reg.reg_para_gamma_vrr_zero_setting.mValue;
					
				}
				else if (k == 1)
				{
					L0 = m_Data->GammaLUT_G[idx1][idx3];
					R0 = m_Data->GammaLUT_G[idx1][idx4];
					L1 = m_Data->GammaLUT_G[idx2][idx3];
					R1 = m_Data->GammaLUT_G[idx2][idx4];
					interp_frame[idx1] = round((double)(L0 * (16 - Detal) + R0 * Detal) / 16);
					interp_frame[idx2] = round((double)(L1 * (16 - Detal) + R1 * Detal) / 16);
					offset = round((double)(interp_frame[idx1] * (16 - coef) + interp_frame[idx2] * coef) / 16) - reg.reg_para_gamma_vrr_zero_setting.mValue;

				}
				else
				{
					L0 = m_Data->GammaLUT_B[idx1][idx3];
					R0 = m_Data->GammaLUT_B[idx1][idx4];
					L1 = m_Data->GammaLUT_B[idx2][idx3];
					R1 = m_Data->GammaLUT_B[idx2][idx4];
					interp_frame[idx1] = round((double)(L0 * (16 - Detal) + R0 * Detal) / 16);
					interp_frame[idx2] = round((double)(L1 * (16 - Detal) + R1 * Detal) / 16);
					offset = round((double)(interp_frame[idx1] * (16 - coef) + interp_frame[idx2] * coef) / 16) - reg.reg_para_gamma_vrr_zero_setting.mValue;
				}
				int out = m_Data->InputImg[ImageIndex] + offset;
				m_Data->OutputImg[ImageIndex] = CLIP(out, 0, 4095);

			}

		}
	}


	return true;
}

bool GammaInitial(_GAMMA_DATA * m_Data)
{
	m_Data->InImgW = reg.reg_para_Img_Width.mValue;
	m_Data->InImgH = reg.reg_para_Img_Height.mValue;
	m_Data->SubPixelNum = 3;

	m_Data->GammaOutput = new unsigned int[m_Data->InImgW * m_Data->InImgH * m_Data->SubPixelNum];
	if (NULL == m_Data->GammaOutput)
	{
		printf("gamma output buffer malloc error!\n");
		return false;
	}

	return true;
}

bool GammaRelease(_GAMMA_DATA * m_Data)
{


	if (NULL != m_Data->GammaOutput)
	{
		delete[] m_Data->GammaOutput;
		m_Data->GammaOutput = NULL;
	}


	return true;
}


bool GammaOutGen(_GAMMA_DATA * m_Data)
{
	unsigned int * GammaResult = m_Data->OutputImg;
	unsigned int * InputImg = m_Data->InputImg;


	unsigned int ImgW = (int)m_Data->InImgW;
	unsigned int ImgH = (int)m_Data->InImgH;
	unsigned int BlendShift = 0;
	//unsigned int * InImg;




	unsigned int i, j;



	unsigned int GainEn = 0;
	unsigned int Idx = 0;
	long long	 GainOne = 1;
	int			UsedInput = 0;

	for (i = 0; i < ImgH; i++)
	{

		for (j = 0; j < ImgW; j++)
		{
			Idx = i * ImgW + j;
		}
	}
	return true;
}

bool HV7607B_DllTestGammaVrr() {
	printf("HV7607B_InterFace_GammaVrr DLL is good\n");
	return true;
}

bool HV7607B_LoadAlgParaGammaVrr(char* cfg_path, bool show_cfg_en, int* w, int* h) {
	//read register
	fstream regInput;
	regInput.open(cfg_path, fstream::in);
	CRegList::FillAllRegisters(regInput, show_cfg_en);
	regInput.close();
	(*w) = reg.reg_para_Img_Width.mValue;
	(*h) = reg.reg_para_Img_Height.mValue;
	return true;
}

bool HV7607B_RunGammaVrr(char InLUTName[3][1024], PicStruct* imageIn, PicStruct* imageOut, char* OutPutFolderPath) {
	int gammavrr_en = reg.reg_para_gamma_vrr_en.mValue;
	int ImgHeight = reg.reg_para_Img_Height.mValue;
	int ImgWidth = reg.reg_para_Img_Width.mValue;
	int Level = 8;
	int MaxValue = imageIn->maxValue;
	int SubPixelNum = 3;
	int gammaStepCount = 1 << 8;
	m_Data.InputImg = new unsigned int[ImgWidth*ImgHeight*SubPixelNum];
	m_Data.OutputImg = new unsigned int[ImgWidth*ImgHeight*SubPixelNum];
	memcpy(m_Data.InputImg, imageIn->dataBuffer, ImgWidth*ImgHeight*SubPixelNum * sizeof(unsigned int));

	m_Data.GammaLUT_R = new int16_t*[Level  * GAMMALUTSET];
	m_Data.GammaLUT_G = new int16_t*[Level  * GAMMALUTSET];
	m_Data.GammaLUT_B = new int16_t*[Level  * GAMMALUTSET];

	for (int i = 0; i < Level; i++) {
		m_Data.GammaLUT_R[i] = new int16_t[gammaStepCount];
		m_Data.GammaLUT_G[i] = new int16_t[gammaStepCount];
		m_Data.GammaLUT_B[i] = new int16_t[gammaStepCount];
	}

	ReadGammaLUT(InLUTName, &m_Data);

	GammaInitial(&m_Data);
	GammaVrrProc(&m_Data, reg, MaxValue);

	if (reg.reg_para_file_dump.mValue == 1) {
		char FileNameTmp[1024];
		sprintf(FileNameTmp, "%s/%s", OutPutFolderPath, "GammaResult.ppm");
		FILE * RGBPPMFile = fopen(FileNameTmp, "wt");
		PPMDump(m_Data.OutputImg, m_Data.OutputLutData, RGBPPMFile, ImgWidth, ImgHeight, SubPixelNum, MaxValue, 0);
		fclose(RGBPPMFile);
	}

	memcpy(imageOut->dataBuffer, m_Data.OutputImg, ImgWidth*ImgHeight*SubPixelNum * sizeof(unsigned int));
	copyPPMInfo(imageIn, imageOut);
	if (NULL != m_Data.InputImg)
	{
		delete[] m_Data.InputImg;
	}
	if (NULL != m_Data.InputImg_12bit)
	{
		delete[] m_Data.InputImg_12bit;
	}
	if (NULL != m_Data.OutputImg)
	{
		delete[] m_Data.OutputImg;
	}
	if (NULL != m_Data.OutputLutData)
	{
		delete[] m_Data.OutputLutData;
	}

	if (NULL != m_Data.GammaLUT_R)
	{
		delete[] m_Data.GammaLUT_R;
	}
	if (NULL != m_Data.GammaLUT_G)
	{
		delete[] m_Data.GammaLUT_G;
	}
	if (NULL != m_Data.GammaLUT_B)
	{
		delete[] m_Data.GammaLUT_B;
	}
	GammaRelease(&m_Data);
	printf("Processing Finish!\n");
	printf("****************************************************************\n\n");
	return true;
}

static bool Read_ppm_Input(unsigned int**InputImage, FILE* in_fname, int * Width, int * Height, int * MaxValue)
{

	char iS[1024];
	unsigned int iX;
	unsigned int W, H, MaxV;
	unsigned int i, j;

	fscanf(in_fname, "%s\n", iS);	//p3, p6

	fscanf(in_fname, "%d\t", &iX);
	W = iX;

	fscanf(in_fname, "%d\n", &iX);
	H = iX;

	fscanf(in_fname, "%d\n", &iX);
	MaxV = iX;

	unsigned int *TmpImage = new unsigned int[W * H * 3];

	for (i = 0; i < H; i++)
	{
		for (j = 0; j < W; j++)
		{
			fscanf(in_fname, "%d", &iX);
			TmpImage[(i*W + j) * 3 + 0] = iX;

			fscanf(in_fname, "%d", &iX);
			TmpImage[(i*W + j) * 3 + 1] = iX;

			fscanf(in_fname, "%d", &iX);
			TmpImage[(i*W + j) * 3 + 2] = iX;
		}
	}

	*Width = W;
	*Height = H;
	*MaxValue = MaxV;
	*InputImage = TmpImage;

	return true;
}

static bool PPMDump(unsigned int *OutputImage, unsigned int *lutInfo, FILE* RGBPPMFile, int ImgWidth, int ImgHeight, int SubPixelNum, int MaxValue, int Mode)
{
	int i, j;
	int SubpixelIdx[6] = { 0,1,2,3,4,5 };

	if ((Mode == 0) || (Mode == 1))
	{
		SubpixelIdx[0] = 0;
		SubpixelIdx[1] = 1;
		SubpixelIdx[2] = 2;
		SubpixelIdx[3] = 3;
		SubpixelIdx[4] = 4;
		SubpixelIdx[5] = 5;
	}
	else
	{
		SubpixelIdx[0] = 3;
		SubpixelIdx[1] = 3;
		SubpixelIdx[2] = 3;
		SubpixelIdx[3] = 3;
		SubpixelIdx[4] = 4;
		SubpixelIdx[5] = 5;
	}

	fprintf(RGBPPMFile, "%s\n", "P3");
	fprintf(RGBPPMFile, "%d ", ImgWidth);
	fprintf(RGBPPMFile, "%d\n", ImgHeight);
	fprintf(RGBPPMFile, "%d\n", MaxValue);

	for (i = 0; i < ImgHeight; i++)
	{
		for (j = 0; j < ImgWidth; j++)
		{

			fprintf(RGBPPMFile, "%d ", OutputImage[(i*ImgWidth + j)*SubPixelNum + SubpixelIdx[0]]);
			fprintf(RGBPPMFile, "%d ", OutputImage[(i*ImgWidth + j)*SubPixelNum + SubpixelIdx[1]]);
			fprintf(RGBPPMFile, "%d\n", OutputImage[(i*ImgWidth + j)*SubPixelNum + SubpixelIdx[2]]);
		}
	}

	return true;
}


static bool ReadGammaLUT(char InLUTName[3][1024], _GAMMA_DATA * m_Data)
{
	unsigned int m;
	unsigned int tmp0 = 0;
	unsigned int LUTSet = GAMMALUTSET;
	unsigned int MaxSubPixelNum = MAXSUBPIXELNUM;

	int k = 0;
	int16_t **LUT = NULL;
	char *name = NULL;
	while (k < 3) {
		if (k == 0) {
			name = InLUTName[0];
			LUT = m_Data->GammaLUT_R;
		}
		else if (k == 1) {
			name = InLUTName[1];
			LUT = m_Data->GammaLUT_G;
		}
		else if (k == 2) {
			name = InLUTName[2];
			LUT = m_Data->GammaLUT_B;
		}
		FILE* file = fopen(name, "rb");
		if (file == NULL) {
			printf("open file is failed");
			return false;
		}

		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 256; j++) {
				if (fscanf(file, "%d", &tmp0) != 1) {
					printf("1\n");
					fclose(file);
					return false;
				}
				LUT[i][j] = tmp0;
			}

		}

		fclose(file);
		++k;
	}
	return true;
}