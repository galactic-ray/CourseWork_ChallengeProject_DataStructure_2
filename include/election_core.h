#ifndef ELECTION_CORE_H
#define ELECTION_CORE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <cctype>
#include <locale>
#include <codecvt>

using namespace std;

// ==================== 数据结构定义 ====================

struct VoteOption {
    int id;
    string text;
    int voteCount;

    VoteOption() : id(0), text(""), voteCount(0) {}
    VoteOption(int i, const string &t) : id(i), text(t), voteCount(0) {}
};

struct VoteTopic {
    int id;
    string title;
    string description;
    vector<VoteOption> options;
    time_t createdAt;
    int votesPerVoter;

    VoteTopic() : id(0), title(""), description(""), createdAt(0), votesPerVoter(1) {}
};

struct TopicVoteRecord {
    int topicId;
    string voterId;
    int optionId;
    time_t votedAt;

    TopicVoteRecord() : topicId(0), voterId(""), optionId(0), votedAt(0) {}
    TopicVoteRecord(int t, const string &v, int o, time_t ts) : topicId(t), voterId(v), optionId(o), votedAt(ts) {}
};

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
        
        bool hasLetter = false;
        for (size_t i = 0; i < name.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(name[i]);
            
            if (c < 0x80) {
                // 仅允许英文字符和空格
                if (std::isalpha(c)) {
                    hasLetter = true;
                    continue;
                }
                if (c == ' ') {
                    continue;
                }
                // 其他 ASCII（数字、标点等）不允许
                return false;
            } else {
                // 非 ASCII：视为中文字符（UTF-8 多字节的一部分）
                hasLetter = true;
                continue;
            }
        }
        return hasLetter;
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
    static bool saveTopics(const vector<VoteTopic> &topics,
                           const string &filename = "topics.csv");

    static bool loadTopics(vector<VoteTopic> &topics,
                           const string &filename = "topics.csv");

    static bool exportTopicReport(const VoteTopic &topic,
                                 const string &filename = "topic_report.txt");

    // 导出/导入：话题 + 选项 + 投票记录（单文件CSV，包含分段表头）
    static bool exportTopicsData(const vector<VoteTopic> &topics,
                                const vector<TopicVoteRecord> &voteHistory,
                                const string &filename = "topics_data.csv");
    static bool importTopicsData(vector<VoteTopic> &topics,
                                vector<TopicVoteRecord> &voteHistory,
                                const string &filename = "topics_data.csv");

    static bool exportSingleTopicData(const VoteTopic &topic,
                                     const vector<TopicVoteRecord> &voteHistory,
                                     const string &filename = "topic_data.csv");
    static bool importSingleTopicData(VoteTopic &topic,
                                     vector<TopicVoteRecord> &voteHistory,
                                     const string &filename = "topic_data.csv");
    /**
     * 保存候选人数据到文件
     * @param candidates 候选人列表
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    // 使用CSV格式保存候选人数据: id,name,department,voteCount
    static bool saveCandidates(const vector<Candidate> &candidates, 
                              const string &filename = "candidates.csv");
    
    /**
     * 从文件加载候选人数据
     * @param candidates 候选人列表（输出参数）
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    // 使用CSV格式加载候选人数据: id,name,department,voteCount
    static bool loadCandidates(vector<Candidate> &candidates, 
                              const string &filename = "candidates.csv");
    
    /**
     * 保存投票向量到文件
     * @param votes 投票向量
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    // 使用CSV格式保存投票数据: 每行一个voteID
    static bool saveVotes(const vector<int> &votes, 
                          const string &filename = "votes.csv");
    
    /**
     * 从文件加载投票向量
     * @param votes 投票向量（输出参数）
     * @param filename 文件名
     * @return true表示成功，false表示失败
     */
    // 使用CSV格式加载投票数据: 支持首行表头
    static bool loadVotes(vector<int> &votes, 
                          const string &filename = "votes.csv");
    
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
     * 按姓名排序候选人（标准字典序升序）
     * @param candidates 候选人列表（会被修改）
     */
    static void sortByName(vector<Candidate> &candidates) {
        auto isAsciiEnglishName = [](const string &name) -> bool {
            if (name.empty()) return false;
            for (unsigned char c : name) {
                if (c >= 0x80) return false;
                if (!(std::isalpha(c) || c == ' ')) {
                    return false;
                }
            }
            return true;
        };
        
        auto makeAsciiKey = [](const string &name) -> string {
            string key;
            key.reserve(name.size());
            for (unsigned char c : name) {
                if (c == ' ') continue;
                key.push_back(static_cast<char>(std::tolower(c)));
            }
            return key;
        };
        
        auto makeChineseKey = [](const string &name) -> string {
            try {
                // 使用中文本地化规则在 UTF-8 字节序上生成排序键，近似按拼音排序
                static std::locale zhLoc("zh_CN.UTF-8");
                const auto &coll = std::use_facet<std::collate<char>>(zhLoc);
                return coll.transform(name.data(), name.data() + name.size());
            } catch (...) {
                // 回退：直接使用原始字节序
                return name;
            }
        };
        
        sort(candidates.begin(), candidates.end(),
             [&](const Candidate &a, const Candidate &b) {
                 bool aEng = isAsciiEnglishName(a.name);
                 bool bEng = isAsciiEnglishName(b.name);
                 
                 // 所有英文名排在中文名之前
                 if (aEng != bEng) {
                     return aEng; // true 在前
                 }
                 
                 if (aEng && bEng) {
                     string ka = makeAsciiKey(a.name);
                     string kb = makeAsciiKey(b.name);
                     if (ka == kb) {
                         return a.id < b.id;
                     }
                     return ka < kb;
                 }
                 
                 // 中文（或混合）名：使用本地化排序键
                 string ka = makeChineseKey(a.name);
                 string kb = makeChineseKey(b.name);
                 if (ka == kb) {
                     return a.id < b.id;
                 }
                 return ka < kb;
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

    vector<VoteTopic> topics;
    unordered_map<int, int> topicIdToIndex;

    // topicId -> voterId -> set<optionId> (已投的选项ID集合)，用于支持“每人可投N票且不能重复投同一选项”
    unordered_map<int, unordered_map<string, unordered_set<int>>> topicVotedUsers;

    int nextTopicId;

    // 话题投票历史（用于管理员撤销最近一次前端投票）
    vector<TopicVoteRecord> topicVoteHistory;

    void updateTopicIndexMap() {
        topicIdToIndex.clear();
        for (size_t i = 0; i < topics.size(); i++) {
            topicIdToIndex[topics[i].id] = i;
        }
    }

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
        topics.clear();
        topicIdToIndex.clear();
        topicVotedUsers.clear();
        topicVoteHistory.clear();
        nextTopicId = 1;
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
    void vote(const vector<int> &votes, bool resetExisting = true);
    
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
     * 撤销最近一次投票
     * @return true 撤销成功，false 无投票可撤销
     */
    bool undoLastVote();
    
    /**
     * 撤销最近k次投票
     * @param k 要撤销的次数（<= 投票历史长度），<=0时不操作
     * @return 实际撤销次数
     */
    int undoLastVotes(int k);
    
    /**
     * 清空所有数据
     */
    void clearAll() {
        candidates.clear();
        idToIndex.clear();
        voteHistory.clear();
        topics.clear();
        topicIdToIndex.clear();
        topicVotedUsers.clear();
        topicVoteHistory.clear();
        nextTopicId = 1;
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

    int createTopic(const string &title, const string &description, const vector<string> &optionTexts, int votesPerVoter = 1);
    bool deleteTopic(int topicId);
    VoteTopic* queryTopic(int topicId);
    const vector<VoteTopic>& getAllTopics() const {
        return topics;
    }

    bool castTopicVote(int topicId, int optionId);
    // 带投票人ID的投票，确保每个投票人在同一话题仅能投一次
    bool castTopicVote(int topicId, int optionId, const string &voterId);
    int getTopicRemainingVotes(int topicId, const string &voterId) const;
    int getTopicTotalVotes(int topicId) const;
    bool undoLastTopicVote(TopicVoteRecord *undone = nullptr);
    const vector<TopicVoteRecord>& getTopicVoteHistory() const { return topicVoteHistory; }
};

#endif // ELECTION_CORE_H

