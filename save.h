#ifndef SAVE_H
#define SAVE_H

#include "includes.h"

const std::string whitespaces = " \n\t\v\0";
const std::string seperator = "->";

template <typename T> inline std::optional<T> convert(const std::optional<std::string> str) {}
template <> inline std::optional<double> convert<double>(const std::optional<std::string> str) 
{return str.has_value() ? std::make_optional(std::stod(str.value().c_str())) : std::nullopt;}
template <> inline std::optional<float> convert<float>(const std::optional<std::string> str) 
{return str.has_value() ? std::make_optional(std::stof(str.value().c_str())) : std::nullopt;}
template <> inline std::optional<int> convert<int>(const std::optional<std::string> str) 
{return str.has_value() ? std::make_optional(std::stoi(str.value().c_str())) : std::nullopt;}
template <> inline std::optional<bool> convert<bool>(const std::optional<std::string> str)
{
    if(str.has_value())
    {
        if(str.value() == "true") return true;
        else if(str.value() == "false") return false;
        else return std::nullopt;
    }
    return std::nullopt;
}

template <typename T> inline std::string convert(const T& data) {static_assert(std::is_arithmetic_v<T>); return std::to_string(data);}
template <> inline std::string convert<bool>(const bool& data) {return data ? "true" : "false";}

struct Container
{
    std::string content;
    std::optional<std::string> name = std::nullopt;
};

auto ParseDirectory = [](const std::string dir)->std::vector<std::string>
{
    std::vector<std::string> directory;
    std::string buffer;
    auto ClearBuffer = [&]()
    {   
        directory.push_back(buffer);
        buffer.clear();
    };
    std::size_t index = 0, next = dir.find_first_of(seperator, index);
    while(index < dir.size() && next != std::string::npos)
    {
        directory.push_back(dir.substr(index, next - index));
        index = next + seperator.size();
        next = dir.find_first_of(seperator, index);
    }
    if(next == std::string::npos)
        directory.push_back(dir.substr(index, dir.size() - index));
    return directory;    
};

struct DataNode
{
    DataNode() = default;
    template <class DataType, class ArgType> inline void SetData(const DataType& data, ArgType arg)
    {
        static_assert(std::is_arithmetic_v<DataType> || std::is_same_v<DataType, bool>);
        static_assert(std::is_convertible_v<ArgType, std::string> || std::is_integral_v<ArgType>);
        SetString(convert<DataType>(data), arg);
    }
    std::optional<std::reference_wrapper<DataNode>> GetProperty(const std::string dir);
    void SetString(const std::string& str, std::size_t index = 0);
    void SetString(const std::string& str, std::string name = "");
    bool HasProperty(const std::string dir);
    void SetData(const std::string str);
    const std::string GetData() const;
    void ClearData();
    DataNode& operator[](const std::string& str);
    std::vector<Container> data;
    std::unordered_map<std::string, DataNode> nodes;
};

inline void Serialize(DataNode& node, const std::string& file)
{
    std::ofstream output(file.c_str(), std::ios::trunc);
    int tabCount = 0;
    auto Indent = [](int count)->std::string
    {
        std::string res;
        for(int index = 0; index < count; index++) res += '\t';
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
            const std::string data = '{' + AddBrackets(p.second.GetData()) + '}';
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
    if(!node.data.empty()) output << Indent(tabCount) << '{' << AddBrackets(node.GetData()) << '}' << '\n';
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
        for(std::size_t index = 0; index < str.size(); ++index)
            if(str[index] == '[')
            {
                std::size_t end = str.find_first_of(']', ++index);
                res += str.substr(index, end - index);
                index = end;
            }
            else if(whitespaces.find(str[index]) == std::string::npos)
                res += str[index];
        return res;
    };

    std::ifstream input(file.c_str());
    std::stringstream content;
    content << input.rdbuf();
    content.str(Trim(content.str()));
    input.close();

    std::string buffer;
    for(int index = 0; index < content.str().size(); index++)
    {
        if(content.str()[index] == '<')
        {
            std::size_t end = content.str().find_first_of('>', ++index);
            bool close = content.str()[index] == '/';
            index += (close ? 1 : 0);
            buffer += content.str().substr(index, end - index);
            index = end;
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
        else if(content.str()[index] == '{')
        {
            std::size_t end = content.str().find_first_of('}', ++index);
            buffer += content.str().substr(index, end - index);
            stack.top().first.get().SetData(buffer);
            buffer.clear();
        }
    }
}

inline std::optional<std::string> GetString(std::optional<DataNode> datanode, std::size_t index = 0)
{
#if defined NO_COLLISIONS
    for(std::size_t i = 0, count = 0; i < datanode.value().data.size(); i++)
        if(!datanode.value().data[i].name.has_value())
            if(count++ == index)
                return datanode.value().data[i].content;
    return {};
#else
    if(!datanode.has_value() || (datanode.has_value() && index >= datanode.value().data.size()))
        return {};
    else
        return datanode.value().data[index].content;
#endif
}

inline std::optional<std::string> GetString(std::optional<DataNode> datanode, std::string name = "")
{
    if(!datanode.has_value())
        return {};
    if(!datanode.value().data.empty())
        for(auto& element : datanode.value().data)
            if(element.name.has_value() && element.name.value() == name)
                return element.content;
    return {};
}

template <class DataType, class ArgType> inline std::optional<DataType> GetData(std::optional<DataNode> datanode, ArgType arg)
{
    static_assert(std::is_arithmetic_v<DataType> || std::is_same_v<DataType, bool>);
    static_assert(std::is_convertible_v<ArgType, std::string> || std::is_integral_v<ArgType>);
    return convert<DataType>(GetString(datanode, arg));
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
#if defined NO_COLLISIONS
    if(index < data.size())
        for(std::size_t i = 0, count = 0; i < data.size(); i++)
            if(!data[i].name.has_value())
                if(count++ == index)
                {
                    data[i].content = str;
                    return;
                }
    data.resize(index + 1);
    data[index].content = str;
#else
    if(index >= data.size()) data.resize(index + 1);
    data[index].content = str;
#endif
}

void DataNode::SetString(const std::string& str, std::string name)
{
    if(!data.empty())
        for(auto& element : data)
            if(element.name.has_value() && element.name.value() == name)
            {
                element.content = str;
                return;   
            }
    data.push_back(Container{str, name.empty() ? std::nullopt : std::make_optional(name)});
}

DataNode& DataNode::operator[](const std::string& str)
{
    if(nodes.count(str) == 0) nodes[str] = DataNode();
    return nodes[str];
}

std::optional<std::reference_wrapper<DataNode>> DataNode::GetProperty(const std::string dir)
{
    std::reference_wrapper<DataNode> datanode = *this;
    for(auto& subdir : ParseDirectory(dir))
    {
        if(datanode.get().nodes.count(subdir) != 0)
            datanode = datanode.get()[subdir];
        else
            return {};
    }
    return datanode;
}

bool DataNode::HasProperty(const std::string dir)
{
    std::reference_wrapper<DataNode> datanode = *this;
    for(auto& subdir : ParseDirectory(dir))
    {
        if(datanode.get().nodes.count(subdir) == 0)
            return false;
        else
            datanode = datanode.get()[subdir];
    }
    return true;
}

void DataNode::ClearData()
{
    data.clear();
    if(!nodes.empty())
        for(auto& node : nodes)
            node.second.ClearData();
    nodes.clear();
}

void DataNode::SetData(const std::string str)
{
    data.clear();
    int index = 0;
    Container buffer;
    while(index < str.size())
    {
        if(str[index] == '(')
        {
            std::size_t end = str.find_first_of(')', ++index);
            if(end == std::string::npos)
                continue;
            else
            {
                buffer.name = str.substr(index, end - index);
                index = end;
            }
        }
        else if(index == str.size() - 1 || str[index] == ',')
        {
            if(index == str.size() - 1) buffer.content += str[index];
            data.push_back(buffer);
            buffer.content.clear();
            buffer.name.reset();
        }
        else
            buffer.content += str[index];
        index++;
    }
}

const std::string DataNode::GetData() const
{
    std::string res;
    for(int index = 0; index < data.size(); index++)
    {
        if(data[index].name.has_value())
            res += '(' + data[index].name.value() + ')';
        res += data[index].content;
        if(index != data.size() - 1) res += ",";
    }
    return res;
}

#endif