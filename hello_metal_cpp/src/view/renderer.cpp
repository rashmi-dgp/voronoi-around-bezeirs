#include "renderer.h"
Renderer::Renderer(MTL::Device* device):
device(device->retain()){
    commandQueue = device->newCommandQueue();
    buildMeshes();
    buildShaders();
}
Renderer::~Renderer() {
    triangleMesh->release();
    trianglePipeline->release();
    generalPipeline->release();
    svgMesh.vertexBuffer->release(); // Release SVG vertex buffer
    svgMesh.indexBuffer->release(); // Release SVG index buffer
    commandQueue->release();
    device->release();
}
void Renderer::buildMeshes() {
    triangleMesh = MeshFactory::buildTriangle(device);
    svgMesh = MeshFactory::buildSVG(device, "//Users/rashmig/Desktop/filled_rect_around_shapes 2/square.svg");
//    normalMesh = MeshFactory::buildNormal(device, "/Users/rashmig/Desktop/line copy 2/horizontal-line-svgrepo-com.svg");
}
void Renderer::buildShaders() {
    trianglePipeline = buildShader("shaders/triangle.metal", "vertexMain", "fragmentMain");
    generalPipeline = buildShader("shaders/general_shader.metal", "vertexMainGeneral", "fragmentMainGeneral");
}
MTL::RenderPipelineState* Renderer::buildShader(const char* filename, const char* vertName, const char* fragName) {
    // Read the source code from the file.
    std::ifstream file(filename);
    std::stringstream reader;
    reader << file.rdbuf();
    std::string raw_string = reader.str();
    NS::String* source_code = NS::String::string(raw_string.c_str(), NS::StringEncoding::UTF8StringEncoding);

    // A Metal Library constructs functions from source code
    NS::Error* error = nullptr;
    MTL::CompileOptions* options = nullptr;
    MTL::Library* library = device->newLibrary(source_code, options, &error);
    if (!library) {
        std::cout << error->localizedDescription()->utf8String() << std::endl;
    }

    NS::String* vertexName = NS::String::string(vertName, NS::StringEncoding::UTF8StringEncoding);
    MTL::Function* vertexMain = library->newFunction(vertexName);

    NS::String* fragmentName = NS::String::string(fragName, NS::StringEncoding::UTF8StringEncoding);
    MTL::Function* fragmentMain = library->newFunction(fragmentName);

    MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineDescriptor->setVertexFunction(vertexMain);
    pipelineDescriptor->setFragmentFunction(fragmentMain);
    pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    auto attributes = vertexDescriptor->attributes();
    auto positionDescriptor = attributes->object(0);
    positionDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    positionDescriptor->setBufferIndex(0);
    positionDescriptor->setOffset(0);
    auto colorDescriptor = attributes->object(1);
    colorDescriptor->setFormat(MTL::VertexFormat::VertexFormatFloat3);
    colorDescriptor->setBufferIndex(0);
    colorDescriptor->setOffset(4 * sizeof(float));
    auto layoutDescriptor = vertexDescriptor->layouts()->object(0);
    layoutDescriptor->setStride(8 * sizeof(float));

    pipelineDescriptor->setVertexDescriptor(vertexDescriptor);

    MTL::RenderPipelineState* pipeline = device->newRenderPipelineState(pipelineDescriptor, &error);
    if (!pipeline) {
        std::cout << error->localizedDescription()->utf8String() << std::endl;
    }

    vertexMain->release();
    fragmentMain->release();
    pipelineDescriptor->release();
    library->release();
    file.close();
    return pipeline;
}



void Renderer::draw(MTK::View* view) {
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPass = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPass);
    encoder->setRenderPipelineState(generalPipeline);
    // Draw SVG
    encoder->setVertexBuffer(svgMesh.vertexBuffer, 0, 0);
    MTL::PrimitiveType primitiveType = MTL::PrimitiveType::PrimitiveTypeLine;
    encoder->drawIndexedPrimitives(primitiveType, svgMesh.indexBuffer->length() / sizeof(ushort), MTL::IndexType::IndexTypeUInt16, svgMesh.indexBuffer, 0);
    encoder->endEncoding();
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    pool->release();
}

