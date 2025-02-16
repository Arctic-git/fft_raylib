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
    // vec4 texelColor = texture(texture0, fragTexCoord);  
    // finalColor = texelColor*colDiffuse*fragColor;  

    vec2 uv = fragTexCoord;     
    // vec3 col = vec3(uv.x, uv.y, 0);
    

    // vec3 col = hsv2rgb(vec3(uv.y, 1.0, 1-abs(uv.y-0.5)*2));
    vec3 col = hsv2rgb(vec3(uv.y, 1.0, 1.));

    finalColor = vec4(col, 1.0);
}                                 
