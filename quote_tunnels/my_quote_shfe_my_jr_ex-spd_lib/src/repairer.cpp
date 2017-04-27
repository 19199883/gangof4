#include <exception>      // std::exception
#include "repairer.h"

// ok
repairer::repairer(int svrid)
{
	this->working_ = false;
	this->seq_no_ = -1;
	this->server_ = svrid;
	this->victim_ = "";
}

// ok
bool repairer::lose_pkg((MDPack &data)
{
	if (-1 == this->seq_no_){// specially process for the first UPD data
		return false;
	}
	else{
		int new_sn_ = data.seqno / 10;
		return this->seq_no_ != new_sn + 1;
	}
}

// ok
bool repairer::find_start_point(MDPack &data)
{
	if (this->working_){
		throw logic_error("wrong invoke find_start_point!")
	}

	if (lose_pkg(data)) this->victim_ = "";

	if (""==this->victim_){
		if (SHFE_FTDC_D_Buy==data.direction){ this->victim_ = data.instrument; }
		return false;
	}else{
		if (data.instrument==this->victim_){ return false; }
		else{
			this->victim_ = "";
			return true; 
		}
	}
}


void repairer::normal_proc_buy_data(MDPack &data)
{
}

void repairer::repair_buy_data(MDPack &data)
{
}

void repairer::normal_proc_sell_data(MDPack &data)
{
}

void reparier::repair_sell_data(MDPack &data)
{
}

void repaired::rev(MDPack &data)
{
	int new_sn_ = data.seqno / 10;

	if (!this->working_){
		this->working_ = this->find_start_point(data);
		// the first normal UDP data as start point
	}else{

	}

	this->seq_no_ = new_sn;
}
