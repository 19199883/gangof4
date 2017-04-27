#ifndef REPAIRER_H
#define REPAIRER_H
#include <string>
#include <queue>

#include "quote_datatype_shfe_my.h"
#include "quote_datatype_shfe_deep.h"

using namespace std;

class MDPackEx
{
	public:
		MDPackEx(): damaged(false)
		{

		}

		MDPack content;
		bool damaged;
};

/*
 * this class repaires data damaged for udp package loss.
 * for more detail to see 上期全挡行情(my_quote_shfe_my_jr_ex-spd_lib).docx
 */
class repairer
{
	public:
		repairer(int svrid);

		/* receive data from UDP. repaires data if it is damaged
		 */
		void rev(MDPack &data);

		/*		 
		 * get next avaialble data
		 * return null if no available data
		 *
		 */
		MDPackEx next();
	private:
		/*		 
		 * find normal data start point when system starts
		 */
		bool find_start_point(MDPack &data);

		/* check whether package loss occurs
		 * return true if package loss occurs; otherwise, false
		 */
		bool lose_pkg((MDPack &data);

		/* 
		 */
		void normal_proc_buy_data(MDPack &data);
		void repair_buy_data(MDPack &data);
		void normal_proc_sell_data(MDPack &data);
		void repair_sell_data(MDPack &data);
		void proc_pkg_loss(MDPack &data);

		// record the contract of victim data when package loss occurs
		string victim_;

		queue buy_queue_;
		queue sell_queue_;

		// initial value is false; it will become true when normal data start point is found.
		bool working_;
		// record current serial number of UDP data	
		int seq_no_;

		// udp server id
		int server_;
};


#endif // REPAIRER_H

