
#pragma once
#include "../config.h"
#include "mesh_factory.h"

class Renderer
{
    public:
        Renderer(MTL::Device* device);
        ~Renderer();
        void draw(MTK::View* view);
        void setZoomFactor(float zoom);
        void setPanOffset(float x, float y);
    private:
        void buildMeshes();
        void buildShaders();
    
        MTL::RenderPipelineState* buildShader(const char* filename, const char* vertName, const char* fragName);
        MTL::Device* device;
        MTL::CommandQueue* commandQueue;
        
        MTL::RenderPipelineState* trianglePipeline, *generalPipeline;
        MTL::Buffer* triangleMesh;
//        Mesh quadMesh;
        Mesh svgMesh;
//        Mesh lineMesh; // Add line mesh
    Mesh normalMesh;
};

//https://ytyt.github.io/siiiimple-bezier/
