#ifndef ELECTION_CORE_H
#define ELECTION_CORE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>

using namespace std;

// ==================== 数据结构定义 ====================

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
            // 允许中文字符（UTF-8编码）
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
                              const string &filename = "candidates.dat");
    
    /**
     * 从文件加载候选人数据
     * @param candidates 候选人列表（输出参数）
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    static bool loadCandidates(vector<Candidate> &candidates, 
                              const string &filename = "candidates.dat");
    
    /**
     * 保存投票向量到文件
     * @param votes 投票向量
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    static bool saveVotes(const vector<int> &votes, 
                          const string &filename = "votes.dat");
    
    /**
     * 从文件加载投票向量
     * @param votes 投票向量（输出参数）
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    static bool loadVotes(vector<int> &votes, 
                          const string &filename = "votes.dat");
    
    /**
     * 导出统计报告到文本文件
     * @param candidates 候选人列表
     * @param winnerID 优胜者ID
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    static bool exportReport(const vector<Candidate> &candidates, 
                             int winnerID, 
                             const string &filename = "election_report.txt");
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
    bool addCandidate(int id, const string &name, const string &department = "");
    
    /**
     * 修改候选人信息
     * @param id 候选人编号
     * @param newName 新姓名
     * @param newDepartment 新所属单位
     * @return true表示成功，false表示失败
     */
    bool modifyCandidate(int id, const string &newName, const string &newDepartment = "");
    
    /**
     * 删除候选人
     * @param id 候选人编号
     * @return true表示成功，false表示失败
     */
    bool deleteCandidate(int id);
    
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
     * 时间复杂度：O(m)，其中m是投票向量的长度
     * 空间复杂度：O(m)
     * @param votes 选举向量v，长度为m，每个元素是候选人ID
     */
    void vote(const vector<int> &votes);
    
    /**
     * 单票投票
     * @param candidateID 候选人编号
     * @return true表示成功，false表示失败
     */
    bool castVote(int candidateID);
    
    /**
     * 查找优胜者（超过半数选票的候选人）
     * 时间复杂度：O(n)，其中n是候选人数量
     * 空间复杂度：O(1)
     * @return 优胜者ID，如果没有超过半数的候选人则返回-1
     */
    int findWinner();
    
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
    }
    
    /**
     * 重置投票（保留候选人，清空得票数）
     */
    void resetVotes() {
        for (auto &c : candidates) {
            c.voteCount = 0;
        }
        voteHistory.clear();
    }
};

#endif // ELECTION_CORE_H

