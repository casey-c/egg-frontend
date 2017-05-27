#include "node.h"

#define GRID_SPACING 16

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
 * Thus, upperLeftPt is the absolute upper left pixel that needs to be drawn (in
 * this Node's relative coordinate system); similarly for the bottom right.
 *
 * upperLeftPt starts at (0,0); however, upon adding children to this node, they
 * may force this point anywhere arbitrarily. When this happens, upperLeftPt.x()
 * will be one GRID_SPACING to the left of the minX of all its children.
 *
 * bottomRightPt starts at (4*GRID_SPACING, 4*GRID_SPACING) for an empty cut.
 * Like the upperLeftPt, it too is dependent on the position of child Nodes.
 *
 * The actual collisionBounds rectangle is smaller than this boundingRect, as it
 * ignores the stroke or other extra spacing required.
 */
QRectF Node::boundingRect() const
{
    return QRectF(upperLeftPt, bottomRightPt);
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
QVariant itemChange(GraphicsItemChange change,
                    const QVariant &value)
{

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

}

/*
 * Override QGraphicsObject::mousePressEvent()
 */
void Node::mousePressEvent(QGraphicsSceneMouseEvent* event)
{

}

/*
 * Override QGraphicsObject::mouseReleaseEvent()
 */
void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{

}

/*
 * Override QGraphicsObject::hoverEnterEvent()
 */
void Node::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{

}

/*
 * Override QGraphicsObject::hoverLeaveEvent()
 */
void Node::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{

}

///////////////
/// Helpers ///
///////////////

/*
 * Return the closest point on the snapping grid
 */
QPointF snapPoint(const QPointF &pt)
{

}

/*
 * Return the distance between two points
 */
qreal dist(const QPointF &a, const QPointF &b)
{

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















