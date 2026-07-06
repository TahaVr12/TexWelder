#version 330

in vec2 fragTexCoord; 
in vec4 fragColor;

uniform sampler2D inputTexture0; // "a"
uniform sampler2D inputTexture1; // "b"

out vec4 finalColor;

void main()
{
    vec4 a = texture(inputTexture0, fragTexCoord);
    vec4 b = texture(inputTexture1, fragTexCoord);
    finalColor = vec4((a.rgb + b.rgb), 1.0);
}