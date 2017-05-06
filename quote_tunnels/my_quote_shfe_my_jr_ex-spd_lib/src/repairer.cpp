#include <exception>      // std::exception
#include <string.h>
#include "repairer.h"
#include "quote_cmn_utility.h"


 std::string repairer::ToString(const MDPack &d) {
	  MY_LOG_DEBUG("(server:%d) MDPack Data: instrument:%s islast:%d seqno:%d direction:%c count: %d", this->server_,d.instrument, (int)d.islast, d.seqno, d.direction, d.count);
//	  for(int i = 0; i < d.count; i++) {
//	      MY_LOG_DEBUG("(server:%d) price%d: %lf, volume%d: %d",this->server_, i, d.data[i].price, i, d.data[i].volume);
//	  }
	 return "";
}

// ok
repairer::repairer()
{
	this->working_ = false;
	this->seq_no_ = -1;
	this->victim_ = "";
}

// ok
bool repairer::lose_pkg(MDPack &data)
{
	if (-1 == this->seq_no_){// specially process for the first UPD data
		return false;
	}
	else{
		int new_sn = data.seqno / 10;

		if ((this->seq_no_+1) != new_sn){
			MY_LOG_DEBUG("(server:%d)package loss:sn from %d to %d", this->server_,this->seq_no_, new_sn);
		}
	
		return (this->seq_no_+1) != new_sn;
	}
}

// ok
bool repairer::find_start_point(MDPack &data)
{
	bool found = false;

	MY_LOG_DEBUG("(server:%d)find start point enter,victim:%s",this->server_, this->victim_.c_str());

	if (this->working_){
		throw logic_error("wrong invoke find_start_point!");
	}

	if (lose_pkg(data)) this->victim_ = "";

	if (""==this->victim_){
		if (SHFE_FTDC_D_Buy==data.direction){ this->victim_ = data.instrument; }
		found = false;
	}else{
		if (strcmp(data.instrument,this->victim_.c_str()) == 0){ found = false; }
		else{
			MY_LOG_DEBUG("(server:%d)start point,sn:%d",this->server_, data.seqno);

			this->working_ = true;
			this->victim_ = "";
			found = true; 
		}
	}


	MY_LOG_DEBUG("(server:%d)start point, inst:%s,victim:%s",this->server_, data.instrument,this->victim_.c_str());

	return found;
}

// ok
void repairer::pull_ready_data()
{
	MY_LOG_DEBUG("(server:%d)pull ready data",this->server_);

	bool ready_data_found = false;
	bool damaged = false;

	if (!this->sell_queue_.empty()){
		while (!this->buy_queue_.empty()){
			if (strcmp(sell_queue_.front().content.instrument,this->buy_queue_.front().content.instrument)>0){
				this->buy_queue_.pop_front();
			}
			else if (strcmp(sell_queue_.front().content.instrument,this->buy_queue_.front().content.instrument)==0){
				MDPackEx &new_data = this->buy_queue_.front();
				if (new_data.damaged) damaged = true;
				this->ready_queue_.push_back(new_data);
				this->buy_queue_.pop_front();
				ready_data_found = true;
			}
			else{ break;}

		}

		// add all sell data inti ready queue
		if (ready_data_found ){
			while (!this->sell_queue_.empty()){
				MDPackEx &new_data = this->sell_queue_.front();
				if (new_data.damaged) damaged = true;
				this->ready_queue_.push_back(new_data);
				this->sell_queue_.pop_front();
			}

			// flag damaged data
			if (damaged){
				char *instrument = this->ready_queue_.back().content.instrument;
				int idx = this->ready_queue_.size()-1;
				while (idx>=0 && strcmp(this->ready_queue_[idx].content.instrument, instrument)==0){
					this->ready_queue_[idx].damaged = true;
					idx--;
				}
			}
		}

		this->sell_queue_.clear();
	} // end if (!this->sell_queue_.empty()){

	int size = this->ready_queue_.size();
	for (int i=0; i< size; i++){
		MY_LOG_DEBUG("(server:%d)pull ready dta,sn:%d,damaged:%d",this->server_, this->ready_queue_[i].content.seqno,this->ready_queue_[i].damaged);

	}
	this->print_queue();
}

// ok
void repairer::normal_proc_buy_data(MDPack &data)
{
	if (this->buy_queue_.empty()){
		if (!this->sell_queue_.empty()){
			MY_LOG_ERROR("(server:%d)normal_proc_buy_data,error(sell queue in NOT empty),sn:%d",this->server_, data.seqno);
		}
	}
	else{
		if (strcmp(data.instrument,this->buy_queue_.back().content.instrument)>=0){ // in one patch
			MY_LOG_DEBUG("(server:%d)normal_proc_buy_data,sn:%d",this->server_, data.seqno);
			if (!this->sell_queue_.empty()){
				MY_LOG_ERROR("(server:%d)normal_proc_buy_data,error(sell queue in NOT empty),sn:%d",this->server_, data.seqno);
			}
		}
		else{ // cross more than one patch
			MY_LOG_DEBUG("(server:%d)normal_proc_buy_data,sn:%d",this->server_, data.seqno);
			// pull ready data
			this->pull_ready_data();	
			// remove un-integrity data from buy queue
			this->buy_queue_.clear();
		}
	}

	// add new data into buy queue
	this->buy_queue_.push_back(MDPackEx(data));
}

// ok
void repairer::repair_buy_data(MDPack &data)
{
	MY_LOG_DEBUG("(server:%d)repair_buy_data enter,sn:%d,",this->server_, data.seqno,data.instrument);

	if (strcmp(this->victim_.c_str(),data.instrument) != 0){
		normal_proc_buy_data(data);
		this->victim_ = "";
	}
	else{ /* discard victim data */ }

	MY_LOG_DEBUG("(server:%d)repair_buy_data exit,sn:%d",this->server_, data.seqno);
}

void repairer::normal_proc_sell_data(MDPack &data)
{
	MY_LOG_DEBUG("(server:%d)normal_proc_sell_data,sn:%d",this->server_, data.seqno);

	this->sell_queue_.push_back(data);
	if (data.count<MAX_PAIR){ this->pull_ready_data(); } 
}

// to here
void repairer::repair_sell_data(MDPack &data)
{
	if (strcmp(this->victim_.c_str(),data.instrument) != 0){
		if (!this->sell_queue_.empty()) {
			MY_LOG_ERROR("(server:%d)repair_sell_data,error, sell queue should be emptyr,sn:%d,victim:%s",this->server_, data.seqno,this->victim_.c_str());
		}
		
		MY_LOG_DEBUG("(server:%d)repair_sell_data,new,sn:%d",this->server_, data.seqno);

		this->sell_queue_.push_back(data);
		if (data.count < MAX_PAIR){ this->pull_ready_data(); }

		this->victim_ = "";
	}
	else{ /* discard victim data*/ }

	MY_LOG_DEBUG("(server:%d)repair_sell_data,exit,sn:%d,victim:%s",this->server_, data.seqno,this->victim_.c_str());
}

// ok
void repairer::flag_damaged_data()
{
	if (!this->buy_queue_.empty()){
		if (this->buy_queue_.back().content.count==MAX_PAIR){
			this->buy_queue_.back().damaged = true;
		}
	}

	if (!this->sell_queue_.empty()){
		if (this->sell_queue_.back().content.count==MAX_PAIR){
			this->sell_queue_.back().damaged = true;
		}
	}
}

void repairer::print_queue()
{
//	int size = this->buy_queue_.size();
//	for (int i=0; i<size; i++){
//		MY_LOG_DEBUG("(server:%d)buy queue:seqno:%d",this->server_, this->buy_queue_[i].content.seqno);
//	}
//
//
//	size = this->sell_queue_.size();
//	for (int i=0; i<size; i++){
//		MY_LOG_DEBUG("(server:%d)sell queue:seqno:%d",this->server_, this->sell_queue_[i].content.seqno);
//	}
//
//
	int size = this->ready_queue_.size();
	for (int i=0; i<size; i++){
		MY_LOG_DEBUG("(server:%d)ready queue:seqno:%d",this->server_, this->ready_queue_[i].content.seqno);
	}
}

// ok
void repairer::proc_pkg_loss(MDPack &data)
{
	this->victim_ = data.instrument;

	MY_LOG_DEBUG("(server:%d)proc_pkg_loss,sn:%d,victim:%s",this->server_, data.seqno,this->victim_.c_str());

	this->flag_damaged_data();

	if (SHFE_FTDC_D_Buy==data.direction){
		if (!this->buy_queue_.empty()){
			if (strcmp(data.instrument,this->buy_queue_.back().content.instrument)<0){ // cross more than one patch

				MY_LOG_DEBUG("(server:%d)proc_pkg_loss,buy, cross patch,sn:%d",this->server_, data.seqno);

				this->pull_ready_data();	
				while (!this->buy_queue_.empty()){this->buy_queue_.pop_front();}
				while (!this->sell_queue_.empty()){this->sell_queue_.pop_front();}
			}
		}

	}
	else if (SHFE_FTDC_D_Sell==data.direction){
		if (!this->sell_queue_.empty()){
			if (strcmp(data.instrument,this->sell_queue_.back().content.instrument)<0){ // corss more than patch

				MY_LOG_DEBUG("(server:%d)proc_pkg_loss,sell, cross patch,sn:%d",this->server_, data.seqno);

				this->pull_ready_data();	
				while (!this->buy_queue_.empty()){this->buy_queue_.pop_front();}
				while (!this->sell_queue_.empty()){this->sell_queue_.pop_front();}
			}
			else if (strcmp(data.instrument,this->sell_queue_.back().content.instrument)>=0){ // in one patch
				
				MY_LOG_DEBUG("(server:%d)proc_pkg_loss,sell, one patch,sn:%d",this->server_, data.seqno);

				this->pull_ready_data();	
			}
		}
	}
}


MDPackEx repairer::next(bool &empty)
{
	empty = true;

	if (!this->ready_queue_.empty()){
		MDPackEx data = this->ready_queue_.front();
		empty = false;
		this->ready_queue_.pop_front();
		return data;
	}
}

// ok
// entrance of repairer class
void repairer::rev(MDPack &data)
{
	MY_LOG_DEBUG("%s", ToString(data).c_str());
	int new_sn = data.seqno / 10;
	if (new_sn !=this->seq_no_ + 1) {
		MY_LOG_WARN("seq no from %d to %d, packet loss", seq_no_,new_sn);
	}

	MY_LOG_WARN("server(%d) rev enter,seqno:%d",this->server_,this->seq_no_);
	//this->print_queue();

	if (!this->working_){ // find normal data start point
		this->find_start_point(data);

		if (this->working_){ // the first normal UDP data as start point
			if (SHFE_FTDC_D_Buy==data.direction){
				this->normal_proc_buy_data(data);
			}else if (SHFE_FTDC_D_Sell==data.direction){
				this->normal_proc_sell_data(data);
			}
		}
	}
	else{ // enter receiving process
		if (lose_pkg(data)){ // enter package loss process procedure
			this->proc_pkg_loss(data);
		}// end package loss process procedure
		else{ // enter package non-loss procedure
			if (""==this->victim_){ // normal data process procedure 
				if (SHFE_FTDC_D_Buy==data.direction){
					this->normal_proc_buy_data(data);
				}
				else if (SHFE_FTDC_D_Sell==data.direction){
					this->normal_proc_sell_data(data);
				}
			}
			else{ // enter data repaireing procedure
				if (SHFE_FTDC_D_Buy==data.direction){
					this->repair_buy_data(data);
				}
				else if (SHFE_FTDC_D_Sell==data.direction){
					this->repair_sell_data(data);
				}
			} // end data repaireing procedure
		} // end package non-loss procedure
	} // end enter receiving process

	this->seq_no_ = new_sn;

	MY_LOG_WARN("server(%d) rev exit,seqno:%d",this->server_,this->seq_no_);
	this->print_queue();
}

//
