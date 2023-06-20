#!/usr/bin/python3

# gen_slr_table.py : SLR(1) parser table generator

from dataclasses import dataclass

input_grammar = ["E' -> E", "E -> E + T", "E -> E - T", "E -> T", "T -> ( E )", "T -> i"]

@dataclass
class Grammar:
    left: str
    right: list[str]

@dataclass
class GrammarSet:
    grammars:   list[Grammar]
    symbols: set[str]
    nonterminals: set[str]
    terminals: set[str] 

@dataclass
class Item:
    grammar: Grammar
    dot: int



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

def tokenize(grammer: list[str], is_argumented: bool = False) -> GrammarSet:
    result: GrammarSet = GrammarSet([], set(), set(), set())
    symbols: set[str] = set()
    nonterminals: set[str] = set()
    terminals: set[str] = set()

    for line in grammer:
        
        check_line_or_exit(line)
        left, right = line.split("->")
        left, right = left.strip(), right.strip()
        result.grammars.append(Grammar(left, []))
        nonterminals.add(left)
        symbols.add(left)
        for token in right.split(" "):
            symbols.add(token)
            result.grammars[-1].right.append(token)

    if not is_argumented:
        result.insert(0, Grammar(result[0].left + "'", [result[0].left]))

    terminals = symbols - nonterminals
    result.symbols = symbols
    result.nonterminals = nonterminals
    result.terminals = terminals
    return result


grammar_set = tokenize(input_grammar, True)

print(grammar_set)

def closure(item_set: set[Item], grammar_set: GrammarSet) -> set[Item]:
    result: set[Item] = set()
    pre_added: set[Item] = set()
    added: set[Item] = [item_set]
    while len(added) > 0:
        pre_added = added
        added = set()
