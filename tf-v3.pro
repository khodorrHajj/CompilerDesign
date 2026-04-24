QT += core gui widgets network

CONFIG += c++17

TARGET = CompilerProject
TEMPLATE = app

SRCDIR = $$PWD/src

SOURCES += \
    $$SRCDIR/main.cpp \
    $$SRCDIR/models/Automaton/State.cpp \
    $$SRCDIR/models/Automaton/Transition.cpp \
    $$SRCDIR/models/Automaton/Automaton.cpp \
    $$SRCDIR/models/Grammar/Grammar.cpp \
    $$SRCDIR/ui/MainWindow.cpp \
    $$SRCDIR/ui/AppTheme.cpp \
    $$SRCDIR/ui/Automaton/AutomatonCanvas.cpp \
    $$SRCDIR/utils/Automaton/NFAtoDFA.cpp \
    $$SRCDIR/utils/Automaton/DFAMinimizer.cpp \
    $$SRCDIR/utils/Automaton/RegexToNFA.cpp \
    $$SRCDIR/utils/Grammar/Parser.cpp \
    $$SRCDIR/utils/Semantic/SemanticAnalyzer.cpp \
    $$SRCDIR/utils/Semantic/CodeGenerator.cpp \
    $$SRCDIR/models/Grammar/ParseTree.cpp \
    $$SRCDIR/models/Grammar/Production.cpp \
    $$SRCDIR/models/LexicalAnalysis/Token.cpp \
    $$SRCDIR/models/Semantic/SymbolTable.cpp \
    $$SRCDIR/models/Semantic/ASTNode.cpp \
    $$SRCDIR/ui/Grammar/ParseTreeWidget.cpp \
    $$SRCDIR/ui/Grammar/ParserWidget.cpp \
    $$SRCDIR/ui/LexicalAnalysis/LexerWidget.cpp \
    $$SRCDIR/ui/Semantic/SemanticAnalyzerWidget.cpp \
    $$SRCDIR/ui/SyntaxHighlighter.cpp \
    $$SRCDIR/utils/LexicalAnalysis/AutomatonManager.cpp \
    $$SRCDIR/utils/LexicalAnalysis/Lexer.cpp \


HEADERS += \
    $$SRCDIR/models/Automaton/State.h \
    $$SRCDIR/models/Automaton/Transition.h \
    $$SRCDIR/models/Automaton/Automaton.h \
    $$SRCDIR/models/Grammar/Grammar.h \
    $$SRCDIR/ui/MainWindow.h \
    $$SRCDIR/ui/AppTheme.h \
    $$SRCDIR/ui/Automaton/AutomatonCanvas.h \
    $$SRCDIR/utils/Automaton/NFAtoDFA.h \
    $$SRCDIR/utils/Automaton/DFAMinimizer.h \
    $$SRCDIR/utils/Automaton/RegexToNFA.h \
    $$SRCDIR/utils/Semantic/SemanticAnalyzer.h \
    $$SRCDIR/utils/Semantic/CodeGenerator.h \
    $$SRCDIR/utils/Grammar/Parser.h \
    $$SRCDIR/models/Grammar/ParseTree.h \
    $$SRCDIR/models/Grammar/Production.h \
    $$SRCDIR/models/LexicalAnalysis/Token.h \
    $$SRCDIR/models/Semantic/SymbolTable.h \
    $$SRCDIR/models/Semantic/ASTNode.h \
    $$SRCDIR/ui/Grammar/ParseTreeWidget.h \
    $$SRCDIR/ui/Grammar/ParserWidget.h \
    $$SRCDIR/ui/LexicalAnalysis/LexerWidget.h \
    $$SRCDIR/ui/Semantic/SemanticAnalyzerWidget.h \
    $$SRCDIR/ui/SyntaxHighlighter.h \
    $$SRCDIR/utils/LexicalAnalysis/AutomatonManager.h \
    $$SRCDIR/utils/LexicalAnalysis/Lexer.h \


INCLUDEPATH += \
    $$SRCDIR \
    $$SRCDIR/models \
    $$SRCDIR/ui \
    $$SRCDIR/utils \




