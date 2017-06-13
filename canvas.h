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

    void clearBounds();
    void addRedBound(QRectF rect);
    void addBlackBound(QRectF rect);
    void addBlueBound(QRectF rect);
    void addGreenBound(QRectF rect);

    void clearDots();
    void addBlackDot(QPointF pt);

private:
    //////////////
    /// Fields ///
    //////////////

    Node* root;
    Node* highlighted;

    QPointF lastMousePos;

    QGraphicsScene* scene;

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
    void addStatement(QString s);
    void addPlaceholder();

    // Highlight
    void highlightRoot();
    void highlightChild();
    void highlightRight();
    void highlightLeft();
    void highlightParent();

    // Debug
    bool showBounds;
    //QGraphicsRectItem* debugBox;
    //QGraphicsRectItem* debugBox2;

    QList<QGraphicsRectItem*> blueBounds;
    QList<QGraphicsRectItem*> blackBounds;
    QList<QGraphicsRectItem*> redBounds;

    QList<QGraphicsEllipseItem*> blackDots;
};

#endif // CANVAS_H
