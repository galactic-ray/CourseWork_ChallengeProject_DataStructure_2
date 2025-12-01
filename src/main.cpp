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
#include "../include/election_core.h"

using namespace std;

/**
 * 候选人数据结构
 */
struct Candidate {
    int id;              // 候选人编号
    string name;         // 候选人姓名
    string department;  // 所属单位（扩展功能）
    int voteCount;       // 得票数
    
    Candidate() : id(0), name(""), department(""), voteCount(0) {}
    Candidate(int i, const string &n, const string &d = "") 
        : id(i), name(n), department(d), voteCount(0) {}
    
    bool operator<(const Candidate &other) const {
        return voteCount > other.voteCount; // 按票数降序
    }
};

// ==================== 数据验证模块 ====================

/**
 * 数据验证类
 * 负责验证输入数据的合法性
 */
class DataValidator {
public:
    /**
     * 验证候选人ID是否合法
     * @param id 候选人ID
     * @return true表示合法，false表示非法
     */
    static bool validateCandidateID(int id) {
        return id > 0; // ID必须为正整数
    }
    
    /**
     * 验证候选人姓名是否合法
     * @param name 候选人姓名
     * @return true表示合法，false表示非法
     */
    static bool validateName(const string &name) {
        if (name.empty() || name.length() > 50) {
            return false;
        }
        // 检查是否包含非法字符
        // 允许：字母、数字、空格、下划线、连字符、中文字符
        for (unsigned char c : name) {
            // 允许ASCII字母数字和常用符号
            if (isalnum(c) || c == ' ' || c == '_' || c == '-') {
                continue;
            }
            // 允许中文字符（UTF-8编码，首字节范围0x80-0xFF）
            // 对于UTF-8，中文字符的首字节通常在0xE0-0xEF范围
            // 这里简化处理：允许所有非ASCII字符（可能是中文）
            if (static_cast<unsigned char>(c) >= 0x80) {
                continue; // 可能是中文字符，允许
            }
            // 其他字符视为非法
            return false;
        }
        return true;
    }
    
    /**
     * 验证投票向量中的ID是否在有效范围内
     * @param voteID 投票ID
     * @param validIDs 有效的候选人ID集合
     * @return true表示合法，false表示非法
     */
    static bool validateVoteID(int voteID, const vector<int> &validIDs) {
        return find(validIDs.begin(), validIDs.end(), voteID) != validIDs.end();
    }
    
    /**
     * 验证投票向量是否合法
     * @param votes 投票向量
     * @param validIDs 有效的候选人ID集合
     * @return 无效投票的数量
     */
    static int validateVoteVector(const vector<int> &votes, 
                                   const vector<int> &validIDs) {
        int invalidCount = 0;
        for (int vote : votes) {
            if (!validateVoteID(vote, validIDs)) {
                invalidCount++;
            }
        }
        return invalidCount;
    }
};

// ==================== 文件管理模块 ====================

/**
 * 文件管理类
 * 负责数据的保存和加载
 */
class FileManager {
public:
    /**
     * 保存候选人数据到文件
     * @param candidates 候选人列表
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    static bool saveCandidates(const vector<Candidate> &candidates, 
                              const string &filename = "candidates.dat") {
        ofstream file(filename, ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        size_t count = candidates.size();
        file.write(reinterpret_cast<const char*>(&count), sizeof(count));
        
        for (const auto &c : candidates) {
            size_t nameLen = c.name.length();
            size_t deptLen = c.department.length();
            
            file.write(reinterpret_cast<const char*>(&c.id), sizeof(c.id));
            file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            file.write(c.name.c_str(), nameLen);
            file.write(reinterpret_cast<const char*>(&deptLen), sizeof(deptLen));
            file.write(c.department.c_str(), deptLen);
            file.write(reinterpret_cast<const char*>(&c.voteCount), sizeof(c.voteCount));
        }
        
        file.close();
        return true;
    }
    
    /**
     * 从文件加载候选人数据
     * @param candidates 候选人列表（输出参数）
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    static bool loadCandidates(vector<Candidate> &candidates, 
                              const string &filename = "candidates.dat") {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        candidates.clear();
        size_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        
        for (size_t i = 0; i < count; i++) {
            Candidate c;
            size_t nameLen, deptLen;
            
            file.read(reinterpret_cast<char*>(&c.id), sizeof(c.id));
            file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            c.name.resize(nameLen);
            file.read(&c.name[0], nameLen);
            file.read(reinterpret_cast<char*>(&deptLen), sizeof(deptLen));
            c.department.resize(deptLen);
            file.read(&c.department[0], deptLen);
            file.read(reinterpret_cast<char*>(&c.voteCount), sizeof(c.voteCount));
            
            candidates.push_back(c);
        }
        
        file.close();
        return true;
    }
    
    /**
     * 保存投票向量到文件
     * @param votes 投票向量
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    static bool saveVotes(const vector<int> &votes, 
                          const string &filename = "votes.dat") {
        ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        for (size_t i = 0; i < votes.size(); i++) {
            file << votes[i];
            if (i < votes.size() - 1) file << " ";
        }
        file << endl;
        
        file.close();
        return true;
    }
    
    /**
     * 从文件加载投票向量
     * @param votes 投票向量（输出参数）
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    static bool loadVotes(vector<int> &votes, 
                          const string &filename = "votes.dat") {
        ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        votes.clear();
        int vote;
        while (file >> vote) {
            votes.push_back(vote);
        }
        
        file.close();
        return true;
    }
    
    /**
     * 导出统计报告到文本文件
     * @param candidates 候选人列表
     * @param winnerID 优胜者ID
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    static bool exportReport(const vector<Candidate> &candidates, 
                             int winnerID, 
                             const string &filename = "election_report.txt") {
        ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        time_t now = time(0);
        file << "========================================\n";
        file << "      投票选举统计报告\n";
        file << "========================================\n";
        file << "生成时间: " << ctime(&now);
        file << "----------------------------------------\n\n";
        
        int totalVotes = 0;
        for (const auto &c : candidates) {
            totalVotes += c.voteCount;
        }
        
        file << "总票数: " << totalVotes << "\n";
        file << "候选人总数: " << candidates.size() << "\n\n";
        
        file << "候选人得票情况:\n";
        file << "----------------------------------------\n";
        file << left << setw(8) << "编号" 
             << setw(20) << "姓名" 
             << setw(20) << "所属单位" 
             << setw(10) << "得票数" 
             << setw(15) << "得票率" << "\n";
        file << "----------------------------------------\n";
        
        vector<Candidate> sorted = candidates;
        sort(sorted.begin(), sorted.end());
        
        for (const auto &c : sorted) {
            double percentage = totalVotes > 0 ? 
                (100.0 * c.voteCount / totalVotes) : 0.0;
            file << left << setw(8) << c.id 
                 << setw(20) << c.name 
                 << setw(20) << c.department 
                 << setw(10) << c.voteCount 
                 << fixed << setprecision(2) << setw(15) << percentage << "%\n";
        }
        
        file << "\n----------------------------------------\n";
        if (winnerID != -1) {
            file << "优胜者: 编号 " << winnerID << "\n";
            for (const auto &c : candidates) {
                if (c.id == winnerID) {
                    file << "姓名: " << c.name << "\n";
                    file << "所属单位: " << c.department << "\n";
                    file << "得票数: " << c.voteCount << "\n";
                    double percentage = totalVotes > 0 ? 
                        (100.0 * c.voteCount / totalVotes) : 0.0;
                    file << "得票率: " << fixed << setprecision(2) 
                         << percentage << "%\n";
                    break;
                }
            }
        } else {
            file << "没有候选人获得超过半数票！\n";
        }
        file << "========================================\n";
        
        file.close();
        return true;
    }
};

// ==================== 统计模块 ====================

/**
 * 统计类
 * 负责各种统计功能
 */
class Statistics {
public:
    /**
     * 计算总票数
     * @param candidates 候选人列表
     * @return 总票数
     */
    static int getTotalVotes(const vector<Candidate> &candidates) {
        int total = 0;
        for (const auto &c : candidates) {
            total += c.voteCount;
        }
        return total;
    }
    
    /**
     * 计算平均得票数
     * @param candidates 候选人列表
     * @return 平均得票数
     */
    static double getAverageVotes(const vector<Candidate> &candidates) {
        if (candidates.empty()) return 0.0;
        return static_cast<double>(getTotalVotes(candidates)) / candidates.size();
    }
    
    /**
     * 获取最高得票数
     * @param candidates 候选人列表
     * @return 最高得票数
     */
    static int getMaxVotes(const vector<Candidate> &candidates) {
        if (candidates.empty()) return 0;
        int maxVotes = candidates[0].voteCount;
        for (const auto &c : candidates) {
            if (c.voteCount > maxVotes) {
                maxVotes = c.voteCount;
            }
        }
        return maxVotes;
    }
    
    /**
     * 获取最低得票数
     * @param candidates 候选人列表
     * @return 最低得票数
     */
    static int getMinVotes(const vector<Candidate> &candidates) {
        if (candidates.empty()) return 0;
        int minVotes = candidates[0].voteCount;
        for (const auto &c : candidates) {
            if (c.voteCount < minVotes) {
                minVotes = c.voteCount;
            }
        }
        return minVotes;
    }
    
    /**
     * 按得票数排序候选人
     * @param candidates 候选人列表（会被修改）
     * @param ascending true表示升序，false表示降序
     */
    static void sortByVotes(vector<Candidate> &candidates, bool ascending = false) {
        if (ascending) {
            sort(candidates.begin(), candidates.end(), 
                 [](const Candidate &a, const Candidate &b) {
                     return a.voteCount < b.voteCount;
                 });
        } else {
            sort(candidates.begin(), candidates.end());
        }
    }
    
    /**
     * 按ID排序候选人
     * @param candidates 候选人列表（会被修改）
     */
    static void sortByID(vector<Candidate> &candidates) {
        sort(candidates.begin(), candidates.end(), 
             [](const Candidate &a, const Candidate &b) {
                 return a.id < b.id;
             });
    }
    
    /**
     * 按姓名排序候选人
     * @param candidates 候选人列表（会被修改）
     */
    static void sortByName(vector<Candidate> &candidates) {
        sort(candidates.begin(), candidates.end(), 
             [](const Candidate &a, const Candidate &b) {
                 return a.name < b.name;
             });
    }
};

// ==================== 核心选举系统 ====================

/**
 * 选举系统核心类
 * 使用STL容器实现投票选举功能
 */
class ElectionSystem {
private:
    vector<Candidate> candidates;           // 候选人列表（使用STL vector）
    unordered_map<int, int> idToIndex;      // ID到索引的映射（使用STL unordered_map）
    vector<int> voteHistory;                // 投票历史记录（使用STL vector）
    
    /**
     * 更新ID到索引的映射
     */
    void updateIndexMap() {
        idToIndex.clear();
        for (size_t i = 0; i < candidates.size(); i++) {
            idToIndex[candidates[i].id] = i;
        }
    }
    
    /**
     * 获取有效的候选人ID列表
     * @return 有效的候选人ID列表
     */
    vector<int> getValidIDs() const {
        vector<int> validIDs;
        for (const auto &c : candidates) {
            validIDs.push_back(c.id);
        }
        return validIDs;
    }
    
public:
    /**
     * 构造函数
     */
    ElectionSystem() {
        candidates.clear();
        idToIndex.clear();
        voteHistory.clear();
    }
    
    /**
     * 添加候选人
     * @param id 候选人编号
     * @param name 候选人姓名
     * @param department 所属单位
     * @return true表示成功，false表示失败（如ID重复）
     */
    bool addCandidate(int id, const string &name, const string &department = "") {
        // 数据验证
        if (!DataValidator::validateCandidateID(id)) {
            cout << "❌ 错误：候选人编号必须为正整数！\n";
            return false;
        }
        
        if (!DataValidator::validateName(name)) {
            cout << "❌ 错误：候选人姓名不合法！\n";
            return false;
        }
        
        // 检查ID是否重复
        if (idToIndex.count(id)) {
            cout << "❌ 错误：候选人编号 " << id << " 已存在！\n";
            return false;
        }
        
        candidates.push_back(Candidate(id, name, department));
        updateIndexMap();
        cout << "✅ 成功添加候选人：编号 " << id << "，姓名 " << name << "\n";
        return true;
    }
    
    /**
     * 修改候选人信息
     * @param id 候选人编号
     * @param newName 新姓名
     * @param newDepartment 新所属单位
     * @return true表示成功，false表示失败
     */
    bool modifyCandidate(int id, const string &newName, const string &newDepartment = "") {
        if (!idToIndex.count(id)) {
            cout << "❌ 错误：候选人编号 " << id << " 不存在！\n";
            return false;
        }
        
        if (!DataValidator::validateName(newName)) {
            cout << "❌ 错误：候选人姓名不合法！\n";
            return false;
        }
        
        int index = idToIndex[id];
        candidates[index].name = newName;
        candidates[index].department = newDepartment;
        cout << "✅ 成功修改候选人信息：编号 " << id << "\n";
        return true;
    }
    
    /**
     * 删除候选人
     * @param id 候选人编号
     * @return true表示成功，false表示失败
     */
    bool deleteCandidate(int id) {
        if (!idToIndex.count(id)) {
            cout << "❌ 错误：候选人编号 " << id << " 不存在！\n";
            return false;
        }
        
        int index = idToIndex[id];
        candidates.erase(candidates.begin() + index);
        updateIndexMap();
        cout << "✅ 成功删除候选人：编号 " << id << "\n";
        return true;
    }
    
    /**
     * 查询候选人
     * @param id 候选人编号
     * @return 候选人指针，如果不存在返回nullptr
     */
    Candidate* queryCandidate(int id) {
        if (!idToIndex.count(id)) {
            return nullptr;
        }
        return &candidates[idToIndex[id]];
    }
    
    /**
     * 获取所有候选人
     * @return 候选人列表的常量引用
     */
    const vector<Candidate>& getAllCandidates() const {
        return candidates;
    }
    
    /**
     * 投票（使用选举向量v）
     * 时间复杂度：O(n)，其中n是投票向量的长度
     * 空间复杂度：O(1)
     * @param votes 选举向量v，长度为n，每个元素是候选人ID
     */
    void vote(const vector<int> &votes) {
        // 重置所有候选人的得票数
        for (auto &c : candidates) {
            c.voteCount = 0;
        }
        
        // 清空投票历史
        voteHistory.clear();
        
        // 获取有效ID列表
        vector<int> validIDs = getValidIDs();
        
        // 验证投票向量
        int invalidCount = DataValidator::validateVoteVector(votes, validIDs);
        if (invalidCount > 0) {
            cout << "⚠️  警告：发现 " << invalidCount << " 张无效选票！\n";
        }
        
        // 统计投票
        for (int voteID : votes) {
            voteHistory.push_back(voteID);
            if (idToIndex.count(voteID)) {
                candidates[idToIndex[voteID]].voteCount++;
            }
        }
        
        cout << "✅ 投票完成！共处理 " << votes.size() << " 张选票。\n";
    }
    
    /**
     * 单票投票
     * @param candidateID 候选人编号
     * @return true表示成功，false表示失败
     */
    bool castVote(int candidateID) {
        if (!idToIndex.count(candidateID)) {
            cout << "❌ 错误：候选人编号 " << candidateID << " 不存在！\n";
            return false;
        }
        
        candidates[idToIndex[candidateID]].voteCount++;
        voteHistory.push_back(candidateID);
        cout << "✅ 投票成功！\n";
        return true;
    }
    
    /**
     * 查找优胜者（超过半数选票的候选人）
     * 使用Boyer-Moore多数投票算法的改进版本
     * 时间复杂度：O(n)，其中n是候选人数量
     * 空间复杂度：O(1)
     * @return 优胜者ID，如果没有超过半数的候选人则返回-1
     */
    int findWinner() {
        if (candidates.empty()) {
            return -1;
        }
        
        int totalVotes = Statistics::getTotalVotes(candidates);
        if (totalVotes == 0) {
            return -1;
        }
        
        // 方法1：直接遍历查找超过半数的候选人
        // 时间复杂度：O(n)，空间复杂度：O(1)
        for (const auto &c : candidates) {
            if (c.voteCount > totalVotes / 2) {
                return c.id;
            }
        }
        
        return -1; // 没有超过半数的候选人
    }
    
    /**
     * 获取投票历史
     * @return 投票历史向量
     */
    const vector<int>& getVoteHistory() const {
        return voteHistory;
    }
    
    /**
     * 清空所有数据
     */
    void clearAll() {
        candidates.clear();
        idToIndex.clear();
        voteHistory.clear();
        cout << "✅ 已清空所有数据！\n";
    }
    
    /**
     * 重置投票（保留候选人，清空得票数）
     */
    void resetVotes() {
        for (auto &c : candidates) {
            c.voteCount = 0;
        }
        voteHistory.clear();
        cout << "✅ 已重置所有投票！\n";
    }
};

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
                case 1: 
                    if (FileManager::saveCandidates(system->getAllCandidates())) {
                        cout << "✅ 候选人数据保存成功！\n";
                    } else {
                        cout << "❌ 保存失败！\n";
                    }
                    waitForEnter();
                    break;
                case 2: {
                    vector<Candidate> candidates;
                    if (FileManager::loadCandidates(candidates)) {
                        // 需要重新构建系统（这里简化处理）
                        cout << "✅ 候选人数据加载成功！\n";
                        cout << "⚠️  注意：加载数据后需要重新添加候选人到系统。\n";
                    } else {
                        cout << "❌ 加载失败！\n";
                    }
                    waitForEnter();
                    break;
                }
                case 3:
                    if (FileManager::saveVotes(system->getVoteHistory())) {
                        cout << "✅ 投票数据保存成功！\n";
                    } else {
                        cout << "❌ 保存失败！\n";
                    }
                    waitForEnter();
                    break;
                case 4: {
                    vector<int> votes;
                    if (FileManager::loadVotes(votes)) {
                        system->resetVotes();      // 数据维护中的“加载投票数据”先清零
                        system->vote(votes, true); // 再重建
                        cout << "✅ 投票数据加载成功！\n";
                    } else {
                        cout << "❌ 加载失败！\n";
                    }
                    waitForEnter();
                    break;
                }
                case 5: {
                    int winnerID = system->findWinner();
                    if (FileManager::exportReport(system->getAllCandidates(), winnerID)) {
                        cout << "✅ 统计报告导出成功！文件名: election_report.txt\n";
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
            cout << "  0. 返回主菜单\n";
            printSeparator();
            
            int choice = inputInt("请选择操作 [0-3]: ");
            
            switch (choice) {
                case 1: analyzeVoteData(); break;
                case 2: analyzeRanking(); break;
                case 3: analyzeDistribution(); break;
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
