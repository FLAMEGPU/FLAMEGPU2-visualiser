#version 430

uniform mat3 _normalMat;
uniform mat4 _projectionMat;
uniform mat4 _viewMat;
uniform mat4 _modelMat;

in vec3 _vertex;
in vec3 _normal;
in vec2 _texCoords;

out vec3 eyeVertex;
out vec3 eyeNormal;
out vec2 texCoords;
out vec4 colorOverride;
flat out int shaderColor;

vec3 getScale();
mat3 getDirection();
vec3 getPosition();
vec4 calculateColor();
void main() {
  // Apply model matrix to raw vertex
  vec4 vert = _modelMat * vec4(_vertex,1.0f);
  // Apply a user defined scale multiplier
  vert.xyz *= getScale();
  // Apply a user defined rotation
  mat3 directionMat = getDirection();
  vert.xyz = directionMat * vert.xyz;
  // Grab model offset from texture array
  vec3 loc_data = getPosition();
  // Apply loc_data translation to vert
  vert.xyz += loc_data;
  // Calculate eye vertex
  eyeVertex = (_viewMat * vert).rgb;
  // Calculate frag vertex
  gl_Position = _projectionMat * vec4(eyeVertex, 1.0f);
  
  // Calc eye normal
  eyeNormal = normalize(_normalMat * transpose(inverse(directionMat)) * normalize(_normal));
  // Calc tex coords
  texCoords = _texCoords;
  // Get color
  colorOverride = calculateColor();
  // Tell frag shader we are overriding the color
  // Likely more performant to include this directly in the frag shader so branches can be optimised out
  shaderColor = 1;
}
