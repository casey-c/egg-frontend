#include "canvas.h"
#include "node.h"
#include <QKeyEvent>
#include <QDebug>

#define SEL_BOX_Z 10

Canvas::Canvas(QWidget* parent) :
    QGraphicsView(parent),
    mouseShiftPress(false),
    noMouseMovement(false),
    showBounds(false)
{
    scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(-200, -200, 400, 400);
    setScene(scene);

    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setMinimumSize(400, 400);

    root = Node::makeRoot(this);
    highlighted = root;

    // Selection box
    selBox = scene->addRect(QRectF(QPointF(0,0), QSizeF(0,0)));
    selBox->setZValue(SEL_BOX_Z);
    selBox->setVisible(false);
}

void Canvas::drawBackground(QPainter* painter, const QRectF &rect)
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    //QRectF sceneRect = this->sceneRect();

    //QLinearGradient gradient(sceneRect.topLeft(),
                             //sceneRect.bottomRight());
    //gradient.setColorAt(0, Qt::white);
    //gradient.setColorAt(1, QColor(Qt::lightGray).lighter(150));

    //painter->fillRect(rect.intersected(sceneRect),
                      //gradient);

    //painter->setBrush(Qt::NoBrush);
    //painter->drawRect(sceneRect);
}

void Canvas::keyPressEvent(QKeyEvent* event)
{
    QGraphicsView::keyPressEvent(event);
    switch(event->key())
    {
    case Qt::Key_X:
        addCut();
        break;
    case Qt::Key_A:
        if ( event->modifiers() & Qt::ControlModifier)
            highlighted->selectAllKids();
        else
            addStatement("A");
        break;
    case Qt::Key_B:
        if ( event->modifiers() & Qt::ControlModifier)
        {
            showBounds = !showBounds;
            qDebug() << "toggle showBounds to" << showBounds;
            if (!showBounds)
                clearBounds();
        }
        else
            addStatement("B");
        break;
    case Qt::Key_C:
        addStatement("C");
        break;
    case Qt::Key_D:
        if (event->modifiers() & Qt::ControlModifier)
            clearSelection();
        else
            addStatement("D");
        break;
    case Qt::Key_E:
        addStatement("E");
        break;
    case Qt::Key_F:
        addStatement("F");
        break;
    case Qt::Key_4:
        addPlaceholder();
        break;
    }
}

void Canvas::mouseMoveEvent(QMouseEvent* event)
{
    if (mouseShiftPress)
    {
        QPointF pt = mapToScene(event->pos());

        qreal minX = qMin(pt.x(), selStart.x());
        qreal minY = qMin(pt.y(), selStart.y());

        qreal width = qAbs(pt.x() - selStart.x());
        qreal height = qAbs(pt.y() - selStart.y());

        // Give a little leeway for clicking
        if (width > 2 || height > 2)
            noMouseMovement = false;

        selBox->setRect(QRectF(QPointF(minX, minY), QSize(width, height)));

        selBox->setVisible(true);
    }
    else
    {
        lastMousePos = mapToScene(event->pos());
    }
    QGraphicsView::mouseMoveEvent(event);
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
    if (event->modifiers() & Qt::ShiftModifier)
    {
        mouseShiftPress = true;
        noMouseMovement = true;
        selStart = mapToScene(event->pos());
    }
    else
    {
        if (highlighted == root)
            clearSelection();

        QGraphicsView::mousePressEvent(event);
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (mouseShiftPress)
    {
        mouseShiftPress = false;
        selBox->setVisible(false);

        if (noMouseMovement)
        {
            qDebug() << "No mouse movement, handle as click";
            if (!highlighted->isRoot())
                highlighted->toggleSelection();
        }
        else
        {
            qDebug() << "Need to determine selection";
            Node::setSelectionFromBox(root, selBox->rect());
        }

    }

    QGraphicsView::mouseReleaseEvent(event);
}

void Canvas::setHighlight(Node* node)
{
    highlighted->removeHighlight();
    highlighted = node;
    highlighted->setAsHighlight();
}

void Canvas::addCut()
{
    Node* n = highlighted->addChildCut(lastMousePos);
    if (n == nullptr)
        return;

    if ( n->getParent() == root )
        scene->addItem(n);

    setHighlight(n);
}

void Canvas::addStatement(QString s)
{
    Node* n = highlighted->addChildStatement(lastMousePos, s);
    if (n == nullptr)
        return;

    if (n->getParent() == root)
        scene->addItem(n);

    setHighlight(n);
}

void Canvas::addPlaceholder()
{
    Node* n = highlighted->addChildStatement(lastMousePos, "");
    if (n == nullptr)
        return;

    if (n->getParent() == root)
        scene->addItem(n);

    setHighlight(n);
}


/// Debug Bounds ///

void Canvas::clearBounds()
{
    for(QGraphicsRectItem* item : blueBounds)
        scene->removeItem(item);
    for(QGraphicsRectItem* item : blackBounds)
        scene->removeItem(item);
    for(QGraphicsRectItem* item : redBounds)
        scene->removeItem(item);

    blueBounds.clear();
    blackBounds.clear();
    redBounds.clear();
}

void Canvas::clearDots()
{
    for (QGraphicsEllipseItem* item : blackDots)
        scene->removeItem(item);

    blackDots.clear();
}

void Canvas::addBlackDot(QPointF pt)
{
    if (showBounds)
        blackDots.append(scene->addEllipse(pt.x() - 1,
                                           pt.y() - 1,
                                           2,
                                           2,
                                           QPen(QColor(0,0,0)),
                                           QBrush(QColor(0,0,0))));
}

void Canvas::addRedBound(QRectF rect)
{
    if(showBounds)
        redBounds.append(scene->addRect(rect, QPen(QColor(255, 0, 0))));
}

void Canvas::addBlackBound(QRectF rect)
{
    if(showBounds)
        blackBounds.append(scene->addRect(rect, QPen(QColor(0, 0, 0))));
}

void Canvas::addBlueBound(QRectF rect)
{
    if(showBounds)
        blueBounds.append(scene->addRect(rect, QPen(QColor(0, 0, 255))));
}

void Canvas::addGreenBound(QRectF rect)
{
    if(showBounds)
        blueBounds.append(scene->addRect(rect, QPen(QColor(0, 255, 0))));
}


// Selection
void Canvas::clearSelection()
{
    for (Node* n : selectedNodes)
        n->deselectThis();

    selectedNodes.clear();
}

void Canvas::selectNode(Node* n)
{
    // Make sure we don't add the same node twice
    if (selectedNodes.contains(n))
        return;

    // Ensure all nodes in the selection share the same parent
    if (!selectedNodes.empty())
    {
        Node* parent = selectedNodes.first()->getParent();
        if (n->getParent() != parent)
            clearSelection();
    }

    n->selectThis();
    selectedNodes.append(n);
}

void Canvas::deselectNode(Node* n)
{
    selectedNodes.removeOne(n);
    n->deselectThis();
}

QList<Node*> Canvas::getSelectedNodes()
{
    return selectedNodes;
}

QList<Node*> Canvas::selectionIncluding(Node* n)
{
    if (selectedNodes.contains(n))
        return selectedNodes;

    clearSelection();
    selectNode(n);
    return selectedNodes;
}

bool Canvas::hasAnySelectedNodes()
{
    return !selectedNodes.empty();
}


void Canvas::removeFromScene(Node* n)
{
    if (n->isRoot())
        return;

    scene->removeItem(n);
    highlighted = n->getParent();
    delete n;
}
