#include <codotaku/camera2d.h>
#include <codotaku/runtime.h>
#include <codotaku/geometry.h>
#include <codotaku/polygon.h>

#include <cmath>
#include <glm/vec3.hpp>
#include <random>
#include <vector>

struct PosVertex {
    glm::vec2 position;
};

struct Instance {
    glm::vec2 offset;
    glm::vec3 color;
    float scale;
};

int main() {
    codotaku::Runtime app({.title = "Triangles", .windowSize = {1024, 768}});

    std::vector<PosVertex> posVerts;
    auto idx = codotaku::generatePolygon(3, 1.0f, 0,
                                         [&](glm::vec2 p, auto) { posVerts.push_back({p}); });

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> pd(-400.0f, 400.0f);
    std::uniform_real_distribution<float> cd(0.0f, 1.0f);
    std::uniform_real_distribution<float> sd(20.0f, 80.0f);

    std::vector<Instance> instances;
    for (int i = 0; i < 10; ++i)
        instances.push_back({{pd(rng), pd(rng)}, {cd(rng), cd(rng), cd(rng)}, sd(rng)});

    auto belt = app.createBelt();
    auto geom = app.createGeometry<PosVertex>(belt, posVerts, idx);
    auto instBuf = app.createBuffer(SDL_GPU_BUFFERUSAGE_VERTEX, instances.data(), instances.size() * sizeof(Instance),
                                    belt);
    belt.flush();

    codotaku::VertexInputBuilder vib;
    vib.addBuffer(0, sizeof(PosVertex), SDL_GPU_VERTEXINPUTRATE_VERTEX);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(PosVertex, position));
    vib.addBuffer(1, sizeof(Instance), SDL_GPU_VERTEXINPUTRATE_INSTANCE);
    vib.addAttribute(1, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(Instance, offset));
    vib.addAttribute(1, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Instance, color));
    vib.addAttribute(1, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT, offsetof(Instance, scale));

    auto pipeline = app.loadPipeline("examples/triangle", vib, app.swapchainFormat(),
                                     "VSMain", "PSMain", 1, 0);

    codotaku::Camera2D camera;

    app.run([&](const codotaku::FrameContext &ctx) {
        auto &in = app.input();
        if (in.mouseWheel != 0.0f)
            camera.setZoom(camera.zoom() * std::pow(1.1f, in.mouseWheel));
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

        SDL_GPUBufferBinding bindings[] = {
            geom.vertexBinding(),
            {.buffer = instBuf.handle(), .offset = 0}
        };

        pass.bindVertexBuffers(0, std::span(bindings));
        geom.bindIndexBuffer(pass);

        pass.drawIndexed(
            geom.indexCount(),
            instances.size()
        );
    });
}
