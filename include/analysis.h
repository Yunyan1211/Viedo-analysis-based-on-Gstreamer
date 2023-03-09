//
// Created by cjlu on 23-2-19.
//

#ifndef DEMO_ANALYSIS_H
#define DEMO_ANALYSIS_H

#endif //DEMO_ANALYSIS_H
#include "vpt_det_tracker.h"
//#include "objcls.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <time.h>
#ifndef _MSC_VER
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#endif
#include <gst/gst.h>
#include <opencv2/opencv.hpp>
#include <unistd.h>//sleep
#include <thread>
using namespace std;

#define test_log_flag 1 

#ifndef _MSC_VER
#include <sys/time.h>
#define MACRO_COUNT_TIME_START(name)  struct timeval macro_tv_start_##name;\
	struct timeval macro_tv_end_##name;\
	gettimeofday(&macro_tv_start_##name,NULL);
#define MACRO_COUNT_TIME_END(name,___total_count___) gettimeofday(&macro_tv_end_##name,NULL);\
	printf("%s time cost: %.2f ms \n", #name, ( (double)(macro_tv_end_##name.tv_sec-macro_tv_start_##name.tv_sec)*1000000+(double)(macro_tv_end_##name.tv_usec-macro_tv_start_##name.tv_usec)  )/1000/___total_count___);
#endif


int test(char* path, char* save_folder)
{
    int batch_size =2;
#define diff_vedio 1

    printf("vpt_version: %s\n", vptdt_get_version());
    double totalTime = 0, avgTime = 0;


    //printf("vdt_init-------\n");
    vptdt_param vptdt_param_;
    vptdt_param_.gpuid=0;
    vptdt_param_.det_thre = 0.6;
    vptdt_param_.max_batchsize = 10;
    vptdt_param_.fusion_interval = 1;   
    void *handle= NULL;
    vptdt_init(&handle,vptdt_param_);

    vector<string> video_key_;
    video_key_.push_back("video_key_0");
    video_key_.push_back("video_key_1");
    video_key_.push_back("video_key_2");
    video_key_.push_back("video_key_3");
    video_key_.push_back("video_key_4");


    //for(int i = 0; i < 100; i++)
    {
        //printf("image\n");
        cv::Mat img;
        cv::VideoCapture cap;
        cap.open(path);
        cap.read(img);
        if (img.data == NULL || img.cols == 0 || img.rows == 0)
        {
            printf("sy_vpt_det_tracker_vdt_process_(error): video  imgdata == null or w == 0 or h == 0.\n");
            return -1;
        }
        if (!cap.isOpened())
        {
            printf("can't open video or camera !\n");
            return -1;
        }

        //int fourcc = cap.get(CV_CAP_PROP_FOURCC);
        //double fps = cap.get(CV_CAP_PROP_FPS);
        //cv::VideoWriter writer("res.avi", fourcc, fps, cv::Size(img.cols, img.rows));

        
        cv::VideoCapture cap1;
        cv::Mat img1;


        //*----------------------- rtsp/ ----------------------------------*/
        int frameCount = 0;
        char saveimgname[260];

        vptdt_result vptdt_res[batch_size];   
        for(int b=0;b<batch_size;b++)
        {
            memset(vptdt_res[b].objinfo, 0, sizeof(vptdt_info) * MAX_OBJ_COUNT);
        }

        xjs_batch_img xjs_batch_img_[batch_size];
        //sy_img batch_img[batch_size];
        cv::Mat batch_mat[batch_size];
        while (cap.read(img) && frameCount < 200)
        {
            if(test_log_flag)
                cout << "---------------- decode " << frameCount << " -----------------"<< endl << endl;

            //if(frameCount ==50)
            //	batch_size =1;
            //else if(frameCount ==60)
            //	batch_size =2;

            
            if(diff_vedio)
            {
                int res =cap1.read(img1);
                if(res == false)
                {
                    printf("cap1 is end!=========================\n");
                }
            }

            for(int b=0;b<batch_size;b++)
            {
                if(diff_vedio && b!=0)
                {
                    batch_mat[b]=img1.clone();
                }
                else
                    batch_mat[b]=img.clone();
            }



            for(int b=0;b<batch_size;b++)
            {
                //batch_img[b].set_data(batch_mat[b].cols, batch_mat[b].rows, batch_mat[b].channels(), batch_mat[b].data);
                //xjs_batch_img_[b].set_data(batch_mat[b].cols, batch_mat[b].rows, batch_mat[b].channels(), batch_mat[b].data, video_key_[b], 0);

                char* video_key = (char*)video_key_[b].c_str();
                xjs_batch_img_[b].set_data(batch_mat[b].cols, batch_mat[b].rows, batch_mat[b].channels(), batch_mat[b].data, video_key, 0);
            }


            //printf("vptdt_process----\n");
            int res = vptdt_process_batch(handle, xjs_batch_img_, batch_size, vptdt_res);
            //int res = vptdt_process(handle, img.data, img.cols, img.rows, vptdt_res);

            if (res < 0)
            {
                break;
            }

            if(test_log_flag)
            {
                for(int b=0;b<batch_size;b++)
                {
                    printf("objcount:%d\n",vptdt_res[b].objcount);
                    for (int c = 0; c < vptdt_res[b].objcount; c++)
                    {
                        
                        rectangle(batch_mat[b], cv::Point2f(vptdt_res[b].objinfo[c].left, vptdt_res[b].objinfo[c].top),
                                  cv::Point2f(vptdt_res[b].objinfo[c].right, vptdt_res[b].objinfo[c].bottom), cv::Scalar(255, 0, 0), 3, 1);

                        //rectangle(batch_mat[b], cvPoint(vptdt_res.objinfo[c].w_res.rect.left_, vptdt_res.objinfo[c].w_res.rect.top_),
                        //	cvPoint(vptdt_res.objinfo[c].w_res.rect.left_+vptdt_res.objinfo[c].w_res.rect.width_, vptdt_res.objinfo[c].w_res.rect.top_+vptdt_res.objinfo[c].w_res.rect.height_), cv::Scalar(0, 255, 0), 3, 1);

                        
                        char str_i[100];
                        memset(str_i, 0, 100);
                        sprintf(str_i, "%d_%.2f_%d", vptdt_res[b].objinfo[c].id,vptdt_res[b].objinfo[c].confidence,vptdt_res[b].objinfo[c].index);
                        cv::putText(batch_mat[b], str_i, cv::Point(vptdt_res[b].objinfo[c].left, vptdt_res[b].objinfo[c].top), cv::FONT_HERSHEY_COMPLEX, 1.3, cv::Scalar(0, 0, 255),  2);

                        printf("vptdt result:%d   %d   %f  %d  %d-%d\n",c,vptdt_res[b].objinfo[c].id,vptdt_res[b].objinfo[c].confidence,vptdt_res[b].objinfo[c].index,vptdt_res[b].objinfo[c].left, vptdt_res[b].objinfo[c].top);
                    }

                    //cv::resize(batch_mat[b], imgResized, cv::Size(1280, 720));
                    sprintf(saveimgname, "%s/%d/%d.jpg", save_folder,b, frameCount);
                    cv::imwrite(saveimgname, batch_mat[b]);

                }
            }

            frameCount++;
        }//end while
    }//end iter

    vptdt_release(&handle);

    printf("after release...\n");
    sleep(100);
    return 0;
}

int test1(cv::Mat img) {
    time_t nowtime;
    struct tm* p;;
    time(&nowtime);
    p = localtime(&nowtime);
    int frameCount = 0;
    vptdt_result vpt_results;
    char saveimgname[260];
    vpt_results.objcount = 0;
    printf("vpt_version: %s\n", vptdt_get_version());
    double totalTime = 0, avgTime = 0;
    vptdt_param vptdt_param_;
    vptdt_param_.gpuid = 0;
    vptdt_param_.det_thre = 0.6;
    vptdt_param_.max_batchsize = 10;
    vptdt_param_.fusion_interval = 1;
    void *handle = NULL;
    vptdt_init(&handle, vptdt_param_);
    if (img.data == NULL || img.cols == 0 || img.rows == 0) {
        printf("sy_vpt_det_tracker_vdt_process_(error): video  imgdata == null or w == 0 or h == 0.\n");
        return -1;
    }

    if(img.data != NULL) {
        if (test_log_flag)
            cout << "---------------- decode " << frameCount << " -----------------" << endl << endl;
        int res = vptdt_process(handle, img.data, img.cols, img.rows, &vpt_results);
        if (0 == vptdt_process(handle, img.data, img.cols, img.rows, &vpt_results)) {
            //ÒÀ´Î´¦ÀíÃ¿¸öÄ¿±ê
            for (int i = 0; i < vpt_results.objcount; ++i) {
                rectangle(img, cv::Point2f(vpt_results.objinfo[i].left, vpt_results.objinfo[i].top),
                          cv::Point2f(vpt_results.objinfo[i].right, vpt_results.objinfo[i].bottom), cv::Scalar(255, 0, 0), 3, 1);
                char str_i[100];
                memset(str_i, 0, 100);
                sprintf(str_i, "%d_%.2f_%d", vpt_results.objinfo[i].id,vpt_results.objinfo[i].confidence,vpt_results.objinfo[i].index);
                cv::putText(img,str_i,cv::Point(vpt_results.objinfo[i].left, vpt_results.objinfo[i].top), cv::FONT_HERSHEY_COMPLEX, 1.3, cv::Scalar(0, 0, 255));

                printf("vptdt result:%d   %d   %f  %d  %d-%d\n",i,vpt_results.objinfo[i].id,vpt_results.objinfo[i].confidence,vpt_results.objinfo[i].index,vpt_results.objinfo[i].left, vpt_results.objinfo[i].top);
                sprintf(saveimgname, "%s/%02d:%02d:%02d.jpg", "/home/cjlu/img/" ,p->tm_hour,p->tm_min,p->tm_sec);
                cv::imwrite(saveimgname, img);
            }
        }
        else {
            printf("fail to analyse");
        }
        frameCount++;
    }
}