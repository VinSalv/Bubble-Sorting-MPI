# 
# Course: High Performance Computing 2021/2022
# 
# Lecturer: Francesco Moscato	fmoscato@unisa.it
#
# Group:
# Lamberti      Martina     0622701476  m.lamberti61@studenti.unisa.it
# Salvati       Vincenzo    0622701550  v.salvati10@studenti.unisa.it
# Silvitelli    Daniele     0622701504  d.silvitelli@studenti.unisa.it
# Sorrentino    Alex        0622701480  a.sorrentino120@studenti.unisa.it
#
# Copyright (C) 2021 - All Rights Reserved
#
# This file is part of EsameHPC.
#
# Contest-MPI is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Contest-MPI is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Contest-MPI.  If not, see <http://www.gnu.org/licenses/>.
#

#
#    @file extract.py
#

# PURPOSE OF THE FILE: Extraction of measures and relative plots. 

import os
import numpy as np
import scipy as sp
import matplotlib.pyplot as plt
import pandas as pd
from scipy import stats
import seaborn as sns
from prettytable import PrettyTable
from prettytable import MARKDOWN
from prettytable import MSWORD_FRIENDLY
import re

config = {
			'init':{
				'png':False,
				'speedup':False
			},
			'radix_sort':{
				'png':False,
				'speedup':False
			},
			'user':{
				'png':False,
				'speedup':False
				},
			'sys':{
				'png':False,
				'speedup':False
				},
			'elapsed':{
				'png':True,
				'speedup':True
				}
		}

def _extract(path_to_folder,plot_columns):
	prev = os.getcwd()
	os.chdir(path_to_folder)
	#List diresctory
	filenames =  [f for f in os.listdir('.') if os.path.isfile(f)]
	if not os.path.exists("png"):
		os.mkdir("png")
	#Remove not csv files
	filenames = [f for f in os.listdir('.') if f.endswith(".csv") and re.match("SIZE-[0-9]+-NTH-[0-9]{2}-O[0-9]-?[0-9]*",f) ]
	print(filenames)
	filenames = sorted(filenames)
	means = {}
	for filename in filenames:
		file_mean = {}
		print('Processing : ' + filename)
		ds = pd.read_csv(filename)
		for col in plot_columns.keys():
			print('Processing : ' + filename + ", Col : " + col)
			x_data = ds[col]
			mean,std=stats.norm.fit(x_data)
			x_data = ds[(ds[col] < (mean + std)) & (ds[col] > (mean - std))][col]
			mean,std=stats.norm.fit(x_data)
			file_mean[col] = mean
			
			if plot_columns[col]['png']:
				sns.histplot(x_data, kde=True)
				plt.savefig("png/" + str(col)+ "_" + filename.split('.')[0] + ".png")
				plt.close()
		means[filename] = file_mean
	os.chdir(prev)
	return means

def _compute_speedup(t,tp,nt,psize):
	speedup = t/tp
	efficiency = t/(tp*float(nt))
	return speedup,efficiency

def _make_table(header,rows,print_table=False,save=True,name=""):
	if save and not name:
		raise Exception("No filename to save file")
	x = PrettyTable()
	x.field_names = header
	x.add_rows(rows)
	if save:
		_save_table(x,name)
	if print_table:
		print(x)
	return x

def _save_table(table,filename):
	with open(filename,"w") as table_file:
		#table.set_style(MARKDOWN)
		table.set_style(MSWORD_FRIENDLY)
		data = table.get_string()
		table_file.write(data)

def _plot_from_table(header,rows,save=True,name="",show_plot=False):
	if save and not name:
		raise Exception("No filename to save file")
	x = [0]
	y = [0]
	speedup_pos = header.index("Speedup")
	thread_pos = header.index("Threads")
	for row in rows[1:]:
		x.append(row[thread_pos])
		y.append(row[speedup_pos])
	x_th = np.array(x)
	fig, ax = plt.subplots(figsize=(12, 8))
	ax.plot(x_th, y, 'ro-', label='Experimental')
	ax.plot(x_th, x_th, color='blue', label='Ideal')
	plt.style.use('seaborn-whitegrid')
	plt.autoscale(enable=True, axis='x', tight=True)
	plt.autoscale(enable=True, axis='y', tight=True)	
	plt.legend()
	plt.xlabel("Processors")
	plt.ylabel("Speedup")
	if show_plot:
		plt.show()
	if save:
		plt.savefig(name)
	plt.close()

def extraction(root=os.path.join(os.path.dirname(os.path.realpath(__file__)),"measure/"), cols=config, threads=[0,1,2,4,8]):
	print("Listing folder for problem size")
	folders =  [f for f in os.listdir(root) if (os.path.isdir(os.path.join(root,f)) and re.match("SIZE-[0-9]+-O[0-9]",f))]
	print(f"Found folders : {folders}")
	for folder in folders:
		print(f"Folder : {folder}")
		joined_path = os.path.join(root,folder)
		means = _extract(joined_path,cols)
		header = {'values':['Version','Threads','Init','Radixsort','User','Sys','Elapsed','Speedup','Efficiency']}
		cells = {'values':[]}
		nt = -1
		for filename_key in means:
			cell = []
			splitted_filename=filename_key.split("-")
			if "NTH-00" in filename_key:
				seq = means[filename_key]['elapsed']
				nt = 1
				cell.append('Serial')
				cell.append(nt)
			else:
				nt = int(splitted_filename[3])
				cell.append('Parallel')
				cell.append(nt)

			for col in cols:
				cell.append(means[filename_key][col])
				if cols[col]['speedup']:
					psize = splitted_filename[1]
					speedup,efficiency = _compute_speedup(seq,means[filename_key][col],nt,psize)
					cell.append(speedup)
					cell.append(efficiency)
			cells['values'].append(cell)
		splitted_folder = folder.split("-")
		size = splitted_folder[1]
		opt = splitted_folder[2]
		table_filename = joined_path + "/psize-" + size + "-" + str(opt) + "-table.csv"
		plot_filename = joined_path + "/speedup-" + str(size) + "-" + str(opt) +  ".png"
		table = _make_table(header['values'],cells['values'],name=table_filename)
		_plot_from_table(header["values"],cells["values"],name=plot_filename)

if __name__ == "__main__":
	extraction()