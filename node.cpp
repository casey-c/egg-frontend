#include "canvas.h"
#include "node.h"

#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QtCore/QtMath>
#include <QDebug>

#define GRID_SPACING 16
#define STROKE_ADJ 2

// Forward declarations for helper functions (implementation located at end)
QPointF snapPoint(const QPointF &pt);
qreal dist(const QPointF &a, const QPointF &b);
QPointF closest(const QList<QPointF> &list, const QPointF &target);
bool rectsCollide(const QRectF &a, const QRectF &b);
void printPt(const QString &s, const QPointF &pt);
void printRect(const QString &s, const QRectF &r);

// Static var intitial declaration
int Node::globalID = 0;

////////////////////
/// Construction ///
////////////////////

/*
 * (Static) Returns a new root node
 */
Node* Node::makeRoot(Canvas* can)
{
    return new Node(can, nullptr, Root, QPointF(0,0));
}

/*
 * Private constructor for all types of nodes
 */
Node::Node(Canvas* can, Node* par, NodeType t, QPointF pt) :
    myID(globalID++),
    canvas(can),
    parent(par),
    type(t),
    upperLeftPt(0,0),
    bottomRightPt(0,0),
    translateOffsetX(0),
    translateOffsetY(0),
    highlighted(false),
    mouseDown(false),
    lastHoverPos(0, 0),
    mouseOffset(0, 0),
    minX(0),
    minY(0),
    maxX(0),
    maxY(0),
    hasPotentialBounds(false)
{

    // Colors
    gradDefault = QRadialGradient( upperLeftPt.x() - 3,
                                   upperLeftPt.y() - 3,
                                   (dist(upperLeftPt, bottomRightPt) / 4 ));
    gradDefault.setColorAt(0, QColor(255, 255, 255));
    gradDefault.setColorAt(1, QColor(185, 185, 185));

    gradHighlighted = QRadialGradient( upperLeftPt.x() - 3,
                                       upperLeftPt.y() - 3,
                                       (dist(upperLeftPt, bottomRightPt) / 4));
    gradHighlighted.setColorAt(0, QColor(240, 240, 240));
    gradHighlighted.setColorAt(1, QColor(210, 210, 210));

    gradClicked = QRadialGradient( upperLeftPt.x() - 3,
                                   upperLeftPt.y() - 3,
                                   (dist(upperLeftPt, bottomRightPt) / 4 ));
    gradClicked.setColorAt(0, QColor(210, 210, 210));
    gradClicked.setColorAt(1, QColor(240, 240, 240));

    shadow.setEnabled(true);

    if ( isRoot() )
    {

    }
    else if ( isCut() )
    {
        setFlag(ItemIsMovable);
        setFlag(ItemSendsGeometryChanges);
        setCacheMode(DeviceCoordinateCache);
        setAcceptHoverEvents(true);

        QPointF snapped = snapPoint(pt);
        upperLeftPt.setX(snapped.x());
        upperLeftPt.setX(snapped.y());

        bottomRightPt.setX(snapped.x() + 4 * GRID_SPACING);
        bottomRightPt.setY(snapped.y() + 4 * GRID_SPACING);
    }
}

/*
 * Allocates a new child cut and returns a ptr to it
 */
Node* Node::addChildCut(QPointF pt)
{
    Node* newChild = new Node(canvas, this, Cut, mapFromScene(pt));
    children.append(newChild);
    newChild->setParentItem(this);
    return newChild;
}

/////////////////
/// Highlight ///
/////////////////

/*
 * Highlight this node
 */
void Node::setAsHighlight()
{
    highlighted = true;
    update();
}

/*
 * Clear highlighting from this node
 */
void Node::removeHighlight()
{
    highlighted = false;
    update();
}

////////////////
/// Graphics ///
////////////////

/*
 * Override QGraphicsObject::boundingRect()
 *
 * Bounding rect is the rectangle used for painting. This means that we need to
 * include the stroke and any potential shadows; this is not just the center
 * area.
 *
 * The actual collisionBounds rectangle is smaller than this boundingRect, as it
 * ignores the stroke or other extra spacing required.
 */
QRectF Node::boundingRect() const
{
    QPointF ul(upperLeftPt.x() - STROKE_ADJ,
               upperLeftPt.y() - STROKE_ADJ);
    QPointF br(upperLeftPt.x() - STROKE_ADJ,
               upperLeftPt.y() - STROKE_ADJ);

    return QRectF(ul, br);
}

/*
 * Override QGraphicsObject::shape()
 *
 * This is the path used internally by the Qt framework for its collision tests.
 * Since comparing paths for collision may be more expensive than the very
 * efficient comparing of rectangles, we'll mostly ignore this function as it
 * just wraps the collisionBounds rectangle inside a path.
 *
 * If we need to compare collision of this Node with other Nodes, just use one
 * of the rectangle comparison functions with the actual collisionBounds of each
 * Node. This way we can ensure that the calculation is efficient instead of
 * just hoping that it is.
 */
QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addRect(collisionBounds);
    return path;
}

/*
 * Returns a copy of the collisionBounds rectangle
 */
QRectF Node::getCollisionRect() const
{
    return QRectF(collisionBounds);
}

/*
 * Override QGraphicsObject::paint
 *
 * Paint the Node itself.
 */
void Node::paint(QPainter* painter,
                 const QStyleOptionGraphicsItem* option,
                 QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QRectF rect;

    if ( isCut() )
        rect = QRectF(boundingRect().topLeft(), boundingRect().bottomRight());

    if (mouseDown)
        painter->setBrush(QBrush(gradClicked));
    else if (highlighted)
        painter->setBrush(QBrush(gradHighlighted));
    else
        painter->setBrush(QBrush(gradDefault));

    painter->drawRoundedRect(rect, 5, 5);
}

//////////////
/// Moving ///
//////////////

/*
 * Override QGraphicsObject::itemChange
 *
 * Control the behavior of the default item movement supplied by the
 * ItemIsMovable flag.
 */
QVariant Node::itemChange(GraphicsItemChange change,
                          const QVariant &value)
{
    if ( change == ItemPositionChange && scene() )
    {
        QPointF snapped = snapPoint(value.toPointF());
        qreal delX = snapped.x() - pos().x();
        qreal delY = snapped.y() - pos().y();

        if ( delX == 0 && delY == 0 )
            return snapped;

        // check for collision
        return snapped;
    }

}

/////////////
/// Mouse ///
/////////////

/*
 * Override QGraphicsObject::hoverMoveEvent()
 *
 * Stores the mouse position as it moves around inside this Node. This is needed
 * as a workaround for difficulty mapping a global mouse position to a point
 * relative to this coordinate system.
 *
 * TODO: figure out a way to convert QCursor::mousePos() into local coords in a
 * way that doesn't have bugs, so that we no longer need this function to update
 * it every single time the mouse moves. That way, we can just poll it once and
 * save cycles spent storing the mousePos from the event.
 */
void Node::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    lastHoverPos = event->pos();

    QGraphicsObject::hoverMoveEvent(event);
}

/*
 * Override QGraphicsObject::mousePressEvent()
 */
void Node::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    mouseOffset = event->pos();
    shadow.setEnabled(true);
    mouseDown = true;
    update();

    QGraphicsObject::mousePressEvent(event);
}

/*
 * Override QGraphicsObject::mouseReleaseEvent()
 */
void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    mouseDown = false;
    shadow.setEnabled(false);
    update();

    QGraphicsObject::mouseReleaseEvent(event);
}

/*
 * Override QGraphicsObject::hoverEnterEvent()
 */
void Node::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    canvas->setHighlight(this);
    QGraphicsObject::hoverEnterEvent(event);
}

/*
 * Override QGraphicsObject::hoverLeaveEvent()
 */
void Node::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    canvas->setHighlight(parent);
    QGraphicsObject::hoverLeaveEvent(event);
}

///////////////
/// Helpers ///
///////////////

/*
 * Return the closest point on the snapping grid
 */
QPointF snapPoint(const QPointF &pt)
{
    int x = pt.x() - (GRID_SPACING / 2);
    int y = pt.y() - (GRID_SPACING / 2);

    // Workaround for negative points
    bool negX = false;
    bool negY = false;

    if (x < 0)
    {
        negX = true;
        x = -x;
    }
    if (y < 0)
    {
        negY = true;
        y = -y;
    }

    // Perform the snap
    x = ((x + GRID_SPACING / 2) / GRID_SPACING) * GRID_SPACING;
    y = ((y + GRID_SPACING / 2) / GRID_SPACING) * GRID_SPACING;

    // Revert negation
    if (negX)
        x = -x;
    if (negY)
        y = -y;

    return QPointF(x, y);
}

/*
 * Return the distance between two points
 */
qreal dist(const QPointF &a, const QPointF &b)
{
    return qSqrt( qPow( ( a.x() - b.x() ), 2) +
                  qPow( ( a.y() - b.y() ), 2) );
}

/*
 * Returns the point in the list closest to the target point. If the list is
 * empty, this function just returns the target point.
 */
QPointF closest(const QList<QPointF> &list, const QPointF &target)
{

}

/*
 * Efficient way to determine if two rectangles collide. These rectangles must
 * be given points in the same coordinate system.
 */
bool rectsCollide(const QRectF &a, const QRectF &b)
{

}

/*
 * Debug function to print out (to qDebug) some text and a point's (x,y) coords
 */
void printPt(const QString &s, const QPointF &pt)
{

}


/*
 * Similar to printPt, but for rectangles
 */
void printRect(const QString &s, const QRectF &r)
{

}















