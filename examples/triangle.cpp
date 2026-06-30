#include <codotaku/camera2d.h>
#include <codotaku/runtime.h>
#include <codotaku/geometry.h>
#include <codotaku/polygon.h>

#include <cmath>
#include <glm/vec3.hpp>
#include <random>
#include <vector>

struct PosVertex { glm::vec2 position; };
struct Instance { glm::vec2 offset; glm::vec3 color; float scale; };
struct Shape { Uint32 firstIndex; Uint32 indexCount; };

int main() {
    codotaku::Runtime app({.title = "Triangles", .windowSize = {1024, 768}});

    // Directly populate single vertex/index buffer via callbacks
    std::vector<PosVertex> verts;
    std::vector<uint32_t> indices;

    auto vertexOffset = 0u;
    auto pushShape = [&](uint32_t sides, float size, float rotation) {
        auto iFirst = static_cast<Uint32>(indices.size());
        codotaku::generatePolygon(sides, size, rotation,
            [&](glm::vec2 p, auto) { verts.push_back({p}); },
            [&](uint32_t i) { indices.push_back(i + vertexOffset); });
        vertexOffset = static_cast<uint32_t>(verts.size());
        return Shape{ iFirst, static_cast<Uint32>(indices.size() - iFirst) };
    };

    auto triShape    = pushShape(3, 1.0f, 0);
    auto sqShape     = pushShape(4, 1.0f, glm::radians(45.0f));
    auto circleShape = pushShape(32, 1.0f, 0);

    // Generate instances into one buffer
    std::mt19937 rng(42);
    std::uniform_real_distribution pd(-400.0f, 400.0f);
    std::uniform_real_distribution cd(0.0f, 1.0f);
    std::uniform_real_distribution sd(20.0f, 80.0f);

    std::vector<Instance> instances;
    auto pushInstances = [&](int n) {
        auto first = static_cast<Uint32>(instances.size());
        for (int i = 0; i < n; ++i)
            instances.push_back({{pd(rng), pd(rng)}, {cd(rng), cd(rng), cd(rng)}, sd(rng)});
        return first;
    };

    auto triFirst    = pushInstances(10);
    auto sqFirst     = pushInstances(10);
    auto circleFirst = pushInstances(10);

    // One buffer for all static geometry, one for all instance data
    auto belt = app.createBelt();
    auto geom = app.createGeometry<PosVertex>(belt, verts, indices);
    auto instBuf = app.createBuffer(SDL_GPU_BUFFERUSAGE_VERTEX, std::span(instances), belt);
    belt.flush();

    codotaku::VertexInputBuilder vib;
    vib.addBuffer(codotaku::VertexSlot::Vertex, sizeof(PosVertex), SDL_GPU_VERTEXINPUTRATE_VERTEX);
    vib.addAttribute(SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(PosVertex, position));
    vib.addBuffer(codotaku::VertexSlot::Instance, sizeof(Instance), SDL_GPU_VERTEXINPUTRATE_INSTANCE);
    vib.addAttribute(SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(Instance, offset));
    vib.addAttribute(SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(Instance, color));
    vib.addAttribute(SDL_GPU_VERTEXELEMENTFORMAT_FLOAT, offsetof(Instance, scale));

    auto pipeline = app.loadPipeline("examples/triangle", vib, app.swapchainFormat(),
                                     "VSMain", "PSMain", 1, 0);

    codotaku::Camera2D camera;

    auto drawShape = [&](auto &pass, const Shape &shape, Uint32 firstInstance, Uint32 instanceCount) {
        SDL_GPUBufferBinding bindings[] = {
            geom.vertexBinding(),
            {.buffer = instBuf.handle(), .offset = 0}
        };
        pass.bindVertexBuffers(0, std::span(bindings));
        geom.bindIndexBuffer(pass);
        pass.drawIndexed(shape.indexCount, instanceCount, shape.firstIndex, 0, firstInstance);
    };

    app.run([&](const codotaku::FrameContext &ctx) {
        auto &in = app.input();
        if (in.mouseWheel != 0.0f) {
            auto oldZoom = camera.zoom();
            auto newZoom = oldZoom * std::pow(1.1f, in.mouseWheel);
            camera.setZoom(newZoom);

            auto pos = camera.position();
            auto cx = static_cast<float>(ctx.swapchainSize.x) * 0.5f;
            auto cy = static_cast<float>(ctx.swapchainSize.y) * 0.5f;
            auto ratio = newZoom / oldZoom;
            pos.x = pos.x * ratio + (static_cast<float>(in.mouseX) - cx) * (ratio - 1.0f);
            pos.y = pos.y * ratio + (cy - static_cast<float>(in.mouseY)) * (ratio - 1.0f);
            camera.setPosition(pos);
        }
        if (in.mouseLeft) {
            auto pos = camera.position();
            pos.x -= in.mouseDX;
            pos.y += in.mouseDY;
            camera.setPosition(pos);
        }

        auto pass = ctx.beginRenderPass();
        auto vp = camera.viewProjection(ctx.swapchainSize);
        pass.pushVertexUniform(0, vp);
        pass.bindPipeline(pipeline);

        drawShape(pass, triShape,    triFirst,    10);
        drawShape(pass, sqShape,     sqFirst,     10);
        drawShape(pass, circleShape, circleFirst, 10);
    });
}
