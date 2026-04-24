#include "MainWindow.h" 
#include "AppTheme.h"
#include "./src/utils/Automaton/NFAtoDFA.h" 
#include "./src/utils/Automaton/DFAMinimizer.h" 
#include "./src/utils/Automaton/RegexToNFA.h" 
#include <QInputDialog> 
#include <QFileDialog>  
#include <QDialog>      
#include <QCheckBox>    
#include <QtMath>       
#include <QScrollArea>  
#include <QButtonGroup> 
#include <QHeaderView>  
#include <QDebug>       
#include <QShortcut>    
#include <QDialog>      
#include <QFrame>
#include <QSizePolicy>
#include <QGridLayout>
#include <QDialogButtonBox>

namespace {

QFrame* makePanel(const char* objectName) {
    QFrame* panel = new QFrame();
    panel->setObjectName(objectName);
    return panel;
}

QFrame* makeDivider() {
    QFrame* divider = makePanel("SectionDivider");
    divider->setFixedHeight(1);
    return divider;
}

QFrame* makeSection(const QString& title, QVBoxLayout** contentLayout) {
    QFrame* section = makePanel("SectionGroup");
    QVBoxLayout* outer = new QVBoxLayout(section);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(8);

    QLabel* label = new QLabel(title);
    label->setObjectName("SectionTitle");
    outer->addWidget(label);

    *contentLayout = new QVBoxLayout();
    (*contentLayout)->setSpacing(6);
    (*contentLayout)->setContentsMargins(0, 0, 0, 0);
    outer->addLayout(*contentLayout);
    return section;
}

QLabel* makeMutedLabel(const QString& text) {
    QLabel* label = new QLabel(text);
    label->setObjectName("MutedText");
    label->setWordWrap(true);
    return label;
}

void setButtonClass(QPushButton* button, const char* className) {
    button->setProperty("class", className);
    button->style()->unpolish(button);
    button->style()->polish(button);
}

QFrame* makeStatCard(QLabel* valueLabel, const QString& caption) {
    QFrame* card = makePanel("StatCard");
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(9, 7, 9, 7);
    layout->setSpacing(2);

    valueLabel->setTextFormat(Qt::RichText);
    valueLabel->setStyleSheet("font-size: 10pt; font-weight: 700; background: transparent;");
    QLabel* captionLabel = makeMutedLabel(caption);
    captionLabel->setStyleSheet("font-size: 8pt; background: transparent;");

    layout->addWidget(valueLabel);
    layout->addWidget(captionLabel);
    return card;
}

}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    
    
    currentAutomaton(nullptr), automatonCounter(0),
    currentSelectedStateId(""), automatonManager(nullptr), canvas(nullptr),
    centralTabs(nullptr), automatonTab(nullptr), lexerWidget(nullptr),
    toolsDock(nullptr), automatonListDock(nullptr), propertiesDock(nullptr),
    automatonList(nullptr),
    typeLabel(nullptr), stateCountLabel(nullptr), transitionCountLabel(nullptr),
    alphabetLabel(nullptr), selectedStateLabel(nullptr), deleteStateBtn(nullptr),
    transitionTable(nullptr), convertNFAtoDFABtn(nullptr), minimizeDFABtn(nullptr),
    selectModeBtn(nullptr), addStateModeBtn(nullptr), addTransitionModeBtn(nullptr),
    deleteModeBtn(nullptr), clearCanvasBtn(nullptr), newAutomatonBtn(nullptr),
    deleteAutomatonBtn(nullptr), renameAutomatonBtn(nullptr),
    newAction(nullptr), openAction(nullptr), saveAction(nullptr), exitAction(nullptr),
    convertAction(nullptr), minimizeAction(nullptr), aboutAction(nullptr),
    selectAction(nullptr), addStateAction(nullptr), addTransitionAction(nullptr),
    deleteAction(nullptr),
    
    playAction(nullptr), stepAction(nullptr), stopAction(nullptr), resetAction(nullptr),
    speedSlider(nullptr), speedLabel(nullptr), simulationTimer(nullptr),
    isSimulating(false), simulationStepIndex(0), simulationAccepted(false), simulationRejected(false) {

    
    setWindowTitle("Compiler Project");
    resize(1200, 700);
    setMinimumSize(1000, 650); 
    setStyleSheet(AppTheme::styleSheet());

    
    automatonManager = new AutomatonManager();
    if (!automatonManager) {
        qCritical() << "Failed to create AutomatonManager!"; 
        return; 
    }

    
    
    setupCentralTabs();

    
    QShortcut* saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::onSave);

    
    createDockWidgets();
    createToolbar(); 

    
    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::onSimulationTimerTimeout);

    
    
    if (canvas) {
        connect(canvas, &AutomatonCanvas::automatonModified,
                this, &MainWindow::onAutomatonModified);
        connect(canvas, &AutomatonCanvas::stateSelected,
                this, &MainWindow::onStateSelected);
    } else {
        qWarning() << "Canvas is null - connections failed"; 
    }

    updateProperties();

    
    statusBar()->showMessage("Ready");
}




MainWindow::~MainWindow() {
    
    
    
    if (canvas) {
        canvas->disconnect();
    }

    if (centralTabs) {
        centralTabs->disconnect();
    }

    
    
    for (auto automaton : automatons) {
        if (automaton) {
            delete automaton; 
        }
    }
    automatons.clear(); 

    
    currentAutomaton = nullptr;

    
    
    
    
    if (automatonManager) {
        delete automatonManager;
        automatonManager = nullptr;
    }

    
    
    
    
}























































void MainWindow::createToolbar() {
    QToolBar* toolbar = addToolBar("Simulation");
    toolbar->setObjectName("CommandBar");
    toolbar->setMovable(false);
    toolbar->setFloatable(false);

    QLabel* title = new QLabel("Compiler Project");
    title->setObjectName("AppTitle");
    toolbar->addWidget(title);

    
    playAction = new QAction(QIcon(), "Play", this);
    playAction->setToolTip("Start Simulation");
    connect(playAction, &QAction::triggered, this, &MainWindow::onPlaySimulation);
    toolbar->addAction(playAction);

    
    stepAction = new QAction(QIcon(), "Step", this);
    stepAction->setToolTip("Step Forward");
    connect(stepAction, &QAction::triggered, this, &MainWindow::onStepSimulation);
    toolbar->addAction(stepAction);

    
    stopAction = new QAction(QIcon(), "Stop", this);
    stopAction->setToolTip("Stop Simulation");
    stopAction->setEnabled(false);
    connect(stopAction, &QAction::triggered, this, &MainWindow::onStopSimulation);
    toolbar->addAction(stopAction);

    
    resetAction = new QAction(QIcon(), "Reset", this);
    resetAction->setToolTip("Reset Simulation");
    connect(resetAction, &QAction::triggered, this, &MainWindow::onResetSimulation);
    toolbar->addAction(resetAction);

    toolbar->addSeparator();

    
    QLabel* speedTitle = new QLabel("Speed");
    speedTitle->setObjectName("MutedText");
    toolbar->addWidget(speedTitle);

    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(100, 2000); 
    speedSlider->setValue(1000);
    speedSlider->setInvertedAppearance(true); 
    speedSlider->setFixedWidth(120);
    connect(speedSlider, &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);
    toolbar->addWidget(speedSlider);

    speedLabel = new QLabel("1.0s");
    toolbar->addWidget(speedLabel);
}

void MainWindow::createDockWidgets() {
    
    
}

void MainWindow::createToolsPanel() {
    toolsDock = new QDockWidget("Tools", this);
    toolsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* toolsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    QGroupBox* modeGroup = new QGroupBox("Drawing Mode");
    QVBoxLayout* modeLayout = new QVBoxLayout();
    modeLayout->setSpacing(3);

    selectModeBtn = new QRadioButton("Select");
    selectModeBtn->setChecked(true);
    connect(selectModeBtn, &QRadioButton::clicked, this, &MainWindow::onSelectMode);
    modeLayout->addWidget(selectModeBtn);

    addStateModeBtn = new QRadioButton("Add State");
    connect(addStateModeBtn, &QRadioButton::clicked, this, &MainWindow::onAddStateMode);
    modeLayout->addWidget(addStateModeBtn);

    addTransitionModeBtn = new QRadioButton("Add Transition");
    connect(addTransitionModeBtn, &QRadioButton::clicked, this, &MainWindow::onAddTransitionMode);
    modeLayout->addWidget(addTransitionModeBtn);

    deleteModeBtn = new QRadioButton("Delete");
    connect(deleteModeBtn, &QRadioButton::clicked, this, &MainWindow::onDeleteMode);
    modeLayout->addWidget(deleteModeBtn);

    modeGroup->setLayout(modeLayout);
    layout->addWidget(modeGroup);

    QGroupBox* actionsGroup = new QGroupBox("Actions");
    QVBoxLayout* actionsLayout = new QVBoxLayout();
    actionsLayout->setSpacing(3);

    clearCanvasBtn = new QPushButton("Clear Canvas");
    connect(clearCanvasBtn, &QPushButton::clicked, this, &MainWindow::onClearCanvas);
    actionsLayout->addWidget(clearCanvasBtn);

    actionsGroup->setLayout(actionsLayout);
    layout->addWidget(actionsGroup);

    layout->addStretch();

    toolsWidget->setLayout(layout);
    toolsWidget->setMaximumWidth(150);
    toolsDock->setWidget(toolsWidget);
    addDockWidget(Qt::LeftDockWidgetArea, toolsDock);
}

void MainWindow::createAutomatonListPanel() {
    automatonListDock = new QDockWidget("Automatons", this);
    automatonListDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* listWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    automatonList = new QListWidget();
    automatonList->setMaximumHeight(120);
    connect(automatonList, &QListWidget::itemClicked,
            this, &MainWindow::onAutomatonSelected);
    layout->addWidget(automatonList);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(3);

    newAutomatonBtn = new QPushButton("New");
    newAutomatonBtn->setMaximumWidth(50);
    connect(newAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onNewAutomaton);
    btnLayout->addWidget(newAutomatonBtn);

    deleteAutomatonBtn = new QPushButton("Del");
    deleteAutomatonBtn->setMaximumWidth(50);
    connect(deleteAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onDeleteAutomaton);
    btnLayout->addWidget(deleteAutomatonBtn);

    renameAutomatonBtn = new QPushButton("Rename");
    connect(renameAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onRenameAutomaton);
    btnLayout->addWidget(renameAutomatonBtn);

    layout->addLayout(btnLayout);

    listWidget->setLayout(layout);
    listWidget->setMaximumWidth(150);
    automatonListDock->setWidget(listWidget);
    addDockWidget(Qt::LeftDockWidgetArea, automatonListDock);
}

void MainWindow::createPropertiesPanel() {
    propertiesDock = new QDockWidget("Properties", this);
    propertiesDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

    QWidget* propsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);

    
    QGroupBox* infoGroup = new QGroupBox("Automaton Info");
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    typeLabel = new QLabel("Type: N/A");
    typeLabel->setStyleSheet("font-weight: bold;");
    infoLayout->addWidget(typeLabel);

    stateCountLabel = new QLabel("States: 0");
    infoLayout->addWidget(stateCountLabel);

    transitionCountLabel = new QLabel("Transitions: 0");
    infoLayout->addWidget(transitionCountLabel);

    alphabetLabel = new QLabel("Alphabet: {}");
    alphabetLabel->setWordWrap(true);
    infoLayout->addWidget(alphabetLabel);

    infoGroup->setLayout(infoLayout);
    layout->addWidget(infoGroup);

    
    selectedStateLabel = new QLabel("No state selected");
    selectedStateLabel->setObjectName("MutedText");
    selectedStateLabel->setAlignment(Qt::AlignCenter);
    selectedStateLabel->setWordWrap(true);
    layout->addWidget(selectedStateLabel);

    
    deleteStateBtn = new QPushButton("Delete Options");
    deleteStateBtn->setVisible(false);
    deleteStateBtn->setProperty("class", "danger");
    connect(deleteStateBtn, &QPushButton::clicked, this, &MainWindow::onDeleteStateOrTransition);
    layout->addWidget(deleteStateBtn);

    
    QGroupBox* allTransGroup = new QGroupBox("All Transitions");
    QVBoxLayout* allTransLayout = new QVBoxLayout();
    allTransLayout->setSpacing(3);

    transitionTable = new QTableWidget();
    transitionTable->setColumnCount(3);
    transitionTable->setHorizontalHeaderLabels({"From", "Symbol", "To"});
    transitionTable->horizontalHeader()->setStretchLastSection(true);
    transitionTable->setMaximumHeight(120);
    transitionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    allTransLayout->addWidget(transitionTable);

    allTransGroup->setLayout(allTransLayout);
    layout->addWidget(allTransGroup);

    
    convertNFAtoDFABtn = new QPushButton("Convert NFA → DFA");
    convertNFAtoDFABtn->setProperty("class", "secondary");
    connect(convertNFAtoDFABtn, &QPushButton::clicked, this, &MainWindow::onConvertNFAtoDFA);
    layout->addWidget(convertNFAtoDFABtn);

    minimizeDFABtn = new QPushButton("Minimize DFA");
    minimizeDFABtn->setProperty("class", "secondary");
    connect(minimizeDFABtn, &QPushButton::clicked, this, &MainWindow::onMinimizeDFA);
    layout->addWidget(minimizeDFABtn);

    
    fromRegexBtn = new QPushButton("From Regex");
    fromRegexBtn->setProperty("class", "secondary");
    connect(fromRegexBtn, &QPushButton::clicked, this, &MainWindow::onFromRegex);
    layout->addWidget(fromRegexBtn);

    
    testAutomatonBtn = new QPushButton("Test Automaton");
    testAutomatonBtn->setProperty("class", "primary");
    connect(testAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onTestAutomaton);
    layout->addWidget(testAutomatonBtn);

    
    traceAutomatonBtn = new QPushButton("Trace Execution");
    traceAutomatonBtn->setProperty("class", "primary");
    connect(traceAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onTraceAutomaton);
    layout->addWidget(traceAutomatonBtn);

    layout->addStretch();

    propsWidget->setLayout(layout);
    propsWidget->setMaximumWidth(250);
    propertiesDock->setWidget(propsWidget);
    addDockWidget(Qt::RightDockWidgetArea, propertiesDock);
}










































void MainWindow::onSelectMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::Select);
    if (selectModeBtn) selectModeBtn->setChecked(true);
    if (selectAction) selectAction->setChecked(true);
    if (addStateModeBtn) addStateModeBtn->setChecked(false);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(false);
    if (deleteModeBtn) deleteModeBtn->setChecked(false);
    if (addStateAction) addStateAction->setChecked(false);
    if (addTransitionAction) addTransitionAction->setChecked(false);
    if (deleteAction) deleteAction->setChecked(false);
    statusBar()->showMessage("Mode: Select");
}

void MainWindow::onAddStateMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::AddState);
    if (addStateModeBtn) addStateModeBtn->setChecked(true);
    if (addStateAction) addStateAction->setChecked(true);
    if (selectModeBtn) selectModeBtn->setChecked(false);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(false);
    if (deleteModeBtn) deleteModeBtn->setChecked(false);
    if (selectAction) selectAction->setChecked(false);
    if (addTransitionAction) addTransitionAction->setChecked(false);
    if (deleteAction) deleteAction->setChecked(false);
    statusBar()->showMessage("Mode: Add State");
}

void MainWindow::onAddTransitionMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::AddTransition);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(true);
    if (addTransitionAction) addTransitionAction->setChecked(true);
    if (selectModeBtn) selectModeBtn->setChecked(false);
    if (addStateModeBtn) addStateModeBtn->setChecked(false);
    if (deleteModeBtn) deleteModeBtn->setChecked(false);
    if (selectAction) selectAction->setChecked(false);
    if (addStateAction) addStateAction->setChecked(false);
    if (deleteAction) deleteAction->setChecked(false);
    statusBar()->showMessage("Mode: Add Transition");
}

void MainWindow::onDeleteMode() {
    if (!canvas) return;

    canvas->setDrawMode(DrawMode::Delete);
    if (deleteModeBtn) deleteModeBtn->setChecked(true);
    if (deleteAction) deleteAction->setChecked(true);
    if (selectModeBtn) selectModeBtn->setChecked(false);
    if (addStateModeBtn) addStateModeBtn->setChecked(false);
    if (addTransitionModeBtn) addTransitionModeBtn->setChecked(false);
    if (selectAction) selectAction->setChecked(false);
    if (addStateAction) addStateAction->setChecked(false);
    if (addTransitionAction) addTransitionAction->setChecked(false);
    statusBar()->showMessage("Mode: Delete");
}

void MainWindow::onNewAutomaton() {
    QDialog dialog(this);
    dialog.setWindowTitle("Create Automaton");
    dialog.setMinimumWidth(450);
    dialog.setStyleSheet(AppTheme::dialogStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setSpacing(10);

    QLabel* titleLabel = new QLabel("Create Automaton");
    titleLabel->setStyleSheet("font-size: 13pt; font-weight: bold; padding: 5px;");
    layout->addWidget(titleLabel);

    QLabel* subtitleLabel = makeMutedLabel("Choose the automaton model to create.");
    layout->addWidget(subtitleLabel);

    QGroupBox* typeGroup = new QGroupBox("Automaton Type");
    QVBoxLayout* typeLayout = new QVBoxLayout();
    typeLayout->setSpacing(15);

    QRadioButton* dfaRadio = new QRadioButton("DFA (Deterministic Finite Automaton)");
    dfaRadio->setChecked(true);
    typeLayout->addWidget(dfaRadio);

    QLabel* dfaDesc = new QLabel(
        "• Each state has exactly ONE transition per symbol\n"
        "• No epsilon (E) transitions allowed\n"
        "• Deterministic - predictable behavior"
        );
    dfaDesc->setObjectName("MutedText");
    dfaDesc->setStyleSheet("font-size: 10pt; margin-left: 30px; padding: 8px;");
    typeLayout->addWidget(dfaDesc);

    typeLayout->addSpacing(10);

    QRadioButton* nfaRadio = new QRadioButton("NFA (Non-deterministic Finite Automaton)");
    typeLayout->addWidget(nfaRadio);

    QLabel* nfaDesc = new QLabel(
        "• States can have MULTIPLE transitions per symbol\n"
        "• Epsilon (E) transitions allowed\n"
        "• Non-deterministic - multiple possible paths"
        );
    nfaDesc->setObjectName("MutedText");
    nfaDesc->setStyleSheet("font-size: 10pt; margin-left: 30px; padding: 8px;");
    typeLayout->addWidget(nfaDesc);

    typeGroup->setLayout(typeLayout);
    layout->addWidget(typeGroup);

    layout->addSpacing(10);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* createBtn = new QPushButton("Create");
    createBtn->setProperty("class", "primary");

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setProperty("class", "secondary");

    btnLayout->addWidget(createBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(createBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        AutomatonType type = dfaRadio->isChecked() ? AutomatonType::DFA : AutomatonType::NFA;

        QString id = generateAutomatonId();
        QString name = QString("%1 %2").arg(type == AutomatonType::DFA ? "DFA" : "NFA").arg(automatonCounter);

        Automaton* newAutomaton = new Automaton(id, name, type);
        if (!newAutomaton) {
            qCritical() << "Failed to create new automaton";
            return;
        }

        automatons[id] = newAutomaton;

        updateAutomatonList();

        for (int i = 0; i < automatonList->count(); ++i) {
            QListWidgetItem* item = automatonList->item(i);
            if (item && item->data(Qt::UserRole).toString() == id) {
                automatonList->setCurrentItem(item);
                setCurrentAutomaton(newAutomaton);
                break;
            }
        }

        QString typeDesc = type == AutomatonType::DFA ?
                               "DFA created. Remember: Each state must have exactly one transition per symbol." :
                               "NFA created. You can add multiple transitions per symbol and use 'E' for epsilon.";

        statusBar()->showMessage(QString("%1 - %2").arg(name).arg(typeDesc), 5000);
    }
}

void MainWindow::onDeleteAutomaton() {
    if (!automatonList) return;

    QListWidgetItem* item = automatonList->currentItem();
    if (!item) {
        showStyledMessageBox("Warning", "Please select an automaton to delete.", QMessageBox::Warning);
        return;
    }

    QString id = item->data(Qt::UserRole).toString();

    if (showConfirmDialog("Delete automaton?",
                          "This will permanently delete the selected automaton.",
                          "Delete",
                          "Cancel",
                          true)) {
        if (automatons.contains(id)) {
            
            if (currentAutomaton && currentAutomaton->getId() == id) {
                currentAutomaton = nullptr;
                currentSelectedStateId = "";
                if (canvas) {
                    canvas->setAutomaton(nullptr);
                }
            }

            delete automatons[id];
            automatons.remove(id);

            updateAutomatonList();
            updateProperties();
            statusBar()->showMessage("Automaton deleted");
        }
    }
}

void MainWindow::onRenameAutomaton() {
    if (!automatonList) return;

    QListWidgetItem* item = automatonList->currentItem();
    if (!item) {
        showStyledMessageBox("Warning", "Please select an automaton to rename.", QMessageBox::Warning);
        return;
    }

    QString id = item->data(Qt::UserRole).toString();
    Automaton* automaton = automatons.value(id);

    if (automaton) {
        QString newName = automaton->getName();
        bool ok = showTextInputDialog("Rename automaton",
                                      "Enter new name:",
                                      automaton->getName(),
                                      newName);

        if (ok && !newName.isEmpty()) {
            automaton->setName(newName);
            updateAutomatonList();
            statusBar()->showMessage(QString("Automaton renamed to: %1").arg(newName));
        }
    }
}

void MainWindow::onAutomatonSelected(QListWidgetItem* item) {
    if (!item) return;

    QString id = item->data(Qt::UserRole).toString();
    Automaton* automaton = automatons.value(id);

    if (automaton) {
        setCurrentAutomaton(automaton);
        statusBar()->showMessage(QString("Selected: %1").arg(automaton->getName()));
    }
}

void MainWindow::onClearCanvas() {
    if (!currentAutomaton) return;

    if (showConfirmDialog("Clear canvas?",
                          "This will remove all states and transitions from the current automaton.",
                          "Clear",
                          "Cancel",
                          true)) {
        currentAutomaton->clear();
        currentSelectedStateId = "";
        if (canvas) {
            canvas->update();
        }
        updateProperties();
        statusBar()->showMessage("Canvas cleared");
    }
}

void MainWindow::onDeleteStateOrTransition() {
    if (!currentAutomaton || currentSelectedStateId.isEmpty()) return;

    
    QString stateIdToDelete = currentSelectedStateId;

    const State* selectedState = currentAutomaton->getState(currentSelectedStateId);
    if (!selectedState) {
        
        currentSelectedStateId = "";
        updateProperties();
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Delete Options");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(AppTheme::dialogStyleSheet());

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);

    QLabel* titleLabel = new QLabel(QString("Delete options for state: <b>%1</b>").arg(selectedState->getLabel()));
    titleLabel->setStyleSheet("font-size: 12pt; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    
    QRadioButton* deleteNodeRadio = new QRadioButton("Delete entire state (node)");
    deleteNodeRadio->setStyleSheet("font-weight: bold; font-size: 11pt;");
    deleteNodeRadio->setChecked(true);
    mainLayout->addWidget(deleteNodeRadio);

    QLabel* nodeWarning = new QLabel("This will remove the state and all of its transitions.");
    nodeWarning->setStyleSheet("color: #F2A0A0; margin-left: 25px; font-size: 9pt; font-style: italic;");
    mainLayout->addWidget(nodeWarning);

    mainLayout->addSpacing(10);

    QRadioButton* deleteTransitionRadio = new QRadioButton("Delete specific transition(s)");
    deleteTransitionRadio->setStyleSheet("font-weight: bold; font-size: 11pt;");
    mainLayout->addWidget(deleteTransitionRadio);

    
    QVector<Transition> fromTransitions = currentAutomaton->getTransitionsFrom(currentSelectedStateId);

    QGroupBox* transitionsGroup = new QGroupBox("Select transitions to delete:");
    transitionsGroup->setEnabled(false);
    QVBoxLayout* transLayout = new QVBoxLayout();

    QListWidget* transitionsList = new QListWidget();
    transitionsList->setSelectionMode(QAbstractItemView::MultiSelection);

    if (fromTransitions.isEmpty()) {
        QListWidgetItem* noTransItem = new QListWidgetItem("(No outgoing transitions)");
        noTransItem->setFlags(Qt::NoItemFlags);
        transitionsList->addItem(noTransItem);
        deleteTransitionRadio->setEnabled(false);
        deleteTransitionRadio->setStyleSheet("color: #6F7884; font-size: 11pt;");
    } else {
        for (const auto& trans : fromTransitions) {
            QString transText = QString("%1 --(%2)--> %3")
            .arg(trans.getFromStateId())
                .arg(trans.getSymbolsString())
                .arg(trans.getToStateId());

            QListWidgetItem* item = new QListWidgetItem(transText);
            item->setData(Qt::UserRole, trans.getToStateId());
            item->setData(Qt::UserRole + 1, trans.getSymbolsString());
            transitionsList->addItem(item);
        }
    }

    transLayout->addWidget(transitionsList);
    transitionsGroup->setLayout(transLayout);
    mainLayout->addWidget(transitionsGroup);

    
    connect(deleteTransitionRadio, &QRadioButton::toggled, [transitionsGroup](bool checked) {
        transitionsGroup->setEnabled(checked);
    });

    mainLayout->addSpacing(15);

    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* deleteBtn = new QPushButton("Delete");
    deleteBtn->setProperty("class", "danger");

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setProperty("class", "secondary");

    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    connect(deleteBtn, &QPushButton::clicked, [&, stateIdToDelete]() { 
        if (deleteNodeRadio->isChecked()) {
            
            if (showConfirmDialog("Delete state?",
                                  QString("Delete state '%1' and all its transitions?").arg(selectedState->getLabel()),
                                  "Delete",
                                  "Cancel",
                                  true)) {
                QString stateId = currentSelectedStateId;
                QString stateLabel = selectedState->getLabel();

                
                currentSelectedStateId = "";

                
                if (currentAutomaton->removeState(stateIdToDelete)) {
                    statusBar()->showMessage(QString("State '%1' deleted").arg(stateLabel), 3000);
                    updateProperties();
                    if (canvas) {
                        canvas->update();
                    }
                    dialog.accept();
                } else {
                    statusBar()->showMessage("Failed to delete state", 3000);
                }
            }
        } else {
            
            QList<QListWidgetItem*> selected = transitionsList->selectedItems();

            if (selected.isEmpty()) {
                showStyledMessageBox("No selection", "Please select at least one transition to delete.", QMessageBox::Warning);
                return;
            }

            int deletedCount = 0;
            for (auto* item : selected) {
                QString toState = item->data(Qt::UserRole).toString();
                QString symbolsStr = item->data(Qt::UserRole + 1).toString();

                
                QStringList symbols = symbolsStr.split(", ");
                for (const QString& symbol : symbols) {
                    QString actualSymbol = symbol;
                    if (actualSymbol == "ε") {
                        actualSymbol = "E";
                    }

                    if (currentAutomaton->removeTransition(currentSelectedStateId, toState, actualSymbol)) {
                        deletedCount++;
                    }
                }
            }

            if (deletedCount > 0) {
                statusBar()->showMessage(QString("Deleted %1 transition(s)").arg(deletedCount), 3000);
                updateProperties();
                if (canvas) {
                    canvas->update();
                }
                dialog.accept();
            }
        }
    });

    dialog.exec();
}

void MainWindow::onConvertNFAtoDFA() {
    if (!currentAutomaton) {
        showStyledMessageBox("Warning", "No automaton selected.", QMessageBox::Warning);
        return;
    }

    if (currentAutomaton->isDFA()) {
        showStyledMessageBox("Info", "This automaton is already a DFA.", QMessageBox::Information);
        return;
    }

    if (!currentAutomaton->isValid()) {
        showStyledMessageBox("Warning",
                             "Current automaton is not valid. Please ensure it has an initial state.",
                             QMessageBox::Warning);
        return;
    }

    try {
        NFAtoDFA converter;
        Automaton* dfaAutomaton = converter.convert(currentAutomaton);

        if (dfaAutomaton) {
            QString id = generateAutomatonId();
            dfaAutomaton->setName(currentAutomaton->getName() + " (DFA)");

            int stateCount = dfaAutomaton->getStateCount();
            int cols = qCeil(qSqrt(stateCount));
            int row = 0, col = 0;

            for (auto& state : dfaAutomaton->getStates()) {
                state.setPosition(QPointF(100 + col * 120, 100 + row * 120));
                col++;
                if (col >= cols) {
                    col = 0;
                    row++;
                }
            }

            automatons[id] = dfaAutomaton;
            updateAutomatonList();

            for (int i = 0; i < automatonList->count(); ++i) {
                QListWidgetItem* item = automatonList->item(i);
                if (item && item->data(Qt::UserRole).toString() == id) {
                    automatonList->setCurrentItem(item);
                    setCurrentAutomaton(dfaAutomaton);
                    break;
                }
            }

            showStyledMessageBox("Success",
                                 QString("NFA converted to DFA successfully!\n\n"
                                         "Original NFA states: %1\n"
                                         "Resulting DFA states: %2")
                                     .arg(currentAutomaton->getStateCount())
                                     .arg(dfaAutomaton->getStateCount()),
                                 QMessageBox::Information);

            statusBar()->showMessage("NFA converted to DFA");
        }
    } catch (const std::exception& e) {
        showStyledMessageBox("Error",
                             QString("Failed to convert NFA to DFA: %1").arg(e.what()),
                             QMessageBox::Critical);
    }
}

void MainWindow::onMinimizeDFA() {
    if (!currentAutomaton) {
        showStyledMessageBox("Warning", "No automaton selected.", QMessageBox::Warning);
        return;
    }

    
    if (currentAutomaton->isNFA()) {
        showStyledMessageBox("Cannot Minimize NFA",
                             "DFA minimization can only be applied to DFAs.\n\n"
                             "This automaton is an NFA (Non-deterministic Finite Automaton).\n\n"
                             "Tip: Convert it to a DFA first using 'Convert NFA to DFA', "
                             "then minimize the resulting DFA.",
                             QMessageBox::Warning);
        return;
    }

    if (!currentAutomaton->isValid()) {
        showStyledMessageBox("Warning",
                             "Current automaton is not valid. Please ensure it has an initial state.",
                             QMessageBox::Warning);
        return;
    }

    
    bool isActuallyDFA = true;
    for (const auto& t : currentAutomaton->getTransitions()) {
        if (t.isEpsilonTransition()) {
            isActuallyDFA = false;
            break;
        }
    }

    if (isActuallyDFA) {
        for (const auto& state : currentAutomaton->getStates()) {
            QMap<QString, int> symbolCount;
            for (const auto& t : currentAutomaton->getTransitions()) {
                if (t.getFromStateId() == state.getId()) {
                    for (const auto& sym : t.getSymbols()) {
                        symbolCount[sym]++;
                        if (symbolCount[sym] > 1) {
                            isActuallyDFA = false;
                            break;
                        }
                    }
                }
                if (!isActuallyDFA) break;
            }
            if (!isActuallyDFA) break;
        }
    }

    if (!isActuallyDFA) {
        showStyledMessageBox("Invalid DFA",
                             "This automaton is marked as DFA but violates DFA rules!\n\n"
                             "• It may have epsilon transitions\n"
                             "• It may have multiple transitions per symbol from the same state\n\n"
                             "Please fix the DFA or convert from NFA properly.",
                             QMessageBox::Warning);
        return;
    }

    try {
        DFAMinimizer minimizer;
        Automaton* minimizedDFA = minimizer.minimize(currentAutomaton);

        if (minimizedDFA) {
            QString id = generateAutomatonId();
            minimizedDFA->setName(currentAutomaton->getName() + " (Minimized)");

            
            int stateCount = minimizedDFA->getStateCount();
            int cols = qCeil(qSqrt(stateCount));
            int row = 0, col = 0;

            for (auto& state : minimizedDFA->getStates()) {
                state.setPosition(QPointF(100 + col * 120, 100 + row * 120));
                col++;
                if (col >= cols) {
                    col = 0;
                    row++;
                }
            }

            automatons[id] = minimizedDFA;
            updateAutomatonList();

            
            for (int i = 0; i < automatonList->count(); ++i) {
                QListWidgetItem* item = automatonList->item(i);
                if (item && item->data(Qt::UserRole).toString() == id) {
                    automatonList->setCurrentItem(item);
                    setCurrentAutomaton(minimizedDFA);
                    break;
                }
            }

            int originalStates = currentAutomaton->getStateCount();
            int minimizedStates = minimizedDFA->getStateCount();
            int reduction = originalStates - minimizedStates;

            showMinimizationCompleteDialog(originalStates, minimizedStates, reduction);

            statusBar()->showMessage(QString("DFA minimized: %1 → %2 states").arg(originalStates).arg(minimizedStates), 5000);
        } else {
            showStyledMessageBox("Error", "Failed to minimize DFA.", QMessageBox::Critical);
        }
    } catch (const std::exception& e) {
        showStyledMessageBox("Error",
                             QString("Failed to minimize DFA: %1").arg(e.what()),
                             QMessageBox::Critical);
    }
}












void MainWindow::onTestAutomaton() {
    if (!currentAutomaton) {
        showStyledMessageBox("No automaton", "Please create or select an automaton first.", QMessageBox::Warning);
        return;
    }

    
    QDialog* testDialog = new QDialog(this);
    testDialog->setWindowTitle("Test Automaton");
    testDialog->resize(500, 400);
    testDialog->setStyleSheet(AppTheme::dialogStyleSheet());

    QVBoxLayout* dialogLayout = new QVBoxLayout(testDialog);

    
    QLabel* infoLabel = new QLabel(QString("<b>Automaton:</b> %1 (%2)")
                                       .arg(currentAutomaton->getName())
                                       .arg(currentAutomaton->getType() == AutomatonType::DFA ? "DFA" : "NFA"));
    dialogLayout->addWidget(infoLabel);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* inputLabel = new QLabel("Test String:");
    QLineEdit* inputField = new QLineEdit();
    inputField->setPlaceholderText("Enter string to test, e.g., 010101");
    QPushButton* testBtn = new QPushButton("Test");
    testBtn->setProperty("class", "primary");

    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(inputField);
    inputLayout->addWidget(testBtn);
    dialogLayout->addLayout(inputLayout);

    
    QTextEdit* resultsText = new QTextEdit();
    resultsText->setReadOnly(true);
    resultsText->setStyleSheet("font-family: 'Courier New';");
    resultsText->setPlaceholderText("Enter a string and run the test.");
    dialogLayout->addWidget(resultsText);

    
    QPushButton* closeBtn = new QPushButton("Close");
    closeBtn->setProperty("class", "secondary");
    connect(closeBtn, &QPushButton::clicked, testDialog, &QDialog::accept);
    dialogLayout->addWidget(closeBtn);

    
    connect(testBtn, &QPushButton::clicked, [=]() {
        QString input = inputField->text();
        if (input.isEmpty()) {
            resultsText->append(QString("<span style='color: %1;'>Please enter a string to test.</span><br>")
                                    .arg(AppTheme::MutedText));
            return;
        }

        bool accepted = currentAutomaton->accepts(input);
        QString result;
        if (accepted) {
            result = QString("<span style='color: %1; font-weight: bold;'>ACCEPTED</span> - Input: \"%2\"")
                         .arg(AppTheme::SecondaryText, input.toHtmlEscaped());
        } else {
            result = QString("<span style='color: %1; font-weight: bold;'>REJECTED</span> - Input: \"%2\"")
                         .arg(AppTheme::Danger, input.toHtmlEscaped());
        }
        resultsText->append(result + "<br>");
        inputField->clear();
    });

    
    connect(inputField, &QLineEdit::returnPressed, testBtn, &QPushButton::click);

    testDialog->exec();
    delete testDialog;
}

void MainWindow::onTraceAutomaton() {
    if (!currentAutomaton) {
        showStyledMessageBox("No automaton", "Please create or select an automaton first.", QMessageBox::Warning);
        return;
    }

    
    QDialog* traceDialog = new QDialog(this);
    traceDialog->setWindowTitle("Trace Automaton Execution");
    traceDialog->resize(700, 500);
    traceDialog->setStyleSheet(AppTheme::dialogStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(traceDialog);

    
    QLabel* infoLabel = new QLabel(QString("<b>Automaton:</b> %1 (%2)")
                                       .arg(currentAutomaton->getName())
                                       .arg(currentAutomaton->getType() == AutomatonType::DFA ? "DFA" : "NFA"));
    layout->addWidget(infoLabel);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* inputLabel = new QLabel("Input String:");
    QLineEdit* inputField = new QLineEdit();
    inputField->setPlaceholderText("Enter string to trace, e.g., 010101");
    QPushButton* traceBtn = new QPushButton("Start Trace");
    traceBtn->setProperty("class", "primary");

    inputLayout->addWidget(inputLabel);
    inputLayout->addWidget(inputField);
    inputLayout->addWidget(traceBtn);
    layout->addLayout(inputLayout);

    
    QTextEdit* traceText = new QTextEdit();
    traceText->setReadOnly(true);
    traceText->setStyleSheet("font-family: 'Courier New'; font-size: 10pt;");
    traceText->setPlaceholderText("Run a trace to see each step.");
    layout->addWidget(traceText);

    
    QPushButton* closeBtn = new QPushButton("Close");
    closeBtn->setProperty("class", "secondary");
    connect(closeBtn, &QPushButton::clicked, traceDialog, &QDialog::accept);
    layout->addWidget(closeBtn);

    
    connect(traceBtn, &QPushButton::clicked, [=]() {
        QString input = inputField->text();
        if (input.isEmpty()) {
            traceText->append(QString("<span style='color: %1;'>Please enter a string to trace.</span><br>")
                                  .arg(AppTheme::MutedText));
            return;
        }

        traceText->clear();
        traceText->append(QString("<div style='color: %1; font-weight: bold;'>EXECUTION TRACE</div>").arg(AppTheme::Text));
        traceText->append(QString("<div style='color: %1;'>Input: \"%2\"</div>")
                              .arg(AppTheme::SecondaryText, input.toHtmlEscaped()));
        traceText->append(QString("<div style='color: %1;'>Length: %2 symbols</div><br>")
                              .arg(AppTheme::MutedText)
                              .arg(input.length()));

        
        QString startState = currentAutomaton->getInitialStateId();
        if (startState.isEmpty()) {
            traceText->append(QString("<div style='color: %1;'>No initial state defined.</div>").arg(AppTheme::Danger));
            return;
        }

        
        QSet<QString> currentStates;
        currentStates.insert(startState);
        currentStates = currentAutomaton->epsilonClosure(currentStates);

        
        auto formatStates = [](const QSet<QString>& states) {
            QStringList list = states.values();
            list.sort();
            return "{" + list.join(", ") + "}";
        };

        traceText->append(QString("<div style='color: %1;'>Start at states: <b>%2</b></div><br>")
                              .arg(AppTheme::SecondaryText)
                              .arg(formatStates(currentStates)));

        bool valid = true;
        for (int i = 0; i < input.length(); i++) {
            QString symbol = QString(input[i]);
            QSet<QString> nextStates;

            
            for (const auto& stateId : currentStates) {
                for (const auto& trans : currentAutomaton->getTransitions()) {
                    if (trans.getFromStateId() == stateId && trans.hasSymbol(symbol)) {
                        nextStates.insert(trans.getToStateId());
                    }
                }
            }

            
            nextStates = currentAutomaton->epsilonClosure(nextStates);

            if (nextStates.isEmpty()) {
                traceText->append(QString("<div style='color: %1;'>Step %2: Read '%3' from states <b>%4</b> -> <b>Dead end</b></div>")
                                      .arg(AppTheme::Danger)
                                      .arg(i + 1).arg(symbol).arg(formatStates(currentStates)));
                valid = false;
                break;
            }

            traceText->append(QString("<div style='color: %1;'>Step %2: Read '<b>%3</b>' from states <b>%4</b> -> states <b>%5</b></div>")
                                  .arg(AppTheme::MutedText)
                                  .arg(i + 1).arg(symbol).arg(formatStates(currentStates)).arg(formatStates(nextStates)));
            currentStates = nextStates;
        }

        traceText->append("<br>");

        if (valid) {
            
            bool isAccepted = false;
            QStringList finalStatesReached;
            
            for (const auto& stateId : currentStates) {
                const State* state = currentAutomaton->getState(stateId);
                if (state && state->getIsFinal()) {
                    isAccepted = true;
                    finalStatesReached.append(stateId);
                }
            }

            if (isAccepted) {
                traceText->append(QString("<div style='color: %1; font-weight: bold;'>ACCEPTED - Ended in final state(s): %2</div>")
                                      .arg(AppTheme::SecondaryText)
                                      .arg(finalStatesReached.join(", ")));
            } else {
                traceText->append(QString("<div style='color: %1; font-weight: bold;'>REJECTED - Ended in non-final state(s): %2</div>")
                                      .arg(AppTheme::Danger)
                                      .arg(formatStates(currentStates)));
            }
        } else {
            traceText->append(QString("<div style='color: %1; font-weight: bold;'>REJECTED - No valid transition found</div>")
                                  .arg(AppTheme::Danger));
        }
    });

    
    connect(inputField, &QLineEdit::returnPressed, traceBtn, &QPushButton::click);

    traceDialog->exec();
    delete traceDialog;
}

void MainWindow::onFromRegex() {
    QDialog dialog(this);
    dialog.setWindowTitle("Create Automaton from Regular Expression");
    dialog.setMinimumWidth(600);
    dialog.setStyleSheet(AppTheme::dialogStyleSheet());

    QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->setSpacing(15);

    
    QLabel* titleLabel = new QLabel("Regular Expression to NFA");
    titleLabel->setStyleSheet("font-size: 14pt; font-weight: bold; padding: 5px;");
    dialogLayout->addWidget(titleLabel);

    
    QLabel* inputLabel = new QLabel("Enter Regular Expression:");
    dialogLayout->addWidget(inputLabel);

    QLineEdit* regexInput = new QLineEdit();
    regexInput->setPlaceholderText("e.g., (a|b)*c, a+b*, (ab)*");
    regexInput->setStyleSheet("font-family: 'Courier New', monospace;");
    dialogLayout->addWidget(regexInput);

    
    QLabel* validationLabel = new QLabel("");
    validationLabel->setWordWrap(true);
    validationLabel->setStyleSheet("padding: 5px;");
    dialogLayout->addWidget(validationLabel);

    
    QGroupBox* syntaxGroup = new QGroupBox("Supported Syntax");
    QVBoxLayout* syntaxLayout = new QVBoxLayout();
    
    QTextEdit* syntaxHelp = new QTextEdit();
    syntaxHelp->setReadOnly(true);
    syntaxHelp->setMaximumHeight(180);
    syntaxHelp->setHtml(
        "<style>body { color: #F2F4F7; } table { width: 100%; } td { padding: 4px; } .op { color: #F2F4F7; font-weight: bold; } .desc { color: #A8B0BA; }</style>"
        "<table>"
        "<tr><td class='op'>a, b, 0, 1, ...</td><td class='desc'>Literal characters</td></tr>"
        "<tr><td class='op'>ab</td><td class='desc'>Concatenation (implicit)</td></tr>"
        "<tr><td class='op'>a|b</td><td class='desc'>Union/Alternation</td></tr>"
        "<tr><td class='op'>a*</td><td class='desc'>Kleene Star (zero or more)</td></tr>"
        "<tr><td class='op'>a+</td><td class='desc'>Plus (one or more)</td></tr>"
        "<tr><td class='op'>a?</td><td class='desc'>Optional (zero or one)</td></tr>"
        "<tr><td class='op'>(...)</td><td class='desc'>Grouping</td></tr>"
        "<tr><td class='op'>E or ε</td><td class='desc'>Epsilon (empty string)</td></tr>"
        "</table>"
        );
    syntaxLayout->addWidget(syntaxHelp);
    syntaxGroup->setLayout(syntaxLayout);
    dialogLayout->addWidget(syntaxGroup);

    
    QGroupBox* examplesGroup = new QGroupBox("Examples");
    QVBoxLayout* examplesLayout = new QVBoxLayout();
    
    QTextEdit* examples = new QTextEdit();
    examples->setReadOnly(true);
    examples->setMaximumHeight(100);
    examples->setHtml(
        "<style>body { color: #F2F4F7; } .ex { color: #F2F4F7; font-family: 'Courier New'; } .expl { color: #A8B0BA; }</style>"
        "<div><span class='ex'>(a|b)*</span> <span class='expl'>- Any number of a's and b's</span></div>"
        "<div><span class='ex'>a+b*</span> <span class='expl'>- One or more a's followed by zero or more b's</span></div>"
        "<div><span class='ex'>(ab|cd)*</span> <span class='expl'>- Any number of ab or cd sequences</span></div>"
        "<div><span class='ex'>a(b|c)*d</span> <span class='expl'>- Starts with a, ends with d, b or c in middle</span></div>"
        );
    examplesLayout->addWidget(examples);
    examplesGroup->setLayout(examplesLayout);
    dialogLayout->addWidget(examplesGroup);

    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* generateBtn = new QPushButton("Generate NFA");
    generateBtn->setProperty("class", "primary");
    generateBtn->setEnabled(false);

    QPushButton* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setProperty("class", "secondary");

    btnLayout->addWidget(generateBtn);
    btnLayout->addWidget(cancelBtn);
    dialogLayout->addLayout(btnLayout);

    
    connect(regexInput, &QLineEdit::textChanged, [&, regexInput, validationLabel, generateBtn](const QString& text) {
        if (text.isEmpty()) {
            validationLabel->setText("");
            validationLabel->setStyleSheet("padding: 5px;");
            generateBtn->setEnabled(false);
            return;
        }

        RegexToNFA converter;
        QString error;
        bool valid = converter.isValidRegex(text, &error);

        if (valid) {
            validationLabel->setText("Valid regular expression");
            validationLabel->setStyleSheet(QString("color: %1; padding: 5px; font-weight: 600;")
                                               .arg(AppTheme::SecondaryText));
            generateBtn->setEnabled(true);
        } else {
            validationLabel->setText(error);
            validationLabel->setStyleSheet(QString("color: %1; padding: 5px; font-weight: 600;")
                                               .arg(AppTheme::Danger));
            generateBtn->setEnabled(false);
        }
    });

    connect(generateBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    
    connect(regexInput, &QLineEdit::returnPressed, [generateBtn]() {
        if (generateBtn->isEnabled()) {
            generateBtn->click();
        }
    });

    if (dialog.exec() == QDialog::Accepted) {
        QString regex = regexInput->text();
        
        if (regex.isEmpty()) {
            return;
        }

        
        RegexToNFA converter;
        Automaton* nfa = converter.convert(regex);

        if (!nfa) {
            showStyledMessageBox("Error", 
                                 "Failed to convert regular expression to NFA.", 
                                 QMessageBox::Critical);
            return;
        }

        
        QString id = nfa->getId();
        automatons[id] = nfa;
        
        updateAutomatonList();

        
        for (int i = 0; i < automatonList->count(); ++i) {
            QListWidgetItem* item = automatonList->item(i);
            if (item && item->data(Qt::UserRole).toString() == id) {
                automatonList->setCurrentItem(item);
                setCurrentAutomaton(nfa);
                break;
            }
        }

        statusBar()->showMessage(QString("Generated NFA from regex: /%1/").arg(regex), 5000);
    }
}

void MainWindow::onAutomatonModified() {
    updateProperties();
}

void MainWindow::onStateSelected(const QString& stateId) {
    currentSelectedStateId = stateId;
    updateProperties();
}

void MainWindow::onNew() {
    onNewAutomaton();
}

void MainWindow::onOpen() {
    showStyledMessageBox("Info", "Load functionality will be implemented in next phase.",
                         QMessageBox::Information);
}

void MainWindow::onSave() {
    if (!currentAutomaton) {
        showStyledMessageBox("Warning", "No automaton to save.", QMessageBox::Warning);
        return;
    }

    showStyledMessageBox("Info", "Save functionality will be implemented in next phase.",
                         QMessageBox::Information);
}

void MainWindow::onExit() {
    if (showConfirmDialog("Exit?",
                          "Close Compiler Project?",
                          "Exit",
                          "Cancel",
                          false)) {
        qApp->quit();
    }
}


void MainWindow::updateProperties() {
    
    if (!typeLabel || !stateCountLabel || !transitionCountLabel ||
        !alphabetLabel || !transitionTable || !convertNFAtoDFABtn ||
        !minimizeDFABtn || !selectedStateLabel || !deleteStateBtn) {
        return;
    }

    if (!currentAutomaton) {
        typeLabel->setText("N/A");
        stateCountLabel->setText("0");
        transitionCountLabel->setText("0");
        alphabetLabel->setText("{}");
        transitionTable->setRowCount(0);
        convertNFAtoDFABtn->setEnabled(false);
        minimizeDFABtn->setEnabled(false);
        if (testAutomatonBtn) testAutomatonBtn->setEnabled(false);
        if (traceAutomatonBtn) traceAutomatonBtn->setEnabled(false);
        if (clearCanvasBtn) clearCanvasBtn->setEnabled(false);
        if (deleteAutomatonBtn) deleteAutomatonBtn->setEnabled(false);
        if (renameAutomatonBtn) renameAutomatonBtn->setEnabled(false);

        selectedStateLabel->setText("Nothing selected\nSelect a state or transition to edit its properties.");
        deleteStateBtn->setVisible(false);

        return;
    }

    
    QString typeText;
    bool isActuallyDFA = true;

    for (const auto& t : currentAutomaton->getTransitions()) {
        if (t.isEpsilonTransition()) {
            isActuallyDFA = false;
            break;
        }
    }

    if (isActuallyDFA) {
        for (const auto& state : currentAutomaton->getStates()) {
            QMap<QString, int> symbolCount;
            for (const auto& t : currentAutomaton->getTransitions()) {
                if (t.getFromStateId() == state.getId()) {
                    for (const auto& sym : t.getSymbols()) {
                        symbolCount[sym]++;
                        if (symbolCount[sym] > 1) {
                            isActuallyDFA = false;
                            break;
                        }
                    }
                }
                if (!isActuallyDFA) break;
            }
            if (!isActuallyDFA) break;
        }
    }

    if (currentAutomaton->isDFA()) {
        if (isActuallyDFA) {
            typeText = QString("<span style='color: %1;'>DFA</span>").arg(AppTheme::Text);
        } else {
            typeText = QString("<span style='color: %1;'>DFA Invalid</span>").arg(AppTheme::Danger);
        }
    } else {
        typeText = QString("<span style='color: %1;'>NFA</span>").arg(AppTheme::Text);
    }

    typeLabel->setText(typeText);

    QString initialStateId = currentAutomaton->getInitialStateId();
    QString stateText = QString::number(currentAutomaton->getStateCount());
    if (initialStateId.isEmpty() && currentAutomaton->getStateCount() > 0) {
        stateText += QString(" <span style='color: %1;'>!</span>").arg(AppTheme::Danger);
    }
    stateCountLabel->setText(stateText);

    transitionCountLabel->setText(QString::number(currentAutomaton->getTransitionCount()));

    QSet<QString> alphabet = currentAutomaton->getAlphabet();
    QStringList alphList = alphabet.values();
    alphList.sort();

    QString alphText = QString("{%1}").arg(alphList.join(", "));
    if (alphabet.isEmpty()) {
        alphText = QString("<span style='color: %1;'>{empty}</span>").arg(AppTheme::MutedText);
    }
    alphabetLabel->setText(alphText);

    
    if (!currentSelectedStateId.isEmpty()) {
        const State* selectedState = currentAutomaton->getState(currentSelectedStateId);
        if (selectedState) {
            QString stateInfo = QString("Selected: <b>%1</b>").arg(selectedState->getLabel());
            if (selectedState->getIsInitial()) stateInfo += " [Initial]";
            if (selectedState->getIsFinal()) stateInfo += " [Final]";

            
            int transCount = currentAutomaton->getTransitionsFrom(currentSelectedStateId).size();
            stateInfo += QString("<br><small>%1 outgoing transition(s)</small>").arg(transCount);

            selectedStateLabel->setText(stateInfo);
            deleteStateBtn->setVisible(true);
        } else {
            
            currentSelectedStateId = "";
            selectedStateLabel->setText("Nothing selected\nSelect a state or transition to edit its properties.");
            deleteStateBtn->setVisible(false);
        }
    } else {
        selectedStateLabel->setText("Nothing selected\nSelect a state or transition to edit its properties.");
        deleteStateBtn->setVisible(false);
    }

    updateTransitionTable();

    
    convertNFAtoDFABtn->setEnabled(currentAutomaton->isNFA() &&
                                   currentAutomaton->isValid());

    minimizeDFABtn->setEnabled(currentAutomaton->isDFA() &&
                               isActuallyDFA &&
                               currentAutomaton->isValid());
    if (testAutomatonBtn) testAutomatonBtn->setEnabled(currentAutomaton->isValid());
    if (traceAutomatonBtn) traceAutomatonBtn->setEnabled(currentAutomaton->isValid());
    if (clearCanvasBtn) clearCanvasBtn->setEnabled(true);
    if (deleteAutomatonBtn) deleteAutomatonBtn->setEnabled(true);
    if (renameAutomatonBtn) renameAutomatonBtn->setEnabled(true);
}

void MainWindow::updateTransitionTable() {
    if (!currentAutomaton || !transitionTable) return;

    const auto& transitions = currentAutomaton->getTransitions();
    transitionTable->setRowCount(transitions.size());

    int row = 0;
    for (const auto& trans : transitions) {
        transitionTable->setItem(row, 0,
                                 new QTableWidgetItem(trans.getFromStateId()));
        transitionTable->setItem(row, 1,
                                 new QTableWidgetItem(trans.getSymbolsString()));
        transitionTable->setItem(row, 2,
                                 new QTableWidgetItem(trans.getToStateId()));
        row++;
    }
}

void MainWindow::updateAutomatonList() {
    if (!automatonList) return;

    automatonList->clear();

    for (auto it = automatons.begin(); it != automatons.end(); ++it) {
        QString id = it.key();
        Automaton* automaton = it.value();

        QString displayName = QString("%1  ·  %2")
                                  .arg(automaton->getName())
                                  .arg(automaton->isDFA() ? "DFA" : "NFA");

        QListWidgetItem* item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, id);

        QString tooltip = QString("%1\nType: %2\nStates: %3\nTransitions: %4")
                              .arg(automaton->getName())
                              .arg(automaton->isDFA() ? "DFA" : "NFA")
                              .arg(automaton->getStateCount())
                              .arg(automaton->getTransitionCount());
        item->setToolTip(tooltip);

        automatonList->addItem(item);
    }
}

QString MainWindow::generateAutomatonId() {
    return QString("auto_%1").arg(automatonCounter++);
}

void MainWindow::setCurrentAutomaton(Automaton* automaton) {
    currentAutomaton = automaton;
    currentSelectedStateId = "";
    if (canvas) {
        canvas->setAutomaton(automaton);
    }
    updateProperties();
}

void MainWindow::showStyledMessageBox(const QString& title, const QString& message,
                                      QMessageBox::Icon icon) {
    Q_UNUSED(icon);

    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setModal(true);
    dialog.setMinimumWidth(420);
    dialog.setStyleSheet(AppTheme::dialogStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(18, 18, 18, 14);
    layout->setSpacing(12);

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 13pt; font-weight: bold;");
    layout->addWidget(titleLabel);

    QLabel* bodyLabel = new QLabel(message);
    bodyLabel->setWordWrap(true);
    bodyLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    bodyLabel->setObjectName("MutedText");
    layout->addWidget(bodyLabel);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();
    QPushButton* okButton = new QPushButton("OK");
    okButton->setProperty("class", "primary");
    buttons->addWidget(okButton);
    layout->addLayout(buttons);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    okButton->setDefault(true);
    dialog.exec();
}

bool MainWindow::showConfirmDialog(const QString& title,
                                   const QString& message,
                                   const QString& confirmText,
                                   const QString& cancelText,
                                   bool danger) {
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setModal(true);
    dialog.setMinimumWidth(430);
    dialog.setStyleSheet(AppTheme::dialogStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(18, 18, 18, 14);
    layout->setSpacing(12);

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 13pt; font-weight: bold;");
    layout->addWidget(titleLabel);

    QLabel* bodyLabel = new QLabel(message);
    bodyLabel->setWordWrap(true);
    bodyLabel->setObjectName("MutedText");
    layout->addWidget(bodyLabel);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();

    QPushButton* cancelButton = new QPushButton(cancelText);
    cancelButton->setProperty("class", "secondary");
    QPushButton* confirmButton = new QPushButton(confirmText);
    confirmButton->setProperty("class", danger ? "danger" : "primary");

    buttons->addWidget(cancelButton);
    buttons->addWidget(confirmButton);
    layout->addLayout(buttons);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(confirmButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    cancelButton->setDefault(true);
    cancelButton->setFocus();

    return dialog.exec() == QDialog::Accepted;
}

void MainWindow::showMinimizationCompleteDialog(int originalStates, int minimizedStates, int removedStates) {
    QDialog dialog(this);
    dialog.setWindowTitle("DFA minimized");
    dialog.setModal(true);
    dialog.setMinimumWidth(430);
    dialog.setStyleSheet(AppTheme::dialogStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(18, 18, 18, 14);
    layout->setSpacing(12);

    QLabel* titleLabel = new QLabel("DFA minimized");
    titleLabel->setStyleSheet("font-size: 13pt; font-weight: bold;");
    layout->addWidget(titleLabel);
    layout->addWidget(makeMutedLabel("The DFA was minimized successfully."));

    QGridLayout* summary = new QGridLayout();
    summary->setHorizontalSpacing(14);
    summary->setVerticalSpacing(8);
    summary->addWidget(makeMutedLabel("Original states"), 0, 0);
    summary->addWidget(new QLabel(QString::number(originalStates)), 0, 1);
    summary->addWidget(makeMutedLabel("Minimized states"), 1, 0);
    summary->addWidget(new QLabel(QString::number(minimizedStates)), 1, 1);
    summary->addWidget(makeMutedLabel("States removed"), 2, 0);
    summary->addWidget(new QLabel(QString::number(removedStates)), 2, 1);
    layout->addLayout(summary);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();
    QPushButton* okButton = new QPushButton("OK");
    okButton->setProperty("class", "primary");
    buttons->addWidget(okButton);
    layout->addLayout(buttons);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    okButton->setDefault(true);
    dialog.exec();
}

bool MainWindow::showTextInputDialog(const QString& title,
                                     const QString& label,
                                     const QString& placeholder,
                                     QString& value) {
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setModal(true);
    dialog.setMinimumWidth(430);
    dialog.setStyleSheet(AppTheme::dialogStyleSheet());

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(18, 18, 18, 14);
    layout->setSpacing(12);

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 13pt; font-weight: bold;");
    layout->addWidget(titleLabel);
    layout->addWidget(makeMutedLabel(label));

    QLineEdit* input = new QLineEdit();
    input->setPlaceholderText(placeholder);
    input->setText(value);
    layout->addWidget(input);

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();
    QPushButton* cancelButton = new QPushButton("Cancel");
    cancelButton->setProperty("class", "secondary");
    QPushButton* okButton = new QPushButton("OK");
    okButton->setProperty("class", "primary");
    buttons->addWidget(cancelButton);
    buttons->addWidget(okButton);
    layout->addLayout(buttons);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(input, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
    okButton->setDefault(true);
    input->setFocus();

    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    value = input->text();
    return true;
}

void MainWindow::setupCentralTabs() {
    centralTabs = new QTabWidget(this);
    if (!centralTabs) {
        qCritical() << "Failed to create central tabs!";
        return;
    }

    centralTabs->setTabPosition(QTabWidget::North);
    centralTabs->setMovable(false);

    automatonTab = new QWidget();
    QHBoxLayout* automatonLayout = new QHBoxLayout(automatonTab);
    automatonLayout->setContentsMargins(0, 0, 0, 0);
    automatonLayout->setSpacing(0);

    QFrame* leftSidebar = makePanel("Sidebar");
    leftSidebar->setMinimumWidth(220);
    leftSidebar->setMaximumWidth(280);
    leftSidebar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftSidebar);
    leftLayout->setContentsMargins(14, 14, 14, 12);
    leftLayout->setSpacing(11);

    QVBoxLayout* modeLayout = nullptr;
    QFrame* modeSection = makeSection("Mode", &modeLayout);
    selectModeBtn = new QRadioButton("Select");
    selectModeBtn->setChecked(true);
    addStateModeBtn = new QRadioButton("Add State");
    addTransitionModeBtn = new QRadioButton("Add Transition");
    deleteModeBtn = new QRadioButton("Delete");
    connect(selectModeBtn, &QRadioButton::clicked, this, &MainWindow::onSelectMode);
    connect(addStateModeBtn, &QRadioButton::clicked, this, &MainWindow::onAddStateMode);
    connect(addTransitionModeBtn, &QRadioButton::clicked, this, &MainWindow::onAddTransitionMode);
    connect(deleteModeBtn, &QRadioButton::clicked, this, &MainWindow::onDeleteMode);
    modeLayout->addWidget(selectModeBtn);
    modeLayout->addWidget(addStateModeBtn);
    modeLayout->addWidget(addTransitionModeBtn);
    modeLayout->addWidget(deleteModeBtn);
    leftLayout->addWidget(modeSection);
    leftLayout->addWidget(makeDivider());

    QVBoxLayout* automataLayout = nullptr;
    QFrame* automataSection = makeSection("Automata", &automataLayout);
    newAutomatonBtn = new QPushButton("New Automaton");
    setButtonClass(newAutomatonBtn, "primary");
    connect(newAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onNewAutomaton);
    automataLayout->addWidget(newAutomatonBtn);

    automatonList = new QListWidget();
    automatonList->setMinimumHeight(120);
    automatonList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(automatonList, &QListWidget::itemClicked,
            this, &MainWindow::onAutomatonSelected);
    automataLayout->addWidget(automatonList);
    automataLayout->addWidget(makeMutedLabel("No automata yet? Create one to start designing states and transitions."));

    QHBoxLayout* automataButtons = new QHBoxLayout();
    automataButtons->setSpacing(8);
    renameAutomatonBtn = new QPushButton("Rename");
    deleteAutomatonBtn = new QPushButton("Delete");
    setButtonClass(renameAutomatonBtn, "secondary");
    setButtonClass(deleteAutomatonBtn, "danger");
    connect(renameAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onRenameAutomaton);
    connect(deleteAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onDeleteAutomaton);
    automataButtons->addWidget(renameAutomatonBtn);
    automataButtons->addWidget(deleteAutomatonBtn);
    automataLayout->addLayout(automataButtons);
    leftLayout->addWidget(automataSection, 1);
    leftLayout->addWidget(makeDivider());

    QVBoxLayout* helpLayout = nullptr;
    QFrame* helpSection = makeSection("Workflow", &helpLayout);
    helpLayout->addWidget(makeMutedLabel("Tip: Add states, connect transitions, then test, trace, convert, or minimize."));
    clearCanvasBtn = new QPushButton("Clear Canvas");
    setButtonClass(clearCanvasBtn, "danger");
    connect(clearCanvasBtn, &QPushButton::clicked, this, &MainWindow::onClearCanvas);
    helpLayout->addWidget(clearCanvasBtn);
    leftLayout->addWidget(helpSection);

    QFrame* canvasShell = makePanel("CanvasShell");
    QVBoxLayout* canvasLayout = new QVBoxLayout(canvasShell);
    canvasLayout->setContentsMargins(14, 10, 14, 14);
    canvasLayout->setSpacing(6);
    canvasShell->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout* canvasHeader = new QHBoxLayout();
    canvasHeader->setContentsMargins(0, 0, 0, 0);
    canvasHeader->setSpacing(6);
    QLabel* canvasTitle = new QLabel("Workspace");
    canvasTitle->setObjectName("SectionTitle");
    canvasHeader->addWidget(canvasTitle);
    canvasHeader->addStretch();
    QStringList canvasControls = {"Zoom +", "Zoom -", "Fit", "Reset"};
    for (const QString& label : canvasControls) {
        QPushButton* button = new QPushButton(label);
        setButtonClass(button, "toolbar");
        button->setFixedHeight(26);
        button->setEnabled(false);
        button->setToolTip("Canvas navigation controls are reserved for a future zoom implementation.");
        canvasHeader->addWidget(button);
    }
    canvasLayout->addLayout(canvasHeader);

    canvas = new AutomatonCanvas(automatonTab);
    if (!canvas) {
        qCritical() << "Failed to create AutomatonCanvas!";
        return;
    }
    connect(canvas, &AutomatonCanvas::newAutomatonRequested,
            this, &MainWindow::onNewAutomaton);
    canvasLayout->addWidget(canvas, 1);

    QFrame* rightSidebar = makePanel("InspectorPanel");
    rightSidebar->setMinimumWidth(280);
    rightSidebar->setMaximumWidth(350);
    rightSidebar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightSidebar);
    rightLayout->setContentsMargins(14, 14, 14, 12);
    rightLayout->setSpacing(11);

    QVBoxLayout* automatonInfoLayout = nullptr;
    QFrame* automatonInfoSection = makeSection("Automaton", &automatonInfoLayout);
    QGridLayout* statsLayout = new QGridLayout();
    statsLayout->setSpacing(8);
    typeLabel = new QLabel("N/A");
    stateCountLabel = new QLabel("0");
    transitionCountLabel = new QLabel("0");
    alphabetLabel = new QLabel("{}");
    alphabetLabel->setWordWrap(true);
    statsLayout->addWidget(makeStatCard(typeLabel, "Type"), 0, 0);
    statsLayout->addWidget(makeStatCard(stateCountLabel, "States"), 0, 1);
    statsLayout->addWidget(makeStatCard(transitionCountLabel, "Transitions"), 1, 0);
    statsLayout->addWidget(makeStatCard(alphabetLabel, "Alphabet"), 1, 1);
    automatonInfoLayout->addLayout(statsLayout);
    rightLayout->addWidget(automatonInfoSection);
    rightLayout->addWidget(makeDivider());

    QVBoxLayout* selectionLayout = nullptr;
    QFrame* selectionSection = makeSection("Selection", &selectionLayout);
    selectedStateLabel = makeMutedLabel("Nothing selected\nSelect a state or transition to edit its properties.");
    selectedStateLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    selectedStateLabel->setMinimumHeight(70);
    selectionLayout->addWidget(selectedStateLabel);
    deleteStateBtn = new QPushButton("Delete Selection");
    setButtonClass(deleteStateBtn, "danger");
    deleteStateBtn->setVisible(false);
    connect(deleteStateBtn, &QPushButton::clicked, this, &MainWindow::onDeleteStateOrTransition);
    selectionLayout->addWidget(deleteStateBtn);
    rightLayout->addWidget(selectionSection);
    rightLayout->addWidget(makeDivider());

    QVBoxLayout* transitionsLayout = nullptr;
    QFrame* transitionsSection = makeSection("Transitions", &transitionsLayout);
    transitionTable = new QTableWidget();
    transitionTable->setColumnCount(3);
    transitionTable->setHorizontalHeaderLabels({"From", "Symbol", "To"});
    transitionTable->horizontalHeader()->setStretchLastSection(true);
    transitionTable->verticalHeader()->setVisible(false);
    transitionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transitionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    transitionTable->setMinimumHeight(120);
    transitionTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    transitionsLayout->addWidget(transitionTable);
    transitionsLayout->addWidget(makeMutedLabel("No transitions yet."));
    rightLayout->addWidget(transitionsSection, 2);
    rightLayout->addWidget(makeDivider());

    QVBoxLayout* operationsLayout = nullptr;
    QFrame* operationsSection = makeSection("Operations", &operationsLayout);
    testAutomatonBtn = new QPushButton("Test Automaton");
    traceAutomatonBtn = new QPushButton("Trace Execution");
    setButtonClass(testAutomatonBtn, "primary");
    setButtonClass(traceAutomatonBtn, "primary");
    connect(testAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onTestAutomaton);
    connect(traceAutomatonBtn, &QPushButton::clicked, this, &MainWindow::onTraceAutomaton);
    QHBoxLayout* primaryOpsLayout = new QHBoxLayout();
    primaryOpsLayout->setSpacing(6);
    primaryOpsLayout->addWidget(testAutomatonBtn);
    primaryOpsLayout->addWidget(traceAutomatonBtn);
    operationsLayout->addLayout(primaryOpsLayout);

    convertNFAtoDFABtn = new QPushButton("Convert NFA to DFA");
    minimizeDFABtn = new QPushButton("Minimize DFA");
    fromRegexBtn = new QPushButton("Build From Regex");
    setButtonClass(convertNFAtoDFABtn, "secondary");
    setButtonClass(minimizeDFABtn, "secondary");
    setButtonClass(fromRegexBtn, "secondary");
    connect(convertNFAtoDFABtn, &QPushButton::clicked, this, &MainWindow::onConvertNFAtoDFA);
    connect(minimizeDFABtn, &QPushButton::clicked, this, &MainWindow::onMinimizeDFA);
    connect(fromRegexBtn, &QPushButton::clicked, this, &MainWindow::onFromRegex);
    operationsLayout->addWidget(convertNFAtoDFABtn);
    operationsLayout->addWidget(minimizeDFABtn);
    operationsLayout->addWidget(fromRegexBtn);
    rightLayout->addWidget(operationsSection);

    automatonLayout->addWidget(leftSidebar, 0);
    automatonLayout->addWidget(canvasShell, 1);
    automatonLayout->addWidget(rightSidebar, 0);

    centralTabs->addTab(automatonTab, "Automaton Designer");

    
    lexerWidget = new LexerWidget();
    if (!lexerWidget) {
        qCritical() << "Failed to create LexerWidget!";
        return;
    }

    
    if (automatonManager && lexerWidget) {
        lexerWidget->setAutomatonManager(automatonManager);
    } else {
        qWarning() << "AutomatonManager or LexerWidget is null, cannot set manager";
    }

    centralTabs->addTab(lexerWidget, "Lexical Analyzer");

    parserWidget = new ParserWidget();
    if (automatonManager && parserWidget) {
        parserWidget->setAutomatonManager(automatonManager);
    }
    centralTabs->addTab(parserWidget, "Parser / Parse Tree");

    semanticWidget = new SemanticAnalyzerWidget();
    if (automatonManager) semanticWidget->setAutomatonManager(automatonManager);
    centralTabs->addTab(semanticWidget, "Semantic Analysis");

    
    setCentralWidget(centralTabs);

    
    connect(centralTabs, &QTabWidget::currentChanged,
            this, &MainWindow::onTabChanged);
}

void MainWindow::onTabChanged(int index) {
    
    bool showDocks = (index == 0);

    if (toolsDock) toolsDock->setVisible(showDocks);
    if (automatonListDock) automatonListDock->setVisible(showDocks);
    if (propertiesDock) propertiesDock->setVisible(showDocks);
    

    switch(index) {
    case 0:
        statusBar()->showMessage("Automaton Designer", 3000);
        break;
    case 1:
        statusBar()->showMessage("Lexical Analyzer", 3000);
        break;
    case 2:
        statusBar()->showMessage("Parser & Parse Tree", 3000);
        break;
    case 3:
        statusBar()->showMessage("Semantic Analysis", 3000);
        break;
    }
}


void MainWindow::onPlaySimulation() {
    if (!currentAutomaton) return;

    if (!isSimulating) {
        
        QString text;
        if (!showTextInputDialog("Simulation input", "Enter input string:", "e.g. 010101", text)) return;

        simulationInput = text;
        simulationStepIndex = 0;
        currentSimulationStates.clear();
        currentSimulationStates.insert(currentAutomaton->getInitialStateId());
        
        
        if (currentAutomaton->getType() == AutomatonType::NFA) {
            currentSimulationStates = currentAutomaton->epsilonClosure(currentSimulationStates);
        }

        isSimulating = true;
        simulationAccepted = false;
        simulationRejected = false;

        
        playAction->setEnabled(false);
        stepAction->setEnabled(false);
        stopAction->setEnabled(true);
        resetAction->setEnabled(false);
        
        
        if (canvas) canvas->setEnabled(false);

        
        simulationTimer->start(speedSlider->value());
        
        
        if (canvas) {
            canvas->setActiveStates(currentSimulationStates);
            statusBar()->showMessage(QString("Simulation started. Input: '%1'").arg(simulationInput));
        }
    } else {
        
    }
}

void MainWindow::onStepSimulation() {
    if (!currentAutomaton) return;

    if (!isSimulating) {
        
        QString text;
        if (!showTextInputDialog("Simulation input", "Enter input string:", "e.g. 010101", text)) return;

        simulationInput = text;
        simulationStepIndex = 0;
        currentSimulationStates.clear();
        currentSimulationStates.insert(currentAutomaton->getInitialStateId());

        
        if (currentAutomaton->getType() == AutomatonType::NFA) {
            currentSimulationStates = currentAutomaton->epsilonClosure(currentSimulationStates);
        }

        isSimulating = true;
        simulationAccepted = false;
        simulationRejected = false;
        
        
        playAction->setEnabled(true); 
        stopAction->setEnabled(true);
        resetAction->setEnabled(true);
        
        if (canvas) {
            canvas->setActiveStates(currentSimulationStates);
            canvas->setEnabled(false);
            statusBar()->showMessage(QString("Step 0. Active states: %1").arg(currentSimulationStates.size()));
        }
    } else {
        
        advanceSimulation();
    }
}

void MainWindow::onStopSimulation() {
    isSimulating = false;
    simulationTimer->stop();
    
    playAction->setEnabled(true);
    stepAction->setEnabled(true);
    stopAction->setEnabled(false);
    resetAction->setEnabled(true);
    
    if (canvas) {
        canvas->clearActiveElements();
        canvas->setEnabled(true);
    }
    statusBar()->showMessage("Simulation stopped.");
}

void MainWindow::onResetSimulation() {
    onStopSimulation();
    if (canvas) canvas->update();
}

void MainWindow::onSimulationTimerTimeout() {
    advanceSimulation();
}

void MainWindow::onSpeedChanged(int value) {
    speedLabel->setText(QString("%1s").arg(value / 1000.0, 0, 'f', 1));
    if (simulationTimer->isActive()) {
        simulationTimer->setInterval(value);
    }
}

void MainWindow::advanceSimulation() {
    if (!currentAutomaton || !isSimulating) return;

    if (simulationStepIndex >= simulationInput.length()) {
        
        simulationTimer->stop();
        
        
        bool accepted = false;
        for (const auto& stateId : currentSimulationStates) {
            State* state = currentAutomaton->getState(stateId);
            if (state && state->getIsFinal()) {
                accepted = true;
                break;
            }
        }
        
        QString result = accepted ? "ACCEPTED" : "REJECTED";
        statusBar()->showMessage(QString("Simulation Finished: %1").arg(result));
        
        if (accepted) {
            showStyledMessageBox("Accepted", "The input string was accepted.", QMessageBox::Information);
        } else {
            showStyledMessageBox("Rejected", "The input string was rejected.", QMessageBox::Warning);
        }
        
        onStopSimulation();
        return;
    }

    
    QChar inputChar = simulationInput[simulationStepIndex];
    QString symbol = QString(inputChar);
    
    QSet<QString> nextStates;
    QSet<QString> activeTransitions;

    for (const auto& currentStateId : currentSimulationStates) {
        
        for (const auto& trans : currentAutomaton->getTransitions()) {
            if (trans.getFromStateId() == currentStateId && trans.hasSymbol(symbol)) {
                nextStates.insert(trans.getToStateId());
                activeTransitions.insert(trans.getFromStateId() + "|" + trans.getToStateId());
            }
        }
    }

    
    if (currentAutomaton->getType() == AutomatonType::NFA) {
        nextStates = currentAutomaton->epsilonClosure(nextStates);
    }

    
    currentSimulationStates = nextStates;
    simulationStepIndex++;

    
    if (canvas) {
        canvas->setActiveStates(currentSimulationStates);
        canvas->setActiveTransitions(activeTransitions);
    }
    
    statusBar()->showMessage(QString("Step %1: Processed '%2'. Active states: %3")
                                 .arg(simulationStepIndex)
                                 .arg(symbol)
                                 .arg(currentSimulationStates.size()));
                                 
    
    if (currentSimulationStates.isEmpty() && currentAutomaton->getType() == AutomatonType::DFA) {
        simulationTimer->stop();
        showStyledMessageBox("Rejected", "The input string reached a dead state and was rejected.", QMessageBox::Warning);
        onStopSimulation();
    }
}
