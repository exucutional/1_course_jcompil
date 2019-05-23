simple akk(simple m, simple n):
		? (m == 0):
				out n + 1|
		@|
		? (n == 0):
				out akk(m - 1, 1)
		@|
		out akk(m - 1, akk(m, n - 1))|
@

simple main():
		out akk(3, 11)|
@ 
