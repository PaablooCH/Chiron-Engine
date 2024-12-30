// ------------- BUFFERS ----------------------

struct ViewProjection
{
    matrix view;
    matrix projection;
};
ConstantBuffer<ViewProjection> viewProjection : register(b0);

struct ModelAttributes
{
    matrix model;
    int uvCorrector;
    //int uvCorrectorSpecularMetallnes;
    //int uvCorrectorNormalMap;
    //int uvCorrectorOcclusion;
};
ConstantBuffer<ModelAttributes> modelAttributes : register(b1);

// ------------- VERTEX SHADER ----------------------

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

struct VS_OUTPUT
{
    float2 texCoord : TEXCOORD;
    float4 position : SV_Position;
};

VS_OUTPUT VSmain(VS_INPUT input)
{
    VS_OUTPUT OUT;
    float4 position = float4(input.position, 1.0f);
    matrix mvp = mul(modelAttributes.model, mul(viewProjection.view, viewProjection.projection));
    OUT.position = mul(position, mvp);
    OUT.texCoord = input.texCoord;

    return OUT;
}

// ------------- PIXEL SHADER ----------------------

Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

struct PS_INPUT
{
    float2 texCoord : TEXCOORD;
};

float4 PSmain(PS_INPUT input) : SV_Target
{
    float2 coord = input.texCoord;
    if (modelAttributes.uvCorrector == 1)
    {
        coord.y = 1.0f - coord.y;
    }
    return t1.Sample(s1, coord);
}