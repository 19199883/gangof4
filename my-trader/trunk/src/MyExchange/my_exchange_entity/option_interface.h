#if !defined(__OPTION_INTERFACE_H_20150722__)
#define __OPTION_INTERFACE_H_20150722__

//libc headers
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

//  MACROS
#define FAILURE (-1)
#define ARRAY_SIZE(a)   (sizeof(a)/sizeof(a[0]))


// Data Type Declaration.
/*
 * NOTE:
 *  1) fortran中real类型C中使用double表示
 *  2）fortran中logical类型C中使用int32_t表示,取值为0或1
 *  3）fortran中integer类型C中使用int32_t表示
 *
 *
 * */
#define UNDERLINE_MAX  	(8)
#define STRIKE_MAX  	(30)
#define TYPE_MAX  	(2)
#define CHAR_LEN	(12)
#define STR_VEC_LEN	(64)

#pragma pack(push)
#pragma pack(8)


/*
 * update before trding (only once) (from trader to so)
 */
struct startup_init_t
{
    char uu[UNDERLINE_MAX * STR_VEC_LEN];
    char tt[TYPE_MAX * STR_VEC_LEN];
    int32_t ss[STRIKE_MAX];
    int32_t ss_band_e[UNDERLINE_MAX * 2];
    int32_t ss_band_e_num[UNDERLINE_MAX];
    double par_percent[7];
    double par_opt_floor_price[2];
    double pricing_par_table[UNDERLINE_MAX*STRIKE_MAX];
    double rr[7*2];
    double trigger[9];
    double par_simle_delta_coeff[UNDERLINE_MAX];
    char par_simle_for_delta[1];
    double max_num_optimisation;
    int32_t max_trade_qty[UNDERLINE_MAX * TYPE_MAX * STRIKE_MAX];
    int32_t max_trade_fut[UNDERLINE_MAX];
    double vol_avg_freq;
    double base_avg_freq;
    int32_t pingpong_time_range;
    double underlying_change_rate_pingpong;
    int32_t pos_option_yesterday[UNDERLINE_MAX * TYPE_MAX * STRIKE_MAX];
    int32_t pos_underlying_yesterday[UNDERLINE_MAX];
    double price_option_yesterday[UNDERLINE_MAX * TYPE_MAX * STRIKE_MAX];
    double price_underlying_yesterday[UNDERLINE_MAX];
    char today_tarding_symbol[STR_VEC_LEN];
    char main_contracts_string[STR_VEC_LEN];
    double yesterday_settlement_price;
    int32_t trade_num;
    double var_init[20];
    char var_par[20*4*STR_VEC_LEN];
    int32_t orders_speed_check_timerange;
    int32_t max_trade_samples_num;
    int32_t orders_delay_check_time;
    int32_t min_contributor_volume;
    int32_t one_second_uplimit_orders;
    double stall[20];
    int32_t stall_num[20];
    double stall_layer[20];
    double stall_weight[20];
    int32_t par_sequence_num[7];
};

/***
 * update periodically during trading (from trader to so )
 */
struct ctrl_t
{
    char uu[UNDERLINE_MAX * STR_VEC_LEN];
    char tt[TYPE_MAX * STR_VEC_LEN];
    int32_t ss[STRIKE_MAX];
    int32_t ss_band_e[UNDERLINE_MAX * 2];
    int32_t ss_band_e_num[UNDERLINE_MAX];
    double aggressivity;
    double trading_restrict_table[UNDERLINE_MAX*TYPE_MAX*STRIKE_MAX*3];
    int32_t should_forget_convex; //logical
    int32_t should_forget_smile;  //logical
    int32_t max_order;
    double fut_weight;
    double alpha_Bary_mkt;
    double alpha_Residus;
    double base_init;
    double target[5];
    double delta_max;
    double offset_restrictions[UNDERLINE_MAX*5*8];
    double pose_supp[8];
    double min_gain_opt[UNDERLINE_MAX * STRIKE_MAX * 2];
    double min_gain_fut[UNDERLINE_MAX];
    char order_control[CHAR_LEN];
    int32_t fitting_num_check_button[UNDERLINE_MAX];
    int32_t trading_button[UNDERLINE_MAX];
    int32_t quote_button[UNDERLINE_MAX];
    double contributor_limit[2 * 2];
    char par_en_pricing_in_options[1];
    char par_en_in_options[1];
    int32_t is_cutting_model_long; //logical
    int32_t is_cutting_model_short; //logical
    double mg_contr[UNDERLINE_MAX * 6]; //contrib related
    double p_smooth_smile_entry;        //contrib related

    double aggresivity;
    double trigger_base;
    double trigger_cov;
    double trigger_DD;
    double trigger_destriker;
    double trigger_DU;
    double trigger_mat;
    double trigger_nu;
    double trigger_spot;
    double trigger_vega;
    double realised_smile_coeff;
    double smile_vol_shift;
    int32_t pingpong_time_range;
    double underlying_changerate_pingpong;
    int32_t max_trade_qty_fut[UNDERLINE_MAX];
    double opt_call_floor;
    double opt_put_floor;
    double volAvg_alpha;
    double baseAvg_alpha;
};

struct monitor_t
{
    char uu[UNDERLINE_MAX * STR_VEC_LEN];
    char tt[TYPE_MAX * STR_VEC_LEN];
    int32_t ss[STRIKE_MAX];
    int32_t ss_band_e[UNDERLINE_MAX * 2];
    int32_t ss_band_e_num[UNDERLINE_MAX];
    int32_t tw_auto_status; //logical
    double impli[UNDERLINE_MAX * 6];
    double init[UNDERLINE_MAX * 7];
    double final[UNDERLINE_MAX * 7];
    double opt_final[UNDERLINE_MAX * STRIKE_MAX * 2];
    double fut_final[UNDERLINE_MAX];
    double opt[UNDERLINE_MAX * STRIKE_MAX * 8];
    double par_pose[UNDERLINE_MAX * 10];
    int32_t pos[UNDERLINE_MAX * TYPE_MAX * STRIKE_MAX];//positions
    int32_t pos_option_today[UNDERLINE_MAX * TYPE_MAX * STRIKE_MAX*2];
    double price_option_today[UNDERLINE_MAX * TYPE_MAX * STRIKE_MAX*2];
    int32_t underlying[UNDERLINE_MAX];
    int32_t pos_underlying_today[UNDERLINE_MAX * 2];
    double price_underlying_today[UNDERLINE_MAX * 2];
    double PL_underlying[UNDERLINE_MAX *3];
    double PL_option[UNDERLINE_MAX * TYPE_MAX * STRIKE_MAX *3];
    double accum_PL_option[UNDERLINE_MAX * 3];
    double best_converse[2];
    double cur_Bary_fut[UNDERLINE_MAX];
    double cur_base[UNDERLINE_MAX];
    int32_t real_maturity;
    int32_t ord_opt_qty[UNDERLINE_MAX * STRIKE_MAX * 2 * 2];
    int32_t ord_future_qty[UNDERLINE_MAX * 2];
    double fut_BSPV[UNDERLINE_MAX * 4];
    double opt_BSPV[UNDERLINE_MAX * STRIKE_MAX * 2 * 4];
    double opt_init[UNDERLINE_MAX * STRIKE_MAX * 2]; 

    int32_t pos_option_yesterday[UNDERLINE_MAX * TYPE_MAX * STRIKE_MAX];
    int32_t pos_underlying_yesterday[UNDERLINE_MAX];
    double price_option_yesterday[UNDERLINE_MAX * TYPE_MAX * STRIKE_MAX];
    double price_underlying_yesterday[UNDERLINE_MAX];
    double opt_total_fee;
    double fut_total_fee;
    double PL_total_opt[UNDERLINE_MAX * 2];
};


#pragma pack(pop)

#endif
