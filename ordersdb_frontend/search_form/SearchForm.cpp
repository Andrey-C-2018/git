#include <basic/XConv.h>
#include <db/MySQL/MySQLConnectionFactory.h>
#include <db/IDbConnection.h>
#include <db_ext/DbTable.h>
#include <db_ext/DependentTableUpdater.h>
#include <xwindows/XLabel.h>
#include <xwindows/XButton.h>
#include <xwindows_ex/HorizontalSizer.h>
#include <xwindows_ex/VerticalSizer.h>
#include <db_controls/DbGrid.h>
#include <db_controls/DbComboBoxCellWidget.h>
#include <db_controls/FilteringEdit.h>
#include <db_controls/FilteringDbComboBox.h>
#include <db_controls/FilteringDateField.h>
#include <db_controls/BinderControls.h>
#include <db_controls/DbNavigator.h>
#include "SearchForm.h"
#include "ZoneFilter.h"
#include "PaidFilter.h"

class CGridCellWidgetCreator final {
	CDbGrid *grid;
	IGridCellWidget *cell_widget;
	bool allocated_n_attached;
public:
	CGridCellWidgetCreator(CDbGrid *grid_) : grid(grid_), \
					cell_widget(nullptr), allocated_n_attached(false) { }

	template <typename CellWidget, typename ... Args> \
		CellWidget *createAndAttachToGrid(const char *field_name, \
											Args ... args) {

			allocated_n_attached = false;
			this->cell_widget = nullptr;

			CellWidget *cell_widget = new CellWidget(args...);
			this->cell_widget = cell_widget;
			grid->SetWidgetForFieldByName(field_name, cell_widget);
			
			allocated_n_attached = true;
			return cell_widget;
		}

	~CGridCellWidgetCreator() {
	
		if (!allocated_n_attached && cell_widget)
			delete cell_widget;
	}
};

//*****************************************************

CSearchForm::CSearchForm(XWindow *parent, const int flags, \
					const Tchar *label, \
					const int X, const int Y, \
					const int width, const int height) : \
				flt_id(nullptr), flt_act(nullptr), flt_order_date_from(nullptr), \
				flt_order_date_to(nullptr), flt_act_reg_date_from(nullptr), \
				flt_act_reg_date_to(nullptr), flt_act_date_from(nullptr), \
				flt_act_date_to(nullptr), flt_payment_date_from(nullptr), \
				flt_payment_date_to(nullptr), flt_center(nullptr), flt_informer(nullptr), \
				flt_order_type(nullptr), flt_stage(nullptr), flt_zone(nullptr), \
				grid(nullptr), advocats_list(nullptr), \
				centers_list(nullptr), order_types_list(nullptr), stages_list(nullptr), \
				canceling_reasons_list(nullptr), \
				grid_x(0), grid_y(0), grid_margin_x(0), grid_margin_y(0) {

	props.open("config.ini");
	params_manager.Init(&props);

	conn = CMySQLConnectionFactory::createConnection(props);
	db_table = createDbTable(conn);

	grid = new CDbGrid(false, db_table);
	setFieldsSizes();
	createCellWidgetsAndAttachToGrid(grid);
	initFilteringControls();

	Create(parent, FL_WINDOW_VISIBLE | FL_WINDOW_CLIPCHILDREN, \
			label, X, Y, width, height);
	displayWidgets();
	adjustUIDependentCellWidgets();
	loadInitialFilterToControls();

	Connect(EVT_COMMAND, btn_apply_filter->GetId(), this, &CSearchForm::OnFilterButtonClick);
	Connect(EVT_COMMAND, btn_add->GetId(), this, &CSearchForm::OnAddRecordButtonClick);
	Connect(EVT_COMMAND, btn_remove->GetId(), this, &CSearchForm::OnRemoveButtonClick);
}

void CSearchForm::setFieldsSizes() {

	grid->SetFieldWidth(0, 2);
	grid->SetFieldLabel(0, _T("��"));
	grid->SetFieldWidth(1, 12);
	grid->SetFieldLabel(1, _T("�����"));
	grid->SetFieldWidth(2, 17);
	grid->SetFieldLabel(2, _T("�������"));
	grid->SetFieldWidth(3, 5);
	grid->SetFieldLabel(3, _T("�"));
	grid->SetFieldLabel(4, _T("����"));
	grid->SetFieldWidth(5, 6);
	grid->SetFieldLabel(5, _T("���"));
	grid->SetFieldWidth(6, 30);
	grid->SetFieldLabel(6, _T("�볺��"));
	grid->SetFieldLabel(7, _T("���� ���."));
	grid->SetFieldWidth(8, 10);
	grid->SetFieldLabel(8, _T("����"));
	grid->SetFieldWidth(9, 17);
	grid->SetFieldLabel(9, _T("���./���."));
	grid->SetFieldWidth(10, 9);
	grid->SetFieldLabel(10, _T("�����"));
	grid->SetFieldLabel(11, _T("���� ���."));
	grid->SetFieldLabel(12, _T("����"));
	grid->SetFieldLabel(13, _T("�������"));
	grid->SetFieldWidth(14, 15);
	grid->SetFieldLabel(14, _T("���"));
	grid->SetFieldLabel(15, _T("�� ������"));
	grid->SetFieldLabel(16, _T("�� ����"));
	grid->SetFieldLabel(17, _T("�� �. ����"));
	grid->SetFieldLabel(18, _T("���� ���."));
	grid->SetFieldWidth(19, 1);
	grid->SetFieldLabel(19, _T("#"));
	grid->SetFieldWidth(20, 30);
	grid->SetFieldLabel(20, _T("������"));
	grid->SetFieldWidth(21, 32);
	grid->SetFieldLabel(21, _T("����������"));
}

void CSearchForm::createCellWidgetsAndAttachToGrid(CDbGrid *grid) {

	assert(grid);
	assert(!advocats_list);
	assert(!centers_list);
	CGridCellWidgetCreator creator(grid);
	
	auto stmt = conn->PrepareQuery("SELECT id_advocat, adv_name_short, adv_name FROM advocats ORDER BY 2");
	auto result_set = stmt->exec();
	auto rs_metadata = stmt->getResultSetMetadata();
	advocats_list = creator.createAndAttachToGrid<CDbComboBoxCellWidget>(\
										"adv_name_short", \
										conn, 1, result_set, rs_metadata, \
										"orders", db_table);
	advocats_list->AddRelation("id_advocat", "id_adv");

	centers_list = creator.createAndAttachToGrid<CDbComboBoxCellWidget>(\
										"center_short_name", \
										conn, "center_short_name", "centers", \
										"orders", db_table);
	centers_list->AddRelation("id_center", "id_center_legalaid");

	canceling_reasons_list = new CComboBoxCellWidget();
	grid->SetWidgetForFieldByName("reason", canceling_reasons_list);

	stmt = conn->PrepareQuery("SELECT id_inf, informer_name, informer_full_name FROM informers ORDER BY 2");
	result_set = stmt->exec();
	rs_metadata = stmt->getResultSetMetadata();
	informers_list = creator.createAndAttachToGrid<CDbComboBoxCellWidget>(\
										"informer_name", \
										conn, 1, result_set, rs_metadata, \
										"payments", db_table);
	informers_list->AddRelation("id_inf", "id_informer");

	order_types_list = creator.createAndAttachToGrid<CDbComboBoxCellWidget>(\
										"type_name", \
										conn, "type_name", "order_types", \
										"orders", db_table);
	order_types_list->AddRelation("id_type", "id_order_type");

	stmt = conn->PrepareQuery("SELECT id_st, stage_name FROM stages ORDER BY 2");
	result_set = stmt->exec();
	rs_metadata = stmt->getResultSetMetadata();
	stages_list = creator.createAndAttachToGrid<CDbComboBoxCellWidget>(\
										"stage_name", \
										conn, 1, result_set, rs_metadata, \
										"payments", db_table);
	stages_list->AddRelation("id_st", "id_stage");
}

void CSearchForm::adjustUIDependentCellWidgets() {

	assert(canceling_reasons_list);

	canceling_reasons_list->AddItem(_T("���������"));
	canceling_reasons_list->AddItem(_T("���������"));
	canceling_reasons_list->AddItem(_T("�����"));
	canceling_reasons_list->AddItem(_T("������"));
	canceling_reasons_list->AddItem(_T("���������"));
	canceling_reasons_list->AddItem(_T("�������"));
	canceling_reasons_list->AddItem(_T("������"));
	canceling_reasons_list->AddItem(_T("���������� � ������"));
	canceling_reasons_list->AddItem(_T("���������"));
}

void CSearchForm::loadInitialFilterToControls() {

	flt_center->SetCurrRecord(params_manager.getDefaultCenter());
	//flt_order_date_to->SetLabel(params_manager.getInitialDate());
	assert(filtering_manager.isFilteringStringChanged());
}

void CSearchForm::initFilteringControls() {

	flt_center = new CFilteringDbComboBox(filtering_manager, \
										centers_list->getMasterResultSet(), 2, 0);
	auto combo_binder = std::make_shared<CDbComboBoxBinderControl>(flt_center);
	int id_expr = filtering_manager.addExpr("a.id_center_legalaid = ?", combo_binder);
	flt_center->setExprId(id_expr);

	flt_id = new CFilteringEdit(filtering_manager, this);
	std::shared_ptr<IBinder> id_binder = std::make_shared<CIntWidgetBinderControl>(flt_id);
	ImmutableString<char> expr("a.id = ?", sizeof("a.id = ?") - 1);
	id_expr = filtering_manager.addExpr(expr, id_binder);
	flt_id->setExprId(id_expr);

	flt_order_date_from = new CFilteringDateField(filtering_manager, this);
	auto date_binder = std::make_shared<CDateWidgetBinderControl>(flt_order_date_from);
	id_expr = filtering_manager.addExpr("a.order_date >= ?", date_binder);
	flt_order_date_from->setExprId(id_expr);

	flt_order_date_to = new CFilteringDateField(filtering_manager, this);
	date_binder = std::make_shared<CDateWidgetBinderControl>(flt_order_date_to);
	id_expr = filtering_manager.addExpr("a.order_date <= ?", date_binder);
	flt_order_date_to->setExprId(id_expr);

	flt_order_type = new CFilteringDbComboBox(filtering_manager, \
										order_types_list->getMasterResultSet(), 2, 0);
	combo_binder = std::make_shared<CDbComboBoxBinderControl>(flt_order_type);
	id_expr = filtering_manager.addExpr("a.id_order_type = ?", combo_binder);
	flt_order_type->setExprId(id_expr);

	flt_advocat = new CFilteringDbComboBox(filtering_manager, \
											advocats_list->getMasterResultSet(), 2, 0);
	combo_binder = std::make_shared<CDbComboBoxBinderControl>(flt_advocat);
	id_expr = filtering_manager.addExpr("a.id_adv = ?", combo_binder);
	flt_advocat->setExprId(id_expr);

	flt_zone = new CZoneFilter(filtering_manager);

	flt_act = new CFilteringEdit(filtering_manager, this);
	auto str_binder = std::make_shared<CStringWidgetBinderControl>(flt_act);
	id_expr = filtering_manager.addExpr("aa.id_act = ?", str_binder);
	flt_act->setExprId(id_expr);

	flt_informer = new CFilteringDbComboBox(filtering_manager, \
											informers_list->getMasterResultSet(), 2, 0);
	combo_binder = std::make_shared<CDbComboBoxBinderControl>(flt_informer);
	id_expr = filtering_manager.addExpr("aa.id_informer = ?", combo_binder);
	flt_informer->setExprId(id_expr);

	flt_stage = new CFilteringDbComboBox(filtering_manager, \
											stages_list->getMasterResultSet(), 1, 0);
	combo_binder = std::make_shared<CDbComboBoxBinderControl>(flt_stage);
	id_expr = filtering_manager.addExpr("aa.id_stage = ?", combo_binder);
	flt_stage->setExprId(id_expr);

	flt_act_reg_date_from = new CFilteringDateField(filtering_manager, this);
	date_binder = std::make_shared<CDateWidgetBinderControl>(flt_act_reg_date_from);
	id_expr = filtering_manager.addExpr("aa.act_reg_date >= ?", date_binder);
	flt_act_reg_date_from->setExprId(id_expr);

	flt_act_reg_date_to = new CFilteringDateField(filtering_manager, this);
	date_binder = std::make_shared<CDateWidgetBinderControl>(flt_act_reg_date_to);
	id_expr = filtering_manager.addExpr("aa.act_reg_date <= ?", date_binder);
	flt_act_reg_date_to->setExprId(id_expr);

	flt_act_date_from = new CFilteringDateField(filtering_manager, this);
	date_binder = std::make_shared<CDateWidgetBinderControl>(flt_act_date_from);
	id_expr = filtering_manager.addExpr("aa.act_date >= ?", date_binder);
	flt_act_date_from->setExprId(id_expr);

	flt_act_date_to = new CFilteringDateField(filtering_manager, this);
	date_binder = std::make_shared<CDateWidgetBinderControl>(flt_act_date_to);
	id_expr = filtering_manager.addExpr("aa.act_date <= ?", date_binder);
	flt_act_date_to->setExprId(id_expr);

	flt_payment_date_from = new CFilteringDateField(filtering_manager, this);
	date_binder = std::make_shared<CDateWidgetBinderControl>(flt_payment_date_from);
	id_expr = filtering_manager.addExpr("aa.payment_date >= ?", date_binder);
	flt_payment_date_from->setExprId(id_expr);

	flt_payment_date_to = new CFilteringDateField(filtering_manager, this);
	date_binder = std::make_shared<CDateWidgetBinderControl>(flt_payment_date_to);
	id_expr = filtering_manager.addExpr("aa.payment_date <= ?", date_binder);
	flt_payment_date_to->setExprId(id_expr);

	flt_paid = new CPaidFilter(filtering_manager);
}

std::shared_ptr<CDbTable> CSearchForm::createDbTable(std::shared_ptr<IDbConnection> conn) {

	std::string query = "SELECT a.zone, c.center_short_name, b.adv_name_short, a.id, a.order_date,";
	query += " t.type_name, a.client_name, a.bdate, sta.stage_name, a.reason, a.cancel_order, a.cancel_date, aa.fee, aa.outgoings,";
	query += " aa.id_act, aa.act_reg_date, aa.act_date, aa.bank_reg_date, aa.payment_date,";
	query += " aa.cycle, aa.article, inf.informer_name, aa.id_stage,";
	query += " a.id_center_legalaid, a.id_adv, a.id_order_type, aa.id_informer ";
	query += "FROM orders a INNER JOIN payments aa ON a.id_center_legalaid = aa.id_center AND a.id = aa.id_order AND a.order_date = aa.order_date";
	query += " INNER JOIN advocats b ON a.id_adv = b.id_advocat";
	query += " INNER JOIN order_types t ON a.id_order_type = t.id_type";
	query += " INNER JOIN informers inf ON aa.id_informer = inf.id_inf";
	query += " INNER JOIN centers c ON a.id_center_legalaid = c.id_center";
	query += " INNER JOIN stages sta ON aa.id_stage = sta.id_st";
	query += " #### ";
	query += " ORDER BY a.id_center_legalaid,a.order_date,a.id,aa.cycle,aa.id_stage";
	query_modifier.Init(query);

	std::string initial_flt = "a.id_center_legalaid = ";
	char buffer[10];
	XConv::ToString(params_manager.getDefaultCenter(), buffer);
	initial_flt += buffer;
	//initial_flt += " AND a.order_date = '";
	query_modifier.changeWherePart(\
		ImmutableString<char>(initial_flt.c_str(), initial_flt.size()));

	auto stmt = conn->PrepareQuery(query_modifier.getQuery().c_str());

	auto db_table = std::make_shared<CDbTable>(conn, CQuery(conn, stmt));
	db_table->setPrimaryTableForQuery("payments");
	db_table->markFieldAsPrimaryKey("id_center_legalaid", "orders");
	db_table->markFieldAsPrimaryKey("id", "orders");
	db_table->markFieldAsPrimaryKey("order_date", "orders");

	db_table->addPrimaryKeyAsRef("id_center", "payments", \
									"id_center_legalaid", "orders");
	db_table->addPrimaryKeyAsRef("id_order", "payments", \
									"id", "orders");
	db_table->addPrimaryKeyAsRef("order_date", "payments", \
									"order_date", "orders");
	db_table->markFieldAsPrimaryKey("id_stage", "payments");
	db_table->markFieldAsPrimaryKey("cycle", "payments");
	
	return db_table;
}

void CSearchForm::displayWidgets() {

	XRect rc;
	const int edit_flags = FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED | \
		FL_WINDOW_CLIPSIBLINGS | FL_EDIT_AUTOHSCROLL;

	GetClientRect(rc);
	CVerticalSizer main_sizer(this, 0, 0, rc.right, rc.bottom, \
								10, 10, 10, 10, \
								DEF_GUI_VERT_GAP, DEF_GUI_ROW_HEIGHT);

	CHorizontalSizer sizer(CSizerPreferences(0, 0, 0, 0, DEF_GUI_HORZ_GAP));
	main_sizer.pushNestedSizer(sizer);
		sizer.addWidget(new XLabel(), _T("�������: "), FL_WINDOW_VISIBLE, \
						XSize(60, DEF_GUI_ROW_HEIGHT));
		sizer.addResizeableWidget(flt_advocat, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(250, DEF_GUI_ROW_HEIGHT), 250);
		flt_advocat->setTabStopManager(this);

		sizer.addWidget(new XLabel(), _T("�����: "), FL_WINDOW_VISIBLE, \
						XSize(50, DEF_GUI_ROW_HEIGHT));
		sizer.addResizeableWidget(flt_center, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(180, DEF_GUI_ROW_HEIGHT), 200);
		flt_center->setTabStopManager(this);

		sizer.addWidget(new XLabel(), _T("����������: "), FL_WINDOW_VISIBLE, \
						XSize(100, DEF_GUI_ROW_HEIGHT));
		sizer.addResizeableWidget(flt_informer, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(250, DEF_GUI_ROW_HEIGHT), 250);
		flt_informer->setTabStopManager(this);
	main_sizer.popNestedSizer();

	main_sizer.pushNestedSizer(sizer);
		sizer.addWidget(new XLabel(), _T("� ���.: "), FL_WINDOW_VISIBLE, \
						XSize(60, DEF_GUI_ROW_HEIGHT));
		sizer.addWidget(flt_id, _T(""), FL_WINDOW_VISIBLE, \
						XSize(45, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("���: "), FL_WINDOW_VISIBLE, \
						XSize(35, DEF_GUI_ROW_HEIGHT));
		sizer.addResizeableWidget(flt_order_type, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(100, DEF_GUI_ROW_HEIGHT), 250);
		flt_order_type->setTabStopManager(this);

		sizer.addWidget(new XLabel(), _T("���� �: "), FL_WINDOW_VISIBLE, \
						XSize(50, DEF_GUI_ROW_HEIGHT + 10));
		sizer.addWidget(flt_order_date_from, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("���� ��: "), FL_WINDOW_VISIBLE, \
						XSize(60, DEF_GUI_ROW_HEIGHT + 10));
		sizer.addWidget(flt_order_date_to, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("����:"), FL_WINDOW_VISIBLE, \
						XSize(40, DEF_GUI_ROW_HEIGHT));
		XTabStopEdit *article = new XTabStopEdit(this);
		sizer.addWidget(article, _T(""), edit_flags, XSize(320, DEF_GUI_ROW_HEIGHT));
	main_sizer.popNestedSizer();

	main_sizer.pushNestedSizer(sizer);
		sizer.addWidget(new XLabel(), _T("� ����: "), FL_WINDOW_VISIBLE, \
						XSize(60, DEF_GUI_ROW_HEIGHT));
		sizer.addWidget(flt_act, _T(""), FL_WINDOW_VISIBLE, \
						XSize(90, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("����: "), FL_WINDOW_VISIBLE, \
						XSize(40, DEF_GUI_ROW_HEIGHT));
		sizer.addResizeableWidget(flt_stage, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(110, DEF_GUI_ROW_HEIGHT), 250);
		flt_stage->setTabStopManager(this);

		sizer.addWidget(new XLabel(), _T("� ��������:"), FL_WINDOW_VISIBLE, \
						XSize(90, DEF_GUI_ROW_HEIGHT));
		XTabStopEdit *cycle = new XTabStopEdit(this);
		sizer.addWidget(cycle, _T(""), edit_flags, XSize(30, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("����: "), FL_WINDOW_VISIBLE, \
						XSize(45, DEF_GUI_ROW_HEIGHT));
		XTabStopEdit *fee = new XTabStopEdit(this);
		sizer.addWidget(fee, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(90, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("�������: "), FL_WINDOW_VISIBLE, \
						XSize(60, DEF_GUI_ROW_HEIGHT));
		XTabStopEdit *outgoings = new XTabStopEdit(this);
		sizer.addWidget(outgoings, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(90, DEF_GUI_ROW_HEIGHT));
	main_sizer.popNestedSizer();

	main_sizer.pushNestedSizer(sizer);
		sizer.addWidget(new XLabel(), _T("���� ������. �: "), FL_WINDOW_VISIBLE, \
						XSize(70, DEF_GUI_ROW_HEIGHT + 10));
		sizer.addWidget(flt_act_reg_date_from, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));
	
		sizer.addWidget(new XLabel(), _T("���� ������. ��: "), FL_WINDOW_VISIBLE, \
						XSize(80, DEF_GUI_ROW_HEIGHT + 10));
		sizer.addWidget(flt_act_reg_date_to, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));
	
		sizer.addWidget(new XLabel(), _T("���� ���� �: "), FL_WINDOW_VISIBLE, \
						XSize(50, DEF_GUI_ROW_HEIGHT + 10));
		sizer.addWidget(flt_act_date_from, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("���� ���� ��: "), FL_WINDOW_VISIBLE, \
						XSize(65, DEF_GUI_ROW_HEIGHT + 10));
		sizer.addWidget(flt_act_date_to, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("���� ������ �: "), FL_WINDOW_VISIBLE, \
						XSize(70, DEF_GUI_ROW_HEIGHT + 10));
		sizer.addWidget(flt_payment_date_from, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("���� ������ ��: "), FL_WINDOW_VISIBLE, \
						XSize(75, DEF_GUI_ROW_HEIGHT + 10));
		sizer.addWidget(flt_payment_date_to, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));
	main_sizer.popNestedSizer();

	main_sizer.pushNestedSizer(sizer);
		sizer.addWidget(new XLabel(), _T("�볺��:"), FL_WINDOW_VISIBLE, \
						XSize(60, DEF_GUI_ROW_HEIGHT));
		XTabStopEdit *client_name = new XTabStopEdit(this);
		sizer.addWidget(client_name, _T(""), edit_flags, XSize(320, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("���� ������:"), FL_WINDOW_VISIBLE, \
						XSize(55, DEF_GUI_ROW_HEIGHT + 10));
		XDateField *bdate = new XDateField(this);
		sizer.addWidget(bdate, _T(""), edit_flags, XSize(80, DEF_GUI_ROW_HEIGHT));

		sizer.addWidget(new XLabel(), _T("���� ���.:"), FL_WINDOW_VISIBLE, \
						XSize(85, DEF_GUI_ROW_HEIGHT + 10));
		sizer.addResizeableWidget(flt_zone, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(50, DEF_GUI_ROW_HEIGHT), 100);

		sizer.addWidget(new XLabel(), _T("��������:"), FL_WINDOW_VISIBLE, \
						XSize(85, DEF_GUI_ROW_HEIGHT + 10));
		sizer.addResizeableWidget(flt_paid, _T(""), FL_WINDOW_VISIBLE | FL_WINDOW_BORDERED, \
						XSize(50, DEF_GUI_ROW_HEIGHT), 100);
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

	grid->HideField(22);
	grid->HideField(23);
	grid->HideField(24);
	grid->HideField(25);
	grid->HideField(26);
	grid->SetFocus();

	grid_x = grid_coords.left;
	grid_y = grid_coords.top;

	XRect margins = main_sizer.getMargins();
	grid_margin_x = margins.left;
	grid_margin_y = margins.top;
}

void CSearchForm::OnFilterButtonClick(XCommandEvent *eve) {

	if (filtering_manager.isFilteringStringChanged()) {

		query_modifier.changeWherePart(filtering_manager.buildFilteringString());

		auto stmt = conn->PrepareQuery(query_modifier.getQuery().c_str());
		if(filtering_manager.apply(stmt))
			db_table->reload(stmt);
	}
	else {

		if(filtering_manager.apply(db_table->getBindingTarget()))
			db_table->reload();
	}
}

void CSearchForm::OnAddRecordButtonClick(XCommandEvent *eve) {


}

void CSearchForm::OnRemoveButtonClick(XCommandEvent *eve) {

}

void CSearchForm::OnSize(XSizeEvent *eve) {

	int width = eve->GetWidth();
	int height = eve->GetHeight();

	if (!(grid && width && height)) return;

	grid->MoveWindow(grid_x, grid_y, \
		width - 2 * grid_margin_x, \
		height - grid_y - grid_margin_y);
}

CSearchForm::~CSearchForm() {

	if (grid && !grid->IsCreated()) delete grid;

	if (flt_zone && !flt_zone->IsCreated()) delete flt_zone;
	if (flt_paid && !flt_paid->IsCreated()) delete flt_paid;
	if (flt_order_type && !flt_order_type->IsCreated()) delete flt_order_type;
	if (flt_stage && !flt_stage->IsCreated()) delete flt_stage;
	if (flt_center && !flt_center->IsCreated()) delete flt_center;
	if (flt_informer && !flt_informer->IsCreated()) delete flt_informer;
	if (flt_advocat && !flt_advocat->IsCreated()) delete flt_advocat;
	if (flt_payment_date_from && !flt_payment_date_from->IsCreated()) delete flt_payment_date_from;
	if (flt_payment_date_to && !flt_payment_date_to->IsCreated()) delete flt_payment_date_to;
	if (flt_act_date_to && !flt_act_date_to->IsCreated()) delete flt_act_date_to;
	if (flt_act_date_from && !flt_act_date_from->IsCreated()) delete flt_act_date_from;
	if (flt_act_reg_date_to && !flt_act_reg_date_to->IsCreated()) delete flt_act_reg_date_to;
	if (flt_act_reg_date_from && !flt_act_reg_date_from->IsCreated()) delete flt_act_reg_date_from;
	if (flt_order_date_to && !flt_order_date_to->IsCreated()) delete flt_order_date_to;
	if (flt_order_date_from && !flt_order_date_from->IsCreated()) delete flt_order_date_from;
	if (flt_act && !flt_act->IsCreated()) delete flt_act;
	if (flt_id && !flt_id->IsCreated()) delete flt_id;
}
