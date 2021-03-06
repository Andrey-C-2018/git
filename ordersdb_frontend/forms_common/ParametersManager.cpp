#include "ParametersManager.h"
#include <basic/XString.h>
#include <basic/XConv.h>
#include <basic/TextConv.h>
#include <basic/IProperties.h>
#include <db/IDbConnection.h>
#include <db/IDbStatement.h>
#include <db/IDbResultSet.h>
#include "AutorizationManager.h"

CParametersManager CParametersManager::inst;

CParametersManager::CParametersManager() : id_user(-1), default_center(-1) { }

void CParametersManager::getParamsValuesFromFile(IProperties *props, \
												std::shared_ptr<IDbConnection> conn) {
	
	assert(props);
	assert(conn);
	Tstring buffer;

	determineUserAndPermissions(props, conn, buffer);

	auto str_date = props->getStringProperty(_T("initial_order_date"), buffer);
	if (!str_date) return;

	CDate initial_order_date;
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
	date_buffer.resize(CDate::SQL_FORMAT_LEN);
	initial_order_date.toStringSQL(&date_buffer[0]);

	date_buffer_w.resize(CDate::GERMAN_FORMAT_LEN);
	initial_order_date.toStringGerman(&date_buffer_w[0]);
}

void CParametersManager::determineUserAndPermissions(IProperties *props, \
												std::shared_ptr<IDbConnection> conn, \
												Tstring &buffer) {

	assert(props);
	assert(conn);

	const Tchar *p_user_name = props->getStringProperty(_T("user_name"), buffer);
	if (!p_user_name || (p_user_name && p_user_name[0] == _T('\0')))
		throw XException(0, _T("�������� 'user_name' �������� � config.ini"));
	Tstring user_name = p_user_name;

	const Tchar *pwd = props->getStringProperty(_T("password"), buffer);
	if (!pwd || (pwd && pwd[0] == _T('\0')))
		throw XException(0, _T("�������� 'password' �������� � config.ini"));

	auto stmt = conn->PrepareQuery("SELECT id_user, id_center FROM users WHERE user_name = ?");
	stmt->bindValue(0, user_name.c_str());
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

	this->default_center = rs->getInt(1, is_null);
	assert(!is_null);
	if (default_center < 1 || default_center > 5) {
		XException e(0, _T("������� ����� ������: "));
		e << default_center;
		throw e;
	}
	
	CAutorizationManager auth_mgr(conn);
	auth_mgr.autorize(id_user, user_name.c_str(), ImmutableString<Tchar>(pwd, Tstrlen(pwd)));

	permissions_mgr.init(conn, id_user, user_name);
}

void CParametersManager::init(IProperties *props, std::shared_ptr<IDbConnection> conn) {

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

	if (!date_buffer.empty()) {
		initial_flt += " AND a.order_date >= '";
		initial_flt += date_buffer;
		initial_flt += "'";
	}

	return std::move(initial_flt);
}

const std::string &CParametersManager::getCenterFilteringStr() const {

	if (!center_flt_str.empty()) return center_flt_str;

	XString<char> str;

	if (default_center == 1) {
		UCS16_ToUTF8(L"��", 2, str);

		center_flt_str = "a.zone = '";
		center_flt_str += str.c_str();
		center_flt_str += "'";
	}
	else {
		str.resize(10);
		XConv::ToString(default_center, &str[0]);

		center_flt_str = "a.id_center_legalaid IN (1,";
		center_flt_str += str.c_str();
		center_flt_str += ')';
	}

	return center_flt_str;
}

CParametersManager::~CParametersManager() { }
