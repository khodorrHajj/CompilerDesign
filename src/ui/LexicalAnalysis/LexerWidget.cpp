#include "LexerWidget.h"

#include <QAbstractItemView>
#include <QFontDatabase>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

#include "../AppTheme.h"

namespace {

QFrame* makePanel() {
    QFrame* panel = new QFrame();
    panel->setObjectName("WorkbenchPanel");
    return panel;
}

QFrame* makeStatusBarPanel() {
    QFrame* panel = new QFrame();
    panel->setObjectName("InlineStatusBar");
    return panel;
}

QLabel* makeSectionTitle(const QString& text) {
    QLabel* label = new QLabel(text);
    label->setObjectName("SectionTitle");
    return label;
}

QLabel* makeMutedLabel(const QString& text, const QString& objectName = "MutedText") {
    QLabel* label = new QLabel(text);
    label->setObjectName(objectName);
    label->setWordWrap(true);
    return label;
}

QWidget* makeEmptyState(QLabel** label,
                        const QString& title,
                        const QString& description) {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(6);
    layout->addStretch();

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(QString("color: %1; font-weight: 600;").arg(AppTheme::Text));
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    *label = makeMutedLabel(description, "EmptyText");
    (*label)->setAlignment(Qt::AlignCenter);
    layout->addWidget(*label);

    layout->addStretch();
    return page;
}

void polishButton(QPushButton* button) {
    button->style()->unpolish(button);
    button->style()->polish(button);
}

void populateTokenRow(QTableWidget* table, int row, const Token& token) {
    table->insertRow(row);

    QColor textColor(AppTheme::Text);
    QColor secondaryText(AppTheme::SecondaryText);
    QColor rowColor = (row % 2 == 0) ? QColor("#0D1116") : QColor("#10151B");

    QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(row + 1));
    QTableWidgetItem* typeItem = new QTableWidgetItem(token.getTypeString());
    QTableWidgetItem* lexemeItem = new QTableWidgetItem(token.getLexeme());
    QTableWidgetItem* lineItem = new QTableWidgetItem(QString::number(token.getLine()));
    QTableWidgetItem* columnItem = new QTableWidgetItem(QString::number(token.getColumn()));

    QList<QTableWidgetItem*> items = {indexItem, typeItem, lexemeItem, lineItem, columnItem};
    for (QTableWidgetItem* item : items) {
        item->setBackground(rowColor);
        item->setForeground(textColor);
    }

    indexItem->setForeground(secondaryText);
    lineItem->setForeground(secondaryText);
    columnItem->setForeground(secondaryText);

    table->setItem(row, 0, indexItem);
    table->setItem(row, 1, typeItem);
    table->setItem(row, 2, lexemeItem);
    table->setItem(row, 3, lineItem);
    table->setItem(row, 4, columnItem);
}

}

LexerWidget::LexerWidget(QWidget *parent)
    : QWidget(parent),
      inputTextEdit(nullptr),
      tokensTable(nullptr),
      errorTextEdit(nullptr),
      tokensStack(nullptr),
      diagnosticsStack(nullptr),
      tokensEmptyLabel(nullptr),
      diagnosticsEmptyLabel(nullptr),
      tokenizeButton(nullptr),
      clearButton(nullptr),
      skipWhitespaceCheckBox(nullptr),
      skipCommentsCheckBox(nullptr),
      statusLabel(nullptr),
      lexer(new Lexer()),
      automatonManager(nullptr) {
    setupUI();
    createConnections();
}

LexerWidget::~LexerWidget() {
    delete lexer;
}

void LexerWidget::setAutomatonManager(AutomatonManager* manager) {
    automatonManager = manager;
    if (lexer) {
        lexer->setAutomatonManager(manager);
    }
}

void LexerWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 12, 14, 12);
    mainLayout->setSpacing(10);

    QFrame* toolbar = new QFrame();
    toolbar->setObjectName("WorkbenchToolbar");
    QVBoxLayout* toolbarLayout = new QVBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(14, 12, 14, 12);
    toolbarLayout->setSpacing(10);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(10);

    QVBoxLayout* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);

    QLabel* titleLabel = new QLabel("Lexical Analyzer");
    titleLabel->setObjectName("WorkbenchTitle");
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(makeMutedLabel("Tokenize source code and inspect generated tokens."));

    headerLayout->addLayout(titleLayout, 1);

    QHBoxLayout* actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(8);
    tokenizeButton = new QPushButton("Tokenize");
    tokenizeButton->setProperty("class", "primary");
    clearButton = new QPushButton("Clear");
    clearButton->setProperty("class", "secondary");
    polishButton(tokenizeButton);
    polishButton(clearButton);
    actionsLayout->addWidget(tokenizeButton);
    actionsLayout->addWidget(clearButton);
    headerLayout->addLayout(actionsLayout);
    toolbarLayout->addLayout(headerLayout);

    QHBoxLayout* optionsLayout = new QHBoxLayout();
    optionsLayout->setSpacing(16);
    optionsLayout->setContentsMargins(0, 0, 0, 0);
    skipWhitespaceCheckBox = new QCheckBox("Skip Whitespace");
    skipWhitespaceCheckBox->setChecked(true);
    skipCommentsCheckBox = new QCheckBox("Skip Comments");
    skipCommentsCheckBox->setChecked(true);
    optionsLayout->addWidget(skipWhitespaceCheckBox);
    optionsLayout->addWidget(skipCommentsCheckBox);
    optionsLayout->addStretch();
    toolbarLayout->addLayout(optionsLayout);

    mainLayout->addWidget(toolbar);

    QSplitter* mainSplitter = new QSplitter(Qt::Vertical);
    mainSplitter->setChildrenCollapsible(false);
    mainSplitter->setHandleWidth(6);

    QWidget* topWorkbench = new QWidget();
    QHBoxLayout* topWorkbenchLayout = new QHBoxLayout(topWorkbench);
    topWorkbenchLayout->setContentsMargins(0, 0, 0, 0);
    topWorkbenchLayout->setSpacing(10);

    QSplitter* contentSplitter = new QSplitter(Qt::Horizontal);
    contentSplitter->setChildrenCollapsible(false);
    contentSplitter->setHandleWidth(6);

    QFrame* sourcePanel = makePanel();
    QVBoxLayout* sourceLayout = new QVBoxLayout(sourcePanel);
    sourceLayout->setContentsMargins(14, 12, 14, 14);
    sourceLayout->setSpacing(8);
    sourceLayout->addWidget(makeSectionTitle("Source"));

    inputTextEdit = new QTextEdit();
    inputTextEdit->setObjectName("CodeEditor");
    inputTextEdit->setPlaceholderText("Enter your source code here...");
    inputTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    inputTextEdit->setText(
        "int main() {\n"
        "    int x = 10;\n"
        "    float y = 3.14;\n"
        "    if (x > 5) {\n"
        "        return x + y;\n"
        "    }\n"
        "    return 0;\n"
        "}"
    );
    sourceLayout->addWidget(inputTextEdit, 1);
    contentSplitter->addWidget(sourcePanel);

    QFrame* tokensPanel = makePanel();
    QVBoxLayout* tokensLayout = new QVBoxLayout(tokensPanel);
    tokensLayout->setContentsMargins(14, 12, 14, 14);
    tokensLayout->setSpacing(8);
    tokensLayout->addWidget(makeSectionTitle("Tokens"));

    tokensStack = new QStackedWidget();
    tokensStack->addWidget(makeEmptyState(&tokensEmptyLabel,
                                          "No tokens generated yet.",
                                          "Run the tokenizer to inspect lexical output."));

    tokensTable = new QTableWidget();
    tokensTable->setObjectName("WorkbenchTable");
    tokensTable->setColumnCount(5);
    tokensTable->setHorizontalHeaderLabels({"#", "Type", "Lexeme", "Line", "Column"});
    tokensTable->setAlternatingRowColors(false);
    tokensTable->setShowGrid(true);
    tokensTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tokensTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tokensTable->setSelectionMode(QAbstractItemView::SingleSelection);
    tokensTable->verticalHeader()->setVisible(false);
    tokensTable->verticalHeader()->setDefaultSectionSize(26);
    tokensTable->horizontalHeader()->setHighlightSections(false);
    tokensTable->horizontalHeader()->setStretchLastSection(false);
    tokensTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tokensTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tokensTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tokensTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tokensTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    tokensTable->setColumnWidth(0, 52);
    tokensTable->setColumnWidth(1, 180);
    tokensStack->addWidget(tokensTable);
    tokensLayout->addWidget(tokensStack, 1);
    contentSplitter->addWidget(tokensPanel);

    contentSplitter->setStretchFactor(0, 5);
    contentSplitter->setStretchFactor(1, 4);
    contentSplitter->setSizes({640, 520});
    topWorkbenchLayout->addWidget(contentSplitter);
    mainSplitter->addWidget(topWorkbench);

    QFrame* diagnosticsPanel = makePanel();
    QVBoxLayout* diagnosticsLayout = new QVBoxLayout(diagnosticsPanel);
    diagnosticsLayout->setContentsMargins(14, 12, 14, 14);
    diagnosticsLayout->setSpacing(8);
    diagnosticsLayout->addWidget(makeSectionTitle("Diagnostics"));

    diagnosticsStack = new QStackedWidget();
    diagnosticsStack->addWidget(makeEmptyState(&diagnosticsEmptyLabel,
                                               "No diagnostics.",
                                               "Lexer warnings and errors appear here."));

    errorTextEdit = new QTextEdit();
    errorTextEdit->setObjectName("DiagnosticsView");
    errorTextEdit->setReadOnly(true);
    errorTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    diagnosticsStack->addWidget(errorTextEdit);
    diagnosticsLayout->addWidget(diagnosticsStack, 1);
    mainSplitter->addWidget(diagnosticsPanel);

    mainSplitter->setStretchFactor(0, 6);
    mainSplitter->setStretchFactor(1, 2);
    mainSplitter->setSizes({520, 160});
    mainLayout->addWidget(mainSplitter, 1);

    QFrame* statusPanel = makeStatusBarPanel();
    QHBoxLayout* statusLayout = new QHBoxLayout(statusPanel);
    statusLayout->setContentsMargins(12, 8, 12, 8);
    statusLayout->setSpacing(8);
    statusLabel = makeMutedLabel("Ready");
    statusLabel->setWordWrap(false);
    statusLayout->addWidget(statusLabel);
    statusLayout->addStretch();
    mainLayout->addWidget(statusPanel);

    setLayout(mainLayout);
}

void LexerWidget::createConnections() {
    connect(tokenizeButton, &QPushButton::clicked, this, &LexerWidget::onTokenizeClicked);
    connect(clearButton, &QPushButton::clicked, this, &LexerWidget::onClearClicked);
}

void LexerWidget::onTokenizeClicked() {
    const QString sourceCode = inputTextEdit->toPlainText();

    if (sourceCode.trimmed().isEmpty()) {
        AppTheme::showWarningDialog(this, "Empty input", "Please enter source code to tokenize.");
        return;
    }

    if (!lexer) {
        AppTheme::showErrorDialog(this, "Lexer unavailable", "The lexer is not initialized.");
        return;
    }

    lexer->setSkipWhitespace(skipWhitespaceCheckBox->isChecked());
    lexer->setSkipComments(skipCommentsCheckBox->isChecked());

    const bool success = lexer->tokenize(sourceCode);
    const QVector<Token> tokens = lexer->getTokens();
    const QVector<LexerError> errors = lexer->getErrors();
    int visibleTokenCount = 0;
    for (const Token& token : tokens) {
        if (token.getType() != TokenType::END_OF_FILE) {
            ++visibleTokenCount;
        }
    }

    displayTokens(tokens);
    displayErrors(errors);

    if (success) {
        setStatusMessage(QString("Tokenized %1 token%2")
                             .arg(visibleTokenCount)
                             .arg(visibleTokenCount == 1 ? "" : "s"),
                         "success");
    } else {
        setStatusMessage(QString("%1 lexical error%2 found")
                             .arg(errors.size())
                             .arg(errors.size() == 1 ? "" : "s"),
                         "error");
    }
}

void LexerWidget::onClearClicked() {
    inputTextEdit->clear();
    tokensTable->setRowCount(0);
    errorTextEdit->clear();
    tokensStack->setCurrentIndex(0);
    diagnosticsStack->setCurrentIndex(0);
    setStatusMessage("Ready");

    if (lexer) {
        lexer->reset();
    }
}

void LexerWidget::displayTokens(const QVector<Token>& tokens) {
    tokensTable->setRowCount(0);

    int row = 0;
    for (const Token& token : tokens) {
        if (token.getType() == TokenType::END_OF_FILE) {
            continue;
        }

        populateTokenRow(tokensTable, row, token);
        ++row;
    }

    tokensStack->setCurrentIndex(row > 0 ? 1 : 0);
}

void LexerWidget::displayErrors(const QVector<LexerError>& errors) {
    errorTextEdit->clear();

    if (errors.isEmpty()) {
        diagnosticsStack->setCurrentIndex(0);
        return;
    }

    QString diagnostics;
    for (const LexerError& error : errors) {
        diagnostics += QString("[error] %1\n").arg(error.toString());
    }

    errorTextEdit->setPlainText(diagnostics.trimmed());
    diagnosticsStack->setCurrentIndex(1);
}

void LexerWidget::setStatusMessage(const QString& message, const QString& tone) {
    QString color = AppTheme::MutedText;
    if (tone == "success") {
        color = AppTheme::SecondaryText;
    } else if (tone == "error") {
        color = AppTheme::Danger;
    }

    statusLabel->setText(message);
    statusLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(color));
}
