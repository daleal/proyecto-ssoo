a="""

"""
b="""

"""

l =a.split("\n")
q = b.split("\n")
print(len(l), len(q))
print(l == q)
if len(l) == len(q):
	for i in range(len(l)):
		if l[i] != q[i]:
			print(l[i], q[i], i)