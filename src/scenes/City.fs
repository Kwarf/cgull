#version 330

uniform vec2 resolution;
uniform float time;
uniform float fadeToBlack;
uniform vec3 cameraPosition;
uniform vec3 cameraRotation;
uniform vec3 seagullPosition;

out vec4 color;

const float MAX_DIST = 100.0;
const int MAX_STEPS = 128;
const float PI = 3.1415926535897932384626433832795;
const float HPI = PI / 2.0;
const float TWOPI = PI * 2.0;

const float BPM = 175.0;
const float SYNC_ROWS_PER_BEAT = 4.0;
const float SYNC_ROW_RATE = (BPM / 60.0) * SYNC_ROWS_PER_BEAT;

// region functions from https://mercury.sexy/hg_sdf/

float sgn(float x) {
    return (x < 0) ? -1 : 1;
}

float vmax(vec3 v) {
    return max(max(v.x, v.y), v.z);
}

void pR(inout vec2 p, float a) {
    p = cos(a) * p + sin(a) * vec2(p.y, -p.x);
}

float pMod1(inout float p, float size) {
    float halfsize = size * 0.5;
    float c = floor((p + halfsize) / size);
    p = mod(p + halfsize, size) - halfsize;
    return c;
}

vec2 pMod2(inout vec2 p, vec2 size) {
    vec2 c = floor((p + size * 0.5) / size);
    p = mod(p + size * 0.5, size) - size * 0.5;
    return c;
}

vec3 pMod3(inout vec3 p, vec3 size) {
    vec3 c = floor((p + size * 0.5) / size);
    p = mod(p + size * 0.5, size) - size * 0.5;
    return c;
}

float pModPolar(inout vec2 p, float repetitions) {
    float angle = 2 * PI / repetitions;
    float a = atan(p.y, p.x) + angle / 2.;
    float r = length(p);
    float c = floor(a / angle);
    a = mod(a, angle) - angle / 2.;
    p = vec2(cos(a), sin(a)) * r;
	// For an odd number of repetitions, fix cell index of the cell in -x direction
	// (cell index would be e.g. -5 and 5 in the two halves of the cell):
    if(abs(c) >= (repetitions / 2))
        c = abs(c);
    return c;
}

float pMirror(inout float p, float dist) {
    float s = sgn(p);
    p = abs(p) - dist;
    return s;
}

float fSphere(vec3 p, float r) {
    return length(p) - r;
}

float fBox(vec3 p, vec3 b) {
    vec3 d = abs(p) - b;
    return length(max(d, vec3(0))) + vmax(min(d, vec3(0)));
}

// endregion hg_sdf functions

float beat = time / (60.0 / BPM);
float row = time * SYNC_ROW_RATE;
float beatHit() {
    float i;
    return 1.0 - modf(beat, i);
}

vec2 mUnion(vec2 a, vec2 b) {
    return (a.x < b.x) ? a : b;
}

float sdRoundCone(vec3 p, float r1, float r2, float h) {
  // sampling independent computations (only depend on shape)
    float b = (r1 - r2) / h;
    float a = sqrt(1.0 - b * b);

  // sampling dependant computations
    vec2 q = vec2(length(p.xz), p.y);
    float k = dot(q, vec2(-b, a));
    if(k < 0.0)
        return length(q) - r1;
    if(k > a * h)
        return length(q - vec2(0.0, h)) - r2;
    return dot(q, vec2(a, b)) - r1;
}

vec2 seagull(vec3 pos) {
    // Body
    vec3 p = pos + vec3(0, 0, -0.5);
    pR(p.yz, -radians(90.0));
    float d = sdRoundCone(p, 0.4, 0.3, 1);
    d = min(d, sdRoundCone(p + vec3(0, 0.5, 0), 0.2, 0.4, 0.5));
    vec2 result = vec2(d, 1.0);
    // Tail
    pR(p.yz, radians(5.0));
    result = mUnion(result, vec2(sdRoundCone(p + vec3(0, -1, 0.1), 0.28, 0.1, 1), 2.0));
    // Beak
    result = mUnion(result, vec2(fBox(pos + vec3(0, 0, -1.25), vec3(0.05, 0.08, 0.2)), 3.0));

    // Wings, mirrored along the x axis, flapping
    float flap = beat * 2 * HPI;
    pMirror(pos.x, 0.2);
    p = pos + vec3(0, 0, -0.2);
    pR(p.xy, sin(flap) * 0.4);
    result = mUnion(result, vec2(fBox(p, vec3(2, 0.1, 0.5)), 1.0));
    p -= vec3(2.95, sin(flap) * 0.4, 0);
    pR(p.xy, sin(flap) * 0.4);
    p.z += 0.2;
    pR(p.xz, -0.2);
    result = mUnion(result, vec2(fBox(p, vec3(0.8, 0.1, 0.45)), 2.0));

    // Eyes, also mirrored
    p = pos + vec3(0, -0.12, -0.9);
    result = mUnion(result, vec2(fSphere(p, 0.05), 4.0));

    return result;
}

vec2 flock(vec3 pos) {
    vec2 result = vec2(1000.0, 0.0);
    pos += vec3(sin(beat / 4 * HPI), 0, sin(beat / 8 * HPI) * 8.0);
    pR(pos.xy, -radians(10.0) * sin(beat / 8 * HPI));
    for(int i = -1; i < 2; i++) {
        vec3 p = pos + vec3(float(i) * 10, 0, abs(i) * 3);
        pR(p.yz, -radians(10.0) * sin(beat / 4 * HPI));
        pR(p.xy, -radians(45.0) * i * sin(beat / 2 * HPI));
        result = mUnion(result, seagull(p));
    }
    return result;
}

vec2 SDF(vec3 pos) {
    vec2 result = vec2(1000.0, 0.0);

    if(row < 384) {
        // Repeate buildings across xz
        vec3 p = pos + vec3(25, 0, 0);
        pMod2(p.xz, vec2(50));
        // Repeat floors along the y axis
        pMod1(p.y, 3);
        float d = fBox(p, vec3(10, 1.2, 10));
        d = min(d, fBox(p, vec3(9.5)));

        result = vec2(d, 5.0);

        // Seagull
        if(row < 256) {
            result = mUnion(result, seagull(pos - seagullPosition));
        } else {
            result = mUnion(result, flock(pos - seagullPosition));
        }
    } else if(row < 512) {
        // Tunnel thing
        vec3 p = pos;
        pModPolar(p.xz, 6);
        p.x -= 3;
        pMod1(p.y, 3);
        pR(p.xz, p.y * 0.2 * sin(beat * 2.0 * HPI));
        float d = fBox(p, vec3(1.0));
        result = vec2(d, 5.0);

        // Seagulls
        p = pos - vec3(0, cameraPosition.y * 2.0, 0);
        pR(p.xz, time);
        pModPolar(p.xz, 6);
        pMod1(p.y, 12);
        p += vec3(-8, 0, 0);
        pR(p.yz, -radians(90.0));
        pR(p.xy, -radians(90.0));
        result = mUnion(result, seagull(p));

        // Centered seagull facing back
        p = pos - vec3(0, cameraPosition.y + 3, -0.5);
        pR(p.yz, -radians(90.0));
        pR(p.xz, -radians(180));
        vec2 sg = seagull(p / 0.5);
        sg.x *= 0.5;
        result = mUnion(result, sg);
    } else {
        // Repeate buildings across xz
        vec3 p = pos + vec3(25, 0, 0);
        pMod2(p.xz, vec2(50));
        // Repeat floors along the y axis
        pMod1(p.y, 3);
        float d = fBox(p, vec3(10, 1.2, 10));
        d = min(d, fBox(p, vec3(9.5)));

        result = vec2(d, 5.0);

        // Grid repeated seagulls?
        p = pos + vec3(0, 25, 25 - time * 500);
        pMod3(p, vec3(50));
        result = mUnion(result, seagull(p));
    }

    return result;
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

float checkersTextureGradBox(in vec2 p, in vec2 ddx, in vec2 ddy) {
    vec2 w = max(abs(ddx), abs(ddy)) + 0.01;
    vec2 i = 2.0 * (abs(fract((p - 0.5 * w) / 2.0) - 0.5) - abs(fract((p + 0.5 * w) / 2.0) - 0.5)) / w;
    return 0.5 - 0.5 * i.x * i.y;
}

// Return a psuedo random value in the range [0, 1), seeded via coord
float rand(vec2 coord) {
    return fract(sin(dot(coord.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 applyFog(vec3 rgb, float dist) {
    float startDist = 200.0;
    float fogAmount = 1.0 - exp(-(dist - 8.0) * (1.0 / startDist));
    return mix(rgb, vec3(0.05, 0.05, 0.1), fogAmount);
}

vec3 render(vec2 uv, vec3 origin, vec3 direction) {
    vec3 color = vec3(0.0);
    IntersectionResult result = castRay(origin, direction);

    float nsteps = result.steps / float(MAX_STEPS);
    // return vec3(result.steps / float(MAX_STEPS), 0.0, 0.0); // Debug draw: Step count

    if(result.material > -1.0) {
        vec3 position = origin + direction * result.distance;
        vec3 normal = calculateNormal(position);
        // return normal * vec3(0.5) + vec3(0.5); // Debug draw: Normals

        // vec3 lightPosition = vec3(0, 1, 0);
        // float lightRadius = 3.0;
        vec3 lightDirection = normalize(vec3(0.3, 0.9, -0.6));
        // vec3 lightDirection = normalize(lightPosition - position);

        color = vec3(1, 0, 0); // This is the "error" color, if I've forgotten to assign a material
        if(result.material == 1.0) { // "Seagull" white
            color = vec3(1.0);
        } else if(result.material == 2.0) { // "Seagull" gray
            color = vec3(0.4);
        } else if(result.material == 3.0) { // "Seagull" beak orange
            color = vec3(1.0, 0.52, 0.0);
        } else if(result.material == 4.0) { // Black
            color = vec3(0);
        } else if(result.material == 5.0) { // Steps-based blue
            color = vec3(0.21, 0.31, 0.78) * result.steps / float(MAX_STEPS);
        }

        // Light
        vec3 directionalLight = vec3(1.0) * max(dot(normal, lightDirection), 0.0);
        vec3 ambientLight = vec3(0.03);
        color *= directionalLight + ambientLight;

        // Shadows
        // IntersectionResult shadowRayIntersection = castRay(position + normal * 0.001, lightDirection);
        // if(shadowRayIntersection.material != -1.0) {
        //     color = mix(color, vec3(0), 0.8);
        // }

        color = applyFog(color, result.distance);
    } else {
        color = vec3(0.07, 0.12, 0.4);
    }

    // Some vignettish looking thing
    color *= smoothstep(0.8, 0.2, length(uv) / 1.0 * 0.3);

    return color;
}

mat3 rotateX(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(vec3(1, 0, 0), vec3(0, c, -s), vec3(0, s, c));
}

mat3 rotateY(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(vec3(c, 0, s), vec3(0, 1, 0), vec3(-s, 0, c));
}

mat3 rotateZ(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(vec3(c, -s, 0), vec3(s, c, 0), vec3(0, 0, 1));
}

vec3 getCameraRayDir(vec2 uv, vec3 position, vec3 rotation) {
    vec3 rd = normalize(vec3(uv / 2, 1));
    rd *= rotateX(rotation.x);
    rd *= rotateY(rotation.y);
    rd *= rotateZ(rotation.z);
    return rd;
}

vec2 normalizeScreenCoords(vec2 screenCoord) {
    vec2 result = 2 * (screenCoord / resolution.xy - 0.5);
    result.x *= resolution.x / resolution.y;
    return result;
}

void main() {
    vec3 camR = cameraRotation;
    if(row >= 512) {
        camR = vec3(sin(time * HPI / 4.0) * 180.0);
    }

    vec2 uv = normalizeScreenCoords(gl_FragCoord.xy);
    vec3 rayDirection = getCameraRayDir(uv, cameraPosition, radians(camR));
    color = pow(vec4(render(uv, cameraPosition, rayDirection), 1), vec4(0.4545));
    color = mix(color, vec4(0, 0, 0, 1), fadeToBlack);
}
