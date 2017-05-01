#includ <exception>      // std::exception
#include <string.h>
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

// ok
void repairer::pull_ready_data()
{
	bool ready_data_found = false;
	bool damaged = false;

	if (!this->sell_queue_.empty()){
		while (!this->buy_queue_.empty()){
			if (strcmp(sell_queue_.front().instrument,this->buy_queue_.front().instrument)>0){
				this->buy_queue_.pop();
			}
			else if (strcmp(sell_queue_.front().instrument,this->buy_queue_.front().instrument)==0){
				MDPackEx new_data &this->buy_queue_.front();
				if (new_data.damaged) damaged = true;
				this->ready_queue_.push_back(ready_data);
				this->buy_queue_.pop();
				ready_data_found = true;
			}
			else{ break;}

		}

		// add all sell data inti ready queue
		if (ready_data_found ){
			while (!this->sell_queue_.empty()){
				MDPackEx &new_data = this->sell_queue_.front();
				if (new_data.damaged) damaged = true;
				this->ready_queue_.push(new_data);
				this->sell_queue_.pop();
			}

			// flag damaged data
			if (damaged){
				char instrument = this->ready_queue_.back().instrument;
				int idx = this->ready_queue_.size()-1;
				while (idx>=0 && strcmp(this->ready_queue_[idx].instrument, instrument)==0){
					this->ready_queue_[idx].damaged = true;
					idx--;
				}
			}
		}

	} // end if (!this->sell_queue_.empty()){
}

// ok
void repairer::normal_proc_buy_data(MDPack &data)
{
	if (this->buy_queue_.empty()){
		this->buy_queue_.push(MDPackEx(data));
	}
	else{
		if (strcmp(data.instrument,this->buy_queue_.back().instrument)>=0){
			this->buy_queue_.push(MDPackEx(data));
		}
		else{
			this->pull_ready_data();	
			while (!this->buy_queue_.empty()){this->buy_queue_.pop();}
		}
	}

	// remove unusable data for package loss
	// it's fact that all left sell data are unusable when receiving buy data
	while (!this->sell_queue_.empty()){this->sell_queue_.pop();}
}

// ok
void repairer::repair_buy_data(MDPack &data)
{
	if (this->victim_!=data.instrument){
		normal_proc_buy_data(data);
		this->victim_ = "";
	}
	else{ // discard victim data }
}

// ok
void repairer::normal_proc_sell_data(MDPack &data)
{
	this->sell_queue_.push(data);
	if (data.count<MAX_PAIR){ this->pull_ready_data()); } 
}

// ok
void reparier::repair_sell_data(MDPack &data)
{
	if (this->victim_!=data.instrument){
		if (!this->sell_queue_.empty()) {
			if (strcmp(data.instrument,this->sell_queue_.back().instrument)<0){// cross more than one patch
				this->pull_ready_data();	
				while (!this->buy_queue_.empty()){this->buy_queue_.pop();}
				while (!this->sell_queue_.empty()){this->sell_queue_.pop();}
			}
			else if (strcmp(data.instrument,this->sell_queue_.back().instrument)>=0){// in one patch
				this->pull_ready_data();	
			}
		}
		
		this->sell_queue_.push(data);
		if (data.count < MAX_PAIR){ this->pull_ready_data(); }

		this->victim_ = "";
	}
	else{ // discard victim data }
}

// ok
void repairer::flag_damaged_data()
{
	if (!this.buy_queue_.empty()){
		if (this->buy_queue_.back().count==MAX_PAIR){
			this->buy_queue_.back().damaged_ = true;
		}
	}

	if (!this.sell_queue_.empty()){
		if (this->sell_queue_.back().count==MAX_PAIR){
			this->sell_queue_.back().damaged_ = true;
		}
	}
}

// ok
void repairer::proc_pkg_loss(MDPack &data)
{
	this->victim_ = data.instrument;

	this->flag_damaged_data();

	if (SHFE_FTDC_D_Buy==data.direction){
		if (!this.buy_queue_.empty()){
			if (strcmp(data.instrument,this->buy_queue_.back().instrument)<0){ // cross more than one patch
				this->pull_ready_data();	
				while (!this->buy_queue_.empty()){this->buy_queue_.pop();}
				while (!this->sell_queue_.empty()){this->sell_queue_.pop();}
			}
		}

	}
	else if (SHFE_FTDC_D_Sell==data.direction){
		if (!this.sell_queue_.empty()){
			if (strcmp(data.instrument,this->sell_queue_.back().instrument)<0){ // corss more than patch
				this->pull_ready_data();	
				while (!this->buy_queue_.empty()){this->buy_queue_.pop();}
				while (!this->sell_queue_.empty()){this->sell_queue_.pop();}
			}
			else if (strcmp(data.instrument,this->sell_queue_.back().instrument)>=0){ // in one patch
				this->pull_ready_data();	
			}
		}
	}
}


void repairer::next(MDPackEx &data, bool empty)
{
	empty = true;

	if (!this->ready_queue_.empty()){
		data = this->ready_queue_.front();
		empty = false;
		this->ready_queue_.pop();
		return;
	}
}

// ok
// entrance of repairer class
void repairer::rev(MDPack &data)
{
	int new_sn_ = data.seqno / 10;
	if (new_sn_ !=seq_no_ + 1) {
		MY_LOG_WARN("seq no from %d to %d, packet loss", seq_no_,new_sn);
	}


	if (!this->working_){ // find normal data start point
		this->working_ = this->find_start_point(data);

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
					this->repaire_buy_data(data);
				}
				else if (SHFE_FTDC_D_Sell==data.direction){
					this->repaire_sell_data(data);
				}
			} // end data repaireing procedure
		} // end package non-loss procedure
	} // end enter receiving process

	this->seq_no_ = new_sn;
}
