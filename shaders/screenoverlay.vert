layout(location=0) in vec3 a_position;
in vec2 a_texCoord;

uniform mat4 u_projection;

out vec2 v_texCoord;

void main()
{
  gl_Position = u_projection * vec4(a_position, 1);
  v_texCoord = a_texCoord;
}
