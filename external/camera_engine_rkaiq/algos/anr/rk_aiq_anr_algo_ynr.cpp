
#include "rk_aiq_anr_algo_ynr.h"

RKAIQ_BEGIN_DECLARE

ANRresult_t ynr_get_mode_cell_idx_by_name(CalibDb_YNR_2_t *pCalibdb, char *name, int *mode_idx)
{
	int i = 0;
	ANRresult_t res = ANR_RET_SUCCESS;

	if(pCalibdb == NULL){
		LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
		return ANR_RET_NULL_POINTER;
	}

	if(name == NULL){
		LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
		return ANR_RET_NULL_POINTER;
	}

	if(mode_idx == NULL){
		LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
		return ANR_RET_NULL_POINTER;
	}

	if(pCalibdb->mode_num < 1){
		LOGE_ANR("%s(%d): mfnr mode cell num is zero\n", __FUNCTION__, __LINE__);
		return ANR_RET_NULL_POINTER;
	}

	for(i=0; i<pCalibdb->mode_num; i++){
		if(strncmp(name, pCalibdb->mode_cell[i].name, sizeof(pCalibdb->mode_cell[i].name)) == 0){
			break;
		}
	}

	if(i<pCalibdb->mode_num){
		*mode_idx = i;
		res = ANR_RET_SUCCESS;
	}else{
		*mode_idx = 0;
		res = ANR_RET_FAILURE;
	}

	LOGD_ANR("%s:%d mode_name:%s  mode_idx:%d i:%d \n", __FUNCTION__, __LINE__, name, *mode_idx, i);
	return res;

}


ANRresult_t ynr_get_setting_idx_by_name(CalibDb_YNR_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx)
{
	int i = 0;
	ANRresult_t res = ANR_RET_SUCCESS;

	if(pCalibdb == NULL){
		LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
		return ANR_RET_NULL_POINTER;
	}

	if(name == NULL){
		LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
		return ANR_RET_NULL_POINTER;
	}

	if(setting_idx == NULL){
		LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
		return ANR_RET_NULL_POINTER;
	}

	for(i=0; i<CALIBDB_NR_SHARP_SETTING_LEVEL; i++){
		if(strncmp(name, pCalibdb->mode_cell[mode_idx].setting[i].snr_mode, sizeof(pCalibdb->mode_cell[mode_idx].setting[i].snr_mode)) == 0){
			break;
		}
	}

	if(i<CALIBDB_NR_SHARP_SETTING_LEVEL){
		*setting_idx = i;
		res = ANR_RET_SUCCESS;
	}else{
		*setting_idx = 0;
		res = ANR_RET_FAILURE;
	}

	LOGD_ANR("%s:%d snr_name:%s  snr_idx:%d i:%d \n", __FUNCTION__, __LINE__,name, *setting_idx, i);
	return res;

}

ANRresult_t ynr_config_setting_param(RKAnr_Ynr_Params_s *pParams, CalibDb_YNR_2_t *pCalibdb, char* param_mode, char* snr_name)
{
	ANRresult_t res = ANR_RET_SUCCESS;
	int mode_idx = 0;
	int setting_idx = 0;

	if(pParams == NULL){
		LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
		return ANR_RET_NULL_POINTER;
	}

	if(pCalibdb == NULL){
		LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
		return ANR_RET_NULL_POINTER;
	}

	res = ynr_get_mode_cell_idx_by_name(pCalibdb, param_mode, &mode_idx);
	if(res != ANR_RET_SUCCESS){
		LOGW_ANR("%s(%d): error!!!  can't find mode name in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
	}
	
	res = ynr_get_setting_idx_by_name(pCalibdb, snr_name, mode_idx, &setting_idx);
	if(res != ANR_RET_SUCCESS){
		LOGW_ANR("%s(%d): error!!!  can't find setting in iq files, use 0 instead\n", __FUNCTION__, __LINE__);
	}

	res = init_ynr_params(pParams, pCalibdb, mode_idx, setting_idx);

	return res;

}
ANRresult_t init_ynr_params(RKAnr_Ynr_Params_s *pYnrParams, CalibDb_YNR_2_t* pYnrCalib, int mode_idx, int setting_idx)
{
    ANRresult_t res = ANR_RET_SUCCESS;
    int i = 0;
    int j = 0;

    if(pYnrParams == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pYnrCalib == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    RKAnr_Ynr_Params_Select_t *pParams = pYnrParams->aYnrParamsISO;
	CalibDb_YNR_ISO_t *pCalibdb = pYnrCalib->mode_cell[mode_idx].setting[setting_idx].ynr_iso;

    short isoCurveSectValue;
    short isoCurveSectValue1;
    int bit_shift;
    int bit_proc;
    int bit_calib;

    bit_calib = 12;
    bit_proc = YNR_SIGMA_BITS;
    bit_shift = bit_calib - bit_proc;
    isoCurveSectValue =  (1 << (bit_calib - ISO_CURVE_POINT_BIT));//rawBit必须大于ISO_CURVE_POINT_BIT
    isoCurveSectValue1 =  (1 << bit_calib);// - 1;//rawBit必须大于ISO_CURVE_POINT_BIT, max use (1 << bit_calib);

#ifndef RK_SIMULATOR_HW
    for(j = 0; j < MAX_ISO_STEP; j++) {
        pParams[j].iso = pCalibdb[j].iso;
    }
#endif

    for(j = 0; j < MAX_ISO_STEP; j++) {
        for(i = 0; i < WAVELET_LEVEL_NUM; i++) {
            pParams[j].loFreqNoiseCi[i] = pCalibdb[j].ynr_lci[i];
            pParams[j].ciISO[i * 3 + 0] = pCalibdb[j].ynr_lhci[i];
            pParams[j].ciISO[i * 3 + 1] = pCalibdb[j].ynr_hlci[i];
            pParams[j].ciISO[i * 3 + 2] = pCalibdb[j].ynr_hhci[i];
        }

        for(i = 0; i < ISO_CURVE_POINT_NUM; i++) {
            float ave1, ave2, ave3, ave4;
            if(i == (ISO_CURVE_POINT_NUM - 1)) {
                ave1 = (float)isoCurveSectValue1;
            }
            else {
                ave1 = (float)(i * isoCurveSectValue);
            }
            pParams[j].lumaPoints[i] = (short)ave1;
            ave2 = ave1 * ave1;
            ave3 = ave2 * ave1;
            ave4 = ave3 * ave1;
            pParams[j].noiseSigma[i] = pCalibdb[j].sigma_curve[0] * ave4
                                       + pCalibdb[j].sigma_curve[1] * ave3
                                       + pCalibdb[j].sigma_curve[2] * ave2
                                       + pCalibdb[j].sigma_curve[3] * ave1
                                       + pCalibdb[j].sigma_curve[4];
            if(pParams[j].noiseSigma[i] < 0) {
                pParams[j].noiseSigma[i] = 0;
            }
        }

        for (i = 0; i < ISO_CURVE_POINT_NUM; i++) {
            if(bit_shift > 0) {
                pParams[j].lumaPoints[i] >>= bit_shift;
            }
            else {
                pParams[j].lumaPoints[i] <<= ABS(bit_shift);
            }
        }

        for(i = 0; i < WAVELET_LEVEL_NUM; i++) {
            pParams[j].loFreqDenoiseWeight[i] = pCalibdb[j].denoise_weight[i];
            pParams[j].loFreqBfScale[i] = pCalibdb[j].lo_bfScale[i];
        }

        for(i = 0; i < 6; i++) {
            pParams[j].loFreqLumaNrCurvePoint[i] = pCalibdb[j].lo_lumaPoint[i];
            pParams[j].loFreqLumaNrCurveRatio[i] = pCalibdb[j].lo_lumaRatio[i];
        }

        pParams[j].loFreqDenoiseStrength[0] = pCalibdb[j].imerge_ratio;
        pParams[j].loFreqDenoiseStrength[1] = pCalibdb[j].imerge_bound;
        pParams[j].loFreqDirectionStrength = pCalibdb[j].lo_directionStrength;

        for(i = 0; i < WAVELET_LEVEL_NUM; i++) {
            pParams[j].hiFreqDenoiseWeight[i] = pCalibdb[j].hi_denoiseWeight[i];
            pParams[j].hiFreqBfScale[i] = pCalibdb[j].hi_bfScale[i];
            pParams[j].hiFreqEdgeSoftness[i] = pCalibdb[j].hwith_d[i];
            pParams[j].hiFreqSoftThresholdScale[i] = pCalibdb[j].hi_soft_thresh_scale[i];
            pParams[j].lscGainRatioAdjust[i] = 1.0;

        }

        for(i = 0; i < 6; i++) {
            pParams[j].hiFreqLumaNrCurvePoint[i] = pCalibdb[j].hi_lumaPoint[i];
            pParams[j].hiFreqLumaNrCurveRatio[i] = pCalibdb[j].hi_lumaRatio[i];
        }
        pParams[j].hiFreqDenoiseStrength = pCalibdb[j].hi_denoiseStrength;

        for(i = 0; i < 7; i++) {
            pParams[j].radialNoiseCtrPoint[i] = 0;
            pParams[j].radialNoiseCtrRatio[i] = 1.0;
        }

        for(i = 0; i < 6; i++) {
            float tmp = pCalibdb[j].y_luma_point[i];
            if(bit_shift > 0)
                tmp /= (1 << bit_shift);
            else
                tmp *= (1 << ABS(bit_shift));

            pParams[j].detailThre[i] = tmp;
            pParams[j].detailThreRatioLevel[0][i] = pCalibdb[j].hgrad_y_level1[i];
            pParams[j].detailThreRatioLevel[1][i] = pCalibdb[j].hgrad_y_level2[i];
            pParams[j].detailThreRatioLevel[2][i] = pCalibdb[j].hgrad_y_level3[i];
            pParams[j].detailThreLevel4[i] = tmp;
            pParams[j].detailThreRatioLevel4[i] = pCalibdb[j].hgrad_y_level4[i];
        }

        pParams[j].detailMinAdjDnW = pCalibdb[j].hi_detailMinAdjDnW;
        pParams[j].waveLetCoeffDeltaHi = 0;
        pParams[j].waveLetCoeffDeltaLo = 0;
        pParams[j].hiValueThre = 0;
        pParams[j].loValueThre = 0;
    }

    memcpy(pYnrParams->ynr_ver_char, pYnrCalib->version, sizeof(pYnrParams->ynr_ver_char));

    return res;
}


ANRresult_t select_ynr_params_by_ISO(RKAnr_Ynr_Params_t *stYnrParam, RKAnr_Ynr_Params_Select_t *stYnrParamSelected, ANRExpInfo_t *pExpInfo, short bitValue)
{
    short multBit;
    float ratio = 0.0f;
    int isoValue = 50;
    RKAnr_Ynr_Params_Select_t *pstYNrTuneParamHi = NULL;
    RKAnr_Ynr_Params_Select_t *pstYNrTuneParamLo = NULL;


    ANRresult_t res = ANR_RET_SUCCESS;

    if(stYnrParam == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(stYnrParamSelected == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pExpInfo == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    /**************change bits*****************/

    memcpy(stYnrParamSelected->ynr_ver_char, stYnrParam->ynr_ver_char, sizeof(stYnrParamSelected->ynr_ver_char));

    /*********************************/
    bitValue = RKAIQ_YNR_SIGMA_BITS;
    multBit  = 1;
    if(bitValue >= 8)
    {
        multBit = 1 << (bitValue - 8);
    }

#if 0
    if(isoValue > 50 && isoValue <= 100)
    {
        ratio = (isoValue - 50) / (float)(100 - 50);

        pstYNrTuneParamHi = &stYnrParam->stYNrParamsISO100;
        pstYNrTuneParamLo = &stYnrParam->stYNrParamsISO50;
    }
    else if(isoValue > 100 && isoValue <= 200)
    {
        ratio = (isoValue - 100) / (float)(200 - 100);

        pstYNrTuneParamHi = &stYnrParam->stYNrParamsISO200;
        pstYNrTuneParamLo = &stYnrParam->stYNrParamsISO100;
    }
    else if(isoValue > 200 && isoValue <= 400)
    {
        ratio = (isoValue - 200) / (float)(400 - 200);

        pstYNrTuneParamHi = &stYnrParam->stYNrParamsISO400;
        pstYNrTuneParamLo = &stYnrParam->stYNrParamsISO200;
    }
    else if(isoValue > 400 && isoValue <= 800)
    {
        ratio = (isoValue - 400) / (float)(800 - 400);

        pstYNrTuneParamHi = &stYnrParam->stYNrParamsISO800;
        pstYNrTuneParamLo = &stYnrParam->stYNrParamsISO400;
    }
    else if(isoValue > 800 && isoValue <= 1600)
    {
        ratio = (isoValue - 800) / (float)(1600 - 800);

        pstYNrTuneParamHi = &stYnrParam->stYNrParamsISO1600;
        pstYNrTuneParamLo = &stYnrParam->stYNrParamsISO800;
    }
    else if(isoValue > 1600 && isoValue <= 3200)
    {
        ratio = (isoValue - 1600) / (float)(3200 - 1600);

        pstYNrTuneParamHi = &stYnrParam->stYNrParamsISO3200;
        pstYNrTuneParamLo = &stYnrParam->stYNrParamsISO1600;
    }
    else if(isoValue > 3200 && isoValue <= 6400)
    {
        ratio = (isoValue - 3200) / (float)(6400 - 3200);

        pstYNrTuneParamHi = &stYnrParam->stYNrParamsISO6400;
        pstYNrTuneParamLo = &stYnrParam->stYNrParamsISO3200;
    }
    else if(isoValue > 6400 && isoValue <= 12800)
    {
        ratio = (isoValue - 6400) / (float)(12800 - 6400);

        pstYNrTuneParamHi = &stYnrParam->stYNrParamsISO12800;
        pstYNrTuneParamLo = &stYnrParam->stYNrParamsISO6400;
    }
    else if(isoValue > 12800)
    {
        ratio = 1;

        pstYNrTuneParamHi = &stYnrParam->stYNrParamsISO12800;
        pstYNrTuneParamLo = &stYnrParam->stYNrParamsISO6400;
    }
    else if(isoValue <= 50)
    {
        ratio = 0;

        pstYNrTuneParamHi = &stYnrParam->stYNrParamsISO100;
        pstYNrTuneParamLo = &stYnrParam->stYNrParamsISO50;
    }
#endif

	if(pExpInfo->mfnr_mode_3to1){
		isoValue = pExpInfo->preIso[pExpInfo->hdr_mode];
	}else{
   		isoValue = pExpInfo->arIso[pExpInfo->hdr_mode];
	}

    int iso_div = 50;
    int lowIso = 50;
    int highIso = 50;
	int i=0;

#ifndef RK_SIMULATOR_HW
    for(i = 0; i < MAX_ISO_STEP - 1; i++) {
        lowIso = stYnrParam->aYnrParamsISO[i].iso;
        highIso = stYnrParam->aYnrParamsISO[i + 1].iso;
        if(isoValue >= lowIso && isoValue <= highIso) {
            ratio = (isoValue - lowIso ) / (float)(highIso - lowIso);
            pstYNrTuneParamHi = &stYnrParam->aYnrParamsISO[i + 1];
            pstYNrTuneParamLo = &stYnrParam->aYnrParamsISO[i];
            break;
        }
    }

	if(i == MAX_ISO_STEP - 1){
	    if(isoValue < stYnrParam->aYnrParamsISO[0].iso) {
	        ratio = 0;
	        lowIso = stYnrParam->aYnrParamsISO[0].iso;
	        highIso = stYnrParam->aYnrParamsISO[1].iso;
	        pstYNrTuneParamHi = &stYnrParam->aYnrParamsISO[1];
	        pstYNrTuneParamLo = &stYnrParam->aYnrParamsISO[0];
	    }
	    if(isoValue > stYnrParam->aYnrParamsISO[MAX_ISO_STEP - 1].iso) {
	        ratio = 1;
	        lowIso = stYnrParam->aYnrParamsISO[MAX_ISO_STEP - 2].iso;
	        highIso = stYnrParam->aYnrParamsISO[MAX_ISO_STEP - 1].iso;
	        pstYNrTuneParamHi = &stYnrParam->aYnrParamsISO[MAX_ISO_STEP - 2];
	        pstYNrTuneParamLo = &stYnrParam->aYnrParamsISO[MAX_ISO_STEP - 1];
	    }
	}
#else
    for(i = 0; i < MAX_ISO_STEP - 1; i++) {
        int lowIso = iso_div * (1 << i);
        int highIso = iso_div * (1 << (i + 1));
        if(isoValue >= lowIso && isoValue <= highIso) {
            ratio = (isoValue - lowIso ) / (float)(highIso - lowIso);
            pstYNrTuneParamHi = &stYnrParam->aYnrParamsISO[i + 1];
            pstYNrTuneParamLo = &stYnrParam->aYnrParamsISO[i];
            break;
        }
    }

	if(i == MAX_ISO_STEP - 1){
	    if(isoValue < iso_div) {
	        ratio = 0;
	        pstYNrTuneParamHi = &stYnrParam->aYnrParamsISO[1];
	        pstYNrTuneParamLo = &stYnrParam->aYnrParamsISO[0];
	    }

	    if(isoValue > iso_div * (2 << MAX_ISO_STEP)) {
	        ratio = 1;
	        pstYNrTuneParamHi = &stYnrParam->aYnrParamsISO[MAX_ISO_STEP - 2];
	        pstYNrTuneParamLo = &stYnrParam->aYnrParamsISO[MAX_ISO_STEP - 1];
	    }
	}
#endif

    LOGD_ANR("oyyf %s:%d  iso:%d low:%d hight:%d ratio:%f\n", __FUNCTION__, __LINE__,
             isoValue, lowIso, highIso, ratio);

    //高频Ci值和亮度噪声曲线
    for(int i = 0; i < 12; i++)
    {
        stYnrParamSelected->ciISO[i] = ratio * (pstYNrTuneParamHi->ciISO[i] - pstYNrTuneParamLo->ciISO[i]) + pstYNrTuneParamLo->ciISO[i];
    }

    for(int i = 0; i < ISO_CURVE_POINT_NUM; i++)
    {
        stYnrParamSelected->noiseSigma[i] = ratio * (pstYNrTuneParamHi->noiseSigma[i] - pstYNrTuneParamLo->noiseSigma[i]) + pstYNrTuneParamLo->noiseSigma[i];
        stYnrParamSelected->lumaPoints[i] = (short)(ratio * (pstYNrTuneParamHi->lumaPoints[i] - pstYNrTuneParamLo->lumaPoints[i]) + pstYNrTuneParamLo->lumaPoints[i]);
    }

    //小波低频层去噪tuning参数
    for(int i = 0; i < WAVELET_LEVEL_NUM; i++)
    {
        stYnrParamSelected->loFreqNoiseCi[i]       = ratio * (pstYNrTuneParamHi->loFreqNoiseCi[i] - pstYNrTuneParamLo->loFreqNoiseCi[i]) + pstYNrTuneParamLo->loFreqNoiseCi[i];
        stYnrParamSelected->loFreqDenoiseWeight[i] = ratio * (pstYNrTuneParamHi->loFreqDenoiseWeight[i] - pstYNrTuneParamLo->loFreqDenoiseWeight[i]) + pstYNrTuneParamLo->loFreqDenoiseWeight[i];
        stYnrParamSelected->loFreqBfScale[i]       = ratio * (pstYNrTuneParamHi->loFreqBfScale[i] - pstYNrTuneParamLo->loFreqBfScale[i]) + pstYNrTuneParamLo->loFreqBfScale[i];
    }

    // fix gain table bug
    //float adj = 16 / sqrt(32.0f) / int(16 / sqrt(32.0f));
#ifndef RK_SIMULATOR_HW
    float isoValue_clip = MIN(isoValue, iso_div * (2 << MAX_ISO_STEP));
    float gain_f = sqrt(50.0f / isoValue_clip);
    if (gain_f < 0.5f)
    {
        for (int i = 0; i < 12; i++)
        {
            stYnrParamSelected->ciISO[i] /= 2.0f;
        }

        for (int i = 0; i < WAVELET_LEVEL_NUM; i++)
        {
            stYnrParamSelected->loFreqNoiseCi[i] /= 2.0f;
        }

        gain_f *= 2.0f;
    }
    for (int i = 0; i < ISO_CURVE_POINT_NUM; i++)
    {
        stYnrParamSelected->noiseSigma[i] *= gain_f;
    }
#endif

    for(int i = 0; i < 6; i++)
    {
        stYnrParamSelected->loFreqLumaNrCurvePoint[i] = ratio * (pstYNrTuneParamHi->loFreqLumaNrCurvePoint[i] - pstYNrTuneParamLo->loFreqLumaNrCurvePoint[i]) + pstYNrTuneParamLo->loFreqLumaNrCurvePoint[i];
        stYnrParamSelected->loFreqLumaNrCurveRatio[i] = ratio * (pstYNrTuneParamHi->loFreqLumaNrCurveRatio[i] - pstYNrTuneParamLo->loFreqLumaNrCurveRatio[i]) + pstYNrTuneParamLo->loFreqLumaNrCurveRatio[i];

        stYnrParamSelected->loFreqLumaNrCurvePoint[i] *= multBit;
    }

    //stYnrParamSelected->loFreqDenoiseStrength = ratio * (pstYNrTuneParamHi->loFreqDenoiseStrength - pstYNrTuneParamLo->loFreqDenoiseStrength) + pstYNrTuneParamLo->loFreqDenoiseStrength;
    stYnrParamSelected->loFreqDenoiseStrength[0] = ratio * (pstYNrTuneParamHi->loFreqDenoiseStrength[0] - pstYNrTuneParamLo->loFreqDenoiseStrength[0]) + pstYNrTuneParamLo->loFreqDenoiseStrength[0];
    stYnrParamSelected->loFreqDenoiseStrength[1] = 1.0f / stYnrParamSelected->loFreqDenoiseStrength[0] / 2;

    stYnrParamSelected->loFreqDirectionStrength = ratio * (pstYNrTuneParamHi->loFreqDirectionStrength - pstYNrTuneParamLo->loFreqDirectionStrength) + pstYNrTuneParamLo->loFreqDirectionStrength;

    //小波高频层去噪tuning参数
    for(int i = 0; i < WAVELET_LEVEL_NUM; i++)
    {
        stYnrParamSelected->hiFreqDenoiseWeight[i]      = ratio * (pstYNrTuneParamHi->hiFreqDenoiseWeight[i] - pstYNrTuneParamLo->hiFreqDenoiseWeight[i]) + pstYNrTuneParamLo->hiFreqDenoiseWeight[i];
        stYnrParamSelected->hiFreqSoftThresholdScale[i] = ratio * (pstYNrTuneParamHi->hiFreqSoftThresholdScale[i] - pstYNrTuneParamLo->hiFreqSoftThresholdScale[i]) + pstYNrTuneParamLo->hiFreqSoftThresholdScale[i];
        stYnrParamSelected->hiFreqBfScale[i]            = ratio * (pstYNrTuneParamHi->hiFreqBfScale[i] - pstYNrTuneParamLo->hiFreqBfScale[i]) + pstYNrTuneParamLo->hiFreqBfScale[i];
        stYnrParamSelected->hiFreqEdgeSoftness[i]       = ratio * (pstYNrTuneParamHi->hiFreqEdgeSoftness[i] - pstYNrTuneParamLo->hiFreqEdgeSoftness[i]) + pstYNrTuneParamLo->hiFreqEdgeSoftness[i];
        stYnrParamSelected->lscGainRatioAdjust[i]       = ratio * (pstYNrTuneParamHi->lscGainRatioAdjust[i] - pstYNrTuneParamLo->lscGainRatioAdjust[i]) + pstYNrTuneParamLo->lscGainRatioAdjust[i];
    }

    for(int i = 0; i < 6; i++)
    {
        stYnrParamSelected->hiFreqLumaNrCurvePoint[i] = ratio * (pstYNrTuneParamHi->hiFreqLumaNrCurvePoint[i] - pstYNrTuneParamLo->hiFreqLumaNrCurvePoint[i]) + pstYNrTuneParamLo->hiFreqLumaNrCurvePoint[i];
        stYnrParamSelected->hiFreqLumaNrCurveRatio[i] = ratio * (pstYNrTuneParamHi->hiFreqLumaNrCurveRatio[i] - pstYNrTuneParamLo->hiFreqLumaNrCurveRatio[i]) + pstYNrTuneParamLo->hiFreqLumaNrCurveRatio[i];

        stYnrParamSelected->hiFreqLumaNrCurvePoint[i] *= multBit;
    }

    stYnrParamSelected->hiFreqDenoiseStrength = ratio * (pstYNrTuneParamHi->hiFreqDenoiseStrength - pstYNrTuneParamLo->hiFreqDenoiseStrength) + pstYNrTuneParamLo->hiFreqDenoiseStrength;

    //
    for(int i = 0; i < 6; i++)
    {
        stYnrParamSelected->detailThre[i]              = ratio * (pstYNrTuneParamHi->detailThre[i] - pstYNrTuneParamLo->detailThre[i]) + pstYNrTuneParamLo->detailThre[i];
        stYnrParamSelected->detailThreRatioLevel[0][i] = ratio * (pstYNrTuneParamHi->detailThreRatioLevel[0][i] - pstYNrTuneParamLo->detailThreRatioLevel[0][i]) + pstYNrTuneParamLo->detailThreRatioLevel[0][i];
        stYnrParamSelected->detailThreRatioLevel[1][i] = ratio * (pstYNrTuneParamHi->detailThreRatioLevel[1][i] - pstYNrTuneParamLo->detailThreRatioLevel[1][i]) + pstYNrTuneParamLo->detailThreRatioLevel[1][i];
        stYnrParamSelected->detailThreRatioLevel[2][i] = ratio * (pstYNrTuneParamHi->detailThreRatioLevel[2][i] - pstYNrTuneParamLo->detailThreRatioLevel[2][i]) + pstYNrTuneParamLo->detailThreRatioLevel[2][i];

        stYnrParamSelected->detailThreLevel4[i]        = ratio * (pstYNrTuneParamHi->detailThreLevel4[i] - pstYNrTuneParamLo->detailThreLevel4[i]) + pstYNrTuneParamLo->detailThreLevel4[i];
        stYnrParamSelected->detailThreRatioLevel4[i]   = ratio * (pstYNrTuneParamHi->detailThreRatioLevel4[i] - pstYNrTuneParamLo->detailThreRatioLevel4[i]) + pstYNrTuneParamLo->detailThreRatioLevel4[i];
    }
    stYnrParamSelected->detailMinAdjDnW = ratio * (pstYNrTuneParamHi->detailMinAdjDnW - pstYNrTuneParamLo->detailMinAdjDnW) + pstYNrTuneParamLo->detailMinAdjDnW;

    //stYnrParamSelected->detailHiHiTh = ratio * (pstYNrTuneParamHi->detailHiHiTh - pstYNrTuneParamLo->detailHiHiTh) + pstYNrTuneParamLo->detailHiHiTh;//
    //stYnrParamSelected->detailHiLoTh = ratio * (pstYNrTuneParamHi->detailHiLoTh - pstYNrTuneParamLo->detailHiLoTh) + pstYNrTuneParamLo->detailHiLoTh;
    //stYnrParamSelected->detailLoHiTh = ratio * (pstYNrTuneParamHi->detailLoHiTh - pstYNrTuneParamLo->detailLoHiTh) + pstYNrTuneParamLo->detailLoHiTh;
    //stYnrParamSelected->detailLoLoTh = ratio * (pstYNrTuneParamHi->detailLoLoTh - pstYNrTuneParamLo->detailLoLoTh) + pstYNrTuneParamLo->detailLoLoTh;
    //stYnrParamSelected->detailHiHiRatio = ratio * (pstYNrTuneParamHi->detailHiHiRatio - pstYNrTuneParamLo->detailHiHiRatio) + pstYNrTuneParamLo->detailHiHiRatio;
    //stYnrParamSelected->detailHiLoRatio = ratio * (pstYNrTuneParamHi->detailHiLoRatio - pstYNrTuneParamLo->detailHiLoRatio) + pstYNrTuneParamLo->detailHiLoRatio;
    //stYnrParamSelected->detailLoHiRatio = ratio * (pstYNrTuneParamHi->detailLoHiRatio - pstYNrTuneParamLo->detailLoHiRatio) + pstYNrTuneParamLo->detailLoHiRatio;
    //stYnrParamSelected->detailLoLoRatio = ratio * (pstYNrTuneParamHi->detailLoLoRatio - pstYNrTuneParamLo->detailLoLoRatio) + pstYNrTuneParamLo->detailLoLoRatio;
    //stYnrParamSelected->detailMaxAdjDnW = ratio * (pstYNrTuneParamHi->detailMaxAdjDnW - pstYNrTuneParamLo->detailMaxAdjDnW) + pstYNrTuneParamLo->detailMaxAdjDnW;

    //stYnrParamSelected->detailHiHiThLevel4H = ratio * (pstYNrTuneParamHi->detailHiHiThLevel4H - pstYNrTuneParamLo->detailHiHiThLevel4H) + pstYNrTuneParamLo->detailHiHiThLevel4H;
    //stYnrParamSelected->detailHiLoThLevel4H = ratio * (pstYNrTuneParamHi->detailHiLoThLevel4H - pstYNrTuneParamLo->detailHiLoThLevel4H) + pstYNrTuneParamLo->detailHiLoThLevel4H;
    //stYnrParamSelected->detailLoHiThLevel4H = ratio * (pstYNrTuneParamHi->detailLoHiThLevel4H - pstYNrTuneParamLo->detailLoHiThLevel4H) + pstYNrTuneParamLo->detailLoHiThLevel4H;
    //stYnrParamSelected->detailLoLoThLevel4H = ratio * (pstYNrTuneParamHi->detailLoLoThLevel4H - pstYNrTuneParamLo->detailLoLoThLevel4H) + pstYNrTuneParamLo->detailLoLoThLevel4H;
    //stYnrParamSelected->detailHiHiRatioLevel4H = ratio * (pstYNrTuneParamHi->detailHiHiRatioLevel4H - pstYNrTuneParamLo->detailHiHiRatioLevel4H) + pstYNrTuneParamLo->detailHiHiRatioLevel4H;
    //stYnrParamSelected->detailHiLoRatioLevel4H = ratio * (pstYNrTuneParamHi->detailHiLoRatioLevel4H - pstYNrTuneParamLo->detailHiLoRatioLevel4H) + pstYNrTuneParamLo->detailHiLoRatioLevel4H;
    //stYnrParamSelected->detailLoHiRatioLevel4H = ratio * (pstYNrTuneParamHi->detailLoHiRatioLevel4H - pstYNrTuneParamLo->detailLoHiRatioLevel4H) + pstYNrTuneParamLo->detailLoHiRatioLevel4H;
    //stYnrParamSelected->detailLoLoRatioLevel4H = ratio * (pstYNrTuneParamHi->detailLoLoRatioLevel4H - pstYNrTuneParamLo->detailLoLoRatioLevel4H) + pstYNrTuneParamLo->detailLoLoRatioLevel4H;
    //
    //径向去噪tuning参数
    for(int i = 0; i < 7; i++)
    {
        stYnrParamSelected->radialNoiseCtrPoint[i] = (short)(ratio * (pstYNrTuneParamHi->radialNoiseCtrPoint[i] - pstYNrTuneParamLo->radialNoiseCtrPoint[i]) + pstYNrTuneParamLo->radialNoiseCtrPoint[i]);
        stYnrParamSelected->radialNoiseCtrRatio[i] = ratio * (pstYNrTuneParamHi->radialNoiseCtrRatio[i] - pstYNrTuneParamLo->radialNoiseCtrRatio[i]) + pstYNrTuneParamLo->radialNoiseCtrRatio[i];
    }

    //小波第四层LL递归参数
    stYnrParamSelected->waveLetCoeffDeltaHi = (short)(ratio * (pstYNrTuneParamHi->waveLetCoeffDeltaHi - pstYNrTuneParamLo->waveLetCoeffDeltaHi) + pstYNrTuneParamLo->waveLetCoeffDeltaHi);
    stYnrParamSelected->waveLetCoeffDeltaLo = (short)(ratio * (pstYNrTuneParamHi->waveLetCoeffDeltaLo - pstYNrTuneParamLo->waveLetCoeffDeltaLo) + pstYNrTuneParamLo->waveLetCoeffDeltaLo);
    stYnrParamSelected->hiValueThre         = (short)(ratio * (pstYNrTuneParamHi->hiValueThre - pstYNrTuneParamLo->hiValueThre) + pstYNrTuneParamLo->hiValueThre);
    stYnrParamSelected->loValueThre         = (short)(ratio * (pstYNrTuneParamHi->loValueThre - pstYNrTuneParamLo->loValueThre) + pstYNrTuneParamLo->loValueThre);
    stYnrParamSelected->ynr_level4_max_gain = ROUND_F(sqrt((float)isoValue / 50) * (1 << GAIN_YNR_FIX_BITS_DECI));

    return res;
}


int  find_top_one_pos(int data)
{
    int i, j = 1;
    int pos = 0;
    for(i = 0; i < 32; i++)
    {
        if(data & j)
        {
            pos = i + 1;
        }
        j = j << 1;
    }
    return pos;
}

ANRresult_t ynr_fix_transfer(RKAnr_Ynr_Params_Select_t* ynr, RKAnr_Ynr_Fix_t *pNrCfg, float gain_ratio, float fStrength)
{
    LOGI_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    ANRresult_t res = ANR_RET_SUCCESS;

    if(ynr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    if(pNrCfg == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

	LOGD_ANR("%s:%d strength:%f\n", __FUNCTION__, __LINE__, fStrength);
	if(fStrength <= 0.0){
		fStrength = 0.000001;
	}
	
    int i = 0;
    int j = 0;
    int tmp = 0;
	int strength_i = 2;

    //0x0104 - 0x0108
    for(i = 0; i < 16; i++) {
        pNrCfg->ynr_sgm_dx[i] = find_top_one_pos(ynr->lumaPoints[i + 1] - ynr->lumaPoints[i]) - 2;
        LOGI_ANR("##########ynr sgm dx[%d] :%d  reg:%d\n", i, ynr->lumaPoints[i], pNrCfg->ynr_sgm_dx[i]);
    }

    //0x010c - 0x012c
    for(i = 0; i < 17; i++) {
        float rate;
        for(j = 0; j < 6; j++) {
            if(ynr->lumaPoints[i] <= ynr->loFreqLumaNrCurvePoint[j])
                break;
        }

        if(j <= 0)
            rate = ynr->loFreqLumaNrCurveRatio[0];
        else if(j >= 6)
            rate = ynr->loFreqLumaNrCurveRatio[5];
        else {
            rate = ((float)ynr->lumaPoints[i] - ynr->loFreqLumaNrCurvePoint[j - 1]) / (ynr->loFreqLumaNrCurvePoint[j] - ynr->loFreqLumaNrCurvePoint[j - 1]);
            rate = ynr->loFreqLumaNrCurveRatio[j - 1] + rate * (ynr->loFreqLumaNrCurveRatio[j] - ynr->loFreqLumaNrCurveRatio[j - 1]);
        }
        tmp = ynr->noiseSigma[i] * (1 << FIX_BIT_NOISE_SIGMA);
        tmp = (int)(rate * tmp);
        //clip sigma be 10bit;
        pNrCfg->ynr_lsgm_y[i] = MIN(tmp / ((1 << (12 - YNR_SIGMA_BITS)) * sqrt(gain_ratio)), (1 << (FIX_BIT_NOISE_SIGMA + 9)) - 1);
      //  pNrCfg->ynr_lsgm_y[i] = tmp / (1 << (12 - YNR_SIGMA_BITS));
     //   if(i==0)
        //    printf("pNrCfg->ynr_lsgm_y[i] %d, tmp %d\n", pNrCfg->ynr_lsgm_y[i], tmp);
    }


    //0x0130
    for(i = 0; i < 4; i++) {
		if(i>strength_i){
			tmp = (ynr->loFreqNoiseCi[i] *(1 << FIX_BIT_CI));
		}else{
          tmp = (ynr->loFreqNoiseCi[i] * fStrength *(1 << FIX_BIT_CI));
		}
		//printf("ynr ci[%d]: ci:%f  reg:0x%x  fstrength: %f \n", i, ynr->loFreqNoiseCi[i], tmp, fStrength);
		
		if(tmp > 0xff){
			tmp = 0xff;
		}
		pNrCfg->ynr_lci[i] = tmp;
    }

    //0x0134
    for(i = 0; i < 4; i++) {
		if(i>strength_i){
			tmp = (ynr->loFreqBfScale[i] * (1 << FIX_BIT_BF_SCALE));
		}else{
          tmp = (ynr->loFreqBfScale[i] * fStrength * (1 << FIX_BIT_BF_SCALE));
		}
		if(tmp > 0xff){
			tmp = 0xff;
		}
		pNrCfg->ynr_lgain_min[i] = tmp;
    }

    //0x0138
    pNrCfg->ynr_lgain_max = (unsigned char)(ynr->loFreqDirectionStrength * (1 << FIX_BIT_DIRECTION_STRENGTH));


    //0x013c
    pNrCfg->ynr_lmerge_bound = (unsigned char)((ynr->loFreqDenoiseStrength[1]) * (1 << FIX_BIT_DENOISE_STRENGTH) );
    pNrCfg->ynr_lmerge_ratio = (unsigned char)((ynr->loFreqDenoiseStrength[0]) * (1 << FIX_BIT_DENOISE_STRENGTH));

    //0x0140
    for(i = 0; i < 4; i++) {
		if(i>strength_i){
        	pNrCfg->ynr_lweit_flt[i] = (unsigned char)(ynr->loFreqDenoiseWeight[i] *  (1 << FIX_BIT_DENOISE_WEIGHT));
		}else{
			pNrCfg->ynr_lweit_flt[i] = (unsigned char)(ynr->loFreqDenoiseWeight[i] * fStrength * (1 << FIX_BIT_DENOISE_WEIGHT));
		}
		if(pNrCfg->ynr_lweit_flt[i] > 0x80){
			pNrCfg->ynr_lweit_flt[i] = 0x80;
		}
    }

    //0x0144 - 0x0164
    for(i = 0; i < 17; i++) {
        float rate;
        for(j = 0; j < 6; j++) {
            if(ynr->lumaPoints[i] <= ynr->hiFreqLumaNrCurvePoint[j])
                break;
        }

        if(j <= 0)
            rate = ynr->hiFreqLumaNrCurveRatio[0];
        else if(j >= 6)
            rate = ynr->hiFreqLumaNrCurveRatio[5];
        else {
            rate = ((float)ynr->lumaPoints[i] - ynr->hiFreqLumaNrCurvePoint[j - 1])
                   / (ynr->hiFreqLumaNrCurvePoint[j] - ynr->hiFreqLumaNrCurvePoint[j - 1]);
            rate = ynr->hiFreqLumaNrCurveRatio[j - 1]
                   + rate * (ynr->hiFreqLumaNrCurveRatio[j] - ynr->hiFreqLumaNrCurveRatio[j - 1]);
        }
        tmp = ynr->noiseSigma[i] * (1 << FIX_BIT_NOISE_SIGMA);
        tmp = (int)(rate * tmp);
        //clip sigma be 10bit;

        pNrCfg->ynr_hsgm_y[i] = MIN(tmp / ((1 << (12 - YNR_SIGMA_BITS)) * sqrt(gain_ratio)), (1 << (FIX_BIT_NOISE_SIGMA + 9)) - 1);
      //  pNrCfg->ynr_hsgm_y[i] = tmp / (1 << (12 - YNR_SIGMA_BITS));
    }

    //0x0168
    for(i = 0; i < 4; i++) {
		if(i>strength_i){
			tmp = (ynr->ciISO[i * 3 + 1] * (1 << FIX_BIT_CI));
		}else{
          tmp = (ynr->ciISO[i * 3 + 1] *  (1 << FIX_BIT_CI));
		}
		if(tmp > 0xff){
			tmp = 0xff;
		}
		pNrCfg->ynr_hlci[i] = tmp;
    }

    //0x016c
    for(i = 0; i < 4; i++) {
		if(i>strength_i){
			tmp = (ynr->ciISO[i * 3 + 0] *  (1 << FIX_BIT_CI));
		}else{
          tmp = (ynr->ciISO[i * 3 + 0] * (1 << FIX_BIT_CI));
		}
		if(tmp > 0xff){
			tmp = 0xff;
		}
		pNrCfg->ynr_lhci[i] = tmp;
    }

    //0x0170
    for(i = 0; i < 4; i++) {
		if(i>strength_i){
			tmp = (ynr->ciISO[i * 3 + 2] * (1 << FIX_BIT_CI));
		}else{
          tmp = (ynr->ciISO[i * 3 + 2] *  (1 << FIX_BIT_CI));
		}
		if(tmp > 0xff){
			tmp = 0xff;
		}
		pNrCfg->ynr_hhci[i] = tmp;
    }

    //0x0174
    for(i = 0; i < 4; i++) {
		if(i>strength_i){
			tmp = (ynr->hiFreqBfScale[i] * (1 << FIX_BIT_BF_SCALE));
		}else{
          tmp = (ynr->hiFreqBfScale[i] * fStrength *  (1 << FIX_BIT_BF_SCALE));
		}
		if(tmp > 0xff){
			tmp = 0xff;
		}
		pNrCfg->ynr_hgain_sgm[i] = tmp;
    }

    //0x0178 - 0x0188
    int wavelvl = 0;
    int EdgeSoftness = 0;
    for(i = 0; i < 4; i++) {
        if(i == 0)wavelvl = 0;
        if(i == 1)wavelvl = 1;
        if(i == 2)wavelvl = 2;
        if(i == 3)wavelvl = 3;
        EdgeSoftness = (int)(ynr->hiFreqEdgeSoftness[wavelvl] * (1 << FIX_BIT_EDGE_SOFTNESS));
        pNrCfg->ynr_hweit_d[0 * 4 + i] = (int)((exp(-(((0 * 0 + 1 * 1) * (1 << (FIX_BIT_EDGE_SOFTNESS + FIX_BIT_EDGE_SOFTNESS))) / (float)(2 * EdgeSoftness * EdgeSoftness)))) * (1 << YNR_exp_lut_y));
        pNrCfg->ynr_hweit_d[1 * 4 + i] = (int)((exp(-(((1 * 1 + 1 * 1) * (1 << (FIX_BIT_EDGE_SOFTNESS + FIX_BIT_EDGE_SOFTNESS))) / (float)(2 * EdgeSoftness * EdgeSoftness)))) * (1 << YNR_exp_lut_y));
        pNrCfg->ynr_hweit_d[2 * 4 + i] = (int)((exp(-(((0 * 0 + 2 * 2) * (1 << (FIX_BIT_EDGE_SOFTNESS + FIX_BIT_EDGE_SOFTNESS))) / (float)(2 * EdgeSoftness * EdgeSoftness)))) * (1 << YNR_exp_lut_y));
        pNrCfg->ynr_hweit_d[3 * 4 + i] = (int)((exp(-(((1 * 1 + 2 * 2) * (1 << (FIX_BIT_EDGE_SOFTNESS + FIX_BIT_EDGE_SOFTNESS))) / (float)(2 * EdgeSoftness * EdgeSoftness)))) * (1 << YNR_exp_lut_y));
        pNrCfg->ynr_hweit_d[4 * 4 + i] = (int)((exp(-(((2 * 2 + 2 * 2) * (1 << (FIX_BIT_EDGE_SOFTNESS + FIX_BIT_EDGE_SOFTNESS))) / (float)(2 * EdgeSoftness * EdgeSoftness)))) * (1 << YNR_exp_lut_y));
        LOGI_ANR("########ynr  hweit wavelvl[%d]: edge:%d weit: %d %d %d %d %d \n",
                 i, EdgeSoftness,
                 pNrCfg->ynr_hweit_d[0 * 4 + i],
                 pNrCfg->ynr_hweit_d[1 * 4 + i],
                 pNrCfg->ynr_hweit_d[2 * 4 + i],
                 pNrCfg->ynr_hweit_d[3 * 4 + i],
                 pNrCfg->ynr_hweit_d[4 * 4 + i]);
    }


    //0x018c - 0x01a0
    for(i = 0; i < 6; i++) {
        pNrCfg->ynr_hgrad_y[i * 4 + 0] = (int)(ynr->detailThreRatioLevel[0][i] * (1 << FIX_BIT_GRAD_ADJUST_CURVE));
        pNrCfg->ynr_hgrad_y[i * 4 + 1] = (int)(ynr->detailThreRatioLevel[1][i] * (1 << FIX_BIT_GRAD_ADJUST_CURVE));
        pNrCfg->ynr_hgrad_y[i * 4 + 2] = (int)(ynr->detailThreRatioLevel[2][i] * (1 << FIX_BIT_GRAD_ADJUST_CURVE));
        pNrCfg->ynr_hgrad_y[i * 4 + 3] = (int)(ynr->detailThreRatioLevel4[i] * (1 << FIX_BIT_GRAD_ADJUST_CURVE));
    }

    //0x01a4 -0x01a8
    for(i = 0; i < 4; i++) {
		if(i>strength_i){
			pNrCfg->ynr_hweit[i] = (unsigned short)(ynr->hiFreqDenoiseWeight[i] *(1 << FIX_BIT_DENOISE_WEIGHT));
		}else{
          pNrCfg->ynr_hweit[i] = (unsigned short)(ynr->hiFreqDenoiseWeight[i] * fStrength *  (1 << FIX_BIT_DENOISE_WEIGHT));
		}
        
		if(pNrCfg->ynr_hweit[i] > 0x1ff){
			pNrCfg->ynr_hweit[i] = 0x1ff;
		}
    }

    //0x01b0
    pNrCfg->ynr_hmax_adjust = (unsigned char)(ynr->detailMinAdjDnW * (1 << FIX_BIT_GRAD_ADJUST_CURVE));

    //0x01b4
    tmp = (ynr->hiFreqDenoiseStrength * fStrength * (1 << FIX_BIT_DENOISE_STRENGTH));
	if(tmp > 0xff){
		tmp = 0xff;
	}
	pNrCfg->ynr_hstrength = tmp;

    //0x01b8
    pNrCfg->ynr_lweit_cmp[0] = (int)(0.1f * (1 << YNR_exp_lut_y) + 0.5f);//13
    pNrCfg->ynr_lweit_cmp[1] = (int)(0.1f * (1 << YNR_exp_lut_y) + 0.5f);//13


    //0x01bc
    pNrCfg->ynr_lmaxgain_lv4 = ynr->ynr_level4_max_gain;

    //0x01c0 - 0x01e0
    for(i = 0; i < 17; i++) {
        tmp = (int)(ynr->noiseSigma[i] * (1 << FIX_BIT_NOISE_SIGMA));
        pNrCfg->ynr_hstv_y[i] = tmp / ((1 << (12 - YNR_SIGMA_BITS)) * sqrt(gain_ratio));
    }

    //0x01e4  - 0x01e8
    if (strcmp(ynr->ynr_ver_char, "V2") == 0) {
        // Lite Version
        for(i = 0; i < 3; i++) {
            pNrCfg->ynr_st_scale[i] = (unsigned short)(ynr->hiFreqSoftThresholdScale[i] * (1 << FIX_BIT_SOFT_THRESHOLD_SCALE_V2));
        }
    } else {
        // old v1 version
        for(i = 0; i < 3; i++) {
            pNrCfg->ynr_st_scale[i] = (unsigned short)(ynr->hiFreqSoftThresholdScale[i] * (1 << FIX_BIT_SOFT_THRESHOLD_SCALE));
        }
    }

#if YNR_FIX_VALUE_PRINTF
    ynr_fix_printf(pNrCfg);
#endif

    LOGI_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return res;
}

ANRresult_t ynr_fix_printf(RKAnr_Ynr_Fix_t * pNrCfg)
{
    LOGD_ANR("%s:(%d) enter \n", __FUNCTION__, __LINE__);

    ANRresult_t res = ANR_RET_SUCCESS;

    if(pNrCfg == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return ANR_RET_NULL_POINTER;
    }

    int i = 0;

    //0x0104 - 0x0108
    for(i = 0; i < 16; i++) {
        LOGD_ANR("(0x0104 - 0x0108) ynr_sgm_dx[%d]:%d \n",
                 i, pNrCfg->ynr_sgm_dx[i]);
    }

    //0x010c - 0x012c
    for(i = 0; i < 17; i++) {
        LOGD_ANR("(0x010c - 0x012c) ynr_lsgm_y[%d]:%d \n",
                 i, pNrCfg->ynr_lsgm_y[i]);
    }

    //0x0130
    for(i = 0; i < 4; i++) {
        LOGD_ANR("(0x0130) ynr_lci[%d]:%d \n",
                 i, pNrCfg->ynr_lci[i]);
    }

    //0x0134
    for(i = 0; i < 4; i++) {
        LOGD_ANR("(0x0134) ynr_lgain_min[%d]:%d \n",
                 i, pNrCfg->ynr_lgain_min[i]);
    }

    //0x0138
    LOGD_ANR("(0x0138) ynr_lgain_max:%d \n",
             pNrCfg->ynr_lgain_max);


    //0x013c
    LOGD_ANR("(0x013c) ynr_lmerge_bound:%d ynr_lmerge_ratio:%d\n",
             pNrCfg->ynr_lmerge_bound,
             pNrCfg->ynr_lmerge_ratio);

    //0x0140
    for(i = 0; i < 4; i++) {
        LOGD_ANR("(0x0140) ynr_lweit_flt[%d]:%d \n",
                 i, pNrCfg->ynr_lweit_flt[i]);
    }

    //0x0144 - 0x0164
    for(i = 0; i < 17; i++) {
        LOGD_ANR("(0x0144 - 0x0164) ynr_hsgm_y[%d]:%d \n",
                 i, pNrCfg->ynr_hsgm_y[i]);
    }

    //0x0168
    for(i = 0; i < 4; i++) {
        LOGD_ANR("(0x0168) ynr_hlci[%d]:%d \n",
                 i, pNrCfg->ynr_hlci[i]);
    }

    //0x016c
    for(i = 0; i < 4; i++) {
        LOGD_ANR("(0x016c) ynr_lhci[%d]:%d \n",
                 i, pNrCfg->ynr_lhci[i]);
    }

    //0x0170
    for(i = 0; i < 4; i++) {
        LOGD_ANR("(0x0170) ynr_hhci[%d]:%d \n",
                 i, pNrCfg->ynr_hhci[i]);
    }

    //0x0174
    for(i = 0; i < 4; i++) {
        LOGD_ANR("(0x0174) ynr_hgain_sgm[%d]:%d \n",
                 i, pNrCfg->ynr_hgain_sgm[i]);
    }

    //0x0178 - 0x0188
    for(i = 0; i < 5; i++) {
        LOGD_ANR("(0x0178 - 0x0188) ynr_hweit_d[%d - %d]:%d %d %d %d \n",
                 i * 4 + 0, i * 4 + 3,
                 pNrCfg->ynr_hweit_d[i * 4 + 0],
                 pNrCfg->ynr_hweit_d[i * 4 + 1],
                 pNrCfg->ynr_hweit_d[i * 4 + 2],
                 pNrCfg->ynr_hweit_d[i * 4 + 3]);
    }


    //0x018c - 0x01a0
    for(i = 0; i < 6; i++) {
        LOGD_ANR("(0x018c - 0x01a0) ynr_hgrad_y[%d - %d]:%d %d %d %d \n",
                 i * 4 + 0, i * 4 + 3,
                 pNrCfg->ynr_hgrad_y[i * 4 + 0],
                 pNrCfg->ynr_hgrad_y[i * 4 + 1],
                 pNrCfg->ynr_hgrad_y[i * 4 + 2],
                 pNrCfg->ynr_hgrad_y[i * 4 + 3]);
    }

    //0x01a4 -0x01a8
    for(i = 0; i < 4; i++) {
        LOGD_ANR("(0x01a4 -0x01a8) ynr_hweit[%d]:%d \n",
                 i, pNrCfg->ynr_hweit[i]);
    }

    //0x01b0
    LOGD_ANR("(0x01b0) ynr_hmax_adjust:%d \n",
             pNrCfg->ynr_hmax_adjust);

    //0x01b4
    LOGD_ANR("(0x01b4) ynr_hstrength:%d \n",
             pNrCfg->ynr_hstrength);

    //0x01b8
    LOGD_ANR("(0x01b8) ynr_lweit_cmp0-1:%d %d\n",
             pNrCfg->ynr_lweit_cmp[0],
             pNrCfg->ynr_lweit_cmp[1]);

    //0x01bc
    LOGD_ANR("(0x01bc) ynr_lmaxgain_lv4:%d \n",
             pNrCfg->ynr_lmaxgain_lv4);

    //0x01c0 - 0x01e0
    for(i = 0; i < 17; i++) {
        LOGD_ANR("(0x01c0 - 0x01e0 ) ynr_hstv_y[%d]:%d \n",
                 i, pNrCfg->ynr_hstv_y[i]);
    }

    //0x01e4  - 0x01e8
    for(i = 0; i < 3; i++) {
        LOGD_ANR("(0x01e4  - 0x01e8 ) ynr_st_scale[%d]:%d \n",
                 i, pNrCfg->ynr_st_scale[i]);
    }

    LOGD_ANR("%s:(%d) exit \n", __FUNCTION__, __LINE__);

    return res;
}

RKAIQ_END_DECLARE

