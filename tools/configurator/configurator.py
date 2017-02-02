#!/usr/bin/python
# -*- coding: UTF-8 -*-

import datetime
import xml.etree.ElementTree as ET
from datetime import date
import os
import shutil
import csv

src_config_file = '../trasev.config'
cur_config_file = 'trasev.config'
stra_setting = 'stra_settings.csv'

def main():
	shutil.copyfile(src_config_file, 'trasev.config')
	backup()

	tree = ET.parse(cur_config_file)
	root = tree.getroot()
	update(root)

	tree.write(cur_config_file, encoding="utf-8") #, xml_declaration=True) 
	shutil.copyfile(cur_config_file, src_config_file)

def backup():
	count = 1
	today = date.today()
	today_str = today.strftime("%Y%m%d")
	bk = 'trasev_{0}_{1}.config'.format(today_str,count)
	while(os.path.exists(bk)):
		count = count + 1
		bk = 'trasev_{0}_{1}.config'.format(today_str,count)
	shutil.copyfile(cur_config_file, bk)

def update(root):
	clear(root)

	strategies = root.find("./strategies")
	# find a strategy element as template
	strategy_temp = strategies.find("./strategy")

	# skip the first row, title row
	with open(stra_setting) as csvfile:
		reader = csv.reader(csvfile)
		id = 10000000
		for row in reader:
			if id == 10000000:
				id = id + 1
				continue
			add_strategy(strategies, strategy_temp, row, id)
			id = id + 1

	# remove template element
	strategies.remove(strategy_temp)

	model_ids = ''
	for strategy in strategies.findall('./strategy'):
		model_ids += str(strategy.get('id'))
		model_ids += ',' 
	model_ids = model_ids.rstrip(',')

	source = root.find("./tca/source")
	source.set('models', model_ids)

def add_strategy(strategies, strategy_temp, row, id):
	strategy_tmp_str = ET.tostring(strategy_temp, encoding="utf-8")
	new_strategy = ET.fromstring(strategy_tmp_str)
	strategies.append(new_strategy);

	new_strategy.set('id', str(id)) 
	new_strategy.set('model_file', row[1]) 

	# respectively copy ev file for every fl50 or fl51 strategry
	ev_name_value = "ev/" + row[1] + ".txt"
	new_strategy.set('ev_name', ev_name_value) 
	strategy_name = row[1]
	ev_file_src = ""
	ev_file_dest = "../" + ev_name_value 
	if 0==strategy_name.find('fl50'):
		ev_file_src = "fl50.txt"
		shutil.copyfile(ev_file_src, ev_file_dest)
	if 0==strategy_name.find('fl51'):
		ev_file_src = "fl51.txt"
		shutil.copyfile(ev_file_src, ev_file_dest)


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

#country_str = ET.tostring(countrys[0])
#new_country = ET.XML(country_str)
#root.append(new_country)
#
