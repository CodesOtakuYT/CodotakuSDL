#include <cmath>
#include <glm/vec3.hpp>
#include <random>
#include <vector>

#include <codotaku/camera2d.h>
#include <codotaku/polygon.h>
#include <codotaku/runtime.h>

struct Vertex { glm::vec2 position; };

struct Instance { glm::vec2 offset; glm::vec3 color; float scale; };

int main() {
    codotaku::Runtime app({ .title = "Shapes", .windowSize = {1024, 768} });

    std::vector<Vertex> triVerts, sqVerts, circleVerts;

    auto triIdx = codotaku::generatePolygon(3, 1.0f, 0,
        [&](glm::vec2 p, auto) { triVerts.push_back({p}); });
    auto sqIdx = codotaku::generatePolygon(4, 1.0f, glm::radians(45.0f),
        [&](glm::vec2 p, auto) { sqVerts.push_back({p}); });
    auto circleIdx = codotaku::generatePolygon(32, 1.0f, 0,
        [&](glm::vec2 p, auto) { circleVerts.push_back({p}); });

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> pd(-400.0f, 400.0f);
    std::uniform_real_distribution<float> cd(0.0f, 1.0f);
    std::uniform_real_distribution<float> sd(20.0f, 80.0f);

    auto makeInstances = [&](int n) {
        std::vector<Instance> inst;
        for (int i = 0; i < n; ++i)
            inst.push_back({{pd(rng), pd(rng)}, {cd(rng), cd(rng), cd(rng)}, sd(rng)});
        return inst;
    };

    auto triInst = makeInstances(10);
    auto sqInst = makeInstances(10);
    auto circleInst = makeInstances(10);

    auto belt = app.createBelt();

    auto triVB = app.createBuffer(SDL_GPU_BUFFERUSAGE_VERTEX, triVerts.data(), triVerts.size() * sizeof(Vertex), belt);
    auto triIB = app.createBuffer(SDL_GPU_BUFFERUSAGE_INDEX, triIdx.data(), triIdx.size() * sizeof(uint32_t), belt);
    auto triIBuf = app.createBuffer(SDL_GPU_BUFFERUSAGE_VERTEX, triInst.data(), triInst.size() * sizeof(Instance), belt);

    auto sqVB = app.createBuffer(SDL_GPU_BUFFERUSAGE_VERTEX, sqVerts.data(), sqVerts.size() * sizeof(Vertex), belt);
    auto sqIB = app.createBuffer(SDL_GPU_BUFFERUSAGE_INDEX, sqIdx.data(), sqIdx.size() * sizeof(uint32_t), belt);
    auto sqIBuf = app.createBuffer(SDL_GPU_BUFFERUSAGE_VERTEX, sqInst.data(), sqInst.size() * sizeof(Instance), belt);

    auto circleVB = app.createBuffer(SDL_GPU_BUFFERUSAGE_VERTEX, circleVerts.data(), circleVerts.size() * sizeof(Vertex), belt);
    auto circleIB = app.createBuffer(SDL_GPU_BUFFERUSAGE_INDEX, circleIdx.data(), circleIdx.size() * sizeof(uint32_t), belt);
    auto circleIBuf = app.createBuffer(SDL_GPU_BUFFERUSAGE_VERTEX, circleInst.data(), circleInst.size() * sizeof(Instance), belt);

    belt.flush();

    codotaku::VertexInputBuilder vib;
    vib.addBuffer(0, sizeof(Vertex), SDL_GPU_VERTEXINPUTRATE_VERTEX);
    vib.addAttribute(0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(Vertex, position));
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

        auto drawShape = [&](const codotaku::Buffer &vb, const codotaku::Buffer &ib,
                             const codotaku::Buffer &instBuf, Uint32 idxCount, Uint32 instCount) {
            pass.bindVertexBuffer(vb, 0);
            pass.bindVertexBuffer(instBuf, 1);
            pass.bindIndexBuffer(ib);
            pass.drawIndexed(idxCount, instCount, 0, 0, 0);
        };

        drawShape(triVB, triIB, triIBuf, static_cast<Uint32>(triIdx.size()), static_cast<Uint32>(triInst.size()));
        drawShape(sqVB, sqIB, sqIBuf, static_cast<Uint32>(sqIdx.size()), static_cast<Uint32>(sqInst.size()));
        drawShape(circleVB, circleIB, circleIBuf, static_cast<Uint32>(circleIdx.size()), static_cast<Uint32>(circleInst.size()));
    });
}
