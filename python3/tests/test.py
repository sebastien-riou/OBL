import sys,os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__),'..')))

import obl

port = 5000
if len(sys.argv)>1:
    port = int(sys.argv[1])
o=obl.Obl.newWithSocket(port=port)

info = o.info()
buf=int(info['stack_buf'],0)
print("%x"%buf)

print(o.read(address=buf))
