#include "stdafx.h"


#include "FilterDlg.inl"
#include "MonpacDlg.inl"
#include "QuickFilter.inl"
#include "StagesDlg.inl"

#include "aboutdlg.inl"
#include "CreateDlg.inl"
#include "NagDlg.inl"
#include "PropertyDlg.inl"
#include "registrationDlg.inl"
#include "SettingsDlg.inl"

namespace Dialogs
{

void Monpac()
{
    Document().Nag();

    CMonpacDialog dlg;
    dlg.DoModal();
}

void FilterHistory()
{
    Document().Nag();

    CFilterDialog dlg;
    dlg.DoModal();
}

void QuickFilter()
{
    Document().Nag();

    CQuickFilter dlg;
    dlg.DoModal();
}

void Stages()
{
    Document().Nag();
   
    CStagesDlg dlg;
    dlg.DoModal();
}

}
