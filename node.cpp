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
QList<QPointF> constructBloom(QPointF scenePos, QPointF sceneTarget);

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
    mouseOffset(0, 0),
    selected(false)
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

        drawBox = QRectF( QPointF(0, 0), QPointF(qreal(EMPTY_CUT_SIZE),
                                                 qreal(EMPTY_CUT_SIZE)) );
        setPos(snapPoint(pt));
    }

    // Colors
    gradDefault = QRadialGradient( drawBox.x() + 3,
                                   drawBox.y() + 3,
                                   (dist(drawBox.topLeft(), drawBox.bottomRight()) * 2 ));
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

    gradSelected = QRadialGradient( drawBox.x() + 3,
                                    drawBox.y() + 3,
                                    (dist(drawBox.topLeft(), drawBox.bottomRight()) * 2 ));
    gradSelected.setColorAt(0, QColor(110, 226, 218));
    gradSelected.setColorAt(1, QColor(0, 209, 140));
}

/* Statement constructor */
Node::Node(Canvas* can, Node* par, QString s, QPointF pt) :
    canvas(can),
    parent(par),
    type(Statement),
    highlighted(false),
    mouseDown(false),
    mouseOffset(0, 0),
    letter(s),
    selected(false)
{
    shadow = new QGraphicsDropShadowEffect(this);
    shadow->setEnabled(false);
    shadow->setBlurRadius(2);
    shadow->setOffset(2);
    this->setGraphicsEffect(shadow);

    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);

    drawBox = QRectF( QPointF(0, 0), QPointF(qreal(STATEMENT_SIZE),
                                             qreal(STATEMENT_SIZE)));
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

    gradSelected = QRadialGradient( drawBox.x() + 3,
                                    drawBox.y() + 3,
                                    (dist(drawBox.topLeft(), drawBox.bottomRight()) * 2 ));
    gradSelected.setColorAt(0, QColor(110, 226, 218));
    gradSelected.setColorAt(1, QColor(0, 209, 140));

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

/////////////////
/// Selection ///
/////////////////

/*
 * Select this node
 */
void Node::selectThis()
{
    selected = true;
    update();
}

/*
 * Deselect this node
 */
void Node::deselectThis()
{
    selected = false;
    update();
}

/*
 * Inverts whether this is selected or not
 */
void Node::toggleSelection()
{
    if (selected)
        canvas->deselectNode(this);
    else
        canvas->selectNode(this);
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
    path.addRect(toCollision(drawBox));
    return path;
}

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

    if (selected)
        painter->setBrush(QBrush(gradSelected));
    else
    {
        if (mouseDown)
            painter->setBrush(QBrush(gradClicked));
        else if (highlighted)
            painter->setBrush(QBrush(gradHighlighted));
        else
            painter->setBrush(QBrush(gradDefault));
    }

    painter->drawRoundedRect(drawBox, qreal(BORDER_RADIUS), qreal(BORDER_RADIUS));

    if ( isStatement() )
    {
        painter->setPen(QPen(QColor(0,0,0,255)));
        painter->setFont(font);
        painter->drawText( drawBox, Qt::AlignCenter, letter );
    }
}

//////////////
/// Sizing ///
//////////////

/*
 * Converts the passed in QRectF (which should be a drawBox) to a collision box
 */
QRectF Node::toCollision(QRectF draw) const
{
    return QRectF( QPointF(draw.topLeft().x() - qreal(COLLISION_OFFSET),
                           draw.topLeft().y() - qreal(COLLISION_OFFSET)),
                   QPointF(draw.bottomRight().x() + qreal(COLLISION_OFFSET),
                           draw.bottomRight().y() + qreal(COLLISION_OFFSET)));
}

/*
 * Converts the passed in QRectF (which should be a collision box) to a drawBox
 */
QRectF Node::toDraw(QRectF coll) const
{
    return QRectF( QPointF(coll.topLeft().x() + qreal(COLLISION_OFFSET),
                           coll.topLeft().y() + qreal(COLLISION_OFFSET)),
                   QPointF(coll.bottomRight().x() - qreal(COLLISION_OFFSET),
                           coll.bottomRight().y() - qreal(COLLISION_OFFSET)));
}

/*
 * Returns a scene mapped collision box, offset by deltaX and deltaY if desired.
 *
 * deltaX and deltaY are given the default value of 0, so the params can be
 * ignored if only interested in the actual collision box and not a prediction
 */
QRectF Node::getSceneCollisionBox(qreal deltaX, qreal deltaY) const
{
    QRectF c = toCollision(drawBox);
    return QRectF(mapToScene( QPointF(c.topLeft().x() + deltaX,
                                      c.topLeft().y() + deltaY)),
                  mapToScene( QPointF(c.bottomRight().x() + deltaX,
                                      c.bottomRight().y() + deltaY )));
}

/*
 * Similar to getSceneCollisionBox, except for a drawBox instead. This isn't
 * super efficient implemented this way, but it's not a big deal and I'm lazy
 */
QRectF Node::getSceneDraw(qreal deltaX, qreal deltaY) const
{
    return toDraw(getSceneCollisionBox(deltaX, deltaY));
}

//////////////////////////
/// Collision Checking ///
//////////////////////////

/*
 * Check a point for collision given a relative move point.
 */
bool Node::checkPotential(QPointF pt, QList<Node*> sel)
{
    QList<Node*> nodes;
    QList<QRectF> potDraws;

    //Node* curr = this;
    //QRectF currPot = getSceneCollisionBox(pt.x(), pt.y());

    QList<Node*> currNodes = sel;
    QList<QRectF> currPots;

    QList< QList<Node*> > updateNodes;
    QList< QList<QRectF> > updateDraws;

    for (Node* n : sel)
        currPots.append(n->getSceneCollisionBox(pt.x(), pt.y()));

    canvas->clearBounds();

    while(true)
    {
        QList<Node*>::iterator itn = currNodes.begin();
        QList<QRectF>::iterator itp = currPots.begin();

        if (currNodes.empty())
            break;

        Node* f = currNodes.first();
        if (f->isRoot())
            break;

        for (; itn != currNodes.end() && itp != currPots.end(); ++itn, ++itp)
        {
            Node* curr = *itn;
            QRectF currPot = *itp;

            if (curr->isRoot())
                // break out of while
                goto update;
                //break; //this gets out of the for loop only...

            // Check the currPot against other siblings
            for (Node* sibling : curr->parent->children)
            {
                if (sibling == curr)
                    continue;
                else if (!currNodes.contains(sibling)) // not in the selection
                {
                    // not moving, so check its collision against the
                    // scene collision box
                    if (rectsCollide(currPot, sibling->getSceneCollisionBox()))
                        return false;
                }
            }


        }

        // Store data in case everything succeeds
        updateNodes.append(currNodes);
        updateDraws.append(currPots);

        // Quit early
        //if (curr->parent->isRoot())
            //goto update;

        // Percolate up
        //currPot = curr->predictParent(toDraw(currPot));
        //curr = curr->parent;
        Node* p = currNodes.first()->parent;

        // Break
        if (p->isRoot())
            break;

        currNodes.clear();
        currNodes.prepend(p);

        QRectF parentPotColl = Node::predictParent(currNodes, currPots);
        canvas->addRedBound(parentPotColl);
        currPots.clear();
        currPots.prepend(parentPotColl);

        //break;

    } // end while

#if 0
        /// OLD:

        if (curr->isRoot())
            break;

        // Check currPot against other siblings
        for (Node* sibling : curr->parent->children)
        {
            if (sibling == curr)
                continue;

            if (rectsCollide(currPot, sibling->getSceneCollisionBox()))
                return false;
        }

        // Store data in case everything succeeds
        nodes.prepend(curr);
        potDraws.prepend(toDraw(currPot));

        // Quit early
        if (curr->parent->isRoot())
            break;

        // Percolate up
        currPot = curr->predictParent(toDraw(currPot));
        curr = curr->parent;
#endif

update:

    //return true;

    updateNodes.pop_back();
    updateDraws.pop_back();

    QList< QList<Node*> >::iterator itn = updateNodes.begin();
    QList< QList<QRectF> >::iterator itr = updateDraws.begin();

    if (updateNodes.size() != updateDraws.size())
        throw "Update not parallel";

    for (; itn!= updateNodes.end(); ++itn, ++itr)
    {
        QList<Node*> nList = (*itn);
        QList<QRectF> rList = (*itr);

        QList<Node*>::iterator nit = nList.begin();
        QList<QRectF>::iterator rit = rList.begin();
        if (nList.size() != rList.size())
            throw "Inner list in update not parallel!";
        for (; nit != nList.end(); ++nit, ++rit)
        {
            Node* n = (*nit);
            QRectF r = (*rit);
            n->setDrawBoxFromPotential(QRectF(n->mapFromScene(r.topLeft()),
                                              n->mapFromScene(r.bottomRight())));
        }

    }

    return true;

    /// TODO: update

    // Update all the nodes
    nodes.pop_back();
    potDraws.pop_back();

    QList<Node*>::iterator it1 = nodes.begin();
    QList<QRectF>::iterator it2 = potDraws.begin();
    for ( ; it1 != nodes.end() && it2 != potDraws.end(); ++it1, ++it2)
    {
        Node* n = (*it1);
        QRectF r = (*it2);
        n->setDrawBoxFromPotential(QRectF(n->mapFromScene(r.topLeft()),
                                          n->mapFromScene(r.bottomRight())));

    }

    return true;
}

/*
 * Takes in a potential drawBox and returns a potential collision box for the
 * parent if the drawBox got updated
 */
//QRectF Node::predictParent(QRectF myPotDraw)
QRectF Node::predictParent(QList<Node*> changedNodes, QList<QRectF> changedCollide)
{
    qreal minX, minY, maxX, maxY;
    minX = minY = BIG_NUMBER;
    maxX = maxY = -BIG_NUMBER;

    QPointF tl, br;

    // TODO: ensure these errors never happen
    if (changedNodes.empty())
        throw "Empty changed nodes";
    if (changedNodes.size() != changedCollide.size())
        throw "Changed nodes not parallel to changedCollide";

    Node* par = changedNodes.first()->parent;

    for (Node* sibling : par->children)
    {
        QList<Node*>::iterator itn = changedNodes.begin();
        QList<QRectF>::iterator itr = changedCollide.begin();

        bool isAltered = false;
        QRectF currDrawBox;

        for (; itn != changedNodes.end(); ++itn, ++itr)
        {
            if ((*itn) == sibling)
            {
                isAltered = true;
                currDrawBox = sibling->toDraw(*itr);

                // So we don't look at the same node again
                changedNodes.erase(itn);
                changedCollide.erase(itr);
                break;
            }
        }

        if (!isAltered)
            currDrawBox = sibling->getSceneDraw();

        sibling->canvas->addBlueBound(currDrawBox);

        // Figure out the points
        tl = par->mapFromScene(currDrawBox.topLeft());
        br = par->mapFromScene(currDrawBox.bottomRight());

        if (tl.x() < minX)
            minX = tl.x();
        if (tl.y() < minY)
            minY = tl.y();
        if (br.x() > maxX)
            maxX = br.x();
        if (br.y() > maxY)
            maxY = br.y();
    }

    // Calculated points are a draw box in parent coords
    QPointF tlp = QPointF(minX - qreal(GRID_SPACING),
                          minY - qreal(GRID_SPACING));
    QPointF brp = QPointF(maxX + qreal(GRID_SPACING),
                          maxY + qreal(GRID_SPACING));

    // Convert back to scene
    QPointF tls = par->mapToScene(tlp);
    QPointF brs = par->mapToScene(brp);
    return par->toCollision(QRectF(tls, brs));
    //return changedNodes.first()->toCollision(QRectF(tls, brs));
}

/*
 * Update the drawBox (this makes sure to tell Qt to repaint what it needs)
 */
void Node::setDrawBoxFromPotential(QRectF potDraw)
{
    prepareGeometryChange();
    drawBox = potDraw;
}

/////////////
/// Mouse ///
/////////////

void Node::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        mouseOffset = event->pos();
        shadow->setEnabled(true);
        mouseDown = true;
        update();
    }
    else if (event->buttons() & Qt::RightButton)
        toggleSelection();
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    mouseDown = false;
    shadow->setEnabled(false);
    update();

    QGraphicsObject::mouseReleaseEvent(event);
}

void Node::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    canvas->setHighlight(this);
    QGraphicsObject::hoverEnterEvent(event);
}

void Node::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    canvas->setHighlight(parent);
    QGraphicsObject::hoverLeaveEvent(event);
}

/*
 * Dragging the mouse around will initiate the collision testing. If it finds a
 * valid target, it will visually move the node there.
 */
void Node::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (mouseDown)
    {
        // Adjusted with the mouseOffset, so that we are standardizing
        // calculations against the upper left corner
        qreal dx = mouseOffset.x();
        qreal dy = mouseOffset.y();

        QList<Node*> sel = canvas->getSelectedNodes();

        // Work on this as a selected item
        if (!sel.contains(this))
        {
            canvas->clearSelection();
            canvas->selectNode(this);
        }
        sel = canvas->getSelectedNodes();

        QList<QPointF> scenePts;

        QPointF adj = QPointF(event->pos().x() - dx, event->pos().y() - dy);
        scenePts.append(mapToScene(adj));

        for (Node* n : sel)
        {
            if (n == this)
                continue;
            scenePts.append(n->mapToScene(n->mapFromParent((n->drawBox.topLeft()))));
        }

        canvas->clearDots();

        for (QPointF pt : scenePts)
            canvas->addBlackDot(pt);

        // Construct potential points to check collision against
        QList<QPointF> bloom = constructBloom(scenePos(), mapToScene(adj));

        if (bloom.empty())
        {
            if (sel.size() == 1)
                canvas->clearSelection();
            return;
        }

        for (QPointF pt : bloom)
        {
            // Pt is now a delta
            if (checkPotential(pt, sel))
            {
                // Found an okay point, so make the move
                moveBy(pt.x(), pt.y());

                for (Node* n : sel)
                {
                    if (n == this)
                        continue;
                    n->moveBy(pt.x(), pt.y());
                }

                if (sel.size() == 1)
                    canvas->clearSelection();
                return;
            }
        }
    }
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


/*
 * scenePos: original / old position of the drawBox topleft point
 * sceneTarget: where the pos might move to (not yet snapped to grid)
 *
 * Note: both pos and target should be in scene coords
 *
 * Returns a list of "bloomed" potential points: basically giving the user a
 * little leeway in their target movement near collision boundaries. These
 * points are sorted such that the snapped target is first, but if that fails,
 * the rest of the bloomed points can be checked in a closest-to-original-pos
 * ordering.
 *
 * If the snapPoint is the same as the original pos, this function will return
 * an empty list (no movement).
 *
 * The points were originally absolute scene points (the contents of the bloom
 * list) -- however, with the multi-selection support, it is easier to
 * manipulate multiple items if they all work off deltas (the new relBloom list
 * that's returned).
 */
QList<QPointF> constructBloom(QPointF scenePos, QPointF sceneTarget)
{
    QPointF snapped = snapPoint(sceneTarget);

    QList<QPointF> bloom;

    if (snapped.x() == scenePos.x() && snapped.y() == scenePos.y())
        return bloom;

    bloom.append(QPointF(snapped.x() - qreal(GRID_SPACING), snapped.y()));
    bloom.append(QPointF(snapped.x(), snapped.y() - qreal(GRID_SPACING)));
    bloom.append(QPointF(snapped.x() + qreal(GRID_SPACING), snapped.y()));
    bloom.append(QPointF(snapped.x(), snapped.y() + qreal(GRID_SPACING)));

    std::sort(bloom.begin(), bloom.end(),
              [scenePos](const QPointF a, const QPointF b) ->
              bool { return dist(a, scenePos) < dist(b, scenePos); });

    bloom.prepend(snapped);

    // Convert to relative
    QList<QPointF> relBloom;
    for (QPointF pt : bloom)
        relBloom.append(QPointF(pt.x() - scenePos.x(),  pt.y() - scenePos.y()));

    return relBloom;
}





































