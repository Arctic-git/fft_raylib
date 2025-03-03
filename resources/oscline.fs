#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float iTime;

vec3 hsv2rgb(vec3 c) {
  vec3 p = abs(fract(c.x + vec3(0.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0);
  return c.z * mix(vec3(1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
}

float decodeFloatFromRGBA(vec4 color) {
  uint intBits = (uint(color.r * 255.0) << 24) | (uint(color.g * 255.0) << 16) |
                 (uint(color.b * 255.0) << 8) | (uint(color.a * 255.0));
  return uintBitsToFloat(intBits);
}

// https://m1el.github.io/woscope-how/
#define EPS 1E-6
#define TAU 6.283185307179586
#define TAUR 2.5066282746310002
#define SQRT2 1.4142135623730951

// A standard gaussian function, used for weighting samples
float gaussian(float x, float sigma) {
  return exp(-(x * x) / (2.0 * sigma * sigma)) / (TAUR * sigma);
}

// This approximates the error function, needed for the gaussian integral
float erf(float x) {
  float s = sign(x), a = abs(x);
  x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
  x *= x;
  return s - s / (x * x);
}

uniform float iLenDarken = 1;
uniform float iSize = 10; // thickness/2
uniform float iIntensity = 1;
uniform float iSigma = 1;
uniform vec4 iColor = vec4(1,0,0,1);

void main() {
  // vec4 texelColor = texture(texture0, fragTexCoord);
  // finalColor = texelColor*colDiffuse*fragColor;

  float len = decodeFloatFromRGBA(fragColor); // length without expansion
  vec2 uv = fragTexCoord; //[0, 1]

  vec2 xy;
  xy.y = (uv.y * 2 - 1) * iSize;            //[-iSize, iSize]
  xy.x = -iSize + uv.x * (iSize * 2 + len); //[-iSize, iSize+len]

  float sigma = iSize / 4.0 * iSigma; // /8 is sharper but not correct?
  float alpha;

  if (len < 0.2) {
    alpha =
        exp(-pow(length(xy), 2.0) / (2.0 * sigma * sigma)) / 2.0 / sqrt(iSize);
  } else {
    alpha = erf(xy.x / SQRT2 / sigma) - erf((xy.x - len) / SQRT2 / sigma);
    alpha *= exp(-xy.y * xy.y / (2.0 * sigma * sigma)) / 2.0 /
             (len * iLenDarken + (1.0 - iLenDarken)) * iSize;
  }


  finalColor = vec4(iColor.xyz, alpha * iIntensity);
}
