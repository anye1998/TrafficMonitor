#include "stdafx.h"
#include "TrafficMonitor.h"
#include "ToolTipHelper.h"
#include "Common.h"
#include "MessageDlg.h"
#include "BaseDialog.h"
#include <map>

bool CToolTipHelper::m_truncated_notified{ false };

//MFC的CToolTipCtrl::UpdateTipText允许的最大字符数（即MAX_TIP_TEXT_LENGTH）
static const int MFC_MAX_TIP_TEXT_LENGTH{ 1024 };

//提示文本被截断后追加的省略号所占用的字符数（用于为省略号预留空间）
static const int TRUNCATED_MARK_RESERVE{ 32 };

//是否已经记录过“提示文本被截断”的日志（每次运行只记录一次，避免刷日志）
static bool s_truncate_logged{ false };

//每个提示框控件在注册工具时使用的TOOLINFO。
//键为提示框控件的窗口句柄；TOOLINFO中的lpszText不使用，只用于定位工具。
static std::map<HWND, TOOLINFO> s_tool_info_cache;

//按照给定的hwnd/uId/uFlags尝试定位提示框中已经注册的工具
static bool GetRegisteredTool(HWND hwnd_tip, HWND hwnd_owner, UINT_PTR u_id, UINT flags, TOOLINFO& result)
{
    TOOLINFO probe{};
    probe.cbSize = sizeof(TOOLINFO);
    probe.hwnd = hwnd_owner;
    probe.uId = u_id;
    probe.uFlags = flags;
    if (::SendMessage(hwnd_tip, TTM_GETTOOLINFO, 0, reinterpret_cast<LPARAM>(&probe)) == FALSE)
        return false;
    probe.lpszText = nullptr;       //不保存指向提示框内部缓冲区的指针
    result = probe;
    return true;
}

//获取提示框控件中与p_wnd对应的工具信息
//优先使用InitToolTip中记录的模板；如果没有（例如别处用CToolTipCtrl::AddTool注册的），
//则按几种常见的注册方式逐一试探，避免因hwnd/uId不匹配而导致TTM_UPDATETIPTEXT静默失败
static bool ResolveToolInfo(HWND hwnd_tip, CWnd* p_wnd, TOOLINFO& result)
{
    if (hwnd_tip == nullptr || p_wnd == nullptr)
        return false;

    const HWND hwnd_wnd{ p_wnd->GetSafeHwnd() };
    CWnd* p_parent{ p_wnd->GetParent() };
    const HWND hwnd_parent{ p_parent != nullptr ? p_parent->GetSafeHwnd() : nullptr };

    auto it{ s_tool_info_cache.find(hwnd_tip) };
    if (it != s_tool_info_cache.end())
    {
        //确认缓存的模板仍然有效（提示框被重建后句柄会变化，理论上不会命中旧的缓存）
        if (GetRegisteredTool(hwnd_tip, it->second.hwnd, it->second.uId, it->second.uFlags, result))
            return true;
        s_tool_info_cache.erase(it);        //缓存失效，重新探测
    }

    struct Candidate
    {
        HWND hwnd;
        UINT_PTR u_id;
        UINT flags;
    };
    //候选1：MFC的CToolTipCtrl::AddTool(CWnd*, ...)采用的方式（hwnd为父窗口，uId为窗口句柄）
    //候选2：以窗口自身作为hwnd
    //候选3：以子控件ID的方式注册（uId为控件ID，hwnd为窗口）
    const Candidate candidates[] = {
        { hwnd_parent, reinterpret_cast<UINT_PTR>(hwnd_wnd), TTF_IDISHWND },
        { hwnd_wnd, reinterpret_cast<UINT_PTR>(hwnd_wnd), TTF_IDISHWND },
        { hwnd_wnd, 0, 0 },
    };
    for (const auto& candidate : candidates)
    {
        if (candidate.hwnd == nullptr && candidate.u_id == 0)
            continue;
        if (GetRegisteredTool(hwnd_tip, candidate.hwnd, candidate.u_id, candidate.flags, result))
        {
            s_tool_info_cache[hwnd_tip] = result;
            return true;
        }
    }
    return false;
}

//按照字符数上限和行数上限从text的开头截取可以显示的部分（按整行截取）
//返回值：是否发生了截断
static bool CutLines(const CString& text, int total_len, int budget, int max_lines, CString& result)
{
    result.Empty();
    int pos{ 0 };
    int line_count{ 0 };
    while (pos < total_len)
    {
        //取出一行（不含行尾的换行符）
        const int line_end{ text.Find(_T("\r\n"), pos) };
        int line_len;
        int next_pos;
        if (line_end < 0)
        {
            line_len = total_len - pos;
            next_pos = total_len;
        }
        else
        {
            line_len = line_end - pos;
            next_pos = line_end + 2;
        }

        //这一行加上它前面的换行符后所占用的字符数
        const int append_len{ line_len + (line_count == 0 ? 0 : 2) };
        if (line_count + 1 > max_lines || result.GetLength() + append_len > budget)
            break;      //超出了行数或字符数的限制，从这里开始截断

        if (line_count > 0)
            result += _T("\r\n");
        result += text.Mid(pos, line_len);
        ++line_count;
        pos = next_pos;
    }
    return pos < total_len;
}

int CToolTipHelper::GetMaxTextLength()
{
    int max_length{ theApp.m_general_data.tooltip_max_length };
    if (max_length < TOOLTIP_TEXT_LENGTH_MIN)
        max_length = TOOLTIP_TEXT_LENGTH_MIN;
    if (max_length > TOOLTIP_TEXT_LENGTH_MAX)
        max_length = TOOLTIP_TEXT_LENGTH_MAX;
    return max_length;
}

int CToolTipHelper::GetMaxVisibleLines(CWnd* p_wnd)
{
    //提示框的高度不能超过屏幕的工作区，因此它能够显示的行数也是有限的
    int work_height{ 1080 };
    MONITORINFO monitor_info{};
    monitor_info.cbSize = sizeof(MONITORINFO);
    HMONITOR h_monitor{ nullptr };
    if (p_wnd != nullptr && ::IsWindow(p_wnd->GetSafeHwnd()))
        h_monitor = ::MonitorFromWindow(p_wnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
    if (h_monitor != nullptr && ::GetMonitorInfo(h_monitor, &monitor_info))
        work_height = monitor_info.rcWork.bottom - monitor_info.rcWork.top;

    //一行文字的高度按照96DPI下16像素估算（系统默认的提示字体大约是9磅），
    //同时留出提示框的边框和上下边距所占的高度
    int line_height{ theApp.DPI(16) };
    if (line_height < 12)
        line_height = 12;
    int max_lines{ (work_height - theApp.DPI(56)) / line_height };
    if (max_lines < 8)
        max_lines = 8;
    if (max_lines > 120)
        max_lines = 120;
    return max_lines;
}

bool CToolTipHelper::IsCategoryEnabled(unsigned int category)
{
    return theApp.m_general_data.IsToolTipCategoryEnable(category);
}

bool CToolTipHelper::IsPluginEnabled(const std::wstring& file_name)
{
    if (!IsCategoryEnabled(TC_PLUGIN))      //所有的插件提示都被隐藏了
        return false;
    return !theApp.m_general_data.tooltip_plugin_hide.Contains(file_name);
}

CString CToolTipHelper::LimitText(const CString& text, int max_length, int max_lines, bool* truncated, int* omitted_chars)
{
    if (truncated != nullptr)
        *truncated = false;
    if (omitted_chars != nullptr)
        *omitted_chars = 0;

    if (max_length < TOOLTIP_TEXT_LENGTH_MIN)
        max_length = TOOLTIP_TEXT_LENGTH_MIN;
    if (max_length > TOOLTIP_TEXT_LENGTH_MAX)
        max_length = TOOLTIP_TEXT_LENGTH_MAX;
    if (max_lines < 1)
        max_lines = 1;

    const int total_len{ text.GetLength() };

    //先按照完整的字符数上限截取。如果没有超出限制则原样返回
    CString result;         //保留下来、可以显示的部分
    if (!CutLines(text, total_len, max_length, max_lines, result))
        return text;

    //发生了截断，为末尾的省略号预留出位置后重新截取一次
    const int reserved_budget{ max_length - TRUNCATED_MARK_RESERVE };
    if (!CutLines(text, total_len, reserved_budget > 0 ? reserved_budget : 1, max_lines, result) || result.IsEmpty())
    {
        //第一行就超出了限制，只能按照字符数硬截断
        int cut{ reserved_budget > 0 ? reserved_budget : 1 };
        if (cut > total_len)
            cut = total_len;
        result = text.Left(cut);
    }
    //去掉末尾可能残留的换行符
    while (!result.IsEmpty() && (result[result.GetLength() - 1] == _T('\r') || result[result.GetLength() - 1] == _T('\n')))
        result.Delete(result.GetLength() - 1);

    //用省略号代替被截断的部分
    const int omitted{ total_len - result.GetLength() };
    result += _T("\r\n");
    result += CCommon::LoadTextFormat(IDS_TOOLTIP_TRUNCATED_MARK, { omitted });

    if (truncated != nullptr)
        *truncated = true;
    if (omitted_chars != nullptr)
        *omitted_chars = omitted;

    return result;
}

CString CToolTipHelper::UpdateTipText(CToolTipCtrl& tool_tip, CWnd* p_wnd, const CString& text)
{
    if (p_wnd == nullptr || !::IsWindow(p_wnd->GetSafeHwnd()) || !::IsWindow(tool_tip.GetSafeHwnd()))
        return text;

    bool truncated{ false };
    int omitted_chars{ 0 };
    CString limited_text{ LimitText(text, GetMaxTextLength(), GetMaxVisibleLines(p_wnd), &truncated, &omitted_chars) };

    const HWND hwnd_tip{ tool_tip.GetSafeHwnd() };
    TOOLINFO tool_info{};
    if (ResolveToolInfo(hwnd_tip, p_wnd, tool_info))
    {
        //注意：这里不能调用CToolTipCtrl::UpdateTipText，因为MFC在这个函数中调用了
        //ENSURE_ARG(AtlStrLen(lpszText) <= MAX_TIP_TEXT_LENGTH)检查了文本的长度，
        //而MAX_TIP_TEXT_LENGTH的值为1024。文本超过这个长度时会抛出CInvalidArgException，
        //该异常从窗口过程中逃出来以后，MFC会弹出一个内容为“遇到不适当的参数。”的对话框。
        //这里直接向提示框控件发送TTM_UPDATETIPTEXT消息，以绕过这一限制。
        //（这一段代码与CToolTipCtrl::UpdateTipText的实现等价，只是去掉了长度检查）
        tool_info.lpszText = const_cast<LPTSTR>(limited_text.GetString());
        ::SendMessage(hwnd_tip, TTM_UPDATETIPTEXT, 0, reinterpret_cast<LPARAM>(&tool_info));
    }
    else
    {
        //没有找到已经注册的工具（正常情况下不会发生）。退回到MFC的实现，
        //但要先把文本限制在MFC允许的长度以内，否则又会抛出CInvalidArgException
        if (limited_text.GetLength() > MFC_MAX_TIP_TEXT_LENGTH)
            limited_text = limited_text.Left(MFC_MAX_TIP_TEXT_LENGTH) + _T("…");
        tool_tip.UpdateTipText(limited_text, p_wnd);
    }

    //只在第一次发生截断时记录一条日志，方便排查“提示长度上限没有按设置生效”之类的问题
    if (truncated && !s_truncate_logged)
    {
        s_truncate_logged = true;
        CString log_info;
        log_info.Format(_T("鼠标提示文本被截断：原始长度 %d 个字符，上限 %d 个字符，省略 %d 个字符。"),
            text.GetLength(), GetMaxTextLength(), omitted_chars);
        CCommon::WriteLog(log_info, theApp.m_log_path.c_str());
    }

    if (truncated)
        NotifyTruncated(GetMaxTextLength(), omitted_chars);

    return limited_text;
}

void CToolTipHelper::InitToolTip(CToolTipCtrl& tool_tip, CWnd* p_wnd)
{
    if (p_wnd == nullptr || !::IsWindow(p_wnd->GetSafeHwnd()) || !::IsWindow(tool_tip.GetSafeHwnd()))
        return;

    //注册工具这一步仍然交给MFC，保证提示框的显示、置顶、跟随等行为与改动前完全一致
    tool_tip.AddTool(p_wnd, _T(""));

    //注册完成后立刻探测并记住这次注册使用的TOOLINFO，
    //这样后面发送TTM_UPDATETIPTEXT时才能精确匹配到这个工具
    TOOLINFO tool_info{};
    if (ResolveToolInfo(tool_tip.GetSafeHwnd(), p_wnd, tool_info))
        s_tool_info_cache[tool_tip.GetSafeHwnd()] = tool_info;
}


void CToolTipHelper::ResetTruncatedNotification()
{
    m_truncated_notified = false;
}

void CToolTipHelper::NotifyTruncated(int max_length, int omitted_chars)
{
    if (m_truncated_notified)       //每次程序运行只提示一次
        return;
    if (!theApp.m_general_data.tooltip_notify_truncated)
        return;
    //如果有对话框正在显示（例如用户正在“选项设置”中修改提示内容），则先不提示，
    //等到对话框关闭后的下一次刷新提示时再判断，避免提示框被挡在后面
    if (!CBaseDialog::IsAllDialogClosed())
        return;

    //先将标志置为true，这样即使在显示提示对话框时又触发了提示框的刷新，也不会再次弹出提示
    m_truncated_notified = true;

    CMessageDlg dlg(AfxGetMainWnd());
    dlg.SetWindowTitle(CCommon::LoadText(IDS_TOOLTIP_TRUNCATED_TITLE));
    dlg.SetInfoText(CCommon::LoadText(IDS_TOOLTIP_TRUNCATED_INFO));
    dlg.SetMessageText(CCommon::LoadTextFormat(IDS_TOOLTIP_TRUNCATED_MESSAGE, { max_length, omitted_chars }));
    dlg.SetStandarnMessageIcon(CMessageDlg::SI_INFORMATION);
    dlg.DoModal();
}
