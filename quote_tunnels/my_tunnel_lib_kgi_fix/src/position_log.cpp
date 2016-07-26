#include "position_log.h"

#include <fstream>
#include <thread>
#include <string.h>
#include <iostream>
#include <iomanip>

#include "my_cmn_log.h"

int position_log::read_data() {
	std::ifstream f_in(file_name_);
	if (!f_in) {
		return -1;
	} else {
		std::string line;
		getline(f_in, line);
		PositionDetail item;
		while (f_in >> item.stock_code) {
			if (f_in.fail()) {
				TNL_LOG_ERROR("read StockCode data failed");
				return -1;
			}
			f_in >> item.exchange_type;
			if (f_in.fail()) {
				TNL_LOG_ERROR("read ExchangeType data failed");
				return -1;
			}
			f_in >> item.direction;
			if (f_in.fail()) {
				TNL_LOG_ERROR("read Direction data failed");
				return -1;
			}
			f_in >> item.position;
			if (f_in.fail()) {
				TNL_LOG_ERROR("read Position data failed");
				return -1;
			}
			f_in >> item.position_avg_price;
			if (f_in.fail()) {
				TNL_LOG_ERROR("read PositionAvgPrice data failed");
				return -1;
			}
			f_in >> item.yestoday_position;
			if (f_in.fail()) {
				TNL_LOG_ERROR("read YesterdayPosition data failed");
				return -1;
			}
			f_in >> item.yd_position_avg_price;
			if (f_in.fail()) {
				TNL_LOG_ERROR("read YesterdayPositionAvgPrice data failed");
				return -1;
			}
			f_in >> item.today_buy_volume;
			if (f_in.fail()) {
				TNL_LOG_ERROR("read TodayBuyVolume data failed");
				return -1;
			}
			f_in >> item.today_aver_price_buy;
			if (f_in.fail()) {
				TNL_LOG_ERROR("read TodayBuyAvgPrice data failed");
				return -1;
			}
			f_in >> item.today_sell_volume;
			if (f_in.fail()) {
				TNL_LOG_ERROR("read TodaySellVolume data failed");
				return -1;
			}
			f_in >> item.today_aver_price_sell;
			if (f_in.fail()) {
				TNL_LOG_ERROR("read TodaySellAvgPrice data failed");
				return -1;
			}

			std::lock_guard<std::mutex> dlock(data_mutex_);
			datas_.push_back(item);
		}
		f_in.close();
	}
	return 0;
}

void position_log::write_data() {
	TNL_LOG_INFO("write position data thread start");
	std::vector<PositionDetail> copy;
	while (running_) {
		{
			std::unique_lock<std::mutex> dlock(data_mutex_);
			while (!updated_) {
				cv_.wait(dlock);
			}
			copy.assign(datas_.begin(), datas_.end());
			updated_ = false;
		}
		std::ofstream f_out(file_name_);
		if (!f_out) {
			TNL_LOG_ERROR("write position data: %s failed", file_name_.c_str());
			return;
		}
		f_out << "StockCode ExchangeType Direction "
				<< "Position PositionAvgPrice "
				<< "YesterdayPosition YesterdayPositionAvgPrice "
				<< "TodayBuyVolume TodayBuyAvgPrice "
				<< "TodaySellVolume TodaySellAvgPrice" << std::endl;

		for (auto item : datas_) {
			f_out << left;
			f_out << setw(10) << item.stock_code << setw(13)
					<< item.exchange_type << setw(10) << item.direction
					<< setw(9) << item.position << setw(17)
					<< item.position_avg_price << setw(18)
					<< item.yestoday_position << setw(26)
					<< item.yd_position_avg_price << setw(15)
					<< item.today_buy_volume << setw(17)
					<< item.today_aver_price_buy << setw(16)
					<< item.today_sell_volume << setw(18)
					<< item.today_aver_price_sell << std::endl;
		}
		f_out.close();
	}
	TNL_LOG_INFO("stop writing the position data");
}

position_log::position_log(const std::string &file_name) :
		running_(false), updated_(false), file_name_(file_name) {
	if (read_data() == 0) {
		TNL_LOG_INFO("read position data: %s success", file_name_.c_str());
	} else {
		TNL_LOG_WARN("read position data: %s failed", file_name_.c_str());
	}
	running_ = true;
	write_ = new std::thread(std::bind(&position_log::write_data, this));
}

position_log::~position_log() {
	std::unique_lock<std::mutex> dlock(data_mutex_);
	updated_ = true;
	cv_.notify_all();
	running_ = false;
	write_->join();
}

void position_log::add_new_trade(const T_TradeReturn &trade) {
	auto it = datas_.begin();
	for (; it != datas_.end(); it++) {
		if (strcmp(it->stock_code, trade.stock_code) == 0) {
			break;
		}
	}
	{
		std::unique_lock<std::mutex> dlock(data_mutex_);

		if (it == datas_.end()) {
			PositionDetail item;
			memset(&item, 0, sizeof(item));
			strcpy(item.stock_code, trade.stock_code);
			item.exchange_type = trade.exchange_type;
			item.direction = trade.direction;

			datas_.push_back(item);
			it = datas_.end() - 1;
		}

		if (trade.direction == MY_TNL_D_BUY) {
			it->today_aver_price_buy = ((it->today_aver_price_buy
					* it->today_buy_volume)
					+ (trade.business_price * trade.business_volume))
					/ (it->today_buy_volume + trade.business_volume);
			it->today_buy_volume += trade.business_volume;
		} else {
			it->today_aver_price_sell = ((it->today_aver_price_sell
					* it->today_sell_volume)
					+ (trade.business_price * trade.business_volume))
					/ (it->today_sell_volume + trade.business_volume);
			it->today_sell_volume += trade.business_volume;
		}

		if (it->direction == trade.direction) {
			it->position += trade.business_volume;
		} else {
			if (it->position >= trade.business_volume) {
				it->position -= trade.business_volume;
			} else {
				it->direction = trade.direction;
				it->position = trade.business_volume - it->position;
			}
		}
		updated_ = true;
	}
	cv_.notify_all();
}

void position_log::get_position(std::vector<PositionDetail> &d) {
	std::lock_guard<std::mutex> lock(data_mutex_);
	d.assign(datas_.begin(), datas_.end());
}
