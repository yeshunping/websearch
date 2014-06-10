fh = open('file.txt')
for line in fh:
    line=line.strip()
    dest = line.replace("easou_", "")
    print str("mv %s %s") % (line, dest)
fh.close()
