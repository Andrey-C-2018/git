#pragma once
#include <table/ITable.h>
#include "IFieldsProperties.h"
#include "CellsLayout.h"
#include "GridCommonDeclarations.h"
#include "FieldsProperties.h"

struct IReloadable;
class CFieldsProperties;

class CGridTableProxy : public ITableEventsHandler {
	std::shared_ptr<const ITable> table;
	IReloadable *table_widget;
	std::shared_ptr<CFieldsProperties> fields_props;

	size_t records_count;
public:
	CGridTableProxy(std::shared_ptr<const ITable> table_, \
					IReloadable *table_widget_);
	
	template <class TCell> \
		inline LayoutObjects CreateLayoutObjects(const int kind_of_layout);

	inline size_t GetFieldsCount() const;
	inline size_t GetRecordsCount() const;
	inline ImmutableString<Tchar> GetCellAsString(const size_t field, const size_t record) const;
	inline void SetCell(const size_t field, const size_t record, const Tchar *value);

	inline std::shared_ptr<const IFieldsProperties> GetFieldsProperties() const;
	inline std::shared_ptr<IFieldsProperties> GetFieldsProperties();

	void SetFieldWidth(const size_t field, const int new_width);

	void OnFieldsCountChanged(const size_t new_fields_count) override;
	void OnRecordsCountChanged(const size_t new_records_count) override;
	void OnTableReset() override;

	virtual ~CGridTableProxy();
};

//**************************************************************

class CSimpleFieldData final {
	const CGridTableProxy *table_proxy;
	size_t field_index;
public:
	CSimpleFieldData() : table_proxy(nullptr), field_index(0) { }

	CSimpleFieldData(const CGridTableProxy *table_proxy_, \
						const size_t field_index_) : \
						table_proxy(table_proxy_), field_index(field_index_) {

		assert(table_proxy);
	}

	CSimpleFieldData(const CSimpleFieldData &obj) = default;
	CSimpleFieldData(CSimpleFieldData &&obj) = default;
	CSimpleFieldData &operator=(const CSimpleFieldData &obj) = default;
	CSimpleFieldData &operator=(CSimpleFieldData &&obj) = default;

	inline ImmutableString<Tchar> operator[](const size_t index) const {

		return table_proxy->GetCellAsString(field_index, index);
	}

	inline std::pair<size_t, size_t> GetCellCoords(const size_t index) {

		return(std::pair<size_t, size_t>(field_index, index));
	}

	inline size_t size() const {

		return table_proxy->GetRecordsCount();
	}
};

class CSimpleRecordData final {
	const CGridTableProxy *table_proxy;
	size_t record_index;
public:
	CSimpleRecordData() : table_proxy(nullptr), record_index(0) { }

	CSimpleRecordData(const CGridTableProxy *table_proxy_, \
						const size_t record_index_) : \
						table_proxy(table_proxy_), record_index(record_index_) {

		assert(table_proxy);
	}

	CSimpleRecordData(const CSimpleRecordData &obj) = default;
	CSimpleRecordData(CSimpleRecordData &&obj) = default;
	CSimpleRecordData &operator=(const CSimpleRecordData &obj) = default;
	CSimpleRecordData &operator=(CSimpleRecordData &&obj) = default;

	inline ImmutableString<Tchar> operator[](const size_t index) const {

		return table_proxy->GetCellAsString(index, record_index);
	}

	inline std::pair<size_t, size_t> GetCellCoords(const size_t index) {

		return(std::pair<size_t, size_t>(index, record_index));
	}

	inline size_t size() const {

		return table_proxy->GetFieldsCount();
	}
};

class CRecordData final {
	const CGridTableProxy *table_proxy;
public:
	CRecordData(const CGridTableProxy *table_proxy_) : \
					table_proxy(table_proxy_) { }

	CRecordData(const CRecordData &obj) = default;
	CRecordData(CRecordData &&obj) = default;
	CRecordData &operator=(const CRecordData &obj) = default;
	CRecordData &operator=(CRecordData &&obj) = default;

	inline CSimpleFieldData operator[](const size_t index) const {

		return CSimpleFieldData(table_proxy, index);
	}

	inline size_t size() const {

		return table_proxy->GetFieldsCount();
	}
};

class CFieldData final {
	const CGridTableProxy *table_proxy;
public:
	CFieldData(const CGridTableProxy *table_proxy_) : \
				table_proxy(table_proxy_) { }

	CFieldData(const CFieldData &obj) = default;
	CFieldData(CFieldData &&obj) = default;
	CFieldData &operator=(const CFieldData &obj) = default;
	CFieldData &operator=(CFieldData &&obj) = default;

	inline CSimpleRecordData operator[](const size_t index) const {

		return CSimpleRecordData(table_proxy, index);
	}

	inline size_t size() const {

		return table_proxy->GetRecordsCount();
	}
};

//**************************************************************

template <class TCell> \
LayoutObjects CGridTableProxy::CreateLayoutObjects(const int kind_of_layout) {

	IGridCellsContainer *cells_obj = nullptr;
	ISharedComposerContainer *shared_composer_container = nullptr;

	if (kind_of_layout == LAYOUT_RECORD) {
		typedef TSimpleFieldLayout<CSimpleFieldData, TCell> CSimpleField;
		typedef TRecordOfFieldsLayout<CRecordData, CSimpleField> CRecord;

		CRecord *rec_layout_cells = new CRecord(CRecordData(this), fields_props);
		cells_obj = rec_layout_cells;
		shared_composer_container = rec_layout_cells;
	}
	else {
		typedef TSimpleRecordLayout<CSimpleRecordData, TCell> CSimpleRecord;
		typedef TFieldOfRecordsLayout<CFieldData, CSimpleRecord> CField;

		CField *field_layout_cells = new CField(CFieldData(this), fields_props);
		cells_obj = field_layout_cells;
		shared_composer_container = field_layout_cells;
	}

	return LayoutObjects(cells_obj, shared_composer_container);
}

ImmutableString<Tchar> \
CGridTableProxy::GetCellAsString(const size_t field, const size_t record) const {

	return this->table->GetCellAsString(field, record);
}

void CGridTableProxy::SetCell(const size_t field, const size_t record, const Tchar *value) {

}

size_t CGridTableProxy::GetFieldsCount() const {

	return fields_props->GetFieldsCount();
}

size_t CGridTableProxy::GetRecordsCount() const {

	return records_count;
}

std::shared_ptr<const IFieldsProperties> CGridTableProxy::GetFieldsProperties() const {

	return fields_props;
}

inline std::shared_ptr<IFieldsProperties> CGridTableProxy::GetFieldsProperties() {

	return fields_props;
}