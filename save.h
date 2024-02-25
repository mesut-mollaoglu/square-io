#ifndef SAVE_H
#define SAVE_H

#include "includes.h"

struct DataNode
{
    DataNode() = default;
    void SetString(const std::string& str, std::size_t index = 0);
    std::string GetString(std::size_t index = 0);
    template <class T> void SetNumber(const T n, std::size_t index = 0)
    {
        if(std::is_arithmetic_v<T>)
            SetString(std::to_string(n), index);
    }
    DataNode& operator[](const std::string& str);
    void SetData(const std::string& str);
    std::string GetData();
    void Clear();
    std::vector<std::string> data;
    std::unordered_map<std::string, DataNode> nodes;
};

inline void Serialize(DataNode& node, const std::string& file)
{
    std::ofstream output(file.c_str(), std::ios::out | std::ios::trunc);
    int tab_count = 0;
    auto indent = [](int count)->std::string
    {
        std::string res;
        for(int i = 0; i < count; i++) res += '\t';
        return res;
    };
    auto write = [&](std::pair<std::string, DataNode> p) -> void
    {
        auto write_node = [&](std::pair<std::string, DataNode> p, auto& write_ref) mutable -> void
        {
            output << indent(tab_count) << "<" << p.first << ">\n";
            if(!p.second.data.empty()) output << indent(tab_count + 1) << "(" << p.second.GetData() << ")" << "\n";
            if(!p.second.nodes.empty())
            {
                tab_count++;
                for(auto& node : p.second.nodes) write_ref(node, write_ref);
                tab_count--;
            }
            output << indent(tab_count) << "</" << p.first << ">\n";
        };
        write_node(p, write_node);
    };
    if(!node.data.empty()) output << indent(tab_count) << "(" << node.GetData() << ")" << "\n";
    for(auto& p : node.nodes) write(p);
    output.close();
}

inline void Deserialize(std::reference_wrapper<DataNode> node, const std::string& file)
{
    std::stack<std::string> name_stack;
    std::stack<std::reference_wrapper<DataNode>> node_stack;
    std::ifstream input(file.c_str());
    node.get().Clear();
    auto trim = [&](const std::string& str) -> std::string
    {
        std::string res;
        for(auto& c : str)
            if(c != '\t' && c != '\n' && c != '\0')
                res += c;
        return res;
    };
    while(!input.eof())
    {
        bool close = false;
        std::string line, buff;
        std::getline(input, line);
        line = trim(line);
        for(int i = 0; i < line.size(); i++)
        {
            if(line[i] == '<')
            {
                ++i;
                if(line[i] == '/')
                {
                    close = true;
                    ++i;
                }
                while(line[i] != '>')
                {
                    buff += line[i];
                    ++i;
                }
                if(!close) 
                {
                    name_stack.push(buff);
                    auto& new_node = node_stack.empty() ? node.get()[buff] : node_stack.top().get()[buff];
                    node_stack.push(new_node);
                }
                else if(name_stack.top() == buff) 
                {
                    node_stack.pop();
                    name_stack.pop();
                }
                buff.clear();
            }
            else if(line[i] == '(')
            {
                ++i;
                while(line[i] != ')')
                {
                    buff += line[i];
                    ++i;
                }
                node_stack.top().get().SetData(buff);
                buff.clear();
            }
        }
    }
}

inline std::vector<std::string> GetProperty(DataNode node, const std::string dir)
{
    std::reference_wrapper<DataNode> datanode = node;
    std::vector<std::string> dir_vec;
    std::string buff;
    auto clear_buffer = [&]()
    {   
        dir_vec.push_back(buff);
        buff.clear();
    };
    for(int i = 0; i < dir.size(); ++i)
    {
        if(dir.substr(i, 2) == "->")
        {
            clear_buffer();
            i+=2;
        }
        else if(i == dir.size() - 1)
        {
            buff += dir[i];
            clear_buffer();
            break;
        }
        buff += dir[i];
    }
    for(int i = 0; i < dir_vec.size(); i++)
    {
        if(datanode.get().nodes.count(dir_vec[i]) != 0)
            datanode = datanode.get()[dir_vec[i]];
        else
            return {"0"};
    }
    return datanode.get().data;
}

inline std::string GetString(DataNode& datanode, const std::string dir, std::size_t index = 0)
{
    return GetProperty(datanode, dir)[index];
}

inline double GetDouble(DataNode& datanode, const std::string dir, std::size_t index = 0)
{
    return std::stod(GetProperty(datanode, dir)[index].c_str());
}

inline int GetInt(DataNode& datanode, const std::string dir, std::size_t index = 0)
{
    return std::stoi(GetProperty(datanode, dir)[index].c_str());
}

#endif

#ifdef SAVE_H
#undef SAVE_H

void DataNode::SetString(const std::string& str, std::size_t index)
{
    if(index >= data.size())
    {
        data.resize(index+1);
    }
    data[index] = str;
}

std::string DataNode::GetString(std::size_t index)
{
    return index < data.size() ? data[index] : "";
}

std::string DataNode::GetData()
{
    std::string res;
    for(int i = 0; i < data.size(); i++)
    {
        res += data[i];
        if(i != data.size() - 1)
            res += ",";
    }
    return res;
}

void DataNode::SetData(const std::string& str)
{
    data.clear();
    std::string buff;
    int index = 0;
    while(index < str.size())
    {
        if(index != str.size() - 1 && str[index] != ',')
            buff += str[index];
        else
        {
            if(index == str.size() - 1)
                buff += str[index];
            data.push_back(buff);
            buff.clear();
        }
        index++;
    }
}

DataNode& DataNode::operator[](const std::string& str)
{
    if(nodes.count(str) == 0)
    {
        nodes[str] = DataNode();
    }
    return nodes[str];
}

void DataNode::Clear()
{
    data.clear();
    if(!nodes.empty()) 
        for(auto& node : nodes)
            node.second.Clear();
    nodes.clear(); 
}

#endif