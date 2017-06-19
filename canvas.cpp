#include "canvas.h"
#include "node.h"
#include <QKeyEvent>
#include <QDebug>

Canvas::Canvas(QWidget* parent) :
    QGraphicsView(parent),
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
    lastMousePos = mapToScene(event->pos());
    QGraphicsView::mouseMoveEvent(event);
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
    // TODO: could put a check in here to ensure we don't select the same node
    // twice, but that may be unnecessary and ineffient

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
