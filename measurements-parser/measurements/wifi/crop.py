import sys


def to_remove(index):
	return index >= 1100 and index <= 8490


def build_line(array):
	i = 0
	str = ""
	for x in array:
		if i != 0:
			str += ','
		str += x
		i += 1
	return str


threshold = 0.004
activated = False
file = sys.argv[1]
new_file = file.split('.')[0] + "-crop.csv"

i = 1
j = 1
with open(file, 'r') as f:
	with open(new_file, 'w+') as new_f:
		for line in f:
			index = line.split(',')[0]
			if index.isnumeric():
				index = int(index)
				value = float(line.split(',')[1])
				if not activated and value >= threshold:
					activated = True
				if activated:
					if not to_remove(j):
						split = line.split(',')
						split[0] = str(i)
						new_f.write(build_line(split))
						i += 1
					j += 1
				else:
					new_f.write(line)
					i += 1
			else:
				new_f.write(line)

