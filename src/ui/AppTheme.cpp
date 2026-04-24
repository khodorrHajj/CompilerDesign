#include "AppTheme.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace AppTheme {

namespace {
QString baseStyleSheet() {
    return QString(R"(
        QMainWindow, QWidget {
            background: %1;
            color: %9;
            font-family: "Segoe UI", "IBM Plex Sans", sans-serif;
            font-size: 10pt;
        }

        QToolTip {
            background: %4;
            color: %9;
            border: 1px solid %7;
            padding: 5px 7px;
        }

        QToolBar#CommandBar {
            background: %3;
            border: none;
            border-bottom: 1px solid %6;
            spacing: 6px;
            padding: 5px 10px;
        }

        QLabel {
            color: %9;
            background: transparent;
        }

        QLabel#AppTitle {
            font-size: 11pt;
            font-weight: 700;
            padding-right: 18px;
        }

        QLabel#WorkbenchTitle {
            color: %9;
            font-size: 12pt;
            font-weight: 700;
        }

        QLabel#SectionTitle {
            color: %9;
            font-size: 8.5pt;
            font-weight: 700;
            letter-spacing: 0.06em;
            text-transform: uppercase;
        }

        QLabel#MutedText, QLabel#EmptyText {
            color: %11;
        }

        QTabWidget::pane {
            border: none;
            background: %1;
        }

        QTabBar {
            background: %1;
        }

        QTabBar::tab {
            background: transparent;
            color: %10;
            border: none;
            border-bottom: 2px solid transparent;
            padding: 8px 15px 7px 15px;
            margin-right: 2px;
            font-weight: 600;
        }

        QTabBar::tab:selected {
            color: %9;
            border-bottom-color: %12;
        }

        QTabBar::tab:hover:!selected {
            color: %9;
            background: %5;
        }

        QFrame#Sidebar {
            background: %3;
            border: none;
            border-right: 1px solid %6;
        }

        QFrame#InspectorPanel {
            background: %3;
            border: none;
            border-left: 1px solid %6;
        }

        QFrame#CanvasShell {
            background: %1;
            border: none;
        }

        QFrame#SectionGroup {
            background: transparent;
            border: none;
        }

        QFrame#WorkbenchToolbar {
            background: %3;
            border: 1px solid %6;
            border-radius: %17px;
        }

        QFrame#WorkbenchPanel {
            background: %3;
            border: 1px solid %6;
            border-radius: %17px;
        }

        QFrame#InlineStatusBar {
            background: %3;
            border: 1px solid %6;
            border-radius: %17px;
        }

        QFrame#SectionDivider {
            background: %6;
            border: none;
            max-height: 1px;
        }

        QFrame#StatCard {
            background: %4;
            border: 1px solid rgba(255, 255, 255, 0.045);
            border-radius: %17px;
        }

        QPushButton, QToolButton {
            background: %4;
            color: %9;
            border: 1px solid %6;
            border-radius: %17px;
            min-height: 26px;
            padding: 5px 10px;
            font-weight: 600;
        }

        QPushButton:hover, QToolButton:hover {
            background: %5;
            border-color: %7;
        }

        QPushButton:pressed, QToolButton:pressed {
            background: #0F1318;
        }

        QPushButton:disabled, QToolButton:disabled {
            background: #101419;
            color: %11;
            border-color: #1C222A;
        }

        QPushButton[class="toolbar"], QToolButton {
            background: transparent;
            border-color: transparent;
            padding: 4px 9px;
            min-height: 24px;
        }

        QPushButton[class="toolbar"]:hover, QToolButton:hover {
            background: %5;
            border-color: transparent;
        }

        QPushButton[class="primary"] {
            background: %12;
            color: %13;
            border-color: %12;
        }

        QPushButton[class="primary"]:hover {
            background: %9;
            border-color: %9;
        }

        QPushButton[class="primary"]:pressed {
            background: %14;
            border-color: %14;
        }

        QPushButton[class="secondary"] {
            background: #12171D;
            color: %9;
            border-color: #2C3440;
        }

        QPushButton[class="danger"] {
            background: #12171D;
            color: %15;
            border-color: %16;
        }

        QPushButton[class="danger"]:hover {
            background: %18;
            border-color: %15;
        }

        QRadioButton {
            color: %9;
            background: transparent;
            border: none;
            border-left: 2px solid transparent;
            padding: 6px 8px;
            min-height: 25px;
            font-weight: 600;
        }

        QRadioButton::indicator {
            width: 0;
            height: 0;
        }

        QRadioButton:hover {
            background: %4;
        }

        QRadioButton:checked {
            color: %9;
            background: %5;
            border-left-color: %12;
        }

        QGroupBox {
            background: transparent;
            color: %9;
            border: 1px solid %6;
            border-radius: %17px;
            margin-top: 16px;
            padding: 12px 8px 8px 8px;
            font-weight: 700;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 8px;
            padding: 0 5px;
            color: %10;
            background: %1;
        }

        QListWidget, QTreeWidget, QTableWidget, QTextEdit, QPlainTextEdit, QLineEdit, QComboBox {
            background: %2;
            color: %9;
            border: 1px solid %6;
            border-radius: %17px;
            selection-background-color: rgba(229, 231, 235, 0.13);
            selection-color: %9;
        }

        QTextEdit, QPlainTextEdit, QLineEdit {
            padding: 6px;
        }

        QTextEdit#CodeEditor, QPlainTextEdit#CodeEditor {
            background: %2;
            color: %9;
            border: 1px solid %6;
            padding: 10px 12px;
            selection-background-color: rgba(255, 255, 255, 0.10);
        }

        QTextEdit#DiagnosticsView, QPlainTextEdit#DiagnosticsView {
            background: #11151A;
            color: %9;
            border: 1px solid %6;
            padding: 10px 12px;
        }

        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QComboBox:focus {
            border-color: %7;
        }

        QCheckBox {
            color: %9;
            background: transparent;
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 14px;
            height: 14px;
            border: 1px solid %7;
            background: %2;
            border-radius: 3px;
        }

        QCheckBox::indicator:checked {
            background: %12;
            border-color: %12;
        }

        QListWidget {
            padding: 3px;
        }

        QListWidget::item, QTreeWidget::item {
            border-radius: 3px;
            padding: 6px 7px;
            margin: 1px;
        }

        QListWidget#WorkbenchList, QTreeWidget#WorkbenchTree {
            background: %2;
            border: 1px solid %6;
        }

        QListWidget::item:selected, QTreeWidget::item:selected, QTableWidget::item:selected {
            background: rgba(229, 231, 235, 0.10);
            color: %9;
        }

        QHeaderView::section {
            background: %3;
            color: %10;
            border: none;
            border-bottom: 1px solid %6;
            padding: 6px 8px;
            font-weight: 700;
        }

        QTableWidget {
            gridline-color: #20262E;
        }

        QTableWidget#WorkbenchTable {
            background: %2;
            alternate-background-color: #0D1116;
            border: 1px solid %6;
        }

        QTableWidget::item {
            padding: 5px;
        }

        QSplitter::handle {
            background: %1;
        }

        QSplitter::handle:hover {
            background: %6;
        }

        QComboBox::drop-down {
            border: none;
            width: 22px;
        }

        QComboBox QAbstractItemView {
            background: %3;
            color: %9;
            border: 1px solid %7;
            selection-background-color: %5;
        }

        QMenu {
            background: %3;
            color: %9;
            border: 1px solid %7;
            padding: 4px;
        }

        QMenu::item {
            padding: 6px 22px;
            border-radius: 3px;
        }

        QMenu::item:selected {
            background: %5;
        }

        QMenu::separator {
            height: 1px;
            background: %6;
            margin: 4px 0;
        }

        QDialog, QMessageBox {
            background: %3;
            color: %9;
        }

        QMessageBox QLabel {
            color: %9;
            min-width: 300px;
        }

        QSlider::groove:horizontal {
            height: 3px;
            background: %6;
            border-radius: 1px;
        }

        QSlider::handle:horizontal {
            background: %12;
            border: 1px solid %3;
            width: 12px;
            height: 12px;
            margin: -5px 0;
            border-radius: 6px;
        }

        QScrollBar:vertical, QScrollBar:horizontal {
            background: %2;
            border: none;
            margin: 0;
        }

        QScrollBar:vertical {
            width: 11px;
        }

        QScrollBar:horizontal {
            height: 11px;
        }

        QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
            background: #3A424D;
            border-radius: 5px;
            min-height: 28px;
            min-width: 28px;
        }

        QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover {
            background: #4A535F;
        }

        QScrollBar::add-line, QScrollBar::sub-line,
        QScrollBar::add-page, QScrollBar::sub-page {
            background: transparent;
            border: none;
            width: 0;
            height: 0;
        }

        QStatusBar {
            background: %3;
            color: %10;
            border-top: 1px solid %6;
            padding: 3px 8px;
        }
    )")
        .arg(Background)       // %1
        .arg(Workspace)        // %2
        .arg(Surface)          // %3
        .arg(SurfaceAlt)       // %4
        .arg(Hover)            // %5
        .arg(Border)           // %6
        .arg(StrongBorder)     // %7
        .arg("")               // %8 reserved
        .arg(Text)             // %9
        .arg(SecondaryText)    // %10
        .arg(MutedText)        // %11
        .arg(Accent)           // %12
        .arg(Background)       // %13
        .arg(AccentPressed)    // %14
        .arg(Danger)           // %15
        .arg(DangerBorder)     // %16
        .arg(Radius)           // %17
        .arg(DangerHover);     // %18
}
}

QString styleSheet() {
    return baseStyleSheet();
}

QString dialogStyleSheet() {
    return baseStyleSheet();
}

namespace {
QLabel* createMutedLabel(const QString& text) {
    QLabel* label = new QLabel(text);
    label->setObjectName("MutedText");
    label->setWordWrap(true);
    return label;
}

void showDialog(QWidget* parent,
                const QString& title,
                const QString& message,
                const QString& buttonClass,
                const QString& buttonText) {
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.setModal(true);
    dialog.setMinimumWidth(360);
    dialog.setStyleSheet(dialogStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(14);

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 13pt; font-weight: 700;");
    layout->addWidget(titleLabel);

    layout->addWidget(createMutedLabel(message));

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();

    QPushButton* okButton = new QPushButton(buttonText);
    okButton->setProperty("class", buttonClass);
    okButton->setDefault(true);
    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    buttons->addWidget(okButton);
    layout->addLayout(buttons);

    dialog.exec();
}
}

void showInfoDialog(QWidget* parent, const QString& title, const QString& message) {
    showDialog(parent, title, message, "primary", "OK");
}

void showWarningDialog(QWidget* parent, const QString& title, const QString& message) {
    showDialog(parent, title, message, "secondary", "OK");
}

void showErrorDialog(QWidget* parent, const QString& title, const QString& message) {
    showDialog(parent, title, message, "danger", "OK");
}

bool showConfirmDialog(QWidget* parent,
                       const QString& title,
                       const QString& message,
                       const QString& confirmText,
                       const QString& cancelText,
                       bool danger) {
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    dialog.setModal(true);
    dialog.setMinimumWidth(380);
    dialog.setStyleSheet(dialogStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(14);

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 13pt; font-weight: 700;");
    layout->addWidget(titleLabel);

    layout->addWidget(createMutedLabel(message));

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();

    QPushButton* cancelButton = new QPushButton(cancelText);
    cancelButton->setProperty("class", "secondary");
    cancelButton->setDefault(true);
    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    buttons->addWidget(cancelButton);

    QPushButton* confirmButton = new QPushButton(confirmText);
    confirmButton->setProperty("class", danger ? "danger" : "primary");
    QObject::connect(confirmButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    buttons->addWidget(confirmButton);

    layout->addLayout(buttons);
    cancelButton->setFocus();

    return dialog.exec() == QDialog::Accepted;
}

}
