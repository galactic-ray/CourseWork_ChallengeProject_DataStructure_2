#include "../include/election_core.h"
#include <iostream>

// ==================== 文件管理模块实现（CSV格式） ====================

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
    
    // 表头
    file << "id,name,department,voteCount\n";
    
    for (const auto &c : candidates) {
        // 简化处理：假定姓名和单位中不包含逗号
        file << c.id << ','
             << c.name << ','
             << c.department << ','
             << c.voteCount << '\n';
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
    
    // 读取表头（可选）
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
    
    file.close();
    return true;
}

bool FileManager::saveVotes(const vector<int> &votes, 
                            const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // 表头
    file << "vote\n";
    for (int v : votes) {
        file << v << '\n';
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
    
    // 读取表头（可选）
    if (!std::getline(file, line)) {
        return false;
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

