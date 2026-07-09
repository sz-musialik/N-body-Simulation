#version 330

// Input vertex attributes
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 source = texture2D(texture0, fragTexCoord) * colDiffuse*fragColor;
    vec4 sum = vec4(0.0);

    // Simple bloom filter (Box/Gaussian approximation)
    for (int x = -3; x < 3; x++) {
        for (int y = -3; y < 3; y++) {
            sum += texture2D(texture0, fragTexCoord + vec2(x, y)*0.002);
        }
    }

    finalColor = source + (sum*sum) * 0.001;
}
