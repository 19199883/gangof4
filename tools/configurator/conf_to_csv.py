#!/usr/bin/python
# -*- coding: UTF-8 -*-

import datetime
import xml.etree.ElementTree as ET
from datetime import date
import os
import shutil
import csv
import sys

def convert():
	src= './trasev.config'
	today = date.today()
	dest = "stra_set" + today.strftime("%Y%m%d") + ".csv"

	tree = ET.parse(src)
	root = tree.getroot()
	with open(dest,'w') as f:
		f.write("{0},{1},{2},{3}\n".format("id", "strategy","contract", "lot")) 
		for straEn in root.findall('.//strategy'):
			stra_id = straEn.get('id')
			stra_name = straEn.get('model_file')
			symbol = straEn.find("./symbol")
			contract = symbol.get("name")
			lot = int(symbol.get("max_pos"))
			f.write("{0},{1},{2},{3}\n".format(stra_id,stra_name,contract,lot))

def main():
	os.chdir(sys.path[0])
	convert()

if __name__=="__main__":   
	main()

