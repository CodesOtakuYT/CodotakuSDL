#include <codotaku/camera2d.h>
#include <codotaku/runtime.h>
#include <codotaku/geometry.h>
#include <codotaku/polygon.h>

#include <cmath>
#include <random>
#include <vector>

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
};

int main() {
    codotaku::Runtime app({ .title = "Triangles", .windowSize = {1024, 768} });

    std::vector<Vertex> triPoly, sqPoly, circlePoly;
    auto triIdx = codotaku::generatePolygon(3, 1.0f, 0,
        [&](glm::vec2 p, auto) { triPoly.push_back({p, {}}); });
    auto sqIdx = codotaku::generatePolygon(4, 1.0f, glm::radians(45.0f),
        [&](glm::vec2 p, auto) { sqPoly.push_back({p, {}}); });
    auto circleIdx = codotaku::generatePolygon(32, 1.0f, 0,
        [&](glm::vec2 p, auto) { circlePoly.push_back({p, {}}); });

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> pd(-400.0f, 400.0f);
    std::uniform_real_distribution<float> cd(0.0f, 1.0f);
    std::uniform_real_distribution<float> sd(20.0f, 80.0f);

    struct Instance { glm::vec2 offset; glm::vec3 color; float scale; };

    auto makeInstances = [&](int n) {
        std::vector<Instance> inst;
        for (int i = 0; i < n; ++i)
            inst.push_back({{pd(rng), pd(rng)}, {cd(rng), cd(rng), cd(rng)}, sd(rng)});
        return inst;
    };

    auto bake = [](const std::vector<Vertex>& poly, const std::vector<uint32_t>& polyIdx,
                   const std::vector<Instance>& instances) {
        std::vector<Vertex> verts;
        std::vector<uint32_t> idx;
        for (auto& inst : instances) {
            auto base = static_cast<uint32_t>(verts.size());
            for (auto& v : poly)
                verts.push_back({v.position * inst.scale + inst.offset, inst.color});
            for (auto i : polyIdx)
                idx.push_back(i + base);
        }
        return std::pair{verts, idx};
    };

    auto [triVerts, triIdxFinal] = bake(triPoly, triIdx, makeInstances(10));
    auto [sqVerts, sqIdxFinal]   = bake(sqPoly, sqIdx, makeInstances(10));
    auto [circVerts, circIdxFinal] = bake(circlePoly, circleIdx, makeInstances(10));

    std::vector<Vertex> allVerts;
    std::vector<uint32_t> allIdx;

    auto append = [&](auto& verts, auto& idx) {
        auto base = static_cast<uint32_t>(allVerts.size());
        for (auto& v : verts) allVerts.push_back(v);
        for (auto i : idx) allIdx.push_back(i + base);
    };
    append(triVerts, triIdxFinal);
    append(sqVerts, sqIdxFinal);
    append(circVerts, circIdxFinal);

    auto belt = app.createBelt();
    auto geom = app.createGeometry<Vertex>(belt, allVerts, allIdx);
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
        geom.drawIndexed(pass);
    });
}
