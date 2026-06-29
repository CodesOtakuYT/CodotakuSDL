#include <codotaku/runtime.h>

struct Vertex {
    float x, y;
    float r, g, b;
};

int main() {
    codotaku::Runtime app({
        .title = "Triangle",
        .windowSize = {640, 480}
    });

    const Vertex vertices[] = {
        {0.0f, -0.5f, 1.0f, 0.0f, 0.0f},
        {0.5f, 0.5f, 0.0f, 1.0f, 0.0f},
        {-0.5f, 0.5f, 0.0f, 0.0f, 1.0f},
    };

    const Uint16 indices[] = {0, 1, 2};

    constexpr size_t vertSize = sizeof(vertices);
    constexpr size_t indexSize = sizeof(indices);

    codotaku::StagingBelt belt(app.device());
    auto buf = app.createBuffer(
        SDL_GPU_BUFFERUSAGE_VERTEX | SDL_GPU_BUFFERUSAGE_INDEX,
        static_cast<Uint32>(vertSize + indexSize));
    belt.upload(buf.handle(), 0, vertices, vertSize);
    belt.upload(buf.handle(), vertSize, indices, indexSize);
    belt.flush();

    codotaku::VertexInputBuilder vib;
    vib.addBuffer(0, sizeof(Vertex), SDL_GPU_VERTEXINPUTRATE_VERTEX);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 0);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, sizeof(float) * 2);

    auto pipeline = app.loadPipeline("examples/triangle", vib, app.swapchainFormat());

    app.run([&](const codotaku::FrameContext &ctx) {
        auto pass = ctx.beginRenderPass({0.0f, 0.0f, 0.0f, 1.0f});
        pass.bindPipeline(pipeline.handle());
        pass.bindVertexBuffer(buf.handle(), 0, 0);
        pass.bindIndexBuffer(buf.handle(), SDL_GPU_INDEXELEMENTSIZE_16BIT, vertSize);
        pass.setViewport(0.0f, 0.0f, 640.0f, 480.0f);
        pass.drawIndexed(3, 1, 0, 0, 0);
    });
}
