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

    QColor getDefault() { return def; }
    QColor getHighlight() { return high; }
    QColor getMouseDown() { return mouse; }
    QColor getSelected() { return sel; }

    // Easy access static methods
    static QColor defaultColor() { return ColorPalette::getInstance().getDefault(); }
    static QColor highlightColor() { return ColorPalette::getInstance().getHighlight(); }
    static QColor mouseDownColor() { return ColorPalette::getInstance().getMouseDown(); }
    static QColor selectColor() { return ColorPalette::getInstance().getSelected(); }

    ColorPalette(ColorPalette const&) = delete;
    void operator=(ColorPalette const&) = delete;

private:
    ColorPalette();

    QColor def, high, mouse, sel;
};

#endif // COLORPALETTE_H
