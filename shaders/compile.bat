set FXC="C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe"

if not exist package\Shaders\Loose mkdir package\Shaders\Loose

set SHADER_FILE=shaders\TerrainTemplate.hlsl
%FXC% /T vs_3_0 /E main /DVS /Fo "package\shaders\loose\SLS2100.vso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=1 /Fo "package\shaders\loose\SLS2092.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=1 /DNUM_PT_LIGHTS=6 /Fo "package\shaders\loose\SLS2094.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=1 /DNUM_PT_LIGHTS=12 /Fo "package\shaders\loose\SLS2096.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=1 /DNUM_PT_LIGHTS=24 /Fo "package\shaders\loose\SLS2098.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=2 /Fo "package\shaders\loose\SLS2100.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=2 /DNUM_PT_LIGHTS=6 /Fo "package\shaders\loose\SLS2102.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=2 /DNUM_PT_LIGHTS=12 /Fo "package\shaders\loose\SLS2104.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=2 /DNUM_PT_LIGHTS=24 /Fo "package\shaders\loose\SLS2106.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=3 /Fo "package\shaders\loose\SLS2108.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=3 /DNUM_PT_LIGHTS=6 /Fo "package\shaders\loose\SLS2110.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=3 /DNUM_PT_LIGHTS=12 /Fo "package\shaders\loose\SLS2112.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=3 /DNUM_PT_LIGHTS=24 /Fo "package\shaders\loose\SLS2114.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=4 /Fo "package\shaders\loose\SLS2116.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=4 /DNUM_PT_LIGHTS=6 /Fo "package\shaders\loose\SLS2118.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=4 /DNUM_PT_LIGHTS=12 /Fo "package\shaders\loose\SLS2120.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=4 /DNUM_PT_LIGHTS=24 /Fo "package\shaders\loose\SLS2122.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=5 /Fo "package\shaders\loose\SLS2124.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=5 /DNUM_PT_LIGHTS=6 /Fo "package\shaders\loose\SLS2126.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=5 /DNUM_PT_LIGHTS=12 /Fo "package\shaders\loose\SLS2128.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=5 /DNUM_PT_LIGHTS=24 /Fo "package\shaders\loose\SLS2130.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=6 /Fo "package\shaders\loose\SLS2132.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=6 /DNUM_PT_LIGHTS=6 /Fo "package\shaders\loose\SLS2134.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=6 /DNUM_PT_LIGHTS=12 /Fo "package\shaders\loose\SLS2136.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=6 /DNUM_PT_LIGHTS=24 /Fo "package\shaders\loose\SLS2138.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=7 /Fo "package\shaders\loose\SLS2140.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=7 /DNUM_PT_LIGHTS=6 /Fo "package\shaders\loose\SLS2142.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=7 /DNUM_PT_LIGHTS=12 /Fo "package\shaders\loose\SLS2144.pso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /DTEX_COUNT=7 /DNUM_PT_LIGHTS=24 /Fo "package\shaders\loose\SLS2146.pso" "%SHADER_FILE%"

set SHADER_FILE=shaders\TerrainLODTemplate.hlsl
%FXC% /T vs_3_0 /E main /DVS /Fo "package\shaders\loose\SLS2002.vso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /Fo "package\shaders\loose\SLS2003.pso" "%SHADER_FILE%"

set SHADER_FILE=shaders\TerrainFadeTemplate.hlsl
%FXC% /T vs_3_0 /E main /DVS /Fo "package\shaders\loose\SLS2080.vso" "%SHADER_FILE%"
%FXC% /T ps_3_0 /E main /DPS /Fo "package\shaders\loose\SLS2082.pso" "%SHADER_FILE%"
