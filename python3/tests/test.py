import sys,os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__),'..')))

import obl

port = 5000
if len(sys.argv)>1:
    port = int(sys.argv[1])
o=obl.Obl.newWithSocket(port=port)

dat=o.ba("00112233445566778899AABBCCDDEEFF")
buf=int(o.info()['stack_buf'],0)
print("%x"%buf)
o.write(data=dat,addr=buf)
o.call(addr=int(o.info()['obl_main'],0),wait_status=False,wait_data=True)
print(o.read(addr=buf,size=16))
assert(dat[3:4]==o.read(addr=buf+3))
o.exit(0)
o.rawRx(length=2) #discard extra \n> after exit of nested OBL
assert(dat[10:11]==o.read(addr=buf+10))
print(o.info())

assert(dat[8:12]==o.read(addr=buf+8, size=4))
assert(dat[8:12]==o.read(addr=buf+8, size=4,access_width=16))
assert(dat[8:12]==o.read(addr=buf+8, size=4,access_width=32))
assert(dat[8:16]==o.read(addr=buf+8, size=8,access_width=64))

o.write(addr=buf+2, data=dat, access_width=16)
assert(dat==o.read(addr=buf+2,size=len(dat)))

o.write(addr=buf+4, data=dat, access_width=32)
assert(dat==o.read(addr=buf+4,size=len(dat)))

o.write(addr=buf+8, data=dat, access_width=64)
assert(dat==o.read(addr=buf+8,size=len(dat)))

o.exit()

print("test PASS")
