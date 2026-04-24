#ifndef PARSERWIDGET_H
#define PARSERWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QFrame>
#include <QStackedWidget>
#include <QDialog>
#include "./src/models/Grammar/Grammar.h"
#include "./src/utils/Grammar/Parser.h"
#include "ParseTreeWidget.h"
#include "./src/utils/LexicalAnalysis/Lexer.h"
#include "./src/utils/LexicalAnalysis/AutomatonManager.h"

class ParserWidget : public QWidget {
    Q_OBJECT

private:
    
    QComboBox* grammarSelector;
    QPushButton* loadGrammarBtn;
    QPushButton* addProductionBtn;
    QPushButton* deleteProductionBtn;
    QPushButton* clearGrammarBtn;

    QListWidget* productionsList;
    QLineEdit* productionInput;

    QTextEdit* inputTextEdit;
    QPushButton* parseButton;
    QPushButton* clearButton;

    QTextEdit* grammarInfoText;
    QTextEdit* parseOutputText;
    QStackedWidget* productionsStack;
    QStackedWidget* grammarInfoStack;
    QStackedWidget* parseOutputStack;
    QPushButton* seeTreeButton;
    QLabel* treeStateLabel;

    QLabel* statusLabel;

    
    Grammar* currentGrammar;
    Parser* parser;
    Lexer* lexer;
    AutomatonManager* automatonManager;
    ParseTree currentParseTree;

public:
    explicit ParserWidget(QWidget *parent = nullptr);
    ~ParserWidget();

    void setAutomatonManager(AutomatonManager* manager);

private slots:
    void onLoadGrammar();
    void onAddProduction();
    void onClearGrammar();
    void onParseClicked();
    void onClearClicked();
    void onProductionSelected(int index);
    void onDeleteProduction();
    void onSeeTreeClicked();

private:
    void setupUI();
    void createConnections();
    void updateGrammarDisplay();
    void updateProductionsList();
    void displayParseResult(const ParseTree& tree);
    void displayParseErrors(const QVector<ParseError>& errors);
    void setStatusMessage(const QString& message, const QString& tone = "neutral");

    void loadPredefinedGrammar(const QString& grammarName);
};

#endif 
