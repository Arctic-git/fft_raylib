#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec2 fragTexCoord; // normalized [0, 1] to bounds of quad
out vec4 fragColor;
out vec2 fragPos; // normalized [0, 1] to screen (cursed dpi)

uniform mat4 mvp;

void main() {
  fragTexCoord = vertexTexCoord;
  fragColor = vertexColor;
  gl_Position = mvp * vec4(vertexPosition, 1.0);
  fragPos = gl_Position.xy;
}
