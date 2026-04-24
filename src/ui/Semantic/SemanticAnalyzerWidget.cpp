#include "SemanticAnalyzerWidget.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QTextBlock>
#include <QTextDocument>
#include <QVBoxLayout>

#include "../AppTheme.h"
#include "../src/ui/SyntaxHighlighter.h"

namespace {

QFrame* makePanel() {
    QFrame* panel = new QFrame();
    panel->setObjectName("WorkbenchPanel");
    return panel;
}

QFrame* makeToolbar() {
    QFrame* panel = new QFrame();
    panel->setObjectName("WorkbenchToolbar");
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

QWidget* makeEmptyState(const QString& title, const QString& description) {
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(6);
    layout->addStretch();

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(QString("color: %1; font-weight: 600;").arg(AppTheme::Text));
    layout->addWidget(titleLabel);

    QLabel* descriptionLabel = makeMutedLabel(description, "EmptyText");
    descriptionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(descriptionLabel);

    layout->addStretch();
    return page;
}

void polishButton(QPushButton* button) {
    button->style()->unpolish(button);
    button->style()->polish(button);
}

QTableWidgetItem* makeTableItem(const QString& text,
                                const QColor& fg,
                                const QColor& bg) {
    QTableWidgetItem* item = new QTableWidgetItem(text);
    item->setForeground(fg);
    item->setBackground(bg);
    return item;
}

}

SemanticAnalyzerWidget::SemanticAnalyzerWidget(QWidget *parent)
    : QWidget(parent),
      sourceCodeEdit(nullptr),
      analyzeButton(nullptr),
      generateCodeButton(nullptr),
      clearButton(nullptr),
      copyCodeButton(nullptr),
      targetLanguageCombo(nullptr),
      symbolTableWidget(nullptr),
      errorsWarningsText(nullptr),
      translatedCodeEdit(nullptr),
      symbolTableStack(nullptr),
      diagnosticsStack(nullptr),
      generatedCodeStack(nullptr),
      astStack(nullptr),
      astTreeWidget(nullptr),
      statusLabel(nullptr),
      semanticAnalyzer(new SemanticAnalyzer()),
      codeGenerator(new CodeGenerator()),
      lexer(new Lexer()),
      automatonManager(nullptr),
      syntaxHighlighter(nullptr) {
    setupUI();
    createConnections();
}

SemanticAnalyzerWidget::~SemanticAnalyzerWidget() {
    delete semanticAnalyzer;
    delete codeGenerator;
    delete lexer;
}

void SemanticAnalyzerWidget::setAutomatonManager(AutomatonManager* manager) {
    automatonManager = manager;
    if (lexer) {
        lexer->setAutomatonManager(manager);
    }
}

void SemanticAnalyzerWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 12, 14, 12);
    mainLayout->setSpacing(10);

    QFrame* header = makeToolbar();
    QVBoxLayout* headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(14, 12, 14, 12);
    headerLayout->setSpacing(10);

    QHBoxLayout* headerRow = new QHBoxLayout();
    headerRow->setSpacing(10);

    QVBoxLayout* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    QLabel* titleLabel = new QLabel("Semantic Analysis");
    titleLabel->setObjectName("WorkbenchTitle");
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(makeMutedLabel("Analyze symbols, inspect the AST, and generate target code."));
    headerRow->addLayout(titleLayout, 1);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(8);
    analyzeButton = new QPushButton("Analyze");
    analyzeButton->setProperty("class", "primary");
    generateCodeButton = new QPushButton("Generate Code");
    generateCodeButton->setProperty("class", "secondary");
    generateCodeButton->setEnabled(false);
    clearButton = new QPushButton("Clear");
    clearButton->setProperty("class", "secondary");
    polishButton(analyzeButton);
    polishButton(generateCodeButton);
    polishButton(clearButton);
    actionLayout->addWidget(analyzeButton);
    actionLayout->addWidget(generateCodeButton);
    actionLayout->addWidget(clearButton);
    headerRow->addLayout(actionLayout);

    headerLayout->addLayout(headerRow);
    mainLayout->addWidget(header);

    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setChildrenCollapsible(false);
    mainSplitter->setHandleWidth(6);

    QSplitter* leftSplitter = new QSplitter(Qt::Vertical);
    leftSplitter->setChildrenCollapsible(false);
    leftSplitter->setHandleWidth(6);

    QFrame* sourcePanel = makePanel();
    QVBoxLayout* sourceLayout = new QVBoxLayout(sourcePanel);
    sourceLayout->setContentsMargins(14, 12, 14, 14);
    sourceLayout->setSpacing(8);
    sourceLayout->addWidget(makeSectionTitle("Source"));

    sourceCodeEdit = new QTextEdit();
    sourceCodeEdit->setObjectName("CodeEditor");
    sourceCodeEdit->setPlaceholderText("Enter your source code here...");
    sourceCodeEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    sourceCodeEdit->setText(
        "int x = 10;\n"
        "float y = 3.14;\n"
        "int z = x + 5;\n"
        "char c = 'A';\n"
        "bool flag = true;"
    );
    syntaxHighlighter = new SyntaxHighlighter(sourceCodeEdit->document());
    sourceLayout->addWidget(sourceCodeEdit, 1);
    leftSplitter->addWidget(sourcePanel);

    QSplitter* leftLowerSplitter = new QSplitter(Qt::Horizontal);
    leftLowerSplitter->setChildrenCollapsible(false);
    leftLowerSplitter->setHandleWidth(6);

    QFrame* symbolPanel = makePanel();
    QVBoxLayout* symbolLayout = new QVBoxLayout(symbolPanel);
    symbolLayout->setContentsMargins(14, 12, 14, 14);
    symbolLayout->setSpacing(8);
    symbolLayout->addWidget(makeSectionTitle("Symbol Table"));

    symbolTableStack = new QStackedWidget();
    symbolTableStack->addWidget(makeEmptyState("No symbols yet.",
                                               "Run semantic analysis to populate the symbol table."));

    symbolTableWidget = new QTableWidget();
    symbolTableWidget->setObjectName("WorkbenchTable");
    symbolTableWidget->setColumnCount(5);
    symbolTableWidget->setHorizontalHeaderLabels({"Name", "Type", "Value", "Scope", "Status"});
    symbolTableWidget->horizontalHeader()->setStretchLastSection(false);
    symbolTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    symbolTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    symbolTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    symbolTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    symbolTableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    symbolTableWidget->horizontalHeader()->setHighlightSections(false);
    symbolTableWidget->verticalHeader()->setVisible(false);
    symbolTableWidget->verticalHeader()->setDefaultSectionSize(26);
    symbolTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    symbolTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    symbolTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    symbolTableWidget->setAlternatingRowColors(false);
    symbolTableStack->addWidget(symbolTableWidget);
    symbolLayout->addWidget(symbolTableStack, 1);
    leftLowerSplitter->addWidget(symbolPanel);

    QFrame* astPanel = makePanel();
    QVBoxLayout* astLayout = new QVBoxLayout(astPanel);
    astLayout->setContentsMargins(14, 12, 14, 14);
    astLayout->setSpacing(8);
    astLayout->addWidget(makeSectionTitle("AST"));

    astStack = new QStackedWidget();
    astStack->addWidget(makeEmptyState("No AST generated.",
                                       "Run analysis to inspect the abstract syntax tree."));

    astTreeWidget = new QTreeWidget();
    astTreeWidget->setObjectName("WorkbenchTree");
    astTreeWidget->setHeaderLabel("AST Structure");
    astTreeWidget->setAlternatingRowColors(false);
    astTreeWidget->setUniformRowHeights(true);
    astTreeWidget->header()->setStretchLastSection(true);
    astStack->addWidget(astTreeWidget);
    astLayout->addWidget(astStack, 1);
    leftLowerSplitter->addWidget(astPanel);

    leftLowerSplitter->setStretchFactor(0, 5);
    leftLowerSplitter->setStretchFactor(1, 4);
    leftLowerSplitter->setSizes({430, 360});
    leftSplitter->addWidget(leftLowerSplitter);
    leftSplitter->setStretchFactor(0, 5);
    leftSplitter->setStretchFactor(1, 4);
    leftSplitter->setSizes({420, 320});

    QSplitter* rightSplitter = new QSplitter(Qt::Vertical);
    rightSplitter->setChildrenCollapsible(false);
    rightSplitter->setHandleWidth(6);

    QFrame* diagnosticsPanel = makePanel();
    QVBoxLayout* diagnosticsLayout = new QVBoxLayout(diagnosticsPanel);
    diagnosticsLayout->setContentsMargins(14, 12, 14, 14);
    diagnosticsLayout->setSpacing(8);
    diagnosticsLayout->addWidget(makeSectionTitle("Diagnostics"));

    diagnosticsStack = new QStackedWidget();
    diagnosticsStack->addWidget(makeEmptyState("No diagnostics.", "Analysis errors and warnings appear here."));

    errorsWarningsText = new QTextEdit();
    errorsWarningsText->setObjectName("DiagnosticsView");
    errorsWarningsText->setReadOnly(true);
    errorsWarningsText->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    diagnosticsStack->addWidget(errorsWarningsText);
    diagnosticsLayout->addWidget(diagnosticsStack, 1);
    rightSplitter->addWidget(diagnosticsPanel);

    QFrame* generatedCodePanel = makePanel();
    QVBoxLayout* generatedCodeLayout = new QVBoxLayout(generatedCodePanel);
    generatedCodeLayout->setContentsMargins(14, 12, 14, 14);
    generatedCodeLayout->setSpacing(8);
    generatedCodeLayout->addWidget(makeSectionTitle("Generated Code"));

    QHBoxLayout* langLayout = new QHBoxLayout();
    langLayout->setSpacing(8);
    QLabel* targetLabel = makeMutedLabel("Target");
    targetLabel->setWordWrap(false);
    langLayout->addWidget(targetLabel);

    targetLanguageCombo = new QComboBox();
    targetLanguageCombo->addItem("Python", QVariant::fromValue(TargetLanguage::PYTHON));
    targetLanguageCombo->addItem("Java", QVariant::fromValue(TargetLanguage::JAVA));
    targetLanguageCombo->addItem("JavaScript", QVariant::fromValue(TargetLanguage::JAVASCRIPT));
    targetLanguageCombo->addItem("Assembly", QVariant::fromValue(TargetLanguage::ASSEMBLY));
    langLayout->addWidget(targetLanguageCombo, 1);
    generatedCodeLayout->addLayout(langLayout);

    generatedCodeStack = new QStackedWidget();
    generatedCodeStack->addWidget(makeEmptyState("No code generated.",
                                                 "Choose a target language and generate code."));

    translatedCodeEdit = new QTextEdit();
    translatedCodeEdit->setObjectName("CodeEditor");
    translatedCodeEdit->setReadOnly(true);
    translatedCodeEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    generatedCodeStack->addWidget(translatedCodeEdit);
    generatedCodeLayout->addWidget(generatedCodeStack, 1);

    copyCodeButton = new QPushButton("Copy Code");
    copyCodeButton->setProperty("class", "secondary");
    copyCodeButton->setEnabled(false);
    polishButton(copyCodeButton);
    generatedCodeLayout->addWidget(copyCodeButton);
    rightSplitter->addWidget(generatedCodePanel);
    rightSplitter->setStretchFactor(0, 2);
    rightSplitter->setStretchFactor(1, 3);
    rightSplitter->setSizes({240, 420});

    mainSplitter->addWidget(leftSplitter);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(0, 3);
    mainSplitter->setStretchFactor(1, 2);
    mainSplitter->setSizes({760, 460});
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

void SemanticAnalyzerWidget::createConnections() {
    connect(analyzeButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onAnalyzeClicked);
    connect(generateCodeButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onGenerateCodeClicked);
    connect(clearButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onClearClicked);
    connect(copyCodeButton, &QPushButton::clicked, this, &SemanticAnalyzerWidget::onCopyCodeClicked);
}

void SemanticAnalyzerWidget::onAnalyzeClicked() {
    const QString sourceCode = sourceCodeEdit->toPlainText().trimmed();

    if (sourceCode.isEmpty()) {
        AppTheme::showWarningDialog(this, "Empty input", "Please enter source code to analyze.");
        return;
    }

    lexer->setSkipWhitespace(true);
    lexer->setSkipComments(true);

    if (!lexer->tokenize(sourceCode)) {
        AppTheme::showErrorDialog(this,
                                  "Tokenization failed",
                                  "Failed to tokenize source code. Check for lexical errors.");
        return;
    }

    semanticAnalyzer->setTokens(lexer->getTokens());
    const bool success = semanticAnalyzer->analyzeProgram();

    displaySymbolTable();
    displayErrorsWarnings();
    displayAST();
    highlightInlineErrors();

    if (success && !semanticAnalyzer->hasErrors()) {
        generateCodeButton->setEnabled(true);
        setStatusMessage("Semantic analysis complete", "success");
    } else {
        generateCodeButton->setEnabled(false);
        setStatusMessage(QString("Semantic errors found: %1")
                             .arg(semanticAnalyzer->getErrors().size()),
                         "error");
    }
}

void SemanticAnalyzerWidget::onGenerateCodeClicked() {
    const TargetLanguage targetLang = targetLanguageCombo->currentData().value<TargetLanguage>();

    codeGenerator->setTokens(lexer->getTokens());
    codeGenerator->setSymbolTable(semanticAnalyzer->getSymbolTable());
    codeGenerator->setTargetLanguage(targetLang);
    codeGenerator->setSourceCode(sourceCodeEdit->toPlainText());

    const QString translatedCode = codeGenerator->generate();
    displayTranslatedCode(translatedCode);

    copyCodeButton->setEnabled(!translatedCode.isEmpty());
    setStatusMessage(QString("Generated %1 code").arg(targetLanguageCombo->currentText()), "success");
}

void SemanticAnalyzerWidget::onClearClicked() {
    sourceCodeEdit->clear();
    symbolTableWidget->setRowCount(0);
    errorsWarningsText->clear();
    translatedCodeEdit->clear();
    astTreeWidget->clear();
    symbolTableStack->setCurrentIndex(0);
    diagnosticsStack->setCurrentIndex(0);
    generatedCodeStack->setCurrentIndex(0);
    astStack->setCurrentIndex(0);
    generateCodeButton->setEnabled(false);
    copyCodeButton->setEnabled(false);
    setStatusMessage("Ready");
}

void SemanticAnalyzerWidget::onCopyCodeClicked() {
    const QString code = translatedCodeEdit->toPlainText();
    if (!code.isEmpty()) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(code);
        setStatusMessage("Code copied to clipboard");
    }
}

void SemanticAnalyzerWidget::displaySymbolTable() {
    symbolTableWidget->setRowCount(0);

    if (!semanticAnalyzer) {
        symbolTableStack->setCurrentIndex(0);
        return;
    }

    const QVector<Symbol> symbols = semanticAnalyzer->getDiscoveredSymbols();
    QColor primaryText(AppTheme::Text);
    QColor secondaryText(AppTheme::SecondaryText);

    for (const auto& symbol : symbols) {
        const int row = symbolTableWidget->rowCount();
        symbolTableWidget->insertRow(row);
        const QColor bgColor = (row % 2 == 0) ? QColor("#0D1116") : QColor("#10151B");

        symbolTableWidget->setItem(row, 0, makeTableItem(symbol.name, primaryText, bgColor));
        symbolTableWidget->setItem(row, 1, makeTableItem(symbol.getTypeString(), primaryText, bgColor));
        symbolTableWidget->setItem(row, 2, makeTableItem(symbol.isInitialized ? symbol.value : "(uninitialized)",
                                                         primaryText,
                                                         bgColor));
        symbolTableWidget->setItem(row, 3, makeTableItem(QString::number(symbol.scope), secondaryText, bgColor));
        symbolTableWidget->setItem(row, 4, makeTableItem(symbol.isInitialized ? "Initialized" : "Declared",
                                                         secondaryText,
                                                         bgColor));
    }

    symbolTableStack->setCurrentIndex(symbols.isEmpty() ? 0 : 1);
}

void SemanticAnalyzerWidget::displayErrorsWarnings() {
    errorsWarningsText->clear();

    const QVector<SemanticError> errors = semanticAnalyzer->getErrors();
    const QVector<SemanticError> warnings = semanticAnalyzer->getWarnings();

    if (errors.isEmpty() && warnings.isEmpty()) {
        diagnosticsStack->setCurrentIndex(0);
        return;
    }

    QString output;

    if (!errors.isEmpty()) {
        output += QString("<span style='color: %1;'><b>Errors</b></span><br>").arg(AppTheme::Danger);
        for (const auto& error : errors) {
            output += QString("<span style='color: %1;'>%2</span><br>")
                          .arg(AppTheme::Danger, error.toString().toHtmlEscaped());
        }
        output += "<br>";
    }

    if (!warnings.isEmpty()) {
        output += "<span style='color: #B9A483;'><b>Warnings</b></span><br>";
        for (const auto& warning : warnings) {
            output += QString("<span style='color: #B9A483;'>%1</span><br>")
                          .arg(warning.toString().toHtmlEscaped());
        }
    }

    errorsWarningsText->setHtml(output);
    diagnosticsStack->setCurrentIndex(1);
}

void SemanticAnalyzerWidget::displayTranslatedCode(const QString& code) {
    translatedCodeEdit->setPlainText(code);
    generatedCodeStack->setCurrentIndex(code.isEmpty() ? 0 : 1);
}

void SemanticAnalyzerWidget::displayAST() {
    astTreeWidget->clear();

    if (!semanticAnalyzer) {
        astStack->setCurrentIndex(0);
        return;
    }

    ASTNode* root = semanticAnalyzer->getAST();
    if (!root) {
        astStack->setCurrentIndex(0);
        return;
    }

    populateASTTree(nullptr, root);
    astTreeWidget->expandAll();
    astStack->setCurrentIndex(1);
}

void SemanticAnalyzerWidget::populateASTTree(QTreeWidgetItem* parent, ASTNode* node) {
    if (!node) {
        return;
    }

    QTreeWidgetItem* item = parent ? new QTreeWidgetItem(parent)
                                   : new QTreeWidgetItem(astTreeWidget);

    item->setText(0, node->toString());

    QColor color(AppTheme::Text);
    switch (node->getType()) {
    case ASTNodeType::PROGRAM:
        color = QColor(AppTheme::Text);
        break;
    case ASTNodeType::DECLARATION:
        color = QColor(AppTheme::SecondaryText);
        break;
    case ASTNodeType::ASSIGNMENT:
        color = QColor("#D0D5DB");
        break;
    case ASTNodeType::LITERAL:
        color = QColor("#B8C0CA");
        break;
    case ASTNodeType::IDENTIFIER:
        color = QColor("#C9CED5");
        break;
    default:
        break;
    }
    item->setForeground(0, color);

    for (ASTNode* child : node->getChildren()) {
        populateASTTree(item, child);
    }
}

void SemanticAnalyzerWidget::highlightInlineErrors() {
    QTextCursor cursor(sourceCodeEdit->document());
    cursor.select(QTextCursor::Document);
    QTextCharFormat clearFormat;
    clearFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
    cursor.setCharFormat(clearFormat);

    if (!semanticAnalyzer) {
        return;
    }

    const QVector<SemanticError> errors = semanticAnalyzer->getErrors();
    const QVector<SemanticError> warnings = semanticAnalyzer->getWarnings();

    for (const auto& error : errors) {
        highlightErrorAtLine(error.line, true);
    }

    for (const auto& warning : warnings) {
        highlightErrorAtLine(warning.line, false);
    }
}

void SemanticAnalyzerWidget::highlightErrorAtLine(int line, bool isError) {
    if (line <= 0) {
        return;
    }

    QTextDocument* doc = sourceCodeEdit->document();
    QTextBlock block = doc->findBlockByLineNumber(line - 1);

    if (!block.isValid()) {
        return;
    }

    QTextCursor cursor(block);
    cursor.select(QTextCursor::LineUnderCursor);

    QTextCharFormat format;
    format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    format.setUnderlineColor(isError ? QColor(AppTheme::Danger) : QColor("#B9A483"));
    cursor.mergeCharFormat(format);
}

void SemanticAnalyzerWidget::setStatusMessage(const QString& message, const QString& tone) {
    QString color = AppTheme::MutedText;
    if (tone == "success") {
        color = AppTheme::SecondaryText;
    } else if (tone == "error") {
        color = AppTheme::Danger;
    }

    statusLabel->setText(message);
    statusLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(color));
}
