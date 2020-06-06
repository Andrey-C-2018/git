#include "ParametersManager.h"
#include <basic/XConv.h>
#include <basic/PropertiesFile.h>
#include <db/IDbConnection.h>
#include <db/IDbStatement.h>
#include <db/IDbResultSet.h>

CParametersManager CParametersManager::inst;

CParametersManager::CParametersManager() : id_user(-1), default_center(-1) { }

void CParametersManager::getParamsValuesFromFile(CPropertiesFile *props, \
												std::shared_ptr<IDbConnection> conn) {
	
	assert(props);
	assert(conn);
	Tstring buffer;

	determineUserAndGroups(props, conn, buffer);

	auto str_date = props->getStringProperty(_T("initial_order_date"), buffer);
	if (!str_date)
		throw XException(0, \
			_T("�������� ��������� ���� 'initial_order_date' �������� � config.ini"));

	if (!initial_order_date.setDateGerman(str_date)) {
		XException e(0, _T("������� ������ ��������� ����: "));
		e << str_date;
		throw e;
	}
	if (initial_order_date > CDate::now()) {
		XException e(0, _T("��������� ���� � �����������: "));
		e << str_date;
		throw e;
	}
	initial_order_date.toStringSQL(date_buffer);
	initial_order_date.toStringGerman(date_buffer_w);

	bool not_found;
	default_center = props->getIntProperty(_T("default_center"), buffer, not_found);
	if(not_found)
		throw XException(0, \
			_T("�������� ������ 'default_center' �������� � config.ini"));

	if (default_center < 1 || default_center > 5) {
		XException e(0, _T("������� ����� ������: "));
		e << default_center;
		throw e;
	}
}

void CParametersManager::determineUserAndGroups(CPropertiesFile *props, \
												std::shared_ptr<IDbConnection> conn, \
												Tstring &buffer) {

	assert(props);
	assert(conn);

	const Tchar *user_name = props->getStringProperty(_T("user_name"), buffer);
	if (!user_name || (user_name && user_name[0] == _T('\0')))
		throw XException(0, _T("�������� 'user_name' �������� � config.ini"));

	auto stmt = conn->PrepareQuery("SELECT id_user FROM users WHERE user_name = ?");
	stmt->bindValue(0, user_name);
	auto rs = stmt->exec();

	if (rs->empty()) {
		XException e(0, _T("���������� '"));
		e << user_name << _T("' �������� � ��. �������� config.ini");
		throw e;
	}
	rs->gotoRecord(0);

	bool is_null;
	this->id_user = rs->getInt(0, is_null);
	assert(!is_null);

	stmt = conn->PrepareQuery("SELECT gr.group_name FROM groups gr INNER JOIN users_groups ug ON gr.id_group = ug.id_group WHERE ug.id_user = ? ORDER BY 1");
	stmt->bindValue(0, id_user);
	rs = stmt->exec();

	size_t groups_count = rs->getRecordsCount();
	if (!groups_count) {
		XException e(0, _T("��������� ��������� ������ ���� ��� ����������� '"));
		e << user_name << _T("'");
		throw e;
	}

	for (size_t i = 0; i < groups_count; ++i) {
		rs->gotoRecord(i);
		groups.emplace_back(rs->getString(0));
	}
}

void CParametersManager::init(CPropertiesFile *props, std::shared_ptr<IDbConnection> conn) {

	if (!inst.areParamsInitilized())
		inst.getParamsValuesFromFile(props, conn);
	else
		throw XException(0, _T("���� CParametersManager ��� ���� �������������"));
}

std::string CParametersManager::getInitialFilteringStr() const {
	
	char int_buffer[10];

	std::string initial_flt = "a.id_center_legalaid = ";
	XConv::ToString(default_center, int_buffer);
	initial_flt += int_buffer;
	initial_flt += " AND a.order_date >= '";
	initial_flt += date_buffer;
	initial_flt += "'";

	return std::move(initial_flt);
}

CParametersManager::~CParametersManager() { }
