vec3 ambient = vec3(0.05);

vec3 sunColor = vec3(0.5, 0.5, 0.5);
vec3 sunDir = normalize(vec3(0.4, 1.0, 0.6));

vec3 BlinnPhong(vec3 fragPos, vec3 fragNormal, vec3 eyePos, float specularity)
{
    vec3 viewDir = normalize(eyePos - fragPos);
    vec3 halfDir = normalize(sunDir + viewDir);

    float diff = max(dot(fragNormal, sunDir), 0.0);
    vec3 diffuse = diff * sunColor;

    float spec = pow(max(dot(fragNormal, halfDir), 0.0), 16.0);
    vec3 specular = specularity * spec * sunColor;

    return ambient + diffuse + specular;
}
