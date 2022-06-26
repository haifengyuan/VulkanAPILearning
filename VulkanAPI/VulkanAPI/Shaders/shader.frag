#version 450

layout(location=0) in vec3 fragcolor;	//Interpolated color from vertex (location must match)

layout(location=0) out vec4 outcolor;	//Final output color (must also have location)

void main(){
	outcolor=vec4(fragcolor,1.0);
}