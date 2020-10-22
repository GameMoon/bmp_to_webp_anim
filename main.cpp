#define LOADBMP_IMPLEMENTATION
#include "loadbmp.h"

#include <stdio.h>
#include <stdlib.h>

#include <webp/encode.h>
#include <webp/mux.h>



int create_frame(const uint8_t * pixels, int width, int height, WebPConfig * config, WebPMuxFrameInfo * frame){

    WebPPicture pic;
    if (!WebPPictureInit(&pic)) return 0;  // version error
    pic.width = width;
    pic.height = height;
    WebPPictureImportRGBA(&pic,pixels,width*4);

    WebPMemoryWriter writer;
    WebPMemoryWriterInit(&writer);
    pic.writer = WebPMemoryWrite;
    pic.custom_ptr = &writer;

    int ok = WebPEncode(config, &pic);
    WebPPictureFree(&pic);   // Always free the memory associated with the input.

    if (!ok) {
        printf("Encoding error: %d\n", pic.error_code);
    } else {
        printf("Frame size: %d\n", writer.size);
    }
    
    WebPDataInit(&frame->bitstream);

    frame->bitstream.bytes = writer.mem;
    frame->bitstream.size = writer.size;

    frame->x_offset = 0;
    frame->y_offset = 0;
    frame->duration = 150;
    frame->id = WEBP_CHUNK_ANMF;
    frame->dispose_method = WEBP_MUX_DISPOSE_NONE;
    frame->blend_method = WEBP_MUX_BLEND;

    return 1;
}


int create_frame_from_bmp(const char * filename,WebPConfig * config, WebPMuxFrameInfo * frame){
    unsigned char *pixels;
    unsigned int width, height;

    loadbmp_decode_file(filename, &pixels, &width, &height, LOADBMP_RGBA);

    return create_frame(pixels,width,height,config,frame);
}

int main(){
    
    const char * file_names[] = { "test_images/test.bmp", "test_images/test2.bmp", "test_images/test3.bmp"};
    //Config setup
    WebPConfig config;
    if (!WebPConfigPreset(&config, WEBP_PRESET_PHOTO, 50.0))
        return 0; // version error
    config.lossless = 1;

    //Animation init
    WebPMux *mux = WebPMuxNew();

    //Create frames
    for(int k = 0; k< 3;k++){
        WebPMuxFrameInfo frame;
        create_frame_from_bmp(file_names[k], &config, &frame);
        WebPMuxPushFrame(mux, &frame, 1);
        WebPDataClear(&frame.bitstream);
    }

    WebPData output_data;
    WebPDataInit(&output_data);

    WebPMuxAnimParams params = {0xFFFFFFFF, 0}; //background , looping

    WebPMuxSetAnimationParams(mux,&params);
    WebPMuxAssemble(mux, &output_data);

    WebPMuxDelete(mux);
    
    printf("Anim size: %d",output_data.size);

    //Save output
    FILE *pFile = fopen("output.webp", "wb");
    fwrite(output_data.bytes, 1, output_data.size, pFile);
    fclose(pFile);

    WebPDataClear(&output_data);


    return 0;
}