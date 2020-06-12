#include <iostream>
#include <iterator>
#include <fstream>
#include <thread>
#include <mutex>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"
#include "utils.h"
#include "mcts.h"

const std::string PLAYER[] = {"MCTS_with_tuple", "MCTS", "tuple", "heuristic", "eat_first", "random"};
const std::string SIMULATION[] = {"(random)", "(eat-first)", "(tuple)"};
std::mutex mtx;
int fight_black_win, fight_white_win;

void fight_thread(int player1, int player2, int sim1, int sim2, Tuple *tuple, int game_count, uint32_t seed) {
    MCTS mcts_tuple(tuple, true, 60000, seed);
    MCTS mcts(tuple, false, 60000, seed);
    TupleRolloutPlayer tuple_player(tuple);
    HeuristicRolloutPlayer heuristic_player;
    GreedyRolloutPlayer greedy_player(seed);
    RandomRolloutPlayer random_player(seed);
    
    int black_win = 0, white_win = 0;
    for (int i = 0; i < game_count; i++) {
        // std::cout << i << "\n";
        Board board;
        int color = 0, step_count = 0, current, sim;
        int skip_count = 0;
        while (true) {
            // std::cout << "hi\n";
            Board before(board);
            current = color ? player2 : player1;
            sim = color ? sim2 : sim1;
            switch (current) {
                case 0:
                    mcts_tuple.playing(board, color, sim);
                    break;
                case 1:
                    mcts.playing(board, color, sim);
                    break;
                case 2:
                    tuple_player.playing(board, color);
                    break;
                case 3:
                    heuristic_player.playing(board, color);
                    break;
                case 4:
                    greedy_player.playing(board, color);
                    break;
                case 5:
                    random_player.playing(board, color);
                    break;
                default:
                    break;
            }
            if (before != board) skip_count = 0;
            else                 skip_count++;
            if (skip_count == 2)    break;
            if (board.game_over())  break;

            color ^= 1; // change player
        }

        int black_bitcount = Bitcount(board.get_board(0));
        int white_bitcount = Bitcount(board.get_board(1));
        // if (black_bitcount + white_bitcount < 60) board.print_board();
        if (black_bitcount > white_bitcount)    black_win++;
        if (black_bitcount < white_bitcount)    white_win++;
    }

    mtx.lock();
    fight_black_win += black_win;
    fight_white_win += white_win;
    mtx.unlock();
}

void fight(int player1, int player2, int sim1, int sim2, Tuple *tuple, int game_count) {
    std::cout << PLAYER[player1];
    if (player1 <= 1)   std::cout << SIMULATION[sim1];
    std::cout << " VS " << PLAYER[player2];
    if (player2 <= 1)   std::cout << SIMULATION[sim2];
    std::cout << std::endl;

    fight_black_win = 0, fight_white_win = 0;
    std::vector<std::thread> threads;
    std::random_device rd;
    for(int i = 0; i < 10; i++) {
        threads.push_back(std::thread(fight_thread, player1, player2, sim1, sim2, tuple, game_count / 10, rd()));
    }
    for (auto& th : threads) {
        th.join();
    }

    std::streamsize ss = std::cout.precision();
    std::cout << "Playing " << game_count << " episodes: \n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Black: " << fight_black_win * 100.0 / (fight_black_win + fight_white_win)
              << " %" << std::endl;
    std::cout << "White: " << fight_white_win * 100.0 / (fight_black_win + fight_white_win)
              << " %\n" << std::endl;
    std::cout.precision(ss);
}

int main(int argc, const char* argv[]) {
    std::cout << "Othello: ";
    std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
    std::cout << std::endl << std::endl;

    size_t total = 1000, block = 0, limit = 0;
    int game_count = 2000;
    std::string tuple_args;
    float epsilon = 0.9;

    for (int i = 1; i < argc; i++) {
        std::string para(argv[i]);
        if (para.find("--total=") == 0) {
            total = std::stoull(para.substr(para.find("=") + 1));
        } else if (para.find("--block=") == 0) {
            block = std::stoull(para.substr(para.find("=") + 1));
        } else if (para.find("--limit=") == 0) {
            limit = std::stoull(para.substr(para.find("=") + 1));
        } else if (para.find("--game=") == 0) {
            game_count = std::stoi(para.substr(para.find("=") + 1));
        } else if (para.find("--tuple=") == 0) {
            tuple_args = para.substr(para.find("=") + 1);
        } else if (para.find("--epsilon=") == 0) {
            epsilon = std::stof(para.substr(para.find("=") + 1));
        }
    }

    Tuple tuple(tuple_args);
    TupleTrainingPlayer play1(0, &tuple, epsilon), play2(1, &tuple, epsilon);
    // MCTSTrainingPlayer play1(0, &tuple, epsilon), play2(1, &tuple, epsilon);
    Statistic stat(total, block, limit);

    // training - lots of episodes
    while (!stat.is_finished()) {
        play1.open_episode();
        play2.open_episode();
        stat.open_episode(play1.role() + ":" + play2.role());
        Episode& game = stat.back();

        // Board board = game.state();
        // board.print_board();

        int skip_count = 0;
        // one episode
        while (true) {
            TupleTrainingPlayer& who = game.take_turns(play1, play2);
            // MCTSTrainingPlayer& who = game.take_turns(play1, play2);
            Action action = who.take_action(game.state());
            if (!game.apply_action(action)) skip_count++;
            else    skip_count = 0;
            if (skip_count == 2)    break;
            if (who.check_for_win(game.state())) break;
        }

        int black_bitcount = Bitcount(game.state().get_board(0));
        int white_bitcount = Bitcount(game.state().get_board(1));
        std::string win_bitcount;
        std::string winner;
        if (black_bitcount > white_bitcount) {
            winner = "Black";
            win_bitcount = "1";
        }
        else if (black_bitcount < white_bitcount) {
            winner = "White";
            win_bitcount = "-1";
        }
        else {
            winner = "Draw";
            win_bitcount = "0";
        }

        play1.close_episode(win_bitcount);
        play2.close_episode(win_bitcount);
        stat.close_episode(winner);

        // after some episodes, test playing result
        if (stat.episode_count() % block == 0) {
            fight(2, 4, 0, 0, &tuple, game_count);
            fight(4, 2, 0, 0, &tuple, game_count);
            fight(2, 5, 0, 0, &tuple, game_count);
            fight(5, 2, 0, 0, &tuple, game_count);
        }
        if (stat.episode_count() % 5000000 == 0) {
            fight(0, 1, 0, 0, &tuple, 200);
            fight(1, 0, 0, 0, &tuple, 200);
            // fight(0, 1, 1, 1, &tuple, 100);
            // fight(1, 0, 1, 1, &tuple, 100);
        }

        if (stat.episode_count() % 500000 == 0) {
            play1.increase_epsilon();
            play2.increase_epsilon();
            std::cout << play1.get_epsilon() << " " << play2.get_epsilon() << "\n";
        }
    }
    return 0;
}