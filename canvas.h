#ifndef CANVAS_H
#define CANVAS_H

#include <QGraphicsView>

class Node;

class Canvas : public QGraphicsView
{
    Q_OBJECT

public:
    Canvas(QWidget* parent = 0);
    ~Canvas() {}

    void setHighlight(Node* node);

    void clearBounds();
    void addRedBound(QRectF rect);
    void addBlackBound(QRectF rect);
    void addBlueBound(QRectF rect);
    void addGreenBound(QRectF rect);

    void clearDots();
    void addBlackDot(QPointF pt);

    void selectNode(Node* n);
    void deselectNode(Node* n);
    void clearSelection();

    QList<Node*> getSelectedNodes();
    QList<Node*> selectionIncluding(Node* n);
    bool hasAnySelectedNodes();

    void surroundNodesWithCut();
    void deleteCutAndSaveOrphans();

    void removeFromScene(Node* n);
    void deleteSelection();

    Node* getRoot() { return root; }

    void updateAll();

signals:
    void toggleTheme();

private:
    //////////////
    /// Fields ///
    //////////////

    Node* root;
    Node* highlighted;

    QPointF lastMousePos;

    QGraphicsScene* scene;

    QGraphicsRectItem* selBox;
    QPointF selStart;
    bool mouseShiftPress;
    bool noMouseMovement;
    bool setHighlightByKeyboard;

    ///////////////
    /// Methods ///
    ///////////////

    // User input
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void drawBackground(QPainter* painter,
                        const QRectF &rect) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

    // Add
    void addCut();
    void addStatement(QString s);
    void addPlaceholder();

    // Highlight
    void highlightRoot();
    void highlightChild();
    void highlightRight();
    void highlightLeft();
    void highlightParent();
    void highlightNode(Node* n);

    // Selection
    QList<Node*> selectedNodes;

    // Debug
    bool showBounds;
    //QGraphicsRectItem* debugBox;
    //QGraphicsRectItem* debugBox2;

    QList<QGraphicsRectItem*> blueBounds;
    QList<QGraphicsRectItem*> blackBounds;
    QList<QGraphicsRectItem*> redBounds;

    QList<QGraphicsEllipseItem*> blackDots;
};

#endif // CANVAS_H
