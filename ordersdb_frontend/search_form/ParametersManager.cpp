#include "ParametersManager.h"
#include <basic/PropertiesFile.h>

CParametersManager::CParametersManager() : default_center(-1) { }

void CParametersManager::Init(CPropertiesFile *props) {
	
	assert(props);
	Tstring buffer;
		
	auto str_date = props->getStringProperty(_T("initial_order_date"), buffer);
	if (!str_date)
		throw XException(0, \
			_T("�������� ��������� ���� 'initial_order_date' ������� � config.ini"));

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

	bool not_found;
	default_center = props->getIntProperty(_T("default_center"), buffer, not_found);
	if(not_found)
		throw XException(0, \
			_T("�������� ������ 'default_center' ������� � config.ini"));

	if (default_center < 1 || default_center > 5) {
		XException e(0, _T("������� ����� ������: "));
		e << default_center;
		throw e;
	}
}

CParametersManager::~CParametersManager() { }
