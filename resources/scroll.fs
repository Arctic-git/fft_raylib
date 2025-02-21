#version 330    

in vec2 fragTexCoord;             
in vec4 fragColor;       

out vec4 finalColor;     

uniform sampler2D texture0;       
uniform vec4 colDiffuse;   
uniform float xScrollOffs;
uniform float yScrollOffs;


void main() {                                 
    vec4 texelColor = texture(texture0, vec2(fragTexCoord.x + xScrollOffs, fragTexCoord.y + yScrollOffs));  
    // vec4 texelColor = texture(texture0, vec2(mod(fragTexCoord.x + xScrollOffs, 1), mod(fragTexCoord.y, 1))); // if texture doesn't wrap

    finalColor = texelColor*colDiffuse*fragColor;       
}                                 
