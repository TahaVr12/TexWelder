#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D inputTexture0; // "t"
uniform sampler2D inputTexture1; // "a"
uniform sampler2D inputTexture2; // "b"

out vec4 finalColor;

void main()
{
    vec4 t = texture(inputTexture0, fragTexCoord);
    vec4 a = texture(inputTexture1, fragTexCoord);
    vec4 b = texture(inputTexture2, fragTexCoord);
    finalColor = vec4(mix(a.rgb, b.rgb, t.rgb), 1.0);
}