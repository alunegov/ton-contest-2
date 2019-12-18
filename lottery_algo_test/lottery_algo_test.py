import random

max_participants_count = 1000000
max_num = 99
nums_count = 3
lucky_nums_count = 13
ticket_price = 1
rounds_count = 24

def gen_nums(count):
    nums = []
    for j in range(count):
        while True:
            new_num = random.randint(0, max_num)
            if new_num not in nums:
                break
        nums.append(new_num)
    return nums

def run_round(prev_balance, prev_round_prize_fund):
    participants_count = random.randint(0, max_participants_count)
    print("participants_count = {0}".format(participants_count))

    round_income = ticket_price * participants_count

    next_balance = prev_balance + round_income

    prize_fund = prev_round_prize_fund + round_income
    print("prize_fund = {0} ({1} + {2})".format(prize_fund, round_income, prev_round_prize_fund))

    three_prize_fund = prize_fund * 0.60
    two_prize_fund = prize_fund * 0.20
    one_prize_fund = prize_fund * 0.17
    # round_income <= prize_fund
    commision = round_income * 0.03
    #commision = prize_fund * 0.03

    participants_nums = []

    for i in range(participants_count):
        nums = gen_nums(nums_count)
        participants_nums.append(nums)

    lucky_nums = gen_nums(lucky_nums_count)
    print(lucky_nums)

    participants_match_nums_count = []
    for i in range(participants_count):
        participants_match_nums_count.append(0)

    for i in range(participants_count):
        for j in range(nums_count):
            if participants_nums[i][j] in lucky_nums:
                participants_match_nums_count[i] = participants_match_nums_count[i] + 1

    got_three = []
    got_two = []
    got_one = []

    for i in range(participants_count):
        if participants_match_nums_count[i] == 3:
            got_three.append(i)
        elif participants_match_nums_count[i] == 2:
            got_two.append(i)
        elif participants_match_nums_count[i] == 1:
            got_one.append(i)

    #print(got_three)
    #print(got_two)
    #print(got_one)

    prizes_sum = 0
    if len(got_three) != 0:
        three_prize = three_prize_fund / len(got_three)
        prizes_sum += three_prize_fund
    else:
        three_prize = -three_prize_fund
    if len(got_two) != 0:
        two_prize = two_prize_fund / len(got_two)
        prizes_sum += two_prize_fund
    else:
        two_prize = -two_prize_fund
    if len(got_one) != 0:
        one_prize = one_prize_fund / len(got_one)
        prizes_sum += one_prize_fund
    else:
        one_prize = -one_prize_fund

    next_balance -= prizes_sum
    next_round_prize_fund = prize_fund - prizes_sum - commision

    print("{1} participants guess three nums and won {0}".format(three_prize, len(got_three)))
    print("{1} participants guess two nums and won {0}".format(two_prize, len(got_two)))
    print("{1} participants guess one num and won {0}".format(one_prize, len(got_one)))
    print("next_round_prize_fund = {0}".format(next_round_prize_fund))
    print("commision = {0}".format(commision))

    return (next_balance, next_round_prize_fund, commision)

random.seed()

balance = 0
var_prize_fund = 0
sum_commision = 0
for i in range(rounds_count):
    print("")
    print("run round {0}".format(i))
    (balance, var_prize_fund, commition) = run_round(balance, var_prize_fund)
    sum_commision += commition

print("")
print("balance = {0}".format(balance))
print("sum_commision = {0}".format(sum_commision))
