#version 430

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D _texture;
uniform vec4 _col;
uniform vec4 _backCol;
uniform ivec2 _viewportDims;
//Blends round corners onto the tex
//This isn't a very nice fn, but it works
float roundCorner()
{
    int rad = 8;
    int radsq = rad*rad;
    float radm1sq = radsq-((rad-1)*(rad-1));
    //Get dimensions of the texture
    ivec2 texDim = textureSize(_texture, 0);
    //Calc how close we are to each bound of tex
    int left = int(texCoords.x*texDim.x);
    int bottom = int(texCoords.y*texDim.y);
    int right = texDim.x - left;
    int top = texDim.y - bottom;
    //No idea, why, but these increments are required
    left++;
    bottom++;
    //gl_FragCoord.y+top>=_viewportDims.y
    //gl_FragCoord.x+right+10000>=_viewportDims.x
    if(right<=rad&&bottom<=rad
    &&gl_FragCoord.y>bottom&&gl_FragCoord.x+right<=_viewportDims.x)
    {
        int b = rad-right;
        int r = rad-bottom;
        int tot = b*b+r*r;//Length^2
        float rtn = ((radsq-tot)/radm1sq);
        return rtn>1.0f?1.0f:rtn;
    }
    else if(right<=rad&&top<=rad
    &&gl_FragCoord.y+top<=_viewportDims.y&&gl_FragCoord.x+right<=_viewportDims.x)
    {
        int b = rad-right;
        int r = rad-top;
        int tot = b*b+r*r;//Length^2
        float rtn = ((radsq-tot)/radm1sq);
        return rtn>1.0f?1.0f:rtn;
    }
    else if(left<=rad&&bottom<=rad
    &&gl_FragCoord.x>left&&gl_FragCoord.y>bottom)
    {
        int b = rad-left;
        int r = rad-bottom;
        int tot = b*b+r*r;//Length^2
        float rtn = ((radsq-tot)/radm1sq);
        return rtn>1.0f?1.0f:rtn;
    }
    else if(left<=rad&&top<=rad
    &&gl_FragCoord.x>left&&gl_FragCoord.y+top<=_viewportDims.y)
    {
        int b = rad-left;
        int r = rad-top;
        int tot = b*b+r*r;//Length^2
        float rtn = ((radsq-tot)/radm1sq);
        return rtn>1.0f?1.0f:rtn;
    }
    else
        return 1.0f;
}
void main()
{
  //TextureLod(0) so even if mipmaps gen, we try to use best quality
  //float tex = textureLod(_texture, vec2(texCoords.x,-texCoords.y),0.0f).r;
  //Grab a solid pixel, ensure no interpolation
  ivec2 texDim = textureSize(_texture, 0);
  float tex = texelFetch(_texture, ivec2(int(texCoords.x*texDim.x),texDim.y-int(texCoords.y*texDim.y)),0).r;
  //Interpolate the forecolour according to the tex
  vec4 foreground = _col*tex;
  //Interpolate the background according to inverse tex
  vec4 background = _backCol*(1-tex);
  background.w *= roundCorner();
  //Blend the colors according to their own alpha channels
  vec3 blend = (foreground.xyz*foreground.w) + (background.xyz*background.w);
  //Output the blended rgb values with a summed alpha channel
  fragColor = vec4(blend, (foreground.w+background.w));
}