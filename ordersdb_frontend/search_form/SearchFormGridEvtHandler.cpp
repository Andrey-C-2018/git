#include "SearchFormGridEvtHandler.h"
#include <xwindows/platform_specific.h>
#include <forms_common/Constraints.h>

void CSearchFormGridEvtHandler::OnSearchFormCellChanged(IGridCellWidget *cell_widget, \
												IOnCellChangedAction &action) const {

	if (constraints->wrong_zone) {
		ErrorBox(_T("��������� ������ �� �����, ������� ���� �������� �� ������ ������"));
		return;
	}
	else if (constraints->old_stage_locked) {

		auto p_field = ordersCellChanged();
		if (p_field != orders_fields_indexes.cend()) {
			if (constraints->old_order_locked && !p_field->change_allowed) {
				ErrorBox(_T("��������� ������ �� ���������, ������� ���� ���� ������ ����� ���� ����"));
				return;
			}
		}
		else {
			ErrorBox(_T("��������� ������ �� �����, ������� ���� �������� �� �������� ������"));
			return;
		}
	}

	ProceedWithChanging(cell_widget, action);
}

void CSearchFormGridEvtHandler::OnCellChanged(IGridCellWidget *cell_widget, \
											IOnCellChangedAction &action) {

	OnSearchFormCellChanged(cell_widget, action);
}

void CSearchFormGridEvtHandler::OnCellChangedIndirectly(IGridCellWidget *cell_widget, \
													IOnCellChangedAction &action) {

	OnSearchFormCellChanged(cell_widget, action);
}

CSearchFormGridEvtHandler::~CSearchFormGridEvtHandler() { }
