#include <codotaku/camera2d.h>
#include <codotaku/runtime.h>
#include <codotaku/geometry.h>
#include <codotaku/polygon.h>

#include <cmath>
#include <vector>

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
};

int main() {
    codotaku::Runtime app({ .title = "Triangle", .windowSize = {640, 480} });

    std::vector<Vertex> verts;
    auto indices = codotaku::generatePolygon(3, 200.0f, 0,
        [&](glm::vec2 p, uint32_t) { verts.push_back({p, {1, 0, 0}}); });

    auto belt = app.createBelt();
    auto geom = app.createGeometry<Vertex>(belt, verts, indices);
    belt.flush();

    codotaku::VertexInputBuilder vib;
    vib.addBuffer(0, sizeof(Vertex), SDL_GPU_VERTEXINPUTRATE_VERTEX);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(Vertex, position));
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Vertex, color));

    auto pipeline = app.loadPipeline("examples/triangle", vib, app.swapchainFormat(),
                                      "VSMain", "PSMain", 1, 0);

    codotaku::Camera2D camera;

    app.run([&](const codotaku::FrameContext &ctx) {
        auto &in = app.input();
        if (in.mouseWheel != 0.0f) {
            camera.setZoom(camera.zoom() * std::pow(1.1f, in.mouseWheel));
        }
        if (in.mouseLeft) {
            auto pos = camera.position();
            pos.x -= in.mouseDX;
            pos.y += in.mouseDY;
            camera.setPosition(pos);
        }

        auto pass = ctx.beginRenderPass();
        auto vp = camera.viewProjection(ctx.swapchainSize);
        pass.pushVertexUniform(0, &vp, sizeof(vp));
        pass.bindPipeline(pipeline);
        geom.drawIndexed(pass);
    });
}
