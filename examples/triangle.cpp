#include <codotaku/runtime.h>

int main()
{
    codotaku::Runtime app({ .title = "Triangle",
                            .windowSize = { 640, 480 } });

    app.run([](const codotaku::FrameContext &ctx) {
        auto pass = ctx.beginRenderPass({ 0.3f, 0.4f, 0.5f, 1.0f });
        pass.setViewport(0.0f, 0.0f, 640.0f, 480.0f);
    });
}
