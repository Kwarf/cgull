#version 330

uniform vec2 resolution;
uniform float time;
uniform vec3 cameraPosition;
uniform vec3 cameraTarget;

out vec4 color;

const float MAX_DIST = 100.0;
const int MAX_STEPS = 128;

float fSphere(vec3 p, float r) {
    return length(p) - r;
}

vec2 SDF(vec3 pos) {
    float d = fSphere(pos, 1.0);
    return vec2(d, 1.0);
}

vec3 calculateNormal(vec3 pos) {
    float c = SDF(pos).x;
    vec2 eps_zero = vec2(0.001, 0.0);
    return normalize(vec3(SDF(pos + eps_zero.xyy).x, SDF(pos + eps_zero.yxy).x, SDF(pos + eps_zero.yyx).x) - c);
}

struct IntersectionResult {
    float distance;
    float material;
    int steps;
};

IntersectionResult castRay(vec3 origin, vec3 direction) {
    float dist = 0.0;

    IntersectionResult result;
    result.material = -1.0;

    for(result.steps = 0; result.steps < MAX_STEPS; result.steps++) {
        vec2 res = SDF(origin + direction * dist);
        if(res.x < (0.0001 * dist)) {
            result.distance = dist;
            return result;
        } else if(res.x > MAX_DIST) {
            result.material = -1.0;
            result.distance = -1.0;
            return result;
        }
        dist += res.x;
        result.material = res.y;
    }

    result.distance = dist;
    return result;
}

vec3 render(vec3 origin, vec3 direction) {
    vec3 color = vec3(0.0);
    IntersectionResult result = castRay(origin, direction);

    // return vec3(result.steps / float(MAX_STEPS), 0.0, 0.0); // Debug draw: Step count

    vec3 lightDirection = normalize(vec3(0.5, 0.5, -0.5));

    if(result.material > -1.0) {
        vec3 position = origin + direction * result.distance;
        vec3 normal = calculateNormal(position);

        // return normal * vec3(0.5) + vec3(0.5); // Debug draw: Normals

        color = vec3(1.0); // TODO: Material color

        vec3 directionalLight = vec3(1.25, 1.2, 0.8) * max(dot(normal, lightDirection), 0.0);
        vec3 ambientLight = vec3(0.03, 0.04, 0.1);
        color *= (directionalLight + ambientLight);
    }

    return color;
}

vec3 getCameraRayDir(vec2 uv, vec3 position, vec3 target) {
    vec3 forward = normalize(target - position);
    vec3 right = normalize(cross(vec3(0, 1, 0), forward));
    vec3 up = normalize(cross(forward, right));

    float fov = 2;
    return normalize(uv.x * right + uv.y * up + forward * fov);
}

vec2 normalizeScreenCoords(vec2 screenCoord) {
    vec2 result = 2 * (screenCoord / resolution.xy - 0.5);
    result.x *= resolution.x / resolution.y;
    return result;
}

void main() {
    vec2 uv = normalizeScreenCoords(gl_FragCoord.xy);
    vec3 rayDirection = getCameraRayDir(uv, cameraPosition, cameraTarget);
    color = vec4(render(cameraPosition, rayDirection), 1);
}
