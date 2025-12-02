#ifndef TEXT_OVERLAY_H
#define TEXT_OVERLAY_H

#include <vector>
#include <string>

// Draw text onto an RGB image
// image: RGB buffer (width * height * 3)
// width: image width
// height: image height
// text: string to draw
// x, y: position of the top-left corner of the text
// color: RGB color (0-255)
void draw_text(
    std::vector<unsigned char> & image,
    int width,
    int height,
    const std::string & text,
    int x,
    int y,
    const std::vector<unsigned char> & color,
    int scale = 1);

#endif
