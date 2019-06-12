#include <iostream>
#include <stdio.h>
#include <random>
#include <chrono>
#include <queue>
#include <unistd.h>
#include "game_state.h"

using namespace std;
typedef unsigned int uint;

uint seed=std::chrono::system_clock::now().time_since_epoch().count();

void print_board(uint bb[3][3]){
    //player 1 -> X
    //player 2 -> O
	char symbols[3]={'.','X','O'};
	for(int i=0;i<3;++i){
		//printf("|  ");
		for(int j=0;j<3;++j){
			printf(" %c ",symbols[bb[i][j]]);
			if(j < 2) printf("|");
		}
		printf(" \n");
		if(i!=2){
			printf("---|---|---\n");
		}else{
			printf("\n");
		}
	}
}

void free_memory_decision_tree(game_state* gmt, game_state* next){
	if(gmt->children == NULL){
		delete gmt;
	}else{
		for(int i=0;i<gmt->moves.size();++i){
			if(gmt->children[i] != NULL){
				if(gmt->children[i] != next){
					free_memory_decision_tree(gmt->children[i], next);
				}
			}
		}
		delete [] gmt->children;
		if(gmt != next->parent){
			delete gmt;
		}
	}
}

void free_memory_path(game_state* root){
	if(root->parent != NULL){
		free_memory_path(root->parent);
		print_board(root->board);
		delete root;
	}else{
		print_board(root->board);
		delete root;
	}
}

void print_tree(game_state* root){ 
	queue<game_state*> bfs;
	bfs.push(root);

	while(!bfs.empty()){
		cout<<bfs.front()->n_empty<<'\t'<<bfs.front()->wins<<'\t'<<bfs.front()->visits<<'\t'<<bfs.front()->ratio<<'\t'<<bfs.front()->selection_ratio<<'\n';
		if(bfs.front()->children!=NULL){
			for(int i=0;i<bfs.front()->moves.size();++i){
				if(bfs.front()->children[i] != NULL){
					bfs.push(bfs.front()->children[i]);
				}
			}
		}
		bfs.pop();
	}

	cout<<"--------------"<<'\n';
}

void playerMove(game_state** root){
	int xx,yy;
	bool valid_move=false;
	cout<<'\n';
	cout<<"Your turn to play."<<'\n';
	while(!valid_move){
		cin>>xx>>yy;
		if((*root)->board[xx][yy] != 0){
			valid_move=false;
			cout<<"Invalid move. Play again."<<'\n';
		}else {
			valid_move=true;
		}
	}
	int selected_index=0;
	if((*root)->children != NULL){
		for(int i=0; i<(*root)->moves.size(); ++i){
			if((*root)->children[i]->board[xx][yy] == ((*root)->children[i]->player)){
				(*root)=(*root)->children[i];
				break;
			}
		}
		free_memory_decision_tree((*root)->parent,(*root));
	}else{
		game_state* player_state=new game_state(*root,PLAYER_2,xx,yy);
		*root=player_state;
	}
}

bool opponent_wins_nextround(uint board[3][3], int &x, int &y){
	int win_AI=0;
	int win_PL=0;
	for(int i=0;i<3;++i){
		for(int j=0;j<3;++j){
			if(board[i][j]==0){
				board[i][j] = 2;

				if (final_state(board) == 2 ){
					win_PL=2;
					x=i;y=j;
				}
				board[i][j] = 1;
				if (final_state(board) == 1 ){
					win_AI=1;
				}
				
				board[i][j] = 0;
			}
		}
	}
	if(win_PL == 2 && win_AI != 1) return true;
	return false;
}

void bot_play(game_state** root){}

int main(){
	int round_number=1;
	std::minstd_rand0 gen0(seed);
	int first_player=(int) round((double)gen0()/gen0.max()) + 1;
	bool gameOver=false;
	game_state* root;

	cout<<"You: "<< "O" << " | Opponent: X"<<'\n';
	if(first_player == PLAYER_1){
		root=new game_state(first_player);
	}else if (first_player == PLAYER_2){
		cout<<"=========="<<"Round "<<round_number<<"==============="<<'\n';
		root=new game_state(first_player);
		print_board(root->board);
		playerMove(&root);
	}

	game_state* selected;
	game_state* expanded;
    int num_iter=500; //TODO: mesmo que o numero de iteracoes seja alto, fazer com que repita algumas simulacoes para obter melhores resultados
    double winRatio=0.0;
    int selected_id=0;
    int d,p1,p2;
    d=0;
    p1=0;
    p2=0;
    
    while(!gameOver){
    	if(first_player == PLAYER_1){
    		cout<<"=========="<<"Round "<<round_number<<"==============="<<'\n';
    		round_number++;
    	}

    //Iteration -> 4 steps
    	for(;;){
    		selection(root, &selected);
    		expansion(selected, &expanded);
    		simulation(&expanded);
    		backtrace(&expanded, expanded->simu);
    		if(num_iter<=0 || expanded->n_empty==0){
    			break;
    		}
    		num_iter--;
    	}

	//print_tree(root);
    	//cout<<endl;
    	num_iter=500;
    	d=0;p1=0;p2=0;
    	selected=NULL;expanded=NULL;

 //Jogada do bot: Escolher a melhor jogada: implica percorrer os children do estado atual e escolher o que tiver maior racio

    //Mas primeiro, verificar se o adversario ganha no proximo turno se o bot nao jogar num
    //determinado sitio, mesmo que o racio seja inferior a outras jogadas
    	int x_next,y_next;

    	if(opponent_wins_nextround(root->board, x_next, y_next)){
    		// for(int i=0; i<root->moves.size(); ++i){
    		// 	if(root->children[i]->board[x_next][y_next] == ((root->player)^3)){
    		// 		root=root->children[i];
    		// 		break;
    		// 	}
    		// }
    		game_state* player_state=new game_state(root,PLAYER_2,x_next,y_next);
			root=player_state;
			root->player=PLAYER_1;
    	}else{
    		for(int i=0; i<root->moves.size(); ++i){
    			if(root->children[i]->ratio >= winRatio){
    				selected_id=i;
    				winRatio=root->children[i]->ratio;
    			}
    		}

    		root=root->children[selected_id];
    	}

    	free_memory_decision_tree(root->parent,root);

    	if (round_number>1) cout<<"Opponent move: "<<'\n';
    	print_board(root->board);
    	if(final_state(root->board) == 1 ){
    		cout<<"Player 1 (AI) wins."<<'\n';
    		gameOver=true;
    		break;
    	}else if(root->n_empty==0){
    		cout<<"Draw."<<'\n';
    		gameOver=true;
    		break;
    	}
    	winRatio=0;
    	selected_id=0;

    	if(first_player == PLAYER_2){
    		round_number++;
    		cout<<"=========="<<"Round "<<round_number<<"==============="<<'\n';
    	}

    //Jogada do player 
    	playerMove(&root);

    	cout<<'\n';
    	print_board(root->board);
    	if(final_state(root->board) == 2 ){
    		cout<<"Player 2 (You) wins."<<'\n';
    		gameOver=true;
    		break;
    	}else if(root->n_empty==0){
    		cout<<"Draw."<<'\n';
    		gameOver=true;
    		break;
    	}


    	
    }

    free_memory_path(root);
    
    return 0;
  }
