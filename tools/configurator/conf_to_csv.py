#!/usr/bin/python
# -*- coding: UTF-8 -*-

import datetime
import xml.etree.ElementTree as ET
from datetime import date
import os
import shutil
import csv

def main():
	convert()

def convert(root):
	src= './trasev.config'
	tree = ET.parse(src)
	root = tree.getroot()
	strategies = root.find("./strategies")
	# find a strategy element as template
	strategy_temp = strategies.find("./strategy")

	# skip the first row, title row
	dest = 'stra_settings.csv'
	with open(stra_setting) as csvfile:
		reader = csv.reader(csvfile)
		for row in reader:
			strategy_tmp_str = ET.tostring(strategy_temp, encoding="utf-8")
			new_strategy = ET.fromstring(strategy_tmp_str)
			strategies.append(new_strategy);
			new_strategy.set('id', str(id)) 
			new_strategy.set('model_file', row[1]) 
			symbol = new_strategy.find("./symbol")
			symbol.set('max_pos', row[3])
			symbol.set('name', row[2])

def clear(root):
	'''
	removes strategy elements from trasev.config until there is 
	a strategy element left.
	'''
	stras = root.findall("./strategies/strategy")
	stra_cnt = len(stras);
	if stra_cnt > 1:
		del stras[0]
	elif stra_cnt == 0:
		raise Exception('There is NOT one strategy element!')

	strategies_e = root.find("./strategies")
	for stra in stras:
		strategies_e.remove(stra)


if __name__=="__main__":   
	main()

