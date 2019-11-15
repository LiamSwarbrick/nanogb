#version 330

// Input fragment attributes (from fragment shader)
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

uniform vec2 source_size;  // texture size
uniform vec2 dest_size;  // size of destination of texture

// Output fragment color
out vec4 finalColor;

void
main()
{
    vec2 texel_size = vec2(dest_size / source_size);

    // clean pixel effect acheived with OpenEmu's implementation from their pixellate shader
    // https://github.com/OpenEmu/OpenEmu/blob/master/OpenEmu/Shaders/Pixellate/pixellate/shaders/pixellate.slang
    vec2 range = vec2(abs(source_size.x / dest_size.x * source_size.x), abs(source_size.y / dest_size.y * source_size.y));

    float left   = fragTexCoord.x - range.x;
    float top    = fragTexCoord.y + range.y;
    float right  = fragTexCoord.x + range.x;
    float bottom = fragTexCoord.y - range.y;

    vec3 tl_col;  // top left
    vec3 br_col;  // bottom right
    vec3 bl_col;  // bottom left
    vec3 tr_col;  // top right

    tl_col = pow(texture(texture0, (floor(vec2(left, top) / texel_size) + 0.5) * texel_size).rgb, vec3(2.2));
    br_col = pow(texture(texture0, (floor(vec2(right, bottom) / texel_size) + 0.5) * texel_size).rgb, vec3(2.2));
    bl_col = pow(texture(texture0, (floor(vec2(left, bottom) / texel_size) + 0.5) * texel_size).rgb, vec3(2.2));
    tr_col = pow(texture(texture0, (floor(vec2(right, top) / texel_size) + 0.5) * texel_size).rgb, vec3(2.2));

    vec2 border = clamp(round(fragTexCoord / texel_size) * texel_size, vec2(left, bottom), vec2(right, top));
    float total_area = 4.0 * range.x * range.y;

    vec3 average_col = ((border.x - left) * (top - border.y) / total_area) * tl_col;
    average_col += ((right - border.x) * (border.y - bottom) / total_area) * br_col;
    average_col += ((border.x - left) * (border.y - bottom) / total_area) * bl_col;
    average_col += ((right - border.x) * (top - border.y) / total_area) * tr_col;
    
    finalColor = vec4(pow(average_col, vec3(1.0 / 2.2)), 1.0) * fragColor;
    // finalColor = vec4(averageColor, 1.0);
    // finalColor = texture(texture0, fragTexCoord);
}
