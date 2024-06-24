

#pragma once
#include "../config.h"
#include "nanosvg.h"
#include <vector>
struct svgVertex {
    float position[2];
    float color[3];
};

//struct Mesh {
//    MTL::Buffer* vertexBuffer, *indexBuffer;
//};
struct Mesh {
    MTL::Buffer* vertexBuffer;
    MTL::Buffer* indexBuffer;
    
};

namespace MeshFactory {
    MTL::Buffer* buildTriangle(MTL::Device* device);
    Mesh buildQuad(MTL::Device* device);
//svg and line ka added
    Mesh buildSVG(MTL::Device* device, const char* svgFilePath); // New method for SVG
    Mesh buildLine(MTL::Device* device); // New method for Line
    Mesh buildRectanglesAlongSVG(MTL::Device* device, const char* svgFilePath);
//    Mesh buildNormal(MTL::Device* device, const char* svgFilePath);
}
