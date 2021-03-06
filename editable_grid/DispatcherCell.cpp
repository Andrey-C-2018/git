#include "DispatcherCell.h"
#include <grid/GridTableProxy.h>
#include "ITabKeyAction.h"

class OnCellChangedAction : public IOnCellChangedAction {
	CGridTableProxy *table_proxy;
	const size_t active_field;
	const size_t active_record;
	IGridCellWidget *cell_widget;
public:
	inline OnCellChangedAction(CGridTableProxy *table_proxy_, \
								const size_t active_field_, \
								const size_t active_record_, \
								IGridCellWidget *cell_widget_) noexcept : \
							table_proxy(table_proxy_),\
							active_field(active_field_), \
							active_record(active_record_), cell_widget(cell_widget_) {

		assert(table_proxy);
		assert(cell_widget);
	}

	void executeAction() override {
		
		ImmutableString<Tchar> value = cell_widget->GetLabel();
		table_proxy->SetCell(active_field, active_record, value.str);
	}

	void executeAction(ImmutableString<Tchar> cell_value) override {

		table_proxy->SetCell(active_field, active_record, cell_value.str);
	}

	virtual ~OnCellChangedAction() { }
};

//**************************************************************

CDispatcherCell::CDispatcherCell() : parent(nullptr), def_active_cell(nullptr), \
									tab_action(nullptr), \
									active_field((size_t)-1), active_record((size_t)-1), \
									active_cell_reached(false), active_cell_lost_focus(true), \
									active_cell_text(nullptr, 0), \
									active_cell_color(255, 255, 255), \
									def_act_cell_coords(0, 0), \
									active_cell_hidden(true), \
									move_def_cell(true), inside_on_click(false), \
									old_scroll_pos(0), table_proxy(nullptr), \
									update_cell_widget_text(true), \
									changes_present(false), \
									skip_reloading(false) { }

CDispatcherCell::CDispatcherCell(CDispatcherCell &&obj) : parent(obj.parent), \
									def_active_cell(obj.def_active_cell), \
									tab_action(obj.tab_action), \
									active_field(obj.active_field), \
									active_record(obj.active_record), \
									active_cell_reached(obj.active_cell_reached), \
									active_cell_lost_focus(obj.active_cell_lost_focus), \
									active_cell_text(obj.active_cell_text), \
									active_cell_color(255, 255, 255), \
									def_act_cell_coords(obj.def_act_cell_coords), \
									active_cell_hidden(obj.active_cell_hidden), \
									move_def_cell(obj.move_def_cell), \
									inside_on_click(obj.inside_on_click), \
									old_scroll_pos(obj.old_scroll_pos), \
									table_proxy(std::move(obj.table_proxy)), \
									event_handler(std::move(obj.event_handler)), \
									update_cell_widget_text(obj.update_cell_widget_text), \
									changes_present(obj.changes_present), \
									skip_reloading(obj.skip_reloading) {

	obj.parent = nullptr;
	obj.def_active_cell = nullptr;
	obj.tab_action = nullptr;
	obj.active_field = (size_t)-1;
	obj.active_record = (size_t)-1;
	obj.active_cell_reached = false;
	obj.active_cell_lost_focus = true;
	obj.active_cell_text.str = nullptr;
	obj.active_cell_text.size = 0;
	obj.def_act_cell_coords.x = 0;
	obj.def_act_cell_coords.y = 0;
	obj.active_cell_hidden = true;
	obj.move_def_cell = false;
	obj.inside_on_click = false;
	obj.old_scroll_pos = 0;
	obj.update_cell_widget_text = true;
	obj.changes_present = false;
	obj.skip_reloading = false;
}

void CDispatcherCell::SetBounds(const int left_bound, const int upper_bound) {

	bounds.left = left_bound;
	bounds.top = upper_bound;
}

void CDispatcherCell::SetParameters(XWindow * parent, ITabKeyAction *tab_action, \
									std::shared_ptr<IGridEventsHandler> grid_events_handler, \
									IGridCellWidget *cell_widget, \
									std::shared_ptr<CGridTableProxy> table_proxy) {

	int flags = table_proxy->GetFieldsCount() && table_proxy->GetRecordsCount() ? \
					FL_WINDOW_VISIBLE : 0;

	assert(!def_active_cell);
	assert(cell_widget);
	def_active_cell = cell_widget;

	assert(parent);
	this->parent = parent;
	assert(tab_action);
	this->tab_action = tab_action;
	assert(cell_widget);
	assert(grid_events_handler);
	this->event_handler = grid_events_handler;

	assert(!this->table_proxy);
	assert(table_proxy);
	this->table_proxy = table_proxy;

	active_field = 0;
	active_record = 0;

	ImmutableString<Tchar> label(nullptr, 0);
	if (table_proxy->GetRecordsCount())
		label = table_proxy->GetCellAsString(active_field, active_record);

	def_active_cell->CreateCellWidget(parent, flags, label.str ? label.str : _T(""), \
											0, 0, 10, 10);
	assert(def_active_cell);

	def_active_cell->SetOnChangeHandler(XEventHandlerData(this, \
											&CDispatcherCell::OnActiveCellTextChanged));
	auto cell_event_handler = std::dynamic_pointer_cast<ICellEventHandler, \
														IGridEventsHandler>(grid_events_handler);
	assert(cell_event_handler);
	def_active_cell->SetOnIndirectChangeHandler(cell_event_handler);
	def_active_cell->SetOnKeyPressHandler(XEventHandlerData(this, \
											&CDispatcherCell::OnActiveCellKeyPressed));
	def_active_cell->SetOnGetFocusHandler(XEventHandlerData(this, \
											&CDispatcherCell::OnActiveCellGetsFocus));
	def_active_cell->SetOnLooseFocusHandler(XEventHandlerData(this, \
											&CDispatcherCell::OnActiveCellLoosesFocus));
}

void CDispatcherCell::Draw(XGC &gc, const XPoint &initial_coords, const XSize &size) {

	if (active_cell_reached && def_active_cell) {
		if (initial_coords != curr_coords || size != curr_size || \
			move_def_cell ) {

			int x(initial_coords.x), y(initial_coords.y);
			int width(size.width), height(size.height);

			if (initial_coords.x < bounds.left) {
				x = bounds.left;
				width = size.width - (bounds.left - initial_coords.x);
			}
			if (initial_coords.y < bounds.top) {
				y = bounds.top;
				height = size.height - (bounds.top - initial_coords.y);
			}

			if (move_def_cell) {
				def_active_cell->MoveWindow(x, y, width, height);
				move_def_cell = false;

				if (active_cell_hidden) {
					def_active_cell->Show();
					active_cell_hidden = false;
				}
			}

			curr_coords = initial_coords;
			curr_size = size;
		}
		active_cell_reached = false;

		if (active_cell_hidden) {
			XColor old_bg_color = gc.SetBackgroundColor(active_cell_color);
			unactive_cell.Draw(gc, initial_coords, size);
			gc.SetBackgroundColor(old_bg_color);
		}
	}
	else
		unactive_cell.Draw(gc, initial_coords, size);
}

void CDispatcherCell::OnActiveCellTextChanged(XCommandEvent *eve) {

	if (update_cell_widget_text) return;
	
	changes_present = true;
}

void CDispatcherCell::OnActiveCellKeyPressed(XKeyboardEvent *eve) {
	bool exec_def_action = true;

	switch (eve->GetKey()) {
	case X_VKEY_ENTER:
		OnClick(active_field, active_record);

		inside_on_click = true;
		parent->SetFocus();
		inside_on_click = false;

		exec_def_action = false;
		break;

	case X_VKEY_ESCAPE:
		RefreshActiveCellWidgetLabel();

		inside_on_click = true;
		parent->SetFocus();
		inside_on_click = false;

		exec_def_action = false;
		break;

	case X_VKEY_TAB:
		tab_action->OnTabKeyPressed();

		inside_on_click = true;
		def_active_cell->SetFocus();
		inside_on_click = false;

		exec_def_action = false;
	}

	eve->ExecuteDefaultEventAction(exec_def_action);
}

void CDispatcherCell::OnActiveCellGetsFocus(XEvent *eve) {

	active_cell_lost_focus = false;
}

void CDispatcherCell::OnActiveCellLoosesFocus(XEvent *eve) {

	active_cell_lost_focus = true;
	OnClick(active_field, active_record);
}

void CDispatcherCell::OnClick(const size_t field, const size_t record) {

	if (inside_on_click) return;
	inside_on_click = true;

	CommitChangesIfPresent();

	size_t prev_active_field = active_field;
	size_t prev_active_record = active_record;

	active_field = field;
	def_active_cell->SetCurrentField(table_proxy->GetAbsoluteFieldIndex(active_field));

	active_record = record;
	OnTableChanged(prev_active_field, prev_active_record);

	inside_on_click = false;
}

void CDispatcherCell::CommitChangesIfPresent() {

	if (changes_present) {
		
		skip_reloading = true;
		OnCellChangedAction action(table_proxy.get(), \
									active_field, active_record, def_active_cell);
		event_handler->OnCellChanged(def_active_cell, action);
		skip_reloading = false;

		changes_present = false;
	}
}

void CDispatcherCell::OnTableChanged(const size_t old_field, const size_t old_record) {

	size_t records_count = table_proxy->GetRecordsCount();
	def_active_cell->Hide();

	if (records_count) {
		if (active_record >= records_count)
			active_record = records_count - 1;

		RefreshActiveCellWidgetLabel();
	}

	active_cell_hidden = true;
	move_def_cell = true;

	if (active_field != old_field || active_record != old_record)
		event_handler->OnActiveCellLocationChanged(active_field, active_record);
}

void CDispatcherCell::Reload() {

	if (skip_reloading) return;

	OnTableChanged(active_field, active_record);
}

void CDispatcherCell::RefreshActiveCellWidgetLabel() {

	ImmutableString<Tchar> label = table_proxy->GetCellAsString(active_field, active_record);
	label.str = label.str ? label.str : _T("");

	update_cell_widget_text = true;
	def_active_cell->SetLabel(label);
	update_cell_widget_text = false;
}

void CDispatcherCell::OnFieldRemoved(const size_t field) {

	assert(active_field != (size_t)-1);
	if (field <= active_field) {

		CommitChangesIfPresent();

		size_t prev_active_field = active_field;
		if (field < active_field || \
			(field == active_field && field + 1 == table_proxy->GetFieldsCount()))
			--active_field;

		size_t index = 0;
		if (prev_active_field == field) {
			if (field + 1 == table_proxy->GetFieldsCount()) index = prev_active_field - 1;
			if (!field) index = 1;
		}
		def_active_cell->SetCurrentField(table_proxy->GetAbsoluteFieldIndex(index));

		OnTableChanged(prev_active_field, active_record);
	}
}