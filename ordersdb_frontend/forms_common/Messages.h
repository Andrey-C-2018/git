#pragma once

constexpr wchar_t E_WRONG_ZONE[] = L"��������� ������ �� �����, ������� ���� �������� �� ������ ������";
constexpr wchar_t E_OLD_ORDER[] = L"��������� ������ �� ���������, ������� ���� ���� ������ ����� ���� ����";
constexpr wchar_t E_OLD_STAGE[] = L"��������� ������ �� �����, ������� ���� �������� �� �������� ������";

const wchar_t *E_ENTRIES[] = { E_OLD_ORDER, E_OLD_STAGE };
enum E_Indexes {
	E_OLD_ORDER_INDEX = 0, \
	E_OLD_STAGE_INDEX = 1
};