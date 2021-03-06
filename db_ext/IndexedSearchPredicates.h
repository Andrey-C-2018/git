#pragma once
#include <string.h>

template <typename Tchar_, typename IndexType, class TRegistry, class GetStrByIndex> \
struct CIndexedTextSearchPredicate {
	const TRegistry &reg;
	GetStrByIndex getStrByIndex;

	inline CIndexedTextSearchPredicate(const TRegistry &reg_) noexcept : reg(reg_) { }

	inline bool operator()(IndexType l, IndexType r) const {

		return strcmp(getStrByIndex(reg, l), (getStrByIndex(reg, r))) < 0;
	}

	inline bool operator()(IndexType l, const Tchar_ *r) const {

		return strcmp(getStrByIndex(reg, l), r) < 0;
	}

	inline bool operator()(const Tchar_ *l, IndexType r) const {

		return strcmp(l, getStrByIndex(reg, r)) < 0;
	}
};

template <typename TType, typename IndexType, class TRegistry, class GetValueByIndex> \
struct CIndexedValueSearchPredicate {
	const TRegistry &reg;
	GetValueByIndex getValueByIndex;

	inline CIndexedValueSearchPredicate(const TRegistry &reg_) : reg(reg_) { }
	inline bool operator()(IndexType l, IndexType r) const {

		return getValueByIndex(reg, l) < getValueByIndex(reg, r);
	}
	inline bool operator()(IndexType l, TType r) const {

		return getValueByIndex(reg, l) < r;
	}
	inline bool operator()(TType l, IndexType r) const {

		return l < getValueByIndex(reg, r);
	}
};
