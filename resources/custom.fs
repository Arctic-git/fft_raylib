#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float iTime;

vec2 iResolution = vec2(1.0, 1.0);

float rand(vec2 co) {
  return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}
float dither(vec2 uv) { return (rand(uv) * 2.0 - 1.0) / (5. * 256.0); }

vec4 line(vec2 uv, float v, float h, vec3 col) {

  float wave = sin(iTime * v + uv.x * h * 1.7) * 0.87;
  uv.y += wave * smoothstep(1.0, 0.0, abs(uv.x));
  float thickness = smoothstep(0.1, 0.0, abs(uv.y));
  float fade =
      smoothstep(1.0, 0.2, abs(uv.y)) * smoothstep(1.0, 0.3, abs(uv.x));
  return vec4(col * thickness * fade, 1.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
  vec2 uv = fragCoord / iResolution.xy;

  // Scale so that x is in the range [0,1]
  uv.x = uv.x * 2.0 - 1.0; // Centered between -1 and 1 for consistency
  uv.y = (2.0 * fragCoord.y - iResolution.y) / iResolution.y; // Keep y centered
  float num_waves = 20.;

  for (float i = 0.0; i <= num_waves; i += 1.0) {
    float t = i / num_waves * 2.;
    // fragColor += line(uv, 1., t, vec3(1.0 - t, abs(1.-t)*0.3, t * 0.6));
    fragColor += line(uv, 1. + t * 0.001, t,
                      vec3(1.0 - t * 1.5, abs(1. - t) * 0.3, t * 0.6)) +
                 vec4(dither(uv));
  }
}

void main() {
  vec4 color;
  mainImage(color, fragTexCoord * iResolution); // shadertoy wants pixel coords
  finalColor = color;
}
