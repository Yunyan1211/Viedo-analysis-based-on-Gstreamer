
#ifndef _MSC_VER
#endif
#include <gst/gst.h>
#include "videodecoder.h"
#include <unistd.h>
#include <thread>

int main(int argc, char** argv)
{
    gst_init(&argc, &argv);
    std::thread([]() {
        decoder_init();
    }).detach();
    sleep(5);
    for(int i=0;i<10;i++){
        /*If you want to get all time's analysis result, change the loop*/
        test1(output_img);
        sleep(1);
    }
	return 0;
}







