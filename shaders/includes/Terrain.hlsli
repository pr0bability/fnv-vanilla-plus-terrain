#define	expand(v)           (((v) - 0.5) / 0.5)
#define	shades(n, l)        saturate(dot(n, l))

float3 blendDiffuseMaps(float3 vertexColor, float2 uv, int texCount, sampler2D tex[7], float blends[7]) {
    float3 color = float3(0, 0, 0);
    [unroll] for (int i = 0; i < texCount; i++) {
        color += tex2D(tex[i], uv).xyz * blends[i];
    }

    return color * vertexColor;
}

float3 blendNormalMaps(float2 uv, int texCount, sampler2D tex[7], float blends[7], float spec[7], out float gloss, out float specExponent) {
    gloss = 0.0f;
    specExponent = 0.0f;

    float3 blendedNormal = float3(0, 0, 0);

    float4 normal;
    [unroll] for (int i = 0; i < texCount; i++){
        normal = tex2D(tex[i], uv);
        blendedNormal += normal.xyz * blends[i];
        gloss += normal.w * blends[i] * (spec[i] > 0 ? 1.0f : 0.0f);
        specExponent += spec[i] * blends[i];
    }

    gloss = saturate(gloss);
    return normalize(expand(blendedNormal));
}

float attenuation(float3 lightVector, float radius) {
    const float3 att = lightVector / radius;
    return 1 - shades(att, att);
}

float3 getDirectionalLighting(float3 lightDir, float3 lightColor, float3 viewDir, float3 normal, float3 albedo, float gloss, float glossPower) {
    lightDir = normalize(lightDir);
    viewDir = normalize(viewDir);
    float3 halfwayDir = normalize(lightDir + viewDir);
    
    float NdotL = shades(normal.xyz, lightDir.xyz);
    
    float specStrength = gloss * pow(abs(shades(normal.xyz, halfwayDir.xyz)), glossPower);
    float3 lighting = albedo.rgb * NdotL * lightColor.rgb;
    lighting += saturate(((0.2 >= NdotL ? (specStrength * saturate(NdotL + 0.5)) : specStrength) * lightColor.rgb));
    
    return lighting;
}

float3 getPointLighting(float3 lightDir, float radius, float3 lightColor, float3 viewDir, float3 normal, float3 albedo, float gloss, float glossPower) {
    float att = attenuation(lightDir, radius);
    
    lightDir = normalize(lightDir);
    viewDir = normalize(viewDir);
    float3 halfwayDir = normalize(lightDir + viewDir);
    
    float NdotL = shades(normal.xyz, lightDir.xyz);
    
    float specStrength = gloss * pow(abs(shades(normal.xyz, halfwayDir.xyz)), glossPower);
    float3 lighting = albedo.rgb * NdotL * lightColor.rgb * att;
    lighting += saturate(((0.2 >= NdotL ? (specStrength * saturate(NdotL + 0.5)) : specStrength) * lightColor.rgb) * att);
    
    return lighting;
}

float3 getPointLightingAtt(float3 lightDir, float att, float3 lightColor, float3 viewDir, float3 normal, float3 albedo, float gloss, float glossPower) {
    lightDir = normalize(lightDir);
    viewDir = normalize(viewDir);
    float3 halfwayDir = normalize(lightDir + viewDir);
    
    float NdotL = shades(normal.xyz, lightDir.xyz);
    
    float specStrength = gloss * pow(abs(shades(normal.xyz, halfwayDir.xyz)), glossPower);
    float3 lighting = albedo.rgb * NdotL * lightColor.rgb * att;
    lighting += saturate(((0.2 >= NdotL ? (specStrength * saturate(NdotL + 0.5)) : specStrength) * lightColor.rgb) * att);
    
    return lighting;
}

float3 getAmbientLighting(float3 albedo, float3 ambientColor) {
    return albedo.rgb * ambientColor.rgb;
}
