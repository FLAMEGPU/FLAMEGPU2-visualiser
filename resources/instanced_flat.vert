#version 430

uniform mat3 _normalMat;
uniform mat4 _modelViewProjectionMat;
uniform mat4 _modelViewMat;
uniform mat4 _modelMat;

in vec3 _vertex;
in vec2 _texCoords;

uniform samplerBuffer x_pos;
uniform samplerBuffer y_pos;
uniform samplerBuffer z_pos;

out vec3 eyeVertex;
out vec3 eyeUNormal;
out vec2 texCoords;

void main()
{
  //Grab model offset from texture array
  vec3 loc_data = vec3(texelFetch(x_pos, gl_InstanceID).x, texelFetch(y_pos, gl_InstanceID).x, texelFetch(z_pos, gl_InstanceID).x);
  loc_data *= 20;
  gl_Position = _modelViewProjectionMat * vec4(_vertex+loc_data,1.0f);

  eyeVertex = (_modelViewMat * vec4(_vertex+loc_data, 1.0f)).rgb;
  texCoords = _texCoords;
}