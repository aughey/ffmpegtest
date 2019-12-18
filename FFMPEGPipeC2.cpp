#pragma warning(disable : 4996)
#pragma warning(disable : 6387)

// tutorial01.c
//
// This tutorial was written by Stephen Dranger (dranger@gmail.com).
//
// Code based on a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1

// A small sample program that shows how to use libavformat and libavcodec to
// read video from a file.
//
// Use the Makefile to build all examples.
//
// Run using
//
// tutorial01 myvideofile.mpg
//
// to write the first five frames from "myvideofile.mpg" to disk in PPM
// format.

#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include <stdio.h>

int main(int argc, char* argv[]) {
    AVFormatContext* pFormatCtx = NULL;
    int             i, videoStream;
    AVCodecContext* pCodecCtx = NULL;
    AVCodec* pCodec = NULL;
    AVFrame* pFrame = NULL;
    AVFrame* pFrameRGB = NULL;
    AVPacket        packet;
    int             frameFinished;
    int             numBytes;
    uint8_t* buffer = NULL;

    AVDictionary* optionsDict = NULL;
    struct SwsContext* sws_ctx = NULL;

    // Register all formats and codecs
    av_register_all();

    const char* filename = "zzzzzzzzzzzzzzzzzzzzzzz.mp4";


    // Open video file
    if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0)
        return -1; // Couldn't open file

      // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
        return -1; // Couldn't find stream information

      // Dump information about file onto standard error
   // av_dump_format(pFormatCtx, 0, argv[1], 0);

    // Find the first video stream
    videoStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    if (videoStream == -1)
        return -1; // Didn't find a video stream

      // Get a pointer to the codec context for the video stream
    pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, &optionsDict) < 0)
        return -1; // Could not open codec


    uint8_t* video_dst_data[4] = { NULL };
    int video_dst_linesize[4];
    int video_dst_bufsize = av_image_alloc(video_dst_data, video_dst_linesize,
        pCodecCtx->width, pCodecCtx->height,
        AV_PIX_FMT_YUV420P, 1);

      // Allocate video frame
    pFrame = av_frame_alloc();

    // Allocate an AVFrame structure
    pFrameRGB = av_frame_alloc();
    if (pFrameRGB == NULL)
        return -1;

    pFrameRGB->width = pCodecCtx->width;
    pFrameRGB->height = pCodecCtx->height;

    // Determine required buffer size and allocate buffer
    numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
        pCodecCtx->height);
    buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

    sws_ctx =
        sws_getContext
        (
            pCodecCtx->width,
            pCodecCtx->height,
            pCodecCtx->pix_fmt,
            pCodecCtx->width,
            pCodecCtx->height,
            AV_PIX_FMT_YUV420P,
            SWS_BILINEAR,
            NULL,
            NULL,
            NULL
        );

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture*)pFrameRGB, buffer, AV_PIX_FMT_YUV420P,
        pCodecCtx->width, pCodecCtx->height);

    // Read frames and save first five frames to disk
    i = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {
            // Decode video frame
            frameFinished = false;
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
                &packet);

            // Did we get a video frame?
            if (frameFinished) {

              /*  printf("video_frame%s n:%d coded_n:%d pts:%s\n",
                    cached ? "(cached)" : "",
                    video_frame_count++, frame->coded_picture_number,
                    av_ts2timestr(frame->pts, &video_dec_ctx->time_base));*/

                // Convert the image from its native format to RGB
                av_image_copy(video_dst_data, video_dst_linesize,
                    (const uint8_t**)(pFrame->data), pFrame->linesize,
                    AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);


             
                sws_scale
                (
                    sws_ctx,
                    (uint8_t const* const*)pFrame->data,
                    pFrame->linesize,
                    0,
                    pCodecCtx->height,
                    pFrameRGB->data,
                    pFrameRGB->linesize
                );

               

                int wrote;
                wrote = fwrite(video_dst_data[0], video_dst_bufsize, 1, stdout);
                if (wrote != 1) break;


                fprintf(stderr, "%d\n", video_dst_bufsize);
                /*

                uint32_t pitchY = pFrameRGB->linesize[0];
                uint32_t pitchU = pFrameRGB->linesize[1];
                uint32_t pitchV = pFrameRGB->linesize[2];

                uint8_t* avY = pFrameRGB->data[0];
                uint8_t* avU = pFrameRGB->data[1];
                uint8_t* avV = pFrameRGB->data[2];

                int total = 0;
                for (uint32_t i = 0; i < pFrameRGB->height; i++) {
                    wrote = fwrite(avY, pFrameRGB->width, 1, stdout);
                    if (wrote != 1) break;
                    total += wrote * pFrameRGB->width;
                    avY += pitchY;
                }

                for (uint32_t i = 0; i < pFrameRGB->height / 2; i++) {
                    wrote = fwrite(avU, pFrameRGB->width / 2, 1, stdout);
                    if (wrote != 1) break;
                    total += wrote * pFrameRGB->width / 2;
                    avU += pitchU;
                }

                for (uint32_t i = 0; i < pFrameRGB->height / 2; i++) {
                    wrote = fwrite(avV, pFrameRGB->width / 2, 1, stdout);
                    if (wrote != 1) break;
                    total += wrote * pFrameRGB->width / 2;
                    avV += pitchV;
                }
                //fprintf(stderr, "Total: %d\n", total);
                //// Save the frame to disk
                //if (++i <= 5)
                //    SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height,
                //        i);
                */
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }

    // Free the RGB image
    av_free(buffer);
    av_free(pFrameRGB);

    // Free the YUV frame
    av_free(pFrame);

    sws_freeContext(sws_ctx);

    // Close the codec
    avcodec_close(pCodecCtx);

    av_free(video_dst_data[0]);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    return 0;
}

