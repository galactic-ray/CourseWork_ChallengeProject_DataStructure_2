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
#include <sstream>
#include <iomanip>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), electionSystem(new ElectionSystem())
{
    setWindowTitle("投票选举管理系统 v2.0 - GUI版");
    setMinimumSize(1000, 700);
    resize(1200, 800);
    
    createMenus();
    createToolBars();
    createStatusBar();
    createCentralWidget();
    
    statusLabel->setText("就绪");
}

MainWindow::~MainWindow()
{
    delete electionSystem;
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction *saveCandidatesAction = fileMenu->addAction("保存候选人数据(&S)");
    saveCandidatesAction->setShortcut(QKeySequence::Save);
    connect(saveCandidatesAction, &QAction::triggered, this, &MainWindow::onSaveCandidates);
    
    QAction *loadCandidatesAction = fileMenu->addAction("加载候选人数据(&L)");
    loadCandidatesAction->setShortcut(QKeySequence::Open);
    connect(loadCandidatesAction, &QAction::triggered, this, &MainWindow::onLoadCandidates);
    
    fileMenu->addSeparator();
    
    QAction *exportReportAction = fileMenu->addAction("导出统计报告(&E)");
    connect(exportReportAction, &QAction::triggered, this, &MainWindow::onExportReport);
    
    fileMenu->addSeparator();
    
    QAction *exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    editMenu = menuBar()->addMenu("编辑(&E)");
    
    QAction *clearAllAction = editMenu->addAction("清空所有数据(&C)");
    connect(clearAllAction, &QAction::triggered, this, &MainWindow::onClearAll);
    
    viewMenu = menuBar()->addMenu("视图(&V)");
    
    QAction *refreshAction = viewMenu->addAction("刷新(&R)");
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, this, [this]() {
        updateCandidateTable();
        updateStatisticsTable();
        updateVoteHistoryList();
        onShowElectionResult();
    });
    
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
    candidateIdEdit->setValidator(new QIntValidator(1, 999999, this));
    formLayout->addRow("编号(&I):", candidateIdEdit);
    
    candidateNameEdit = new QLineEdit();
    candidateNameEdit->setMaxLength(50);
    formLayout->addRow("姓名(&N):", candidateNameEdit);
    
    candidateDeptEdit = new QLineEdit();
    candidateDeptEdit->setMaxLength(100);
    formLayout->addRow("所属单位(&D):", candidateDeptEdit);
    
    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addCandidateBtn = new QPushButton("添加");
    modifyCandidateBtn = new QPushButton("修改");
    deleteCandidateBtn = new QPushButton("删除");
    queryCandidateBtn = new QPushButton("查询");
    refreshCandidateBtn = new QPushButton("刷新");
    
    buttonLayout->addWidget(addCandidateBtn);
    buttonLayout->addWidget(modifyCandidateBtn);
    buttonLayout->addWidget(deleteCandidateBtn);
    buttonLayout->addWidget(queryCandidateBtn);
    buttonLayout->addWidget(refreshCandidateBtn);
    buttonLayout->addStretch();
    
    formLayout->addRow(buttonLayout);
    
    // 表格区域
    QGroupBox *tableGroup = new QGroupBox("候选人列表");
    QVBoxLayout *tableLayout = new QVBoxLayout(tableGroup);
    
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
    
    tableLayout->addWidget(candidateTable);
    
    mainLayout->addWidget(inputGroup);
    mainLayout->addWidget(tableGroup);
    
    // 连接信号
    connect(addCandidateBtn, &QPushButton::clicked, this, &MainWindow::onAddCandidate);
    connect(modifyCandidateBtn, &QPushButton::clicked, this, &MainWindow::onModifyCandidate);
    connect(deleteCandidateBtn, &QPushButton::clicked, this, &MainWindow::onDeleteCandidate);
    connect(queryCandidateBtn, &QPushButton::clicked, this, &MainWindow::onQueryCandidate);
    connect(refreshCandidateBtn, &QPushButton::clicked, this, &MainWindow::onRefreshCandidateList);
    
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
    
    // 投票历史区域
    QGroupBox *historyGroup = new QGroupBox("投票历史");
    QVBoxLayout *historyLayout = new QVBoxLayout(historyGroup);
    
    voteHistoryList = new QListWidget();
    historyLayout->addWidget(voteHistoryList);
    
    QPushButton *showHistoryBtn = new QPushButton("刷新历史");
    connect(showHistoryBtn, &QPushButton::clicked, this, &MainWindow::onShowVoteHistory);
    historyLayout->addWidget(showHistoryBtn);
    
    mainLayout->addWidget(singleVoteGroup);
    mainLayout->addWidget(batchVoteGroup);
    mainLayout->addWidget(historyGroup);
    
    // 连接信号
    connect(singleVoteBtn, &QPushButton::clicked, this, &MainWindow::onSingleVote);
    connect(batchVoteBtn, &QPushButton::clicked, this, &MainWindow::onBatchVote);
    connect(importVotesBtn, &QPushButton::clicked, this, &MainWindow::onImportVotesFromFile);
    connect(resetVotesBtn, &QPushButton::clicked, this, &MainWindow::onResetVotes);
    
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
    
    // 排序和刷新
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(new QLabel("排序方式:"));
    sortComboBox = new QComboBox();
    sortComboBox->addItems(QStringList() << "按得票数降序" << "按得票数升序" 
                                         << "按编号" << "按姓名");
    controlLayout->addWidget(sortComboBox);
    
    sortBtn = new QPushButton("排序");
    refreshStatisticsBtn = new QPushButton("刷新");
    controlLayout->addWidget(sortBtn);
    controlLayout->addWidget(refreshStatisticsBtn);
    controlLayout->addStretch();
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
    connect(refreshStatisticsBtn, &QPushButton::clicked, this, &MainWindow::onShowStatistics);
    
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
    refreshResultBtn = new QPushButton("刷新结果");
    exportReportBtn = new QPushButton("导出报告");
    buttonLayout->addWidget(refreshResultBtn);
    buttonLayout->addWidget(exportReportBtn);
    buttonLayout->addStretch();
    resultLayout->addLayout(buttonLayout);
    
    mainLayout->addWidget(resultGroup);
    
    // 连接信号
    connect(refreshResultBtn, &QPushButton::clicked, this, &MainWindow::onShowElectionResult);
    connect(exportReportBtn, &QPushButton::clicked, this, &MainWindow::onExportReport);
    
    mainTabWidget->addTab(resultWidget, "选举结果");
}

void MainWindow::createDataMaintenanceWidget()
{
    maintenanceWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(maintenanceWidget);
    
    QGroupBox *operationGroup = new QGroupBox("数据操作");
    QGridLayout *gridLayout = new QGridLayout(operationGroup);
    
    saveCandidatesBtn = new QPushButton("保存候选人数据");
    loadCandidatesBtn = new QPushButton("加载候选人数据");
    saveVotesBtn = new QPushButton("保存投票数据");
    loadVotesBtn = new QPushButton("加载投票数据");
    clearAllBtn = new QPushButton("清空所有数据");
    
    gridLayout->addWidget(saveCandidatesBtn, 0, 0);
    gridLayout->addWidget(loadCandidatesBtn, 0, 1);
    gridLayout->addWidget(saveVotesBtn, 1, 0);
    gridLayout->addWidget(loadVotesBtn, 1, 1);
    gridLayout->addWidget(clearAllBtn, 2, 0, 1, 2);
    
    QGroupBox *logGroup = new QGroupBox("操作日志");
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
    buttonLayout->addWidget(analyzeVoteDataBtn);
    buttonLayout->addWidget(analyzeRankingBtn);
    buttonLayout->addWidget(analyzeDistributionBtn);
    buttonLayout->addStretch();
    analysisLayout->addLayout(buttonLayout);
    
    analysisText = new QTextBrowser();
    analysisLayout->addWidget(analysisText);
    
    mainLayout->addWidget(analysisGroup);
    
    // 连接信号
    connect(analyzeVoteDataBtn, &QPushButton::clicked, this, &MainWindow::onAnalyzeVoteData);
    connect(analyzeRankingBtn, &QPushButton::clicked, this, &MainWindow::onAnalyzeRanking);
    connect(analyzeDistributionBtn, &QPushButton::clicked, this, &MainWindow::onAnalyzeDistribution);
    
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

void MainWindow::onRefreshCandidateList()
{
    updateCandidateTable();
}

void MainWindow::onCandidateTableSelectionChanged()
{
    QList<QTableWidgetItem*> items = candidateTable->selectedItems();
    if (!items.isEmpty()) {
        int row = items[0]->row();
        QString id = candidateTable->item(row, 0)->text();
        candidateIdEdit->setText(id);
        onQueryCandidate();
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
    
    for (const QString &part : parts) {
        bool ok;
        int vote = part.toInt(&ok);
        if (ok && vote > 0) {
            votes.push_back(vote);
        }
    }
    
    if (votes.empty()) {
        showMessage("错误", "无效的投票向量！", true);
        return;
    }
    
    electionSystem->vote(votes);
    
    int totalVotes = votes.size();
    const vector<Candidate> &candidates = electionSystem->getAllCandidates();
    vector<int> validIDList;
    for (const auto &c : candidates) {
        validIDList.push_back(c.id);
    }
    
    int invalidCount = DataValidator::validateVoteVector(votes, validIDList);
    
    QString message = QString("批量投票完成！\n总票数: %1").arg(totalVotes);
    if (invalidCount > 0) {
        message += QString("\n无效票数: %1").arg(invalidCount);
    }
    
    showMessage("成功", message);
    updateCandidateTable();
    updateStatisticsTable();
    updateVoteHistoryList();
    statusLabel->setText(QString("已处理 %1 张选票").arg(totalVotes));
}

void MainWindow::onImportVotesFromFile()
{
    QString filename = QFileDialog::getOpenFileName(this, "选择投票文件", 
                                                    ".", "数据文件 (*.dat *.txt);;所有文件 (*.*)");
    if (filename.isEmpty()) {
        return;
    }
    
    vector<int> votes;
    if (FileManager::loadVotes(votes, filename.toStdString())) {
        electionSystem->vote(votes);
        showMessage("成功", QString("成功从文件加载 %1 张选票").arg(votes.size()));
        updateCandidateTable();
        updateStatisticsTable();
        updateVoteHistoryList();
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
        statusLabel->setText("已重置所有投票");
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
        summaryText->setPlainText("暂无数据");
        return;
    }
    
    int totalVotes = Statistics::getTotalVotes(candidates);
    double avgVotes = Statistics::getAverageVotes(candidates);
    int maxVotes = Statistics::getMaxVotes(candidates);
    int minVotes = Statistics::getMinVotes(candidates);
    
    QString summary = QString(
        "候选人总数: %1\n"
        "总票数: %2\n"
        "平均得票数: %3\n"
        "最高得票数: %4\n"
        "最低得票数: %5")
        .arg(candidates.size())
        .arg(totalVotes)
        .arg(avgVotes, 0, 'f', 2)
        .arg(maxVotes)
        .arg(minVotes);
    
    summaryText->setPlainText(summary);
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
                                                    "candidates.dat", 
                                                    "数据文件 (*.dat);;所有文件 (*.*)");
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
                                                    ".", "数据文件 (*.dat);;所有文件 (*.*)");
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
        statusLabel->setText(QString("已加载 %1 个候选人").arg(candidates.size()));
    } else {
        showMessage("错误", "加载失败！", true);
    }
}

void MainWindow::onSaveVotes()
{
    QString filename = QFileDialog::getSaveFileName(this, "保存投票数据", 
                                                    "votes.dat", 
                                                    "数据文件 (*.dat *.txt);;所有文件 (*.*)");
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
                                                    ".", "数据文件 (*.dat *.txt);;所有文件 (*.*)");
    if (filename.isEmpty()) {
        return;
    }
    
    vector<int> votes;
    if (FileManager::loadVotes(votes, filename.toStdString())) {
        electionSystem->vote(votes);
        showMessage("成功", QString("成功加载 %1 张选票").arg(votes.size()));
        maintenanceLog->append(QString("[%1] 加载投票数据: %2 (%3张选票)")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                               .arg(filename)
                               .arg(votes.size()));
        updateCandidateTable();
        updateStatisticsTable();
        updateVoteHistoryList();
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

// ==================== 辅助函数 ====================

void MainWindow::updateCandidateTable()
{
    const vector<Candidate> &candidates = electionSystem->getAllCandidates();
    
    candidateTable->setRowCount(candidates.size());
    
    for (size_t i = 0; i < candidates.size(); i++) {
        candidateTable->setItem(i, 0, new QTableWidgetItem(QString::number(candidates[i].id)));
        candidateTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(candidates[i].name)));
        candidateTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(candidates[i].department)));
        candidateTable->setItem(i, 3, new QTableWidgetItem(QString::number(candidates[i].voteCount)));
    }
    
    candidateTable->resizeColumnsToContents();
}

void MainWindow::updateStatisticsTable()
{
    updateStatisticsTable(electionSystem->getAllCandidates());
}

void MainWindow::updateStatisticsTable(const vector<Candidate> &candidates)
{
    statisticsTable->setRowCount(candidates.size());
    
    int totalVotes = Statistics::getTotalVotes(candidates);
    
    for (size_t i = 0; i < candidates.size(); i++) {
        double percentage = totalVotes > 0 ? 
            (100.0 * candidates[i].voteCount / totalVotes) : 0.0;
        
        statisticsTable->setItem(i, 0, new QTableWidgetItem(QString::number(candidates[i].id)));
        statisticsTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(candidates[i].name)));
        statisticsTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(candidates[i].department)));
        statisticsTable->setItem(i, 3, new QTableWidgetItem(QString::number(candidates[i].voteCount)));
        statisticsTable->setItem(i, 4, new QTableWidgetItem(QString::number(percentage, 'f', 2) + "%"));
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

