#ifndef ZSPIPELINE_H
#define ZSPIPELINE_H
#define GLEW_STATIC
#include <GL/glew.h>
#include "zs-shader.h"
#include "zs-mesh.h"
#include "zs-texture.h"

#include <QMainWindow>
#include <SDL2/SDL.h>

#define GO_RENDER_TYPE_NONE 0
#define GO_RENDER_TYPE_TILE 1
#define GO_RENDER_TYPE_3D 2

#define PIPELINE_STATE_DEFAULT 0
#define PIPELINE_STATE_PICKING 1

class RenderPipeline{
public:
    ZSPIRE::Shader tile_shader; //Shader to draw tiles
    ZSPIRE::Shader diffuse_shader;
    ZSPIRE::Shader pick_shader; //Shader to draw & pick objects
public:
    int current_state;
    void setup();
    bool InitGLEW();
    void render(SDL_Window* w, void* projectedit_ptr);
    unsigned int render_getpickedObj(void* projectedit_ptr, int mouseX, int mouseY);
    void updateShadersCameraInfo(ZSPIRE::Camera* cam_ptr);
    ZSPIRE::Shader* processShaderOnObject(void* _obj);

    RenderPipeline();
};


#endif // ZSPIPELINE_H
