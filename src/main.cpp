#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include "../include/election_core.h"

using namespace std;

// 核心数据结构与算法已在 include/election_core.h 中实现

// ==================== 用户界面模块 ====================

/**
 * 用户界面类
 * 负责美观的界面显示和用户交互
 */
class UI {
private:
    ElectionSystem *system;
    
    /**
     * 显示分隔线
     */
    void printSeparator() {
        cout << "═══════════════════════════════════════════════════════════\n";
    }
    
    /**
     * 显示标题
     */
    void printTitle(const string &title) {
        cout << "\n";
        printSeparator();
        cout << "  " << title << "\n";
        printSeparator();
    }
    
    /**
     * 等待用户按键
     */
    void waitForEnter() {
        cout << "\n按回车键继续...";
        cin.clear();  // 清除可能的错误状态
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // 清除缓冲区直到换行符（包括换行符）
    }
    
    /**
     * 清屏（跨平台）
     */
    void clearScreen() {
        #ifdef _WIN32
            ::system("cls");
        #else
            ::system("clear");
        #endif
    }
    
    /**
     * 输入整数
     * @param prompt 提示信息
     * @return 输入的整数
     */
    int inputInt(const string &prompt) {
        int value;
        cout << prompt;
        while (!(cin >> value)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "❌ 输入错误，请输入一个整数：";
        }
        // 清除输入缓冲区中剩余的字符（包括换行符），确保后续输入正常
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return value;
    }
    
    /**
     * 输入字符串
     * @param prompt 提示信息
     * @return 输入的字符串
     */
    string inputString(const string &prompt) {
        string value;
        cout << prompt;
        getline(cin, value);
        return value;
    }
    
public:
    /**
     * 构造函数
     */
    UI(ElectionSystem *sys) : system(sys) {}
    
    /**
     * 显示主菜单
     */
    void showMainMenu() {
        clearScreen();
        printTitle("投票选举管理系统");
        cout << "  1. 候选人管理\n";
        cout << "  2. 投票管理\n";
        cout << "  3. 查询统计\n";
        cout << "  4. 选举结果\n";
        cout << "  5. 数据维护\n";
        cout << "  6. 高级功能\n";
        cout << "  0. 退出系统\n";
        printSeparator();
        cout << "请选择操作 [0-6]: ";
    }
    
    /**
     * 候选人管理菜单
     */
    void candidateManagementMenu() {
        while (true) {
            clearScreen();
            printTitle("候选人管理");
            cout << "  1. 添加候选人\n";
            cout << "  2. 修改候选人\n";
            cout << "  3. 删除候选人\n";
            cout << "  4. 查询候选人\n";
            cout << "  5. 显示所有候选人\n";
            cout << "  0. 返回主菜单\n";
            printSeparator();
            
            int choice = inputInt("请选择操作 [0-5]: ");
            
            switch (choice) {
                case 1: addCandidateUI(); break;
                case 2: modifyCandidateUI(); break;
                case 3: deleteCandidateUI(); break;
                case 4: queryCandidateUI(); break;
                case 5: showAllCandidates(); break;
                case 0: return;
                default: 
                    cout << "❌ 无效的选择！\n";
                    waitForEnter();
            }
        }
    }
    
    /**
     * 添加候选人界面
     */
    void addCandidateUI() {
        printTitle("添加候选人");
        int id = inputInt("请输入候选人编号: ");
        string name = inputString("请输入候选人姓名: ");
        string dept = inputString("请输入所属单位（可选，直接回车跳过）: ");
        
        system->addCandidate(id, name, dept);
        waitForEnter();
    }
    
    /**
     * 修改候选人界面
     */
    void modifyCandidateUI() {
        printTitle("修改候选人");
        int id = inputInt("请输入要修改的候选人编号: ");
        string name = inputString("请输入新姓名: ");
        string dept = inputString("请输入新所属单位（可选，直接回车跳过）: ");
        
        system->modifyCandidate(id, name, dept);
        waitForEnter();
    }
    
    /**
     * 删除候选人界面
     */
    void deleteCandidateUI() {
        printTitle("删除候选人");
        int id = inputInt("请输入要删除的候选人编号: ");
        
        cout << "⚠️  确认删除？(y/n): ";
        char confirm;
        cin >> confirm;
        cin.ignore();
        
        if (confirm == 'y' || confirm == 'Y') {
            system->deleteCandidate(id);
        } else {
            cout << "已取消删除操作。\n";
        }
        waitForEnter();
    }
    
    /**
     * 查询候选人界面
     */
    void queryCandidateUI() {
        printTitle("查询候选人");
        int id = inputInt("请输入候选人编号: ");
        
        Candidate *c = system->queryCandidate(id);
        if (c) {
            cout << "\n候选人信息：\n";
            cout << "  编号: " << c->id << "\n";
            cout << "  姓名: " << c->name << "\n";
            cout << "  所属单位: " << c->department << "\n";
            cout << "  得票数: " << c->voteCount << "\n";
        } else {
            cout << "❌ 未找到编号为 " << id << " 的候选人！\n";
        }
        waitForEnter();
    }
    
    /**
     * 显示所有候选人
     */
    void showAllCandidates() {
        printTitle("所有候选人列表");
        
        const vector<Candidate> &candidates = system->getAllCandidates();
        if (candidates.empty()) {
            cout << "暂无候选人。\n";
        } else {
            cout << left << setw(8) << "编号" 
                 << setw(20) << "姓名" 
                 << setw(20) << "所属单位" 
                 << setw(10) << "得票数" << "\n";
            cout << "────────────────────────────────────────────────────────\n";
            
            for (const auto &c : candidates) {
                cout << left << setw(8) << c.id 
                     << setw(20) << c.name 
                     << setw(20) << c.department 
                     << setw(10) << c.voteCount << "\n";
            }
        }
        waitForEnter();
    }
    
    /**
     * 投票管理菜单
     */
    void voteManagementMenu() {
        while (true) {
            clearScreen();
            printTitle("投票管理");
            cout << "  1. 单票投票\n";
            cout << "  2. 批量投票（输入投票向量）\n";
            cout << "  3. 从文件导入投票\n";
            cout << "  4. 查看投票历史\n";
            cout << "  5. 重置投票\n";
            cout << "  0. 返回主菜单\n";
            printSeparator();
            
            int choice = inputInt("请选择操作 [0-5]: ");
            
            switch (choice) {
                case 1: singleVoteUI(); break;
                case 2: batchVoteUI(); break;
                case 3: importVotesFromFileUI(); break;
                case 4: showVoteHistoryUI(); break;
                case 5: 
                    system->resetVotes();
                    waitForEnter();
                    break;
                case 0: return;
                default: 
                    cout << "❌ 无效的选择！\n";
                    waitForEnter();
            }
        }
    }
    
    /**
     * 单票投票界面
     */
    void singleVoteUI() {
        printTitle("单票投票");
        int id = inputInt("请输入候选人编号: ");
        system->castVote(id);
        waitForEnter();
    }
    
    /**
     * 批量投票界面
     */
    void batchVoteUI() {
        printTitle("批量投票（输入投票向量）");
        cout << "请输入投票向量（用空格分隔的候选人编号，输入-1结束）:\n";
        cout << "例如: 1 2 1 3 1 1 -1\n";
        
        vector<int> votes;
        int vote;
        while (cin >> vote) {
            if (vote == -1) break;
            votes.push_back(vote);
        }
        
        // 清除输入流的错误状态和剩余字符
        if (cin.fail() && !cin.eof()) {
            cin.clear();
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        if (!votes.empty()) {
            system->vote(votes, false); // 批量投票：在原有票数基础上累加
        } else {
            cout << "❌ 投票向量为空！\n";
        }
        waitForEnter();
    }
    
    /**
     * 从文件导入投票界面
     */
    void importVotesFromFileUI() {
        printTitle("从文件导入投票");
        string filename = inputString("请输入文件名（默认: votes.csv）: ");
        if (filename.empty()) filename = "votes.csv";
        
        vector<int> votes;
        if (FileManager::loadVotes(votes, filename)) {
            cout << "✅ 成功从文件加载 " << votes.size() << " 张选票。\n";
            system->vote(votes, false); // 从文件导入视为一次批量累加投票
        } else {
            cout << "❌ 文件加载失败！\n";
        }
        waitForEnter();
    }
    
    /**
     * 显示投票历史界面
     */
    void showVoteHistoryUI() {
        printTitle("投票历史");
        
        const vector<int> &history = system->getVoteHistory();
        if (history.empty()) {
            cout << "暂无投票记录。\n";
        } else {
            cout << "投票向量: ";
            for (size_t i = 0; i < history.size(); i++) {
                cout << history[i];
                if (i < history.size() - 1) cout << " ";
            }
            cout << "\n";
            cout << "总票数: " << history.size() << "\n";
        }
        waitForEnter();
    }
    
    /**
     * 查询统计菜单
     */
    void statisticsMenu() {
        while (true) {
            clearScreen();
            printTitle("查询统计");
            cout << "  1. 显示得票统计\n";
            cout << "  2. 按得票数排序\n";
            cout << "  3. 按编号排序\n";
            cout << "  4. 按姓名排序\n";
            cout << "  5. 统计摘要\n";
            cout << "  0. 返回主菜单\n";
            printSeparator();
            
            int choice = inputInt("请选择操作 [0-5]: ");
            
            switch (choice) {
                case 1: showVoteStatistics(); break;
                case 2: sortByVotes(); break;
                case 3: sortByID(); break;
                case 4: sortByName(); break;
                case 5: showStatisticsSummary(); break;
                case 0: return;
                default: 
                    cout << "❌ 无效的选择！\n";
                    waitForEnter();
            }
        }
    }
    
    /**
     * 显示得票统计
     */
    void showVoteStatistics() {
        printTitle("得票统计");
        
        const vector<Candidate> &candidates = system->getAllCandidates();
        if (candidates.empty()) {
            cout << "暂无候选人。\n";
        } else {
            int totalVotes = Statistics::getTotalVotes(candidates);
            
            cout << left << setw(8) << "编号" 
                 << setw(20) << "姓名" 
                 << setw(20) << "所属单位" 
                 << setw(10) << "得票数" 
                 << setw(15) << "得票率" << "\n";
            cout << "────────────────────────────────────────────────────────────────────\n";
            
            vector<Candidate> sorted = candidates;
            Statistics::sortByVotes(sorted, false);
            
            for (const auto &c : sorted) {
                double percentage = totalVotes > 0 ? 
                    (100.0 * c.voteCount / totalVotes) : 0.0;
                cout << left << setw(8) << c.id 
                     << setw(20) << c.name 
                     << setw(20) << c.department 
                     << setw(10) << c.voteCount 
                     << fixed << setprecision(2) << setw(15) << percentage << "%\n";
            }
            
            cout << "\n总票数: " << totalVotes << "\n";
        }
        waitForEnter();
    }
    
    /**
     * 按得票数排序
     */
    void sortByVotes() {
        printTitle("按得票数排序");
        cout << "排序方式：\n";
        cout << "  1. 降序（得票多在前）\n";
        cout << "  2. 升序（得票少在前）\n";
        
        int choice = inputInt("请选择 [1-2]: ");
        
        vector<Candidate> candidates = system->getAllCandidates();
        Statistics::sortByVotes(candidates, choice == 2);
        
        cout << "\n排序结果：\n";
        cout << left << setw(8) << "编号" 
             << setw(20) << "姓名" 
             << setw(20) << "所属单位" 
             << setw(10) << "得票数" << "\n";
        cout << "────────────────────────────────────────────────────────\n";
        
        for (const auto &c : candidates) {
            cout << left << setw(8) << c.id 
                 << setw(20) << c.name 
                 << setw(20) << c.department 
                 << setw(10) << c.voteCount << "\n";
        }
        waitForEnter();
    }
    
    /**
     * 按编号排序
     */
    void sortByID() {
        printTitle("按编号排序");
        
        vector<Candidate> candidates = system->getAllCandidates();
        Statistics::sortByID(candidates);
        
        cout << "\n排序结果：\n";
        cout << left << setw(8) << "编号" 
             << setw(20) << "姓名" 
             << setw(20) << "所属单位" 
             << setw(10) << "得票数" << "\n";
        cout << "────────────────────────────────────────────────────────\n";
        
        for (const auto &c : candidates) {
            cout << left << setw(8) << c.id 
                 << setw(20) << c.name 
                 << setw(20) << c.department 
                 << setw(10) << c.voteCount << "\n";
        }
        waitForEnter();
    }
    
    /**
     * 按姓名排序
     */
    void sortByName() {
        printTitle("按姓名排序");
        
        vector<Candidate> candidates = system->getAllCandidates();
        Statistics::sortByName(candidates);
        
        cout << "\n排序结果：\n";
        cout << left << setw(8) << "编号" 
             << setw(20) << "姓名" 
             << setw(20) << "所属单位" 
             << setw(10) << "得票数" << "\n";
        cout << "────────────────────────────────────────────────────────\n";
        
        for (const auto &c : candidates) {
            cout << left << setw(8) << c.id 
                 << setw(20) << c.name 
                 << setw(20) << c.department 
                 << setw(10) << c.voteCount << "\n";
        }
        waitForEnter();
    }
    
    /**
     * 显示统计摘要
     */
    void showStatisticsSummary() {
        printTitle("统计摘要");
        
        const vector<Candidate> &candidates = system->getAllCandidates();
        if (candidates.empty()) {
            cout << "暂无候选人。\n";
        } else {
            int totalVotes = Statistics::getTotalVotes(candidates);
            double avgVotes = Statistics::getAverageVotes(candidates);
            int maxVotes = Statistics::getMaxVotes(candidates);
            int minVotes = Statistics::getMinVotes(candidates);
            
            cout << "候选人总数: " << candidates.size() << "\n";
            cout << "总票数: " << totalVotes << "\n";
            cout << "平均得票数: " << fixed << setprecision(2) << avgVotes << "\n";
            cout << "最高得票数: " << maxVotes << "\n";
            cout << "最低得票数: " << minVotes << "\n";
        }
        waitForEnter();
    }
    
    /**
     * 显示选举结果
     */
    void showElectionResult() {
        printTitle("选举结果");
        
        const vector<Candidate> &candidates = system->getAllCandidates();
        if (candidates.empty()) {
            cout << "暂无候选人。\n";
        } else {
            int winnerID = system->findWinner();
            int totalVotes = Statistics::getTotalVotes(candidates);
            
            if (winnerID != -1) {
                Candidate *winner = system->queryCandidate(winnerID);
                cout << "🎉 优胜者：\n";
                cout << "  编号: " << winner->id << "\n";
                cout << "  姓名: " << winner->name << "\n";
                cout << "  所属单位: " << winner->department << "\n";
                cout << "  得票数: " << winner->voteCount << "\n";
                double percentage = totalVotes > 0 ? 
                    (100.0 * winner->voteCount / totalVotes) : 0.0;
                cout << "  得票率: " << fixed << setprecision(2) << percentage << "%\n";
                cout << "\n✅ 该候选人获得超过半数选票！\n";
            } else {
                cout << "❌ 没有候选人获得超过半数选票！\n";
                cout << "\n所有候选人得票情况：\n";
                showVoteStatistics();
            }
        }
        waitForEnter();
    }
    
    /**
     * 数据维护菜单
     */
    void dataMaintenanceMenu() {
        while (true) {
            clearScreen();
            printTitle("数据维护");
            cout << "  1. 保存候选人数据\n";
            cout << "  2. 加载候选人数据\n";
            cout << "  3. 保存投票数据\n";
            cout << "  4. 加载投票数据\n";
            cout << "  5. 导出统计报告\n";
            cout << "  6. 清空所有数据\n";
            cout << "  0. 返回主菜单\n";
            printSeparator();
            
            int choice = inputInt("请选择操作 [0-6]: ");
            
            switch (choice) {
                case 1: {
                    string filename = inputString("请输入保存文件名（默认: candidates.csv）: ");
                    if (filename.empty()) filename = "candidates.csv";
                    if (FileManager::saveCandidates(system->getAllCandidates(), filename)) {
                        cout << "✅ 候选人数据保存成功！文件: " << filename << "\n";
                    } else {
                        cout << "❌ 保存失败！\n";
                    }
                    waitForEnter();
                    break;
                }
                case 2: {
                    string filename = inputString("请输入加载文件名（默认: candidates.csv）: ");
                    if (filename.empty()) filename = "candidates.csv";
                    vector<Candidate> candidates;
                    if (FileManager::loadCandidates(candidates, filename)) {
                        // 使用加载的数据重建系统中的候选人与票数
                        system->clearAll();
                        for (const auto &c : candidates) {
                            system->addCandidate(c.id, c.name, c.department);
                            Candidate *loaded = system->queryCandidate(c.id);
                            if (loaded) {
                                loaded->voteCount = c.voteCount;
                            }
                        }
                        cout << "✅ 候选人数据加载成功！文件: " << filename
                             << "（" << candidates.size() << " 个候选人）\n";
                    } else {
                        cout << "❌ 加载失败！\n";
                    }
                    waitForEnter();
                    break;
                }
                case 3: {
                    string filename = inputString("请输入保存文件名（默认: votes.csv）: ");
                    if (filename.empty()) filename = "votes.csv";
                    if (FileManager::saveVotes(system->getVoteHistory(), filename)) {
                        cout << "✅ 投票数据保存成功！文件: " << filename << "\n";
                    } else {
                        cout << "❌ 保存失败！\n";
                    }
                    waitForEnter();
                    break;
                }
                case 4: {
                    string filename = inputString("请输入加载文件名（默认: votes.csv）: ");
                    if (filename.empty()) filename = "votes.csv";
                    vector<int> votes;
                    if (FileManager::loadVotes(votes, filename)) {
                        system->resetVotes();      // 数据维护中的“加载投票数据”先清零
                        system->vote(votes, true); // 再重建
                        cout << "✅ 投票数据加载成功！文件: " << filename
                             << "（" << votes.size() << " 张选票）\n";
                    } else {
                        cout << "❌ 加载失败！\n";
                    }
                    waitForEnter();
                    break;
                }
                case 5: {
                    string filename = inputString("请输入报告文件名（默认: election_report.txt）: ");
                    if (filename.empty()) filename = "election_report.txt";
                    int winnerID = system->findWinner();
                    if (FileManager::exportReport(system->getAllCandidates(), winnerID, filename)) {
                        cout << "✅ 统计报告导出成功！文件名: " << filename << "\n";
                    } else {
                        cout << "❌ 导出失败！\n";
                    }
                    waitForEnter();
                    break;
                }
                case 6:
                    cout << "⚠️  确认清空所有数据？(y/n): ";
                    char confirm;
                    cin >> confirm;
                    cin.ignore();
                    if (confirm == 'y' || confirm == 'Y') {
                        system->clearAll();
                    } else {
                        cout << "已取消操作。\n";
                    }
                    waitForEnter();
                    break;
                case 0: return;
                default: 
                    cout << "❌ 无效的选择！\n";
                    waitForEnter();
            }
        }
    }
    
    /**
     * 高级功能菜单
     */
    void advancedFeaturesMenu() {
        while (true) {
            clearScreen();
            printTitle("高级功能");
            cout << "  1. 投票数据分析\n";
            cout << "  2. 候选人排名分析\n";
            cout << "  3. 得票分布分析\n";
            cout << "  4. 性能测试\n";
            cout << "  0. 返回主菜单\n";
            printSeparator();
            
            int choice = inputInt("请选择操作 [0-4]: ");
            
            switch (choice) {
                case 1: analyzeVoteData(); break;
                case 2: analyzeRanking(); break;
                case 3: analyzeDistribution(); break;
                case 4: analyzePerformance(); break;
                case 0: return;
                default: 
                    cout << "❌ 无效的选择！\n";
                    waitForEnter();
            }
        }
    }
    
    /**
     * 投票数据分析
     */
    void analyzeVoteData() {
        printTitle("投票数据分析");
        
        const vector<int> &history = system->getVoteHistory();
        if (history.empty()) {
            cout << "暂无投票数据。\n";
        } else {
            map<int, int> voteCount; // 统计每个候选人的投票次数
            for (int vote : history) {
                voteCount[vote]++;
            }
            
            cout << "投票分布：\n";
            for (const auto &pair : voteCount) {
                Candidate *c = system->queryCandidate(pair.first);
                if (c) {
                    cout << "  编号 " << pair.first << " (" << c->name 
                         << "): " << pair.second << " 票\n";
                }
            }
            
            cout << "\n投票趋势：\n";
            cout << "  前10张选票: ";
            for (size_t i = 0; i < min(10UL, history.size()); i++) {
                cout << history[i] << " ";
            }
            cout << "\n";
        }
        waitForEnter();
    }
    
    /**
     * 候选人排名分析
     */
    void analyzeRanking() {
        printTitle("候选人排名分析");
        
        const vector<Candidate> &candidates = system->getAllCandidates();
        if (candidates.empty()) {
            cout << "暂无候选人。\n";
        } else {
            vector<Candidate> sorted = candidates;
            Statistics::sortByVotes(sorted, false);
            
            cout << "排名\t编号\t姓名\t\t得票数\n";
            cout << "────────────────────────────────────\n";
            for (size_t i = 0; i < sorted.size(); i++) {
                cout << (i + 1) << "\t" << sorted[i].id << "\t" 
                     << sorted[i].name << "\t\t" << sorted[i].voteCount << "\n";
            }
        }
        waitForEnter();
    }
    
    /**
     * 得票分布分析
     */
    void analyzeDistribution() {
        printTitle("得票分布分析");
        
        const vector<Candidate> &candidates = system->getAllCandidates();
        if (candidates.empty()) {
            cout << "暂无候选人。\n";
        } else {
            int maxVotes = Statistics::getMaxVotes(candidates);
            
            cout << "得票分布（可视化）：\n\n";
            
            for (const auto &c : candidates) {
                int barLength = maxVotes > 0 ? (50 * c.voteCount / maxVotes) : 0;
                cout << left << setw(20) << c.name << " [";
                for (int i = 0; i < barLength; i++) {
                    cout << "█";
                }
                for (int i = barLength; i < 50; i++) {
                    cout << " ";
                }
                cout << "] " << c.voteCount << " 票\n";
            }
        }
        waitForEnter();
    }
    
    /**
     * 性能测试
     * 在不同规模下测量批量投票与查找优胜者的耗时
     */
    void analyzePerformance() {
        printTitle("性能测试");
        
        struct CaseConfig {
            int candidates;
            int votes;
        };
        
        const CaseConfig cases[] = {
            {10,    100},
            {100,   10000},
            {1000,  100000}
        };
        
        cout << "理论复杂度：\n";
        cout << "  批量投票：O(m)，m 为选票数量\n";
        cout << "  查找优胜者：O(n)，n 为候选人数\n";
        cout << "  排序：O(n log n)\n\n";
        
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
            
            using clock = std::chrono::steady_clock;
            auto startVote = clock::now();
            perfSystem.vote(votes, true);
            auto endVote = clock::now();
            auto voteMs = std::chrono::duration_cast<std::chrono::milliseconds>(endVote - startVote).count();
            
            auto startFind = clock::now();
            int winner = perfSystem.findWinner();
            (void)winner;
            auto endFind = clock::now();
            auto findMs = std::chrono::duration_cast<std::chrono::milliseconds>(endFind - startFind).count();
            
            cout << "场景： " << cfg.candidates << " 个候选人，"
                 << cfg.votes << " 张选票\n";
            cout << "  批量投票耗时： " << voteMs << " ms\n";
            cout << "  查找优胜者耗时： " << findMs << " ms\n\n";
        }
        
        waitForEnter();
    }
    
    /**
     * 运行主程序
     */
    void run() {
        while (true) {
            showMainMenu();
            int choice = inputInt("");
            
            switch (choice) {
                case 1: candidateManagementMenu(); break;
                case 2: voteManagementMenu(); break;
                case 3: statisticsMenu(); break;
                case 4: 
                    clearScreen();
                    showElectionResult(); 
                    break;
                case 5: dataMaintenanceMenu(); break;
                case 6: advancedFeaturesMenu(); break;
                case 0: 
                    cout << "\n感谢使用投票选举管理系统！再见！\n";
                    return;
                default: 
                    cout << "❌ 无效的选择！\n";
                    waitForEnter();
            }
        }
    }
};

// ==================== 测试用例 ====================

/**
 * 运行测试用例
 */
void runTestCases() {
    cout << "\n═══════════════════════════════════════════════════════════\n";
    cout << "  测试用例执行\n";
    cout << "═══════════════════════════════════════════════════════════\n\n";
    
    ElectionSystem system;
    
    // 测试用例1：基本功能测试
    cout << "【测试用例1】基本功能测试\n";
    cout << "───────────────────────────────────────────────────────────\n";
    system.addCandidate(1, "张三", "计算机学院");
    system.addCandidate(2, "李四", "数学学院");
    system.addCandidate(3, "王五", "物理学院");
    
    vector<int> votes1 = {1, 2, 1, 3, 1, 1, 1, 2, 1, 1}; // 1号得7票，超过半数
    system.vote(votes1, true);
    
    int winner1 = system.findWinner();
    cout << "投票向量: ";
    for (int v : votes1) cout << v << " ";
    cout << "\n优胜者ID: " << winner1 << "\n";
    cout << "预期结果: 1\n";
    cout << (winner1 == 1 ? "✅ 测试通过\n" : "❌ 测试失败\n");
    cout << "\n";
    
    // 测试用例2：没有超过半数的候选人
    cout << "【测试用例2】没有超过半数的候选人\n";
    cout << "───────────────────────────────────────────────────────────\n";
    system.resetVotes();
    vector<int> votes2 = {1, 2, 3, 1, 2, 3}; // 每人2票，没有超过半数
    system.vote(votes2, true);
    
    int winner2 = system.findWinner();
    cout << "投票向量: ";
    for (int v : votes2) cout << v << " ";
    cout << "\n优胜者ID: " << winner2 << "\n";
    cout << "预期结果: -1（无优胜者）\n";
    cout << (winner2 == -1 ? "✅ 测试通过\n" : "❌ 测试失败\n");
    cout << "\n";
    
    // 测试用例3：数据验证测试
    cout << "【测试用例3】数据验证测试\n";
    cout << "───────────────────────────────────────────────────────────\n";
    bool test3_1 = system.addCandidate(-1, "测试"); // 无效ID
    bool test3_2 = system.addCandidate(1, ""); // 空姓名
    bool test3_3 = system.addCandidate(1, "重复"); // 重复ID
    cout << "无效ID测试: " << (test3_1 ? "❌ 失败" : "✅ 通过") << "\n";
    cout << "空姓名测试: " << (test3_2 ? "❌ 失败" : "✅ 通过") << "\n";
    cout << "重复ID测试: " << (test3_3 ? "❌ 失败" : "✅ 通过") << "\n";
    cout << "\n";
    
    // 测试用例4：无效投票处理
    cout << "【测试用例4】无效投票处理\n";
    cout << "───────────────────────────────────────────────────────────\n";
    system.resetVotes();
    vector<int> votes4 = {1, 2, 99, 1, 3, 88, 1}; // 包含无效ID
    system.vote(votes4, true);
    cout << "投票向量包含无效ID: 99, 88\n";
    cout << "系统应能识别并忽略无效投票\n";
    cout << "\n";
    
    cout << "═══════════════════════════════════════════════════════════\n";
    cout << "  测试用例执行完成\n";
    cout << "═══════════════════════════════════════════════════════════\n\n";
}

// ==================== 主函数 ====================

int main() {
    // 设置输出格式
    cout << fixed << setprecision(2);
    
    cout << "\n";
    cout << "╔═══════════════════════════════════════════════════════════╗\n";
    cout << "║                                                           ║\n";
    cout << "║           投票选举管理系统 v1.0                           ║\n";
    cout << "║                                                           ║\n";
    cout << "║   基于STL的投票选举问题解决方案                           ║\n";
    cout << "║                                                           ║\n";
    cout << "╚═══════════════════════════════════════════════════════════╝\n";
    
    cout << "\n请选择模式：\n";
    cout << "  1. 交互式界面\n";
    cout << "  2. 运行测试用例\n";
    cout << "请选择 [1-2]: ";
    
    int mode;
    cin >> mode;
    cin.ignore();
    
    if (mode == 2) {
        runTestCases();
        cout << "按回车键退出...";
        cin.get();
        return 0;
    }
    
    // 交互式模式
    ElectionSystem system;
    UI ui(&system);
    
    // 可以在这里加载初始数据
    // FileManager::loadCandidates(...);
    
    ui.run();
    
    return 0;
}
