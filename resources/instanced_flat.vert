#version 430

uniform mat4 _projectionMat;
uniform mat4 _viewMat;
uniform mat4 _modelMat;

in vec3 _vertex;
in vec2 _texCoords;

uniform samplerBuffer x_pos;
uniform samplerBuffer y_pos;
uniform samplerBuffer z_pos;

out vec3 eyeVertex;
out vec2 texCoords;

void main()
{
  // Apply model matrix to raw vertex
  vec4 vert = _modelMat * vec4(_vertex,1.0f);
  //Grab model offset from texture array
  vec3 loc_data = vec3(texelFetch(x_pos, gl_InstanceID).x, texelFetch(y_pos, gl_InstanceID).x, texelFetch(z_pos, gl_InstanceID).x);
  loc_data *= 10;
  // Apply loc_data translation to vert
  vert.xyz += loc_data;
  // Calculate eye vertex
  eyeVertex = (_viewMat * vert).rgb;
  // Calculate frag vertex
  gl_Position = _projectionMat * vec4(eyeVertex,1.0f);

  texCoords = _texCoords;
}