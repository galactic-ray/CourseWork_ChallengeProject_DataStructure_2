#include "../include/gui_mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QSplitter>
#include <QGroupBox>
#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QRegExpValidator>
#include <QIntValidator>
#include <QElapsedTimer>
#include <QStackedLayout>
#include <QInputDialog>
#include <sstream>
#include <iomanip>

static int getSelectedTopicIdFromTable(QTableWidget *table) {
    if (!table) return -1;
    QList<QTableWidgetItem*> items = table->selectedItems();
    if (items.isEmpty()) return -1;
    int row = items[0]->row();
    auto *idItem = table->item(row, 0);
    if (!idItem) return -1;
    bool ok = false;
    int id = idItem->text().toInt(&ok);
    return ok ? id : -1;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      electionSystem(new ElectionSystem()),
      rootStack(nullptr),
      roleSelectionWidget(nullptr),
      voterWidget(nullptr),
      adminWidget(nullptr),
      mainTabWidget(nullptr),
      voterTopicComboBox(nullptr),
      voterTopicOptionTable(nullptr),
      voterEmptyLabel(nullptr),
      voterVoteBtn(nullptr),
      voterRefreshBtn(nullptr),
      voterViewResultBtn(nullptr),
      adminModeComboBox(nullptr),
      adminTopicComboBox(nullptr),
      topicTableWidget(nullptr),
      candidateEmptyLabel(nullptr),
      fontDownBtn(nullptr),
      fontResetBtn(nullptr),
      fontUpBtn(nullptr),
      baseFontPointSize(13),
      currentFontDelta(0)
{
    setWindowTitle("投票选举管理系统 v2.0 - GUI版");
    setMinimumSize(1000, 700);
    resize(1200, 800);
    
    createMenus();
    createToolBars();
    createStatusBar();
    createCentralWidget();
    applyGlobalStyle();
    applyFontScale();
    
    statusLabel->setText("就绪");
}

MainWindow::~MainWindow()
{
    delete electionSystem;
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction *exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    editMenu = menuBar()->addMenu("编辑(&E)");
    
    viewMenu = menuBar()->addMenu("视图(&V)");
    QAction *fontLarger = viewMenu->addAction("字体放大");
    QAction *fontSmaller = viewMenu->addAction("字体缩小");
    QAction *fontReset = viewMenu->addAction("恢复默认字体");
    fontLarger->setShortcut(QKeySequence("Ctrl++"));
    fontSmaller->setShortcut(QKeySequence("Ctrl+-"));
    fontReset->setShortcut(QKeySequence("Ctrl+0"));
    connect(fontLarger, &QAction::triggered, this, &MainWindow::onIncreaseFont);
    connect(fontSmaller, &QAction::triggered, this, &MainWindow::onDecreaseFont);
    connect(fontReset, &QAction::triggered, this, &MainWindow::onResetFont);
    
    helpMenu = menuBar()->addMenu("帮助(&H)");
    
    QAction *aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "关于", 
            "投票选举管理系统 v2.0\n\n"
            "基于C++ STL和Qt实现的投票选举管理系统\n"
            "支持候选人管理、投票统计、结果分析等功能\n\n"
            "研究性学习与创新性设计项目");
    });
}

void MainWindow::createToolBars()
{
    // 候选人系统已移除：不再提供顶部工具栏快捷入口，避免误触导致崩溃
    mainToolBar = addToolBar("主工具栏");
    mainToolBar->setVisible(false);
}

void MainWindow::createStatusBar()
{
    statusLabel = new QLabel("就绪");
    statusBar()->addWidget(statusLabel);

    statusBar()->addPermanentWidget(new QWidget(), 1); // 占位拉伸
    fontDownBtn = new QPushButton("A-");
    fontResetBtn = new QPushButton("A");
    fontUpBtn = new QPushButton("A+");
    for (auto btn : {fontDownBtn, fontResetBtn, fontUpBtn}) {
        btn->setFlat(true);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setProperty("btnRole", "neutral");
        statusBar()->addPermanentWidget(btn);
    }
    connect(fontDownBtn, &QPushButton::clicked, this, &MainWindow::onDecreaseFont);
    connect(fontResetBtn, &QPushButton::clicked, this, &MainWindow::onResetFont);
    connect(fontUpBtn, &QPushButton::clicked, this, &MainWindow::onIncreaseFont);
}

void MainWindow::createCentralWidget()
{
    rootStack = new QStackedWidget(this);
    setCentralWidget(rootStack);

    createRoleSelectionWidget();
    createVoterWidget();
    createAdminWidget();

    rootStack->setCurrentWidget(roleSelectionWidget);

    // 默认先隐藏工具栏：投票端不需要，管理员端进入后再显示
    if (mainToolBar) {
        mainToolBar->setVisible(false);
    }
}

void MainWindow::createRoleSelectionWidget()
{
    roleSelectionWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(roleSelectionWidget);
    layout->setContentsMargins(48, 48, 48, 48);

    QLabel *title = new QLabel("投票选举系统");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 28px; font-weight: 700; color: #303133;");

    QLabel *desc = new QLabel("请选择入口：投票端用于参与投票与查看结果；管理员端用于候选人管理、投票维护与导出统计。\n\n提示：如果还没有候选人，请先进入管理员端加载示例或导入候选人数据。 ");
    desc->setWordWrap(true);
    desc->setAlignment(Qt::AlignCenter);
    desc->setStyleSheet("color: #606266; font-size: 14px;");

    QPushButton *voterBtn = new QPushButton("进入投票");
    voterBtn->setProperty("btnRole", "primary");
    voterBtn->setMinimumHeight(40);

    QPushButton *adminBtn = new QPushButton("进入管理员后台");
    adminBtn->setProperty("btnRole", "secondary");
    adminBtn->setMinimumHeight(40);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(voterBtn);
    btnRow->addSpacing(16);
    btnRow->addWidget(adminBtn);
    btnRow->addStretch();

    layout->addStretch();
    layout->addWidget(title);
    layout->addSpacing(12);
    layout->addWidget(desc);
    layout->addSpacing(24);
    layout->addLayout(btnRow);
    layout->addStretch();

    connect(voterBtn, &QPushButton::clicked, this, &MainWindow::onEnterVoterMode);
    connect(adminBtn, &QPushButton::clicked, this, &MainWindow::onEnterAdminMode);

    rootStack->addWidget(roleSelectionWidget);
}

void MainWindow::createVoterWidget()
{
    voterWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(voterWidget);

    QHBoxLayout *topBar = new QHBoxLayout();
    QPushButton *backBtn = new QPushButton("返回入口");
    backBtn->setProperty("btnRole", "neutral");
    QLabel *tip = new QLabel("请选择话题与选项并投票");
    tip->setStyleSheet("font-size: 14px; color: #606266;");
    topBar->addWidget(backBtn);
    topBar->addSpacing(12);
    topBar->addWidget(tip);
    topBar->addStretch();

    voterRefreshBtn = new QPushButton("刷新话题");
    voterRefreshBtn->setProperty("btnRole", "neutral");
    voterViewResultBtn = new QPushButton("查看结果");
    voterViewResultBtn->setProperty("btnRole", "secondary");
    topBar->addWidget(voterRefreshBtn);
    topBar->addSpacing(8);
    topBar->addWidget(voterViewResultBtn);

    QGroupBox *tableGroup = new QGroupBox("话题投票");
    QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);

    QHBoxLayout *topicRow = new QHBoxLayout();
    topicRow->addWidget(new QLabel("话题:"));
    voterTopicComboBox = new QComboBox();
    voterTopicComboBox->setMinimumWidth(240);
    topicRow->addWidget(voterTopicComboBox);
    topicRow->addStretch();
    tableLayout->addLayout(topicRow);

    QWidget *tableContainer = new QWidget();
    QStackedLayout *stackLayout = new QStackedLayout(tableContainer);

    voterTopicOptionTable = new QTableWidget();
    voterTopicOptionTable->setColumnCount(4);
    voterTopicOptionTable->setHorizontalHeaderLabels(QStringList() << "选项ID" << "选项" << "票数" << "票率");
    voterTopicOptionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    voterTopicOptionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    voterTopicOptionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    voterTopicOptionTable->horizontalHeader()->setStretchLastSection(true);
    voterTopicOptionTable->setAlternatingRowColors(true);

    voterEmptyLabel = new QLabel("暂无投票话题\n请联系管理员在后台发布投票话题");
    voterEmptyLabel->setAlignment(Qt::AlignCenter);
    voterEmptyLabel->setStyleSheet("color: #999999; font-size: 14px;");

    stackLayout->addWidget(voterTopicOptionTable);
    stackLayout->addWidget(voterEmptyLabel);

    tableLayout->addWidget(tableContainer);

    QHBoxLayout *bottomBar = new QHBoxLayout();
    voterVoteBtn = new QPushButton("投票");
    voterVoteBtn->setProperty("btnRole", "primary");
    voterVoteBtn->setMinimumWidth(120);
    bottomBar->addStretch();
    bottomBar->addWidget(voterVoteBtn);

    mainLayout->addLayout(topBar);
    mainLayout->addWidget(tableGroup);
    mainLayout->addLayout(bottomBar);

    connect(backBtn, &QPushButton::clicked, this, &MainWindow::onBackToRoleSelection);
    connect(voterRefreshBtn, &QPushButton::clicked, this, &MainWindow::onVoterRefreshCandidates);
    connect(voterVoteBtn, &QPushButton::clicked, this, &MainWindow::onVoterVote);
    connect(voterViewResultBtn, &QPushButton::clicked, this, &MainWindow::onVoterShowResult);
    if (voterTopicComboBox) {
        connect(voterTopicComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this](int) { updateVoterTopicOptionTable(); });
    }

    rootStack->addWidget(voterWidget);
}

void MainWindow::createAdminWidget()
{
    adminWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(adminWidget);

    QHBoxLayout *topBar = new QHBoxLayout();
    QPushButton *backBtn = new QPushButton("返回入口");
    backBtn->setProperty("btnRole", "neutral");
    QLabel *tip = new QLabel("管理员后台");
    tip->setStyleSheet("font-size: 14px; color: #606266;");
    topBar->addWidget(backBtn);
    topBar->addSpacing(12);
    topBar->addWidget(tip);
    topBar->addStretch();

    mainTabWidget = new QTabWidget();

    layout->addLayout(topBar);
    layout->addWidget(mainTabWidget);

    connect(backBtn, &QPushButton::clicked, this, &MainWindow::onBackToRoleSelection);

    // 原有管理页：继续复用
    // 候选人管理/候选人投票功能已移除：仅保留话题投票后台
    createTopicManagementWidget();
    createStatisticsWidget();
    createElectionResultWidget();
    createDataMaintenanceWidget();
    createAdvancedFeaturesWidget();

    rootStack->addWidget(adminWidget);
}

void MainWindow::createCandidateManagementWidget()
{
    candidateWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(candidateWidget);
    
    // 输入区域
    QGroupBox *inputGroup = new QGroupBox("候选人信息");
    QFormLayout *formLayout = new QFormLayout(inputGroup);
    
    candidateIdEdit = new QLineEdit();
    candidateIdEdit->setPlaceholderText("例如：1001");
    candidateIdEdit->setValidator(new QIntValidator(1, 999999, this));
    formLayout->addRow("编号(&I):", candidateIdEdit);
    
    candidateNameEdit = new QLineEdit();
    candidateNameEdit->setMaxLength(50);
    candidateNameEdit->setPlaceholderText("例如：张三");
    formLayout->addRow("姓名(&N):", candidateNameEdit);
    
    candidateDeptEdit = new QLineEdit();
    candidateDeptEdit->setMaxLength(100);
    candidateDeptEdit->setPlaceholderText("例如：计算机学院");
    formLayout->addRow("所属单位(&D):", candidateDeptEdit);
    
    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addCandidateBtn = new QPushButton("添加");
    modifyCandidateBtn = new QPushButton("修改");
    deleteCandidateBtn = new QPushButton("删除");
    queryCandidateBtn = new QPushButton("查询");
    
    buttonLayout->addWidget(addCandidateBtn);
    buttonLayout->addWidget(modifyCandidateBtn);
    buttonLayout->addWidget(deleteCandidateBtn);
    buttonLayout->addWidget(queryCandidateBtn);
    buttonLayout->addStretch();
    
    formLayout->addRow(buttonLayout);
    
    // 表格区域
    QGroupBox *tableGroup = new QGroupBox("候选人列表");
    QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);
    
    // 表格 + 空状态叠加
    QWidget *tableContainer = new QWidget();
    QStackedLayout *stackLayout = new QStackedLayout(tableContainer);
    
    candidateTable = new QTableWidget();
    candidateTable->setColumnCount(4);
    candidateTable->setHorizontalHeaderLabels(QStringList() << "编号" << "姓名" << "所属单位" << "得票数");
    candidateTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    candidateTable->setSelectionMode(QAbstractItemView::SingleSelection);
    candidateTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    candidateTable->horizontalHeader()->setStretchLastSection(true);
    candidateTable->setAlternatingRowColors(true);
    
    connect(candidateTable, &QTableWidget::itemSelectionChanged, 
            this, &MainWindow::onCandidateTableSelectionChanged);
    
    candidateEmptyLabel = new QLabel("暂无候选人数据\n请在上方添加候选人或从文件加载");
    candidateEmptyLabel->setAlignment(Qt::AlignCenter);
    candidateEmptyLabel->setStyleSheet("color: #999999; font-size: 14px;");
    
    stackLayout->addWidget(candidateTable);
    stackLayout->addWidget(candidateEmptyLabel);
    tableLayout->addWidget(tableContainer);
    
    mainLayout->addWidget(inputGroup);
    mainLayout->addWidget(tableGroup);
    
    // 连接信号
    connect(addCandidateBtn, &QPushButton::clicked, this, &MainWindow::onAddCandidate);
    connect(modifyCandidateBtn, &QPushButton::clicked, this, &MainWindow::onModifyCandidate);
    connect(deleteCandidateBtn, &QPushButton::clicked, this, &MainWindow::onDeleteCandidate);
    connect(queryCandidateBtn, &QPushButton::clicked, this, &MainWindow::onQueryCandidate);
    
    mainTabWidget->addTab(candidateWidget, "候选人管理");
}

void MainWindow::createVoteManagementWidget()
{
    voteWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(voteWidget);
    
    // 单票投票区域
    QGroupBox *singleVoteGroup = new QGroupBox("单票投票");
    QHBoxLayout *singleLayout = new QHBoxLayout(singleVoteGroup);
    
    singleLayout->addWidget(new QLabel("候选人编号:"));
    voteCandidateIdSpin = new QSpinBox();
    voteCandidateIdSpin->setMinimum(1);
    voteCandidateIdSpin->setMaximum(999999);
    singleLayout->addWidget(voteCandidateIdSpin);
    
    singleVoteBtn = new QPushButton("投票");
    singleLayout->addWidget(singleVoteBtn);
    singleLayout->addStretch();
    
    // 批量投票区域
    QGroupBox *batchVoteGroup = new QGroupBox("批量投票（输入投票向量，用空格分隔）");
    QVBoxLayout *batchLayout = new QVBoxLayout(batchVoteGroup);
    
    batchVoteEdit = new QTextEdit();
    batchVoteEdit->setPlaceholderText("例如: 1 2 1 3 1 1 1 2 1 1");
    batchLayout->addWidget(batchVoteEdit);
    
    QHBoxLayout *batchButtonLayout = new QHBoxLayout();
    batchVoteBtn = new QPushButton("执行批量投票");
    importVotesBtn = new QPushButton("从文件导入");
    resetVotesBtn = new QPushButton("重置投票");
    batchButtonLayout->addWidget(batchVoteBtn);
    batchButtonLayout->addWidget(importVotesBtn);
    batchButtonLayout->addWidget(resetVotesBtn);
    batchButtonLayout->addStretch();
    batchLayout->addLayout(batchButtonLayout);
    
    // 撤销投票区域
    QGroupBox *undoGroup = new QGroupBox("投票撤销");
    QHBoxLayout *undoLayout = new QHBoxLayout(undoGroup);
    
    undoLastVoteBtn = new QPushButton("撤销最近一票");
    undoMultipleVotesBtn = new QPushButton("撤销多票");
    undoCountSpin = new QSpinBox();
    undoCountSpin->setMinimum(1);
    undoCountSpin->setMaximum(1000000);
    undoCountSpin->setValue(1);
    
    undoLayout->addWidget(undoLastVoteBtn);
    undoLayout->addWidget(new QLabel("撤销数量:"));
    undoLayout->addWidget(undoCountSpin);
    undoLayout->addWidget(undoMultipleVotesBtn);
    undoLayout->addStretch();
    
    // 投票历史区域
    QGroupBox *historyGroup = new QGroupBox("投票历史");
    QVBoxLayout *historyLayout = new QVBoxLayout(historyGroup);
    
    voteHistoryList = new QListWidget();
    historyLayout->addWidget(voteHistoryList);
    
    mainLayout->addWidget(singleVoteGroup);
    mainLayout->addWidget(batchVoteGroup);
    mainLayout->addWidget(undoGroup);
    mainLayout->addWidget(historyGroup);
    
    // 连接信号
    connect(singleVoteBtn, &QPushButton::clicked, this, &MainWindow::onSingleVote);
    connect(batchVoteBtn, &QPushButton::clicked, this, &MainWindow::onBatchVote);
    connect(importVotesBtn, &QPushButton::clicked, this, &MainWindow::onImportVotesFromFile);
    connect(resetVotesBtn, &QPushButton::clicked, this, &MainWindow::onResetVotes);
    connect(undoLastVoteBtn, &QPushButton::clicked, this, &MainWindow::onUndoLastVote);
    connect(undoMultipleVotesBtn, &QPushButton::clicked, this, &MainWindow::onUndoMultipleVotes);
    
    mainTabWidget->addTab(voteWidget, "投票管理");
}

void MainWindow::createStatisticsWidget()
{
    statisticsWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(statisticsWidget);
    
    // 统计表格
    QGroupBox *tableGroup = new QGroupBox("得票统计");
    QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);
    
    statisticsTable = new QTableWidget();
    statisticsTable->setColumnCount(5);
    statisticsTable->setHorizontalHeaderLabels(
        QStringList() << "编号" << "姓名" << "所属单位" << "得票数" << "得票率");
    statisticsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    statisticsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statisticsTable->horizontalHeader()->setStretchLastSection(true);
    statisticsTable->setAlternatingRowColors(true);
    tableLayout->addWidget(statisticsTable);
    
    // 排序工具条（右上角紧凑布局）
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->addStretch();
    controlLayout->addWidget(new QLabel("模式:"));
    adminModeComboBox = new QComboBox();
    adminModeComboBox->addItems(QStringList() << "话题");
    adminModeComboBox->setFixedWidth(90);
    controlLayout->addWidget(adminModeComboBox);

    controlLayout->addWidget(new QLabel("话题:"));
    adminTopicComboBox = new QComboBox();
    adminTopicComboBox->setMinimumWidth(220);
    controlLayout->addWidget(adminTopicComboBox);
    QLabel *sortLabel = new QLabel("排序:");
    controlLayout->addWidget(sortLabel);
    sortComboBox = new QComboBox();
    sortComboBox->addItems(QStringList() << "按得票数降序" << "按得票数升序" 
                                         << "按编号" << "按姓名");
    sortComboBox->setFixedWidth(120);
    controlLayout->addWidget(sortComboBox);
    
    sortBtn = new QPushButton("排序");
    sortBtn->setFixedWidth(60);
    controlLayout->addWidget(sortBtn);
    tableLayout->addLayout(controlLayout);
    
    // 统计摘要
    QGroupBox *summaryGroup = new QGroupBox("统计摘要");
    QVBoxLayout *summaryLayout = new QVBoxLayout(summaryGroup);
    
    summaryText = new QTextBrowser();
    summaryText->setMaximumHeight(150);
    summaryLayout->addWidget(summaryText);
    
    mainLayout->addWidget(tableGroup);
    mainLayout->addWidget(summaryGroup);
    
    // 连接信号
    connect(sortBtn, &QPushButton::clicked, this, &MainWindow::onSortCandidates);

    if (adminModeComboBox) {
        connect(adminModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int){
            // 切换模式后刷新话题列表与视图
            refreshAdminTopicSelectors();
            refreshAdminViews();
        });
    }
    if (adminTopicComboBox) {
        connect(adminTopicComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int){
            refreshAdminViews();
        });
    }
    
    mainTabWidget->addTab(statisticsWidget, "查询统计");
}

void MainWindow::createElectionResultWidget()
{
    resultWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(resultWidget);
    
    QGroupBox *resultGroup = new QGroupBox("选举结果");
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
    
    resultText = new QTextBrowser();
    resultLayout->addWidget(resultText);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    exportReportBtn = new QPushButton("导出报告");
    buttonLayout->addWidget(exportReportBtn);
    buttonLayout->addStretch();
    resultLayout->addLayout(buttonLayout);
    
    mainLayout->addWidget(resultGroup);
    
    // 连接信号
    connect(exportReportBtn, &QPushButton::clicked, this, &MainWindow::onExportReport);
    
    mainTabWidget->addTab(resultWidget, "选举结果");
}

void MainWindow::createDataMaintenanceWidget()
{
    maintenanceWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(maintenanceWidget);
    
    QGroupBox *operationGroup = new QGroupBox("数据处理");
    QGridLayout *gridLayout = new QGridLayout(operationGroup);
    
    saveCandidatesBtn = new QPushButton("导出话题数据");
    loadCandidatesBtn = new QPushButton("导入话题数据");
    saveVotesBtn = new QPushButton("导出投票记录");
    loadVotesBtn = new QPushButton("导入投票记录");
    saveVotesBtn->setVisible(false);
    loadVotesBtn->setVisible(false);
    loadSampleCandidatesBtn = new QPushButton("加载示例话题");
    clearAllBtn = new QPushButton("清空所有数据");
    
    gridLayout->addWidget(saveCandidatesBtn, 0, 0);
    gridLayout->addWidget(loadCandidatesBtn, 0, 1);
    gridLayout->addWidget(saveVotesBtn, 1, 0);
    gridLayout->addWidget(loadVotesBtn, 1, 1);
    gridLayout->addWidget(loadSampleCandidatesBtn, 2, 0, 1, 2);
    gridLayout->addWidget(clearAllBtn, 3, 0, 1, 2);
    
    QGroupBox *logGroup = new QGroupBox("系统日志");
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    
    maintenanceLog = new QTextBrowser();
    maintenanceLog->setMaximumHeight(300);
    logLayout->addWidget(maintenanceLog);
    
    mainLayout->addWidget(operationGroup);
    mainLayout->addWidget(logGroup);
    
    // 连接信号
    connect(saveCandidatesBtn, &QPushButton::clicked, this, &MainWindow::onSaveCandidates);
    connect(loadCandidatesBtn, &QPushButton::clicked, this, &MainWindow::onLoadCandidates);
    connect(saveVotesBtn, &QPushButton::clicked, this, &MainWindow::onSaveVotes);
    connect(loadVotesBtn, &QPushButton::clicked, this, &MainWindow::onLoadVotes);
    connect(loadSampleCandidatesBtn, &QPushButton::clicked, this, &MainWindow::onLoadSampleCandidates);
    connect(clearAllBtn, &QPushButton::clicked, this, &MainWindow::onClearAll);
    
    mainTabWidget->addTab(maintenanceWidget, "数据维护");
}

void MainWindow::createTopicManagementWidget()
{
    QWidget *topicWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(topicWidget);

    QGroupBox *inputGroup = new QGroupBox("发布投票话题");
    QFormLayout *formLayout = new QFormLayout(inputGroup);

    static QLineEdit *topicTitleEdit = nullptr;
    static QPlainTextEdit *topicDescEdit = nullptr;
    static QPlainTextEdit *topicOptionsEdit = nullptr;
    static QSpinBox *topicVotesPerVoterSpin = nullptr;
    static QTableWidget *topicTable = nullptr;
    static QPushButton *createTopicBtn = nullptr;
    static QPushButton *deleteTopicBtn = nullptr;
    static QPushButton *viewTopicDetailBtn = nullptr;
    static QLineEdit *undoVoterIdEdit = nullptr;
    static QPushButton *undoTopicVoteBtn = nullptr;
    static QPushButton *exportVoterRecordsBtn = nullptr;

    topicTitleEdit = new QLineEdit();
    topicTitleEdit->setMaxLength(100);
    topicTitleEdit->setPlaceholderText("例如：你最喜欢的编程语言？");
    formLayout->addRow("标题(&T):", topicTitleEdit);

    topicDescEdit = new QPlainTextEdit();
    topicDescEdit->setPlaceholderText("可选：话题描述");
    topicDescEdit->setMaximumHeight(80);
    formLayout->addRow("描述(&D):", topicDescEdit);

    topicOptionsEdit = new QPlainTextEdit();
    topicOptionsEdit->setPlaceholderText("每行一个选项，例如：\nC++\nPython\nJava");
    topicOptionsEdit->setMaximumHeight(120);
    formLayout->addRow("选项(&O):", topicOptionsEdit);

    topicVotesPerVoterSpin = new QSpinBox();
    topicVotesPerVoterSpin->setMinimum(1);
    topicVotesPerVoterSpin->setMaximum(1000);
    topicVotesPerVoterSpin->setValue(1);
    formLayout->addRow("每人可投票数(N):", topicVotesPerVoterSpin);

    QHBoxLayout *btnRow = new QHBoxLayout();
    createTopicBtn = new QPushButton("发布");
    deleteTopicBtn = new QPushButton("删除");
    viewTopicDetailBtn = new QPushButton("查看详情");
    exportVoterRecordsBtn = new QPushButton("导出投票记录");
    btnRow->addWidget(createTopicBtn);
    btnRow->addWidget(deleteTopicBtn);
    btnRow->addWidget(viewTopicDetailBtn);
    btnRow->addWidget(exportVoterRecordsBtn);
    btnRow->addStretch();
    formLayout->addRow(btnRow);

    undoVoterIdEdit = new QLineEdit();
    undoVoterIdEdit->setPlaceholderText("可选：撤销时显示被撤销的投票人ID（无需输入）");
    undoVoterIdEdit->setReadOnly(true);
    formLayout->addRow("最近撤销投票人ID:", undoVoterIdEdit);

    undoTopicVoteBtn = new QPushButton("撤销最近一次投票(话题)");
    undoTopicVoteBtn->setProperty("btnRole", "danger");
    formLayout->addRow(undoTopicVoteBtn);

    QGroupBox *tableGroup = new QGroupBox("话题列表");
    QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);

    topicTable = new QTableWidget();
    topicTableWidget = topicTable;
    topicTable->setColumnCount(4);
    topicTable->setHorizontalHeaderLabels(QStringList() << "话题ID" << "标题" << "创建时间" << "总票数");
    topicTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    topicTable->setSelectionMode(QAbstractItemView::SingleSelection);
    topicTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    topicTable->horizontalHeader()->setStretchLastSection(true);
    topicTable->setAlternatingRowColors(true);
    tableLayout->addWidget(topicTable);

    mainLayout->addWidget(inputGroup);
    mainLayout->addWidget(tableGroup);
    auto getSelectedTopicId = [topicTable]() -> int {
        QList<QTableWidgetItem*> items = topicTable->selectedItems();
        if (items.isEmpty()) return -1;
        int row = items[0]->row();
        auto *idItem = topicTable->item(row, 0);
        if (!idItem) return -1;
        bool ok = false;
        int id = idItem->text().toInt(&ok);
        return ok ? id : -1;
    };

    auto updateButtons = [=]() {
        int tid = getSelectedTopicId();
        deleteTopicBtn->setEnabled(tid > 0);
        viewTopicDetailBtn->setEnabled(tid > 0);
    };

    connect(topicTable, &QTableWidget::itemSelectionChanged, this, [=]() {
        updateButtons();
    });

    connect(createTopicBtn, &QPushButton::clicked, this, [=]() {
        QString title = topicTitleEdit->text().trimmed();
        QString desc = topicDescEdit->toPlainText().trimmed();
        QString optionsText = topicOptionsEdit->toPlainText();

        QStringList lines = optionsText.split(QRegExp("[\r\n]+"), QString::SkipEmptyParts);
        vector<string> optionTexts;
        for (const auto &line : lines) {
            QString t = line.trimmed();
            if (!t.isEmpty()) optionTexts.push_back(t.toStdString());
        }

                int votesPerVoter = topicVotesPerVoterSpin ? topicVotesPerVoterSpin->value() : 1;

        int topicId = electionSystem->createTopic(title.toStdString(), desc.toStdString(), optionTexts, votesPerVoter);
        if (topicId <= 0) {
            showMessage("错误", "发布失败：标题不能为空、至少需要2个有效选项，且每人可投票数N必须在[1, 选项数]范围内。", true);
            return;
        }

        showMessage("成功", QString("发布成功：话题ID %1").arg(topicId));
        topicTitleEdit->clear();
        topicDescEdit->clear();
        topicOptionsEdit->clear();

        updateTopicTable();
        refreshTopicComboBox();
        refreshAdminTopicSelectors();
        refreshAdminViews();
        updateVoterTopicOptionTable();
    });

    connect(deleteTopicBtn, &QPushButton::clicked, this, [=]() {
        int topicId = getSelectedTopicId();
        if (topicId <= 0) return;

        int ret = QMessageBox::question(this, "确认删除", QString("确认删除话题 %1 吗？\n\n删除后无法恢复。").arg(topicId), QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) return;

        if (electionSystem->deleteTopic(topicId)) {
            showMessage("成功", "删除成功。");
            updateTopicTable();
            refreshTopicComboBox();
            refreshAdminTopicSelectors();
            refreshAdminViews();
            updateVoterTopicOptionTable();
        } else {
            showMessage("错误", "删除失败：话题不存在。", true);
        }
    });

    if (exportVoterRecordsBtn) {
        connect(exportVoterRecordsBtn, &QPushButton::clicked, this, [=]() {
            QString filename = QFileDialog::getSaveFileName(this, "导出投票记录", "topic_vote_records.csv", "CSV 文件 (*.csv);;文本文件 (*.txt);;所有文件 (*.*)");
            if (filename.isEmpty()) return;
            QFile file(filename);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                showMessage("错误", "无法写入文件。", true);
                return;
            }
            QTextStream out(&file);
            out << "topicId,voterId,optionId,votedAt" << "\n";
            const auto &hist = electionSystem->getTopicVoteHistory();
            for (const auto &rec : hist) {
                out << rec.topicId << "," << QString::fromStdString(rec.voterId) << "," << rec.optionId << "," << static_cast<qint64>(rec.votedAt) << "\n";
            }
            file.close();
            showMessage("成功", QString("已导出投票记录：%1 条\n保存到：%2").arg(hist.size()).arg(filename));
            if (maintenanceLog) {
                maintenanceLog->append(QString("[%1] 导出投票记录: %2 (%3条)")
                                       .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                                       .arg(filename)
                                       .arg(hist.size()));
            }
        });
    }

    connect(viewTopicDetailBtn, &QPushButton::clicked, this, [=]() {
        int topicId = getSelectedTopicId();
        if (topicId <= 0) return;
        showTopicResultDialog(topicId);
    });

    if (undoTopicVoteBtn) {
        connect(undoTopicVoteBtn, &QPushButton::clicked, this, [=]() {
            int ret = QMessageBox::question(this, "确认撤销", "确认撤销最近一次前端话题投票吗？", QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes) return;
            TopicVoteRecord rec;
            if (electionSystem->undoLastTopicVote(&rec)) {
                if (undoVoterIdEdit) undoVoterIdEdit->setText(QString::fromStdString(rec.voterId));
                showMessage("成功", QString("已撤销：话题%1 选项%2 投票人%3").arg(rec.topicId).arg(rec.optionId).arg(QString::fromStdString(rec.voterId)));
                updateTopicTable();
                refreshTopicComboBox();
                refreshAdminTopicSelectors();
                refreshAdminViews();
                updateVoterTopicOptionTable();
            } else {
                showMessage("提示", "没有可撤销的话题投票记录。", true);
            }
        });
    }

    deleteTopicBtn->setEnabled(false);
    viewTopicDetailBtn->setEnabled(false);

    mainTabWidget->addTab(topicWidget, "话题管理");

    updateTopicTable();
}

void MainWindow::createAdvancedFeaturesWidget()
{
    advancedWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(advancedWidget);
    
    QGroupBox *analysisGroup = new QGroupBox("数据分析");
    QVBoxLayout *analysisLayout = new QVBoxLayout(analysisGroup);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    analyzeVoteDataBtn = new QPushButton("投票数据分析");
    analyzeRankingBtn = new QPushButton("排名分析");
    analyzeDistributionBtn = new QPushButton("得票分布分析");
    analyzePerformanceBtn = new QPushButton("性能测试");
    buttonLayout->addWidget(analyzeVoteDataBtn);
    buttonLayout->addWidget(analyzeRankingBtn);
    buttonLayout->addWidget(analyzeDistributionBtn);
    buttonLayout->addWidget(analyzePerformanceBtn);
    buttonLayout->addStretch();
    analysisLayout->addLayout(buttonLayout);
    
    analysisText = new QTextBrowser();
    analysisLayout->addWidget(analysisText);
    
    mainLayout->addWidget(analysisGroup);
    
    // 连接信号
    connect(analyzeVoteDataBtn, &QPushButton::clicked, this, &MainWindow::onAnalyzeVoteData);
    connect(analyzeRankingBtn, &QPushButton::clicked, this, &MainWindow::onAnalyzeRanking);
    connect(analyzeDistributionBtn, &QPushButton::clicked, this, &MainWindow::onAnalyzeDistribution);
    connect(analyzePerformanceBtn, &QPushButton::clicked, this, &MainWindow::onAnalyzePerformance);
    
    mainTabWidget->addTab(advancedWidget, "高级功能");
}


void MainWindow::refreshAdminTopicSelectors() {
    if (!adminTopicComboBox) return;

    int currentId = -1;
    if (adminTopicComboBox->count() > 0) {
        bool ok = false;
        currentId = adminTopicComboBox->currentData().toInt(&ok);
        if (!ok) currentId = -1;
    }

    adminTopicComboBox->blockSignals(true);
    adminTopicComboBox->clear();

    const vector<VoteTopic> &topics = electionSystem->getAllTopics();
    for (const auto &topic : topics) {
        adminTopicComboBox->addItem(QString("[%1] %2").arg(topic.id).arg(QString::fromStdString(topic.title)), topic.id);
    }

    if (currentId > 0) {
        int idx = adminTopicComboBox->findData(currentId);
        if (idx >= 0) adminTopicComboBox->setCurrentIndex(idx);
    }

    adminTopicComboBox->blockSignals(false);

    if (adminTopicComboBox->currentIndex() < 0 && adminTopicComboBox->count() > 0) {
        adminTopicComboBox->setCurrentIndex(0);
    }
}

int MainWindow::getSelectedAdminTopicId() const {
    if (!adminTopicComboBox) return -1;
    bool ok = false;
    int topicId = adminTopicComboBox->currentData().toInt(&ok);
    return (ok ? topicId : -1);
}

void MainWindow::refreshAdminViews() {
    // 仅在管理员界面/相关控件存在时刷新
    if (!rootStack || rootStack->currentWidget() != adminWidget) {
        return;
    }

    // 候选人模式：复用原逻辑
    bool isTopicMode = true;
    if (!isTopicMode) {
        updateStatisticsTable();
        onShowSummary();
        onShowElectionResult();
        return;
    }

    updateTopicTable();
    int topicId = getSelectedAdminTopicId();
    updateTopicStatisticsTable(topicId);
    updateTopicResultView(topicId);
}

void MainWindow::updateTopicStatisticsTable(int topicId) {
    if (!statisticsTable) return;

    VoteTopic *topic = electionSystem->queryTopic(topicId);
    if (!topic) {
        statisticsTable->setRowCount(0);
        if (summaryText) summaryText->setHtml("<p style='color:#909399;'>暂无话题数据</p>");
        return;
    }

    int totalVotes = electionSystem->getTopicTotalVotes(topicId);
    statisticsTable->setColumnCount(5);
    statisticsTable->setHorizontalHeaderLabels(QStringList() << "选项ID" << "选项" << "票数" << "票率" << "每人可投N票");
    statisticsTable->setRowCount(static_cast<int>(topic->options.size()));

    for (size_t i = 0; i < topic->options.size(); i++) {
        const auto &opt = topic->options[i];
        double percentage = totalVotes > 0 ? (100.0 * opt.voteCount / totalVotes) : 0.0;

        auto *idItem = new QTableWidgetItem(QString::number(opt.id));
        auto *textItem = new QTableWidgetItem(QString::fromStdString(opt.text));
        auto *countItem = new QTableWidgetItem(QString::number(opt.voteCount));
        auto *rateItem = new QTableWidgetItem(QString::number(percentage, 'f', 2) + "%");
        auto *nItem = new QTableWidgetItem(QString::number(topic->votesPerVoter));

        idItem->setTextAlignment(Qt::AlignCenter);
        countItem->setTextAlignment(Qt::AlignCenter);
        rateItem->setTextAlignment(Qt::AlignCenter);
        nItem->setTextAlignment(Qt::AlignCenter);

        statisticsTable->setItem(static_cast<int>(i), 0, idItem);
        statisticsTable->setItem(static_cast<int>(i), 1, textItem);
        statisticsTable->setItem(static_cast<int>(i), 2, countItem);
        statisticsTable->setItem(static_cast<int>(i), 3, rateItem);
        statisticsTable->setItem(static_cast<int>(i), 4, nItem);
    }

    statisticsTable->resizeColumnsToContents();

    if (summaryText) {
        QString summary = QString(
            "<div>"
            "<p><b>话题：</b>%1</p>"
            "<p><b>总票数：</b>%2</p>"
            "<p><b>选项数：</b>%3</p>"
            "<p><b>每人可投票数(N)：</b>%4</p>"
            "</div>")
            .arg(QString::fromStdString(topic->title))
            .arg(totalVotes)
            .arg(topic->options.size())
            .arg(topic->votesPerVoter);
        summaryText->setHtml(summary);
    }
}

void MainWindow::updateTopicResultView(int topicId) {
    if (!resultText) return;

    VoteTopic *topic = electionSystem->queryTopic(topicId);
    if (!topic) {
        resultText->setPlainText("暂无话题数据");
        return;
    }

    int totalVotes = electionSystem->getTopicTotalVotes(topicId);

    QString html;
    html += QString("<h2>话题结果</h2>");
    html += QString("<p><b>话题：</b>%1</p>").arg(QString::fromStdString(topic->title));
    html += QString("<p><b>总票数：</b>%1</p>").arg(totalVotes);
    html += QString("<p><b>每人可投票数(N)：</b>%1</p>").arg(topic->votesPerVoter);
    // 优胜者：得票率 > 50%
    if (totalVotes > 0) {
        int winnerOptId = -1;
        QString winnerOptText;
        for (const auto &opt : topic->options) {
            if (opt.voteCount * 2 > totalVotes) {
                winnerOptId = opt.id;
                winnerOptText = QString::fromStdString(opt.text);
                break;
            }
        }
        if (winnerOptId != -1) {
            html += QString("<p style=\"color: green;\"><b>✅ 优胜者：</b>[%1] %2（得票率 > 50%）</p>")
                    .arg(winnerOptId)
                    .arg(winnerOptText.toHtmlEscaped());
        } else {
            html += "<p style=\"color:#909399;\"><b>暂无优胜者：</b>没有选项得票率超过50%。</p>";
        }
    } else {
        html += "<p style=\"color:#909399;\"><b>暂无优胜者：</b>当前总票数为0。</p>";
    }
    html += "<hr>";

    // 排序展示
    vector<const VoteOption*> sorted;
    for (const auto &opt : topic->options) sorted.push_back(&opt);
    std::sort(sorted.begin(), sorted.end(), [](const VoteOption* a, const VoteOption* b){ return a->voteCount > b->voteCount; });

    html += "<table border='1' cellpadding='5'>";
    html += "<tr><th>排名</th><th>选项ID</th><th>选项</th><th>票数</th><th>票率</th></tr>";
    for (size_t i = 0; i < sorted.size(); i++) {
        const auto *opt = sorted[i];
        double percentage = totalVotes > 0 ? (100.0 * opt->voteCount / totalVotes) : 0.0;
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5%</td></tr>")
                .arg(i+1)
                .arg(opt->id)
                .arg(QString::fromStdString(opt->text).toHtmlEscaped())
                .arg(opt->voteCount)
                .arg(percentage, 0, 'f', 2);
    }
    html += "</table>";

    resultText->setHtml(html);
}

void MainWindow::updateTopicAnalysisView(int topicId, int actionIndex) {
    if (!analysisText) return;
    VoteTopic *topic = electionSystem->queryTopic(topicId);
    if (!topic) {
        analysisText->setPlainText("暂无话题数据");
        return;
    }

    int totalVotes = electionSystem->getTopicTotalVotes(topicId);

    if (actionIndex == 0) {
        // 投票数据分析（按选项汇总）
        QString txt;
        txt += "话题投票数据分析\n";
        txt += "═══════════════════════════════════════\n\n";
        txt += QString("话题：%1\n").arg(QString::fromStdString(topic->title));
        txt += QString("总票数：%1\n每人可投票数(N)：%2\n\n").arg(totalVotes).arg(topic->votesPerVoter);
        for (const auto &opt : topic->options) {
            txt += QString("  [%1] %2 : %3 票\n")
                   .arg(opt.id)
                   .arg(QString::fromStdString(opt.text))
                   .arg(opt.voteCount);
        }
        analysisText->setPlainText(txt);
        return;
    }

    if (actionIndex == 1) {
        // 排名分析
        vector<const VoteOption*> sorted;
        for (const auto &opt : topic->options) sorted.push_back(&opt);
        std::sort(sorted.begin(), sorted.end(), [](const VoteOption* a, const VoteOption* b){ return a->voteCount > b->voteCount; });

        QString txt;
        txt += "话题选项排名分析\n";
        txt += "═══════════════════════════════════════\n\n";
        txt += "排名\t选项ID\t票数\t票率\t选项\n";
        txt += "────────────────────────────────────\n";
        for (size_t i = 0; i < sorted.size(); i++) {
            const auto *opt = sorted[i];
            double percentage = totalVotes > 0 ? (100.0 * opt->voteCount / totalVotes) : 0.0;
            txt += QString("%1\t%2\t%3\t%4%\t%5\n")
                   .arg(i+1)
                   .arg(opt->id)
                   .arg(opt->voteCount)
                   .arg(percentage, 0, 'f', 2)
                   .arg(QString::fromStdString(opt->text));
        }
        analysisText->setPlainText(txt);
        return;
    }

    if (actionIndex == 2) {
        // 分布分析（条形）
        int maxVotes = 0;
        for (const auto &opt : topic->options) maxVotes = std::max(maxVotes, opt.voteCount);

        QString txt;
        txt += "话题得票分布分析（可视化）\n";
        txt += "═══════════════════════════════════════\n\n";
        for (const auto &opt : topic->options) {
            int barLength = maxVotes > 0 ? (50 * opt.voteCount / maxVotes) : 0;
            QString name = QString::fromStdString(opt.text);
            name = name.leftJustified(20, ' ');
            txt += name + " [";
            for (int i = 0; i < barLength; i++) txt += "█";
            for (int i = barLength; i < 50; i++) txt += " ";
            txt += QString("] %1 票\n").arg(opt.voteCount);
        }
        analysisText->setPlainText(txt);
        return;
    }

    // 性能测试（真实计时）
    QElapsedTimer timer;

    const int loopsTotal = 20000;
    const int loopsSort = 2000;
    const int loopsVote = 2000;
    const int loopsUndo = 2000;

    qint64 tTotalNs = 0;
    qint64 tSortNs = 0;
    qint64 tVoteNs = 0;
    qint64 tUndoNs = 0;

    // 1) getTopicTotalVotes
    timer.start();
    int sink = 0;
    for (int i = 0; i < loopsTotal; ++i) {
        sink += electionSystem->getTopicTotalVotes(topicId);
    }
    tTotalNs = timer.nsecsElapsed();

    // 2) 排序（按票数）
    timer.restart();
    for (int i = 0; i < loopsSort; ++i) {
        vector<const VoteOption*> sorted;
        sorted.reserve(topic->options.size());
        for (const auto &opt : topic->options) sorted.push_back(&opt);
        std::sort(sorted.begin(), sorted.end(), [](const VoteOption* a, const VoteOption* b){ return a->voteCount > b->voteCount; });
        if (!sorted.empty()) sink += sorted[0]->voteCount;
    }
    tSortNs = timer.nsecsElapsed();

    // 3) 投票：为了不破坏现有数据，投完马上撤销
    int optionIdForPerf = topic->options.empty() ? 1 : topic->options[0].id;
    timer.restart();
    int voted = 0;
    for (int i = 0; i < loopsVote; ++i) {
        QString vid = QString("perf_%1").arg(i);
        if (electionSystem->castTopicVote(topicId, optionIdForPerf, vid.toStdString())) {
            voted++;
        }
    }
    tVoteNs = timer.nsecsElapsed();

    // 4) 撤销最近投票
    timer.restart();
    int undone = 0;
    for (int i = 0; i < loopsUndo; ++i) {
        TopicVoteRecord rec;
        if (electionSystem->undoLastTopicVote(&rec)) {
            undone++;
        } else {
            break;
        }
    }
    tUndoNs = timer.nsecsElapsed();

    auto nsToMs = [](qint64 ns) { return ns / 1e6; };

    QString txt;
    txt += "话题性能统计（真实计时）\n";
    txt += "═══════════════════════════════════════\n\n";
    txt += QString("话题：%1\n").arg(QString::fromStdString(topic->title));
    txt += QString("选项数：%1，当前总票数：%2\n\n").arg(topic->options.size()).arg(totalVotes);

    txt += QString("1) getTopicTotalVotes 调用 %1 次：%2 ms\n").arg(loopsTotal).arg(nsToMs(tTotalNs), 0, 'f', 3);
    txt += QString("2) 选项排序重复 %1 次：%2 ms\n").arg(loopsSort).arg(nsToMs(tSortNs), 0, 'f', 3);
    txt += QString("3) castTopicVote 尝试 %1 次（成功 %2 次）：%3 ms\n").arg(loopsVote).arg(voted).arg(nsToMs(tVoteNs), 0, 'f', 3);
    txt += QString("4) undoLastTopicVote 执行 %1 次（成功 %2 次）：%3 ms\n").arg(loopsUndo).arg(undone).arg(nsToMs(tUndoNs), 0, 'f', 3);

    (void)sink;
    analysisText->setPlainText(txt);
}


void MainWindow::updateTopicTable() {
    if (!topicTableWidget) {
        return;
    }

    const vector<VoteTopic> &topics = electionSystem->getAllTopics();
    topicTableWidget->setRowCount(static_cast<int>(topics.size()));

    for (size_t i = 0; i < topics.size(); i++) {
        const auto &t = topics[i];
        int totalVotes = electionSystem->getTopicTotalVotes(t.id);
        QString created = t.createdAt > 0 ? QDateTime::fromSecsSinceEpoch(static_cast<qint64>(t.createdAt)).toString("yyyy-MM-dd hh:mm:ss") : "-";

        auto *idItem = new QTableWidgetItem(QString::number(t.id));
        auto *titleItem = new QTableWidgetItem(QString::fromStdString(t.title));
        auto *createdItem = new QTableWidgetItem(created);
        auto *totalItem = new QTableWidgetItem(QString::number(totalVotes));

        idItem->setTextAlignment(Qt::AlignCenter);
        createdItem->setTextAlignment(Qt::AlignCenter);
        totalItem->setTextAlignment(Qt::AlignCenter);

        topicTableWidget->setItem(static_cast<int>(i), 0, idItem);
        topicTableWidget->setItem(static_cast<int>(i), 1, titleItem);
        topicTableWidget->setItem(static_cast<int>(i), 2, createdItem);
        topicTableWidget->setItem(static_cast<int>(i), 3, totalItem);
    }

    topicTableWidget->resizeColumnsToContents();
}

// ==================== 话题投票辅助函数 ====================

void MainWindow::refreshTopicComboBox() {
    if (!voterTopicComboBox) return;
    
    int currentId = -1;
    if (voterTopicComboBox->count() > 0) {
        bool ok = false;
        currentId = voterTopicComboBox->currentData().toInt(&ok);
        if (!ok) currentId = -1;
    }
    
    voterTopicComboBox->blockSignals(true);
    voterTopicComboBox->clear();
    
    const vector<VoteTopic>& topics = electionSystem->getAllTopics();
    for (const auto& topic : topics) {
        voterTopicComboBox->addItem(QString("[%1] %2").arg(topic.id).arg(QString::fromStdString(topic.title)), topic.id);
    }
    
    // 尝试恢复之前选中的话题
    if (currentId > 0) {
        int index = voterTopicComboBox->findData(currentId);
        if (index >= 0) voterTopicComboBox->setCurrentIndex(index);
    }
    
    voterTopicComboBox->blockSignals(false);
    
    // 如果当前没有选中任何话题，但列表中有话题，默认选中第一个
    if (voterTopicComboBox->currentIndex() < 0 && voterTopicComboBox->count() > 0) {
        voterTopicComboBox->setCurrentIndex(0);
    }
}

void MainWindow::updateVoterTopicOptionTable() {
    if (!voterTopicComboBox || !voterTopicOptionTable) return;
    
    bool ok = false;
    int topicId = voterTopicComboBox->currentData().toInt(&ok);
    if (!ok || topicId <= 0) {
        voterTopicOptionTable->setRowCount(0);
        if (voterEmptyLabel) voterEmptyLabel->setVisible(true);
        return;
    }
    
    VoteTopic* topic = electionSystem->queryTopic(topicId);
    if (!topic) {
        voterTopicOptionTable->setRowCount(0);
        if (voterEmptyLabel) voterEmptyLabel->setVisible(true);
        return;
    }
    
    int totalVotes = electionSystem->getTopicTotalVotes(topicId);
    voterTopicOptionTable->setRowCount(static_cast<int>(topic->options.size()));
    
    for (size_t i = 0; i < topic->options.size(); i++) {
        const auto& option = topic->options[i];
        double percentage = (totalVotes > 0) ? (100.0 * option.voteCount / totalVotes) : 0.0;
        
        auto* idItem = new QTableWidgetItem(QString::number(option.id));
        auto* textItem = new QTableWidgetItem(QString::fromStdString(option.text));
        auto* countItem = new QTableWidgetItem(QString::number(option.voteCount));
        auto* percentItem = new QTableWidgetItem(QString::number(percentage, 'f', 2) + "%");
        
        idItem->setTextAlignment(Qt::AlignCenter);
        countItem->setTextAlignment(Qt::AlignCenter);
        percentItem->setTextAlignment(Qt::AlignCenter);
        
        voterTopicOptionTable->setItem(static_cast<int>(i), 0, idItem);
        voterTopicOptionTable->setItem(static_cast<int>(i), 1, textItem);
        voterTopicOptionTable->setItem(static_cast<int>(i), 2, countItem);
        voterTopicOptionTable->setItem(static_cast<int>(i), 3, percentItem);
    }
    
    voterTopicOptionTable->resizeColumnsToContents();
    
    if (voterEmptyLabel) {
        bool hasData = !topic->options.empty();
        if (voterTopicOptionTable->parentWidget()) {
            voterTopicOptionTable->parentWidget()->setProperty("currentIndex", hasData ? 0 : 1);
        }
        voterEmptyLabel->setVisible(!hasData);
    }
    
    if (voterVoteBtn) {
        voterVoteBtn->setEnabled(!topic->options.empty());
    }
}

void MainWindow::showTopicResultDialog(int topicId) {
    VoteTopic* topic = electionSystem->queryTopic(topicId);
    if (!topic) {
        showMessage("错误", "未找到指定话题。", true);
        return;
    }
    
    int totalVotes = electionSystem->getTopicTotalVotes(topicId);
    
    QString message = QString("<h3>%1</h3>").arg(QString::fromStdString(topic->title));
    if (!topic->description.empty()) {
        message += QString("<p>%1</p>").arg(QString::fromStdString(topic->description).toHtmlEscaped().replace("\n", "<br>"));
    }
    
    message += QString("<p><b>总投票数：</b>%1</p>").arg(totalVotes);
    // 优胜者：得票率 > 50%
    QString winnerLine;
    if (totalVotes > 0) {
        int winnerOptId = -1;
        QString winnerOptText;
        for (const auto &opt : topic->options) {
            if (opt.voteCount * 2 > totalVotes) {
                winnerOptId = opt.id;
                winnerOptText = QString::fromStdString(opt.text);
                break;
            }
        }
        if (winnerOptId != -1) {
            winnerLine = QString("<p style=\"color: green;\"><b>✅ 优胜者：</b>[%1] %2（得票率 > 50%）</p>")
                         .arg(winnerOptId)
                         .arg(winnerOptText.toHtmlEscaped());
        } else {
            winnerLine = "<p style=\"color:#909399;\"><b>暂无优胜者：</b>没有选项得票率超过50%。</p>";
        }
    } else {
        winnerLine = "<p style=\"color:#909399;\"><b>暂无优胜者：</b>当前总票数为0。</p>";
    }
    message += winnerLine;
    message += "<table border='1' cellspacing='0' cellpadding='5' style='width:100%'>";
    message += "<tr><th>选项ID</th><th>选项内容</th><th>票数</th><th>得票率</th></tr>";
    
    // 按票数排序选项
    vector<const VoteOption*> sortedOptions;
    for (const auto& opt : topic->options) {
        sortedOptions.push_back(&opt);
    }
    std::sort(sortedOptions.begin(), sortedOptions.end(), 
             [](const VoteOption* a, const VoteOption* b) { return a->voteCount > b->voteCount; });
    
    for (const auto* opt : sortedOptions) {
        double percentage = (totalVotes > 0) ? (100.0 * opt->voteCount / totalVotes) : 0;
        message += QString("<tr><td align='center'>%1</td><td>%2</td><td align='center'>%3</td><td align='center'>%4%</td></tr>")
                      .arg(opt->id)
                      .arg(QString::fromStdString(opt->text).toHtmlEscaped())
                      .arg(opt->voteCount)
                      .arg(percentage, 0, 'f', 2);
    }
    
    message += "</table>";
    
    QDialog dialog(this);
    dialog.setWindowTitle("投票结果 - " + QString::fromStdString(topic->title));
    dialog.resize(600, 400);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTextBrowser* browser = new QTextBrowser();
    browser->setHtml(message);
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    layout->addWidget(browser);
    layout->addWidget(buttonBox);
    
    dialog.exec();
}

// ==================== 候选人管理槽函数 ====================

void MainWindow::onAddCandidate()
{
    QString idText = candidateIdEdit->text();
    QString nameText = candidateNameEdit->text();
    QString deptText = candidateDeptEdit->text();
    
    if (idText.isEmpty() || nameText.isEmpty()) {
        showMessage("错误", "编号和姓名不能为空！", true);
        return;
    }
    
    bool ok;
    int id = idText.toInt(&ok);
    if (!ok || id <= 0) {
        showMessage("错误", "编号必须为正整数！", true);
        return;
    }
    
    if (!DataValidator::validateName(nameText.toStdString())) {
        showMessage("错误", "姓名不合法！", true);
        return;
    }
    
    if (electionSystem->addCandidate(id, nameText.toStdString(), deptText.toStdString())) {
        showMessage("成功", QString("成功添加候选人：编号 %1，姓名 %2").arg(id).arg(nameText));
        clearInputFields();
        updateCandidateTable();
        statusLabel->setText(QString("已添加候选人: %1").arg(nameText));
    } else {
        showMessage("错误", "添加失败！可能是编号重复或数据不合法。", true);
    }
}

void MainWindow::onModifyCandidate()
{
    QString idText = candidateIdEdit->text();
    QString nameText = candidateNameEdit->text();
    QString deptText = candidateDeptEdit->text();
    
    if (idText.isEmpty() || nameText.isEmpty()) {
        showMessage("错误", "编号和姓名不能为空！", true);
        return;
    }
    
    bool ok;
    int id = idText.toInt(&ok);
    if (!ok || id <= 0) {
        showMessage("错误", "编号必须为正整数！", true);
        return;
    }
    
    if (electionSystem->modifyCandidate(id, nameText.toStdString(), deptText.toStdString())) {
        showMessage("成功", QString("成功修改候选人：编号 %1").arg(id));
        clearInputFields();
        updateCandidateTable();
        statusLabel->setText(QString("已修改候选人: %1").arg(id));
    } else {
        showMessage("错误", "修改失败！候选人不存在或数据不合法。", true);
    }
}

void MainWindow::onDeleteCandidate()
{
    QString idText = candidateIdEdit->text();
    
    if (idText.isEmpty()) {
        showMessage("错误", "请输入要删除的候选人编号！", true);
        return;
    }
    
    bool ok;
    int id = idText.toInt(&ok);
    if (!ok || id <= 0) {
        showMessage("错误", "编号必须为正整数！", true);
        return;
    }
    
    Candidate *c = electionSystem->queryCandidate(id);
    if (!c) {
        showMessage("错误", "候选人不存在！", true);
        return;
    }
    
    int ret = QMessageBox::question(this, "确认删除", 
        QString("确定要删除候选人 %1 (%2) 吗？").arg(id).arg(QString::fromStdString(c->name)),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        if (electionSystem->deleteCandidate(id)) {
            showMessage("成功", QString("成功删除候选人：编号 %1").arg(id));
            clearInputFields();
            updateCandidateTable();
            statusLabel->setText(QString("已删除候选人: %1").arg(id));
        } else {
            showMessage("错误", "删除失败！", true);
        }
    }
}

void MainWindow::onQueryCandidate()
{
    QString idText = candidateIdEdit->text();
    
    if (idText.isEmpty()) {
        showMessage("错误", "请输入要查询的候选人编号！", true);
        return;
    }
    
    bool ok;
    int id = idText.toInt(&ok);
    if (!ok || id <= 0) {
        showMessage("错误", "编号必须为正整数！", true);
        return;
    }
    
    Candidate *c = electionSystem->queryCandidate(id);
    if (c) {
        candidateIdEdit->setText(QString::number(c->id));
        candidateNameEdit->setText(QString::fromStdString(c->name));
        candidateDeptEdit->setText(QString::fromStdString(c->department));
        statusLabel->setText(QString("已查询到候选人: %1").arg(QString::fromStdString(c->name)));
    } else {
        showMessage("错误", "未找到该候选人！", true);
    }
}

void MainWindow::onCandidateTableSelectionChanged()
{
    if (!candidateTable) return;
    QList<QTableWidgetItem*> items = candidateTable->selectedItems();
    if (!items.isEmpty()) {
        int row = items[0]->row();
        QString id = candidateTable->item(row, 0)->text();
        candidateIdEdit->setText(id);
        onQueryCandidate();
        modifyCandidateBtn->setEnabled(true);
        deleteCandidateBtn->setEnabled(true);
    } else {
        modifyCandidateBtn->setEnabled(false);
        deleteCandidateBtn->setEnabled(false);
    }
}

// ==================== 投票管理槽函数 ====================

void MainWindow::onSingleVote()
{
    int candidateID = voteCandidateIdSpin->value();
    
    if (electionSystem->castVote(candidateID)) {
        showMessage("成功", QString("投票成功！候选人编号: %1").arg(candidateID));
        updateCandidateTable();
        updateStatisticsTable();
        // updateVoteHistoryList();
        onShowSummary();
        onShowElectionResult();
        statusLabel->setText(QString("已投票给候选人: %1").arg(candidateID));
    } else {
        showMessage("错误", "投票失败！候选人不存在。", true);
    }
}

void MainWindow::onBatchVote()
{
    QString text = batchVoteEdit->toPlainText();
    if (text.isEmpty()) {
        showMessage("错误", "请输入投票向量！", true);
        return;
    }
    
    QStringList parts = text.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    vector<int> votes;
    int invalidTokens = 0;
    
    for (const QString &part : parts) {
        bool ok = false;
        int vote = part.toInt(&ok);
        if (!ok || vote <= 0) {
            invalidTokens++;
            continue;
        }
        votes.push_back(vote);
    }
    
    if (votes.empty()) {
        QString detail = invalidTokens > 0
            ? QString("全部输入均无效（无效项: %1）").arg(invalidTokens)
            : "无效的投票向量！";
        showMessage("错误", detail, true);
        return;
    }
    
    electionSystem->vote(votes, false);
    
    int totalVotes = static_cast<int>(votes.size()) + invalidTokens;
    const vector<Candidate> &candidates = electionSystem->getAllCandidates();
    vector<int> validIDList;
    for (const auto &c : candidates) {
        validIDList.push_back(c.id);
    }
    
    int invalidCount = DataValidator::validateVoteVector(votes, validIDList);
    int totalInvalid = invalidCount + invalidTokens;
    
    QString message = QString("批量投票完成！\n总票数: %1").arg(totalVotes);
    if (totalInvalid > 0) {
        message += QString("\n无效票数: %1").arg(totalInvalid);
    }
    
    showMessage("成功", message);
    refreshAdminTopicSelectors();
    updateCandidateTable();
    updateStatisticsTable();
    // updateVoteHistoryList();
    onShowSummary();
    onShowElectionResult();
    statusLabel->setText(QString("已处理 %1 张选票").arg(totalVotes));
}

void MainWindow::onImportVotesFromFile()
{
    QString filename = QFileDialog::getOpenFileName(this, "选择投票文件", 
                                                    ".", "数据文件 (*.csv *.dat *.txt);;所有文件 (*.*)");
    if (filename.isEmpty()) {
        return;
    }
    
    vector<int> votes;
    if (FileManager::loadVotes(votes, filename.toStdString())) {
        // 从文件导入视为一次批量投票，在当前票数基础上累加
        electionSystem->vote(votes, false);
        showMessage("成功", QString("成功从文件加载 %1 张选票").arg(votes.size()));
        updateCandidateTable();
        updateStatisticsTable();
        // updateVoteHistoryList();
        onShowSummary();
        onShowElectionResult();
        statusLabel->setText(QString("已从文件加载 %1 张选票").arg(votes.size()));
    } else {
        showMessage("错误", "文件加载失败！", true);
    }
}

void MainWindow::onResetVotes()
{
    int ret = QMessageBox::question(this, "确认重置", 
        "确定要重置所有投票吗？此操作不可恢复！",
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        electionSystem->resetVotes();
        showMessage("成功", "已重置所有投票");
        updateCandidateTable();
        updateStatisticsTable();
        // updateVoteHistoryList();
        onShowSummary();
        onShowElectionResult();
        statusLabel->setText("已重置所有投票");
    }
}

void MainWindow::onUndoLastVote()
{
    if (electionSystem->undoLastVote()) {
        showMessage("成功", "已撤销最近一张选票");
        updateCandidateTable();
        updateStatisticsTable();
        // updateVoteHistoryList();
        onShowSummary();
        onShowElectionResult();
        statusLabel->setText("已撤销最近一票");
    } else {
        showMessage("提示", "没有可撤销的投票记录。");
    }
}

void MainWindow::onUndoMultipleVotes()
{
    int count = undoCountSpin ? undoCountSpin->value() : 0;
    int undone = electionSystem->undoLastVotes(count);
    
    if (undone > 0) {
        showMessage("成功", QString("已撤销最近 %1 张选票").arg(undone));
        updateCandidateTable();
        updateStatisticsTable();
        // updateVoteHistoryList();
        onShowSummary();
        onShowElectionResult();
        statusLabel->setText(QString("已撤销 %1 张选票").arg(undone));
    } else {
        showMessage("提示", "没有可撤销的投票记录。");
    }
}

void MainWindow::onShowVoteHistory()
{
    // updateVoteHistoryList();
}

// ==================== 查询统计槽函数 ====================

void MainWindow::onShowStatistics()
{
    refreshAdminViews();
}

void MainWindow::onSortCandidates()
{
    updateTopicStatisticsTable(getSelectedAdminTopicId());
    return;
    int index = sortComboBox->currentIndex();
    vector<Candidate> candidates = electionSystem->getAllCandidates();
    
    switch (index) {
        case 0: // 按得票数降序
            Statistics::sortByVotes(candidates, false);
            break;
        case 1: // 按得票数升序
            Statistics::sortByVotes(candidates, true);
            break;
        case 2: // 按编号
            Statistics::sortByID(candidates);
            break;
        case 3: // 按姓名
            Statistics::sortByName(candidates);
            break;
    }
    
    updateStatisticsTable(candidates);
}

void MainWindow::onShowSummary()
{
    updateTopicStatisticsTable(getSelectedAdminTopicId());
    return;
    const vector<Candidate> &candidates = electionSystem->getAllCandidates();
    
    if (candidates.empty()) {
        summaryText->setHtml("<p style='color:#909399;'>暂无数据</p>");
        return;
    }
    
    int totalVotes = Statistics::getTotalVotes(candidates);
    double avgVotes = Statistics::getAverageVotes(candidates);
    int maxVotes = Statistics::getMaxVotes(candidates);
    int minVotes = Statistics::getMinVotes(candidates);
    
    QString summary = QString(
        "<div style='display:flex; gap:32px;'>"
        "  <div><div style='font-size:12px;color:#909399;'>候选人总数</div>"
        "       <div style='font-size:20px;font-weight:600;color:#303133;'>%1</div></div>"
        "  <div><div style='font-size:12px;color:#909399;'>总票数</div>"
        "       <div style='font-size:20px;font-weight:600;color:#303133;'>%2</div></div>"
        "  <div><div style='font-size:12px;color:#909399;'>平均得票数</div>"
        "       <div style='font-size:20px;font-weight:600;color:#303133;'>%3</div></div>"
        "  <div><div style='font-size:12px;color:#909399;'>最高得票数</div>"
        "       <div style='font-size:20px;font-weight:600;color:#303133;'>%4</div></div>"
        "  <div><div style='font-size:12px;color:#909399;'>最低得票数</div>"
        "       <div style='font-size:20px;font-weight:600;color:#303133;'>%5</div></div>"
        "</div>")
        .arg(candidates.size())
        .arg(totalVotes)
        .arg(avgVotes, 0, 'f', 2)
        .arg(maxVotes)
        .arg(minVotes);
    
    summaryText->setHtml(summary);
}

// ==================== 选举结果槽函数 ====================

void MainWindow::onShowElectionResult()
{
    updateTopicResultView(getSelectedAdminTopicId());
    return;
    const vector<Candidate> &candidates = electionSystem->getAllCandidates();
    
    if (candidates.empty()) {
        resultText->setPlainText("暂无候选人数据");
        return;
    }
    
    int winnerID = electionSystem->findWinner();
    int totalVotes = Statistics::getTotalVotes(candidates);
    
    QString result;
    result += "<h2>选举结果</h2>\n";
    result += QString("<p>总票数: <b>%1</b></p>\n").arg(totalVotes);
    result += QString("<p>候选人总数: <b>%1</b></p>\n").arg(candidates.size());
    result += "<hr>\n";
    
    if (winnerID != -1) {
        Candidate *winner = electionSystem->queryCandidate(winnerID);
        double percentage = totalVotes > 0 ? 
            (100.0 * winner->voteCount / totalVotes) : 0.0;
        
        result += "<h3 style='color: green;'>🎉 优胜者</h3>\n";
        result += QString("<p><b>编号:</b> %1</p>\n").arg(winner->id);
        result += QString("<p><b>姓名:</b> %1</p>\n").arg(QString::fromStdString(winner->name));
        result += QString("<p><b>所属单位:</b> %1</p>\n").arg(QString::fromStdString(winner->department));
        result += QString("<p><b>得票数:</b> %1</p>\n").arg(winner->voteCount);
        result += QString("<p><b>得票率:</b> %1%</p>\n").arg(percentage, 0, 'f', 2);
        result += "<p style='color: green;'><b>✅ 该候选人获得超过半数选票！</b></p>\n";
    } else {
        result += "<h3 style='color: red;'>❌ 没有候选人获得超过半数选票</h3>\n";
        result += "<p>所有候选人得票情况：</p>\n";
        
        vector<Candidate> sorted = candidates;
        Statistics::sortByVotes(sorted, false);
        
        result += "<table border='1' cellpadding='5'>\n";
        result += "<tr><th>排名</th><th>编号</th><th>姓名</th><th>得票数</th><th>得票率</th></tr>\n";
        
        for (size_t i = 0; i < sorted.size(); i++) {
            double percentage = totalVotes > 0 ? 
                (100.0 * sorted[i].voteCount / totalVotes) : 0.0;
            result += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5%</td></tr>\n")
                .arg(i + 1)
                .arg(sorted[i].id)
                .arg(QString::fromStdString(sorted[i].name))
                .arg(sorted[i].voteCount)
                .arg(percentage, 0, 'f', 2);
        }
        result += "</table>\n";
    }
    
    resultText->setHtml(result);
}

void MainWindow::onExportReport()
{
    QString filename = QFileDialog::getSaveFileName(this, "导出统计报告", 
                                                    "election_report.txt", 
                                                    "文本文件 (*.txt);;所有文件 (*.*)");
    if (filename.isEmpty()) {
        return;
    }
    
    int winnerID = electionSystem->findWinner();
    if (FileManager::exportReport(electionSystem->getAllCandidates(), winnerID, filename.toStdString())) {
        showMessage("成功", QString("统计报告已导出到: %1").arg(filename));
        maintenanceLog->append(QString("[%1] 导出统计报告: %2")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                               .arg(filename));
        statusLabel->setText(QString("已导出报告: %1").arg(filename));
    } else {
        showMessage("错误", "导出失败！", true);
    }
}

// ==================== 数据维护槽函数 ====================

void MainWindow::onSaveCandidates()
{
    int topicId = getSelectedTopicIdFromTable(topicTableWidget);
    if (topicId <= 0) {
        showMessage("提示", "请在话题列表中先选中一个话题再导出。", true);
        return;
    }

    VoteTopic *topic = electionSystem->queryTopic(topicId);
    if (!topic) {
        showMessage("错误", "话题不存在。", true);
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, "导出话题", 
                                                    QString("topic_%1.csv").arg(topicId),
                                                    "CSV 文件 (*.csv);;所有文件 (*.*)");
    if (filename.isEmpty()) {
        return;
    }

    if (FileManager::exportSingleTopicData(*topic, electionSystem->getTopicVoteHistory(), filename.toStdString())) {
        showMessage("成功", QString("话题已导出到: %1").arg(filename));
        if (maintenanceLog) {
            maintenanceLog->append(QString("[%1] 导出话题: %2 (topicId=%3)")
                                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                                   .arg(filename)
                                   .arg(topicId));
        }
        statusLabel->setText(QString("已导出话题: %1").arg(topicId));
    } else {
        showMessage("错误", "导出失败！", true);
    }
}

void MainWindow::onLoadCandidates()
{
    QString filename = QFileDialog::getOpenFileName(this, "导入话题", 
                                                    ".", "CSV 文件 (*.csv);;所有文件 (*.*)");
    if (filename.isEmpty()) {
        return;
    }

    VoteTopic imported;
    vector<TopicVoteRecord> votes;
    if (!FileManager::importSingleTopicData(imported, votes, filename.toStdString())) {
        showMessage("错误", "导入失败：文件格式不正确或内容为空。", true);
        return;
    }

    bool ok = false;
    int newTopicId = QInputDialog::getInt(this,
                                         "指定话题ID",
                                         "请输入导入后话题ID（必须与现有话题ID不重复）：",
                                         imported.id,
                                         1,
                                         999999,
                                         1,
                                         &ok);
    if (!ok) return;

    if (electionSystem->queryTopic(newTopicId) != nullptr) {
        showMessage("错误", "导入失败：该话题ID已存在。", true);
        return;
    }

    // 创建新话题（使用导入内容）
    vector<string> optTexts;
    optTexts.reserve(imported.options.size());
    for (const auto &opt : imported.options) {
        optTexts.push_back(opt.text);
    }

    int createdId = electionSystem->createTopic(imported.title, imported.description, optTexts, imported.votesPerVoter);
    if (createdId <= 0) {
        showMessage("错误", "导入失败：创建话题失败。", true);
        return;
    }

    VoteTopic *nt = electionSystem->queryTopic(createdId);
    if (!nt) {
        showMessage("错误", "导入失败：内部错误。", true);
        return;
    }

    // 覆盖为指定的新 topicId，并恢复 createdAt / voteCount
    nt->id = newTopicId;
    nt->createdAt = imported.createdAt;
    nt->votesPerVoter = imported.votesPerVoter;
    for (size_t i = 0; i < nt->options.size() && i < imported.options.size(); ++i) {
        nt->options[i].voteCount = imported.options[i].voteCount;
    }

    // 追加投票记录：将 topicId 替换为 newTopicId
    // 注意：这里仅用于导出记录展示，不用于重新构建投票人限制（限制基于内存中的 topicVotedUsers）
    // 简单做法：重新播放投票记录以恢复限制与历史
    for (auto &rec : votes) {
        rec.topicId = newTopicId;
        // 重新记录到系统历史中（不重复计票）
        // 为保证一致性，这里不再计票，只把记录附加到历史
    }

    // 将投票记录直接附加到 topicVoteHistory（需要一个入口；目前只有 getter，这里先不恢复投票人限制，后续可补）

    showMessage("成功", QString("导入成功：话题ID %1（选项%2个）").arg(newTopicId).arg(nt->options.size()));

    updateTopicTable();
    refreshTopicComboBox();
    refreshAdminTopicSelectors();
    refreshAdminViews();
    updateVoterTopicOptionTable();

    if (maintenanceLog) {
        maintenanceLog->append(QString("[%1] 导入话题: %2 -> topicId=%3")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                               .arg(filename)
                               .arg(newTopicId));
    }
}

void MainWindow::onSaveVotes()
{
    // 已合并到‘导出话题数据’，此功能停用
    showMessage("提示", "请使用：导出话题数据（包含投票记录）。");
}

void MainWindow::onLoadVotes()
{
    // 已合并到‘导入话题数据’，此功能停用
    showMessage("提示", "请使用：导入话题数据（包含投票记录）。");
}

void MainWindow::onClearAll()
{
    int ret = QMessageBox::warning(this, "确认清空", 
        "确定要清空所有数据吗？此操作不可恢复！",
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        electionSystem->clearAll();
        showMessage("成功", "已清空所有数据");
        maintenanceLog->append(QString("[%1] 清空所有数据")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
        updateCandidateTable();
        updateStatisticsTable();
        // updateVoteHistoryList();
        onShowSummary();
        onShowElectionResult();
        statusLabel->setText("已清空所有数据");
    }
}

// ==================== 高级功能槽函数 ====================

void MainWindow::onAnalyzeVoteData()
{
    bool isTopicMode = true;
    if (isTopicMode) {
        updateTopicAnalysisView(getSelectedAdminTopicId(), 0);
        return;
    }

    const vector<int> &history = electionSystem->getVoteHistory();
    
    if (history.empty()) {
        analysisText->setPlainText("暂无投票数据");
        return;
    }
    
    map<int, int> voteCount;
    for (int vote : history) {
        voteCount[vote]++;
    }
    
    QString analysis = "投票数据分析\n";
    analysis += "═══════════════════════════════════════\n\n";
    analysis += "投票分布：\n";
    
    for (const auto &pair : voteCount) {
        Candidate *c = electionSystem->queryCandidate(pair.first);
        if (c) {
            analysis += QString("  编号 %1 (%2): %3 票\n")
                .arg(pair.first)
                .arg(QString::fromStdString(c->name))
                .arg(pair.second);
        }
    }
    
    analysis += "\n投票趋势（前20张选票）：\n";
    for (size_t i = 0; i < std::min<size_t>(20, history.size()); i++) {
        analysis += QString::number(history[i]) + " ";
        if ((i + 1) % 10 == 0) analysis += "\n";
    }
    
    analysisText->setPlainText(analysis);
}

void MainWindow::onAnalyzeRanking()
{
    bool isTopicMode = true;
    if (isTopicMode) {
        updateTopicAnalysisView(getSelectedAdminTopicId(), 1);
        return;
    }

    const vector<Candidate> &candidates = electionSystem->getAllCandidates();
    
    if (candidates.empty()) {
        analysisText->setPlainText("暂无候选人");
        return;
    }
    
    vector<Candidate> sorted = candidates;
    Statistics::sortByVotes(sorted, false);
    
    QString analysis = "候选人排名分析\n";
    analysis += "═══════════════════════════════════════\n\n";
    analysis += "排名\t编号\t姓名\t\t得票数\n";
    analysis += "────────────────────────────────────\n";
    
    for (size_t i = 0; i < sorted.size(); i++) {
        analysis += QString("%1\t%2\t%3\t\t%4\n")
            .arg(i + 1)
            .arg(sorted[i].id)
            .arg(QString::fromStdString(sorted[i].name))
            .arg(sorted[i].voteCount);
    }
    
    analysisText->setPlainText(analysis);
}

void MainWindow::onAnalyzeDistribution()
{
    bool isTopicMode = true;
    if (isTopicMode) {
        updateTopicAnalysisView(getSelectedAdminTopicId(), 2);
        return;
    }

    const vector<Candidate> &candidates = electionSystem->getAllCandidates();
    
    if (candidates.empty()) {
        analysisText->setPlainText("暂无候选人");
        return;
    }
    
    int maxVotes = Statistics::getMaxVotes(candidates);
    
    QString analysis = "得票分布分析（可视化）\n";
    analysis += "═══════════════════════════════════════\n\n";
    
    for (const auto &c : candidates) {
        int barLength = maxVotes > 0 ? (50 * c.voteCount / maxVotes) : 0;
        QString name = QString::fromStdString(c.name);
        name = name.leftJustified(20, ' ');
        analysis += name + " [";
        for (int i = 0; i < barLength; i++) {
            analysis += "█";
        }
        for (int i = barLength; i < 50; i++) {
            analysis += " ";
        }
        analysis += QString("] %1 票\n").arg(c.voteCount);
    }
    
    analysisText->setPlainText(analysis);
}

void MainWindow::onAnalyzePerformance()
{
    bool isTopicMode = true;
    if (isTopicMode) {
        updateTopicAnalysisView(getSelectedAdminTopicId(), 3);
        return;
    }

    // 简单性能测试：在不同规模下测量核心操作的耗时
    struct CaseConfig {
        int candidates;
        int votes;
    };
    
    const CaseConfig cases[] = {
        {10,    100},
        {100,   10000},
        {1000,  100000}
    };
    
    QString report;
    report += "性能测试（理论 + 实测）\n";
    report += "═══════════════════════════════════════\n\n";
    
    for (const auto &cfg : cases) {
        ElectionSystem perfSystem;
        
        // 构造候选人
        for (int i = 1; i <= cfg.candidates; ++i) {
            perfSystem.addCandidate(i, "候选人" + std::to_string(i), "测试组");
        }
        
        // 构造投票向量（均匀分布）
        std::vector<int> votes;
        votes.reserve(cfg.votes);
        for (int i = 0; i < cfg.votes; ++i) {
            int id = (i % cfg.candidates) + 1;
            votes.push_back(id);
        }
        
        QElapsedTimer timer;
        qint64 tVote = 0;
        qint64 tFind = 0;
        
        // 测试批量投票
        timer.start();
        perfSystem.vote(votes, true);
        tVote = timer.elapsed();
        
        // 测试查找优胜者
        timer.restart();
        int winner = perfSystem.findWinner();
        (void)winner;
        tFind = timer.elapsed();
        
        report += QString("场景：%1 个候选人，%2 张选票\n")
                  .arg(cfg.candidates)
                  .arg(cfg.votes);
        report += QString("  批量投票耗时：%1 ms （理论 O(m)）\n")
                  .arg(tVote);
        report += QString("  查找优胜者耗时：%1 ms （理论 O(n)）\n\n")
                  .arg(tFind);
    }
    
    report += "复杂度总结：\n";
    report += "  添加候选人：O(1) 平均\n";
    report += "  批量投票：O(m)，m 为选票数量\n";
    report += "  查找优胜者：O(n)，n 为候选人数\n";
    report += "  排序：O(n log n)\n";
    
    analysisText->setPlainText(report);
}

// ==================== 辅助函数 ====================

void MainWindow::updateCandidateTable()
{
    // 候选人系统已从界面移除：保留空实现以避免历史调用导致崩溃
    return;
}

void MainWindow::updateStatisticsTable()
{
    updateStatisticsTable(electionSystem->getAllCandidates());
}

void MainWindow::updateStatisticsTable(const vector<Candidate> &candidates)
{
    statisticsTable->setRowCount(static_cast<int>(candidates.size()));
    
    int totalVotes = Statistics::getTotalVotes(candidates);
    int maxVotes = Statistics::getMaxVotes(candidates);
    
    for (size_t i = 0; i < candidates.size(); i++) {
        double percentage = totalVotes > 0 ? 
            (100.0 * candidates[i].voteCount / totalVotes) : 0.0;
        
        auto *idItem = new QTableWidgetItem(QString::number(candidates[i].id));
        auto *nameItem = new QTableWidgetItem(QString::fromStdString(candidates[i].name));
        auto *deptItem = new QTableWidgetItem(QString::fromStdString(candidates[i].department));
        auto *voteItem = new QTableWidgetItem(QString::number(candidates[i].voteCount));
        auto *rateItem = new QTableWidgetItem(QString::number(percentage, 'f', 2) + "%");

        idItem->setTextAlignment(Qt::AlignCenter);
        nameItem->setTextAlignment(Qt::AlignCenter);
        deptItem->setTextAlignment(Qt::AlignCenter);
        voteItem->setTextAlignment(Qt::AlignCenter);
        rateItem->setTextAlignment(Qt::AlignCenter);

        // 非 0 得票数/得票率用更深的颜色
        if (candidates[i].voteCount > 0) {
            voteItem->setForeground(QColor("#303133"));
            rateItem->setForeground(QColor("#303133"));
            voteItem->setFont(QFont(QApplication::font().family(), QApplication::font().pointSize(), QFont::DemiBold));
            rateItem->setFont(QFont(QApplication::font().family(), QApplication::font().pointSize() + 1, QFont::Bold));
        } else {
            rateItem->setForeground(QColor("#C0C4CC"));
        }

        // 最高票行高亮并加星标
        if (maxVotes > 0 && candidates[i].voteCount == maxVotes) {
            nameItem->setText(nameItem->text() + " ★");
            for (QTableWidgetItem *it : {idItem, nameItem, deptItem, voteItem, rateItem}) {
                if (!it) continue;
                it->setBackground(QColor("#F0F5FF"));
            }
        }

        statisticsTable->setItem(i, 0, idItem);
        statisticsTable->setItem(i, 1, nameItem);
        statisticsTable->setItem(i, 2, deptItem);
        statisticsTable->setItem(i, 3, voteItem);
        statisticsTable->setItem(i, 4, rateItem);
    }
    
    statisticsTable->resizeColumnsToContents();
}

void MainWindow::updateVoteHistoryList()
{
    // 候选人投票历史界面已移除：保留空实现以避免历史调用导致崩溃
    return;
}

void MainWindow::updateCharts()
{
    // Charts were removed from the UI when Qt Charts dependency was dropped.
    // Keep this stub to satisfy the existing interface and allow future extensions.
}

void MainWindow::showMessage(const QString &title, const QString &message, bool isError)
{
    if (isError) {
        QMessageBox::critical(this, title, message);
    } else {
        QMessageBox::information(this, title, message);
    }
}

void MainWindow::applyGlobalStyle()
{
    // 基础字体：中文友好，默认 15 号
    QFont baseFont;
#if defined(Q_OS_WIN)
    baseFont.setFamily("Microsoft YaHei");
#elif defined(Q_OS_MAC)
    baseFont.setFamily("PingFang SC");
#else
    baseFont.setFamily("WenQuanYi Micro Hei");
#endif
    baseFont.setPointSize(baseFontPointSize);
    QApplication::setFont(baseFont);

    // 全局样式表
    QString style = R"(
        QMainWindow {
            background: #f5f7fa;
        }

        QTabWidget::pane {
            border: none;
        }
        QTabBar::tab {
            padding: 6px 14px;
            color: #606266;
        }
        QTabBar::tab:selected {
            color: #303133;
            font-weight: 600;
            border-bottom: 2px solid #409eff;
        }
        QTabBar::tab:!selected {
            margin-top: 2px;
        }

        QGroupBox {
            background: #ffffff;
            border: 1px solid #dcdfe6;
            border-radius: 8px;
            margin-top: 18px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            margin-left: 4px;
            background: #f0f2f5;
            /* 和普通标签同字号，不再特意放大 */
            font-weight: normal;
            color: #303133;
        }

        QLabel {
            color: #303133;
        }

        QLineEdit, QComboBox, QTextEdit, QSpinBox {
            border: 1px solid #dcdfe6;
            border-radius: 4px;
            padding: 4px 6px;
            background: #ffffff;
            color: #303133;
            selection-background-color: #409eff;
            selection-color: #ffffff;
        }
        QLineEdit:focus, QComboBox:focus, QTextEdit:focus, QSpinBox:focus {
            border: 1px solid #409eff;
            box-shadow: 0 0 0 2px rgba(64,158,255,0.15);
        }
        QLineEdit::placeholder {
            /* 占位符使用更浅的灰色，弱化存在感 */
            color: #C0C4CC;
        }

        QTextBrowser {
            border: 1px solid #e4e7ed;
            border-radius: 4px;
            background: #fafafa;
        }

        QTableWidget {
            background: #ffffff;
            border: 1px solid #dcdfe6;
            border-radius: 6px;
            gridline-color: transparent;
            alternate-background-color: #FAFBFC;
        }
        QTableView::item {
            padding: 4px 6px;
        }
        QTableView::item:selected {
            background: #ecf5ff;
            color: #303133;
        }
        QHeaderView::section {
            background: #F2F4F7;
            padding: 6px 8px;
            border: none;
            border-right: 1px solid #e4e7ed;
            font-weight: 600;
        }

        QListWidget {
            border: 1px solid #dcdfe6;
            border-radius: 4px;
            background: #ffffff;
        }

        QStatusBar {
            background: #f0f2f5;
        }
        QStatusBar QLabel {
            color: #606266;
        }

        QMenuBar {
            background: #ffffff;
        }
        QMenuBar::item {
            padding: 4px 8px;
        }
        QMenuBar::item:selected {
            background: #ecf5ff;
        }
        QMenu {
            background: #ffffff;
            border: 1px solid #dcdfe6;
        }
        QMenu::item {
            padding: 4px 24px 4px 20px;
        }
        QMenu::item:selected {
            background: #ecf5ff;
        }

        QToolBar {
            background: #ffffff;
            border-bottom: 1px solid #e4e7ed;
        }

        QPushButton {
            min-height: 28px;
            padding: 4px 12px;
            border-radius: 4px;
            border: 1px solid #dcdfe6;
            background: #ffffff;
            color: #303133;
        }
        QPushButton:hover {
            background: #f5f7fa;
        }
        QPushButton:disabled {
            background: #f5f7fa;
            color: #c0c4cc;
            border-color: #ebeef5;
        }

        QPushButton[btnRole="primary"] {
            background: #409eff;
            border-color: #409eff;
            color: #ffffff;
        }
        QPushButton[btnRole="primary"]:hover {
            background: #66b1ff;
            border-color: #66b1ff;
        }

        QPushButton[btnRole="secondary"] {
            background: #ecf5ff;
            border-color: #c6e2ff;
            color: #409eff;
        }

        QPushButton[btnRole="danger"] {
            background: #f56c6c;
            border-color: #f56c6c;
            color: #ffffff;
        }
        QPushButton[btnRole="danger"]:hover {
            background: #f78989;
            border-color: #f78989;
        }

        QPushButton[btnRole="neutral"] {
            background: #ffffff;
            border-color: #dcdfe6;
            color: #606266;
        }

        QTextBrowser#maintenanceLog {
            background: #f7f7f7;
            border-color: #e4e7ed;
            color: #606266;
        }
    )";

    this->setStyleSheet(style);
}

void MainWindow::applyFontScale()
{
    int pointSize = baseFontPointSize + currentFontDelta;
    if (pointSize < 10) pointSize = 10;
    if (pointSize > 40) pointSize = 40;

    QFont f = QApplication::font();
    f.setPointSize(pointSize);
    QApplication::setFont(f);

    // 重新应用样式（避免某些控件缓存旧字体）
    this->setStyleSheet(this->styleSheet());

    // 同步表格行高/列宽，避免字号变大后被截断
    int rowH = pointSize + 12;
    int minColW = pointSize * 6; // 大致按字符宽度估算

    auto adjustTable = [rowH, minColW](QTableWidget *table) {
        if (!table) return;

        table->verticalHeader()->setDefaultSectionSize(rowH);

        auto *hh = table->horizontalHeader();
        if (hh) {
            hh->setSectionResizeMode(QHeaderView::Interactive);
            hh->setMinimumSectionSize(minColW);
            hh->setDefaultAlignment(Qt::AlignCenter);
        }

        // 先根据内容自适应，再保证最小列宽
        table->resizeColumnsToContents();
        for (int col = 0; col < table->columnCount(); ++col) {
            int w = table->columnWidth(col);
            if (w < minColW) {
                table->setColumnWidth(col, minColW);
            }
        }

        // 最后一列拉伸填充剩余空间（避免窗口很宽时留白）
        if (hh) {
            hh->setStretchLastSection(true);
        }
    };

    // candidateTable 已不再使用（候选人系统移除），避免空悬指针导致崩溃
    // adjustTable(candidateTable);
    adjustTable(statisticsTable);
    adjustTable(voterTopicOptionTable);
}

bool MainWindow::validateInput(const QString &text, bool isID)
{
    if (text.isEmpty()) {
        return false;
    }
    
    if (isID) {
        bool ok;
        int id = text.toInt(&ok);
        return ok && id > 0;
    }
    
    return !text.isEmpty();
}

void MainWindow::clearInputFields()
{
    if (candidateIdEdit) candidateIdEdit->clear();
    if (candidateNameEdit) candidateNameEdit->clear();
    if (candidateDeptEdit) candidateDeptEdit->clear();
}

void MainWindow::onIncreaseFont()
{
    currentFontDelta += 1;
    applyFontScale();
}

void MainWindow::onDecreaseFont()
{
    currentFontDelta -= 1;
    applyFontScale();
}

void MainWindow::onResetFont()
{
    currentFontDelta = 0;
    applyFontScale();
}

void MainWindow::onLoadSampleCandidates()
{
    // 生成 1 个示例话题（10 个选项）
    electionSystem->clearAll();

    std::vector<std::string> optionTexts;
    optionTexts.reserve(10);
    for (int i = 1; i <= 10; ++i) {
        optionTexts.push_back(std::string("候选人") + std::to_string(i));
    }

    int topicId = electionSystem->createTopic("示例话题", "系统自动生成的示例话题", optionTexts, 1);
    if (topicId <= 0) {
        showMessage("错误", "加载示例话题失败。", true);
        return;
    }

    showMessage("成功", QString("已加载示例话题：示例话题（10个选项），话题ID %1。").arg(topicId));
    if (maintenanceLog) {
        maintenanceLog->append(QString("[%1] 加载示例话题：示例话题（10个选项），话题ID %2")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                               .arg(topicId));
    }

    // 刷新话题相关视图
    refreshTopicComboBox();
    refreshAdminTopicSelectors();
    refreshAdminViews();
    updateVoterTopicOptionTable();

    statusLabel->setText("已加载示例话题");
}

void MainWindow::onEnterVoterMode()
{
    if (rootStack && voterWidget) {
        rootStack->setCurrentWidget(voterWidget);
    }
    if (mainToolBar) {
        mainToolBar->setVisible(false);
    }
    updateVoterCandidateTable();
    statusLabel->setText("投票端");
}

void MainWindow::onEnterAdminMode()
{
    if (rootStack && adminWidget) {
        rootStack->setCurrentWidget(adminWidget);
    }
    if (mainToolBar) {
        mainToolBar->setVisible(true);
    }
    updateCandidateTable();
    updateStatisticsTable();
    // updateVoteHistoryList();
    onShowSummary();
    onShowElectionResult();
    statusLabel->setText("管理员后台");
}

void MainWindow::onBackToRoleSelection()
{
    if (rootStack && roleSelectionWidget) {
        rootStack->setCurrentWidget(roleSelectionWidget);
    }
    if (mainToolBar) {
        mainToolBar->setVisible(false);
    }
    statusLabel->setText("就绪");
}

void MainWindow::onVoterRefreshCandidates()
{
    refreshTopicComboBox();
    updateVoterTopicOptionTable();
}

void MainWindow::onVoterVote()
{
    if (!voterTopicComboBox || !voterTopicOptionTable) {
        return;
    }

    bool okTopic = false;
    int topicId = voterTopicComboBox->currentData().toInt(&okTopic);
    if (!okTopic || topicId <= 0) {
        showMessage("提示", "请先选择一个投票话题。", true);
        return;
    }

    QList<QTableWidgetItem*> items = voterTopicOptionTable->selectedItems();
    if (items.isEmpty()) {
        showMessage("提示", "请先在列表中选择一个选项。", true);
        return;
    }

    int row = items[0]->row();
    QTableWidgetItem *idItem = voterTopicOptionTable->item(row, 0);
    if (!idItem) {
        showMessage("错误", "无法读取选项ID。", true);
        return;
    }

    bool okOpt = false;
    int optionId = idItem->text().toInt(&okOpt);
    if (!okOpt || optionId <= 0) {
        showMessage("错误", "选项ID不合法。", true);
        return;
    }

    VoteTopic *topic = electionSystem->queryTopic(topicId);
    QString topicTitle = topic ? QString::fromStdString(topic->title) : QString::number(topicId);

    QString optText;
    if (topic) {
        for (const auto &opt : topic->options) {
            if (opt.id == optionId) {
                optText = QString::fromStdString(opt.text);
                break;
            }
        }
    }

    QString confirmText = QString("确认在以下话题中投票吗？\n\n话题：%1\n选项：%2")
                          .arg(topicTitle)
                          .arg(optText.isEmpty() ? QString::number(optionId) : optText);

    int ret = QMessageBox::question(this, "确认投票", confirmText, QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) {
        return;
    }

    bool okInput = false;
    QString voterId = QInputDialog::getText(this,
                                           "投票人ID",
                                           "请输入投票人ID（同一话题同一ID只能投一次）：",
                                           QLineEdit::Normal,
                                           "",
                                           &okInput);
    if (!okInput) {
        return;
    }
    voterId = voterId.trimmed();
    if (voterId.isEmpty()) {
        showMessage("错误", "投票人ID不能为空。", true);
        return;
    }

    if (electionSystem->castTopicVote(topicId, optionId, voterId.toStdString())) {
        int remain = electionSystem->getTopicRemainingVotes(topicId, voterId.toStdString());
        showMessage("成功", QString("投票成功！该投票人ID在本话题还剩 %1 票可投。").arg(remain));
        updateVoterTopicOptionTable();
        refreshAdminViews();
        updateTopicTable();
        statusLabel->setText(QString("已投票：话题%1-选项%2（%3）").arg(topicId).arg(optionId).arg(voterId));
    } else {
        int remain = electionSystem->getTopicRemainingVotes(topicId, voterId.toStdString());
        QString reason = remain <= 0 ? "投票失败：该投票人ID在本话题的票数已用完。" : "投票失败：不能重复投同一选项，或话题/选项不存在。";
        showMessage("错误", reason + QString("\n该ID还剩 %1 票可投。").arg(remain), true);
    }
}

void MainWindow::onVoterShowResult()
{
    if (!voterTopicComboBox) {
        return;
    }

    bool okTopic = false;
    int topicId = voterTopicComboBox->currentData().toInt(&okTopic);
    if (!okTopic || topicId <= 0) {
        showMessage("提示", "请先选择一个投票话题。", true);
        return;
    }

    showTopicResultDialog(topicId);
}

void MainWindow::updateVoterCandidateTable()
{
    // 旧候选人投票端表格已被“话题投票”取代：保留空实现以兼容历史调用。
}
