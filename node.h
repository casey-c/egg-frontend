#ifndef NODE_H
#define NODE_H

#include "quantumbool.h"

#include <QGraphicsObject>
#include <QRadialGradient>
#include <QGraphicsDropShadowEffect>

class Canvas;

enum NodeType
{
    Root,
    Cut,
    Statement,
    Placeholder
};

class Node : public QGraphicsObject
{
public:
    static Node* makeRoot(Canvas* can);
    ~Node() {}

    // Add
    Node* addChildCut(QPointF pt);
    Node* addChildStatement(QPointF pt, QString t);
    Node* addChildPlaceholder(QPointF pt);

    // Highlight
    void setAsHighlight();
    void removeHighlight();

    // Identifiers
    bool isRoot() const { return type == Root; }
    bool isCut() const { return type == Cut; }
    bool isStatement() const { return type == Statement; }
    bool isPlaceholder() const { return type == Placeholder; }

    // Getters
    Node* getParent() const { return parent; }
    Node* getRightSibling();
    Node* getLeftSibling();
    Node* getChild();

private:

    //////////////
    /// Fields ///
    //////////////

    static int globalID;
    int myID;

    Canvas* canvas;
    Node* parent;

    // Visual details
    NodeType type;
    //QPointF upperLeftPt, bottomRightPt;
    //qreal translateOffsetX, translateOffsetY;
    QString text;

    bool highlighted;
    bool mouseDown;

    QRadialGradient gradDefault;
    QRadialGradient gradHighlighted;
    QRadialGradient gradClicked;

    QGraphicsDropShadowEffect* shadow;

    // Important points
    QPointF lastHoverPos;
    QPointF mouseOffset;

    // Children
    QList<Node*> children;
    //qreal minX, minY, maxX, maxY;

    // Collision
    //QRectF collisionBounds;
    //QRectF potentialBounds;
    //QuantumBool hasPotentialBounds;

    // New Important Points & Rects
    qreal width, height; // in absolute pixels (a multiple of GRID_SPACING)

    QPointF deltaPosShift; // a pixel amount to shift the main pos() by
    QPointF potentialPosShift; // for checking collision mid-movement

    QuantumBool hasDifferentPotentialBounds; // mid movement

    QRectF drawBox; // top left is pos().x() - deltaPosShift.x(), etc.
    QRectF collisionBox; // drawBox, but grown in all directions by GRID_SPACING / 2 + 1

    //QRectF potentialCollisionBox;

    ///////////////
    /// Methods ///
    ///////////////

    // Private constructor
    Node(Canvas* can, Node* par, NodeType t, QPointF pt);

    // Graphics
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    QRectF getCollisionRect() const;
    void updateCollisionBox(); //set the collisionBox

    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    // Moving
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant &value) override;

    // Mouse
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};

#endif // NODE_H
