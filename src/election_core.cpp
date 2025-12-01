#include "../include/election_core.h"
#include <iostream>

// ==================== 文件管理模块实现 ====================

bool FileManager::saveCandidates(const vector<Candidate> &candidates, 
                                  const string &filename) {
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

bool FileManager::loadCandidates(vector<Candidate> &candidates, 
                                 const string &filename) {
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

bool FileManager::saveVotes(const vector<int> &votes, 
                            const string &filename) {
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

bool FileManager::loadVotes(vector<int> &votes, 
                            const string &filename) {
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

void ElectionSystem::vote(const vector<int> &votes) {
    // 重置所有候选人的得票数
    for (auto &c : candidates) {
        c.voteCount = 0;
    }
    
    // 清空投票历史
    voteHistory.clear();
    
    // 获取有效ID列表
    vector<int> validIDs = getValidIDs();
    
    // 统计投票
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

