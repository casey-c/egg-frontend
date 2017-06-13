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
#define COLLISION_OFFSET (GRID_SPACING / 2) - 1

#define EMPTY_CUT_SIZE (4 * GRID_SPACING)
#define STATEMENT_SIZE (2 * GRID_SPACING)
#define BIG_NUMBER 999999999

#define BORDER_RADIUS 10

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
        //setFlag(ItemIsMovable);
        setFlag(ItemSendsGeometryChanges);
        setCacheMode(DeviceCoordinateCache);
        setAcceptHoverEvents(true);

        width = height = qreal(EMPTY_CUT_SIZE);
        drawBox = QRectF( QPointF(0, 0), QPointF(width, height) );
        setPos(snapPoint(pt));
    }

    // Colors
    gradDefault = QRadialGradient( drawBox.x() + 3,
                                   drawBox.y() + 3,
                                   (dist(drawBox.topLeft(), drawBox.bottomRight()) * 2 ));
    //gradDefault.setColorAt(0, QColor(225, 225, 225));
    //gradDefault.setColorAt(1, QColor(185, 185, 185));
    gradDefault.setColorAt(0, QColor(249, 249, 249));
    gradDefault.setColorAt(1, QColor(249, 249, 249));

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

/* Statement constructor */
Node::Node(Canvas* can, Node* par, QString s, QPointF pt) :
    canvas(can),
    parent(par),
    type(Statement),
    highlighted(false),
    mouseDown(false),
    lastHoverPos(0, 0),
    mouseOffset(0, 0),
    width(0),
    height(0),
    letter(s)
{
    shadow = new QGraphicsDropShadowEffect(this);
    shadow->setEnabled(false);
    shadow->setBlurRadius(2);
    shadow->setOffset(2);
    this->setGraphicsEffect(shadow);

    //setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);

    width = height = qreal(STATEMENT_SIZE);
    drawBox = QRectF( QPointF(0, 0), QPointF(width, height));
    setPos(snapPoint(pt));

    gradDefault = QRadialGradient( drawBox.x() + 3,
                                   drawBox.y() + 3,
                                   (dist(drawBox.topLeft(), drawBox.bottomRight()) * 2 ));
    gradDefault.setColorAt(0, QColor(0, 0, 0, 0));
    gradDefault.setColorAt(1, QColor(0, 0, 0, 0));

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

    font = QFont();
    font.setPixelSize(GRID_SPACING * 2 - 6);
}

Node* Node::addChildCut(QPointF pt)
{
    Node* newChild = new Node(canvas, this, Cut, mapFromScene(pt));
    children.append(newChild);
    newChild->setParentItem(this);

    return newChild;
}

Node* Node::addChildStatement(QPointF pt, QString t)
{
    Node* newChild = new Node(canvas, this, t, mapFromScene(pt));
    children.append(newChild);
    newChild->setParentItem(this);

    return newChild;
}

void Node::setDrawBoxFromPotential(QRectF potential)
{
    prepareGeometryChange();

    QRectF sceneDraw = sceneCollisionToSceneDraw(potential);
    drawBox = QRectF(mapFromScene(sceneDraw.topLeft()),
                     mapFromScene(sceneDraw.bottomRight()));
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

QRectF Node::boundingRect() const
{
    return drawBox;
}

QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addRect( getDrawAsCollision(drawBox) );
    return path;
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

    if (isRoot())
        return;

    if (isStatement())
        painter->setPen(QPen(QColor(0,0,0,0)));

    if (mouseDown)
        painter->setBrush(QBrush(gradClicked));
    else if (highlighted)
        painter->setBrush(QBrush(gradHighlighted));
    else
        painter->setBrush(QBrush(gradDefault));

    painter->drawRoundedRect(drawBox, qreal(BORDER_RADIUS), qreal(BORDER_RADIUS));

    if ( isStatement() )
    {
        painter->setPen(QPen(QColor(0,0,0,255)));
        painter->setFont(font);
        painter->drawText( drawBox, Qt::AlignCenter, letter );
    }
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
        //return collisionLessPoint(value.toPointF());
        qDebug() << "successful move";

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
    potentialPts.append(QPointF(snapped.x() - qreal(GRID_SPACING), snapped.y()));
    potentialPts.append(QPointF(snapped.x(), snapped.y() - qreal(GRID_SPACING)));
    potentialPts.append(QPointF(snapped.x() + qreal(GRID_SPACING), snapped.y()));
    potentialPts.append(QPointF(snapped.x(), snapped.y() + qreal(GRID_SPACING)));

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
                np->rect = pPotRect;
                //np->rect = QRectF(p->mapFromScene(pPotRect.topLeft()),
                                  //p->mapFromScene(pPotRect.bottomRight()));
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

QRectF Node::genParentPotential(QRectF myPotential)
{
    qreal mix, miy, max, may; // min max
    mix = miy = BIG_NUMBER;
    max = may = -BIG_NUMBER;

    QPointF tl, br;
    for (Node* sibling : parent->children)
    {
        if (sibling == this)
        {
            // Convert the given argument to a drawBox
            QRectF potDraw = sceneCollisionToSceneDraw(myPotential);
            tl = parent->mapFromScene(potDraw.topLeft());
            br = parent->mapFromScene(potDraw.bottomRight());
            canvas->addBlueBound(potDraw);
        }
        else
        {
            // Use the existing drawBox
            QRectF sceneDraw = sceneCollisionToSceneDraw(sibling->getSceneCollisionBox());
            tl = parent->mapFromScene(sceneDraw.topLeft());
            br = parent->mapFromScene(sceneDraw.bottomRight());
            canvas->addGreenBound(sceneDraw);
        }

        if (tl.x() < mix)
            mix = tl.x();
        if (tl.y() < miy)
            miy = tl.y();
        if (br.x() > max)
            max = br.x();
        if (br.y() > may)
            may = br.y();
    }

    // Absolutely disgusting placeholder logic
    /*
    if (parent->children.size() == 1)
    {
        qreal cw = max - mix;
        qreal ch = may - miy;

        if (cw > max)
            max = cw;
        if (ch > may)
            may = ch;
        if (mix > 0)
            mix = 0;
        if (miy > 0)
            miy = 0;
    }
    */

    // Calculated points are in parent coords
    //printMinMax(mix, miy, max, may);
    QPointF tlp = QPointF(mix - qreal(GRID_SPACING),
                          miy - qreal(GRID_SPACING));
    QPointF brp = QPointF(max + qreal(GRID_SPACING),
                          may + qreal(GRID_SPACING));

    // Convert back to scene
    QPointF tls = parent->mapToScene(tlp);
    QPointF brs = parent->mapToScene(brp);

    QRectF rect(tls, brs);
    canvas->addBlackBound(rect);
    canvas->addRedBound(getDrawAsCollision(rect));
    return getDrawAsCollision(rect);
}

bool Node::rectAvoidsCollision(QRectF rect) const
{
    if ( isRoot() )
        return true;

    canvas->clearBounds();
    canvas->addBlueBound(rect);

    for (Node* sibling : parent->children)
    {
        if (sibling == this)
            continue;

        QRectF sibBox = sibling->getSceneCollisionBox();

        if (rectsCollide(rect, sibBox) )
        {
            canvas->addRedBound(sibBox);
            return false;
        }
        else
        {
            canvas->addBlackBound(sibBox);
        }
    }
    return true;
}

QRectF Node::getSceneCollisionBox(qreal deltaX, qreal deltaY) const
{
    QRectF my = getDrawAsCollision(drawBox);
    return QRectF(mapToScene( QPointF(my.topLeft().x() + deltaX,
                                      my.topLeft().y() + deltaY)),
                  mapToScene( QPointF(my.bottomRight().x() + deltaX,
                                      my.bottomRight().y() + deltaY )));
}

QRectF Node::getDrawAsCollision(const QRectF &draw) const
{
    return QRectF(QPointF(draw.topLeft().x() - qreal(COLLISION_OFFSET),
                          draw.topLeft().y() - qreal(COLLISION_OFFSET)),
                  QPointF(draw.bottomRight().x() + qreal(COLLISION_OFFSET),
                          draw.bottomRight().y() + qreal(COLLISION_OFFSET)));
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

void Node::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    qDebug() << "mouse press";
    mouseOffset = event->pos();
    shadow->setEnabled(true);
    mouseDown = true;
    update();
    qDebug() << "mouse press post update";

    if (event->buttons() & Qt::RightButton)
    {
        qDebug() << "Right click";
        canvas->clearBounds();
        canvas->addBlackBound(this->getSceneCollisionBox());

        QRectF dr( mapToScene(drawBox.topLeft()), mapToScene(drawBox.bottomRight()));
        canvas->addGreenBound(dr);

        qDebug() << "coll off" << COLLISION_OFFSET;
        printRect("dr", dr);
        printRect("coll", this->getSceneCollisionBox());

        canvas->addRedBound(sceneCollisionToSceneDraw(getSceneCollisionBox()));
    }

    //QGraphicsObject::mousePressEvent(event);
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    qDebug() << "Release mouse";
    mouseDown = false;
    shadow->setEnabled(false);
    update();

    QGraphicsObject::mouseReleaseEvent(event);
}

void Node::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    qDebug() << "Hover enter";
    canvas->setHighlight(this);
    QGraphicsObject::hoverEnterEvent(event);
}

void Node::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    qDebug() << "Hover leave";
    canvas->setHighlight(parent);
    QGraphicsObject::hoverLeaveEvent(event);
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (mouseDown)
    {
        canvas->clearDots();

        QPointF inMyCoords = event->pos();
        QPointF target = mapToParent(event->pos());

        QPointF scenePt = mapToScene(event->pos());
        printPt("scene", scenePt);
        //canvas->addBlackDot(scenePt);

        printPt("dragged to", target);


        printPt("myCoords", inMyCoords);
        printPt("mouseOffset", mouseOffset);

        // offset so we hit the actual pos()
        target.setX(target.x() - mouseOffset.x());
        target.setY(target.y() - mouseOffset.y());
        canvas->addBlackDot(parent->mapToScene(target));

        QPointF collisionLess = collisionLessPoint(target);

        if ( collisionLess.x() == pos().x() &&
             collisionLess.y() == pos().y()   )
        {
            qDebug() << "No movement";
            return;
        }

        printPt("original pos", pos());
        printPt("collisionLess", collisionLess);
        qreal deltaX = collisionLess.x() - pos().x() - mouseOffset.x();
        qreal deltaY = collisionLess.y() - pos().y() - mouseOffset.y();
        qDebug() << "deltaX" << deltaX;
        qDebug() << "deltaY" << deltaY;

        QRectF rect = getSceneCollisionBox(deltaX, deltaY);
        canvas->addRedBound(rect);

        //moveBy(deltaX, deltaY);
    }
}

/// New ///

// rect is a collision box in scene coords
// this function just reverses what the getSceneCollisionBox
// does to a given rectangle
QRectF Node::sceneCollisionToSceneDraw(QRectF rect)
{
    return QRectF( QPointF( rect.topLeft().x() + qreal(COLLISION_OFFSET),
                            rect.topLeft().y() + qreal(COLLISION_OFFSET)),
                   QPointF( rect.bottomRight().x() - qreal(COLLISION_OFFSET),
                            rect.bottomRight().y() - qreal(COLLISION_OFFSET)) );
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
 * be located
 */
bool rectsCollide(const QRectF &a, const QRectF &b)
{
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

/*
 * Another print debug, but for four min max qreals
 */
void printMinMax(qreal minX, qreal minY, qreal maxX, qreal maxY)
{
    qDebug() << "min: ("
             << minX
             << ","
             << minY
             << ") | max: ("
             << maxX
             << ","
             << maxY
             << ")";
}















