import os

excludedir = [".\\.git"]

def update_source(filename, oldlicense, license):
    utfstr = chr(0xef)+chr(0xbb)+chr(0xbf)
    fdata = file(filename,"r+").read()
    isUTF = False
    if (fdata.startswith(utfstr)):
        isUTF = True
        fdata = fdata[3:]
    if (oldlicense != None):
        if (fdata.startswith(oldlicense)):
            fdata = fdata[len(oldlicense):]
    print "updating "+filename
    fdata = license + fdata
    if (isUTF):
        file(filename,"w").write(utfstr+fdata)
    else:
        file(filename,"w").write(fdata)

def recursive_traversal(dir,  oldlicense, license):
    global excludedir
    fns = os.listdir(dir)
    print "listing "+dir
    for fn in fns:
        fullfn = os.path.join(dir,fn)
        if (fullfn in excludedir):
            continue
        if (os.path.isdir(fullfn)):
            recursive_traversal(fullfn, oldlicense, license)
        else:
            if (fullfn.endswith(".c") or fullfn.endswith(".cpp") or fullfn.endswith(".h") or fullfn.endswith(".hpp") or fullfn.endswith(".cs") or fullfn.endswith(".js") or fullfn.endswith(".as")):
                update_source(fullfn, oldlicense, license)

oldcright = file("license_old.txt","r+").read()
cright = file("license.txt","r+").read()
recursive_traversal(".\\", oldcright, cright)
exit()
