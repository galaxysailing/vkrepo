#include <iostream>
#include "../engine/renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"
int main(void){

    int w, h, comp;
    unsigned char* image = stbi_load("../workdir/textures/avatar.jpg", &w, &h, &comp, 0);
    if(image){
        printf("ok!");
        stbi_image_free(image);
    }


    return 0;
}