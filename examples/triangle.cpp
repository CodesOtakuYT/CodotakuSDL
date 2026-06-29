#include <codotaku/runtime.h>
#include <codotaku/geometry.h>
#include <codotaku/polygon.h>

#include <vector>

struct Vertex {
    float x, y;
    float r, g, b;
};

int main() {
    codotaku::Runtime app({ .title = "Triangle", .windowSize = {640, 480} });

    std::vector<Vertex> verts;
    auto indices = codotaku::generatePolygon(3, 0.5f, 0,
        [&](glm::vec2 p, uint32_t) { verts.push_back({p.x, p.y, 1, 0, 0}); });

    auto belt = app.createBelt();
    auto geom = app.createGeometry<Vertex>(belt, verts, indices);
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
