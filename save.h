#ifndef SAVE_H
#define SAVE_H

#include "includes.h"

const std::string whitespaces = " \n\t\v\0";
const std::string seperator = "->";

struct DataNode
{
    DataNode() = default;
    template <class T> void SetNumber(const T& data, std::size_t index = 0)
    {
        if(std::is_arithmetic_v<T>) SetString(std::to_string(data), index);
    }
    std::optional<std::reference_wrapper<DataNode>> GetProperty(std::string dir);
    void SetString(const std::string& str, std::size_t index = 0);
    void SetBool(const bool data, std::size_t index = 0);
    void SetData(const std::string str);
    const std::string GetData() const;
    void ClearData();
    DataNode& operator[](const std::string& str);
    std::vector<std::string> data;
    std::unordered_map<std::string, DataNode> nodes;
};

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
            const std::string data = '(' + AddBrackets(p.second.GetData()) + ')';
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
    if(!node.data.empty()) output << Indent(tabCount) << "(" << AddBrackets(node.GetData()) << ")" << "\n";
    for(auto& p : node.nodes) Write(p);
    output.close();
}

inline void Deserialize(std::reference_wrapper<DataNode> node, const std::string& file)
{
    node.get().ClearData();
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
        else if(filedata.str()[i++] == '(')
        {
            while(filedata.str()[i] != ')')
            {
                buffer += filedata.str()[i];
                ++i;
            }
            stack.top().first.get().SetData(buffer);
            buffer.clear();
        }
    }
}

inline std::optional<std::string> GetString(std::optional<DataNode> datanode, std::size_t index = 0)
{
    if(!datanode.has_value() || index >= datanode.value().data.size())
        return {};
    else
        return datanode.value().data[index];
}

inline std::optional<bool> GetBool(std::optional<DataNode> datanode, std::size_t index = 0)
{
    std::optional<std::string> str = GetString(datanode, index);
    if(str.has_value())
        return str.value() == "true";
    else 
        return {};
}

inline std::optional<double> GetDouble(std::optional<DataNode> datanode, std::size_t index = 0)
{
    std::optional<std::string> str = GetString(datanode, index);
    if(str.has_value())
        return std::stod(str.value().c_str());
    else 
        return {};
}

inline std::optional<float> GetFloat(std::optional<DataNode> datanode, std::size_t index = 0)
{
    std::optional<std::string> str = GetString(datanode, index);
    if(str.has_value())
        return std::stof(str.value().c_str());
    else 
        return {};
}

inline std::optional<int> GetInt(std::optional<DataNode> datanode, std::size_t index = 0)
{
    std::optional<std::string> str = GetString(datanode, index);
    if(str.has_value())
        return std::stoi(str.value().c_str());
    else 
        return {};
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

DataNode& DataNode::operator[](const std::string& str)
{
    if(nodes.count(str) == 0) nodes[str] = DataNode();
    return nodes[str];
}

std::optional<std::reference_wrapper<DataNode>> DataNode::GetProperty(std::string dir)
{
    std::reference_wrapper<DataNode> datanode = std::ref(*this);
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
            return {};
    }
    return datanode;
}

void DataNode::ClearData()
{
    data.clear();
    if(!nodes.empty())
        for(auto& node : nodes)
            node.second.ClearData();
    nodes.clear();
}

void DataNode::SetBool(const bool data, std::size_t index)
{
    SetString(data ? "true" : "false", index);
}

void DataNode::SetData(const std::string str)
{
    data.clear();
    std::string buffer;
    int index = 0;
    while(index < str.size())
    {
        if(index != str.size() - 1 && str[index] != ',')
            buffer += str[index];
        else
        {
            if(index == str.size() - 1) buffer += str[index];
            data.push_back(buffer);
            buffer.clear();
        }
        index++;
    }
}

const std::string DataNode::GetData() const
{
    std::string res;
    for(int i = 0; i < data.size(); i++)
    {
        res += data[i];
        if(i != data.size() - 1) res += ",";
    }
    return res;
}

#endif