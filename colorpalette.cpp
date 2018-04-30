#include "colorpalette.h"

ColorPalette& ColorPalette::getInstance() {
    static ColorPalette instance;
    return instance;
}


ColorPalette::ColorPalette() {
    //setDarkTheme();
    setLightTheme();
}

void ColorPalette::setLightTheme() {
    def   = QColor(249, 249, 249);
    high  = QColor(238, 238, 238);
    mouse = QColor(228, 228, 228);
    sel   = QColor(128, 193, 142);
    font  = QColor(11, 11, 11);
    stroke = QColor(16,16,16);
    canvas = def;
}

void ColorPalette::setDarkTheme() {
    def   = QColor(65, 65, 65);
    high  = QColor(75, 75, 75);
    mouse = QColor(95, 95, 95);
    sel   = QColor(211, 139, 72);
    font  = QColor(249, 249, 249);
    stroke = QColor(232,232,232);
    canvas = def;
}
