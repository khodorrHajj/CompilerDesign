#include "ParseTreeWidget.h"

#include <QBrush>
#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QPen>
#include <QVBoxLayout>
#include <QtMath>

#include "../AppTheme.h"

ParseTreeCanvas::ParseTreeCanvas(QWidget *parent)
    : QWidget(parent),
      nodeWidth(100),
      nodeHeight(50),
      horizontalSpacing(30),
      verticalSpacing(80) {
    setMinimumSize(800, 600);
    setAutoFillBackground(true);
}

void ParseTreeCanvas::setParseTree(const ParseTree& tree) {
    root = tree.getRoot();
    nodePositions.clear();
    nodeBounds.clear();

    if (root) {
        const double startX = 50;
        const double startY = 50;

        const double treeWidth = calculateSubtreeLayout(root, startX, startY, 0);
        const int maxDepth = getMaxDepth(root);

        const int width = qMax(1000, static_cast<int>(treeWidth + 100));
        const int height = qMax(600, maxDepth * static_cast<int>(nodeHeight + verticalSpacing) + 150);

        setMinimumSize(width, height);
        resize(width, height);

        qDebug() << "Tree dimensions - Width:" << width << "Height:" << height
                 << "Max depth:" << maxDepth;
    }

    update();
}

void ParseTreeCanvas::clear() {
    root = nullptr;
    nodePositions.clear();
    nodeBounds.clear();
    update();
}

double ParseTreeCanvas::calculateSubtreeLayout(std::shared_ptr<ParseTreeNode> node,
                                               double x, double y, int depth) {
    Q_UNUSED(depth);

    if (!node) {
        return x;
    }

    const auto& children = node->getChildren();

    if (children.isEmpty()) {
        QPointF pos(x, y);
        nodePositions[node.get()] = pos;
        nodeBounds[node.get()] = getNodeRect(pos);
        return x + nodeWidth + horizontalSpacing;
    }

    const double childY = y + nodeHeight + verticalSpacing;
    double childStartX = x;
    double leftmostChildX = x;
    double rightmostChildX = x;

    for (int i = 0; i < children.size(); ++i) {
        const double childTreeWidth = calculateSubtreeLayout(children[i], childStartX, childY, depth + 1);
        const QPointF childPos = nodePositions[children[i].get()];

        if (i == 0) {
            leftmostChildX = childPos.x();
        }
        rightmostChildX = childPos.x();
        childStartX = childTreeWidth;
    }

    const double parentX = (leftmostChildX + rightmostChildX) / 2.0;
    const QPointF parentPos(parentX, y);

    nodePositions[node.get()] = parentPos;
    nodeBounds[node.get()] = getNodeRect(parentPos);

    return childStartX;
}

void ParseTreeCanvas::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(AppTheme::Workspace));

    if (!root) {
        painter.setPen(QColor(AppTheme::Text));
        QFont font = painter.font();
        font.setPointSize(14);
        painter.setFont(font);
        painter.drawText(rect(),
                         Qt::AlignCenter,
                         "No parse tree to display\n\nParse an input string to generate a tree.");
        return;
    }

    drawEdgesRecursive(painter, root);
    drawNodesRecursive(painter, root);
}

void ParseTreeCanvas::drawEdgesRecursive(QPainter& painter, std::shared_ptr<ParseTreeNode> node) {
    if (!node || !nodePositions.contains(node.get())) {
        return;
    }

    const QPointF parentPos = nodePositions[node.get()];
    const QPointF parentCenter(parentPos.x() + nodeWidth / 2, parentPos.y() + nodeHeight);

    for (const auto& child : node->getChildren()) {
        if (nodePositions.contains(child.get())) {
            const QPointF childPos = nodePositions[child.get()];
            const QPointF childCenter(childPos.x() + nodeWidth / 2, childPos.y());
            drawEdge(painter, parentCenter, childCenter);
            drawEdgesRecursive(painter, child);
        }
    }
}

void ParseTreeCanvas::drawNodesRecursive(QPainter& painter, std::shared_ptr<ParseTreeNode> node) {
    if (!node) {
        return;
    }

    drawNode(painter, node);

    for (const auto& child : node->getChildren()) {
        drawNodesRecursive(painter, child);
    }
}

void ParseTreeCanvas::drawNode(QPainter& painter, std::shared_ptr<ParseTreeNode> node) {
    if (!nodePositions.contains(node.get())) {
        return;
    }

    const QPointF pos = nodePositions[node.get()];
    const QRectF rect = getNodeRect(pos);

    QColor bgColor;
    QColor borderColor;
    QColor textColor;

    if (node->getIsTerminal()) {
        bgColor = QColor("#141920");
        borderColor = QColor(AppTheme::StrongBorder);
        textColor = QColor(AppTheme::Text);
    } else if (node->getSymbol() == QString(QChar(0x03B5)) || node->getSymbol() == "Îµ") {
        bgColor = QColor("#11151A");
        borderColor = QColor(AppTheme::Border);
        textColor = QColor(AppTheme::SecondaryText);
    } else {
        bgColor = QColor("#101419");
        borderColor = QColor(AppTheme::StrongBorder);
        textColor = QColor(AppTheme::Text);
    }

    painter.setPen(QPen(borderColor, 2));
    painter.setBrush(QBrush(bgColor));

    if (node->getIsTerminal()) {
        painter.drawRect(rect);
    } else {
        painter.drawEllipse(rect);
    }

    painter.setPen(textColor);
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    QString displayText = node->getSymbol();
    if (node->getValue() != node->getSymbol() && !node->getValue().isEmpty()) {
        displayText = node->getValue();
    }

    const QFontMetrics fm(font);
    const QString elidedText = fm.elidedText(displayText, Qt::ElideRight, static_cast<int>(nodeWidth) - 10);
    painter.drawText(rect, Qt::AlignCenter, elidedText);
}

void ParseTreeCanvas::drawEdge(QPainter& painter, const QPointF& from, const QPointF& to) {
    const QColor edgeColor(AppTheme::Border);
    painter.setPen(QPen(edgeColor, 2));
    painter.drawLine(from, to);

    QPen arrowPen(edgeColor, 2);
    painter.setPen(arrowPen);

    const double angle = atan2(to.y() - from.y(), to.x() - from.x());
    const double arrowSize = 8;

    const QPointF arrowP1 = to - QPointF(sin(angle + M_PI / 3) * arrowSize,
                                         cos(angle + M_PI / 3) * arrowSize);
    const QPointF arrowP2 = to - QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                         cos(angle + M_PI - M_PI / 3) * arrowSize);

    QVector<QPointF> arrowHead;
    arrowHead << to << arrowP1 << arrowP2;

    painter.setBrush(edgeColor);
    painter.drawPolygon(arrowHead.data(), 3);
}

QRectF ParseTreeCanvas::getNodeRect(const QPointF& pos) const {
    return QRectF(pos.x(), pos.y(), nodeWidth, nodeHeight);
}

int ParseTreeCanvas::getMaxDepth(std::shared_ptr<ParseTreeNode> node, int currentDepth) {
    if (!node) {
        return currentDepth;
    }

    int maxChildDepth = currentDepth;
    for (const auto& child : node->getChildren()) {
        const int childDepth = getMaxDepth(child, currentDepth + 1);
        maxChildDepth = qMax(maxChildDepth, childDepth);
    }

    return maxChildDepth;
}

int ParseTreeCanvas::countLeaves(std::shared_ptr<ParseTreeNode> node) {
    if (!node) {
        return 0;
    }

    if (node->getChildren().isEmpty()) {
        return 1;
    }

    int count = 0;
    for (const auto& child : node->getChildren()) {
        count += countLeaves(child);
    }

    return count;
}

ParseTreeWidget::ParseTreeWidget(QWidget *parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    canvas = new ParseTreeCanvas();

    scrollArea = new QScrollArea();
    scrollArea->setWidget(canvas);
    scrollArea->setWidgetResizable(false);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "   border: none;"
        "   background-color: #0A0D11;"
        "}"
        "QWidget#qt_scrollarea_viewport {"
        "   background-color: #0A0D11;"
        "}"
        "QScrollBar:horizontal {"
        "   background-color: #0A0D11;"
        "   height: 11px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "   background-color: #3A424D;"
        "   border-radius: 5px;"
        "   min-width: 28px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "   background-color: #4A535F;"
        "}"
        "QScrollBar:vertical {"
        "   background-color: #0A0D11;"
        "   width: 11px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background-color: #3A424D;"
        "   border-radius: 5px;"
        "   min-height: 28px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background-color: #4A535F;"
        "}"
        "QScrollBar::add-line, QScrollBar::sub-line,"
        "QScrollBar::add-page, QScrollBar::sub-page {"
        "   background: transparent;"
        "   border: none;"
        "   width: 0;"
        "   height: 0;"
        "}"
    );

    layout->addWidget(scrollArea);
    setLayout(layout);
}

void ParseTreeWidget::setParseTree(const ParseTree& tree) {
    canvas->setParseTree(tree);
}

void ParseTreeWidget::clear() {
    canvas->clear();
}
