#version 330

uniform float Value;

out vec4 finalColor;

void main()
{
    finalColor = vec4(vec3(Value), 1.0);
}