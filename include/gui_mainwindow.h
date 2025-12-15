#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QKeySequence>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QGroupBox>
#include <QComboBox>
#include <QTabWidget>
#include <QSplitter>
#include <QListWidget>
#include <QSpinBox>
#include <QTextBrowser>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "election_core.h"  // 在include目录中，直接引用

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMenuBar;
class QToolBar;
QT_END_NAMESPACE

/**
 * 主窗口类
 * 提供完整的GUI界面用于投票选举管理
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 视图与字体
    void onIncreaseFont();
    void onDecreaseFont();
    void onResetFont();
    
    // 候选人管理
    void onAddCandidate();
    void onModifyCandidate();
    void onDeleteCandidate();
    void onQueryCandidate();
    
    // 投票管理
    void onSingleVote();
    void onBatchVote();
    void onImportVotesFromFile();
    void onResetVotes();
    void onShowVoteHistory();
    void onUndoLastVote();
    void onUndoMultipleVotes();
    
    // 查询统计
    void onShowStatistics();
    void onSortCandidates();
    void onShowSummary();
    
    // 选举结果
    void onShowElectionResult();
    void onExportReport();
    
    // 数据维护
    void onSaveCandidates();
    void onLoadCandidates();
    void onSaveVotes();
    void onLoadVotes();
    void onClearAll();
    void onLoadSampleCandidates();
    
    // 高级功能
    void onAnalyzeVoteData();
    void onAnalyzeRanking();
    void onAnalyzeDistribution();
    void onAnalyzePerformance();
    
    // 表格选择变化
    void onCandidateTableSelectionChanged();
    
    // 更新图表
    void updateCharts();

private:
    // 界面初始化
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createCentralWidget();
    void createCandidateManagementWidget();
    void createVoteManagementWidget();
    void createStatisticsWidget();
    void createElectionResultWidget();
    void createDataMaintenanceWidget();
    void createAdvancedFeaturesWidget();
    
    // 辅助函数
    void updateCandidateTable();
    void updateStatisticsTable();
    void updateStatisticsTable(const std::vector<Candidate> &candidates);
    void updateVoteHistoryList();
    void showMessage(const QString &title, const QString &message, bool isError = false);
    bool validateInput(const QString &text, bool isID = false);
    void clearInputFields();
    void applyGlobalStyle();
    void applyFontScale();
    
    // 核心系统
    ElectionSystem *electionSystem;
    
    // 主界面组件
    QTabWidget *mainTabWidget;
    
    // 候选人管理界面
    QWidget *candidateWidget;
    QTableWidget *candidateTable;
    QLabel *candidateEmptyLabel;
    QLineEdit *candidateIdEdit;
    QLineEdit *candidateNameEdit;
    QLineEdit *candidateDeptEdit;
    QPushButton *addCandidateBtn;
    QPushButton *modifyCandidateBtn;
    QPushButton *deleteCandidateBtn;
    QPushButton *queryCandidateBtn;
    QPushButton *undoLastVoteBtn;
    QSpinBox *undoCountSpin;
    QPushButton *undoMultipleVotesBtn;
    
    // 投票管理界面
    QWidget *voteWidget;
    QSpinBox *voteCandidateIdSpin;
    QTextEdit *batchVoteEdit;
    QPushButton *singleVoteBtn;
    QPushButton *batchVoteBtn;
    QPushButton *importVotesBtn;
    QPushButton *resetVotesBtn;
    QListWidget *voteHistoryList;
    
    // 统计界面
    QWidget *statisticsWidget;
    QTableWidget *statisticsTable;
    QComboBox *sortComboBox;
    QPushButton *sortBtn;
    QTextBrowser *summaryText;
    
    // 选举结果界面
    QWidget *resultWidget;
    QTextBrowser *resultText;
    QPushButton *exportReportBtn;
    
    // 数据维护界面
    QWidget *maintenanceWidget;
    QPushButton *saveCandidatesBtn;
    QPushButton *loadCandidatesBtn;
    QPushButton *saveVotesBtn;
    QPushButton *loadVotesBtn;
    QPushButton *clearAllBtn;
    QPushButton *loadSampleCandidatesBtn;
    QTextBrowser *maintenanceLog;
    
    // 高级功能界面
    QWidget *advancedWidget;
    QTextBrowser *analysisText;
    QPushButton *analyzeVoteDataBtn;
    QPushButton *analyzeRankingBtn;
    QPushButton *analyzeDistributionBtn;
    QPushButton *analyzePerformanceBtn;
    
    // 菜单和工具栏
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *helpMenu;
    QToolBar *mainToolBar;
    
    // 状态栏
    QLabel *statusLabel;
    QPushButton *fontDownBtn;
    QPushButton *fontResetBtn;
    QPushButton *fontUpBtn;
    
    // 字体缩放
    int baseFontPointSize;
    int currentFontDelta;
};

#endif // MAINWINDOW_H

