#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

out vec4 FragColor;

void main() {
    vec3 norm = normalize(Normal);
    // Directional light coming from above-left
    vec3 lightDir = normalize(vec3(-0.5, -1.0, -0.3));
    float ambient = 0.3;
    float diff = max(dot(norm, -lightDir), 0.0);
    float diffuse = 0.7 * diff;

    vec3 lit = Color * (ambient + diffuse);
    FragColor = vec4(lit, 1.0);
}

