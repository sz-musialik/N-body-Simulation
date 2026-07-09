#version 330

// Input parameters
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in mat4 instanceTransform; 

// Output parameters for fragment shader
out vec2 fragTexCoord;
out vec4 fragColor;

// Global Matrix
uniform mat4 mvp;
uniform vec4 u_color;

void main()
{
    fragTexCoord = vertexTexCoord;
	fragColor = u_color;

    // Vertex transform by matrix instance
    gl_Position = mvp * instanceTransform * vec4(vertexPosition, 1.0);
}
