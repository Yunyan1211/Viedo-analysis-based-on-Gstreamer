#include <stddef.h>


//多batch下使用
#ifndef __XJS_IMG__
#define __XJS_IMG__
typedef struct xjs_batch_img
{
	int w_;
	int h_;
	int c_;
	unsigned char * data_;
	//std::string video_key_;
	const char* video_key_; 
	long frameCount_; //暂时没有用到
	void set_data(int m_w, int m_h, int m_c, unsigned char * m_data, const char* m_video_key, long fcnt_)
	{
		w_ = m_w;
		h_ = m_h;
		c_ = m_c;
		data_ = m_data;
		video_key_ = m_video_key;
		frameCount_ = fcnt_;
	}
} xjs_batch_img;
#endif


//---------------- 人车物、城管类分析结果结构体定义 ------------------//
#define MAX_OBJ_COUNT 100


//Rect
#ifndef __SY_RECT__
#define __SY_RECT__
typedef struct sy_rect
{
	int left_;
	int top_;
	int width_;
	int height_;
	sy_rect(int m_left, int m_top, int m_width, int m_height) :left_(m_left), top_(m_top), width_(m_width), height_(m_height) {};
	sy_rect() {};
} sy_rect;
#endif


#ifndef VD_RESULT_
#define VD_RESULT_
typedef struct vd_result
{
	sy_rect rect;
	float score;
}vd_result;
#endif

#ifndef VPTDT_INFO
#define VPTDT_INFO
typedef struct vptdt_info	//结果结构体
{
	int left;
	int top;
	int right;
	int bottom;
	int center_x;			//目标框中心点x
	int center_y; 			//目标框中心点y
	int id;					//	目标唯一ID，同一ID为同一目标
	double confidence;		//	置信度
	int index;              //人车物9类的类别：0-person 1-bike 2-motor 3- tricycle 4-car 5-bigbus 6-lorry货车 7-tractor 8-midibus	
							//城管类的类别：0-占道经营 1-废弃家具 2-游商小贩 3-暴露垃圾 4-垃圾满溢 5-沿街晾晒 6-打包垃圾 7-物料堆放

	//输出目标的其他属性结果(如车窗车身信息、蔬菜瓜果坐标)
	vd_result w_res;
	vd_result b_res;	
}vptdt_info;
#endif


#ifndef VPTDT_RESULT
#define VPTDT_RESULT
typedef struct vptdt_result
{
	vptdt_info objinfo[MAX_OBJ_COUNT];
	int objcount;
}vptdt_result;
#endif


