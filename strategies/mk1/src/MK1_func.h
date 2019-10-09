#ifndef MK1_FUNC_H
#define MK1_FUNC_H

#include "common.h"
#include "common_mm.h"
#include "interface_v30.h"
#include "MK1_var_def.h"

void s_init_MK1_parameters();
void s_destroy_MK1_parameters();

int update_lastThreeRec(int iBS, double rPrice);

int s_MK1_sig_gen(struct in_data *lstCurrInData, struct in_data *lstPreInData,
	struct all_parameter *lstGlobalPara, st_loc_trade_info *p_stLocTradeInfo,
	int liTickNo, int *laiFullOrderBookBuy, int *laiFullOrderBookSel,
	int liLenFullOrderBook, st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder);

int s_MK1_sig_gen_normal(struct in_data *lstCurrInData, struct in_data *lstPreInData,
	struct all_parameter *lstGlobalPara, st_loc_trade_info *p_stLocTradeInfo,
	int liTickNo, int *laiFullOrderBookBuy, int *laiFullOrderBookSel,
	int liLenFullOrderBook, st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder);

int s_generate_Mk1_cancel_orders(double *larBP, double *larSP, double lrTick, int iCurTime,
    st_loc_trade_info *p_stLocTradeInfo, st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder);

int transfer_2_seconds(int itime);


int s_MK1_eod_square(const struct in_data *lstCurrInData, const struct all_parameter *lstGlobalPara,
	st_loc_trade_info *p_stLocTradeInfo, const int liTickNo,
	st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder);

int s_MK1_eod_square_flat_netpos(const struct in_data *lstCurrInData, const struct all_parameter *lstGlobalPara,
	st_loc_trade_info *p_stLocTradeInfo, const int liTickNo,
	st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder);

int s_MK1_eod_final_square(const struct in_data *lstCurrInData, const struct all_parameter *lstGlobalPara,
	st_loc_trade_info *p_stLocTradeInfo, const int liTickNo,
	st_usr_order *stCurUsrOrder, int iCurUsrOrder,
	st_usr_order *stNewUsrOrder, int iMaxUsrOrder, int *piNewUsrOrder);

int s_MK1_calc_profitloss(st_order_info *p_stOrder, struct sec_parameter *p_stSecPara, st_loc_trade_info *p_stLocTradeInfo);

int s_MK1_run_each_record(struct signal_t *lastStructTSignal, int *piNoTSignal);
int s_MK1_run_each_record_1lvl(struct signal_t *lastStructTSignal, int *piNoTSignal);

int s_MK1_variable_init(struct in_data *p_stCurrInData, struct all_parameter *p_stGlobalPara);

int s_MK1_common_variable_init(struct in_data *p_stCurrInData, struct all_parameter *p_stGlobalPara);

int s_MK1_output_log(int liTickNo, FILE *liFileNumber);

int s_MK1_set_parameters();

#endif
