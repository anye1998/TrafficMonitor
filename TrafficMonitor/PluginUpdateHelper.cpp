#include "stdafx.h"
#include "PluginUpdateHelper.h"
#include "Common.h"
#include "TrafficMonitor.h"
#include "TinyXml2Helper.h"

static void VersionFromString(const std::string& version_str, std::vector<int>& versions)
{
    versions.clear();
    //拆分字符串
    std::vector<std::string> vec_version;
    CCommon::StringSplit(version_str, '.', vec_version);
    //转换为整数保存
    for (const auto& str : vec_version)
    {
        //去掉除数字以外的字符
        std::string str_ver;
        for (auto ch : str)
        {
            if (ch >= '0' && ch <= '9')
                str_ver.push_back(ch);
        }

        versions.push_back(atoi(str_ver.c_str()));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
PluginVersion::PluginVersion()
{
}

PluginVersion::PluginVersion(const std::wstring& version)
{
    m_version_str = CCommon::AsciiToStr(version);
    VersionFromString(m_version_str, m_version);
}

PluginVersion::PluginVersion(const std::string& version)
{
    m_version_str = version;
    VersionFromString(m_version_str, m_version);
}

bool PluginVersion::operator<(const PluginVersion& another) const
{
    for (size_t i{}; i < m_version.size() || i < another.m_version.size(); i++)
    {
        const int lhs = GetSubVersion(i);
        const int rhs = another.GetSubVersion(i);

        if (lhs < rhs)
            return true;

        if (lhs > rhs)
            return false;
    }
    return false;
}

bool PluginVersion::operator==(const PluginVersion& another) const
{
    for (size_t i{}; i < m_version.size() || i < another.m_version.size(); i++)
    {
        if (GetSubVersion(i) != another.GetSubVersion(i))
            return false;
    }
    return true;
}

std::string PluginVersion::GetVersionString() const
{
    return m_version_str;
}

std::wstring PluginVersion::GetVersionWString() const
{
    return CCommon::AsciiToUnicode(m_version_str);
}

int PluginVersion::GetSubVersion(size_t index) const
{
    if (index < m_version.size())
        return m_version[index];
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPluginUpdateHelper::CheckForUpdate()
{
    m_latest_versions.clear();

    //同时从两个仓库获取插件版本清单：
    //  1. 本仓库（anye1998/TrafficMonitorPlugins）：本分支维护的插件（例如 MouseStatus）的新版本会先出现在这里；
    //  2. 原项目（zhongyang219/TrafficMonitorPlugins）：其余插件仍然沿用原项目的清单。
    //两个清单里都出现的插件，取版本号较大的那个，因此任一仓库发布新版本都能被检测到。
    const std::wstring plugin_version_urls[] = {
        L"https://raw.githubusercontent.com/anye1998/TrafficMonitorPlugins/main/plugins_version.xml",
        L"https://raw.githubusercontent.com/zhongyang219/TrafficMonitorPlugins/main/plugins_version.xml",
    };

    bool succeed{ false };
    for (const auto& url : plugin_version_urls)
    {
        std::string version_info;
        if (!CCommon::GetURL(url, version_info))       //获取url内容
            continue;
        succeed = true;
        ParsePluginVersionInfo(version_info);
    }
    return succeed;
}

void CPluginUpdateHelper::ParsePluginVersionInfo(const std::string& version_info)
{
    tinyxml2::XMLDocument doc;
    doc.Parse(version_info.c_str(), version_info.size());
    //遍历插件版本
    CTinyXml2Helper::IterateChildNode(doc.RootElement(), [&](tinyxml2::XMLElement* ele) {
        std::wstring file_name = CCommon::StrToUnicode(CTinyXml2Helper::ElementAttribute(ele, "file_name"));
        std::string plugin_version(CTinyXml2Helper::ElementAttribute(ele, "version"));
        PluginVersion version(plugin_version);
        auto iter = m_latest_versions.find(file_name);
        if (iter == m_latest_versions.end())
            m_latest_versions.emplace(file_name, version);
        else if (iter->second < version)        //两个清单里都有时，取版本较大的
            iter->second = version;
    });
}

const PluginVersion& CPluginUpdateHelper::GetPluginLatestVersions(const std::wstring& file_name) const
{
    auto iter = m_latest_versions.find(file_name);
    if (iter != m_latest_versions.end())
    {
        return iter->second;
    }
    static PluginVersion empty_version;
    return empty_version;
}
