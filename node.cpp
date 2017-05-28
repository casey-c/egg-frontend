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

#define EMPTY_CUT_SIZE (4 * GRID_SPACING)

#define COLLISION_OFFSET (GRID_SPACING / 2) + 1

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
    highlighted(false),
    mouseDown(false),
    lastHoverPos(0, 0),
    mouseOffset(0, 0),
    width(0),
    height(0),
    deltaPosShift(0, 0),
    potentialPosShift(0, 0),
    hasDifferentPotentialBounds(false)
{


    QPointF snapped = snapPoint(pt);
    //shadow.setEnabled(true);

    if ( isRoot() )
    {

    }
    else if ( isCut() )
    {
        qDebug() << "Adding cut";
        setFlag(ItemIsMovable);
        setFlag(ItemSendsGeometryChanges);
        setCacheMode(DeviceCoordinateCache);
        setAcceptHoverEvents(true);

        width = height = EMPTY_CUT_SIZE;
        drawBox = QRectF( QPointF(0, 0), QPointF(width, height) );
        updateCollisionBox();

        setPos(snapped);
    }

    // Colors
    gradDefault = QRadialGradient( snapped.x() - 3,
                                   snapped.y() - 3,
                                   (dist(drawBox.topLeft(), drawBox.bottomRight()) / 4 ));
    gradDefault.setColorAt(0, QColor(255, 255, 255));
    gradDefault.setColorAt(1, QColor(185, 185, 185));

    gradHighlighted = QRadialGradient( snapped.x() - 3,
                                       snapped.y() - 3,
                                       (dist(drawBox.topLeft(), drawBox.bottomRight()) / 4 ));
    gradHighlighted.setColorAt(0, QColor(240, 240, 240));
    gradHighlighted.setColorAt(1, QColor(210, 210, 210));

    gradClicked = QRadialGradient( snapped.x() - 3,
                                   snapped.y() - 3,
                                   (dist(drawBox.topLeft(), drawBox.bottomRight()) / 4 ));
    gradClicked.setColorAt(0, QColor(210, 210, 210));
    gradClicked.setColorAt(1, QColor(240, 240, 240));
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
    return drawBox;
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
    path.addRect(collisionBox);
    return path;
}

/*
 * Returns the collisionBox mapped to scene
 */
QRectF Node::getCollisionRect() const
{
    return QRectF(mapToScene(collisionBox.topLeft()),
                  mapToScene(collisionBox.bottomRight()));
}

/*
 * sets the collisionBox based off a change in the drawBox
 * collision box is in local coords
 */
void Node::updateCollisionBox()
{
    collisionBox = QRectF(QPointF(drawBox.topLeft().x() - COLLISION_OFFSET,
                                  drawBox.topLeft().y() - COLLISION_OFFSET),
                          QPointF(drawBox.bottomRight().x() + COLLISION_OFFSET,
                                  drawBox.bottomRight().y() + COLLISION_OFFSET));
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
    {
        rect = QRectF(drawBox.topLeft(), drawBox.bottomRight());
        printRect("paint rect", rect);
    }

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
        qDebug() << "ItemPositionChange";
        QPointF snapped = snapPoint(value.toPointF());
        printPt("snapped", snapped);
        qreal delX = snapped.x() - pos().x();
        qreal delY = snapped.y() - pos().y();

        if ( delX == 0 && delY == 0 )
            return snapped;

        printPt("delta", QPointF(delX, delY));

        QRectF potentialRect = getCollisionRect();
        printRect("pr-pre:",potentialRect);
        potentialRect.translate(delX, delY);
        printRect("pr-post:",potentialRect);

        // check for collision
        for (Node* sibling : parent->children)
        {
            if (sibling == this)
                continue;

            if (rectsCollide(sibling->getCollisionRect(), potentialRect ))
            {
                qDebug() << "collision!";
                return pos();
            }
        }
        return snapped;
    }

    return QGraphicsItem::itemChange(change, value);
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
    printRect("checking collision for a", a);
    printRect("against b", b);

    qreal ax1, ax2, ay1, ay2;
    a.getCoords(&ax1, &ay1, &ax2, &ay2);

    qreal bx1, bx2, by1, by2;
    b.getCoords(&bx1, &by1, &bx2, &by2);

    return ( ax1 < bx2 &&
             ax2 > bx1 &&
             ay1 < by2 &&
             ay2 > by1 );
}

/*
 * Debug function to print out (to qDebug) some text and a point's (x,y) coords
 */
void printPt(const QString &s, const QPointF &pt)
{
    qDebug() << s
             << pt.x()
             << ","
             << pt.y();
}


/*
 * Similar to printPt, but for rectangles
 */
void printRect(const QString &s, const QRectF &r)
{
    qDebug() << s
             << r.topLeft().x()
             << ","
             << r.topLeft().y()
             << ":"
             << r.bottomRight().x()
             << ","
             << r.bottomRight().y();
}















