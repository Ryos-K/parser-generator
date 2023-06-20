/*
gen_slr_table.cpp は SLR(1) パーサのテーブルを生成するプログラムです．
このプログラムを実行すると、標準出力に正準 LR(0) 集成，及び SLR(1) テーブルが出力されます．
実行方法は以下の通りです．
```
    $ g++ gen_slr_table.cpp -o gen_slr_table && ./gen_slr_table
```
実行結果を以下に示します．なお，FOLLOW 集合及び FIRST 集合は ε や相互再帰への対応が困難であったため，手計算により求めました．
```
    Canonical Collection
    I0
    E' -> .E
    E -> .E+T
    E -> .E-T
    E -> .T
    T -> .(E)
    T -> .i

    I1
    E -> .E+T
    E -> .E-T
    E -> .T
    T -> .(E)
    T -> (.E)
    T -> .i

    I2
    T -> i.

    I3
    E' -> E.
    E -> E.+T
    E -> E.-T

    I4
    E -> T.

    I5
    E -> E.+T
    E -> E.-T
    T -> (E.)

    I6
    E -> E+.T
    T -> .(E)
    T -> .i

    I7
    E -> E-.T
    T -> .(E)
    T -> .i

    I8
    T -> (E).

    I9
    E -> E+T.

    I10
    E -> E-T.

    Action Table
    State   +       -       (       )       i       $       E       T
    0                       s1              s2      acc     3       4
    1                       s1              s2              5       4
    2       r5      r5              r5              r5
    3       s6      s7                              r0
    4       r3      r3              r3              r3
    5       s6      s7              s8
    6                       s1              s2                      9
    7                       s1              s2                      10
    8       r4      r4              r4              r4
    9       r1      r1              r1              r1
    10      r2      r2              r2              r2
```
*/

#include <bits/stdc++.h>

struct Rule {
    int id;
    std::string lhs;
    std::vector<std::string> rhs;

    bool operator<(const Rule &rhs) const { return id < rhs.id; }

    bool operator==(const Rule &rhs) const { return id == rhs.id; }
};

struct Item {
    Rule &rule;
    int dot_pos;

    bool operator<(const Item &rhs) const {
        if (rule.id == rhs.rule.id) {
            return dot_pos < rhs.dot_pos;
        }
        return rule.id < rhs.rule.id;
    }

    bool operator==(const Item &rhs) const { return rule.id == rhs.rule.id && dot_pos == rhs.dot_pos; }
};

class SLRTable_Generator {
  private:
    std::vector<Rule> rules = {
        {0, "E'", {"E"}}, {1, "E", {"E", "+", "T"}}, {2, "E", {"E", "-", "T"}}, {3, "E", {"T"}}, {4, "T", {"(", "E", ")"}}, {5, "T", {"i"}},
    };
    std::map<std::string, std::set<std::string>> follow = {
        {"E'", {"$"}},
        {"E", {"$", "+", "-", ")"}},
        {"T", {"$", "+", "-", ")"}},
    };
    std::string start_symbol = "E'";
    std::vector<std::string> terminals = {"+", "-", "(", ")", "i"};
    std::vector<std::string> non_terminals = {"E'", "E", "T"};
    std::vector<std::string> symbols;
    std::vector<std::set<Item>> items_list;
    std::map<std::pair<int, std::string>, std::string> action;
    std::vector<std::vector<std::string>> table;

    void itemize() {
        std::set<Item> items;
        for (auto &rule : rules) {
            items.insert(Item{rule, 0});
        }
        items_list.push_back(items);
    }

    void gen_cannonical_collection() {
        int index;
        std::vector<std::set<Item>>::iterator itr;
        symbols = terminals;
        symbols.insert(symbols.end(), non_terminals.begin(), non_terminals.end());
        itemize();
        for (size_t i = 0; i < items_list.size(); i++) {
            for (auto &symbol : symbols) {
                auto new_items = go_to(items_list[i], symbol);
                if (new_items.size() == 0)
                    continue;
                if ((itr = std::find(items_list.begin(), items_list.end(), new_items)) == items_list.end()) {
                    items_list.push_back(new_items);
                    index = items_list.size() - 1;
                } else {
                    index = std::distance(items_list.begin(), itr);
                }
                if (std::find(terminals.begin(), terminals.end(), symbol) != terminals.end()) {
                    action[std::make_pair(i, symbol)] = "s" + std::to_string(index);
                } else {
                    action[std::make_pair(i, symbol)] = std::to_string(index);
                }
            }
        }
    }

    void closure(std::set<Item> &items) {
        size_t prev_size;
        do {
            prev_size = items.size();
            auto items_copy = items;
            for (auto &item : items_copy) {
                if (item.dot_pos >= item.rule.rhs.size())
                    continue;

                auto next_symbol = item.rule.rhs[item.dot_pos];
                if (std::find(non_terminals.begin(), non_terminals.end(), next_symbol) == non_terminals.end())
                    continue;
                for (auto &rule : rules) {
                    if (rule.lhs == next_symbol) {
                        items.insert(Item{rule, 0});
                    }
                }
            }
        } while (items.size() != prev_size);
    }

    std::set<Item> go_to(std::set<Item> items, std::string symbol) {
        std::set<Item> new_items;
        for (auto &item : items) {
            if (item.dot_pos >= item.rule.rhs.size())
                continue;
            if (item.rule.rhs[item.dot_pos] == symbol) {
                new_items.insert(Item{item.rule, item.dot_pos + 1});
            }
        }
        closure(new_items);
        return new_items;
    }

    void add_reduction() {

        for (auto &items : items_list) {
            for (auto &item : items) {
                if (item.rule.lhs == start_symbol && item.dot_pos == item.rule.rhs.size() - 1) {
                    action[std::make_pair(std::distance(items_list.begin(), std::find(items_list.begin(), items_list.end(), items)), "$")] = "acc";
                }
                if (item.dot_pos == item.rule.rhs.size()) {
                    for (auto &symbol : follow[item.rule.lhs]) {
                        int no = std::distance(items_list.begin(), std::find(items_list.begin(), items_list.end(), items));
                        auto p = std::make_pair(no, symbol);
                        if (action.find(p) != action.end()) {
                            std::cout << "Conflict at " << no << " " << symbol << std::endl;
                            exit(0);
                        }
                        action.insert(std::make_pair(p, "r" + std::to_string(item.rule.id)));
                    }
                }
            }
        }
    }

    void refresh_table(std::vector<std::string> header) {
        size_t height = items_list.size();
        size_t width = terminals.size() + non_terminals.size();
        table.assign(height + 2, std::vector<std::string>(width + 2, ""));
        table[0][0] = "State";
        for (size_t i = 0; i < height; i++) {
            table[i + 1][0] = std::to_string(i);
        }
        for (size_t i = 0; i < width; i++) {
            table[0][i + 1] = header[i];
        }
        for (auto &action : action) {
            int row = action.first.first + 1;
            int col = std::distance(header.begin(), std::find(header.begin(), header.end(), action.first.second)) + 1;
            table[row][col] = action.second;
        }
    }

  public:
    void run() {
        gen_cannonical_collection();
        add_reduction();
    }

    void show_item_list() {
        std::cout << "Item List" << std::endl;
        for (size_t i = 0; i < items_list.size(); i++) {
            std::cout << "I" << i << std::endl;
            for (auto &item : items_list[i]) {
                std::cout << item.rule.lhs << " -> ";
                for (size_t j = 0; j < item.rule.rhs.size(); j++) {
                    if (j == item.dot_pos)
                        std::cout << ".";
                    std::cout << item.rule.rhs[j];
                }
                if (item.dot_pos == item.rule.rhs.size())
                    std::cout << ".";
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    }

    void show_table() {
        std::cout << "Action Table" << std::endl;
        refresh_table({"+", "-", "(", ")", "i", "$", "E", "T"});

        for (auto &row : table) {
            for (auto &col : row) {
                std::cout << col << "\t";
            }
            std::cout << std::endl;
        }
    }
};

int main(int argc, char const *argv[]) {
    SLRTable_Generator slr;
    slr.run();
    slr.show_item_list();
    slr.show_table();
    return 0;
}
