#pragma once
#include <db_ext/IDbInserter.h>
#include <db_ext/DbInsertHelper.h>
#include <db_controls/InsertionBinders.h>

struct IDbStatement;
class XComboBox;
class CDbTable;

class CPaymentsInserter : public IDbInserter {
	enum {
		FIELDS_COUNT = 37
	};
	CDbInsertHelper ins_helper;

	std::shared_ptr<IDbConnection> conn;
	std::shared_ptr<IDbStatement> stmt;
	std::shared_ptr<CDbTable> db_table;

	CDbComboBox *stage;
	XWidget *cycle;
	XWidget *act_no;
	XWidget *article;
	XWidget *fee;
	XWidget *outg_post, *outg_daynight;
	CDbComboBox *informer;
	XWidget *id_act;
	XWidget *act_date;
	XWidget *act_reg_date;

	XWidget *age, *inv, *lang, *ill, *zek, *vpr;
	XWidget *reduce, *change, *close, *zv;
	XWidget *min_penalty, *nm_suv, *zv_kr, *no_ch_Ist;
	XWidget *Koef;
	CDbComboBox *checker;

public:
	CPaymentsInserter(std::shared_ptr<CDbTable> db_table_);
	
	CPaymentsInserter(const CPaymentsInserter &obj) = delete;
	CPaymentsInserter(CPaymentsInserter &&obj) = delete;
	CPaymentsInserter &operator=(const CPaymentsInserter &obj) = delete;
	CPaymentsInserter &operator=(CPaymentsInserter &&obj) = delete;

	inline void setStageWidget(CDbComboBox *stage) { this->stage = stage; }
	inline void setCycleWidget(XWidget *cycle) { this->cycle = cycle; }
	inline void setActNoWidget(XWidget *act_no) { this->act_no = act_no; }
	inline void setArticleWidget(XWidget *article) { this->article = article; }
	inline void setFeeWidget(XWidget *fee) { this->fee = fee; }
	inline void setOutgPostWidget(XWidget *outg_post) { this->outg_post = outg_post; }
	inline void setOutgDayNightWidget(XWidget *outg_daynight) { this->outg_daynight = outg_daynight; }

	inline void setInformerWidget(CDbComboBox *informer) { this->informer = informer; }
	inline void setActWidget(XWidget *id_act) { this->id_act = id_act; }
	inline void setActDateWidget(XWidget *act_date) { this->act_date = act_date; }
	inline void setActRegDateWidget(XWidget *act_reg_date) { this->act_reg_date = act_reg_date; }

	inline void setAgeWidget(XWidget *age) { this->age = age; }
	inline void setInvWidget(XWidget *inv) { this->inv = inv; }
	inline void setLangWidget(XWidget *lang) { this->lang = lang; }
	inline void setIllWidget(XWidget *ill) { this->ill = ill; }
	inline void setZekWidget(XWidget *zek) { this->zek = zek; }
	inline void setVprWidget(XWidget *vpr) { this->vpr = vpr; }
	inline void setReduceWidget(XWidget *reduce) { this->reduce = reduce; }
	inline void setChangeWidget(XWidget *change) { this->change = change; }
	inline void setCloseWidget(XWidget *close) { this->close = close; }
	inline void setZvilnWidget(XWidget *zv) { this->zv = zv; }
	inline void setMinPenaltyWidget(XWidget *min_penalty) { this->min_penalty = min_penalty; }
	inline void setNmSuvWidget(XWidget *nm_suv) { this->nm_suv = nm_suv; }
	inline void setZvilnKrWidget(XWidget *zv_kr) { this->zv_kr = zv_kr; }
	inline void setNoCh1instWidget(XWidget *no_ch_Ist) { this->no_ch_Ist = no_ch_Ist; }

	inline void setKoeffWidget(XWidget *Koef) { this->Koef = Koef; }
	inline void setCheckerWidget(CDbComboBox *checker) { this->checker = checker; }

	void getCurrRecord();

	void prepare(std::shared_ptr<IDbConnection> conn) override;
	bool insert() override;

	void Dispose();
	virtual ~CPaymentsInserter();
};
