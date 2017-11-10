from random import randint

x1 = []
x2 = []
x3 = []
x4 = []


for i in range(89):
    x1.append(randint(0,1))

for i in range(97):
    x2.append(randint(0,1))

for i in range(101):
    x3.append(randint(0,1))

for i in range(103):
    x4.append(randint(0,1))


key = []

for i in range(89):
    key.append(x1[i]^x2[i]^(x1[i]*x3[i])^(x1[i]*x2[i]*x4[i]))

col_1 = 0
col_2 = 0
col_3 = 0
col_4 = 0

for i in range(89):
    if x1[i] == key[i]:
        col_1 += 1
    if x2[i] == key[i]:
        col_2 += 1
    if x3[i] == key[i]:
        col_3 += 1
    if x4[i] == key[i]:
        col_4 += 1


print "x1: " , col_1
print "x2: " , col_2
print "x3: " , col_3
print "x4: " , col_4
