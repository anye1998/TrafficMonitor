#pragma once
#include "BaseDialog.h"
#include "SpinEdit.h"
#include "CommonData.h"

//鼠标提示内容设置对话框
//用于设置鼠标提示中要显示哪些内容，以及提示文本的长度上限
//这些设置对主窗口和任务栏窗口的鼠标提示同时生效
class CToolTipSettingsDlg : public CBaseDialog
{
    DECLARE_DYNAMIC(CToolTipSettingsDlg)

public:
    CToolTipSettingsDlg(const GeneralSettingData& data, CWnd* p_parent = nullptr);
    virtual ~CToolTipSettingsDlg();

    //获取修改后的设置（只包含鼠标提示相关的设置）
    const GeneralSettingData& GetData() const { return m_data; }

    enum { IDD = IDD_TOOLTIP_SETTINGS_DIALOG };

protected:
    GeneralSettingData m_data;              //保存正在编辑的设置
    CSpinEdit m_max_length_edit;            //提示文本最大字符数的编辑框
    CListCtrl m_plugin_list;                //插件列表
    std::vector<std::wstring> m_plugin_file_names;      //插件列表中每一项对应的插件文件名

    void InitCategoryCheck();
    void InitPluginList();
    void SetControlEnable();

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedPluginCheck();
    afx_msg void OnBnClickedRestoreDefaultButton();
    virtual void OnOK();
};
