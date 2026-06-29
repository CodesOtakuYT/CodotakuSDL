#include <codotaku/camera2d.h>
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
    auto indices = codotaku::generatePolygon(3, 200.0f, 0,
        [&](glm::vec2 p, uint32_t) { verts.push_back({p.x, p.y, 1, 0, 0}); });

    auto belt = app.createBelt();
    auto geom = app.createGeometry<Vertex>(belt, verts, indices);
    belt.flush();

    codotaku::VertexInputBuilder vib;
    vib.addBuffer(0, sizeof(Vertex), SDL_GPU_VERTEXINPUTRATE_VERTEX);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, 0);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, sizeof(float) * 2);

    auto pipeline = app.loadPipeline("examples/triangle", vib, app.swapchainFormat(),
                                      "VSMain", "PSMain", 1, 0);

    codotaku::Camera2D camera;

    app.run([&](const codotaku::FrameContext &ctx) {
        auto &in = app.input();
        if (in.mouseLeft) {
            auto pos = camera.position();
            pos -= glm::vec2(in.mouseDX, in.mouseDY) / camera.zoom();
            camera.setPosition(pos);
        }

        auto pass = ctx.beginRenderPass();
        auto vp = camera.viewProjection(ctx.swapchainSize);
        pass.pushVertexUniform(0, &vp, sizeof(vp));
        pass.bindPipeline(pipeline);
        geom.drawIndexed(pass);
    });
}
