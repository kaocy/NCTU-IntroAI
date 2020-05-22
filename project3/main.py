import sys
import time
import random
import math
from itertools import combinations

direction = [(-1, -1), (-1, 0), (-1, 1), (0, -1), (0, 1), (1, -1), (1, 0), (1, 1)]

class Control():
    def __init__(self, board_size, num_mines):
        self.board_size = board_size
        self.num_mines = num_mines
        self.mines = [[False for _ in range(board_size[1])] for _ in range(board_size[0])]
        self.marked = [[-1 for _ in range(board_size[1])] for _ in range(board_size[0])]

    def initialize_board(self):
        coordinates = []
        for x in range(self.board_size[0]):
            for y in range(self.board_size[1]):
                coordinates.append((x, y))

        # sample the positions of mines
        mine_coordinates = random.sample(coordinates, self.num_mines)
        for coord in mine_coordinates:
            self.mines[coord[0]][coord[1]] = True

        print('Mines:')
        for row in self.mines:
            for col in row:
                if col:
                    print('*', end=' ')
                else:
                    print('-', end=' ')
            print('')

    def get_initial_safe_cells(self):
        # find all the safe cells
        coordinates = []
        for x in range(self.board_size[0]):
            for y in range(self.board_size[1]):
                if not self.mines[x][y]:
                    coordinates.append((x, y))

        num_safe_cells = round(math.sqrt(self.board_size[0] * self.board_size[1]))
        # num_safe_cells = 150
        return random.sample(coordinates, num_safe_cells)

    def get_unmarked_neighbors(self, x, y):
        # calculate the mines of unmarked neighbor cells
        num_mines = 0
        cells = []
        for d in direction:
            px = x + d[0]
            py = y + d[1]
            if self.is_outside(px, py) or self.marked[px][py] != -1:
                continue
            cells.append((px, py))
            if self.mines[px][py]:
                num_mines += 1

        return num_mines, cells

    def mark_cell(self, x, y, value):
        self.marked[x][y] = value

    def print_marked_cells(self):
        count = 0
        print('\nResults:')
        for row in self.marked:
            for col in row:
                if col == 1:
                    count += 1
                    print('*', end=' ')
                else:
                    print('-', end=' ')
            print('')
        print(f'Total mark {count} mines.')
        return count

    def is_outside(self, x, y):
        # check the given position is outside the board
        return (x < 0) or (x >= self.board_size[0]) or (y < 0) or (y >= self.board_size[1])


class Player():
    def __init__(self):
        self.kb = []
        self.kb0 = []

    def initialize_kb(self, safe_cells):
        # transfer the safe cells to single-literal clause and insert into KB
        for cell in safe_cells:
            cnf = [(cell[0], cell[1], False)]
            self.kb.append(cnf)

    def handle_single_literal_clause(self):
        cnf1 = self.kb.pop(0)
        self.kb0.append(cnf1[0])

        for cnf2 in self.kb:
            cnf = self.handle_resolution(list(cnf1), list(cnf2))
            self.insert_clause(cnf)

        return cnf1[0]

    def handle_multiple_literal_clause(self):
        cnfs = []
        delete_index = []

        # pairwise matching
        for i, cnf1 in enumerate(self.kb):
            resolution = False
            for j, cnf2 in enumerate(self.kb[i + 1:]):
                if len(cnf1) > 2 or len(cnf2) > 2:
                    continue
                cnf = self.handle_resolution(list(cnf1), list(cnf2))
                if len(cnf) > 0:
                    resolution = True
                    cnfs.append(cnf)

            if resolution:
                delete_index.append(i)

        for cnf in cnfs:
            self.insert_clause(cnf)
        for i in reversed(delete_index):
            del self.kb[i]

    def find_complementary_pairs(self, cnf1, cnf2):
        # find out the pairs of complementary literals of two clauses
        pairs = []
        for i, l1 in enumerate(cnf1):
            for j, l2 in enumerate(cnf2):
                if l1[0] == l2[0] and l1[1] == l2[1] and l1[2] != l2[2]:
                    pairs.append((i, j))
        return pairs
 
    def handle_resolution(self, cnf1, cnf2):
        pairs = self.find_complementary_pairs(cnf1, cnf2)
        if len(pairs) != 1:
            return []

        # do the resolution to generate a new clause
        pair = pairs[0]
        del cnf1[pair[0]]
        del cnf2[pair[1]]
        cnf = cnf1 + cnf2
        return cnf

    def generate_clause_from_hint(self, num_mines, cells):
        num_cells = len(cells)

        # generate clauses according to the situation
        if num_mines == 0:
            for cell in cells:
                cnf = [(cell[0], cell[1], False)]
                self.insert_clause(cnf)
        elif num_mines == num_cells:
            for cell in cells:
                cnf = [(cell[0], cell[1], True)]
                self.insert_clause(cnf)
        elif num_mines < num_cells:
            num = num_cells - num_mines + 1
            combs = list(combinations(cells, num))
            for comb in combs:
                cnf = []
                for cell in list(comb):
                    cnf.append((cell[0], cell[1], True))
                self.insert_clause(list(cnf))

            num = num_mines + 1
            combs = list(combinations(cells, num))
            for comb in combs:
                cnf = []
                for cell in list(comb):
                    cnf.append((cell[0], cell[1], False))
                self.insert_clause(list(cnf))

    def insert_clause(self, cnf):
        if len(cnf) == 0:
            return

        # do the resolution with all the clauses in KB0
        cnf.sort()
        updated_cnf = []
        for l1 in cnf:
            deleted = False
            for l2 in self.kb0:
                if l1[0] == l2[0] and l1[1] == l2[1] and l1[2] != l2[2]:
                    deleted = True
                    break
            if not deleted:
                updated_cnf.append(l1)

        if self.check_duplication(updated_cnf):
            return
        if self.check_subsumption(updated_cnf):
            return

        if len(updated_cnf) == 1:
            self.kb.insert(0, updated_cnf)
        else:
            self.kb.append(updated_cnf)

    def check_duplication(self, cnf1):
        if len(self.kb) == 0:
            return False

        for cnf2 in self.kb:
            if (len(cnf1) != len(cnf2)):
                return False
            for i in range(len(cnf1)):
                if cnf1[i] != cnf2[i]:
                    return False
        return True

    def check_subsumption(self, cnf1):
        kb = []
        res = False
        for cnf2 in self.kb:
            if len(cnf1) < len(cnf2) and self.check_strict(cnf1, cnf2):
                continue
            if len(cnf1) > len(cnf2) and self.check_strict(cnf2, cnf1):
                res = True
            kb.append(cnf2)

        if len(kb) < len(self.kb):
            self.kb = kb
        return res

    def check_strict(self, cnf1, cnf2):
        for l1 in cnf1:
            find = False
            for l2 in cnf2:
                if l1 == l2:
                    find = True
                    break

            if not find:
                return False

        return True

    def check_termination(self):
        return (len(self.kb) == 0)


class Game():
    def __init__(self, level):
        if level == 'Easy':
            self.board_size = (9, 9)
            self.num_mines = 10
        if level == 'Medium':
            self.board_size = (16, 16)
            self.num_mines = 25
        if level == 'Hard':
            self.board_size = (30, 16)
            self.num_mines = 99
        self.control = Control(self.board_size, self.num_mines)
        self.player = Player()

    def start(self):
        self.control.initialize_board()
        safe_cells = self.control.get_initial_safe_cells()
        self.player.initialize_kb(safe_cells)

        stuck_count = 0
        while True:
            if self.player.check_termination() or stuck_count >= 5:
                return self.control.print_marked_cells() == self.num_mines

            # if there is a single-literal clause in KB
            if len(self.player.kb[0]) == 1:
                stuck_count = 0
                # print('case1')
                cell = self.player.handle_single_literal_clause()
                self.control.mark_cell(cell[0], cell[1], int(cell[2]))
                if cell[2] == False:
                    num_mines, cells = self.control.get_unmarked_neighbors(cell[0], cell[1])
                    self.player.generate_clause_from_hint(num_mines, cells)
            else:
                # print('case2')
                stuck_count += 1
                self.player.handle_multiple_literal_clause()


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: python main.py [Easy|Medium|hard]')
        sys.exit()

    success_count = 0
    total_time = 0.0
    for i in range(100):
        start = time.time()
        game = Game(sys.argv[1])
        success = game.start()
        end = time.time()
        if success:
            total_time += end - start
            success_count += 1

    print(f'Success count: {success_count}')
    if success_count > 0:
        print(f'Average time: {total_time / success_count} seconds')
