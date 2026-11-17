#pragma once
#include "CommonData.h"

//鼠标提示文本的处理
//用于替代CToolTipCtrl::UpdateTipText，以实现以下目的：
//  1. 绕过MFC在CToolTipCtrl::UpdateTipText中对文本长度的限制
//     （MFC中该限制为MAX_TIP_TEXT_LENGTH，即1024个字符，超出时会抛出CInvalidArgException，
//      并弹出一个内容为“遇到不适当的参数。”的对话框）；
//  2. 在文本超长时保留可以显示的内容，并用省略号替代被截断的部分；
//  3. 文本被截断时只提示一次，避免每秒刷新时反复弹出对话框。
class CToolTipHelper
{
public:
    //获取当前配置的字符数上限（做完有效性检查后的值）
    static int GetMaxTextLength();

    //获取提示框在当前显示器上一屏最多能够显示的行数
    static int GetMaxVisibleLines(CWnd* p_wnd);

    //判断一个鼠标提示的内容类别是否启用
    static bool IsCategoryEnabled(unsigned int category);

    //判断一个插件的提示是否启用。file_name为插件dll的文件名（含扩展名）
    static bool IsPluginEnabled(const std::wstring& file_name);

    //按照字符数上限和行数上限截断提示文本，被截断的部分用省略号代替
    //max_length: 允许的最大字符数
    //max_lines: 允许的最大行数
    //truncated[out]: 是否发生了截断
    //omitted_chars[out]: 被省略的字符数
    //返回值: 处理后的文本（如果没有超出限制则原样返回）
    static CString LimitText(const CString& text, int max_length, int max_lines, bool* truncated = nullptr, int* omitted_chars = nullptr);

    //创建好提示框控件后调用，向提示框注册要显示提示的窗口
    //不使用CToolTipCtrl::AddTool，是为了自己记录注册时使用的TOOLINFO（hwnd/uId），
    //这样后面发送TTM_UPDATETIPTEXT时能够精确匹配到已经注册的工具，
    //否则消息会静默失败，表现为鼠标提示完全不显示
    static void InitToolTip(CToolTipCtrl& tool_tip, CWnd* p_wnd);

    //约束提示框的位置，使它始终完整显示在显示器的工作区内（工作区已排除任务栏）。
    //系统默认的提示框只保证不超出显示器范围，当窗口贴在屏幕底部（例如任务栏窗口）时，
    //提示框的下半部分会落到任务栏后面而被挡住，因此需要把提示框向上推回来
    static void InstallPositionClamp(CToolTipCtrl& tool_tip, CWnd* p_wnd);

    //更新鼠标提示的文本，返回实际显示的文本
    //注意：这里不使用CToolTipCtrl::UpdateTipText，因为它限制了文本长度不能超过1024个字符
    static CString UpdateTipText(CToolTipCtrl& tool_tip, CWnd* p_wnd, const CString& text);

    //重置“已经提示过文本被截断”的标志，使下一次超长时还会弹出提示
    static void ResetTruncatedNotification();

private:
    //文本被截断时的提示（每次程序运行只显示一次）
    static void NotifyTruncated(int max_length, int omitted_chars);

    static bool m_truncated_notified;   //是否已经显示过“提示文本被截断”的提示
};
