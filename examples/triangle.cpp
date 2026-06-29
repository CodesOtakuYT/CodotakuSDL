#include <codotaku/runtime.h>

#include <SDL3/SDL_log.h>

struct Vertex {
    float x, y;
    float r, g, b;
};

int main() {
    SDL_LogInfo(SDL_LOG_CATEGORY_GPU, "Creating runtime...");
    codotaku::Runtime app({
        .title = "Triangle",
        .windowSize = {640, 480}
    });

    SDL_LogInfo(SDL_LOG_CATEGORY_GPU, "Loading shaders...");
    auto vert = app.loadShader("examples/triangle_vert", "VSMain");
    auto frag = app.loadShader("examples/triangle_frag", "PSMain");

    const Vertex vertices[] = {
        {0.0f, -0.5f, 1.0f, 0.0f, 0.0f},
        {0.5f, 0.5f, 0.0f, 1.0f, 0.0f},
        {-0.5f, 0.5f, 0.0f, 0.0f, 1.0f},
    };

    const Uint16 indices[] = {0, 1, 2};

    SDL_LogDebug(SDL_LOG_CATEGORY_GPU, "Creating buffers...");
    codotaku::StagingBelt belt(app.device());
    auto vBuf = app.createVertexBuffer(vertices, sizeof(vertices), belt);
    auto iBuf = app.createIndexBuffer(indices, sizeof(indices), belt);
    belt.flush();

    codotaku::VertexInputBuilder vib;
    vib.addBuffer(0, sizeof(Vertex), SDL_GPU_VERTEXINPUTRATE_VERTEX);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 0);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, sizeof(float) * 2);

    SDL_LogDebug(SDL_LOG_CATEGORY_GPU, "Creating graphics pipeline...");
    auto pipeline = app.createPipeline(vert, frag, vib, app.swapchainFormat());

    SDL_LogInfo(SDL_LOG_CATEGORY_GPU, "Entering main loop...");
    app.run([&](const codotaku::FrameContext &ctx) {
        auto pass = ctx.beginRenderPass({0.0f, 0.0f, 0.0f, 1.0f});
        pass.bindPipeline(pipeline.handle());
        pass.bindVertexBuffer(vBuf.handle());
        pass.bindIndexBuffer(iBuf.handle(), SDL_GPU_INDEXELEMENTSIZE_16BIT);
        pass.setViewport(0.0f, 0.0f, 640.0f, 480.0f);
        pass.drawIndexed(3, 1, 0, 0, 0);
    });
}
