#include "canvas.h"
#include "node.h"

#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QtCore/QtMath>
#include <QDebug>

#define DEBUG_BOUNDS 1 // Make it 1 to enable B key to show bounds on move

#define GRID_SPACING 16
#define STROKE_ADJ 2

#define EMPTY_CUT_SIZE (4 * GRID_SPACING)

#define COLLISION_OFFSET (GRID_SPACING / 2) + 1

// Forward declarations for helper functions (implementation located at end)
QPointF snapPoint(const QPointF &pt);
qreal dist(const QPointF &a, const QPointF &b);
bool rectsCollide(const QRectF &a, const QRectF &b);
void printPt(const QString &s, const QPointF &pt);
void printRect(const QString &s, const QRectF &r);
void printMinMax(qreal minX, qreal minY, qreal maxX, qreal maxY);

// Helper struct
struct NodePotential
{
    Node* node;
    QRectF rect;
};

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
    minX(0),
    minY(0),
    maxX(0),
    maxY(0),
    width(0),
    height(0)
{
    // Drop shadow on click and drag
    shadow = new QGraphicsDropShadowEffect(this);
    shadow->setEnabled(false);
    shadow->setBlurRadius(2);
    shadow->setOffset(2);
    this->setGraphicsEffect(shadow);

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
        setPos(snapPoint(pt));
    }

    // Colors
    gradDefault = QRadialGradient( drawBox.x() + 3,
                                   drawBox.y() + 3,
                                   (dist(drawBox.topLeft(), drawBox.bottomRight()) * 2 ));
    gradDefault.setColorAt(0, QColor(225, 225, 225));
    gradDefault.setColorAt(1, QColor(185, 185, 185));

    gradHighlighted = QRadialGradient( drawBox.x() + 3,
                                       drawBox.y() + 3,
                                       (dist(drawBox.topLeft(), drawBox.bottomRight()) * 2 ));
    gradHighlighted.setColorAt(0, QColor(240, 240, 240));
    gradHighlighted.setColorAt(1, QColor(210, 210, 210));

    gradClicked = QRadialGradient( drawBox.x() + 3,
                                   drawBox.y() + 3,
                                   (dist(drawBox.topLeft(), drawBox.bottomRight()) * 2 ));
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

    updateChildMinMax();

    return newChild;
}

void Node::updateChildMinMax()
{
    if ( isRoot() )
        return;

    qDebug() << "pre";
    printMinMax(minX, minY, maxX, maxY);

    //canvas->clearBounds();

    // Reset
    minX = minY = 0;
    maxX = maxY = EMPTY_CUT_SIZE;

    for (Node* child : children)
    {
        QPointF tl, br;
        if (child->hasDifferentPotentialBounds.checkAndClear())
        {
            tl = mapFromScene(child->scenePotentialBounds.topLeft());
            br = mapFromScene(child->scenePotentialBounds.bottomRight());
        }
        else
        {
            tl = mapFromScene(child->getSceneCollisionBox().topLeft());
            br = mapFromScene(child->getSceneCollisionBox().bottomRight());
        }

        if (tl.x() < minX)
            minX = tl.x();
        if (tl.y() < minY)
            minY = tl.y();
        if (br.x() > maxX)
            maxX = br.x();
        if (br.y() > maxY)
            maxY = br.y();
    }

    qDebug() << "post";
    printMinMax(minX, minY, maxX, maxY);

    QRectF childBox(mapToScene(snapPoint(QPointF(minX, minY))),
                    mapToScene(snapPoint(QPointF(maxX + GRID_SPACING,
                                                 maxY + GRID_SPACING))));

    bool collidesWithSibling = false;
    for (Node* sibling : parent->children)
    {
        if (sibling == this)
            continue;

        if (rectsCollide(childBox, sibling->getSceneCollisionBox()))
        {
            //canvas->addBlueBound(sibling->getSceneCollisionBox());
            collidesWithSibling = true;
            break;
        }
    }
    if (!collidesWithSibling)
    {
        //canvas->addBlackBound(childBox);

        // TODO: make it only update on valid
    }
    //else
        //canvas->addRedBound(childBox);

    // Ideally these should be checked for validity first, but for now
    // just update it regardless of collision
    QRectF potential(mapFromScene(childBox.topLeft()),
                     mapFromScene(childBox.bottomRight()));
    canvas->addRedBound(childBox);
    setDrawBoxFromPotential(potential);

    // Percolate up
    parent->updateChildMinMax();
}

void Node::setDrawBoxFromPotential(QRectF potential)
{
    prepareGeometryChange();

    drawBox = potential;

}

// Assumes the quantum bool is set
//QRectF Node::convertTempCollisionToDrawBox()
//{
    //QPointF tl = potentialBounds.topLeft();
    //QPointF br = potentialBounds.bottomRight();
//
    //tl.setX(tl.x() + COLLISION_OFFSET);
    //tl.setY(tl.y() + COLLISION_OFFSET);
    //br.setX(br.x() - COLLISION_OFFSET);
    //br.setY(br.y() - COLLISION_OFFSET);

    //return QRectF(tl, br);
//}

#if 0
void Node::calculateChildBox()
{
    if (children.empty())
        return;

    Node* first = children.first();
    QPointF firstTL, firstBR;
    if (first->hasDifferentPotentialBounds.check())
    {
        QRectF rect = first->convertTempCollisionToDrawBox();
        firstTL = first->mapToParent(rect.topLeft());
        firstBR = first->mapToParent(rect.bottomRight());
    }
    else
    {
        firstTL = first->mapToParent(first->drawBox.topLeft());
        firstBR = first->mapToParent(first->drawBox.bottomRight());
    }

    minX = firstTL.x();
    minY = firstTL.y();
    maxX = firstBR.x();
    maxY = firstBR.y();

    for (Node* child : children)
    {
        QPointF childTL, childBR;

        if (child->hasDifferentPotentialBounds.check())
        {
            QRectF rect = child->convertTempCollisionToDrawBox();
            childTL = child->mapToParent(rect.topLeft());
            childBR = child->mapToParent(rect.bottomRight());
        }
        else
        {
            childTL = child->mapToParent(child->drawBox.topLeft());
            childBR = child->mapToParent(child->drawBox.bottomRight());
        }

        if (childTL.x() < minX)
            minX = childTL.x();
        if (childTL.y() < minY)
            minY = childTL.y();
        if (childBR.x() > maxX)
            maxX = childBR.x();
        if (childBR.y() > maxY)
            maxY = childBR.y();
    }

#if 1 // kind of an interesting placeholder technique
    if (minX > 0)
        minX = 0;
    if (minY > 0)
        minY = 0;
    if (maxX < EMPTY_CUT_SIZE)
        maxX = EMPTY_CUT_SIZE;
    if (maxY < EMPTY_CUT_SIZE)
        maxY = EMPTY_CUT_SIZE;
#endif

    childBox = QRectF(QPointF(minX, minY),
                      QPointF(maxX, maxY));
    printRect("childBox", childBox);
}
#endif

// Assumes calculateChildBox() was called and updated
// successfully
#if 0
void Node::resizeToFitChildBox()
{
    QPointF tl = childBox.topLeft();
    QPointF br = childBox.bottomRight();

#if 1
    tl.setX(tl.x() - GRID_SPACING);
    tl.setY(tl.y() - GRID_SPACING);
    br.setX(br.x() + GRID_SPACING);
    br.setY(br.y() + GRID_SPACING);
#endif

    prepareGeometryChange();
    drawBox = QRectF(tl, br);
    updateCollisionBox();
    update();
}
#endif

//QRectF Node::getChildBoxInScene() const
//{
    //QPointF tl = mapToScene(childBox.topLeft());
    //QPointF br = mapToScene(childBox.bottomRight());

    //return QRectF(tl, br);
//}

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
    path.addRect( getDrawAsCollision(drawBox) );
    return path;
}

/*
 * Returns the collisionBox mapped to scene
 */
//QRectF Node::getCollisionRect() const
//{
    //return QRectF(mapToScene(collisionBox.topLeft()),
                  //mapToScene(collisionBox.bottomRight()));
//}

/*
 * sets the collisionBox based off a change in the drawBox
 * collision box is in local coords
 *
 * NOTE: call this function every time you adjust the drawBox!
 */
//void Node::updateCollisionBox()
//{
    //collisionBox = getDrawAsCollision(drawBox);
    //collisionBox = QRectF(QPointF(drawBox.topLeft().x() - COLLISION_OFFSET,
                                  //drawBox.topLeft().y() - COLLISION_OFFSET),
                          //QPointF(drawBox.bottomRight().x() + COLLISION_OFFSET,
                                  //drawBox.bottomRight().y() + COLLISION_OFFSET));
//}

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
        //printRect("paint rect", rect);
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
        return collisionLessPoint(value.toPointF());

    return QGraphicsItem::itemChange(change, value);
}

/*
 * Returns a grid-aligned point close to val such that if the Node were to move
 * there, no collision would occur. In other words, take the point val and try
 * and find a snapped point that wouldn't cause a collision.
 *
 * This function uses a "bloom" technique that searches an extra four snapped
 * points in the cardinal directions away from the snapped node. This allows for
 * fuzzier detection of where the user wanted to move, making it less
 * frustrating to move a node near other potentially colliding nodes.
 *
 * If no point is found, pos() is returned, as it will result in no movement
 * whatsoever.
 */
QPointF Node::collisionLessPoint(QPointF val)
{
    // Put val onto the snapping grid
    QPointF snapped = snapPoint(val);

    // No movement, so no need to check updated collision
    if ( snapped.x() - pos().x() == 0 &&
         snapped.y() - pos().y() == 0 )
        return pos();

    // Build up a "bloom" of potential points to check
    QList<QPointF> potentialPts;
    potentialPts.append(QPointF(snapped.x() - GRID_SPACING, snapped.y()));
    potentialPts.append(QPointF(snapped.x(), snapped.y() - GRID_SPACING));
    potentialPts.append(QPointF(snapped.x() + GRID_SPACING, snapped.y()));
    potentialPts.append(QPointF(snapped.x(), snapped.y() + GRID_SPACING));

    // Sort by distance, so we search for the closest first
    std::sort(potentialPts.begin(), potentialPts.end(),
          [this](const QPointF a, const QPointF b) ->
          bool { return dist(a, this->pos()) < dist(b, this->pos()); });

    // Always check the target before any bloomed points
    potentialPts.prepend(snapped);

    // Check all these points to see if they would cause a collision
    for ( QPointF pt : potentialPts )
    {
        QRectF rect = getSceneCollisionBox(pt.x() - pos().x(),
                                           pt.y() - pos().y());

        // No collision with direct siblings
        if ( rectAvoidsCollision(rect) )
        {
            // TODO: percolate up
            hasDifferentPotentialBounds.set();
            scenePotentialBounds = rect;

            canvas->clearBounds();

            bool avoidedCollision = true;

            QList<NodePotential*> nodesToUpdate;

            Node* p = parent;
            Node* c = this;
            QRectF cPotRect = rect;

            while (!p->isRoot())
            {
                // Parent potential rect is in scene coords
                QRectF pPotRect = c->genParentPotential(cPotRect);

                // Check if parent potential rect collides with anything
                for (Node* n : p->parent->children)
                {
                    if (n == p)
                        continue;
                    if (rectsCollide(pPotRect, n->getSceneCollisionBox()))
                    {
                        avoidedCollision = false;
                        break;
                    }
                }

                if (!avoidedCollision)
                    break;

                // No collisions at this level, so store this info to update
                // later if the percolation succeeds
                NodePotential* np = new NodePotential;
                np->node = p;
                np->rect = QRectF(p->mapFromScene(pPotRect.topLeft()),
                                  p->mapFromScene(pPotRect.bottomRight()));
                nodesToUpdate.append(np);

                // Percolate up
                c = c->parent;
                p = p->parent;
                cPotRect = pPotRect;
            }

            // A collision occured up in the percolation: clean up allocation
            if (!avoidedCollision)
            {
                for (NodePotential* np : nodesToUpdate)
                    delete np;
                continue; // check the next potential pt
            }

            // Everything's ok: update the drawBoxes to the saved potentials
            for (NodePotential* np : nodesToUpdate)
            {
                np->node->setDrawBoxFromPotential(np->rect);
                delete np;
            }

            // And perform the move
            return pt;
        }
    }

    // None of those points avoided collision
    return pos();
}

/*
 * Returns (in scene coords) a potential bounds for my parent based on my
 * altered drawbox. I.e. calculate and return a potential drawBox for my
 * direct parent.
 *
 * myPotential is in scene coords
 */
QRectF Node::genParentPotential(QRectF myPotential)
{
    qreal mix, miy, max, may; // min max
    mix = miy = 0;
    max = may = EMPTY_CUT_SIZE;
    QPointF tl, br;


    //canvas->clearBounds();
    for (Node* sibling : parent->children)
    {
        if (sibling == this)
        {
            // Use my potential instead
            tl = parent->mapFromScene(myPotential.topLeft());
            br = parent->mapFromScene(myPotential.bottomRight());
            //canvas->addBlueBound(myPotential);
            //printPt("(potential)tl", tl);
            //printPt("(potential)br", br);
        }
        else
        {
            // Use the sibling's actual scene collision bounds
            tl = parent->mapFromScene(sibling->getSceneCollisionBox().topLeft());
            br = parent->mapFromScene(sibling->getSceneCollisionBox().bottomRight());
            //printPt("tl", tl);
            //printPt("br", br);
        }

        if (tl.x() < mix)
            mix = tl.x();
        if (tl.y() < miy)
            miy = tl.y();
        if (br.x() > max)
            max = br.x();
        if (br.y() > may)
            may = br.y();

        // Debug
        //qDebug() << "min:" << mix << "," << miy;
        //qDebug() << "max:" << max << "," << may;
    }

    // Calculated points are in parent coords
    QPointF tlp = QPointF(mix, miy);
    QPointF brp = QPointF(max + GRID_SPACING,
                          may + GRID_SPACING);

    // Convert back to scene
    QPointF tls = parent->mapToScene(snapPoint(tlp));
    QPointF brs = parent->mapToScene(snapPoint(brp));

    QRectF rect(tls, brs);
    //canvas->addRedBound(rect);
    return rect;
}

/*
 * Check if a given potential collision box rect would collide with any direct
 * siblings.
 */
bool Node::rectAvoidsCollision(QRectF rect) const
{
    if ( isRoot() )
        return true;

#if DEBUG_BOUNDS
    canvas->clearBounds();
    canvas->addBlueBound(rect);
    bool ret = true;
#endif

    for (Node* sibling : parent->children)
    {
        if (sibling == this)
            continue;

        QRectF sibBox = sibling->getSceneCollisionBox();

        if (rectsCollide(rect, sibBox) )
        {
#if DEBUG_BOUNDS
            canvas->addRedBound(sibBox);
            ret = false;
#else
            return false;
#endif
        }
#if DEBUG_BOUNDS
        else
        {
            canvas->addBlackBound(sibBox);
        }
#endif
    }
#if DEBUG_BOUNDS
    return ret;
#else
    return true;
#endif
}

/*
 * returns the collision rect in scene coords, offset by the given deltas
 */
//QRectF Node::getTranslatedSceneCollisionRect(qreal deltaX, qreal deltaY) const
//{
    //QRectF rect(mapToScene(drawBox.topLeft()), mapToScene(drawBox.bottomRight()));
    //QRectF rect = getCollisionRect(); // already in scene coords
    //rect.translate(deltaX, deltaY);
    //return rect;
//}

QRectF Node::getSceneCollisionBox(qreal deltaX, qreal deltaY) const
{
    int w = drawBox.width();
    int h = drawBox.height();

    return QRectF( mapToScene(QPointF(drawBox.x() - COLLISION_OFFSET + deltaX,
                           drawBox.y() - COLLISION_OFFSET + deltaY)),
                   mapToScene(QPointF(drawBox.x() + w + COLLISION_OFFSET + deltaX,
                           drawBox.y() + h + COLLISION_OFFSET + deltaY)));

    //return QRectF( QPointF(scenePos().x() - COLLISION_OFFSET + deltaX,
                           //scenePos().y() - COLLISION_OFFSET + deltaY),
                   //QPointF(scenePos().x() + w + COLLISION_OFFSET + deltaX,
                           //scenePos().y() + h + COLLISION_OFFSET + deltaY));

     //return QRectF( mapToScene(QPointF(mp.x() - COLLISION_OFFSET,
                                      //mp.y() - COLLISION_OFFSET)),
                   //mapToScene(QPointF(mp.x() + w + COLLISION_OFFSET,
                                      //mp.y() + h + COLLISION_OFFSET)));
}

//QRectF Node::getPotentialSceneCollision(qreal dx, qreal dy) const
//{
    //return rectToScene(getDrawAsCollision(getTranslatedDrawBox(dx, dy)));

//}

/*
 * Returns a potential draw box if it were translated by the given deltas
 */
//QRectF Node::getTranslatedDrawBox(qreal deltaX, qreal deltaY) const
//{
    //QRectF rect(drawBox.topLeft(), drawBox.bottomRight());
    //rect.translate(deltaX, deltaY);
    //return rect;
//}

/*
 * Converts the drawable rectangle given in params into an actual collisionBox
 */
QRectF Node::getDrawAsCollision(const QRectF &draw) const
{
    return QRectF(QPointF(draw.topLeft().x() - COLLISION_OFFSET,
                          draw.topLeft().y() - COLLISION_OFFSET),
                  QPointF(draw.bottomRight().x() + COLLISION_OFFSET,
                          draw.bottomRight().y() + COLLISION_OFFSET));
}

/*
 * Helper to convert a rectangle to scene coords; this is because the built in
 * Qt function (mapToScene) that takes in a rectangle spits out a polygon. We
 * want to keep it as a rectangle instead.
 */
//QRectF Node::rectToScene(const QRectF &rect) const
//{
    //return QRectF(mapToScene(rect.topLeft()),
                  //mapToScene(rect.bottomRight()));
//}

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
    shadow->setEnabled(true);
    mouseDown = true;
    update();

    if (event->buttons() & Qt::RightButton)
    {
        qDebug() << "Right click";
        canvas->clearBounds();
        canvas->addBlackBound(this->getSceneCollisionBox());
    }

    QGraphicsObject::mousePressEvent(event);
}

/*
 * Override QGraphicsObject::mouseReleaseEvent()
 */
void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    mouseDown = false;
    shadow->setEnabled(false);
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
 * Efficient way to determine if two rectangles collide. These rectangles must
 * be given points in the same coordinate system.
 */
bool rectsCollide(const QRectF &a, const QRectF &b)
{
    //printRect("checking collision for a", a);
    //printRect("against b", b);

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

void printMinMax(qreal minX, qreal minY, qreal maxX, qreal maxY)
{
    qDebug() << "min: ("
             << minX
             << ","
             << minY
             << ") | max: ("
             << maxX
             << ","
             << maxY;
}















