# read my_exchange_fut_op_[yymmdd].log, calculate position for every strategy
# and then save these position to files.
#

#!/usr/bin/python
# -*- coding: UTF-8 -*-

from enum import Enum
import xml.etree.ElementTree as ET
from datetime import date

class RecordT(Enum):
	plaOrd  = 'PlaceOrder'
	ordRes = 'OrderRespond'
	ordRtn = 'OrderReturn'
	traRtn = 'TradeReturn'

class SideT(Enum):
	buy  = '0'
	sell = '1'


class PosEfctT(Enum):
	open_pos = '0'
	close_pos =	'1'

class StatusT(Enum):
	# un-report
	unreport = '0' 
	# reported
	reporded = 'a'
	# partial filled
	partial_filled = 'p'
	# filled
	filled = 'c'
	# cancelling
	cancelling = 'f'
	# rejected
	rejected = 'e'
	# cancelled
	cancelled =	'd'

class Strategy:
	def __init__(self,stra_id):
		self.stra_id = stra_id
		self.pos_long = 0
		self.pos_short = 0
		self.stra_name = ""
		self.contract = ""

class PosCalc:
	def __init__(self,stra_id):
		# this field store strategy info, e.g. name,id,position, and so on
		self.straDict = {}

	# get log file name of my_exchange module 
	def getDataSrc(self):
		today = date.today()
		return "my_exchange_fut_op_" + today.strftime("%Y%m%d") + ".log"

	# respectively save position of every strategy 
	# to file which name is as my_exchange_fut_yyyymmdd.log
	def savePos(self):

	# process a line of data frm my_exchange_fut_yyyymmdd.log
	def proc(self,line):
		if -1 != line.find(RecordT.traRtn):
			procTraRec(line)

	# process a line of traded record
	def procTraRed(self,line):
		fields = line.split(';')
		# serial number
		fld0 = fields[0]
		seri_no = fld0[fld0.find('serial_no: ')+11 : ]
		stra_id = seri_no[len(seri_no)-8 : ] 
		targStra = straDict.get(stra_id)
		if None == targStra:
			targStra = Strategy(stra_id)
			straDict[stra_id] = newStra

		# filled quantity
		fld2 = fields[2]
		filled_qty = fld2[fld2.find('business_volume: ')+17 : ]

		# stock code
		fld5 = fields[5]
		stock_code = fld5[fld5.find('stock_code: ')+12 : ]
		targStra.contract = stock_code

		# direction
		fld6 = fields[6]
		dire = fld6[fld6.find('direction: ')+11 : ]
		# position effect
		fld7 = fields[7]
		posEff = fld7[fld7.find('open_close: ')+12 : ]

		if posEff == PosEfctT.open_pos:
			if dire == SideT.buy:
				targStra.pos_long += int(filled_qty)	
			elif dire == SideT.sell:
				targStra.pos_short += int(filled_qty)	
			else:
				print("error:undefined " + dire)

		if posEff == PosEfctT.open_pos:
			if dire == SideT.buy:
				targStra.pos_short 1= int(filled_qty)	
			elif dire == SideT.sell:
				targStra.pos_long -= int(filled_qty)	
			else:
				print("error:undefined " + dire)

	# positions for every strategy
	def calc(self):
		dataSrc = getDataSrc
		with open(dataSrc) as f:
		    for line in f:
		        proc(line)

	# build strategy objects by trasev.config and put them into 
	# strategies member field
	def buildStra(self):
		tree = ET.parse(cur_config_file)
		root = tree.getroot()
		for straEn in root.findall('./stragegy'):
			stra_id = straEn.get('id')
			stra_name = straEn.get('model_file')
			Strategy stra
			stra.stra_id = stra_id
			stra.stra_name = stra_name
			straDict[stra_id] = stra

def main():
	buildStra()
	calc
	savePos

if __name__=="__main__":   
	main()
