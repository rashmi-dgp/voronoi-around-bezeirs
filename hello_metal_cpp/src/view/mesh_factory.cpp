#include "mesh_factory.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "nanosvg.h"
#include "config.h"
#include <cmath>

using namespace std;

//logic: calculate tangent vectors-> normalize them -> calculate normal by 90deg rotation
std::pair<float, float> normalize(float x, float y) {
    float length = std::sqrt(x * x + y * y);
    return { x / length, y / length };
}

std::pair<float, float> CubicBezierTangent(const std::pair<float, float>& p0, const std::pair<float, float>& p1, const std::pair<float, float>& p2, const std::pair<float, float>& p3, float t) {
    float x = 3 * (1 - t) * (1 - t) * (p1.first - p0.first) + 6 * (1 - t) * t * (p2.first - p1.first) + 3 * t * t * (p3.first - p2.first);
    float y = 3 * (1 - t) * (1 - t) * (p1.second - p0.second) + 6 * (1 - t) * t * (p2.second - p1.second) + 3 * t * t * (p3.second - p2.second);
    return { x, y };
}

void GenerateUnitNormalVertices(const Vertex& startPoint, const Vertex& controlPoint1, const Vertex& controlPoint2, const Vertex& endPoint, int numLines, std::vector<Vertex> &vertices, std::vector<ushort> &indices, ushort &index) {
    // Calculate tangent vector at start point
    auto[startTangentX, startTangentY] = CubicBezierTangent({ startPoint.pos[0], startPoint.pos[1] }, { controlPoint1.pos[0], controlPoint1.pos[1] }, { controlPoint2.pos[0], controlPoint2.pos[1] }, { endPoint.pos[0], endPoint.pos[1] }, 0.0f);
    // Calculate tangent vector at end point
    auto[endTangentX, endTangentY] = CubicBezierTangent({ startPoint.pos[0], startPoint.pos[1] }, { controlPoint1.pos[0], controlPoint1.pos[1] }, { controlPoint2.pos[0], controlPoint2.pos[1] }, { endPoint.pos[0], endPoint.pos[1] }, 1.0f);

    // Normalize tangent vectors
    auto[startTangentXNorm, startTangentYNorm] = normalize(startTangentX, startTangentY);
    auto[endTangentXNorm, endTangentYNorm] = normalize(endTangentX, endTangentY);

    // Calculate normal vectors by rotating tangent vectors by 90 degrees
    float startNormalX = -startTangentYNorm;
    float startNormalY = startTangentXNorm;
    float endNormalX = -endTangentYNorm;
    float endNormalY = endTangentXNorm;

    // Define length for visualizing normals
    float normalLength = 0.1f;

    // Calculate end points of the normals (both positive and negative)
    Vertex startNormalEndPointPos = { { startPoint.pos[0] + startNormalX * normalLength, startPoint.pos[1] + startNormalY * normalLength }, { 1.0f, 0.0f, 0.0f } };
    Vertex startNormalEndPointNeg = { { startPoint.pos[0] - startNormalX * normalLength, startPoint.pos[1] - startNormalY * normalLength }, { 1.0f, 0.0f, 0.0f } };
    Vertex endNormalEndPointPos = { { endPoint.pos[0] + endNormalX * normalLength, endPoint.pos[1] + endNormalY * normalLength }, { 1.0f, 0.0f, 0.0f } };
    Vertex endNormalEndPointNeg = { { endPoint.pos[0] - endNormalX * normalLength, endPoint.pos[1] - endNormalY * normalLength }, { 1.0f, 0.0f, 0.0f } };

    //for rect:
    // Add vertices for normals (both positive and negative)
    vertices.push_back(endNormalEndPointPos);  // 0: End of positive normal at start point
    vertices.push_back(startNormalEndPointPos);    // 1: End of positive normal at end point
    vertices.push_back(startNormalEndPointNeg);    // 2: End of negative normal at end point
    vertices.push_back(endNormalEndPointNeg);  // 3: End of negative normal at start point
    vertices.push_back(endNormalEndPointPos);
    // Add indices to form the rectangle (two triangles)
    indices.push_back(index);     // 0
    indices.push_back(index + 1); // 1
    indices.push_back(index + 2); // 2

    indices.push_back(index );     // 0
    indices.push_back(index + 2); // 2
    indices.push_back(index + 3); // 3

    index += 4; // Move to the next set of vertices
}


void GenerateCubicBezierVertices(const Vertex& startPoint, const Vertex& controlPoint1, const Vertex& controlPoint2, const Vertex& endPoint, int numLines, std::vector<Vertex> &vertices, std::vector<ushort> &indices, ushort &index){
    for(float i = 0; i<=1; i+=0.002){
        float t = 1.0f*i;
        float t2 = t*t;
        float t3 = t2*t;
        float t_dash = (1.0f - t);
        float t_dash2 = t_dash*t_dash;
        float t_dash3 = t_dash*t_dash2;

        Vertex cur;
        cur.pos[0] = (t_dash3) * startPoint.pos[0] + (3*t_dash2*t)*controlPoint1
            .pos[0] + (3*t_dash*t2)*controlPoint2.pos[0] + t3*endPoint.pos[0];
        cur.pos[1] = (t_dash3) * startPoint.pos[1] + (3*t_dash2*t)*controlPoint1.pos[1] + (3*t_dash*t2)*controlPoint2.pos[1] + t3*endPoint.pos[1];


        std::clamp(cur.pos[0], -1.0f, 1.0f);
        std::clamp(cur.pos[1], -1.0f, 1.0f);
        cur.color= {0.0f,0.0f,0.0f};
        vertices.push_back(cur);
//        indices.push_back(index);
//        index++;
    }
}
std::vector<Vertex> GenerateCubicBezierVerticesFromPoints( const Vertex& p0, const Vertex& p1, const Vertex& p2, const Vertex& p3, int numLines){
    std::vector<Vertex> vertices;
    for(int i = 0; i<=numLines; i++){
        // generate the vertex for the current value of t
        float t = 1.0f*i/numLines;
        float t2 = t*t;
        float t3 = t2*t;
        float t_dash = (1.0f - t);
        float t_dash2 = t_dash*t_dash;
        float t_dash3 = t_dash*t_dash;
        Vertex cur;
        cur.pos[0] = (t_dash3) * p0.pos[0] + (3*t_dash2*t)*p1.pos[0] + (3*t_dash*t2)*p2.pos[0] + t3*p3.pos[0];
        cur.pos[1] = (t_dash3) * p0.pos[1] + (3*t_dash2*t)*p1.pos[1] + (3*t_dash*t2)*p2.pos[1] + t3*p3.pos[1];
        std::clamp(cur.pos[0], -1.0f, 1.0f);
        std::clamp(cur.pos[1], -1.0f, 1.0f);
        vertices.push_back(cur);
//        return cur;
    }
    return vertices;
}
void GenerateCBFromBezeir(const std::vector<Vertex>& vertices, std::vector<Vertex>& newVertices) {
    int noOfGroups = vertices.size() / 4;
    for (int i = 0; i < noOfGroups; ++i) {
        std::vector<Vertex> bezierVertices = GenerateCubicBezierVerticesFromPoints(
            vertices[i * 4], vertices[i * 4 + 1], vertices[i * 4 + 2], vertices[i * 4 + 3], 1); // Assuming 100 lines
        newVertices.insert(newVertices.end(), bezierVertices.begin(), bezierVertices.end());
    }
}
MTL::Buffer* MeshFactory::buildTriangle(MTL::Device* device) {
    // Declare the data to send
    Vertex vertices[3] = {
        {{-0.75, -0.75}, {1.0, 1.0, 0.0}},
        {{ 0.75, -0.75}, {1.0, 1.0, 0.0}},
        {{  0.0,  0.75}, {1.0, 1.0, 0.0}}
    };
    // Create a buffer to hold it
    MTL::Buffer* buffer = device->newBuffer(3 * sizeof(Vertex), MTL::ResourceStorageModeShared);
    // Upload to buffer
    memcpy(buffer->contents(), vertices, 3 * sizeof(Vertex));
    return buffer;
}
Mesh MeshFactory::buildQuad(MTL::Device* device) {
    Mesh mesh;
    // Declare the data to send
    Vertex vertices[4] = {
        {{-0.75, -0.75}, {0.0, 0.0, 0.0}},
        {{ 0.75, -0.75}, {0.0, 0.0, 0.0}},
        {{ 0.75,  0.75}, {0.0, 0.0, 0.0}},
        {{-0.75,  0.75}, {0.0, 0.0, 0.0}},
    };
    ushort indices[] = {
        0, 1,  // Bottom line
        1, 2,  // Right line
        2, 3,  // Top line
        3, 0   // Left line
    };
    // Vertex buffer
    mesh.vertexBuffer = device->newBuffer(4 * sizeof(Vertex), MTL::ResourceStorageModeShared);
    memcpy(mesh.vertexBuffer->contents(), vertices, 4 * sizeof(Vertex));

    // Index buffer
    mesh.indexBuffer = device->newBuffer(8 * sizeof(ushort), MTL::ResourceStorageModeShared);
    memcpy(mesh.indexBuffer->contents(), indices, 8 * sizeof(ushort));

    return mesh;
}

Mesh MeshFactory::buildSVG(MTL::Device* device, const char* svgFilePath) {
    Mesh mesh;
    std::vector<Vertex> vertices;
    std::vector<ushort> indices;
    
    // Load SVG
    NSVGimage* image = nsvgParseFromFile(svgFilePath, "px", 96);
    if (!image) {
        std::cerr << "Could not open SVG image." << std::endl;
        return mesh;
    }

    ushort index = 0;
    std::ofstream outFile("cubic_bezier_shapes.txt");
    
    // Calculate bounds
    float xMax = 0.0f, yMax = 0.0f, xMin = 0.0f, yMin = 0.0f;
    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        xMin = shape->bounds[0];
        yMin = shape->bounds[1];
        xMax = shape->bounds[2];
        yMax = shape->bounds[3];
    }
    std::cout << xMin << " " << xMax << " " << yMin << " " << yMax << "\n";

    float widthOfImage = image->width;
    float heightOfImage = image->height;
    float div = std::max(widthOfImage, heightOfImage);
    std::cout << widthOfImage << " " << heightOfImage << "\n";

    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
            for (int i = 0; i < path->npts - 1; i += 3) {
                float* p = &path->pts[i * 2];
                outFile << "Shape ID: " << (shape->id ? shape->id : "none") << ", Path Points:";
                std::vector<Vertex> temp;
                for (int j = 0; j < 4; ++j) {
                    float xcoord = 2 * ((p[j * 2]) / div) - 1.0f;
                    float ycoord = 1.0f - 2 * ((p[j * 2 + 1]) / div);
                    temp.push_back({ { xcoord, ycoord }, { 1.0f, 0.0f, 0.0f } });
                    outFile << "[" << xcoord << ", " << ycoord << "]";
                    if (j < 3) outFile << ", ";
                }
                GenerateCubicBezierVertices(temp[0], temp[1], temp[2], temp[3], 100, vertices, indices, index);
                //index++;
                GenerateUnitNormalVertices(temp[0], temp[1], temp[2], temp[3], 100, vertices, indices, index);
                outFile << std::endl;
            }
        }
    }
    
    
    
    mesh.vertexBuffer = device->newBuffer(vertices.size() * sizeof(Vertex), MTL::ResourceStorageModeShared);
    memcpy(mesh.vertexBuffer->contents(), vertices.data(), vertices.size() * sizeof(Vertex));

    for (int i = 1; i < vertices.size(); i++) {
        indices.push_back(i - 1);
        indices.push_back(i);
    }

    mesh.indexBuffer = device->newBuffer(indices.size() * sizeof(ushort), MTL::ResourceStorageModeShared);
    memcpy(mesh.indexBuffer->contents(), indices.data(), indices.size() * sizeof(ushort));

    return mesh;
}

//
//
//Mesh MeshFactory::buildNormal(MTL::Device* device, const char* svgFilePath) {
//    Mesh mesh;
//    std::vector<Vertex> vertices;
//    std::vector<ushort> indices;
//    // Load SVG
//    NSVGimage* image = nsvgParseFromFile(svgFilePath, "px", 96);
//    if (!image) {
//        std::cerr << "Could not open SVG image." << std::endl;
//        return mesh;
//    }
//    ushort index = 0;
//    std::ofstream outFile("cubic_bezier_shapes.txt");
//    // go through all shapes and calculate bounds: xMax and yMax
//    float xMax=0.0f, yMax=0.0f, xMin=0.0f, yMin=0.0f;
//    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next){
//        xMin = shape->bounds[0];
//        yMin = shape->bounds[1];
//        xMax = shape->bounds[2];
//        yMax = shape->bounds[3];
//    }
//    std::cout<<xMin<<" "<<xMax<<" "<<yMin<<" "<<yMax<<"\n";
//    float imageWidht, imageHeight;
////    NSVGimage* nsvgImage;
//    float widthOfImage = image->width;
//    float heightOfImage= image->height;
//    float div = max(widthOfImage,heightOfImage);
//    std::cout<<widthOfImage<<" "<<heightOfImage<<"\n";
//    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next){
//        for (NSVGpath* path = shape->paths; path != nullptr; path = path->next){
//            for (int i = 0; i < path->npts - 1; i += 3){
//                float* p = &path->pts[i * 2];
//                outFile << "Shape ID: " << (shape->id ? shape->id : "none") << ", Path Points:";
//                std::vector<Vertex> temp;
//                for (int j = 0; j < 4; ++j){
//                    float xcoord=0.0f, ycoord=0.0f;
//                    xcoord = 2 * ((p[j*2])/div)  - 1.0f;
//                    ycoord = 1.0f - 2 * ((p[j*2 + 1])/div) ;
//                    temp.push_back({{xcoord,ycoord}, {1.0, 0.0, 0.0}});
////                    indices.push_back(index++);
//                    outFile << "[" << 2*(p[j * 2] / 600) - 1.0f<< ", " << 2*(p[j * 2 + 1] / 600)-1.0f << "]";
//                    if (j < 3) outFile << ", ";
//                }
//                GenerateUnitNormalVertices(temp[0], temp[1], temp[2], temp[3], 1, vertices, indices, index);
//                outFile << std::endl;
//            }
//        }
//    }
//    mesh.vertexBuffer = device->newBuffer(vertices.size() * sizeof(Vertex), MTL::ResourceStorageModeShared);
//    memcpy(mesh.vertexBuffer->contents(), vertices.data(), vertices.size() * sizeof(Vertex));
////    int i = 0;
//    for(int i =1; i< vertices.size(); i++)
//    {
//        indices.push_back(i-1);
//        indices.push_back(i);
//    }
//    mesh.indexBuffer = device->newBuffer(indices.size() * sizeof(ushort), MTL::ResourceStorageModeShared);
//    memcpy(mesh.indexBuffer->contents(), indices.data(), indices.size() * sizeof(ushort));
//    return mesh;
//   }


/*
Mesh MeshFactory::buildSVG(MTL::Device* device, const char* svgFilePath) {
    Mesh mesh;
    std::vector<Vertex> vertices;
    std::vector<ushort> indices;
    
    // Load SVG
    NSVGimage* image = nsvgParseFromFile(svgFilePath, "px", 96);
    if (!image) {
        std::cerr << "Could not open SVG image." << std::endl;
        return mesh;
    }
    
    ushort index = 0;
    float width = sqrt(2); // Width of the rectangle
    
    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
            bool isFirstSegment = true;
            float* p = path->pts;
            
            for (int i = 0; i < path->npts - 1; i += 3, p += 6) {
                std::vector<Vertex> temp;
                for (int j = 0; j < 4; ++j) {
                    float xcoord = 2 * (p[j * 2] / 600.0f) - 1.0f;
                    float ycoord = 1.0f - 2 * (p[j * 2 + 1] / 600.0f);
                    temp.push_back({{xcoord, ycoord}, {0.0, 0.0, 0.0}}); // Black color for SVG path
                }
                
                // Add the SVG path segment as black
                for (int j = 0; j < 4; ++j) {
                    vertices.push_back(temp[j]);
                }
                
                indices.push_back(index);
                indices.push_back(index + 1);
                indices.push_back(index + 2);
                indices.push_back(index);
                indices.push_back(index + 2);
                indices.push_back(index + 3);

                index += 4;
                
                // Generate and add the enclosing rectangle as red only for the first segment
                if (isFirstSegment) {
                    GenerateRectangleForPath(temp[0], temp[3], width, vertices, indices, index);
                    isFirstSegment = false;
                }
            }
        }
    }
    
    mesh.vertexBuffer = device->newBuffer(vertices.size() * sizeof(Vertex), MTL::ResourceStorageModeShared);
    memcpy(mesh.vertexBuffer->contents(), vertices.data(), vertices.size() * sizeof(Vertex));
    mesh.indexBuffer = device->newBuffer(indices.size() * sizeof(ushort), MTL::ResourceStorageModeShared);
    memcpy(mesh.indexBuffer->contents(), indices.data(), indices.size() * sizeof(ushort));
    
    return mesh;
}

*/
/*
Mesh MeshFactory::buildSVG(MTL::Device* device, const char* svgFilePath) {
    Mesh mesh;
    std::vector<Vertex> vertices;
    std::vector<ushort> indices;
    
    // Load SVG
    NSVGimage* image = nsvgParseFromFile(svgFilePath, "px", 96);
    if (!image) {
        std::cerr << "Could not open SVG image." << std::endl;
        return mesh;
    }
    
    ushort index = 0;
    float width = sqrt(2); // Width of the rectangle
    
    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
            float* p = path->pts;
            for (int i = 0; i < path->npts - 1; i += 3, p += 6) {
                std::vector<Vertex> temp;
                for (int j = 0; j < 4; ++j) {
                    float xcoord = 2 * (p[j * 2] / 600.0f) - 1.0f;
                    float ycoord = 1.0f - 2 * (p[j * 2 + 1] / 600.0f);
                    temp.push_back({{xcoord, ycoord}, {0.0, 0.0, 0.0}}); // Black color for SVG path
                }
                
                // Add the SVG path segment as black
                for (int j = 0; j < 4; ++j) {
                    vertices.push_back(temp[j]);
                }
                
                indices.push_back(index);
                indices.push_back(index + 1);
                indices.push_back(index + 2);
                indices.push_back(index);
                indices.push_back(index + 2);
                indices.push_back(index + 3);

                index += 4;
                
                // Generate and add the enclosing rectangles as red
                GenerateRectangleForPath(temp[0], temp[3], width, vertices, indices, index);
            }
        }
    }
    
    mesh.vertexBuffer = device->newBuffer(vertices.size() * sizeof(Vertex), MTL::ResourceStorageModeShared);
    memcpy(mesh.vertexBuffer->contents(), vertices.data(), vertices.size() * sizeof(Vertex));
    mesh.indexBuffer = device->newBuffer(indices.size() * sizeof(ushort), MTL::ResourceStorageModeShared);
    memcpy(mesh.indexBuffer->contents(), indices.data(), indices.size() * sizeof(ushort));
    
    return mesh;
}
*/


/*
void GenerateRectanglesForPath(const Vertex& start, const Vertex& end, float width, std::vector<Vertex>& vertices, std::vector<ushort>& indices, ushort& index) {
     Calculate the direction of the path
    float dx = end.pos[0] - start.pos[0];
    float dy = end.pos[1] - start.pos[1];
    
     Calculate the length of the path
    float length = sqrt(dx * dx + dy * dy);
    
     Normalize the direction
    float nx = dx / length;
    float ny = dy / length;
    
     Calculate the normal vectors
    float perpX = -ny * width / 2;
    float perpY = nx * width / 2;

     Calculate the corners of the rectangle
    Vertex corners[4];
    corners[0] = {{start.pos[0] + perpX, start.pos[1] + perpY}, {1.0, 0.0, 0.0}};  Red color
    corners[1] = {{start.pos[0] - perpX, start.pos[1] - perpY}, {1.0, 0.0, 0.0}};  Red color
    corners[2] = {{end.pos[0] - perpX, end.pos[1] - perpY}, {1.0, 0.0, 0.0}};  Red color
    corners[3] = {{end.pos[0] + perpX, end.pos[1] + perpY}, {1.0, 0.0, 0.0}};  Red color
    
     Add corners to vertices
    for (int i = 0; i < 4; ++i) {
        vertices.push_back(corners[i]);
    }
    
     Add indices for two triangles forming the rectangle
    indices.push_back(index);
    indices.push_back(index + 1);
    indices.push_back(index + 2);
    indices.push_back(index);
    indices.push_back(index + 2);
    indices.push_back(index + 3);

    index += 4;
}
Mesh MeshFactory::buildSVG(MTL::Device* device, const char* svgFilePath) {
    Mesh mesh;
    std::vector<Vertex> vertices;
    std::vector<ushort> indices;
    
     Load SVG
    NSVGimage* image = nsvgParseFromFile(svgFilePath, "px", 96);
    if (!image) {
        std::cerr << "Could not open SVG image." << std::endl;
        return mesh;
    }
    
    ushort index = 0;
    float width = sqrt(2);  Width of the rectangle
    
    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
            for (int i = 0; i < path->npts - 1; i += 3) {
                float* p = &path->pts[i * 2];
                
                std::vector<Vertex> temp;
                for (int j = 0; j < 4; ++j) {
                    float xcoord = 2 * (p[j * 2] / 600.0f) - 1.0f;
                    float ycoord = 1.0f - 2 * (p[j * 2 + 1] / 600.0f);
                    temp.push_back({{xcoord, ycoord}, {0.0, 0.0, 0.0}});  Black color for SVG path
                }
                
                 Add the SVG path segment as black
                for (int j = 0; j < 4; ++j) {
                    vertices.push_back(temp[j]);
                }
                
                indices.push_back(index);
                indices.push_back(index + 1);
                indices.push_back(index + 2);
                indices.push_back(index);
                indices.push_back(index + 2);
                indices.push_back(index + 3);

                index += 4;
                
                 Generate and add the enclosing rectangles as red
                GenerateRectanglesForPath(temp[0], temp[3], width, vertices, indices, index);
            }
        }
    }
    
    mesh.vertexBuffer = device->newBuffer(vertices.size() * sizeof(Vertex), MTL::ResourceStorageModeShared);
    memcpy(mesh.vertexBuffer->contents(), vertices.data(), vertices.size() * sizeof(Vertex));
    mesh.indexBuffer = device->newBuffer(indices.size() * sizeof(ushort), MTL::ResourceStorageModeShared);
    memcpy(mesh.indexBuffer->contents(), indices.data(), indices.size() * sizeof(ushort));
    
    return mesh;
}

*/
