#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QColor>

/*
 * A singleton, manages colors for all nodes
 */

class ColorPalette
{
public:
    static ColorPalette& getInstance();

    // Easy access static methods
    static QColor defaultColor() { return ColorPalette::getInstance().def; }
    static QColor highlightColor() { return ColorPalette::getInstance().high; }
    static QColor mouseDownColor() { return ColorPalette::getInstance().mouse; }
    static QColor selectColor() { return ColorPalette::getInstance().sel; }
    static QColor fontColor() { return ColorPalette::getInstance().font; }
    static QColor strokeColor() { return ColorPalette::getInstance().stroke; }
    static QColor canvasColor() { return ColorPalette::getInstance().canvas; }
    static QColor targetColor() { return ColorPalette::getInstance().target; }

    static void darkTheme() { ColorPalette::getInstance().setDarkTheme(); }
    static void lightTheme() { ColorPalette::getInstance().setLightTheme(); }


    ColorPalette(ColorPalette const&) = delete;
    void operator=(ColorPalette const&) = delete;

private:
    ColorPalette();
    void setDarkTheme();
    void setLightTheme();

    QColor def, high, mouse, sel, font, stroke, canvas, target;
};

#endif // COLORPALETTE_H
