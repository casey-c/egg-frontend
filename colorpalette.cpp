#include "colorpalette.h"

ColorPalette& ColorPalette::getInstance() {
    static ColorPalette instance;
    return instance;
}


ColorPalette::ColorPalette() {
    def   = QColor(249, 249, 249);
    high  = QColor(238, 238, 238);
    mouse = QColor(228, 228, 228);
    sel   = QColor(128, 193, 142);
}
