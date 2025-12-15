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
#include <sstream>
#include <iomanip>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      electionSystem(new ElectionSystem()),
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
    mainToolBar = addToolBar("主工具栏");
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    QAction *addCandidateAction = mainToolBar->addAction("添加候选人");
    connect(addCandidateAction, &QAction::triggered, this, &MainWindow::onAddCandidate);
    
    QAction *voteAction = mainToolBar->addAction("投票");
    connect(voteAction, &QAction::triggered, this, &MainWindow::onSingleVote);
    
    mainToolBar->addSeparator();
    
    QAction *resultAction = mainToolBar->addAction("查看结果");
    connect(resultAction, &QAction::triggered, this, &MainWindow::onShowElectionResult);
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
    mainTabWidget = new QTabWidget(this);
    setCentralWidget(mainTabWidget);
    
    createCandidateManagementWidget();
    createVoteManagementWidget();
    createStatisticsWidget();
    createElectionResultWidget();
    createDataMaintenanceWidget();
    createAdvancedFeaturesWidget();
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
    
    saveCandidatesBtn = new QPushButton("保存候选人数据");
    loadCandidatesBtn = new QPushButton("加载候选人数据");
    saveVotesBtn = new QPushButton("保存投票数据");
    loadVotesBtn = new QPushButton("加载投票数据");
    loadSampleCandidatesBtn = new QPushButton("加载示例候选人");
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
        updateVoteHistoryList();
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
    updateCandidateTable();
    updateStatisticsTable();
    updateVoteHistoryList();
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
        updateVoteHistoryList();
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
        updateVoteHistoryList();
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
        updateVoteHistoryList();
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
        updateVoteHistoryList();
        onShowSummary();
        onShowElectionResult();
        statusLabel->setText(QString("已撤销 %1 张选票").arg(undone));
    } else {
        showMessage("提示", "没有可撤销的投票记录。");
    }
}

void MainWindow::onShowVoteHistory()
{
    updateVoteHistoryList();
}

// ==================== 查询统计槽函数 ====================

void MainWindow::onShowStatistics()
{
    updateStatisticsTable();
    onShowSummary();
}

void MainWindow::onSortCandidates()
{
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
    QString filename = QFileDialog::getSaveFileName(this, "保存候选人数据", 
                                                    "candidates.csv", 
                                                    "数据文件 (*.csv *.txt);;CSV 文件 (*.csv);;文本文件 (*.txt);;所有文件 (*.*)");
    if (filename.isEmpty()) {
        return;
    }
    
    if (FileManager::saveCandidates(electionSystem->getAllCandidates(), filename.toStdString())) {
        showMessage("成功", QString("候选人数据已保存到: %1").arg(filename));
        maintenanceLog->append(QString("[%1] 保存候选人数据: %2")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                               .arg(filename));
        statusLabel->setText(QString("已保存候选人数据: %1").arg(filename));
    } else {
        showMessage("错误", "保存失败！", true);
    }
}

void MainWindow::onLoadCandidates()
{
    QString filename = QFileDialog::getOpenFileName(this, "加载候选人数据", 
                                                    ".", "数据文件 (*.csv *.txt);;CSV 文件 (*.csv);;文本文件 (*.txt);;所有文件 (*.*)");
    if (filename.isEmpty()) {
        return;
    }
    
    vector<Candidate> candidates;
    if (FileManager::loadCandidates(candidates, filename.toStdString())) {
        // 清空现有数据并加载
        electionSystem->clearAll();
        for (const auto &c : candidates) {
            electionSystem->addCandidate(c.id, c.name, c.department);
            // 恢复得票数
            Candidate *loaded = electionSystem->queryCandidate(c.id);
            if (loaded) {
                loaded->voteCount = c.voteCount;
            }
        }
        
        showMessage("成功", QString("成功加载 %1 个候选人").arg(candidates.size()));
        maintenanceLog->append(QString("[%1] 加载候选人数据: %2 (%3个候选人)")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                               .arg(filename)
                               .arg(candidates.size()));
        updateCandidateTable();
        updateStatisticsTable();
        onShowSummary();
        onShowElectionResult();
        statusLabel->setText(QString("已加载 %1 个候选人").arg(candidates.size()));
    } else {
        showMessage("错误", "加载失败！", true);
    }
}

void MainWindow::onSaveVotes()
{
    QString filename = QFileDialog::getSaveFileName(this, "保存投票数据", 
                                                    "votes.csv", 
                                                    "数据文件 (*.csv *.txt);;CSV 文件 (*.csv);;文本文件 (*.txt);;所有文件 (*.*)");
    if (filename.isEmpty()) {
        return;
    }
    
    if (FileManager::saveVotes(electionSystem->getVoteHistory(), filename.toStdString())) {
        showMessage("成功", QString("投票数据已保存到: %1").arg(filename));
        maintenanceLog->append(QString("[%1] 保存投票数据: %2")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                               .arg(filename));
        statusLabel->setText(QString("已保存投票数据: %1").arg(filename));
    } else {
        showMessage("错误", "保存失败！", true);
    }
}

void MainWindow::onLoadVotes()
{
    QString filename = QFileDialog::getOpenFileName(this, "加载投票数据", 
                                                    ".", "数据文件 (*.csv *.txt);;CSV 文件 (*.csv);;文本文件 (*.txt);;所有文件 (*.*)");
    if (filename.isEmpty()) {
        return;
    }
    
    vector<int> votes;
    if (FileManager::loadVotes(votes, filename.toStdString())) {
        // 数据维护中的“加载投票数据”用于从文件重建一次完整投票结果，
        // 因此这里先主动清零，再重新累加。
        electionSystem->resetVotes();
        electionSystem->vote(votes, true);
        showMessage("成功", QString("成功加载 %1 张选票").arg(votes.size()));
        maintenanceLog->append(QString("[%1] 加载投票数据: %2 (%3张选票)")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                               .arg(filename)
                               .arg(votes.size()));
        updateCandidateTable();
        updateStatisticsTable();
        updateVoteHistoryList();
        onShowSummary();
        onShowElectionResult();
        statusLabel->setText(QString("已加载 %1 张选票").arg(votes.size()));
    } else {
        showMessage("错误", "加载失败！", true);
    }
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
        updateVoteHistoryList();
        onShowSummary();
        onShowElectionResult();
        statusLabel->setText("已清空所有数据");
    }
}

// ==================== 高级功能槽函数 ====================

void MainWindow::onAnalyzeVoteData()
{
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
    for (size_t i = 0; i < std::min(20UL, history.size()); i++) {
        analysis += QString::number(history[i]) + " ";
        if ((i + 1) % 10 == 0) analysis += "\n";
    }
    
    analysisText->setPlainText(analysis);
}

void MainWindow::onAnalyzeRanking()
{
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
    const vector<Candidate> &candidates = electionSystem->getAllCandidates();
    
    candidateTable->setRowCount(static_cast<int>(candidates.size()));
    
    for (size_t i = 0; i < candidates.size(); i++) {
        auto *idItem   = new QTableWidgetItem(QString::number(candidates[i].id));
        auto *nameItem = new QTableWidgetItem(QString::fromStdString(candidates[i].name));
        auto *deptItem = new QTableWidgetItem(QString::fromStdString(candidates[i].department));
        auto *voteItem = new QTableWidgetItem(QString::number(candidates[i].voteCount));

        idItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        voteItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        candidateTable->setItem(i, 0, idItem);
        candidateTable->setItem(i, 1, nameItem);
        candidateTable->setItem(i, 2, deptItem);
        candidateTable->setItem(i, 3, voteItem);
    }
    
    candidateTable->resizeColumnsToContents();

    if (candidateEmptyLabel) {
        bool hasData = !candidates.empty();
        candidateTable->parentWidget()->setProperty("currentIndex", hasData ? 0 : 1);
        candidateEmptyLabel->setVisible(!hasData);
    }
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

        // 数字列右对齐
        idItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        voteItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rateItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

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
    const vector<int> &history = electionSystem->getVoteHistory();
    
    voteHistoryList->clear();
    
    QStringList items;
    for (int vote : history) {
        items << QString::number(vote);
    }
    
    voteHistoryList->addItems(items);
    
    if (!history.empty()) {
        voteHistoryList->scrollToBottom();
    }
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
            font-size: 13px;
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
            font-size: 13px;
            color: #303133;
        }

        QLabel {
            color: #303133;
            font-size: 13px;
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
            font-size: 13px;
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
            font-size: 13px;
        }
    )";

    this->setStyleSheet(style);
}

void MainWindow::applyFontScale()
{
    int pointSize = baseFontPointSize + currentFontDelta;
    if (pointSize < 10) pointSize = 10;
    if (pointSize > 22) pointSize = 22;

    QFont f = QApplication::font();
    f.setPointSize(pointSize);
    QApplication::setFont(f);

    // 同步表格行高
    int rowH = pointSize + 10;
    auto adjustTable = [rowH](QTableWidget *table) {
        if (!table) return;
        table->verticalHeader()->setDefaultSectionSize(rowH);
    };
    adjustTable(candidateTable);
    adjustTable(statisticsTable);
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
    candidateIdEdit->clear();
    candidateNameEdit->clear();
    candidateDeptEdit->clear();
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
    struct SampleCandidate {
        int id;
        const char *name;
        const char *dept;
    };
    
    const SampleCandidate samples[] = {
        {1,  "张三",   "计算机学院"},
        {2,  "李四",   "计算机学院"},
        {3,  "王五",   "数学学院"},
        {4,  "赵六",   "数学学院"},
        {5,  "孙琪",   "物理学院"},
        {6,  "周八",   "物理学院"},
        {7,  "吴九",   "经管学院"},
        {8,  "郑十",   "经管学院"},
        {9,  "陈一",   "外国语学院"},
        {10, "杨二",   "外国语学院"}
    };
    
    // 清空现有候选人和投票数据
    electionSystem->clearAll();
    
    // 导入示例候选人
    for (const auto &s : samples) {
        electionSystem->addCandidate(s.id, s.name, s.dept);
    }
    
    showMessage("成功", "已加载示例候选人名单（10人）。");
    if (maintenanceLog) {
        maintenanceLog->append(QString("[%1] 加载示例候选人名单（10人）")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));
    }
    
    updateCandidateTable();
    updateStatisticsTable();
    onShowSummary();
    onShowElectionResult();
    statusLabel->setText("已加载示例候选人名单");
}

