/*******************************************************************************************
* Version: wh_vpt_dt_version:0.0.2.20220821
* CopyRight: 
* UpdateDate: 20220821
* Content：人车物检测跟踪
********************************************************************************************/
#ifndef VPTDETTRACK_H_
#define VPTDETTRACK_H_



#ifdef _MSC_VER
#ifdef VPTDETTRACK_EXPORTS
#define VPTDETTRACK_API __declspec(dllexport)
#else
#define VPTDETTRACK_API __declspec(dllimport)
#endif
#else
#define VPTDETTRACK_API __attribute__ ((visibility ("default")))
#include <stdlib.h>
#endif




#include <vector>
#include <iostream>
#include "vpt_head.h"

#ifdef __cplusplus
extern "C"
{
#endif


#ifndef VPTDETTRACK_PARAM
#define VPTDETTRACK_PARAM
typedef struct vptdt_param
{
	int gpuid;		//运行卡号 GPU模式下有效
	float det_thre;	//检测阈值 默认为0.6
	int max_batchsize;//最大batchsize，用于预开辟显存
	
	int fusion_interval;		//1:每帧都检测 2：跳一帧检测 以此类推
	vptdt_param() : gpuid(0), det_thre(0.6), max_batchsize(10), fusion_interval(1) {};
}vptdt_param;
#endif

/*************************************************************************
* FUNCTION: vptdt_init
* PURPOSE: 初始化
* PARAM:
[in] handle		 -处理句柄
[in] param		 -初始化参数
* RETURN:	success(0) or error code(<0)
*************************************************************************/
VPTDETTRACK_API int vptdt_init(void **handle, vptdt_param param);

/*************************************************************************
* FUNCTION: vptdt_release
* PURPOSE: 资源释放
* PARAM:
[in] handle		- 处理句柄
* RETURN:	NULL
* NOTES:
*************************************************************************/
VPTDETTRACK_API void vptdt_release(void **handle);




/*************************************************************************
* FUNCTION: vptdt_process
* PURPOSE: 人车物检测跟踪
* PARAM:
  [in] handle		- 处理句柄
  [in] rgb		    - 图片数据（3通道BGR数据 cv::Mat格式）
  [in] width		- 图片宽度
  [in] height		- 图片高度
  [in] result		- 检测跟踪结果
* RETURN:	 数量(>= 0)或错误(<0)
* NOTES:
*************************************************************************/
VPTDETTRACK_API int vptdt_process(void * handle,unsigned char * rgb, int width, int height, vptdt_result * result);
VPTDETTRACK_API int vptdt_process_batch(void * handle, xjs_batch_img *img_data_array,  int batchsize, vptdt_result * result);


/*************************************************************************
* FUNCTION: vptdt_get_version
* PURPOSE: 获取版本号
* PARAM:	NULL
* RETURN:	版本号
* NOTES:
*************************************************************************/
VPTDETTRACK_API const char * vptdt_get_version();


#ifdef __cplusplus
};
#endif


#endif
