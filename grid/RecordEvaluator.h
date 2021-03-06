#pragma once

class CRecordEvaluator{
public:
	struct CPosition {
		size_t item_index;
		int x;
		int offset;

		inline CPosition() : item_index(0), x(0), offset(0){ }

		inline bool IsNull() const { 
			return item_index == (size_t)-1; 
		}
	};
private:
	int sum;
	size_t curr_item_index;

	template <class GetItemSizePredicate> \
	inline CPosition EvalPositionDirect(const size_t items_count,\
										GetItemSizePredicate items_sizes,\
										const int offset);
	template <class GetItemSizePredicate> \
	inline CPosition EvalPositionReverse(GetItemSizePredicate items_sizes,\
										const int offset);
public:
	CRecordEvaluator() : sum(0), curr_item_index(0){ }
	CRecordEvaluator(const CRecordEvaluator &obj) = default;
	CRecordEvaluator(CRecordEvaluator &&obj) = default;
	CRecordEvaluator &operator=(const CRecordEvaluator &obj) = default;
	CRecordEvaluator &operator=(CRecordEvaluator &&obj) = default;

	inline bool operator<(const CRecordEvaluator &obj) const;

	template <class GetItemSizePredicate> \
	inline CPosition EvalPosition(bool straight_direction,\
									const size_t items_count,\
									GetItemSizePredicate items_sizes,\
									const int offset);

	template <class GetItemSizePredicate> \
	inline CPosition FocusOnItem(const int total_sum, \
									const int view_area_width, \
									const size_t items_count, \
									const size_t item_index, \
									GetItemSizePredicate items_sizes, \
									const int offset);

	template <class GetItemSizePredicate> \
	inline CPosition RemoveItem(const int total_sum, \
								const int view_area_width, \
								const size_t items_count, \
								const size_t item_index, \
								GetItemSizePredicate items_sizes, \
								const int offset);
	void Reset(){ sum = 0; curr_item_index = 0; }
	virtual ~CRecordEvaluator(){ }
};

//*************************************************************

bool CRecordEvaluator::operator<(const CRecordEvaluator &obj) const {

	return curr_item_index < obj.curr_item_index;
}

template <class GetItemSizePredicate> \
CRecordEvaluator::CPosition CRecordEvaluator::EvalPositionDirect(const size_t items_count, \
										GetItemSizePredicate items_sizes, \
										const int offset){

	size_t i;
	int prev_sum = sum;

	i = curr_item_index;
	while(sum < offset && i < items_count){
		prev_sum = sum;
		sum += items_sizes(i);
		curr_item_index = i;
		++i;
	}
	i -= (int)(sum > offset);

	CPosition position;

	position.item_index = i;
	position.x = (sum == offset) ? sum : prev_sum;
	position.offset = offset;

	sum = prev_sum;
	return position;
}

template <class GetItemSizePredicate> \
CRecordEvaluator::CPosition CRecordEvaluator::EvalPositionReverse(GetItemSizePredicate items_sizes, \
												const int offset){
	size_t i = curr_item_index;
	CPosition position;

	position.x = sum;
	while(i > 0 && offset < position.x){
		position.x -= items_sizes(i - 1);
		--i;
	}

	position.item_index = curr_item_index = i;
	position.offset = offset;

	sum = position.x;
	return position;
}

template <class GetItemSizePredicate>
CRecordEvaluator::CPosition CRecordEvaluator::EvalPosition(bool straight_direction, \
										const size_t items_count, \
										GetItemSizePredicate items_sizes, \
										const int offset) {

	if(straight_direction)
		return EvalPositionDirect(items_count, items_sizes, offset);

	return EvalPositionReverse(items_sizes, offset);
}

template <class GetItemSizePredicate> \
CRecordEvaluator::CPosition CRecordEvaluator::FocusOnItem(const int total_sum, \
														const int view_area_width, \
														const size_t items_count, \
														const size_t item_index, \
														GetItemSizePredicate items_sizes, \
														const int offset) {
	CPosition position;
	position.offset = offset;
	position.item_index = curr_item_index;
	position.x = sum;

	assert(item_index < items_count);
	if (total_sum - offset == view_area_width || total_sum <= view_area_width)	
		return position;
	
	if (curr_item_index < item_index) {
		size_t i = curr_item_index;
		int sum_t = sum;

		while (sum_t <= view_area_width + offset && i <= item_index) {
			sum_t += items_sizes(i);
			++i;
		}

		if (sum_t <= view_area_width + offset)
			return position;
		
		while (i <= item_index) {
			sum_t += items_sizes(i);
			++i;
		}
		int new_offset = sum_t - view_area_width;
		return EvalPositionDirect(items_count, items_sizes, new_offset);
	}
	else {
		size_t i = curr_item_index;
		while (i > item_index) {
			sum -= items_sizes(i - 1);
			--i;
		}

		position.offset = sum;
		position.item_index = curr_item_index = i;
		position.x = sum;
		return position;
	}
}

template <class GetItemSizePredicate> \
CRecordEvaluator::CPosition CRecordEvaluator::RemoveItem(const int total_sum, \
														const int view_area_width, \
														const size_t items_count, \
														const size_t item_index, \
														GetItemSizePredicate items_sizes, \
														const int offset) {
	CPosition position;
	position.offset = offset;
	if (total_sum < view_area_width) return position;

	int removed_item_size = items_sizes(item_index);
	
	if(item_index < curr_item_index) {
		sum -= removed_item_size;
		--curr_item_index;

		position.item_index = curr_item_index;
		position.x = sum;
		position.offset = offset - removed_item_size;
		return position;
	}

	int new_offset = offset;
	if (item_index == curr_item_index) {
		new_offset = sum;

		position = EvalPositionDirect(items_count, items_sizes, new_offset);
	}

	int total_sum_decr = total_sum - removed_item_size;
	if (new_offset + view_area_width > total_sum_decr) {
		new_offset = (total_sum_decr >= view_area_width) ? \
						total_sum_decr - view_area_width : 0;
		
		position = EvalPositionReverse(items_sizes, new_offset);
	}

	return position;
}
