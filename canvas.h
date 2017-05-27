#ifndef CANVAS_H
#define CANVAS_H

#include <QGraphicsView>

class Node;

class Canvas : public QGraphicsView
{
public:
    Canvas(QWidget* parent = 0);
    ~Canvas() {}

    void setHighlight(Node* node);

private:
    //////////////
    /// Fields ///
    //////////////

    Node* root;
    Node* highlighted;

    QPointF lastMousePos;

    ///////////////
    /// Methods ///
    ///////////////

    // User input
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void drawBackground(QPainter* painter,
                        const QRectF &rect) override;

    // Add
    void addCut();

    // Highlight
    void highlightRoot();
    void highlightChild();
    void highlightRight();
    void highlightLeft();
    void highlightParent();
};

#endif // CANVAS_H
