#!/usr/bin/python
# -*- encoding: utf-8 -*-
import os
import sys
import time

def generateRecordsHeaderInclude(folderPath, module, version, info):
    timeStr = time.strftime("%Y-%m-%d %H:%M:%S")
    fileName = "%s_version.h"%(module)
    try:
        filePath = os.path.join(folderPath, fileName)
        with open(filePath, "w") as file:
            file.write("#ifndef _%s_VERSION_H_\r\n" %module.upper())
            file.write("#define _%s_VERSION_H_\r\n" %module.upper())
            file.write("\r\n")
            
            file.write("#define %s_VERSION    \"%s %s %s\"\r\n" %(module.upper(), version, timeStr, info))

            file.write("\r\n#endif /* _%s_VERSION_H_ */\r\n\r\n" %module.upper()) 
    except:
        print("create file %s error" % filePath)
        return

    print("create file %s success!" % filePath)
    
    

if __name__ == '__main__':
    if len(sys.argv) < 5: 
        print("python %s path module version info"%sys.argv[0])
        sys.exit()
    script_name = sys.argv[0] #第一个参数指的是脚本名称
    folderPath = sys.argv[1]  #第二个参数，此时也是一个str列表
    module = sys.argv[2] #第三个参数 
    version = sys.argv[3]
    info = sys.argv[4]
    #print("params : %s %s %s %s" %(script_name, folderPath, module, version, info))
    #print('folderPath = ' + folderPath)
    
    generateRecordsHeaderInclude(folderPath, module, version, info)
