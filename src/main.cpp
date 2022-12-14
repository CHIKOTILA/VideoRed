#include <stdio.h>
#include <GLFW/glfw3.h>
#include "video_read.h"
bool load_frame(const char* filename, int* width, int*height, unsigned char **data);

int main(int ardv, const char** agvv) {
    GLFWwindow* window;

    if (!glfwInit())
    {
        printf("Could init Glfw\n");
        return 1;
    }

    window = glfwCreateWindow(1280, 720, "VideoRedactor", NULL, NULL);
    if (!window)
    {
        printf("Could init Glfw\n");
        return 1;
    }

   // int frameWidth, frameHeight;
   // unsigned char* frameData;
   VideoReaderState vrState{};
    if (!video_reader_open_file(&vrState,"/Applications/Новая папка/C++/yashaLaz.MP4" )) {
        printf("couldn't open video file");
        return 1;
    }
    glfwMakeContextCurrent(window);




   // video_reader_close(&vrState);

  // if(!load_frame( "/Applications/Новая папка/C++/yashaLaz.MP4", &frameWidth, &frameHeight, &frameData) ){
  //     printf("Couldnt load video frame");
  //     return 1;
  // }



//create texture
    GLuint tex_handle;
    glGenTextures(1, &tex_handle); // draw squres in OpenGL texture
    glBindTexture(GL_TEXTURE_2D, tex_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    int frameHeight = vrState.height;
    int frameWidth = vrState.width;
    uint8_t* frameData = new uint8_t [frameHeight * frameWidth * 4];

    double firstFrameTime;


    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int windowWidth, windowHeight;
        glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, windowWidth, windowHeight, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);

        //read a new frame and load into texture

        int64_t pts;
        if (!video_reader_read_frame(&vrState, frameData, &pts)){
            printf("couldn't load video frame ");
            return 1;
        }

    //    static bool firstFrame = true;
    //    if (firstFrame) {
    //        glfwSetTime(0.0);
    //        firstFrame = false;
    //    }
//
    //    double ptInSec = pts * (double )vrState.timeBase.num / (double)vrState.timeBase.den;
    //    while (ptInSec > glfwGetTime()) {
    //        glfwWaitEventsTimeout(ptInSec -glfwGetTime());
    //    }
//
        glBindTexture(GL_TEXTURE_2D, tex_handle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth, frameHeight,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, frameData);



        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex_handle);
        glBegin(GL_QUADS);
            glTexCoord2d(0,0 ); glVertex2i(500,500);
            glTexCoord2d(1,0 ); glVertex2i(500 + frameWidth,500);
            glTexCoord2d(1,1 ); glVertex2i(500 + frameWidth,500 + frameHeight);
            glTexCoord2d(0,1 ); glVertex2i(500,500 + frameHeight);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(window); //  change buffers plases
        glfwPollEvents();
    }
    return 0;
}
