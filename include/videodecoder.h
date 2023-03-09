//
// Created by cjlu on 23-2-2.
//

#ifndef VIDEODECODER_H
#define VIDEODECODER_H
#include <gst/gst.h>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include "analysis.h"

using namespace std;
cv::Mat output_img;
#define test_log_flag 1
/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
    GstElement* pipeline;
    GstElement* source;
    GstElement* depay;
    GstElement* parse;
    GstElement* decoder;
    GstElement* convert;
    GstElement* sink;
    GstElement* cudadowload;
    GstElement* app_queue;
} CustomData;

/* This function will be called by the pad-added signal */
void pad_added_handler(GstElement* src, GstPad* new_pad, CustomData* data) {
    GstPad* sink_pad = gst_element_get_static_pad(data->depay, "sink");
    GstPadLinkReturn ret;
    GstCaps* new_pad_caps = NULL;
    GstStructure* new_pad_struct = NULL;
    const gchar* new_pad_type = NULL;
    g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));
    /* If our converter is already linked, we have nothing to do here */
    if (gst_pad_is_linked(sink_pad)) {
        g_print("We are already linked.\n");
        goto exit;
    }
    /* Check the new pad's type */
    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    new_pad_type = gst_structure_get_name(new_pad_struct);
    /* Attempt the link */
    ret = gst_pad_link(new_pad, sink_pad);
    if (GST_PAD_LINK_FAILED(ret)) {
        g_print("Type is '%s' but link failed.\n", new_pad_type);
    }
    else {
        g_print("Link succeeded (type '%s').\n", new_pad_type);
    }
    exit:
    /* Unreference the new pad's caps, if we got them */
    if (new_pad_caps != NULL)
        gst_caps_unref(new_pad_caps);
    /* Unreference the sink pad */
    gst_object_unref(sink_pad);
}

static GstFlowReturn ReadVideoFrame_callback(GstElement* sink, gpointer user_data)
{
    CustomData* data = (CustomData*)user_data;
    GstSample* sample = nullptr;
    GstMapInfo map;
    GstCaps *caps = NULL;
    GstBuffer *buffer = NULL;
    const GstStructure *info = NULL;
    GstFlowReturn ret = GST_FLOW_OK;
    int sample_width = 1920;
    int sample_height = 1080;
    // use pull-sample to get the frame，Map to 'map' variable，use map to copy the frame data.
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    gst_buffer_map(buffer, &map, GST_MAP_READ);
    if (ret != GST_FLOW_OK) {
        g_printerr("can't pull GstSample.");
        return ret;
    }
    if (sample) {
        buffer = gst_sample_get_buffer(sample);
        if (buffer == NULL) {
            g_printerr("get buffer is null");
            goto exit;
        }
        gst_buffer_map(buffer, &map, GST_MAP_READ);
        caps = gst_sample_get_caps(sample);
        if (caps == NULL) {
            g_printerr("get caps is null");
            goto exit;
        }
        info = gst_caps_get_structure(caps, 0);
        if (info == NULL) {
            g_printerr("get info is null");
            goto exit;
        }
        if (map.data == NULL) {
            g_printerr("appsink buffer data empty\n");
            goto exit;
        }
        gst_structure_get_int(info, "width", &sample_width);
        gst_structure_get_int(info, "height", &sample_height);
        cv::Mat rgbImg(sample_height, sample_width, CV_8UC3,
                       (unsigned char *) map.data, cv::Mat::AUTO_STEP);
        rgbImg.copyTo(output_img);

    }

    exit:
    if (buffer) {
        gst_buffer_unmap(buffer, &map);
        printf("no buffer");
    }
    if (sample) {
        gst_sample_unref(sample);
        printf("no sample");
    }
    return GST_FLOW_OK;
}

int decoder_init(){
    GstCaps *video_caps;
    CustomData data;
    GstBus* bus;
    GstMessage* msg;
    GstStateChangeReturn ret;
    gboolean terminate = FALSE;
    /* Initialize GStreamer */
    /* Create the elements */
    data.source = gst_element_factory_make("rtspsrc", "source");
    g_object_set(G_OBJECT(data.source), "latency", 2000, NULL);
    data.depay = gst_element_factory_make("rtph264depay", "depay");
    data.parse = gst_element_factory_make("h264parse", "parse");
    data.decoder = gst_element_factory_make("nvh264dec", "decoder");
    data.convert = gst_element_factory_make("cudaconvert", "convert");
    data.cudadowload = gst_element_factory_make("cudadownload", "cudadownload");
    data.app_queue = gst_element_factory_make ("queue", "app_queue");
    data.sink = gst_element_factory_make("appsink", "sink");
    if (!data.decoder || !data.convert || !data.cudadowload)//If no GPU to use, using the softdecode.
    {
        data.decoder = gst_element_factory_make("avdec_h264", "decoder");
        data.convert = gst_element_factory_make("videoconvert", "cudaconvert");
        data.cudadowload = gst_element_factory_make("queue", "cudadownload");
        g_print("NO NVIDIA GPU to use! Use SoftDecode\n");
    }
    else
    {
        g_print("Use HardDecode!\n");
    }
    video_caps = gst_caps_new_simple("video/x-raw",
                 "width", G_TYPE_INT, 1920,
                 "height", G_TYPE_INT, 1080,
                 "framerate", GST_TYPE_FRACTION, 0, 1,
                 "format", G_TYPE_STRING, "BGR", NULL);

    /* Set the URI to play */
    g_object_set(data.source, "location","rtsp://admin:admin123@192.168.1.192/h264/ch1/main/av_stream", NULL);
    /* Connect to the pad-added signal */
    g_signal_connect(data.source, "pad-added", G_CALLBACK(pad_added_handler), &data);
    g_object_set (data.sink, "caps", video_caps, NULL);
    g_object_set(data.sink,   "emit-signals", TRUE,  NULL);
    g_signal_connect(data.sink, "new-sample", G_CALLBACK(ReadVideoFrame_callback), &data);
    /* Create the empty pipeline */
    data.pipeline = gst_pipeline_new("test-pipeline");
    if (!data.pipeline || !data.decoder || !data.parse || !data.depay || !data.source || !data.convert || !data.cudadowload ||!data.app_queue|| !data.sink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Build the pipeline.  If we are NOT linking the source at this point. We will do it later. */
    gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.depay, data.parse, data.decoder, data.convert,data.cudadowload,data.app_queue, data.sink, NULL);

    if (!gst_element_link_many(data.depay, data.parse, data.decoder, data.convert, data.cudadowload,data.app_queue, data.sink, NULL)) {
        g_printerr("Elements could not be linked.\n");
        gst_object_unref(data.pipeline);
        return -1;
    }
    /* Start playing */
    ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(data.pipeline);
        return -1;
    }
    /* Listen to the bus */
    bus = gst_element_get_bus(data.pipeline);

    do {

        msg =
                gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                           (GstMessageType)(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

        /* Parse message */
        if (msg != NULL) {
            GError* err;
            gchar* debug_info;
            switch (GST_MESSAGE_TYPE(msg)) {
                case GST_MESSAGE_ERROR:
                    gst_message_parse_error(msg, &err, &debug_info);
                    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                    g_clear_error(&err);
                    g_free(debug_info);
                    terminate = TRUE;
                    break;
                case GST_MESSAGE_EOS:
                    g_print("End-Of-Stream reached.\n");
                    terminate = TRUE;
                    break;
                case GST_MESSAGE_STATE_CHANGED:
                    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data.pipeline)) {
                        GstState old_state, new_state, pending_state;
                        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
                        g_print("Pipeline state changed from %s to %s:\n",
                                gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
                    }
                    break;
                default:
                    g_printerr("Unexpected message received.\n");
                    break;
            }
            gst_message_unref(msg);
        }
    } while (!terminate);
    gst_object_unref(bus);
    gst_element_set_state(data.pipeline, GST_STATE_NULL);
    gst_object_unref(data.pipeline);
    /* Free resources */
    return 0;
}
#endif //VIDEODECODER_VIDEODECODER_H
