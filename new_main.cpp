//Grande TODO: politica de jogada-> jogar para ganhar tendo em conta quando pode perder
//             aumentar o board
#include <iostream>
#include <stdio.h>
#include <random>
#include <chrono>
#include <queue>
#include <string>
#include <unistd.h>
#include "game_state.h"
#include "bot_class.h"

using namespace std;
typedef unsigned int uint;
typedef std::mt19937 mersenne_engine;
unsigned seed=std::chrono::system_clock::now().time_since_epoch().count();
mersenne_engine gen2;
int n_wins1,n_wins2,draws;

bool check_winner(game_state* const root, int first_player){
    if(final_state(root->board) == first_player ){
                cout<<"Player "<< first_player<<" wins."<<'\n';
                if(first_player == 1){ n_wins1++;}
                else {n_wins2++;}

                return true;
    }else if(root->n_empty==0){
                cout<<"Draw."<<'\n';
                draws++;
                return true;
    }

    return false;
}
    void free_memory_decision_tree(game_state*& gmt, game_state* next, bool delete_all=false){
        if(gmt->children == NULL){
            if(gmt != next){
                delete gmt;
                gmt=NULL;
            }
        }else{
            for(int i=0;i<gmt->moves.size();++i){
                if(gmt->children[i] != NULL){
                    if(gmt->children[i] != next || delete_all == true){
                        free_memory_decision_tree(gmt->children[i], next);
                    }
                }
            }
            if(gmt != next->parent){
                delete [] gmt->children;
                gmt->children=NULL;
            }

            if(gmt != next->parent && gmt != next){
                delete gmt;
                gmt=NULL;
            }
        }
    }
int main(){
        std::minstd_rand0 gen0(seed);
        gen2=mersenne_engine(seed);
        n_wins1=0;
        draws=0;
        n_wins2=0;
    for(int i=0;i<1;++i){
        int round_number=1;
        int first_player=(int) round((double)gen0()/gen0.max()) + 1;
        bool gameOver=false;
        game_state* root1=NULL;
        game_state* root2=NULL;;

        game_state* selected=NULL;
        game_state* expanded=NULL;
        int num_iter=500; 
        double winRatio=0.0;
        int selected_id=0;
    
        root1=new game_state();
        root2=new game_state();
        Bot player1=Bot(root1,1);
        Bot player2=Bot(root2,2);

        cout<<"<------------------------------>"<<'\n';
        cout<<"<-----------NEW GAME----------->"<<'\n';
        cout<<"<------------------------------>"<<'\n';

        //First round
        if(first_player==1){
            player1.first_play();
            print_board(player1.tree->board);
            player2.update(player1.tree);
            print_board(player2.tree->board);
        }else{
            player2.first_play();
            print_board(player2.tree->board);
            player1.update(player2.tree);
            print_board(player1.tree->board);
        }

        while(!gameOver){

            cout<<"=========="<<"Round "<<round_number+1<<"==============="<<'\n';

        	switch(first_player){
                case 1:
                    player1.update(player2.tree);
                    print_board(player1.tree->board);
                    gameOver=check_winner(player1.tree,first_player);

                    if(!gameOver){
                        player2.update(player1.tree);
                        print_board(player2.tree->board);
                        gameOver=check_winner(player2.tree,first_player^3);
                    }
                    break;
                case 2:
                    player2.update(player1.tree);
                    print_board(player2.tree->board);
                    gameOver=check_winner(player2.tree,first_player);

                    if(!gameOver){
                        player1.update(player2.tree);
                        print_board(player1.tree->board);
                        gameOver=check_winner(player1.tree,first_player^3);
                    }
                    break;
                default:
                    break;    
            }
        	
            round_number++;

        }

        cout<<"<------------------------------->"<<'\n';
        if(player1.tree != NULL){
            delete player1.tree;
        }
        if(player2.tree != NULL){
            delete player2.tree;
        }
        gameOver=false;
        string st="P1: "+to_string(n_wins1)+" | PL2: "+to_string(n_wins2)+ " | Draws: "+to_string(draws);
        cout<<st<<'\r'<<flush;
        cout<<endl;
    }

    return 0;
  }