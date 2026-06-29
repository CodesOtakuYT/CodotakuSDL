#include <codotaku/runtime.h>

#include <SDL3/SDL_log.h>

struct Vertex
{
    float x, y;
    float r, g, b;
};

int main()
{
    SDL_LogInfo(SDL_LOG_CATEGORY_GPU, "Creating runtime...");
    codotaku::Runtime app({ .title = "Triangle",
                            .windowSize = { 640, 480 } });

    auto *device = app.device();

    SDL_LogInfo(SDL_LOG_CATEGORY_GPU, "Loading shaders...");
    auto vert = app.loadShader("examples/triangle_vert", SDL_GPU_SHADERSTAGE_VERTEX, "VSMain");
    auto frag = app.loadShader("examples/triangle_frag", SDL_GPU_SHADERSTAGE_FRAGMENT, "PSMain");

    const Vertex vertices[] = {
        { 0.0f, -0.5f, 1.0f, 0.0f, 0.0f },
        { 0.5f, 0.5f, 0.0f, 1.0f, 0.0f },
        { -0.5f, 0.5f, 0.0f, 0.0f, 1.0f },
    };

    const Uint16 indices[] = { 0, 1, 2 };

    SDL_LogDebug(SDL_LOG_CATEGORY_GPU, "Creating buffers...");
    codotaku::StagingBelt belt(device);
    auto vBuf = app.createBuffer(
        SDL_GPU_BUFFERUSAGE_VERTEX,
        { reinterpret_cast<const Uint8 *>(vertices), sizeof(vertices) },
        belt);
    auto iBuf = app.createBuffer(
        SDL_GPU_BUFFERUSAGE_INDEX,
        { reinterpret_cast<const Uint8 *>(indices), sizeof(indices) },
        belt);
    belt.flush();

    SDL_GPUVertexBufferDescription vbDesc{};
    vbDesc.slot = 0;
    vbDesc.pitch = sizeof(Vertex);
    vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

    SDL_GPUVertexAttribute attrs[2];
    attrs[0].location = 0;
    attrs[0].buffer_slot = 0;
    attrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    attrs[0].offset = 0;
    attrs[1].location = 1;
    attrs[1].buffer_slot = 0;
    attrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attrs[1].offset = sizeof(float) * 2;

    SDL_GPUVertexInputState vertexInput{};
    vertexInput.num_vertex_buffers = 1;
    vertexInput.vertex_buffer_descriptions = &vbDesc;
    vertexInput.num_vertex_attributes = 2;
    vertexInput.vertex_attributes = attrs;

    auto swapchainFormat = SDL_GetGPUSwapchainTextureFormat(device, app.window());

    SDL_GPUColorTargetDescription colorTarget{};
    colorTarget.format = swapchainFormat;

    SDL_LogDebug(SDL_LOG_CATEGORY_GPU, "Creating graphics pipeline...");
    SDL_GPUGraphicsPipelineCreateInfo pi{};
    pi.vertex_shader = vert.handle();
    pi.fragment_shader = frag.handle();
    pi.vertex_input_state = vertexInput;
    pi.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pi.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pi.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
    pi.target_info.color_target_descriptions = &colorTarget;
    pi.target_info.num_color_targets = 1;

    auto *pipeline = SDL_CreateGPUGraphicsPipeline(device, &pi);
    if (!pipeline) {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Failed to create graphics pipeline: %s", SDL_GetError());
        return 1;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_GPU, "Entering main loop...");
    app.run([&](const codotaku::FrameContext &ctx) {
        auto pass = ctx.beginRenderPass({ 0.0f, 0.0f, 0.0f, 1.0f });
        pass.bindPipeline(pipeline);
        pass.bindVertexBuffer(vBuf.handle());
        pass.bindIndexBuffer(iBuf.handle(), SDL_GPU_INDEXELEMENTSIZE_16BIT);
        pass.setViewport(0.0f, 0.0f, 640.0f, 480.0f);
        pass.drawIndexed(3, 1, 0, 0, 0);
    });

    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}
