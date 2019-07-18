#include <db/MySQL/MySQLConnectionFactory.h>
#include <db/IDbConnection.h>
#include <db_controls/DbGrid.h>
#include <db_controls/DbComboBox.h>
#include <db_controls/DbComboBoxCellWidget.h>
#include <db_controls/BinderControls.h>
#include <db_controls/FilteringEdit.h>
#include <db_controls/DbNavigator.h>
#include <xwindows/HorizontalSizer.h>
#include <xwindows/VerticalSizer.h>
#include <xwindows/XLabel.h>
#include "AdvocatsBook.h"
#include "AdvocatsGridEvtHandler.h"
#include "PostIndexCellWidget.h"

CAdvocatsBook::CAdvocatsBook(const Tchar *class_name, \
								const Tchar *label, const int X, const int Y, \
								const int width, const int height) : \
	flt_id(nullptr), btn_apply_filter(nullptr), btn_add(nullptr), btn_remove(nullptr), \
	grid(nullptr), examiners_list(nullptr), adv_org_types_list(nullptr), districts_list(nullptr), \
	grid_x(0), grid_y(0), grid_margin_x(0), grid_margin_y(0) {

	props.open("config.ini");

	initFilteringControls();

	conn = CMySQLConnectionFactory::createConnection(props);
	db_table = createDbTable(conn);
	
	grid = new CDbGrid(db_table, std::make_shared<CAdvocatsGridEvtHandler>(db_table));
	setFieldsSizes();
	createCellWidgetsAndAttachToGrid(grid);

	Create(class_name, FL_WINDOW_VISIBLE | FL_WINDOW_CLIPCHILDREN, \
			label, X, Y, width, height);
	DisplayWidgets();
	adjustUIDependentCellWidgets(grid);

	Connect(EVT_SIZE, this, &CAdvocatsBook::OnSize);
	Connect(EVT_COMMAND, btn_apply_filter->GetId(), this, &CAdvocatsBook::OnFilterButtonClick);
	Connect(EVT_COMMAND, btn_add->GetId(), this, &CAdvocatsBook::OnAddRecordButtonClick);
	Connect(EVT_COMMAND, btn_remove->GetId(), this, &CAdvocatsBook::OnRemoveButtonClick);
}
	
std::shared_ptr<CDbTable> CAdvocatsBook::createDbTable(std::shared_ptr<IDbConnection> conn) {

	std::string query = "SELECT b.id_advocat, b.adv_name, b.license_no, b.license_date,";
	query += " e.exm_name, b.post_index, b.address, b.adv_bdate,";
	query += " d.distr_center, b.org_name, b.org_type ";
	query += "FROM advocats b INNER JOIN examiners e ON b.id_exm = e.id_examiner";
	query += " INNER JOIN districts d ON b.id_main_district = d.id_distr";
	query += " #### ";
	query += "ORDER BY adv_name";
	query_modifier.Init(query);

	auto stmt = conn->PrepareQuery(query_modifier.getQuery().c_str());

	auto db_table = std::make_shared<CDbTable>(conn, CQuery(conn, stmt));
	db_table->setPrimaryTableForQuery("advocats");
	db_table->markFieldAsPrimaryKey("id_advocat");

	return db_table;
}

void CAdvocatsBook::setFieldsSizes() {

	grid->SetFieldWidth(0, 4);
	grid->SetFieldLabel(0, _T("ID"));
	grid->SetFieldWidth(1, 37);
	grid->SetFieldLabel(1, _T("ϲ� ��������"));
	grid->SetFieldWidth(2, 10);
	grid->SetFieldLabel(2, _T("� ��."));
	grid->SetFieldWidth(4, 70);
	grid->SetFieldLabel(4, _T("�����, ���� ����� ��������"));
	grid->SetFieldWidth(5, 6);
	grid->SetFieldWidth(6, 75);
	grid->SetFieldWidth(8, 25);
	grid->SetFieldWidth(9, 55);
}

void CAdvocatsBook::createCellWidgetsAndAttachToGrid(CDbGrid *grid) {

	assert(grid);
	assert(!examiners_list);
	assert(!districts_list);
	assert(!adv_org_types_list);

	try {
		examiners_list = new CDbComboBoxCellWidget(conn, "exm_name", \
			"examiners", "advocats", db_table);
		examiners_list->AddRelation("id_examiner", "id_exm");
		grid->SetWidgetForFieldByName("exm_name", examiners_list);

		auto post_index_widget = new CPostIndexCellWidget();
		grid->SetWidgetForFieldByName("post_index", post_index_widget);

		districts_list = new CDbComboBoxCellWidget(conn, "distr_center", \
			"districts", "advocats", db_table);
		districts_list->AddRelation("id_distr", "id_main_district");
		grid->SetWidgetForFieldByName("distr_center", districts_list);

		adv_org_types_list = new CComboBoxCellWidget();
		grid->SetWidgetForFieldByName("org_type", adv_org_types_list);
	}
	catch (...) {
		if (examiners_list && !examiners_list->IsCreated()) delete examiners_list;
		if (districts_list && !districts_list->IsCreated()) delete districts_list;
		if (adv_org_types_list && !adv_org_types_list->IsCreated())
			delete adv_org_types_list;

		throw;
	}
}

void CAdvocatsBook::adjustUIDependentCellWidgets(CDbGrid *grid) {

	assert(adv_org_types_list);
	adv_org_types_list->AddItem(_T("���"));
	adv_org_types_list->AddItem(_T("��"));
	adv_org_types_list->AddItem(_T("��"));
}

void CAdvocatsBook::DisplayWidgets() {

	XRect rc;
	const int edit_flags = FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED | \
							FL_WINDOW_CLIPSIBLINGS | FL_EDIT_AUTOHSCROLL;

	GetClientRect(rc);
	CVerticalSizer main_sizer(this, 0, 0, rc.right, rc.bottom, \
								10, 10, 10, 10, \
								DEF_GUI_VERT_GAP, DEF_GUI_ROW_HEIGHT);

	CHorizontalSizer sizer(CSizerPreferences(0, 0, 0, 0, DEF_GUI_HORZ_GAP));
	main_sizer.pushNestedSizer(sizer);
		sizer.addWidget(new XLabel(), _T("ID: "), FL_WINDOW_VISIBLE, \
						XSize(20, DEF_GUI_ROW_HEIGHT));
		adv_inserter.setIdAdvocatWidget(flt_id);
		sizer.addWidget(flt_id, _T(""), FL_WINDOW_VISIBLE, \
						XSize(45, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("ϲ�: "), FL_WINDOW_VISIBLE, \
						XSize(30, DEF_GUI_ROW_HEIGHT));
		XEdit *edit_adv_name = new XEdit();
		adv_inserter.setAdvNameWidget(edit_adv_name);
		sizer.addWidget(edit_adv_name, _T(""), edit_flags, XSize(280, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("� ���.: "), FL_WINDOW_VISIBLE, \
						XSize(40, DEF_GUI_ROW_HEIGHT + 10));
		XEdit *license_no = new XEdit();
		adv_inserter.setLicenseNoWidget(license_no);
		sizer.addWidget(license_no, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("���� ���.: "), FL_WINDOW_VISIBLE, \
						XSize(40, DEF_GUI_ROW_HEIGHT + 10));
		XEdit *license_date = new XEdit();
		adv_inserter.setLicenseDateWidget(license_date);
		sizer.addWidget(license_date, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(80, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("��� �����:"), FL_WINDOW_VISIBLE, \
						XSize(45, DEF_GUI_ROW_HEIGHT + 10));
		CDbComboBox *examiner = new CDbComboBox(examiners_list->getMasterResultSet(), 1, 0);
		adv_inserter.setExaminerWidget(examiner);
		sizer.addResizeableWidget(examiner, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(250, DEF_GUI_ROW_HEIGHT), 150);
	main_sizer.popNestedSizer();

	main_sizer.pushNestedSizer(sizer);
		sizer.addWidget(new XLabel(), _T("����. ������:"), FL_WINDOW_VISIBLE, \
						XSize(50, DEF_GUI_ROW_HEIGHT + 10));
		XEdit *post_index = new XEdit();
		adv_inserter.setPostIndexWidget(post_index);
		sizer.addWidget(post_index, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(70, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("������:"), FL_WINDOW_VISIBLE, \
						XSize(53, DEF_GUI_ROW_HEIGHT));
		XEdit *address = new XEdit();
		adv_inserter.setAddressWidget(address);
		sizer.addWidget(address, _T(""), edit_flags, XSize(380, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("���� ������:"), FL_WINDOW_VISIBLE, \
						XSize(55, DEF_GUI_ROW_HEIGHT + 10));
		XEdit *bdate = new XEdit();
		adv_inserter.setBDateWidget(bdate);
		sizer.addWidget(bdate, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("���.:"), FL_WINDOW_VISIBLE, \
						XSize(40, DEF_GUI_ROW_HEIGHT));
		XEdit *tel = new XEdit();
		adv_inserter.setTelWidget(tel);
		sizer.addWidget(tel, _T(""), edit_flags, XSize(200, DEF_GUI_ROW_HEIGHT));
	main_sizer.popNestedSizer();

	main_sizer.pushNestedSizer(sizer);
		sizer.addWidget(new XLabel(), _T("E-mail:"), FL_WINDOW_VISIBLE, \
						XSize(50, DEF_GUI_ROW_HEIGHT));
		XEdit *email = new XEdit();
		adv_inserter.setEmailWidget(email);
		sizer.addWidget(email, _T(""), edit_flags, XSize(200, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("����� ������:"), FL_WINDOW_VISIBLE, \
						XSize(55, DEF_GUI_ROW_HEIGHT + 10));
		CDbComboBox *distr = new CDbComboBox(districts_list->getMasterResultSet(), 2, 0);
		adv_inserter.setDistrictWidget(distr);
		sizer.addResizeableWidget(distr, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
									XSize(190, DEF_GUI_ROW_HEIGHT), 150);

		sizer.addWidget(new XLabel(), _T("����� ���.:"), FL_WINDOW_VISIBLE, \
						XSize(45, DEF_GUI_ROW_HEIGHT + 10));
		XEdit *org = new XEdit();
		adv_inserter.setOrgNameWidget(org);
		sizer.addWidget(org, _T(""), edit_flags, XSize(200, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("��� ���.:"), FL_WINDOW_VISIBLE, \
						XSize(40, DEF_GUI_ROW_HEIGHT + 10));
		XComboBox *org_type = new XComboBox();
		adv_inserter.setOrgTypeWidget(org_type);
		sizer.addResizeableWidget(org_type, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED | \
									FL_COMBOBOX_DROPDOWN, XSize(50, DEF_GUI_ROW_HEIGHT), 150);
		org_type->AddItem(_T("���"));
		org_type->AddItem(_T("��"));
		org_type->AddItem(_T("��"));
	main_sizer.popNestedSizer();

	main_sizer.pushNestedSizer(sizer);
		btn_apply_filter = new XButton();
		sizer.addWidget(btn_apply_filter, _T("Գ����"), FL_WINDOW_VISIBLE, \
						XSize(100, DEF_GUI_ROW_HEIGHT));

		auto db_navigator = new CDbNavigator(db_table);
		sizer.addWidget(db_navigator, _T(""), FL_WINDOW_VISIBLE, \
						XSize(100, DEF_GUI_ROW_HEIGHT));

		btn_add = new XButton();
		sizer.addWidget(btn_add, _T("+"), FL_WINDOW_VISIBLE, \
						XSize(30, DEF_GUI_ROW_HEIGHT));

		btn_remove = new XButton();
		sizer.addWidget(btn_remove, _T("-"), FL_WINDOW_VISIBLE, \
						XSize(30, DEF_GUI_ROW_HEIGHT));
	main_sizer.popNestedSizer();

	XRect grid_coords = main_sizer.addLastWidget(grid);
	grid->SetFocus();

	grid_x = grid_coords.left;
	grid_y = grid_coords.top;

	XRect margins = main_sizer.getMargins();
	grid_margin_x = margins.left;
	grid_margin_y = margins.top;

	adv_inserter.prepare(conn);
}

void CAdvocatsBook::initFilteringControls() {

	flt_id = new CFilteringEdit(filtering_manager);
	std::shared_ptr<IBinder> id_binder = std::make_shared<CIntWidgetBinderControl>(flt_id);
	
	ImmutableString<char> expr("b.id_advocat = ?", sizeof("b.id_advocat = ?") - 1);
	int id_expr = filtering_manager.addExpr(expr, id_binder);
	flt_id->setExprId(id_expr);
}

void CAdvocatsBook::OnFilterButtonClick(XCommandEvent *eve) {

	if (filtering_manager.isWherePartChanged()) {

		query_modifier.changeWherePart(filtering_manager.getSQLWherePart());
		
		auto stmt = conn->PrepareQuery(query_modifier.getQuery().c_str());
		filtering_manager.apply(stmt);
		db_table->reload(stmt);
	}
	else {

		filtering_manager.apply(db_table->getBindingTarget());
		db_table->reload();
	}
}

void CAdvocatsBook::OnAddRecordButtonClick(XCommandEvent *eve) {

	adv_inserter.insert();
	db_table->reload();
}

void CAdvocatsBook::OnRemoveButtonClick(XCommandEvent *eve) {

	int option = _plMessageBoxYesNo(_T("�������� �������� �����?"));
	if(option == IDYES)
		db_table->removeCurrentRecord();
}

void CAdvocatsBook::OnSize(XSizeEvent *eve) {

	int width = eve->GetWidth();
	int height = eve->GetHeight();

	if (!(grid && width && height)) return;
		
	grid->MoveWindow(grid_x, grid_y, \
					width - 2 * grid_margin_x, \
					height - grid_y - grid_margin_y);
}

CAdvocatsBook::~CAdvocatsBook() {

	if (grid && !grid->IsCreated()) delete grid;
	if (flt_id && !flt_id->IsCreated()) delete flt_id;
}