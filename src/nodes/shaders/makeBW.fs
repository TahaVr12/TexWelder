#version 330

in vec2 fragTexCoord; 
in vec4 fragColor;

uniform sampler2D inputTexture0; // "in"

out vec4 finalColor;

void main()
{
    vec4 inp = texture(inputTexture0, fragTexCoord);
    
    float gray = dot(inp.rgb, vec3(0.3, 0.587, 0.114));

    finalColor = vec4(vec3(gray), 1.0);
}