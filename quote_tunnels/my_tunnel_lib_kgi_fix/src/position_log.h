#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <my_trade_tunnel_struct.h>

class position_log {
public:
	position_log(const std::string &file_name);
	~position_log();
	void add_new_trade(const T_TradeReturn &trade);
	void get_position(std::vector<PositionDetail> &d);

private:
	std::thread *write_;
	int read_data();
	void write_data();
	volatile bool running_;
	volatile bool updated_;
	std::mutex data_mutex_;
	std::string file_name_;
	std::vector<PositionDetail> datas_;
	std::condition_variable cv_;
};
