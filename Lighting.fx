cbuffer cbData
{
	float4x4 World; 
	float4x4 View; 
	float4x4 Projection;

	float4 gDiffuseMtrl; 
	float4 gDiffuseLight; 
	
	float4 gAmbientMtrl;
	float4 gAmbientLight;

	float4 gSpecularMtrl;
	float4 gSpecularLight;
	
	float3 gEyePosW;
	float gSpecularPower;

	float3 gLightVecW;
};

struct VS_IN
{
	float4 posL   : POSITION;
	float3 normalL : NORMAL;
};

struct VS_OUT
{
	float4 Pos    : SV_POSITION;
	float3 Norm	  : NORMAL;
	float3 PosW   : POSITION;
};

VS_OUT VS(VS_IN vIn)
{
	VS_OUT output = (VS_OUT)0;

	output.Pos = mul( vIn.posL, World ); 
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	// Convert from local to world normal
	float3 normalW = mul(float4(vIn.posL.rgb, 0.0f), World).xyz;

	normalW = normalize(normalW);

	output.Norm = normalW;

	output.PosW = mul(vIn.posL, World).xyz;

	return output;
	/*
	// Compute Colour
	// Compute the reflection vector
	float3 r = reflect(-gLightVecW, normalW);
	// Determine how much (if any) specular light makes it into the eye
	float t = pow(max(dot(r, toEye), 0.0f), gSpecularPower);
	// Determine the diffuse light intensity that strikes the vertex
	float s = max(dot(gLightVecW, normalW), 0.0f);
	// Compute the ambient, diffuse, and specular terms seperately
	float3 spec = t*(gSpecularMtrl*gSpecularLight).rgb;
	float3 diffuse = s*(gDiffuseMtrl*gDiffuseLight).rgb;
	float3 ambient = gAmbientMtrl * gAmbientLight;
	// Sum all the terms together and copy over the diffuse alpha.
	output.Col.rgb = ambient + diffuse + spec;
	//output.Col.rgb = diffuse + ambient;
	output.Col.a = gDiffuseMtrl.a;

	//output.Col.rgb = s*(gDiffuseMtrl*gDiffuseLight).rgb; 
	//output.Col.a = gDiffuseMtrl.a;
 */
	
}

float4 PS(VS_OUT pIn) : SV_Target
{
	//compute the vector from the vertex to the eye position
	//output.Pos is currently the position in world space
	float3 toEye = normalize(gEyePosW - pIn.PosW.xyz);
	float3 normalW = normalize(pIn.Norm);

	float3 lightVecNorm = normalize(gLightVecW);

	// Compute Colour
	// Compute the reflection vector.
	float3 r = reflect(-lightVecNorm, normalW);
	// Determine how much (if any) specular light makes it
	// into the eye.
	float t = pow(max(dot(r, toEye), 0.0f), gSpecularPower);
	// Determine the diffuse light intensity that strikes the vertex.
	float s = max(dot(lightVecNorm, normalW), 0.0f);

	if (s <= 0.0f)
	{
		t = 0.0f;
	}

	// Compute the ambient, diffuse, and specular terms separately.
	float3 spec = t*(gSpecularMtrl*gSpecularLight).rgb;
	float3 diffuse = s*(gDiffuseMtrl*gDiffuseLight).rgb;
	float3 ambient = (gAmbientMtrl * gAmbientLight).rgb;

	// Sum all the terms together and copy over the diffuse alpha.
	float4 finalColour;

	finalColour.rgb = ambient + diffuse + spec;
	finalColour.a = gDiffuseMtrl.a;

	return finalColour;
	//return pIn.Col;
}

technique11 Render
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) ); SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
	}
}
