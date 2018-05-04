#include "canvas.h"
#include "node.h"
#include <QKeyEvent>
#include <QDebug>
#include <QScrollBar>
#include "constants.h"
#include "colorpalette.h"

class MainWindow;

Canvas::Canvas(QWidget* parent) :
    QGraphicsView(parent),
    mouseShiftPress(false),
    noMouseMovement(false),
    setHighlightByKeyboard(false),
    showBounds(true)
{
    scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    //scene->setSceneRect(-200, -200, 400, 400);
    scene->setSceneRect(0,0,10000,10000);
    setScene(scene);

    setCacheMode(CacheBackground);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    setMinimumSize(400, 400);

    root = Node::makeRoot(this);
    highlighted = root;

    // Selection box
    selBox = scene->addRect(QRectF(QPointF(0,0), QSizeF(0,0)));
    selBox->setZValue(SEL_BOX_Z);
    selBox->setVisible(false);

    // Scroll bars (debug testing)
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void Canvas::drawBackground(QPainter* painter, const QRectF &rect)
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    QRectF sceneRect = this->sceneRect();

    //QLinearGradient gradient(sceneRect.topLeft(),
                             //sceneRect.bottomRight());
    //gradient.setColorAt(0, Qt::white);
    //gradient.setColorAt(1, QColor(Qt::lightGray).lighter(150));

    painter->fillRect(rect.intersected(sceneRect), ColorPalette::canvasColor());

    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sceneRect);
}

void Canvas::keyPressEvent(QKeyEvent* event)
{
    QGraphicsView::keyPressEvent(event);
    QString key = event->text();
    qDebug() << "key pressed" << key;

    if ((event->modifiers() & Qt::ShiftModifier) ||
        (event->text().length() > 0 && event->text().at(0).isUpper()))
    {
        switch(event->key())
        {
        case Qt::Key_X:
          addCut();
          break;
        case Qt::Key_H: {
            qDebug() << "shift left";
            //translate(-GRID_SPACING, 0);
            //rotate(45);
            QScrollBar* h = horizontalScrollBar();
            h->setValue(h->value() + 2*GRID_SPACING);
            //horizontalScrollBar()->setValue(horizontalScrollBar()->value() - GRID_SPACING);
            break;
        }
        case Qt::Key_L: {
            qDebug() << "shift right";
            //translate(GRID_SPACING, 0);
            QScrollBar* h = horizontalScrollBar();
            h->setValue(h->value() - 2*GRID_SPACING);
            break;
        }
        case Qt::Key_J: {
            qDebug() << "shift down";
            QScrollBar* v = verticalScrollBar();
            v->setValue(v->value() - 2*GRID_SPACING);
            break;
        }
        case Qt::Key_K: {
            qDebug() << "shift up";
            QScrollBar* v = verticalScrollBar();
            v->setValue(v->value() + 2*GRID_SPACING);
            break;
        }
        case Qt::Key_I:
            scale(1.1,1.1);
            break;
        case Qt::Key_O:
            scale(.91,.91);
            break;
        case Qt::Key_R: {
            rotate(45);
            break;
        }
        case Qt::Key_T:
            emit toggleTheme();
            break;
        case Qt::Key_S:
            qDebug() << "surround with cut";
            surroundNodesWithCut();
            break;
        case Qt::Key_A:
            highlighted->selectAllKids();
            break;
        case Qt::Key_E:
            qDebug() << "erase cut but keep kids";
            deleteCutAndSaveOrphans();
            break;
        }
    }
    else if (event->modifiers() & Qt::ControlModifier)
    {
        switch(event->key())
        {
        case Qt::Key_B:
          {
            showBounds = !showBounds;
            qDebug() << "toggle showBounds to" << showBounds;
            if (!showBounds)
              clearBounds();
          }
          break;
        case Qt::Key_D:
            clearSelection();
            break;
        }
    }

    else
    {
      switch(event->key())
      {
      case Qt::Key_A: addStatement("A"); break;
      case Qt::Key_B: addStatement("B"); break;
      case Qt::Key_C: addStatement("C"); break;
      case Qt::Key_D: addStatement("D"); break;
      case Qt::Key_E: addStatement("E"); break;
      case Qt::Key_F: addStatement("F"); break;
      case Qt::Key_G: addStatement("G"); break;
      case Qt::Key_H: addStatement("H"); break;
      case Qt::Key_I: addStatement("I"); break;
      case Qt::Key_J: addStatement("J"); break;
      case Qt::Key_K: addStatement("K"); break;
      case Qt::Key_L: addStatement("L"); break;
      case Qt::Key_M: addStatement("M"); break;
      case Qt::Key_N: addStatement("N"); break;
      case Qt::Key_O: addStatement("O"); break;
      case Qt::Key_P: addStatement("P"); break;
      case Qt::Key_Q: addStatement("Q"); break;
      case Qt::Key_R: addStatement("R"); break;
      case Qt::Key_S: addStatement("S"); break;
      case Qt::Key_T: addStatement("T"); break;
      case Qt::Key_U: addStatement("U"); break;
      case Qt::Key_V: addStatement("V"); break;
      case Qt::Key_W: addStatement("W"); break;
      case Qt::Key_X: addStatement("X"); break;
      case Qt::Key_Y: addStatement("Y"); break;
      case Qt::Key_Z: addStatement("Z"); break;
      case Qt::Key_4:
        addPlaceholder();
        break;
      }
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
        if (setHighlightByKeyboard) {
            qDebug() << "mouse moved canvas and keyboard mode";
            setHighlightByKeyboard = false;
            highlightNode(root);
        }
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

void Canvas::surroundNodesWithCut() {
    if (selectedNodes.empty()) {
        if (highlighted == root)
            return;
        else
            selectNode(highlighted);
    }

    //selectNode(highlighted);
    Node* par = selectedNodes.first()->getParent();

    qDebug() << "lastMousePos" << lastMousePos.x() << lastMousePos.y();

    Node* n = par->addChildCut(lastMousePos);
    if (par == root)
        scene->addItem(n);

    qDebug() << "created node " << n->getID();

    for (Node* s : selectedNodes) {
       n->adoptChild(s);
    }

    clearSelection();
    highlightNode(n);
    n->updateAncestors();
}

void Canvas::highlightNode(Node* n) {
    highlighted->removeHighlight();
    highlighted = n;
    n->setAsHighlight();
    n->update();
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

  // Don't want to select root
  if (n == root)
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

// TODO: make it selection based, only works on highlight right now
void Canvas::deleteCutAndSaveOrphans() {
    if (selectedNodes.empty())
        selectNode(highlighted);

    // Can't delete non-cuts
    for (Node* n : selectedNodes) {
        if (!n->isCut())
            return;
    }

    for (Node* n : selectedNodes) {
        Node* par = n->getParent();

        QList<Node*> orphans = n->getChildren();
        for (Node* o : orphans) {
            qDebug() << "saving orphan" << o->getID();
            QRectF oSceneDraw = o->getSceneDraw();

            o->removeColorDueToUnselectedParent();
            par->adoptChild(o);

            if (par->isRoot())
                scene->addItem(o);

            // Fix the position into the new coords
            QRectF newScene = o->getSceneDraw();

            qreal dx = oSceneDraw.left() - newScene.left();
            qreal dy = oSceneDraw.top() - newScene.top();

            o->moveBy(dx,dy);
        }
    }

    deleteSelection();
    qDebug() << "clearing selection";
    clearSelection();
}


void Canvas::removeFromScene(Node* n)
{
  if (n->isRoot())
    return;

  scene->removeItem(n);
  highlighted = n->getParent();
  delete n;
}

void Canvas::addNodeToScene(Node* n) {
    scene->addItem(n);
}

void Canvas::deleteSelection()
{
  if (selectedNodes.empty() || selectedNodes.first()->isRoot() )
    return;

  // Update highlight
  highlightNode(selectedNodes.first()->getParent());
  setHighlightByKeyboard = true;

  // Delete everything
  for (Node* n : selectedNodes)
  {
      scene->removeItem(n);
      delete n;
  }
  selectedNodes.clear();
}

void Canvas::updateAll() {
    for (QGraphicsItem* i : scene->items() ) {
        Node* n = dynamic_cast<Node*>(i);
        if (n != nullptr)
            n->update();
    }
    invalidateScene(sceneRect());
}
