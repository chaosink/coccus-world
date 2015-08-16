#version 430 core

in VertexData {
	vec4 color;
	vec2 uv;
} vertexIn;
out vec4 color;

uniform sampler2D texture;
uniform int object;

float PI = 3.14159265358979323846;

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform vec4      iMouse;                // mouse pixel coords
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)
uniform sampler2D iChannel0;             // input channel. XX = 2D/Cube
uniform sampler2D iChannel1;             // input channel. XX = 2D/Cube
uniform sampler2D iChannel2;             // input channel. XX = 2D/Cube
uniform sampler2D iChannel3;             // input channel. XX = 2D/Cube
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform float     iChannelTime[4];       // channel playback time (in sec)



//Ether by nimitz (twitter: @stormoid)

#define t iGlobalTime
mat2 m(float a){float c=cos(a), s=sin(a);return mat2(c,-s,s,c);}
float map(vec3 p){
    p.xz*= m(t*0.4);p.xy*= m(t*0.3);
    vec3 q = p*2.+t*1.;
    return length(p+vec3(sin(t*0.7)))*log(length(p)+1.) + sin(q.x+sin(q.z+sin(q.y)))*0.5 - 1.;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ){	
	vec2 p = fragCoord.xy/iResolution.y - vec2(.9,.5);
    vec3 cl = vec3(0.);
    float d = 2.5;
    for(int i=0; i<=5; i++)	{
		vec3 p = vec3(0,0,5.) + normalize(vec3(p, -1.))*d;
        float rz = map(p);
		float f =  clamp((rz - map(p+.1))*0.5, -.1, 1. );
        vec3 l = vec3(0.1,0.3,.4) + vec3(5., 2.5, 3.)*f;
        cl = cl*l + (1.-smoothstep(0., 2.5, rz))*.7*l;
		d += min(rz, 1.);
	}
    fragColor = vec4(cl, 1.);
}



void main() {
	switch(object) {
		case 0:
			color = vertexIn.color;
			break;
		case 1:
			color = vec4(texture2D(texture, vertexIn.uv).rgb, texture2D(texture, vertexIn.uv).a * 0.7);
			break;
		case 2:
			vec2 uv = vertexIn.uv * 2 - 1;
			float uv_l = length(uv);
			uv_l = pow(uv_l, sin(iGlobalTime + vertexIn.color.b * PI * 2) * 0.1 + 0.1 + 0.8);
			uv = normalize(uv) * uv_l;
			uv = uv / 2 + 0.5;
			vec2 uv_d = vec2((vertexIn.uv * 2 - 1) * (vertexIn.uv * 2 - 1)) / 2 - 0.5;
			vec3 color_rgb = texture2D(texture, uv).rgb * (sin(iGlobalTime + vertexIn.color.b * PI * 2) * (1 - vertexIn.color.a) / 2 + (1 + vertexIn.color.a) / 2);
			if     (vertexIn.color.r == 1) color = vec4(color_rgb.brg, 1.0);
			else if(vertexIn.color.g == 1) color = vec4(color_rgb.rbg, 1.0);
			else                           color = vec4(color_rgb.rgb, 1.0);
			break;
		case 3:
		//	mainImage(color, gl_FragCoord.xy);
			color = vec4(texture2D(texture, vertexIn.uv).rgb, 0.6);
			break;
		case 4:
		//	mainImage(color, gl_FragCoord.xy);
			color = vec4(texture2D(texture, vertexIn.uv).rgb, 0.3);
			break;
	}
}
