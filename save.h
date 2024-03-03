#ifndef SAVE_H
#define SAVE_H

#include "includes.h"

const std::string whitespaces = " \n\t\v\0";
const std::string seperator = "-->";

struct DataNode
{
    DataNode() = default;
    void SetString(const std::string& str, std::size_t index = 0);
    std::string GetString(std::size_t index = 0);
    DataNode& operator[](const std::string& str);
    std::vector<std::string> data;
    std::unordered_map<std::string, DataNode> nodes;
};

inline void Clear(std::reference_wrapper<DataNode> datanode)
{
    datanode.get().data.clear();
    if(!datanode.get().nodes.empty())
        for(auto& node : datanode.get().nodes)
            Clear(node.second);
    datanode.get().nodes.clear();
}

inline void SetBool(std::reference_wrapper<DataNode> datanode, const bool b, std::size_t index = 0)
{
    datanode.get().SetString(b ? "true" : "false", index);
}

template <class T> inline void SetNumber(std::reference_wrapper<DataNode> datanode, const T num, std::size_t index = 0)
{
    if(std::is_arithmetic_v<T>) datanode.get().SetString(std::to_string(num), index);
}

inline void SetData(std::reference_wrapper<DataNode> datanode, const std::string str)
{
    datanode.get().data.clear();
    std::string buffer;
    int index = 0;
    while(index < str.size())
    {
        if(index != str.size() - 1 && str[index] != ',')
            buffer += str[index];
        else
        {
            if(index == str.size() - 1) buffer += str[index];
            datanode.get().data.push_back(buffer);
            buffer.clear();
        }
        index++;
    }
}

inline const std::string GetData(DataNode& datanode)
{
    std::string res;
    for(int i = 0; i < datanode.data.size(); i++)
    {
        res += datanode.data[i];
        if(i != datanode.data.size() - 1) res += ",";
    }
    return res;
}

inline void Serialize(DataNode& node, const std::string& file)
{
    std::ofstream output(file.c_str(), std::ios::trunc);
    int tabCount = 0;
    auto Indent = [](int count)->std::string
    {
        std::string res;
        for(int i = 0; i < count; i++) res += '\t';
        return res;
    };
    auto AddBrackets = [](std::string str)->std::string
    {
        if(str.find(whitespaces.c_str(), 0, 1) != std::string::npos) return '[' + str + ']';
        else return str;
    };
    auto Write = [&](std::pair<std::string, DataNode> p) -> void
    {
        auto WriteNode = [&](std::pair<std::string, DataNode> p, auto& WriteRef) mutable -> void
        {
            const std::string name = AddBrackets(p.first) + '>';
            const std::string data = '(' + AddBrackets(GetData(p.second)) + ')';
            if(!p.second.nodes.empty())
            {
                output << Indent(tabCount++) << '<' << name << '\n';
                if(!p.second.data.empty()) output << Indent(tabCount) << data << '\n';
                for(auto& node : p.second.nodes) WriteRef(node, WriteRef);
                output << Indent(--tabCount) << "</" << name << '\n';
            }
            else
            {
                output << Indent(tabCount) << '<' << name << data << "</" << name << '\n';
            }
        };
        WriteNode(p, WriteNode);
    };
    if(!node.data.empty()) output << Indent(tabCount) << "(" << AddBrackets(GetData(node)) << ")" << "\n";
    for(auto& p : node.nodes) Write(p);
    output.close();
}

inline void Deserialize(std::reference_wrapper<DataNode> node, const std::string& file)
{
    Clear(node);
    std::stack<std::pair<std::reference_wrapper<DataNode>, std::string>> stack;

    auto Trim = [&](const std::string& str) -> std::string
    {
        std::string res;
        for(int i = 0; i < str.size(); ++i)
            if(str[i] == '[')
            {
                int end = str.find_first_of(']', ++i);
                res += str.substr(i, end - i);
                i = end;
            }
            else if(whitespaces.find(str[i]) == std::string::npos)
                res += str[i];
        return res;
    };

    std::ifstream input(file.c_str());
    std::stringstream filedata;
    filedata << input.rdbuf();
    filedata.str(Trim(filedata.str()));
    input.close();

    std::string buffer;
    for(int i = 0; i < filedata.str().size(); i++)
    {
        if(filedata.str()[i] == '<')
        {
            bool close = false;
            if(filedata.str()[++i] == '/')
            {
                close = true;
                ++i;
            }
            while(filedata.str()[i] != '>')
            {
                buffer += filedata.str()[i];
                ++i;
            }
            if(!close) 
            {
                std::reference_wrapper<DataNode> newNode = stack.empty() ? node.get()[buffer] : stack.top().first.get()[buffer];
                stack.push(std::make_pair(newNode, buffer));
            }
            else if(stack.top().second == buffer) 
            {
                stack.pop();
            }
            buffer.clear();
        }
        else if(filedata.str()[i] == '(')
        {
            ++i;
            while(filedata.str()[i] != ')')
            {
                buffer += filedata.str()[i];
                ++i;
            }
            SetData(stack.top().first.get(), buffer);
            buffer.clear();
        }
    }
}

inline std::vector<std::string> GetProperty(DataNode& node, const std::string dir)
{
    std::reference_wrapper<DataNode> datanode = node;
    std::vector<std::string> directory;
    std::string buffer;
    auto ClearBuffer = [&]()
    {   
        directory.push_back(buffer);
        buffer.clear();
    };
    for(int i = 0; i < dir.size(); ++i)
    {
        if(dir.substr(i, seperator.size()) == seperator)
        {
            ClearBuffer();
            i+=seperator.size();
        }
        else if(i == dir.size() - 1)
        {
            buffer += dir[i];
            ClearBuffer();
            break;
        }
        buffer += dir[i];
    }
    for(auto& subdir : directory)
    {
        if(datanode.get().nodes.count(subdir) != 0)
            datanode = datanode.get()[subdir];
        else
            return {"0"};
    }
    return datanode.get().data;
}

inline std::string GetString(DataNode& datanode, const std::string dir, std::size_t index = 0)
{
    return GetProperty(datanode, dir)[index];
}

inline bool GetBool(DataNode& datanode, const std::string dir, std::size_t index = 0)
{
    return GetProperty(datanode, dir)[index] == "true";
}

inline double GetDouble(DataNode& datanode, const std::string dir, std::size_t index = 0)
{
    return std::stod(GetProperty(datanode, dir)[index].c_str());
}

inline float GetFloat(DataNode& datanode, const std::string dir, std::size_t index = 0)
{
    return std::stof(GetProperty(datanode, dir)[index].c_str());
}

inline int GetInt(DataNode& datanode, const std::string dir, std::size_t index = 0)
{
    return std::stoi(GetProperty(datanode, dir)[index].c_str());
}

template <typename T, typename ...Args> const std::string GetDirectory(T base, Args... args) 
{
    if constexpr (!sizeof...(args)) 
        return base;
    else
        return base + seperator + GetDirectory(args...);
}

#endif

#ifdef SAVE_H
#undef SAVE_H

void DataNode::SetString(const std::string& str, std::size_t index)
{
    if(index >= data.size()) data.resize(index+1);
    data[index] = str;
}

std::string DataNode::GetString(std::size_t index)
{
    return index < data.size() ? data[index] : "";
}

DataNode& DataNode::operator[](const std::string& str)
{
    if(nodes.count(str) == 0) nodes[str] = DataNode();
    return nodes[str];
}

#endif