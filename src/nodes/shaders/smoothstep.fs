#version 330

in vec2 fragTexCoord; 
in vec4 fragColor;

uniform sampler2D inputTexture0; // "input"
uniform sampler2D inputTexture1; // "edge0"
uniform sampler2D inputTexture2; // "edge1"

out vec4 finalColor;

void main()
{
    vec3 x = texture(inputTexture0, fragTexCoord).rgb;
    vec3 edge0 = texture(inputTexture1, fragTexCoord).rgb;
    vec3 edge1 = texture(inputTexture2, fragTexCoord).rgb;

    finalColor = vec4(smoothstep(edge0, edge1, x), 1.0);
}