#ifndef _DEMURA_H_
#define _DEMURA_H_
#include "regMgr.h"


#define IN_OUT_CONFIG_FILE "IN_OUT.cfg"
#define DEBUGEN					0
#define	POSTGAMMAEN				1
#define _GAMMA_BLEND_FUNC_EN_	1
#define OUTPUT_LUT_INFO			1


#define _GAMMA_BLEND_FACTOR_CURVE_STEP_BIT_		4
#define _GAMMA_BLEND_FACTOR_CURVE_STEP_			((1<<_GAMMA_BLEND_FACTOR_CURVE_STEP_BIT_)+1)
#define _GAMMA_BLEND_FACTOR_INTERPOLATION_BIT_	4
#define _GAMMA_BLEND_FACTOR_BIT_				8
#define _GAMMA_BLEND_GLBGAIN_BIT_				9

#define DATADUMPEN			1
#define DEBUGDATADUMPEN		1
#define DECDUMPEN			0

#define INIMGBIT			10
#define GAMMAOUTBIT			12
#define MAXSUBPIXELNUM		3
#define GAMMALUTSET			1    //多张表使用
#define GAMMAOUTPUTBIT		12	

#define GAMMAPATTERNROW		4
#define GAMMAPATTERNCOL		8
#define GAMMABLENDBIT		10

#define HSVSUBPIXELNUM		3
#define MAXSATBIT			10
#define MAXSAT				((1<<MAXSATBIT) - 1)



#define ABS(a)	((a) > 0 ? (a) : (0 - (a)))
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define CLIP(x, a, b)	(MIN(MAX((x), (a)), (b)))

class CRegParam_Encode
{
public:
	CRegParam_Encode(string prefix);
	~CRegParam_Encode();


	//Gamma param
	CRegMgr<int>				reg_para_test;
	CRegMgr<int>				reg_para_file_dump;//1bits
	CRegMgr<int>				reg_para_Img_Width;//12bits
	CRegMgr<int>				reg_para_Img_Height;//12bits
	CRegMgr<int>				reg_para_gamma_vrr_en;//1bit 
	CRegMgr<int>                reg_para_gamma_vrr_zero_setting;
	CRegMgr<int>                regr_para_frame_rate_level_0;
	CRegMgr<int>                reg_para_gamma_vrr_sw_en;
	CRegMgr<int>                reg_para_gamma_vrr_frame_level;

};



typedef struct _GAMMA_DATA_s
{
	unsigned int * InputImg;
	unsigned int * InputImg_12bit;
	unsigned int * OutputImg;
	unsigned int * OutputLutData;
	unsigned int InImgW;
	unsigned int InImgH;
	unsigned int SubPixelNum;
	unsigned int GammaStep;
	unsigned int GammaLUTbit;

	int16_t ** GammaLUT_R;
	int16_t ** GammaLUT_G;
	int16_t ** GammaLUT_B;
	unsigned int * GammaOutput;

} _GAMMA_DATA;


bool GammaVrrProc(_GAMMA_DATA * m_Data, CRegParam_Encode reg, int MaxValue);
bool GammaInitial(_GAMMA_DATA * m_Data);
bool GammaRelease(_GAMMA_DATA * m_Data);
#endif

