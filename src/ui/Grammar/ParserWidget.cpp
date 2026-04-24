#include "ParserWidget.h"

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QSplitter>
#include <QVBoxLayout>
#include <QDialog>

#include "../AppTheme.h"

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

}

ParserWidget::ParserWidget(QWidget *parent)
    : QWidget(parent),
      grammarSelector(nullptr),
      loadGrammarBtn(nullptr),
      addProductionBtn(nullptr),
      deleteProductionBtn(nullptr),
      clearGrammarBtn(nullptr),
      productionsList(nullptr),
      productionInput(nullptr),
      inputTextEdit(nullptr),
      parseButton(nullptr),
      clearButton(nullptr),
      grammarInfoText(nullptr),
      parseOutputText(nullptr),
      productionsStack(nullptr),
      grammarInfoStack(nullptr),
      parseOutputStack(nullptr),
      seeTreeButton(nullptr),
      treeStateLabel(nullptr),
      statusLabel(nullptr),
      currentGrammar(new Grammar()),
      parser(nullptr),
      lexer(new Lexer()),
      automatonManager(nullptr),
      currentParseTree() {
    parser = new Parser(currentGrammar);

    setupUI();
    createConnections();

    loadPredefinedGrammar("Expression");
    updateGrammarDisplay();
    updateProductionsList();
    setStatusMessage("Ready");
}

ParserWidget::~ParserWidget() {
    delete currentGrammar;
    delete parser;
    delete lexer;
}

void ParserWidget::setAutomatonManager(AutomatonManager* manager) {
    automatonManager = manager;
    if (lexer) {
        lexer->setAutomatonManager(manager);
    }
}

void ParserWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(14, 12, 14, 12);
    mainLayout->setSpacing(10);

    QFrame* header = makeToolbar();
    QVBoxLayout* headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(14, 12, 14, 12);
    headerLayout->setSpacing(10);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(10);

    QVBoxLayout* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(2);
    QLabel* titleLabel = new QLabel("Parser / Parse Tree");
    titleLabel->setObjectName("WorkbenchTitle");
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(makeMutedLabel("Define a grammar, parse input, and inspect the generated parse tree."));
    topRow->addLayout(titleLayout, 1);

    QHBoxLayout* actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(8);
    parseButton = new QPushButton("Parse");
    parseButton->setProperty("class", "primary");
    clearButton = new QPushButton("Clear");
    clearButton->setProperty("class", "secondary");
    polishButton(parseButton);
    polishButton(clearButton);
    actionsLayout->addWidget(parseButton);
    actionsLayout->addWidget(clearButton);
    topRow->addLayout(actionsLayout);

    headerLayout->addLayout(topRow);
    mainLayout->addWidget(header);

    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->setChildrenCollapsible(false);
    mainSplitter->setHandleWidth(6);

    QWidget* leftPanel = new QWidget();
    leftPanel->setMinimumWidth(320);
    leftPanel->setMaximumWidth(420);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(10);

    QFrame* grammarPanel = makePanel();
    QVBoxLayout* grammarLayout = new QVBoxLayout(grammarPanel);
    grammarLayout->setContentsMargins(14, 12, 14, 14);
    grammarLayout->setSpacing(10);
    grammarLayout->addWidget(makeSectionTitle("Grammar"));

    QLabel* presetLabel = makeMutedLabel("Preset");
    presetLabel->setWordWrap(false);
    grammarLayout->addWidget(presetLabel);

    QHBoxLayout* presetLayout = new QHBoxLayout();
    presetLayout->setSpacing(8);
    grammarSelector = new QComboBox();
    grammarSelector->addItem("Expression Grammar (LL)");
    grammarSelector->addItem("Arithmetic Grammar");
    grammarSelector->addItem("Statement Grammar");
    loadGrammarBtn = new QPushButton("Load");
    loadGrammarBtn->setProperty("class", "secondary");
    polishButton(loadGrammarBtn);
    presetLayout->addWidget(grammarSelector, 1);
    presetLayout->addWidget(loadGrammarBtn);
    grammarLayout->addLayout(presetLayout);
    leftLayout->addWidget(grammarPanel);

    QFrame* productionsPanel = makePanel();
    QVBoxLayout* productionsLayout = new QVBoxLayout(productionsPanel);
    productionsLayout->setContentsMargins(14, 12, 14, 14);
    productionsLayout->setSpacing(8);
    productionsLayout->addWidget(makeSectionTitle("Productions"));

    productionsStack = new QStackedWidget();
    productionsStack->addWidget(makeEmptyState("No productions defined.", "Load a preset or add productions manually."));
    productionsList = new QListWidget();
    productionsList->setObjectName("WorkbenchList");
    productionsList->setSelectionMode(QAbstractItemView::SingleSelection);
    productionsStack->addWidget(productionsList);
    productionsLayout->addWidget(productionsStack, 1);

    productionInput = new QLineEdit();
    productionInput->setPlaceholderText("E -> T E' | id");
    productionsLayout->addWidget(productionInput);

    QHBoxLayout* productionActions = new QHBoxLayout();
    productionActions->setSpacing(8);
    addProductionBtn = new QPushButton("Add");
    addProductionBtn->setProperty("class", "primary");
    deleteProductionBtn = new QPushButton("Delete");
    deleteProductionBtn->setProperty("class", "danger");
    deleteProductionBtn->setEnabled(false);
    clearGrammarBtn = new QPushButton("Clear All");
    clearGrammarBtn->setProperty("class", "danger");
    polishButton(addProductionBtn);
    polishButton(deleteProductionBtn);
    polishButton(clearGrammarBtn);
    productionActions->addWidget(addProductionBtn);
    productionActions->addWidget(deleteProductionBtn);
    productionActions->addStretch();
    productionActions->addWidget(clearGrammarBtn);
    productionsLayout->addLayout(productionActions);
    leftLayout->addWidget(productionsPanel, 1);

    QFrame* infoPanel = makePanel();
    QVBoxLayout* infoLayout = new QVBoxLayout(infoPanel);
    infoLayout->setContentsMargins(14, 12, 14, 14);
    infoLayout->setSpacing(8);
    infoLayout->addWidget(makeSectionTitle("Grammar Information"));

    grammarInfoStack = new QStackedWidget();
    grammarInfoStack->addWidget(makeEmptyState("No grammar details available.", "Select or define a grammar to see details."));
    grammarInfoText = new QTextEdit();
    grammarInfoText->setObjectName("DiagnosticsView");
    grammarInfoText->setReadOnly(true);
    grammarInfoText->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    grammarInfoStack->addWidget(grammarInfoText);
    infoLayout->addWidget(grammarInfoStack);
    leftLayout->addWidget(infoPanel);

    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    QFrame* inputPanel = makePanel();
    QVBoxLayout* inputLayout = new QVBoxLayout(inputPanel);
    inputLayout->setContentsMargins(14, 12, 14, 14);
    inputLayout->setSpacing(8);
    inputLayout->addWidget(makeSectionTitle("Input"));

    inputTextEdit = new QTextEdit();
    inputTextEdit->setObjectName("CodeEditor");
    inputTextEdit->setMaximumHeight(88);
    inputTextEdit->setPlaceholderText("Enter expression to parse, e.g., id + id * id");
    inputTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    inputTextEdit->setText("id + id * id");
    inputLayout->addWidget(inputTextEdit);
    rightLayout->addWidget(inputPanel);

    QFrame* outputPanel = makePanel();
    QVBoxLayout* outputLayout = new QVBoxLayout(outputPanel);
    outputLayout->setContentsMargins(14, 12, 14, 14);
    outputLayout->setSpacing(8);
    outputLayout->addWidget(makeSectionTitle("Parse Output"));

    parseOutputStack = new QStackedWidget();
    parseOutputStack->addWidget(makeEmptyState("Parse output will appear here.", "Run the parser to inspect the result and diagnostics."));
    parseOutputText = new QTextEdit();
    parseOutputText->setObjectName("DiagnosticsView");
    parseOutputText->setReadOnly(true);
    parseOutputText->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    parseOutputStack->addWidget(parseOutputText);
    outputLayout->addWidget(parseOutputStack);
    rightLayout->addWidget(outputPanel);

    QFrame* treePanel = makePanel();
    QVBoxLayout* treeLayout = new QVBoxLayout(treePanel);
    treeLayout->setContentsMargins(14, 12, 14, 14);
    treeLayout->setSpacing(8);
    treeLayout->addWidget(makeSectionTitle("Parse Tree"));

    treeStateLabel = makeMutedLabel("No parse tree to display.\nParse an input string to generate a tree.");
    treeStateLabel->setAlignment(Qt::AlignCenter);
    treeStateLabel->setMinimumHeight(88);
    treeLayout->addWidget(treeStateLabel, 1);

    seeTreeButton = new QPushButton("See Tree");
    seeTreeButton->setProperty("class", "secondary");
    seeTreeButton->setEnabled(false);
    polishButton(seeTreeButton);
    treeLayout->addWidget(seeTreeButton);
    rightLayout->addWidget(treePanel);

    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setSizes({360, 860});
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

void ParserWidget::createConnections() {
    connect(loadGrammarBtn, &QPushButton::clicked, this, &ParserWidget::onLoadGrammar);
    connect(addProductionBtn, &QPushButton::clicked, this, &ParserWidget::onAddProduction);
    connect(deleteProductionBtn, &QPushButton::clicked, this, &ParserWidget::onDeleteProduction);
    connect(clearGrammarBtn, &QPushButton::clicked, this, &ParserWidget::onClearGrammar);
    connect(parseButton, &QPushButton::clicked, this, &ParserWidget::onParseClicked);
    connect(clearButton, &QPushButton::clicked, this, &ParserWidget::onClearClicked);
    connect(seeTreeButton, &QPushButton::clicked, this, &ParserWidget::onSeeTreeClicked);
    connect(productionInput, &QLineEdit::returnPressed, this, &ParserWidget::onAddProduction);
    connect(productionsList, &QListWidget::currentRowChanged, this, &ParserWidget::onProductionSelected);
}

void ParserWidget::onLoadGrammar() {
    const QString selected = grammarSelector->currentText();

    if (selected.contains("Expression")) {
        loadPredefinedGrammar("Expression");
    } else if (selected.contains("Arithmetic")) {
        loadPredefinedGrammar("Arithmetic");
    } else if (selected.contains("Statement")) {
        loadPredefinedGrammar("Statement");
    } else {
        currentGrammar->clear();
        currentGrammar->setName("Custom Grammar");
        currentGrammar->setStartSymbol("S");
    }

    updateGrammarDisplay();
    updateProductionsList();
    setStatusMessage(QString("Grammar loaded: %1").arg(currentGrammar->getName()));
}

void ParserWidget::loadPredefinedGrammar(const QString& grammarName) {
    delete currentGrammar;

    if (grammarName == "Expression") {
        currentGrammar = new Grammar(Grammar::createExpressionGrammar());
    } else if (grammarName == "Arithmetic") {
        currentGrammar = new Grammar(Grammar::createArithmeticGrammar());
    } else if (grammarName == "Statement") {
        currentGrammar = new Grammar(Grammar::createSimpleStatementGrammar());
    } else {
        currentGrammar = new Grammar("Custom", "S");
    }

    parser->setGrammar(currentGrammar);
}

void ParserWidget::onAddProduction() {
    const QString prodStr = productionInput->text().trimmed();

    if (prodStr.isEmpty()) {
        AppTheme::showWarningDialog(this, "Empty production", "Please enter a production rule.");
        return;
    }

    const Production prod = Production::fromString(prodStr);

    if (prod.getNonTerminal().isEmpty()) {
        AppTheme::showWarningDialog(this,
                                    "Invalid production",
                                    "Invalid production format.\n\nUse: A -> B C");
        return;
    }

    currentGrammar->addProduction(prod);
    updateGrammarDisplay();
    updateProductionsList();

    productionInput->clear();
    setStatusMessage(QString("Production added: %1").arg(prod.toString()));
}

void ParserWidget::onDeleteProduction() {
    const int currentRow = productionsList->currentRow();
    if (currentRow >= 0) {
        currentGrammar->removeProduction(currentRow);
        updateGrammarDisplay();
        updateProductionsList();
        setStatusMessage("Production deleted", "error");
    }
}

void ParserWidget::onClearGrammar() {
    if (AppTheme::showConfirmDialog(this,
                                    "Clear grammar?",
                                    "This will remove all productions from the current grammar.",
                                    "Clear",
                                    "Cancel",
                                    true)) {
        currentGrammar->clear();
        updateGrammarDisplay();
        updateProductionsList();
        setStatusMessage("Grammar cleared", "error");
    }
}

void ParserWidget::onParseClicked() {
    if (!currentGrammar || currentGrammar->getProductions().isEmpty()) {
        AppTheme::showWarningDialog(this, "No grammar", "Please load or define a grammar first.");
        return;
    }

    const QString input = inputTextEdit->toPlainText().trimmed();

    if (input.isEmpty()) {
        AppTheme::showWarningDialog(this, "Empty input", "Please enter input to parse.");
        return;
    }

    lexer->setSkipWhitespace(true);
    lexer->setSkipComments(true);

    if (!lexer->tokenize(input)) {
        parseOutputText->clear();
        parseOutputStack->setCurrentIndex(1);
        currentParseTree = ParseTree();
        seeTreeButton->setEnabled(false);
        treeStateLabel->setText("No parse tree to display.\nParse an input string to generate a tree.");
        displayParseErrors(QVector<ParseError>());
        parseOutputText->append("<span style='color: #F2A0A0;'>Tokenization failed.</span>");
        setStatusMessage("Parse failed", "error");
        return;
    }

    const QVector<Token> tokens = lexer->getTokens();
    parser->setTokens(tokens);

    const ParseTree tree = parser->parse();

    displayParseResult(tree);

    if (parser->hasErrors()) {
        displayParseErrors(parser->getErrors());
    }
}

void ParserWidget::onClearClicked() {
    inputTextEdit->clear();
    parseOutputText->clear();
    parseOutputStack->setCurrentIndex(0);
    currentParseTree = ParseTree();
    seeTreeButton->setEnabled(false);
    treeStateLabel->setText("No parse tree to display.\nParse an input string to generate a tree.");
    setStatusMessage("Ready");
}

void ParserWidget::onSeeTreeClicked() {
    if (currentParseTree.isEmpty()) {
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Parse Tree");
    dialog.setModal(true);
    dialog.resize(900, 640);
    dialog.setStyleSheet(AppTheme::dialogStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(16, 16, 16, 14);
    layout->setSpacing(10);

    QLabel* titleLabel = new QLabel("Parse Tree");
    titleLabel->setObjectName("WorkbenchTitle");
    layout->addWidget(titleLabel);
    layout->addWidget(makeMutedLabel("Inspect the generated parse tree in a larger view."));

    ParseTreeWidget* treeWidget = new ParseTreeWidget(&dialog);
    treeWidget->setParseTree(currentParseTree);
    layout->addWidget(treeWidget, 1);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();
    QPushButton* closeButton = new QPushButton("Close");
    closeButton->setProperty("class", "secondary");
    polishButton(closeButton);
    buttons->addWidget(closeButton);
    layout->addLayout(buttons);

    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

void ParserWidget::onProductionSelected(int index) {
    deleteProductionBtn->setEnabled(index >= 0);
}

void ParserWidget::updateGrammarDisplay() {
    if (!currentGrammar || currentGrammar->getProductions().isEmpty()) {
        grammarInfoText->clear();
        grammarInfoStack->setCurrentIndex(0);
        return;
    }

    QString info;
    info += QString("<span style='color: %1;'><b>Grammar</b></span><br>").arg(AppTheme::Text);
    info += QString("<span style='color: %1;'>Name:</span> <span style='color: %2;'>%3</span><br>")
                .arg(AppTheme::SecondaryText, AppTheme::Text, currentGrammar->getName());
    info += QString("<span style='color: %1;'>Start Symbol:</span> <span style='color: %2;'>%3</span><br><br>")
                .arg(AppTheme::SecondaryText, AppTheme::Text, currentGrammar->getStartSymbol());

    const QSet<QString> terminals = currentGrammar->getTerminals();
    const QSet<QString> nonTerminals = currentGrammar->getNonTerminals();

    info += QString("<span style='color: %1;'>Non-Terminals:</span><br><span style='color: %2;'>{ %3 }</span><br><br>")
                .arg(AppTheme::SecondaryText, AppTheme::Text, QStringList(nonTerminals.values()).join(", "));
    info += QString("<span style='color: %1;'>Terminals:</span><br><span style='color: %2;'>{ %3 }</span><br><br>")
                .arg(AppTheme::SecondaryText, AppTheme::Text, QStringList(terminals.values()).join(", "));
    info += QString("<span style='color: %1;'>Total Productions:</span> <span style='color: %2;'>%3</span>")
                .arg(AppTheme::SecondaryText, AppTheme::Text)
                .arg(currentGrammar->getProductions().size());

    grammarInfoText->setHtml(info);
    grammarInfoStack->setCurrentIndex(1);
}

void ParserWidget::updateProductionsList() {
    productionsList->clear();

    if (!currentGrammar) {
        productionsStack->setCurrentIndex(0);
        return;
    }

    for (const auto& prod : currentGrammar->getProductions()) {
        productionsList->addItem(prod.toString());
    }

    const bool hasProductions = productionsList->count() > 0;
    productionsStack->setCurrentIndex(hasProductions ? 1 : 0);
    deleteProductionBtn->setEnabled(hasProductions && productionsList->currentRow() >= 0);
}

void ParserWidget::displayParseResult(const ParseTree& tree) {
    parseOutputText->clear();
    parseOutputStack->setCurrentIndex(1);
    currentParseTree = tree;

    QString output;
    output += QString("<span style='color: %1;'><b>Parsing Result</b></span><br><br>").arg(AppTheme::Text);
    output += QString("<span style='color: %1;'>Input:</span> <span style='color: %2;'>%3</span><br>")
                  .arg(AppTheme::SecondaryText, AppTheme::Text, inputTextEdit->toPlainText().toHtmlEscaped());
    output += QString("<span style='color: %1;'>Grammar:</span> <span style='color: %2;'>%3</span><br><br>")
                  .arg(AppTheme::SecondaryText, AppTheme::Text, currentGrammar->getName().toHtmlEscaped());

    if (tree.isEmpty()) {
        output += "<span style='color: #F2A0A0;'><b>Parse failed</b></span><br>";
        seeTreeButton->setEnabled(false);
        treeStateLabel->setText("No parse tree to display.\nParse an input string to generate a tree.");
        setStatusMessage("Parse failed", "error");
    } else {
        output += "<span style='color: #A8B0BA;'><b>Parse successful</b></span><br><br>";
        output += QString("<span style='color: %1;'>Parse tree generated and ready to inspect.</span><br>")
                      .arg(AppTheme::SecondaryText);

        seeTreeButton->setEnabled(true);
        treeStateLabel->setText("Parse tree generated.\nClick See Tree to open the visualization.");
        setStatusMessage("Parse successful", "success");
    }

    parseOutputText->setHtml(output);
}

void ParserWidget::displayParseErrors(const QVector<ParseError>& errors) {
    if (errors.isEmpty()) {
        return;
    }

    QString errorOutput = QString("<br><span style='color: %1;'><b>Parse Errors</b></span><br>")
                              .arg(AppTheme::Danger);

    for (const auto& error : errors) {
        errorOutput += QString("<span style='color: %1;'>- %2</span><br>")
                           .arg(AppTheme::Danger, error.toString().toHtmlEscaped());
    }

    parseOutputText->append(errorOutput);
}

void ParserWidget::setStatusMessage(const QString& message, const QString& tone) {
    QString color = AppTheme::MutedText;
    if (tone == "success") {
        color = AppTheme::SecondaryText;
    } else if (tone == "error") {
        color = AppTheme::Danger;
    }

    statusLabel->setText(message);
    statusLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(color));
}
