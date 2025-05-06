// Template for terrain shaders for blending up to 7 textures and using up to 12 pointlights.

#if defined(__INTELLISENSE__)
    #define PS
    #define NUM_PT_LIGHTS 24
#endif

#define REVERSED_DEPTH

#ifndef TEX_COUNT
    #define TEX_COUNT 1
#endif

#if defined(NUM_PT_LIGHTS) && (NUM_PT_LIGHTS > 24)
    #define NUM_PT_LIGHTS 24
#endif

#include "includes/Terrain.hlsli"

struct VS_INPUT {
    float4 position : POSITION;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 normal : NORMAL;
    float4 uv : TEXCOORD0;
    float4 vertex_color : COLOR0;
    float4 blend_0 : TEXCOORD1;
    float4 blend_1 : TEXCOORD2;
};

struct VS_OUTPUT {
    float4 blend_0 : COLOR0;
    float4 blend_1 : COLOR1;
    float4 sPosition : POSITION;
    float2 uv : TEXCOORD0;
    float3 vertex_color : TEXCOORD1;
    float3 lPosition : TEXCOORD2;
    float3 tangent : TEXCOORD3;
    float3 binormal : TEXCOORD4;
    float3 normal : TEXCOORD5;
    float4 fogPosition : TEXCOORD6;
    float3 viewPosition : TEXCOORD7;
};

#ifdef VS

row_major float4x4 ModelViewProj : register(c0);

float4 FogParam : register(c14);
float3 FogColor : register(c15);
float4 EyePosition : register(c16);

VS_OUTPUT main(VS_INPUT IN) {
    VS_OUTPUT OUT;

    float4 posPS = mul(ModelViewProj, IN.position.xyzw);

    OUT.blend_0 = IN.blend_0;
    OUT.blend_1 = IN.blend_1;
    
    OUT.sPosition.xyzw = posPS;
    OUT.uv.xy = IN.uv.xy;
    OUT.vertex_color.xyz = clamp(IN.vertex_color.rgb, 0.0f, 1.0f);
    OUT.lPosition.xyz = IN.position.xyz;
    OUT.tangent.xyz = IN.tangent.xyz;
    OUT.binormal.xyz = IN.binormal.xyz;
    OUT.normal.xyz = IN.normal.xyz;
    OUT.fogPosition.xyzw = posPS;
    OUT.viewPosition.xyz = EyePosition.xyz;

    return OUT;
};

#endif  // Vertex shader.

struct PS_INPUT
{
    float2 uv : TEXCOORD0;
    float3 vertex_color : TEXCOORD1_centroid;
    float3 lPosition : TEXCOORD2_centroid;
    float3 tangent : TEXCOORD3_centroid;
    float3 binormal : TEXCOORD4_centroid;
    float3 normal : TEXCOORD5_centroid;
    float4 blend_0 : COLOR0;
    float4 blend_1 : COLOR1;
    float4 fogPosition : TEXCOORD6_centroid;
    float3 viewPosition : TEXCOORD7_centroid;
};

struct PS_OUTPUT {
    float4 color_0 : COLOR0;
};

#ifdef PS

sampler2D BaseMap[7]:register(s0);
sampler2D NormalMap[7]:register(s7);

float4 AmbientColor : register(c1);
float4 PSLightColor[8] : register(c3);
float4 PSLightDir : register(c18);
float4 PSLightPosition[7] : register(c19);

float4 LandSpec[2] : register(c32);
float4 LandHeight[2] : register(c34);
float4 FogParam : register(c36);
float4 FogColor : register(c37);

#ifdef NUM_PT_LIGHTS
float4 PointLightColor[NUM_PT_LIGHTS] : register(c39);
float4 PointLightPosition[NUM_PT_LIGHTS] : register(c63);
float  PointLightCount : register(c88);
#endif

PS_OUTPUT main(PS_INPUT IN) {
    PS_OUTPUT OUT;
    
    int texCount = TEX_COUNT;
    float3 tangent = normalize(IN.tangent.xyz);
    float3 binormal = normalize(IN.binormal.xyz);
    float3 normal = normalize(IN.normal.xyz);
    float3x3 tbn = float3x3(tangent, binormal, normal);
    float3 eyeDir = normalize(mul(tbn, IN.viewPosition.xyz - IN.lPosition.xyz));
    float dist = length(IN.viewPosition.xyz - IN.lPosition.xyz);
    
    float blends[7] = { IN.blend_0.x, IN.blend_0.y, IN.blend_0.z, IN.blend_0.w, IN.blend_1.x, IN.blend_1.y, IN.blend_1.z };
    float spec[7] = { LandSpec[0].x, LandSpec[0].y, LandSpec[0].z, LandSpec[0].w, LandSpec[1].x, LandSpec[1].y, LandSpec[1].z };

    float gloss = 0.0f;
    float specExponent = 0.0f;
    float3 baseColor = blendDiffuseMaps(IN.vertex_color, IN.uv.xy, texCount, BaseMap, blends);
    float3 combinedNormal = blendNormalMaps(IN.uv.xy, texCount, NormalMap, blends, spec, gloss, specExponent);

    float3 lightTS = mul(tbn, PSLightDir.xyz);
    
    float3 lighting = getDirectionalLighting(lightTS, PSLightColor[0].rgb, eyeDir, combinedNormal, baseColor, gloss, specExponent);
    lighting += getAmbientLighting(baseColor, AmbientColor.rgb);
    
    #if defined(NUM_PT_LIGHTS)
        [loop] for (int i = 0; i < PointLightCount; i++) {
            float3 pointlightDir = PointLightPosition[i].xyz - IN.lPosition.xyz;
            float att = attenuation(pointlightDir, PointLightPosition[i].w);
        
            [branch]
            if (att > 0.001) {
                pointlightDir = mul(tbn, pointlightDir);
                lighting += getPointLightingAtt(pointlightDir, att, PointLightColor[i].rgb, eyeDir, combinedNormal, baseColor, gloss, specExponent);
            }
        }
    #endif
    
    // Per pixel fog.
    float4 fog;
    float3 fogPos = IN.fogPosition.xyz;
    #ifdef REVERSED_DEPTH
        fogPos.z = IN.fogPosition.w - IN.fogPosition.z;
    #endif
    float fogStrength = 1 - saturate((FogParam.x - length(fogPos)) / FogParam.y);
    fog.rgb = FogColor.rgb;
    fog.a = pow(fogStrength, FogParam.z);
    
    float3 finalColor = lighting;
    finalColor = lerp(finalColor, fog.rgb, fog.a); // Apply fog.

    OUT.color_0.a = 1;
    OUT.color_0.rgb = finalColor;

    return OUT;
};

#endif  // Pixel shader.
