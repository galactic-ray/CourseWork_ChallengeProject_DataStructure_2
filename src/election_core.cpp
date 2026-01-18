#include "../include/election_core.h"
#include <iostream>

// ==================== 文件管理模块实现（CSV / 文本格式） ====================

// 简单辅助：获取文件扩展名（小写）
static std::string getFileExtensionLower(const std::string &filename) {
    auto pos = filename.find_last_of('.');
    if (pos == std::string::npos) return "";
    std::string ext = filename.substr(pos + 1);
    for (auto &ch : ext) ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    return ext;
}

// 简单辅助：去掉字符串首尾空白
static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool FileManager::saveCandidates(const vector<Candidate> &candidates, 
                                  const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    std::string ext = getFileExtensionLower(filename);
    
    if (ext == "txt") {
        // 文本格式：简单空白分隔，便于人工查看与编辑
        file << "id name department voteCount\n";
        for (const auto &c : candidates) {
            file << c.id << ' '
                 << c.name << ' '
                 << c.department << ' '
                 << c.voteCount << '\n';
        }
    } else {
        // 默认CSV格式: id,name,department,voteCount
        file << "id,name,department,voteCount\n";
        for (const auto &c : candidates) {
            // 简化处理：假定姓名和单位中不包含逗号
            file << c.id << ','
                 << c.name << ','
                 << c.department << ','
                 << c.voteCount << '\n';
        }
    }
    
    file.close();
    return true;
}

bool FileManager::loadCandidates(vector<Candidate> &candidates, 
                                 const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    candidates.clear();
    std::string line;
    std::string ext = getFileExtensionLower(filename);
    
    if (ext == "txt") {
        // 文本格式：支持首行表头；每行按空白切分: id name department voteCount
        bool firstLine = true;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty()) continue;
            
            std::stringstream ss(line);
            std::string idStr, name, dept, voteStr;
            if (!(ss >> idStr >> name >> dept)) {
                // 可能是表头
                if (firstLine) {
                    firstLine = false;
                }
                continue;
            }
            // voteCount 可选，缺省为0
            if (!(ss >> voteStr)) {
                voteStr = "0";
            }
            
            Candidate c;
            try {
                c.id = std::stoi(trim(idStr));
                c.name = trim(name);
                c.department = trim(dept);
                c.voteCount = std::stoi(trim(voteStr));
            } catch (...) {
                continue; // 跳过格式错误的行
            }
            
            candidates.push_back(c);
            firstLine = false;
        }
    } else {
        // CSV格式：首行表头，其后每行一个候选人
        if (!std::getline(file, line)) {
            return false;
        }
        
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty()) continue;
            
            std::stringstream ss(line);
            std::string idStr, name, dept, voteStr;
            
            if (!std::getline(ss, idStr, ',')) continue;
            if (!std::getline(ss, name, ',')) continue;
            if (!std::getline(ss, dept, ',')) continue;
            if (!std::getline(ss, voteStr, ',')) continue;
            
            Candidate c;
            try {
                c.id = std::stoi(trim(idStr));
                c.name = trim(name);
                c.department = trim(dept);
                c.voteCount = std::stoi(trim(voteStr));
            } catch (...) {
                continue; // 跳过格式错误的行
            }
            
            candidates.push_back(c);
        }
    }
    
    file.close();
    return true;
}

bool FileManager::saveVotes(const vector<int> &votes, 
                            const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    std::string ext = getFileExtensionLower(filename);
    
    if (ext == "txt") {
        // 文本格式：每行一个投票ID，不写表头
        for (int v : votes) {
            file << v << '\n';
        }
    } else {
        // CSV格式：首行表头“vote”，每行一个ID
        file << "vote\n";
        for (int v : votes) {
            file << v << '\n';
        }
    }
    
    file.close();
    return true;
}

bool FileManager::loadVotes(vector<int> &votes, 
                            const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    votes.clear();
    std::string line;
    std::string ext = getFileExtensionLower(filename);
    
    if (ext == "txt") {
        // 文本格式：支持空白分隔或每行一个数字，无强制表头
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty()) continue;
            
            std::stringstream ss(line);
            std::string token;
            while (ss >> token) {
                try {
                    int v = std::stoi(token);
                    votes.push_back(v);
                } catch (...) {
                    // 忽略非数字token
                    continue;
                }
            }
        }
    } else {
        // CSV格式：首行可能是表头，也可能就是第一个数字
        if (!std::getline(file, line)) {
            return false;
        }
        
        line = trim(line);
        if (!line.empty()) {
            try {
                int v0 = std::stoi(line);
                votes.push_back(v0);
            } catch (...) {
                // 视为表头，忽略
            }
        }
        
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty()) continue;
            
            try {
                int v = std::stoi(line);
                votes.push_back(v);
            } catch (...) {
                // 跳过非数字行（例如表头）
                continue;
            }
        }
    }
    
    file.close();
    return true;
}

bool FileManager::exportReport(const vector<Candidate> &candidates, 
                               int winnerID, 
                               const string &filename) {
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



bool FileManager::exportTopicsData(const vector<VoteTopic> &topics,
                                  const vector<TopicVoteRecord> &voteHistory,
                                  const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Section 1: topics
    file << "#TOPICS\n";
    file << "topicId,title,description,createdAt,votesPerVoter\n";
    for (const auto &t : topics) {
        // 简化：假设 title/description 不包含逗号（如需更强健可做CSV转义）
        file << t.id << ','
             << t.title << ','
             << t.description << ','
             << static_cast<long long>(t.createdAt) << ','
             << t.votesPerVoter << '\n';
    }

    // Section 2: options
    file << "#OPTIONS\n";
    file << "topicId,optionId,text,voteCount\n";
    for (const auto &t : topics) {
        for (const auto &opt : t.options) {
            file << t.id << ','
                 << opt.id << ','
                 << opt.text << ','
                 << opt.voteCount << '\n';
        }
    }

    // Section 3: votes
    file << "#VOTES\n";
    file << "topicId,voterId,optionId,votedAt\n";
    for (const auto &rec : voteHistory) {
        file << rec.topicId << ','
             << rec.voterId << ','
             << rec.optionId << ','
             << static_cast<long long>(rec.votedAt) << '\n';
    }

    file.close();
    return true;
}

bool FileManager::importTopicsData(vector<VoteTopic> &topics,
                                  vector<TopicVoteRecord> &voteHistory,
                                  const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    topics.clear();
    voteHistory.clear();

    string line;
    enum class Section { None, Topics, Options, Votes };
    Section sec = Section::None;

    // Temporary map: topicId -> index
    unordered_map<int, size_t> tidToIdx;

    auto splitCsv = [](const string &s) {
        vector<string> out;
        string cur;
        for (char ch : s) {
            if (ch == ',') {
                out.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(ch);
            }
        }
        out.push_back(cur);
        return out;
    };

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (line == "#TOPICS") { sec = Section::Topics; continue; }
        if (line == "#OPTIONS") { sec = Section::Options; continue; }
        if (line == "#VOTES") { sec = Section::Votes; continue; }

        // skip header lines
        if (line.rfind("topicId,", 0) == 0) continue;

        auto cols = splitCsv(line);
        try {
            if (sec == Section::Topics) {
                if (cols.size() < 5) continue;
                VoteTopic t;
                t.id = std::stoi(cols[0]);
                t.title = cols[1];
                t.description = cols[2];
                t.createdAt = static_cast<time_t>(std::stoll(cols[3]));
                t.votesPerVoter = std::stoi(cols[4]);
                topics.push_back(t);
                tidToIdx[t.id] = topics.size() - 1;
            } else if (sec == Section::Options) {
                if (cols.size() < 4) continue;
                int tid = std::stoi(cols[0]);
                int oid = std::stoi(cols[1]);
                string text = cols[2];
                int vc = std::stoi(cols[3]);
                if (!tidToIdx.count(tid)) continue;
                VoteOption opt;
                opt.id = oid;
                opt.text = text;
                opt.voteCount = vc;
                topics[tidToIdx[tid]].options.push_back(opt);
            } else if (sec == Section::Votes) {
                if (cols.size() < 4) continue;
                int tid = std::stoi(cols[0]);
                string vid = cols[1];
                int oid = std::stoi(cols[2]);
                time_t ts = static_cast<time_t>(std::stoll(cols[3]));
                voteHistory.push_back(TopicVoteRecord(tid, vid, oid, ts));
            }
        } catch (...) {
            continue;
        }
    }

    file.close();
    return !topics.empty();
}


bool FileManager::exportSingleTopicData(const VoteTopic &topic,
                                       const vector<TopicVoteRecord> &voteHistory,
                                       const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << "#TOPICS\n";
    file << "topicId,title,description,createdAt,votesPerVoter\n";
    file << topic.id << ','
         << topic.title << ','
         << topic.description << ','
         << static_cast<long long>(topic.createdAt) << ','
         << topic.votesPerVoter << '\n';

    file << "#OPTIONS\n";
    file << "topicId,optionId,text,voteCount\n";
    for (const auto &opt : topic.options) {
        file << topic.id << ','
             << opt.id << ','
             << opt.text << ','
             << opt.voteCount << '\n';
    }

    file << "#VOTES\n";
    file << "topicId,voterId,optionId,votedAt\n";
    for (const auto &rec : voteHistory) {
        if (rec.topicId != topic.id) continue;
        file << rec.topicId << ','
             << rec.voterId << ','
             << rec.optionId << ','
             << static_cast<long long>(rec.votedAt) << '\n';
    }

    file.close();
    return true;
}

bool FileManager::importSingleTopicData(VoteTopic &topic,
                                       vector<TopicVoteRecord> &voteHistory,
                                       const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    topic = VoteTopic();
    topic.options.clear();
    voteHistory.clear();

    string line;
    enum class Section { None, Topics, Options, Votes };
    Section sec = Section::None;

    auto splitCsv = [](const string &s) {
        vector<string> out;
        string cur;
        for (char ch : s) {
            if (ch == ',') {
                out.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(ch);
            }
        }
        out.push_back(cur);
        return out;
    };

    int parsedTopicId = -1;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (line == "#TOPICS") { sec = Section::Topics; continue; }
        if (line == "#OPTIONS") { sec = Section::Options; continue; }
        if (line == "#VOTES") { sec = Section::Votes; continue; }

        if (line.rfind("topicId,", 0) == 0) continue;

        auto cols = splitCsv(line);
        try {
            if (sec == Section::Topics) {
                if (cols.size() < 5) continue;
                topic.id = std::stoi(cols[0]);
                parsedTopicId = topic.id;
                topic.title = cols[1];
                topic.description = cols[2];
                topic.createdAt = static_cast<time_t>(std::stoll(cols[3]));
                topic.votesPerVoter = std::stoi(cols[4]);
            } else if (sec == Section::Options) {
                if (cols.size() < 4) continue;
                int tid = std::stoi(cols[0]);
                if (parsedTopicId != -1 && tid != parsedTopicId) continue;
                VoteOption opt;
                opt.id = std::stoi(cols[1]);
                opt.text = cols[2];
                opt.voteCount = std::stoi(cols[3]);
                topic.options.push_back(opt);
            } else if (sec == Section::Votes) {
                if (cols.size() < 4) continue;
                int tid = std::stoi(cols[0]);
                if (parsedTopicId != -1 && tid != parsedTopicId) continue;
                string vid = cols[1];
                int oid = std::stoi(cols[2]);
                time_t ts = static_cast<time_t>(std::stoll(cols[3]));
                voteHistory.push_back(TopicVoteRecord(tid, vid, oid, ts));
            }
        } catch (...) {
            continue;
        }
    }

    file.close();
    return topic.id > 0 && topic.options.size() >= 2;
}
// ==================== 核心选举系统实现 ====================

bool ElectionSystem::addCandidate(int id, const string &name, const string &department) {
    // 数据验证
    if (!DataValidator::validateCandidateID(id)) {
        return false;
    }
    
    if (!DataValidator::validateName(name)) {
        return false;
    }
    
    // 检查ID是否重复
    if (idToIndex.count(id)) {
        return false;
    }
    
    candidates.push_back(Candidate(id, name, department));
    updateIndexMap();
    return true;
}

bool ElectionSystem::modifyCandidate(int id, const string &newName, const string &newDepartment) {
    if (!idToIndex.count(id)) {
        return false;
    }
    
    if (!DataValidator::validateName(newName)) {
        return false;
    }
    
    int index = idToIndex[id];
    candidates[index].name = newName;
    candidates[index].department = newDepartment;
    return true;
}

bool ElectionSystem::deleteCandidate(int id) {
    if (!idToIndex.count(id)) {
        return false;
    }
    
    int index = idToIndex[id];
    candidates.erase(candidates.begin() + index);
    updateIndexMap();
    return true;
}

void ElectionSystem::vote(const vector<int> &votes, bool resetExisting) {
    // 为了满足“除非主动清零，否则所有投票都累加”的需求，
    // 这里不再根据 resetExisting 清空数据，真正的清零操作由 resetVotes()/clearAll() 控制。
    (void)resetExisting; // 避免未使用参数告警
    
    // 获取有效ID列表
    vector<int> validIDs = getValidIDs();
    
    int invalidCount = DataValidator::validateVoteVector(votes, validIDs);
    if (invalidCount > 0) {
        cout << "⚠️  警告：发现 " << invalidCount << " 张无效选票！\n";
    }
    
    // 统计投票（在现有基础上累加）
    for (int voteID : votes) {
        voteHistory.push_back(voteID);
        if (idToIndex.count(voteID)) {
            candidates[idToIndex[voteID]].voteCount++;
        }
    }
}

bool ElectionSystem::castVote(int candidateID) {
    if (!idToIndex.count(candidateID)) {
        return false;
    }
    
    candidates[idToIndex[candidateID]].voteCount++;
    voteHistory.push_back(candidateID);
    return true;
}

int ElectionSystem::findWinner() {
    if (candidates.empty()) {
        return -1;
    }
    
    int totalVotes = Statistics::getTotalVotes(candidates);
    if (totalVotes == 0) {
        return -1;
    }
    
    // 直接遍历查找超过半数的候选人
    // 时间复杂度：O(n)，空间复杂度：O(1)
    for (const auto &c : candidates) {
        if (c.voteCount > totalVotes / 2) {
            return c.id;
        }
    }
    
    return -1; // 没有超过半数的候选人
}

bool ElectionSystem::undoLastVote() {
    if (voteHistory.empty()) {
        return false;
    }
    
    int lastVoteID = voteHistory.back();
    voteHistory.pop_back();
    
    // 减少该候选人的得票数
    if (idToIndex.count(lastVoteID)) {
        int index = idToIndex[lastVoteID];
        if (candidates[index].voteCount > 0) {
            candidates[index].voteCount--;
        }
    }
    
    return true;
}

int ElectionSystem::undoLastVotes(int k) {
    if (k <= 0) {
        return 0;
    }
    
    int actualCount = 0;
    int timesToUndo = std::min(k, static_cast<int>(voteHistory.size()));
    
    for (int i = 0; i < timesToUndo; i++) {
        if (undoLastVote()) {
            actualCount++;
        }
    }
    
    return actualCount;
}

int ElectionSystem::createTopic(const string &title, const string &description, const vector<string> &optionTexts, int votesPerVoter) {
    if (trim(title).empty()) {
        return -1;
    }
    if (optionTexts.size() < 2) {
        return -1;
    }

    VoteTopic topic;
    topic.id = nextTopicId++;
    topic.title = title;
    topic.description = description;
    topic.createdAt = time(nullptr);
    topic.votesPerVoter = votesPerVoter;

    int nextOptionId = 1;
    for (const auto &optTextRaw : optionTexts) {
        string optText = trim(optTextRaw);
        if (optText.empty()) continue;
        topic.options.push_back(VoteOption(nextOptionId++, optText));
    }

    if (topic.options.size() < 2) {
        return -1;
    }

    if (topic.votesPerVoter <= 0 || topic.votesPerVoter > static_cast<int>(topic.options.size())) {
        return -1;
    }

    topics.push_back(topic);
    updateTopicIndexMap();
    return topic.id;
}

bool ElectionSystem::deleteTopic(int topicId) {
    if (!topicIdToIndex.count(topicId)) {
        return false;
    }
    int idx = topicIdToIndex[topicId];
    topics.erase(topics.begin() + idx);
    // 清理该话题的已投票记录
    topicVotedUsers.erase(topicId);
    updateTopicIndexMap();
    return true;
}

VoteTopic* ElectionSystem::queryTopic(int topicId) {
    if (!topicIdToIndex.count(topicId)) {
        return nullptr;
    }
    return &topics[topicIdToIndex[topicId]];
}

bool ElectionSystem::castTopicVote(int topicId, int optionId) {
    VoteTopic *topic = queryTopic(topicId);
    if (!topic) {
        return false;
    }

    for (auto &opt : topic->options) {
        if (opt.id == optionId) {
            opt.voteCount++;
            return true;
        }
    }
    return false;
}

bool ElectionSystem::castTopicVote(int topicId, int optionId, const string &voterId) {
    VoteTopic *topic = queryTopic(topicId);
    if (!topic) {
        return false;
    }

    string vid = trim(voterId);
    if (vid.empty()) {
        return false;
    }

    if (topic->votesPerVoter <= 0) {
        return false;
    }

    // topicId -> voterId -> set<optionId>
    auto &voterMap = topicVotedUsers[topicId];
    auto &optionSet = voterMap[vid];

    // 已投票数达到上限
    if (static_cast<int>(optionSet.size()) >= topic->votesPerVoter) {
        return false;
    }

    // 不允许重复投同一选项
    if (optionSet.count(optionId)) {
        return false;
    }

    for (auto &opt : topic->options) {
        if (opt.id == optionId) {
            opt.voteCount++;
            optionSet.insert(optionId);
            topicVoteHistory.push_back(TopicVoteRecord(topicId, vid, optionId, time(nullptr)));
            return true;
        }
    }

    // 选项不存在：回滚空记录（可选）
    return false;
}


int ElectionSystem::getTopicRemainingVotes(int topicId, const string &voterId) const {
    auto itIdx = topicIdToIndex.find(topicId);
    if (itIdx == topicIdToIndex.end()) {
        return 0;
    }

    const VoteTopic &topic = topics[itIdx->second];
    if (topic.votesPerVoter <= 0) {
        return 0;
    }

    string vid = trim(voterId);
    if (vid.empty()) {
        return topic.votesPerVoter;
    }    auto itTopic = topicVotedUsers.find(topicId);
    if (itTopic == topicVotedUsers.end()) {
        return topic.votesPerVoter;
    }
    auto itVoter = itTopic->second.find(vid);
    if (itVoter == itTopic->second.end()) {
        return topic.votesPerVoter;
    }

    int used = static_cast<int>(itVoter->second.size());
    int remain = topic.votesPerVoter - used;
    return remain < 0 ? 0 : remain;
}


bool ElectionSystem::undoLastTopicVote(TopicVoteRecord *undone) {
    if (topicVoteHistory.empty()) {
        return false;
    }

    TopicVoteRecord rec = topicVoteHistory.back();
    topicVoteHistory.pop_back();

    if (undone) {
        *undone = rec;
    }

    VoteTopic *topic = queryTopic(rec.topicId);
    if (!topic) {
        return false;
    }

    // 找到选项并减票
    for (auto &opt : topic->options) {
        if (opt.id == rec.optionId) {
            if (opt.voteCount > 0) {
                opt.voteCount--;
            }
            break;
        }
    }

    // 从投票人记录中移除该选项
    auto itTopic = topicVotedUsers.find(rec.topicId);
    if (itTopic != topicVotedUsers.end()) {
        auto itVoter = itTopic->second.find(rec.voterId);
        if (itVoter != itTopic->second.end()) {
            itVoter->second.erase(rec.optionId);
            if (itVoter->second.empty()) {
                itTopic->second.erase(itVoter);
            }
        }
        if (itTopic->second.empty()) {
            topicVotedUsers.erase(itTopic);
        }
    }

    return true;
}

int ElectionSystem::getTopicTotalVotes(int topicId) const {
    if (!topicIdToIndex.count(topicId)) {
        return 0;
    }
    const VoteTopic &topic = topics[topicIdToIndex.at(topicId)];
    int total = 0;
    for (const auto &opt : topic.options) {
        total += opt.voteCount;
    }
    return total;
}
