#pragma once
#include <map>
#include "IGridCellWidget.h"

class XWidget;

class CMultipleCellWidget : public IGridCellWidget {
	std::map<size_t, XWidget *> widgets;
public:
	CMultipleCellWidget();
	virtual ~CMultipleCellWidget();
};

