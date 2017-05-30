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

    //QRectF getChildBoxInScene() const;

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
    //QRectF childBox;

    // New Important Points & Rects
    qreal width, height; // in absolute pixels (a multiple of GRID_SPACING)

    //QuantumBool hasDifferentPotentialBounds; // mid movement

    QRectF drawBox; // size of everything that gets drawn by this node
    //QRectF collisionBox; // drawBox, but grown in all directions by GRID_SPACING / 2 + 1

    //QRectF potentialBounds;

    ///////////////
    /// Methods ///
    ///////////////

    // Private constructor
    Node(Canvas* can, Node* par, NodeType t, QPointF pt);

    // Graphics
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    //QRectF getCollisionRect() const;
    //void updateCollisionBox(); //set the collisionBox


    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    // Moving
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant &value) override;

    QPointF collisionLessPoint(QPointF val);
    bool rectAvoidsCollision(QRectF rect) const;

    QRectF getSceneCollisionBox(qreal deltaX = 0, qreal deltaY = 0) const;
    //QRectF getSceneCollisionBox() const;
    //QRectF getTranslatedSceneCollisionRect(qreal delX, qreal delY) const;

    //void calculateChildBox();
    //void resizeToFitChildBox();

    QRectF getPotentialSceneCollision(qreal deltaX, qreal deltaY) const;

    QRectF getTranslatedDrawBox(qreal deltaX, qreal deltaY) const;
    QRectF getDrawAsCollision(const QRectF &draw) const;
    QRectF rectToScene(const QRectF &rect) const;

    //QRectF convertTempCollisionToDrawBox();

    // Mouse
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};

#endif // NODE_H
