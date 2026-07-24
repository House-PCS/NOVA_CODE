import re
import os
import shutil
import sys
import tkinter as tk
from tkinter import filedialog, dialog


def change_file():
    netlist_file = input("目标文件为：\n")
    post_fix = input("需要加的后缀为：\n")
    file_name = os.path.basename(netlist_file)
    origin_file = open(file_name, 'r', errors='ignore')
    new_file_name = "new_"+file_name

    change_list=[]
    for i in origin_file:
        if(re.search(r'CHECK_LICENSE_TYPE',i)):
            pattern = str(i).split('CHECK_LICENSE_TYPE = "')
            string = str(pattern[1]).split(',')
            if(string[0] not in change_list):
                change_list.append(string[0])
        elif(re.search(r'^\(\* ORIG_REF_NAME',i)):
            pattern = str(i).split('ORIG_REF_NAME = "')
            string = str(pattern[1]).split('"')
            if(string[0] not in change_list):
                change_list.append(string[0])

    origin_file.close()
    print(change_list)

    #new_file = open(new_file_name, 'w+', errors='ignore')
    shutil.copy(file_name, new_file_name)

    index = 0
    print(index)
    identify = 0

    for index in range(len(change_list)):
        f = open(new_file_name, 'r+')
        all_the_lines = f.readlines()
        f.seek(0)
        f.truncate()
        for line in all_the_lines:
            if(identify==1):
                f.write(line)
            elif (re.search(r'pragma', line)):
                f.write(line)
                identify = 1
            else:
                f.write(line.replace(change_list[index],change_list[index]+'_'+str(post_fix)))
        f.close()
        identify = 0

change_file()
            
