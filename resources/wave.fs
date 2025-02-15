#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

vec3 hsv2rgb(vec3 c) {
  vec3 p = abs(fract(c.x + vec3(0.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0);
  return c.z * mix(vec3(1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

void main() {
  vec2 uv = fragTexCoord;
  // float wave = texture(texture0, fragTexCoord).r;
  int t_width = 256;
  // float wave = texelFetch(texture0, ivec2(int((t_width-1) * uv.x), 0), 0).r;
  // float wave_next = texelFetch(texture0, ivec2(int((t_width-1) * uv.x) + 1, 0), 0).r;
  // float wave_next2 = texelFetch(texture0, ivec2(int((t_width-1) * uv.x) + 8, 0), 0).r;
  // float dw = abs(wave_next2 - wave);
  float wave = texture(texture0, vec2(uv.x*(t_width-1)/t_width, 0)).r;
  float wave_next = texture(texture0, vec2(uv.x*(t_width-1)/t_width + (1./t_width), 0)).r;

  // vec3 val = vec3(1.0 - smoothstep(0.0, 0.02, abs(wave - uv.y)));
  // float val = 1.0 - smoothstep(0.0, max(abs(wave-wave_next), 0.02), abs(wave
  // - uv.y));

  float dy = wave - uv.y;
  float dyn = wave_next - uv.y;
  float dn = wave_next - wave;
  float val = 0.;

  float tol = 0.01;
  float smo = 0.00;

  if (dn < 0.) {
    float tmp = dy;
    dy = dyn;
    dyn = tmp;
  }

  float d = min(-(dy - (tol + smo)), (dyn + (tol + smo)));
  val = smoothstep(0.0, smo, d);

  // val = 1.0 - smoothstep(0.0, max(abs(wave-wave_next), 0.02), dy);

  // vec3 col = hsv2rgb(vec3(dw * 5, 1., val));
  vec3 col = hsv2rgb(vec3(0.3, 1., val));

  // output final color
  finalColor = vec4(col, 1.0);
}
