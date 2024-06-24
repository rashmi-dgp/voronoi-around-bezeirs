
#pragma once
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <vector>

struct Vertex {
    simd::float2 pos;   //(x,y)
    simd::float3 color; //(r,g,b)
    
    Vertex()
    {
        pos = {0.0f, 0.0f};
        color = {0.0f, 0.0f, 0.0f};
    }
    
    Vertex (std::vector<float> _pos, std::vector<float> _color)
    {
        pos = {_pos[0], _pos[1]};
        color = {_color[0], _color[1], _color[2]};
    }
};
