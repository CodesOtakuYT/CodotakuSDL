#include <codotaku/runtime.h>
#include <codotaku/geometry.h>

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

    const Uint32 indices[] = {0, 1, 2};

    codotaku::StagingBelt belt(app.device());
    codotaku::Geometry<Vertex> geom(app, vertices, indices);
    geom.upload(belt, vertices, indices);
    belt.flush();

    codotaku::VertexInputBuilder vib;
    vib.addBuffer(0, sizeof(Vertex), SDL_GPU_VERTEXINPUTRATE_VERTEX);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 0);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, sizeof(float) * 2);

    auto pipeline = app.loadPipeline("examples/triangle", vib, app.swapchainFormat());

    app.run([&](const codotaku::FrameContext &ctx) {
        auto pass = ctx.beginRenderPass();
        pass.bindPipeline(pipeline);
        geom.drawIndexed(pass);
    });
}
