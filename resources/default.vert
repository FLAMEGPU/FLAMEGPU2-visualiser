#version 430

uniform mat3 _normalMat;
uniform mat4 _modelViewProjectionMat;
uniform mat4 _modelViewMat;

in vec3 _vertex;
in vec3 _normal;
in vec2 _texCoords;

out vec3 eyeVertex;
out vec3 eyeNormal;
out vec2 texCoords;

void main()
{
  gl_Position = _modelViewProjectionMat * vec4(_vertex,1.0f);

  eyeNormal = normalize(_normalMat * _normal) ;
  eyeVertex = (_modelViewMat * vec4(_vertex, 1.0f)).rgb;
  texCoords = _texCoords;
}