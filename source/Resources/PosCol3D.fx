float4x4 gWorldViewProj: WorldViewProjection;
Texture2D gDiffuseMap: DiffuseMap;
Texture2D gNormalMap: NormalMap;
Texture2D gGlossinessMap: GlossinessMap;
Texture2D gSpecularMap: SpecularMap;
float3 gLightDir: LightDirection;
float4x4 gViewInverse: VIEWINVERSE;
float4x4 gWorld: WORLD;

// Constants
float PI = float(3.14159);
float INTENSITY = float(7.0);
float SHININESS = float(25.0);
float3 AMBIENT = float3(0.025, 0.025, 0.025);

struct VS_INPUT
{
	float3 Position: POSITION;
	float2 Uv: TEXCOORD;
	float3 Normal: NORMAL;
	float3 Tangent: TANGENT;
};

struct VS_OUTPUT
{
	float4 Position: SV_POSITION;
	float4 WorldPosition: WORLDPOSITION;
	float2 Uv: TEXCOORD0;
	float3 Normal: NORMAL;
	float3 Tangent: TANGENT;
};

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;// or Mirror, Clamp, Border
	AddressV = Wrap;
};

/// Vertex shader
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	output.Uv = input.Uv;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorld);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld);
	output.WorldPosition = mul(input.Position, gWorld);

	return output;
}

float CalculatePhong(float3 specular, float phongExponent, float3 lightDirection, float3 viewDirection, float3 normal)
{
	float3 reflected = reflect(normal, lightDirection);
	float angle = max(0, dot(reflected, viewDirection));
	float phongValue = specular * pow(angle, phongExponent);

	return phongValue;
}

float3 CalculateLambert(float kd, float3 color)
{
	return mul(color, kd / PI);
}

// Pixel shader
float4 PS(VS_OUTPUT input): SV_TARGET
{
	float3 binormal = normalize(cross(input.Normal, input.Tangent));
	float3x3 tangentSpaceAxis = float3x3(normalize(input.Tangent), binormal, normalize(input.Normal));
	float3 normalMapSample = gNormalMap.Sample(samPoint, input.Uv).xyz;
	float3 mappedNormal = 2.f * normalMapSample - 1.f;
	float3 tangentSpaceNormal = normalize(mul(mappedNormal, tangentSpaceAxis));

	float observedArea = dot(tangentSpaceNormal, -gLightDir);
	//observedArea = saturate(observedArea);

	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);

	float3 specularMapSample = gSpecularMap.Sample(samPoint, input.Uv).xyz;
	float3 glossinessMapSample = gGlossinessMap.Sample(samPoint, input.Uv).xyz;
	float3 diffuseMapSample = gDiffuseMap.Sample(samPoint, input.Uv).xyz;

	float phongValue = CalculatePhong(specularMapSample, glossinessMapSample.x * SHININESS, gLightDir, viewDirection, tangentSpaceNormal);
	float3 diffuse = CalculateLambert(1.f, diffuseMapSample);

	// ((diffuse * int) + phong + ambient) * obser
	//float3 finalColor = INTENSITY * (AMBIENT + diffuse + phongValue) * observedArea;
	float3 finalColor = ((diffuse * INTENSITY) + phongValue + AMBIENT) * observedArea;

	return float4(finalColor, 1);
}

// Technique
technique11 DefaultTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}