#include "AdvocatsListEvtHandler.h"
#include <xwindows/platform_specific.h>

const Tchar msg[] = _T("��� �������� �� ������ ���� ����� ����� �� �����");

CAdvocatsListEvtHandler::CAdvocatsListEvtHandler(std::shared_ptr<CDbTable> db_table) : \
													CDbGridEventsHandler(db_table) { }

void CAdvocatsListEvtHandler::OnCellChanged(IGridCellWidget *cell_widget, \
											IOnCellChangedAction &action) {

	WarningBox(msg);
}

void CAdvocatsListEvtHandler::OnCellChangedIndirectly(IGridCellWidget *cell_widget, \
														IOnCellChangedAction &action) {

	WarningBox(msg);
}

CAdvocatsListEvtHandler::~CAdvocatsListEvtHandler() { }
