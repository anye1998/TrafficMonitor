#pragma once

class PluginVersion
{
public:
    PluginVersion();
    PluginVersion(const std::wstring& version);
    PluginVersion(const std::string& version);

    bool operator<(const PluginVersion& another) const;
    bool operator==(const PluginVersion& another) const;

    std::string GetVersionString() const;
    std::wstring GetVersionWString() const;

protected:
    int GetSubVersion(size_t index) const;

private:
    std::vector<int> m_version;
    std::string m_version_str;
};

class CPluginUpdateHelper
{
public:
    //从本仓库和原项目的插件版本清单中各取一次，合并后作为插件的最新版本信息
    bool CheckForUpdate();

    //获取一个插件的最新版本
    const PluginVersion& GetPluginLatestVersions(const std::wstring& file_name) const;

private:
    //解析一份插件版本清单的xml内容，把结果合并到 m_latest_versions 中
    void ParsePluginVersionInfo(const std::string& version_info);

private:
    std::map<std::wstring, PluginVersion> m_latest_versions;    //在线获取到的每个插件的最新版本信息
};

