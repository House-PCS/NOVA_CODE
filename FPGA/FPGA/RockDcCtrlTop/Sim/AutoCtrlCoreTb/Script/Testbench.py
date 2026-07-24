#   data                                                revise
#   2019.10.16             取消//注释和/* */注释
#   2019.10.18             完善了正则匹配，使支持位宽为参数，且有空格场景，比如[   7:0]写法
#   2019.10.24             完善了正则匹配，支持位宽为多个参数运算的场景，比如reg [ PARAMTER + 1 : 0 ]的写法
#   2019.11.08             完善了正则匹配，支持输入输出wire和reg后没有空格，直接是位宽标识的场景，即reg[3 :0]



#coding=utf8
import re
import uuid
import os
import sys
import shutil
import tkinter as tk
from tkinter import filedialog, dialog

window = tk.Tk()
window.title('bench生成器')  # 标题
w= 350
h = 140
ws =window.winfo_screenwidth()
hs =window.winfo_screenheight()
window.geometry('%dx%d+%d+%d'% (w,h,(ws-w)/2,(hs-h)/2))#先构造一个 frame 框架准备装 frame1 和 frame2
file_path = ''
file_text = ''
text1 = tk.Text(window, width=30, height=2, bg='orange', font=('Arial', 12))
text1.pack()



def open_file():
    '''
    打开文件
    :return:
    '''
    global file_path
    global file_text
    file_path = filedialog.askopenfilename(title=u'选择文件', initialdir=(os.path.expanduser('H:/')))
    print('打开文件：', file_path)
    if file_path is not None:
        with open(file=file_path, mode='r+', encoding='utf-8') as file:
            file_text = file_path
        text1.insert('insert', file_text)

#删除所有的注释
def delete(file_name):
    fdr = open(file_name, 'r', errors='ignore')
    #fdw = open("output" + file_name, 'w’, errors='ignore')
    fdw = open("output" + file_name, 'w', errors='ignore')

    _map = {}
    outstring = ''

    line = fdr.readline()
    while line:
        while True:
            # 这里要注意，我用的是re.S 比如print("aaa\n")
            m = re.compile('\".*\"', re.S)
            _str = m.search(line)
            # 如果没匹配成功，就合并，然后下一行
            if None == _str:
                outstring += line
                break
            key = str(uuid.uuid1())
            #
            m = re.compile('\".*\"', re.S)
            outtmp = re.sub(m, key, line, 1)
            line = outtmp
            _map[key] = _str.group(0)
        line = fdr.readline()

    m = re.compile(r'//.*')
    outtmp = re.sub(m, ' ', outstring)
    outstring = outtmp

    m = re.compile(r'/\*.*?\*/', re.S)
    outtmp = re.sub(m, ' ', outstring)
    outstring = outtmp

    for key in _map.keys():
        outstring = outstring.replace(key, _map[key])

    fdw.write(outstring)
    fdw.close()





def bench():
    verilog_path = os.path.dirname(file_text)
    print('The path is:  '+verilog_path+'\n')
    cur_path = sys.path[0]

    if os.path.exists(verilog_path):
        os.chdir(verilog_path)
        file_name =os.path.basename(file_text)
        delete(file_name)
        top_name = file_name.split('.', 1)
        p = open("output" + file_name, 'r', errors='ignore')
        q = open('tb_' + 'temp', 'w')
        z = open('port.v','w')
        temp = '`timescale 1ns/1ns' + '\n' + \
               '//=================================================' + '\n' + \
               '//  Company             :   NOVASTAR' + '\n' + \
               '//  Engineer            :   name' + '\n' + \
               '//  Module Name         :   ' + 'tb_' + top_name[0] + '\n' + \
               '//  Description         :                             ' + '\n' + \
               '//=================================================' + '\n' + \
               '\n\n\n\n\n' + \
               'module  ' + 'tb_' + top_name[0] + ';' + '\n' + '\n' + \
               '//_________________________________//' + '\n' + \
               '//      dump the fsdb wave           ' + '\n' + \
               '//_________________________________//' + '\n' + \
               'initial' + '\n' + \
               'begin' + '\n' + \
               '    $fsdbDumpfile("test.fsdb");' + '\n' + \
               '    $fsdbDumpvars;' + '\n' + \
               '    $display("***************wave dump start******************",$time);' + '\n' + \
               'end' + '\n\n\n\n\n' + \
               '//---------------------------------//' + '\n' + \
               '//      internal signal              ' + '\n' + \
               '//_________________________________//' + '\n' + \
               '\n'
        q.write(temp)

        #正则匹配`ifdef和input ports
        initial_ports = re.compile('''
                                     (`ifdef|`endif)(\s+\\n|\s+\w+)|
                                     (input)               #2
                                     (\s+)                 #3
                                     (wire\s+|wire|reg\s+|reg)?        #4
                                     (\[.*?\]\s+?)?		   #5
									 (\s+)?      		   #6
                                     (\w+)                 #7
                                     ''', re.VERBOSE)

        #将port申明的部分单独提取出来
        for port in p:
            if(re.search(r'endmodule;',port)):
                break
            else:
                z.write(port)
        z.close()
        with open('port.v', 'r', errors='ignore') as file_obj:
            comment = file_obj.read()

        #找到文件中符合正则表达式的部分
        groups_initial = (initial_ports).findall(comment)

        RegAfterSpace1 = 4      #reg后到申明位宽的空格数
        RegAfterSpace2 = 22     #从位宽申明到信号申明的空格数
        RegAfterSpace3 = 20     #从信号申明到封号的空格数
        InitialAfterSpace = 20  #从初始化信号到等号的空格数
        InstantAfterSpace1= 30  #例化信号到前括号的空格数
        InstantAfterSpace2= 30  #前括号到后括号的空格数


        num = len(groups_initial)
        for i in range(num):
            if(('`ifdef' in groups_initial[i])|('`endif' in groups_initial[i])):
                q.write(groups_initial[i][0]+ groups_initial[i][1]+'\n')
            else: #写入reg申明，且进行对齐操作
                q.write('reg' + (RegAfterSpace1)*' '+groups_initial[i][5] + \
                        (RegAfterSpace2 -len(groups_initial[i][5]))*' '+ groups_initial[i][7] +\
                        (RegAfterSpace3 -len(groups_initial[i][7]))*' '+';\n')

        q.write('\n\n\n')

        q.write('initial begin\n')
        for j in range(num):
            if (('`ifdef' in groups_initial[j]) | ('`endif' in groups_initial[j])):
                q.write(groups_initial[j][0] + groups_initial[j][1] + '\n')
            else: #信号初始化，且进行对齐操作
                q.write(4*' '+ groups_initial[j][7] + (InitialAfterSpace -len(groups_initial[j][7]))*' '+'= 0 ;\n')
        q.write('end\n\n\n')

        clk_reg = '//---------------------------------//' + '\n' + \
                  '//      clk generate                 ' + '\n' + \
                  '//_________________________________//' + '\n' + \
                  'parameter CLOCK_PERIOD = 10;\n' + \
                  'reg          system_clk  ;\n\n'+\
                  'initial begin\n' + \
                  '    system_clk = 0;\n' + \
                  'end\n\n' + \
                  'always #CLOCK_PERIOD system_clk = ~system_clk;\n\n\n' + \
                  '//---------------------------------//' + '\n' + \
                  '//      DUT instantiate              ' + '\n' + \
                  '//_________________________________//' + '\n\n\n' + \
                  top_name[0] + '  ' + 'DUT\n' + '(\n'
        q.write(clk_reg)

        # 正则匹配`ifdef和input/output/inout ports
        regex_ports = re.compile('''
                                    (`ifdef|`endif)(\s+\\n|\s+\w+)|
                                    (input|output|inout)  #2
                                    (\s+)                 #3
                                    (wire\s+|wire|reg\s+|reg)?        #4
                                    (\[.*?\]\s+?)?      #5
                                    (\w+)                 #6
                                    ''', re.VERBOSE)

        groups_ports = regex_ports.findall(comment)

        num = len(groups_ports)
        for k in range(num):
            if ('//' in groups_ports[k]):
                print('have a zhushi')
            else:
                if(('`ifdef' in groups_ports[k])|('`endif' in groups_ports[k])):
                    q.write(groups_ports[k][0] + groups_ports[k][1] + '\n')
                else: #模块DUT例化，且进行对齐操作
                    if(k==num-1):
                        q.write('    .' + groups_ports[k][6]+(InstantAfterSpace1 -len(groups_ports[k][6]))*' '+\
                                '( ' + groups_ports[k][6] + (InstantAfterSpace2 -len(groups_ports[k][6]))*' '+\
                                ')\n')
                    else:
                        q.write('    .' + groups_ports[k][6]+(InstantAfterSpace1 - len(groups_ports[k][6])) * ' '+\
                                '( ' + groups_ports[k][6] + (InstantAfterSpace2 - len(groups_ports[k][6])) * ' '+\
                                '),\n')
        q.write(');' + '\n\n\n' + 'endmodule')

        p.close()
        q.close()
        os.remove('port.v')
        os.remove('output' + file_name)

#避免生成的bench中`ifdef和`endif之间没有字符的情况
        final = open('tb_' + 'temp', 'r')
        point = open('tb_' + file_name, 'w')

        global bypass
        bypass=0
        for line in final:
            if(re.search(r'`ifdef',line)):
                store = line
                bypass=1
            elif(re.search(r'`endif',line)):
                if(bypass==1):
                    bypass=0
                    continue
                else:
                    point.write(line)
            elif(bypass==1):
                bypass=0
                point.write(store)
                point.write(line)
            else:
                bypass=0
                point.write(line)

        point.close()
        final.close()
        os.remove('tb_' + 'temp')

        shutil.move(os.getcwd() + '\\' + 'tb_' + file_name, cur_path + '\\tb_' + file_name)
        print('\n'+'************************************************\n'+\
        'the testbench has been generated!!\n'+\
              '************************************************')




#生成两个button
bt1 = tk.Button(window, text='打开文件', width=15, height=2, command=open_file)
bt1.pack()
bt2 = tk.Button(window, text='生成bench', width=15, height=2, command=bench)
bt2.pack()


window.mainloop()  # 显示
