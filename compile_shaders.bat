@echo off
shadercross shaders/triangle.hlsl -s HLSL -d SPIRV -t vertex   -e VSMain -o examples/triangle.vert.spv  || exit /b
shadercross shaders/triangle.hlsl -s HLSL -d DXIL  -t vertex   -e VSMain -o examples/triangle.vert.dxil || exit /b
shadercross shaders/triangle.hlsl -s HLSL -d SPIRV -t fragment -e PSMain -o examples/triangle.frag.spv  || exit /b
shadercross shaders/triangle.hlsl -s HLSL -d DXIL  -t fragment -e PSMain -o examples/triangle.frag.dxil || exit /b
echo Shaders compiled successfully.
