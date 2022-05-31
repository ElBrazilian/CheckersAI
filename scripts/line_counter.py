import os
import sys
sys.setrecursionlimit(5000)


searchedExt = ['.c', '.h', '.py']
excludes = []

lines = 0
numfiles = 0
numdirs = 0
numfileswithext = 0
def parseDir(dir):
    print('Parsing :',dir)
    global lines, numfiles, numdirs, numfileswithext
    currentDir = os.listdir(dir)
    files = []
    dirs = []
    others = []

    for file in currentDir:
        if os.path.isfile(os.path.join(dir,file)):
            files.append(file)
        elif os.path.isdir(os.path.join(dir,file)):
            dirs.append(file)
        else:
            others.append(file)

    for file in files:
        filename, ext = os.path.splitext(file)
        if ext in searchedExt:
            numfileswithext += 1
            with open(os.path.join(dir,file),'r') as file:
                fileLines = len(file.readlines())
                lines += fileLines

    for nd in dirs:
        if nd in excludes:
            continue
        parseDir(os.path.join(dir, nd))

    numfiles += len(files)
    numdirs += len(dirs)

if __name__ == '__main__':
    parseDir(os.getcwd())
    print('{} files found ({} with the right extension) in {} folders'.format(numfiles, numfileswithext, numdirs))
    print(lines, 'lines from the current working directory ({})'.format(os.getcwd()))
