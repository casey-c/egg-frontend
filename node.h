#ifndef NODE_H
#define NODE_H

#include <QGraphicsObject>
#include <QRadialGradient>
#include <QGraphicsDropShadowEffect>

class Canvas;

enum NodeType
{
    Root,
    Cut,
    Statement,
    Placeholder
};

class Node : public QGraphicsObject
{
public:
    static Node* makeRoot(Canvas* can);
    ~Node();

    // Add
    Node* addChildCut(QPointF pt, bool usePrediction = true);
    Node* addChildStatement(QPointF pt, QString t, bool usePrediction = true);
    Node* addChildPlaceholder(QPointF pt);

    // Highlight
    void setAsHighlight();
    void removeHighlight();

    // Identifiers
    bool isRoot() const { return type == Root; }
    bool isCut() const { return type == Cut; }
    bool isStatement() const { return type == Statement; }
    bool isPlaceholder() const { return type == Placeholder; }

    // Getters
    Node* getParent() const { return parent; }
    QList<Node*> getChildren() const { return children; }
    Node* getRightSibling();
    Node* getLeftSibling();
    Node* getChild();

    // Selection
    void toggleSelection();
    void selectThis();
    void deselectThis();
    void selectAllKids();
    void colorDueToSelectedParent();
    void removeColorDueToUnselectedParent();

    static void setSelectionFromBox(Node* root, QRectF selBox);

    void adoptChild(Node* n);
    void updateAncestors();

    int getID() { return myID; }

    QRectF getSceneDraw(qreal deltaX = 0, qreal deltaY = 0) const;

private:

    //////////////
    /// Fields ///
    //////////////

    static int globalID;
    int myID;

    Canvas* canvas;
    Node* parent;

    // Visual details
    NodeType type;
    QString text;

    QRectF drawBox;

    bool highlighted;
    bool mouseDown;

    // Copy
    bool locked; // can accept copy events
    bool copying;
    Node* copyMeToParent();

    //QRadialGradient gradDefault;
    //QRadialGradient gradHighlighted;
    //QRadialGradient gradClicked;
    //QRadialGradient gradSelected;

    QGraphicsDropShadowEffect* shadow;

    // Important points
    QPointF mouseOffset;

    // Children
    QList<Node*> children;

    // Statement specific details
    QString letter;
    QFont font;

    // Selection
    bool selected, parentSelected;

    // Change parent
    bool ghost;
    Node* determineNewParent(QPointF pt);
    void raiseAllAncestors();
    void lowerAllAncestors();

    Node* newParent;
    Node* newCopy;
    bool target;

    // Add
    QPointF findPoint(const QList<QPointF> &bloom, qreal w, qreal h, bool isStatement = false);

    ///////////////
    /// Methods ///
    ///////////////

    // Private constructor
    Node(Canvas* can, Node* par, NodeType t, QPointF pt);
    Node(Canvas* can, Node* par, QString s, QPointF pt);


    // Graphics
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    // Sizing
    QRectF toCollision(QRectF draw) const;
    QRectF toDraw(QRectF collision) const;
    QRectF getSceneCollisionBox(qreal deltaX = 0, qreal deltaY = 0) const;

    // Collision Checking
    static bool checkPotential(QList<Node*> sel, QPointF pt);
    QRectF predictMySceneDraw(QList<Node*> altNodes, QList<QRectF> altDraws);
    void setDrawBoxFromPotential(QRectF potDraw);

    // Mouse
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
};

#endif // NODE_H
