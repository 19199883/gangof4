#ifndef COMMON_MOD_H
#define COMMON_MOD_H




int s_read_fut_struct(int liTickNo);
int s_set_global_parameters();

int f_check_valid_data(struct in_data *p_stCurrInData, struct in_data *p_stPreInData,
	int liTickNo, struct all_parameter *p_stGlobalPara);
int f_check_valid_data_1lvl(struct in_data *p_stCurrInData, struct in_data *p_stPreInData,
	int liTickNo, struct all_parameter *p_stGlobalPara);

void mysub0(int liFileNumber, char *lcFileName);


int s_output_log(int liTickNo, FILE *p_iFileNumber, char *lcFileName);

int s_update_full_order_book(struct in_data *lstCurrInData, struct in_data *lstPreInData,
	struct all_parameter *lstGlobalPara,int *laiFullOrderBookBuy, int *laiFullOrderBookSel,
	int liLenFullOrderBook);


#endif
