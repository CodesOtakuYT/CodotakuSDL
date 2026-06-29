struct VSOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

VSOutput VSMain(float2 position : POSITION, float3 color : COLOR)
{
    VSOutput output;
    output.position = float4(position, 0.0f, 1.0f);
    output.color = color;
    return output;
}

float4 PSMain(float3 color : COLOR) : SV_Target
{
    return float4(color, 1.0f);
}
