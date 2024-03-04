#include <filesystem>
#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
using namespace std;
const int MaxFieldLen = 16;
int64_t UserCnt = 0;
int64_t TotalArticleCnt = 0;
int64_t LastUserID = 1;
int64_t LastArticleID = 1;
std::streampos RightMasterPos = 0;
std::streampos RightSlavePos = 0;
vector<int> rubbish_m;
vector<int> rubbish_s;
struct Key
{
    int64_t user_id = 0;
    int64_t address = -1;
};
vector<Key> index_table;
struct User
{
    int64_t user_id = 0;
    char username[MaxFieldLen] = {};
    char password[MaxFieldLen] = {};
    int64_t adress = -1;
    int64_t slave_adress = -1;
};
struct Article
{
    int64_t author_code = 0;
    int64_t article_code = 0;
    char category[MaxFieldLen] = {};
    char name[MaxFieldLen] = {};
    int64_t next = -1;
};
std::ostream& operator<<(std::ostream& os, const User& a)
{
    os << "User ID: " << a.user_id << ", "
        << "Username: " << a.username << ", "
        << "Password: " << a.password << ", "
        << "First Article Address: " << a.slave_adress << ", ";
    return os;
}
std::ostream& operator<<(std::ostream& os, const Article& a)
{
    os << "+-----------------------------------------------------------------+\n";
    os << "| Author Code  | Article Code  | Name           | Category        |\n";
    os << "+-----------------------------------------------------------------+\n";
    os << "| " << std::setw(12) << a.author_code << " | " << std::setw(13) << a.article_code << " | " << std::setw(14) << a.name << " | " << std::setw(14) << a.category << "  |\n";
    os << "+-----------------------------------------------------------------+\n" << "\n";
    return os;

}
bool FillUserInfo(User& u, string name, string pass)
{
    for (int i = 0; i < MaxFieldLen; ++i)
    {
        u.username[i] = 0;
        u.password[i] = 0;
    }
    if (name.size() > MaxFieldLen - 1)
    {
        cout << "Error: the entered text is too long. Don't write more than 15 symbols" << "\n";
        return false;
    }
    if (pass.size() > MaxFieldLen - 1)
    {
        cout << "Error: the entered text is too long. Don't write more than 15 symbols" << "\n";
        return false;
    }
    for (int i = 0; i < name.size(); ++i) u.username[i] = name[i];
    for (int i = 0; i < pass.size(); ++i) u.password[i] = pass[i];
    return true;
}
bool FillArticleInfo(Article& a, string name, string category)
{
    for (int i = 0; i < MaxFieldLen; ++i)
    {
        a.name[i] = 0;
        a.category[i] = 0;
    }
    if (name.size() > MaxFieldLen - 1)
    {
        cout << "Error: the entered text is too long. Don't write more than 15 symbols" << "\n";
        return false;
    }
    if (category.size() > MaxFieldLen - 1)
    {
        cout << "Error: the entered text is too long. Don't write more than 15 symbols" << "\n";
        return false;
    }
    for (int i = 0; i < name.size(); ++i) a.name[i] = name[i];
    for (int i = 0; i < category.size(); ++i) a.category[i] = category[i];
    return true;
}
bool ReadMaster(User& record, std::fstream& file, const std::streampos& pos)
{
    if (!file)  return false;
    file.seekg(pos);
    file.read(reinterpret_cast<char*>(&record), sizeof(User));
    return !file.fail();
}
bool ReadSlave(Article& record, std::fstream& file, const std::streampos& pos)
{
    if (!file)  return false;
    file.seekg(pos);
    file.read(reinterpret_cast<char*>(&record), sizeof(Article));
    return !file.fail();
}
bool WriteMaster(const User& record, std::fstream& file, const std::streampos& pos)
{
    if (!file) return false;
    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&record), sizeof(User));
    file.flush();
    return !file.fail();
}
bool WriteSlave(const Article& record, std::fstream& file, const std::streampos& pos)
{
    if (!file) return false;
    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&record), sizeof(Article));
    file.flush();
    return !file.fail();
}
bool WriteKey(Key& record, std::fstream& file, std::streampos& pos)
{
    if (!file) return false;
    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&record), sizeof(Key));
    file.flush();
    pos = pos + static_cast<std::streamoff>(sizeof(Key));
    return !file.fail();
}
void PrintNodesMaster(std::fstream& file, const std::streampos& record_pos, bool IsFirstRow)
{
    User tmp;
    std::streampos read_pos = record_pos;
    if (!ReadMaster(tmp, file, read_pos))
    {
        std::cerr << "Unable to update next_ptr. Error: read failed" << "\n";
        return;
    }
    if (IsFirstRow) cout << "+----------------------------------------------------------+\n";
    if (IsFirstRow) cout << "| User ID  | Username         | Password         | Address |\n";
    if (IsFirstRow) cout << "+----------------------------------------------------------+\n";
    cout << "| " << std::setw(8) << tmp.user_id << " | " << std::setw(16) << tmp.username << " | " << std::setw(16) << tmp.password << " | " << std::setw(8) << record_pos << "|\n";
    cout << "+----------------------------------------------------------+\n";
}

void PrintNodesSlave(std::fstream& file, const std::streampos& record_pos)
{
    Article tmp;
    std::streampos read_pos = record_pos;
    cout << "+-------------------------------------------------------------------------------------------------+\n";
    cout << "| Author Code  | Article Code  | Name           | Category       | Address | Next Article Address |\n";
    cout << "+-------------------------------------------------------------------------------------------------+\n";
    while (read_pos != -1)
    {
        if (!ReadSlave(tmp, file, read_pos))
        {
            std::cerr << "Unable to update next_ptr. Error: read failed" << "\n";
            break;
        }
        cout << "| " << std::setw(12) << tmp.author_code << " | " << std::setw(13) << tmp.article_code << " | " << std::setw(14) << tmp.name << " | " << std::setw(14) << tmp.category << " | " << std::setw(7) << read_pos << " | " << std::setw(20) << tmp.next << " |\n";
        read_pos = tmp.next;
    }
    cout << "+-------------------------------------------------------------------------------------------------+\n" << "\n";
}
int StrToInt(string& s)
{
    int num = 0;
    for (int i = s.size() - 1; i >= 0; --i)
    {
        if (s[i] < 48 || s[i]>57)
        {
            s = "error";
            return -1;
        }
        num = 10 * num + (s[i] - 48);
    }
    return num;
}
int GetUserIndex(int UserID)
{
    for (int i = 0; i < index_table.size(); ++i)
    {
        if (index_table[i].user_id == UserID) return i;
    }
    return -1;
}
int GetSlaveAdress(int master_index, int slave_ID, std::fstream& masterfile, std::fstream& slavefile, std::streampos& prev_pos)
{
    std::streampos user_address = index_table[master_index].address;
    User user;
    ReadMaster(user, masterfile, user_address);
    if (user.slave_adress == -1) return -1;
    Article tmp;
    std::streampos CurPos = user.slave_adress;
    prev_pos = -1;
    while (CurPos != -1)
    {
        if (!ReadSlave(tmp, slavefile, CurPos)) return -2;
        if (tmp.article_code == slave_ID) return CurPos;
        prev_pos = CurPos;
        CurPos = tmp.next;
    }
    return -3;
}
int GetMasterIndex(int slave_ID, std::fstream& masterfile, std::fstream& slavefile, std::streampos& prev_pos)
{
    int adress;
    for (int i = 0; i < index_table.size(); ++i)
    {
        adress = GetSlaveAdress(i, slave_ID, masterfile, slavefile, prev_pos);
        if (adress >= 0) return i;
    }
    return -1;
}
void CopyMaster(std::fstream& masterfile, int MasterIndex, int new_address)
{
    int prev_address = index_table[MasterIndex].address;
    User user;
    ReadMaster(user, masterfile, prev_address);
    index_table[MasterIndex].address = new_address;
    WriteMaster(user, masterfile, new_address);
}
void CopySlave(std::fstream& masterfile, std::fstream& slavefile, int MasterIndex, int prev_adress, int cur_adress, int next_adress)
{
    int user_adress = index_table[MasterIndex].address;
    User user;
    Article tmp;
    ReadMaster(user, masterfile, user_adress);
    ReadSlave(tmp, slavefile, cur_adress);
    if (prev_adress == -1)
    {
        user.slave_adress = next_adress;
        WriteMaster(user, masterfile, user_adress);
        WriteSlave(tmp, slavefile, next_adress);
    }
    else
    {
        WriteSlave(tmp, slavefile, next_adress);
        ReadSlave(tmp, slavefile, prev_adress);
        tmp.next = next_adress;
        WriteSlave(tmp, slavefile, prev_adress);
    }
}
void Rewrite(std::fstream& masterfile, std::fstream& slavefile)
{
    User user;
    Article tmp;
    std::streampos NewMasterPos = 0;
    std::streampos NewSlavePos = 0;
    std::streampos NextPos, PrevPos;
    int UserNum;
    vector<pair<int, int> >vm;
    vector<pair<int, int> >vs;
    int UserCnt = index_table.size();
    for (int i = 0; i < UserCnt; ++i) vm.push_back({ index_table[i].address,i });
    sort(vm.begin(), vm.end());
    for (int i = 0; i < UserCnt; ++i)
    {
        UserNum = vm[i].second;
        if (NewMasterPos != vm[i].first) CopyMaster(masterfile, UserNum, NewMasterPos);
        NewMasterPos = NewMasterPos + static_cast<std::streamoff>(sizeof(User));
    }
    for (int i = 0; i < UserCnt; ++i)
    {
        ReadMaster(user, masterfile, index_table[i].address);
        if (user.slave_adress != -1)
        {
            NextPos = user.slave_adress;
            ReadSlave(tmp, slavefile, NextPos);
            vs.push_back({ NextPos,i });
            NextPos = tmp.next;
            while (NextPos != -1)
            {
                ReadSlave(tmp, slavefile, NextPos);
                vs.push_back({ NextPos,i });
                NextPos = tmp.next;
            }
        }
    }
    sort(vs.begin(), vs.end());
    for (int i = 0; i < vs.size(); ++i)
    {
        UserNum = vs[i].second;
        if (NewSlavePos != vs[i].first)
        {
            ReadMaster(user, masterfile, index_table[UserNum].address);
            PrevPos = user.slave_adress;
            ReadSlave(tmp, slavefile, PrevPos);
            while (tmp.next != -1 && tmp.next != vs[i].first)
            {
                PrevPos = tmp.next;
                ReadSlave(tmp, slavefile, PrevPos);
            }
            if (tmp.next == -1) PrevPos = -1;
            CopySlave(masterfile, slavefile, UserNum, PrevPos, vs[i].first, NewSlavePos);
        }
        NewSlavePos = NewSlavePos + static_cast<std::streamoff>(sizeof(Article));
    }
}
void GetM(int UserID, std::fstream& masterfile)
{
    int master_index = GetUserIndex(UserID);
    if (master_index == -1)
    {
        cout << "Error: User with given ID doesn't exist" << "\n";
        return;
    }
    PrintNodesMaster(masterfile, index_table[master_index].address, true);
}
void GetS(int ArticleID, std::fstream& masterfile, std::fstream& slavefile)
{
    std::streampos PrevSlavePos = -1;
    int master_index = GetMasterIndex(ArticleID, masterfile, slavefile, PrevSlavePos);
    if (master_index == -1)
    {
        cout << "Error: the Article with given ID doesn't exist" << "\n";
        return;
    }
    int64_t slave_adress = GetSlaveAdress(master_index, ArticleID, masterfile, slavefile, PrevSlavePos);
    Article TempArticle;
    if (slave_adress == -2)
    {
        std::cerr << "Unable to update next_ptr. Error: read failed" << "\n";
        return;
    }
    ReadSlave(TempArticle, slavefile, slave_adress);
    cout << TempArticle << "\n";
}
void DelM(int UserID, std::fstream& masterfile, std::fstream& slavefile)
{
    int master_index = GetUserIndex(UserID);
    if (master_index == -1)
    {
        cout << "Error: User with given ID doesn't exist" << "\n";
        return;
    }
    std::streampos user_address = index_table[master_index].address;
    rubbish_m.push_back(user_address);
    User user;
    ReadMaster(user, masterfile, user_address);
    index_table.erase(index_table.begin() + master_index);
    --UserCnt;
    cout << "User with ID " << UserID << " deleted!" << "\n";
    if (user.slave_adress == -1) return;
    std::streampos CurPos = user.slave_adress;
    Article TempArticle;
    while (CurPos != -1)
    {
        if (!ReadSlave(TempArticle, slavefile, CurPos))
        {
            std::cerr << "Unable to update next_ptr. Error: read failed" << "\n";
            break;
        }
        rubbish_s.push_back(CurPos);
        --TotalArticleCnt;
        CurPos = TempArticle.next;
    }
}
void DelS(int ArticleID, std::fstream& masterfile, std::fstream& slavefile)
{
    std::streampos PrevSlavePos = -1;
    int master_index = GetMasterIndex(ArticleID, masterfile, slavefile, PrevSlavePos);
    if (master_index == -1)
    {
        cout << "Error: the Article with given ID doesn't exist" << "\n";
        return;
    }
    std::streampos user_address = index_table[master_index].address;
    std::streampos slave_adress = GetSlaveAdress(master_index, ArticleID, masterfile, slavefile, PrevSlavePos);
    if (slave_adress == -2)
    {
        std::cerr << "Unable to update next_ptr. Error: read failed" << "\n";
        return;
    }
    Article TempArticle;
    ReadSlave(TempArticle, slavefile, slave_adress);
    rubbish_s.push_back(slave_adress);
    User user;
    if (PrevSlavePos == -1)
    {
        ReadMaster(user, masterfile, user_address);
        user.slave_adress = TempArticle.next;
        WriteMaster(user, masterfile, user_address);
    }
    else
    {
        slave_adress = TempArticle.next;
        ReadSlave(TempArticle, slavefile, PrevSlavePos);
        TempArticle.next = slave_adress;
        WriteSlave(TempArticle, slavefile, PrevSlavePos);
    }
    cout << "Article with ID " << ArticleID << " deleted!" << "\n";
    --TotalArticleCnt;
}
void UpdateM(int UserID, string field, string val, std::fstream& masterfile)
{
    int master_index = GetUserIndex(UserID);
    if (master_index == -1)
    {
        cout << "Error: User with given ID doesn't exist" << "\n";
        return;
    }
    std::streampos user_address = index_table[master_index].address;
    User user;
    ReadMaster(user, masterfile, user_address);
    if (field != "username" && field != "password")
    {
        cout << "Error: the User doesn't have a field named " << field << "\n";
        return;
    }
    bool FilledCorrectly;
    if (field == "username") FilledCorrectly = FillUserInfo(user, val, user.password);
    if (field == "password") FilledCorrectly = FillUserInfo(user, user.username, val);
    if (!FilledCorrectly) return;
    WriteMaster(user, masterfile, user_address);
    cout << field << " is updated! New value: " << val << "\n";
}
void UpdateS(int ArticleID, string field, string val, std::fstream& masterfile, std::fstream& slavefile)
{
    std::streampos PrevSlavePos = -1;
    int master_index = GetMasterIndex(ArticleID, masterfile, slavefile, PrevSlavePos);
    if (master_index == -1)
    {
        cout << "Error: Article with given ID doesn't exist" << "\n";
        return;
    }
    std::streampos slave_adress = GetSlaveAdress(master_index, ArticleID, masterfile, slavefile, PrevSlavePos);
    if (slave_adress == -2)
    {
        std::cerr << "Unable to update next_ptr. Error: read failed" << "\n";
        return;
    }
    Article TempArticle;
    ReadSlave(TempArticle, slavefile, slave_adress);
    if (field != "name" && field != "category")
    {
        cout << "Error: the Article doesn't have a field named " << field << "\n";
        return;
    }
    bool FilledCorrectly;
    if (field == "name") FilledCorrectly = FillArticleInfo(TempArticle, val, TempArticle.category);
    if (field == "category") FilledCorrectly = FillArticleInfo(TempArticle, TempArticle.name, val);
    if (!FilledCorrectly) return;
    WriteSlave(TempArticle, slavefile, slave_adress);
    cout << field << " is updated! New value: " << val << "\n";
}
void InsertM(string name, string pass, std::fstream& masterfile)
{
    User user;
    user.user_id = LastUserID;
    bool FilledCorrectly = FillUserInfo(user, name, pass);
    bool RightPosUsed = false;
    std::streampos InsertPos;
    if (rubbish_m.empty())
    {
        InsertPos = RightMasterPos;
        RightPosUsed = true;
    }
    else
    {
        InsertPos = rubbish_m[rubbish_m.size() - 1];
        rubbish_m.pop_back();
    }
    if (!FilledCorrectly) return;
    WriteMaster(user, masterfile, InsertPos);
    Key k{ LastUserID,InsertPos };
    index_table.push_back(k);
    if (RightPosUsed) RightMasterPos = RightMasterPos + static_cast<std::streamoff>(sizeof(User));
    cout << "new User added! ID: " << LastUserID << ", username: " << name << ", password: " << pass << ", adress: " << InsertPos << "\n";
    ++LastUserID;
    ++UserCnt;
}
void InsertS(int UserID, string name, string category, std::fstream& masterfile, std::fstream& slavefile)
{
    int master_index = GetUserIndex(UserID);
    if (master_index == -1)
    {
        cout << "Error: User with given ID doesn't exist" << "\n";
        return;
    }
    std::streampos user_address = index_table[master_index].address;
    User user;
    ReadMaster(user, masterfile, user_address);
    bool RightPosUsed = false;
    std::streampos PrevSlavePos = -1;
    std::streampos InsertPos, CurPos;
    Article TempArticle;
    if (rubbish_s.empty())
    {
        InsertPos = RightSlavePos;
        RightPosUsed = true;
    }
    else
    {
        InsertPos = rubbish_s[0];
        rubbish_s.erase(rubbish_s.begin());
    }
    if (user.slave_adress == -1)
    {
        user.slave_adress = InsertPos;
        WriteMaster(user, masterfile, user_address);
    }
    else
    {
        CurPos = user.slave_adress;
        while (CurPos != -1)
        {
            if (!ReadSlave(TempArticle, slavefile, CurPos))
            {
                std::cerr << "Unable to update next_ptr. Error: read failed" << "\n";
                break;
            }
            PrevSlavePos = CurPos;
            CurPos = TempArticle.next;
        }
    }
    TempArticle.article_code = LastArticleID;
    TempArticle.author_code = UserID;
    TempArticle.next = -1;
    bool FilledCorrectly = FillArticleInfo(TempArticle, name, category);
    if (!FilledCorrectly) return;
    WriteSlave(TempArticle, slavefile, InsertPos);
    if (PrevSlavePos != -1)
    {
        ReadSlave(TempArticle, slavefile, PrevSlavePos);
        TempArticle.next = InsertPos;
        WriteSlave(TempArticle, slavefile, PrevSlavePos);
    }
    cout << "new Article added! ID: " << LastArticleID << ", name: " << name << ", category: " << category << ", address: " << InsertPos << "\n";
    if (RightPosUsed) RightSlavePos = RightSlavePos + static_cast<std::streamoff>(sizeof(Article));
    ++LastArticleID;
    ++TotalArticleCnt;
}
int CalcS(int UserID, std::fstream& masterfile, std::fstream& slavefile)
{
    int master_index = GetUserIndex(UserID);
    if (master_index == -1) return -1;
    int ArticleCnt = 0;
    std::streampos user_address = index_table[master_index].address;
    User user;
    ReadMaster(user, masterfile, user_address);
    if (user.slave_adress == -1) return 0;
    std::streampos CurPos = user.slave_adress;
    Article TempArticle;
    while (CurPos != -1)
    {
        if (!ReadSlave(TempArticle, slavefile, CurPos))
        {
            std::cerr << "Unable to update next_ptr. Error: read failed" << "\n";
            break;
        }
        CurPos = TempArticle.next;
        ++ArticleCnt;
    }
    return ArticleCnt;
}
void UtS(string val, std::fstream& masterfile, std::fstream& slavefile)
{
    std::streampos user_address, CurPos;
    User user;
    if (val == "all")
    {
        for (int i = 0; i < index_table.size(); ++i)
        {
            user_address = index_table[i].address;
            ReadMaster(user, masterfile, user_address);
            cout << "User with ID " << user.user_id << ":" << "\n";
            if (user.slave_adress == -1) cout << "This User doesn't have any articles" << "\n";
            else
            {
                CurPos = user.slave_adress;
                PrintNodesSlave(slavefile, CurPos);
            }
        }
        return;
    }
    int UserID = StrToInt(val);
    if (val == "error")
    {
        cout << "Error: incorrect input" << "\n";
        return;
    }
    int master_index = GetUserIndex(UserID);
    if (master_index == -1)
    {
        cout << "Error: User with given ID doesn't exist" << "\n";
        return;
    }
    user_address = index_table[master_index].address;
    ReadMaster(user, masterfile, user_address);
    if (user.slave_adress == -1)
    {
        cout << "Error: the User with a given ID doesn't have any articles" << "\n";
        return;
    }
    CurPos = user.slave_adress;
    PrintNodesSlave(slavefile, CurPos);
}
std::vector<std::string> DivideLine(const std::string& line)
{
    std::vector<std::string> parts;
    std::istringstream iss(line);
    std::string part;
    while (iss >> part) parts.push_back(part);
    return parts;
}
void ProcessCommand(const std::string& line, std::fstream& masterfile, std::fstream& slavefile, bool& NotFinished)
{
    std::vector<std::string> parts = DivideLine(line);
    string CommandName = parts[0];
    int cnt = parts.size();
    if (CommandName == "help")
    {
        if (cnt != 1)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        cout << "Laboratory work 1. Made by Kozurak Denis from K-24 group" << "\n";
        cout << "This program allows you to work with two tables from ER Database Diagram: User and Article tables" << "\n";
        cout << "List of possible commands:" << "\n";
        cout << "get-m [userID]: finds and writes info about a User with given ID" << "\n";
        cout << "get-s [articleID]: finds and writes info about the Article with a given ID" << "\n";
        cout << "del-m [userID]: finds and deletes info about the User with given ID and all of his articles" << "\n";
        cout << "del-s [articleID]: finds and deletes info about the Article with given ID" << "\n";
        cout << "update-m [userID] [field] [value]: updates the value of the field of a certain User" << "\n";
        cout << "update-s [articleID] [field] [value]: updates the value of a certain Article" << "\n";
        cout << "insert-m [username] [password]: inserts the new User with given parameteres" << "\n";
        cout << "insert-s [userID] [name] [category]: inserts the new Article with given parameteres to the User with given ID" << "\n";
        cout << "calc-m: calculates the number of Users" << "\n";
        cout << "calc-s [userID]: calculates the number of Articles written by a User with given ID" << "\n";
        cout << "ut-m: writes info about all Users" << "\n";
        cout << "ut-s [userID]: writes info about all Articles written by a User with given ID. Enter `ut-s all` to get info about ALL Articles" << "\n";
        cout << "exit: ends the program" << "\n";
        return;
    }
    if (CommandName == "get-m")
    {
        if (cnt != 2)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        int UserID = StrToInt(parts[1]);
        if (parts[1] == "error")
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        GetM(UserID, masterfile);
        return;
    }
    if (CommandName == "get-s")
    {
        if (cnt != 2)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        int ArticleID = StrToInt(parts[1]);
        if (parts[1] == "error")
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        GetS(ArticleID, masterfile, slavefile);
        return;
    }
    if (CommandName == "del-m")
    {
        if (cnt != 2)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        int UserID = StrToInt(parts[1]);
        if (parts[1] == "error")
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        DelM(UserID, masterfile, slavefile);
        return;
    }
    if (CommandName == "del-s")
    {
        if (cnt != 2)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        int ArticleID = StrToInt(parts[1]);
        if (parts[1] == "error")
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        DelS(ArticleID, masterfile, slavefile);
        return;
    }
    if (CommandName == "update-m")
    {
        if (cnt != 4)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        int UserID = StrToInt(parts[1]);
        if (parts[1] == "error")
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        string field = parts[2];
        string val = parts[3];
        UpdateM(UserID, field, val, masterfile);
        return;
    }
    if (CommandName == "update-s")
    {
        if (cnt != 4)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        int ArticleID = StrToInt(parts[1]);
        if (parts[1] == "error")
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        string field = parts[2];
        string val = parts[3];
        UpdateS(ArticleID, field, val, masterfile, slavefile);
        return;
    }
    if (CommandName == "insert-m")
    {
        if (cnt != 3)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        string name = parts[1];
        string pass = parts[2];
        InsertM(name, pass, masterfile);
        return;
    }
    if (CommandName == "insert-s")
    {
        if (cnt != 4)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        int UserID = StrToInt(parts[1]);
        if (parts[1] == "error")
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        string name = parts[2];
        string category = parts[3];
        InsertS(UserID, name, category, masterfile, slavefile);
        return;
    }
    if (CommandName == "calc-m")
    {
        if (cnt != 1)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        cout << "The amount of Users in the table is " << UserCnt << "\n";
        return;
    }
    if (CommandName == "calc-s")
    {
        if (cnt != 2)
        {
            cout << "Error: incorrect command";
            return;
        }
        int UserID = StrToInt(parts[1]);
        if (parts[1] == "error")
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        int cnt = CalcS(UserID, masterfile, slavefile);
        if (cnt == -1) cout << "Error: User with given ID doesn't exist" << "\n";
        else cout << "The amount of Articles written by this User is " << cnt << "\n";
        return;
    }
    if (CommandName == "ut-m")
    {
        if (cnt != 1)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        PrintNodesMaster(masterfile, index_table[0].address, true);
        for (int i = 1; i < index_table.size(); ++i) PrintNodesMaster(masterfile, index_table[i].address, false);
        return;
    }
    if (CommandName == "ut-s")
    {
        if (cnt != 2)
        {
            cout << "Error: incorrect command" << "\n";
            return;
        }
        UtS(parts[1], masterfile, slavefile);
        return;
    }
    if (CommandName == "exit")
    {
        NotFinished = false;
        return;
    }
    cout << "Error: incorrect command" << "\n";
    return;
}
int main()
{
    const std::string slavename = "SP.bin";
    const std::string mastername = "Sfl.bin";
    const std::string tablename = "Sind.bin";
    string command;
    bool NotFinished = true;
    std::fstream slavefile(slavename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    std::fstream masterfile(mastername, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    std::fstream indextable(tablename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    auto err = errno;
    if (err == ENOENT)
    {
        slavefile = std::fstream(slavename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        masterfile = std::fstream(mastername, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        indextable = std::fstream(tablename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    }
    if (!slavefile)
    {
        std::cerr << "Unable to open file=" << slavename << "\n";
        return -1;
    }
    if (!masterfile)
    {
        std::cerr << "Unable to open file=" << mastername << "\n";
        return -1;
    }
    if (!indextable)
    {
        std::cerr << "Unable to open file=" << tablename << "\n";
        return -1;
    }
    while (std::getline(std::cin, command) && NotFinished)
    {
        if (rubbish_m.size() > 3 || rubbish_s.size() > 3)
        {
            Rewrite(masterfile, slavefile);
            rubbish_m.clear();
            rubbish_s.clear();
            RightMasterPos = index_table.size() * static_cast<std::streamoff>(sizeof(User));
            RightSlavePos = TotalArticleCnt * static_cast<std::streamoff>(sizeof(Article));
            std::filesystem::resize_file(mastername, RightMasterPos);
            std::filesystem::resize_file(slavename, RightSlavePos);

        }
        ProcessCommand(command, masterfile, slavefile, NotFinished);
        if (!NotFinished) break;
    }
    cout << "End of the program";
    std::streampos CurPos = 0;
    for (int i = 0; i < index_table.size(); ++i) WriteKey(index_table[i], indextable, CurPos);
    return 0;
}