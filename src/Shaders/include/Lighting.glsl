vec3 ambient = vec3(0.05);

vec3 BlinnPhong(vec3 fragPos, vec3 fragNormal, vec3 eyePos, float specularity)
{
    vec3 viewDir = normalize(eyePos - fragPos);
    vec3 halfDir = normalize(ubo.sunDir + viewDir);

    float diff = max(dot(fragNormal, ubo.sunDir), 0.0);
    vec3 diffuse = diff * ubo.sunColor;

    float spec = pow(max(dot(fragNormal, halfDir), 0.0), 16.0);
    vec3 specular = specularity * spec * ubo.sunColor;

    return ambient + diffuse + specular;
}
