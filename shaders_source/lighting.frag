#version 330 core
layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragWorldPos;

uniform sampler2D baseTexture;

// material: x = ka, y = kd, z = ks, w = shininess
uniform vec4 material;

uniform vec3 ambientColor;

// Directional light
uniform vec3 directionalLight;   // direction (world space)
uniform vec3 directionalColor;

// Camera
uniform vec3 cameraPos;

// -------- Point light --------
struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

uniform PointLight pointLight;

// -------- Point light function --------
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.w);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 /
        (light.constant +
         light.linear * distance +
         light.quadratic * distance * distance);

    vec3 ambient  = material.x * light.color;
    vec3 diffuse  = material.y * diff * light.color;
    vec3 specular = material.z * spec * light.color;

    return (ambient + diffuse + specular) * attenuation;
}

// -------- Main --------
void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(cameraPos - FragWorldPos);

    // ----- Directional light -----
    vec3 ambientIntensity = material.x * ambientColor;

    vec3 lightDir = normalize(-directionalLight);
    float lambertFactor = max(dot(norm, lightDir), 0.0);
    vec3 diffuseIntensity = material.y * directionalColor * lambertFactor;

    vec3 specularIntensity = vec3(0.0);
    if (lambertFactor > 0.0) {
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = max(dot(viewDir, reflectDir), 0.0);
        specularIntensity = material.z * directionalColor * pow(spec, material.w);
    }

    // Combine directional lighting
    vec3 lightIntensity =
        ambientIntensity +
        diffuseIntensity +
        specularIntensity;

    // ----- Point light (fairy glow) -----
    lightIntensity += CalcPointLight(pointLight, norm, FragWorldPos, viewDir);

    // Texture modulation LAST
    FragColor = vec4(lightIntensity, 1.0) * texture(baseTexture, TexCoord);
}
