#!/usr/bin/env python3

import sys

tones = ["C","C#","D","D#","E","F","F#","G","G#","A","A#"]
twinkle_tones = "ACDEFG"
SAMPLE_RATE = 8000

def get_scale(base):
    res = []
    for tone in twinkle_tones:
        res.append(base * pow(2, tones.index(tone) / 12))
    return res

def get_cycles(scale):
    return [round(SAMPLE_RATE / f) for f in scale]

def get_score(base):
    return max(abs(f - SAMPLE_RATE / round(SAMPLE_RATE / f)) for f in get_scale(base))

def print_scale(base):
    for tone in twinkle_tones:
        f = base * pow(2, tones.index(tone) / 12)
        cyc = round(SAMPLE_RATE / f)
        err = abs(f - SAMPLE_RATE / cyc)
        print(f"{tone:<2s}: {round(f):4d}Hz -> {cyc:3d} cycles, error {round(err,1)}Hz")

def roman(num):
    res = ''
    for dig,val in [('m', 1000),('c', 100), ('x',10)]:
        res += dig * (num // val)
        num %= val
    return res + ('i' * num)

def twinkle(base, fname):
    score = "ccggaaGffeeddC"
    cycles = dict(zip(twinkle_tones, get_cycles(get_scale(base))))
    with open(fname, "w") as out:
        for t in score:
            cyc = roman(cycles[t.upper()])
            l = 3
            if t.isupper():
                l *= 2
            lp = l // 3
            print(f"g {cyc}\ns {'m'*l}\nG\ns {'m'*lp}", file=out)

def main():
    scores = []
    for base in range(100, 8000):
        scores.append((get_score(base), base))
    scores.sort()
    print("10 best base frequencies for \"C\":")
    for score,base in scores[:10]:
        print(f"{base} Hz: {score}")

    print_scale(scores[0][1])
    twinkle(scores[0][1], "generated_twinkle.txt")

if __name__=="__main__":
    sys.exit(main())
