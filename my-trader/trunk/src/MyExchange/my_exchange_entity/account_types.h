#pragma once

namespace trading_channel_agent
{
	enum account_types
	{
		/*
		投机
		*/
		speculate = '0',

		/*
		保值
		*/
		hedging = '1',

		/*
		套利
		*/
		arbitrage = '2'

	};
}
