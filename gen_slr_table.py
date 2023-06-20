#!/usr/bin/python3

# gen_slr_table.py : SLR(1) parser table generator


input_grammer = ["E' -> E", "E -> E + T", "E -> E - T", "E -> T", "T -> ( E )", "T -> i"]

def check_line_or_exit(line: str) -> bool:
    if line.count("->") != 1:
        print("Invalid grammer : '->' at " + line)
        exit(1)
    if line.count("|") > 0:
        print("Invalid grammer : '|' at " + line)
        exit(1)
    if line.count(".") > 0:
        print("Invalid grammer : '.' at " + line)
        exit(1)
    return True

def tokenize(grammer: list[str], is_argumented: bool = False) -> tuple[list[list[str]], tuple[set[str], set[str], set[str]]]:
    result: list[list[str]] = []
    symbols: set[str] = set()
    nonterminals: set[str] = set()
    terminals: set[str] = set()
    for line in grammer:
        check_line_or_exit(line)
        left, right = line.split("->")
        left, right = left.strip(), right.strip()
        result.append([left])
        nonterminals.add(left)
        symbols.add(left)
        for token in right.split(" "):
            symbols.add(token)
            result[-1].append(token)
        

    terminals = symbols - nonterminals

    return result, (symbols, nonterminals, terminals)


tokens, symbols = tokenize(input_grammer)

print(tokens)
print(symbols[0])
print(symbols[1])
print(symbols[2])
