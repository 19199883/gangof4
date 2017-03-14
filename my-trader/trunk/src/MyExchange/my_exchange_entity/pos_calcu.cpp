#include "pos_calcu.h"

pos_calc* pos_calc::ins_ = NULL;
mutex* pos_calc::mtx_ins_ = NULL;

pos_calc *pos_calc::instance()
{
	if (NULL == pos_calc::ins_){
		lock_guard<std::mutex> lck (mtx_ins_);
		if (NULL == pos_calc::ins_){
			pos_calc::ins_ = new pos_calc();
		}
	}

	return 	pos_calc::ins_; 
}
