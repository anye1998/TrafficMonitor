// ToolTipSettingsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "ToolTipSettingsDlg.h"
#include "Common.h"
#include "PluginManager.h"
#include "FilePathHelper.h"
#include "afxdialogex.h"

//提示类别和对应的复选框控件ID
static const std::pair<unsigned int, UINT> CATEGORY_CTRLS[] = {
    { TC_TODAY_TRAFFIC, IDC_TOOLTIP_TODAY_TRAFFIC_CHK },
    { TC_NET_SPEED, IDC_TOOLTIP_NET_SPEED_CHK },
    { TC_CPU_USAGE, IDC_TOOLTIP_CPU_CHK },
    { TC_MEMORY, IDC_TOOLTIP_MEMORY_CHK },
    { TC_CPU_FREQ, IDC_TOOLTIP_CPU_FREQ_CHK },
    { TC_GPU_USAGE, IDC_TOOLTIP_GPU_CHK },
    { TC_TEMPERATURE, IDC_TOOLTIP_TEMPERATURE_CHK },
    { TC_HDD_USAGE, IDC_TOOLTIP_HDD_CHK },
};

IMPLEMENT_DYNAMIC(CToolTipSettingsDlg, CBaseDialog)

CToolTipSettingsDlg::CToolTipSettingsDlg(const GeneralSettingData& data, CWnd* p_parent /*=nullptr*/)
    : CBaseDialog(IDD_TOOLTIP_SETTINGS_DIALOG, p_parent), m_data{ data }
{
}

CToolTipSettingsDlg::~CToolTipSettingsDlg()
{
}

void CToolTipSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TOOLTIP_MAX_LENGTH_EDIT, m_max_length_edit);
    DDX_Control(pDX, IDC_TOOLTIP_PLUGIN_LIST, m_plugin_list);
}

BEGIN_MESSAGE_MAP(CToolTipSettingsDlg, CBaseDialog)
    ON_BN_CLICKED(IDC_TOOLTIP_PLUGIN_CHK, &CToolTipSettingsDlg::OnBnClickedPluginCheck)
    ON_BN_CLICKED(IDC_TOOLTIP_RESTORE_DEFAULT_BUTTON, &CToolTipSettingsDlg::OnBnClickedRestoreDefaultButton)
END_MESSAGE_MAP()

BOOL CToolTipSettingsDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();

    // TODO:  在此添加额外的初始化
    SetIcon(theApp.GetMenuIcon(IDI_SETTINGS), FALSE);       // 设置小图标

    InitCategoryCheck();
    InitPluginList();

    //初始化提示文本长度上限
    m_max_length_edit.SetRange(static_cast<short>(TOOLTIP_TEXT_LENGTH_MIN), static_cast<short>(TOOLTIP_TEXT_LENGTH_MAX), 256);
    SetMaxLengthCtrl(m_data.tooltip_max_length);

    CheckDlgButton(IDC_TOOLTIP_NOTIFY_TRUNCATED_CHK, m_data.tooltip_notify_truncated);

    SetControlEnable();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}

void CToolTipSettingsDlg::InitCategoryCheck()
{
    for (const auto& item : CATEGORY_CTRLS)
    {
        CheckDlgButton(item.second, m_data.IsToolTipCategoryEnable(item.first) ? TRUE : FALSE);
    }
    CheckDlgButton(IDC_TOOLTIP_PLUGIN_CHK, m_data.IsToolTipCategoryEnable(TC_PLUGIN) ? TRUE : FALSE);
}

void CToolTipSettingsDlg::InitPluginList()
{
    m_plugin_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_LABELTIP);
    CRect rect{};
    m_plugin_list.GetClientRect(&rect);
    int column_width{ rect.Width() - theApp.DPI(4) };
    if (column_width < theApp.DPI(40))
        column_width = theApp.DPI(40);
    m_plugin_list.InsertColumn(0, CCommon::LoadText(IDS_PLUGIN_NAME), LVCFMT_LEFT, column_width);

    int row{ 0 };
    for (const auto& plugin_info : theApp.m_plugins.GetPlugins())
    {
        //只有已成功载入、且接口版本支持鼠标提示的插件才会显示在这里
        if (plugin_info.plugin == nullptr || plugin_info.state != CPluginManager::PluginState::PS_SUCCEED)
            continue;
        if (plugin_info.plugin->GetAPIVersion() < 2)
            continue;

        const std::wstring file_name{ CFilePathHelper(plugin_info.file_path).GetFileName() };
        std::wstring plugin_name{ plugin_info.Property(ITMPlugin::TMI_NAME) };
        if (plugin_name.empty())
            plugin_name = file_name;

        m_plugin_list.InsertItem(row, plugin_name.c_str());
        m_plugin_file_names.push_back(file_name);
        //默认情况下所有插件的提示都是显示的
        m_plugin_list.SetCheck(row, m_data.tooltip_plugin_hide.Contains(file_name) ? FALSE : TRUE);
        ++row;
    }
}

void CToolTipSettingsDlg::SetControlEnable()
{
    //只有勾选了“插件提示”时，插件列表才可用
    bool plugin_enable{ IsDlgButtonChecked(IDC_TOOLTIP_PLUGIN_CHK) != 0 };
    m_plugin_list.EnableWindow(plugin_enable);
    EnableDlgCtrl(IDC_TOOLTIP_PLUGIN_HINT_STATIC, plugin_enable);
    m_plugin_list.Invalidate();
}

void CToolTipSettingsDlg::SetMaxLengthCtrl(int value)
{
    if (value < TOOLTIP_TEXT_LENGTH_MIN)
        value = TOOLTIP_TEXT_LENGTH_MIN;
    if (value > TOOLTIP_TEXT_LENGTH_MAX)
        value = TOOLTIP_TEXT_LENGTH_MAX;
    m_max_length_edit.SetValue(value);      //微调按钮会同时更新编辑框中的文本
    //微调按钮没有创建成功时（例如控件没有被DDX子类化），SetValue不会起作用，这里直接设置文本，
    //保证编辑框中始终能看到当前的数值，避免用户以为设置没有生效
    CString str_now;
    m_max_length_edit.GetWindowText(str_now);
    if (str_now.IsEmpty())
    {
        CString str_value;
        str_value.Format(_T("%d"), value);
        m_max_length_edit.SetWindowText(str_value);
    }
}

int CToolTipSettingsDlg::GetMaxLengthFromCtrl()
{
    CString str;
    m_max_length_edit.GetWindowText(str);
    str.Trim();

    //输入为空或含有非数字字符时，视为无效输入
    if (str.IsEmpty())
        return -1;
    for (int i = 0; i < str.GetLength(); i++)
    {
        if (str[i] < _T('0') || str[i] > _T('9'))
            return -1;
    }

    const int value{ _ttoi(str.GetString()) };
    if (value < TOOLTIP_TEXT_LENGTH_MIN || value > TOOLTIP_TEXT_LENGTH_MAX)
        return -2;      //超出允许的范围

    return value;
}

void CToolTipSettingsDlg::OnBnClickedPluginCheck()
{
    // TODO: 在此添加控件通知处理程序代码
    SetControlEnable();
}

void CToolTipSettingsDlg::OnBnClickedRestoreDefaultButton()
{
    // TODO: 在此添加控件通知处理程序代码
    for (const auto& item : CATEGORY_CTRLS)
        CheckDlgButton(item.second, TRUE);
    CheckDlgButton(IDC_TOOLTIP_PLUGIN_CHK, TRUE);
    CheckDlgButton(IDC_TOOLTIP_NOTIFY_TRUNCATED_CHK, TRUE);
    SetMaxLengthCtrl(TOOLTIP_TEXT_LENGTH_DEFAULT);
    for (int i = 0; i < m_plugin_list.GetItemCount(); i++)
        m_plugin_list.SetCheck(i, TRUE);
    SetControlEnable();
}

void CToolTipSettingsDlg::OnOK()
{
    // TODO: 在此添加专用代码和/或调用基类
    //读取勾选的提示类别
    unsigned int categories{ 0 };
    for (const auto& item : CATEGORY_CTRLS)
    {
        if (IsDlgButtonChecked(item.second) != 0)
            categories |= item.first;
    }
    if (IsDlgButtonChecked(IDC_TOOLTIP_PLUGIN_CHK) != 0)
        categories |= TC_PLUGIN;
    if (categories == 0)        //至少要保留一项，否则鼠标提示中没有任何内容
    {
        MessageBox(CCommon::LoadText(IDS_TOOLTIP_SELECT_AT_LEAST_ONE), nullptr, MB_ICONWARNING | MB_OK);
        return;
    }
    m_data.tooltip_categories = categories;

    //读取不显示提示的插件（保存的是没有勾选的插件）
    m_data.tooltip_plugin_hide.data().clear();
    for (int i = 0; i < static_cast<int>(m_plugin_file_names.size()); i++)
    {
        if (m_plugin_list.GetCheck(i) == FALSE)
            m_data.tooltip_plugin_hide.SetStrContained(m_plugin_file_names[i], true);
    }

    //读取提示文本的长度上限
    //注意：输入无效时不能悄悄按最小值处理，否则用户会以为设置没生效（提示长度总是被限制在最小值）
    const int max_length{ GetMaxLengthFromCtrl() };
    if (max_length < 0)
    {
        MessageBox(CCommon::LoadTextFormat(IDS_TOOLTIP_LENGTH_INVALID, { TOOLTIP_TEXT_LENGTH_MIN, TOOLTIP_TEXT_LENGTH_MAX }),
            nullptr, MB_ICONWARNING | MB_OK);
        m_max_length_edit.SetFocus();
        m_max_length_edit.SetSel(0, -1);
        return;
    }
    m_data.tooltip_max_length = max_length;

    m_data.tooltip_notify_truncated = (IsDlgButtonChecked(IDC_TOOLTIP_NOTIFY_TRUNCATED_CHK) != 0);

    CBaseDialog::OnOK();
}
