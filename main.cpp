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
	printf("| ");
	for(int j=0;j<3;++j){
	    printf("%c ",symbols[bb[i][j]]);
	}
	printf("|\n");
    }
}
void free_memory(game_state* gmt){//funfact:dfs
    if(gmt->children == NULL){
	delete gmt;
    }else{
	for(int i=0;i<gmt->moves.size();++i){
	    if(gmt->children[i] != NULL){
		free_memory(gmt->children[i]);
	    }
	}
	delete [] gmt->children;
	delete gmt;
    }
}
void print_tree(game_state* root){ //funfact:bfs
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
   /*for(int i=0; i<(*root)->moves.size(); ++i){*/
	//if((*root)->children[i] != NULL){
	    //if((*root)->children[i]->board[xx][yy]==2){
		//*root=(*root)->children[i];
		//break;
	    //}
	//}
    /*}*/

    game_state* player_state=new game_state(*root,PLAYER_2,xx,yy);
    //TODO:este ponteiro nao esta a ser free...memory leak

    *root=player_state;
}

bool opponent_wins_nextround(uint board[3][3]){
    uint board_cp[3][3];
    memcpy(board_cp, board, sizeof(&board));
    int wins=0;
    for(int i=0;i<3;++i){
	for(int j=0;j<3;++j){
	    if(board[i][j]==0){
		board_cp[i][j]=2;
		wins=final_state(board_cp);
		if(wins==2) return true;
		board_cp[i][j]=0;
	    }
	}
    }
    return false;
}

		

int main(){
    std::minstd_rand0 gen0(seed);
    int first_player=(int) round((double)gen0()/gen0.max()) + 1;
    bool gameOver=false;
    game_state* root;
    
    if(first_player == PLAYER_1){
	cout<<"Opponent first move: "<<'\n';
	root=new game_state(first_player);
    }else if (first_player == PLAYER_2){
	cout<<"First Move: "<<'\n';
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
    //Iteration -> 4 steps
       for(;;){
	    selection(root, &selected);
	    expansion(selected, &expanded);
	    simulation(&expanded);
		switch (expanded->simu){
		    case 0:
			d++;
			break;
		    case 1:
			p1++;
			break;
		    case 2:
			p2++;
			break;
		}
	   /*cout<<"Draws: "<<d<<" \t\t";*/
	    //cout<<"Player1 wins: "<<p1<<" \t\t";
	    //cout<<"Player2 wins: "<<p2<<" \t\r";
	    /*cout.flush();*/

	    backtrace(&expanded, expanded->simu);
	    if(num_iter<=0 || expanded->n_empty==0){
		break;
	    }
	    num_iter--;
	}

	//print_tree(root);
	cout<<endl;
	num_iter=500;
	d=0;p1=0;p2=0;
	selected=NULL;expanded=NULL;
	
 //Jogada do bot: Escolher a melhor jogada: implica percorrer os children do estado atual e escolher o que tiver maior racio

    //Mas primeiro, verificar se o adversario ganha no proximo turno se o bot nao jogar num
    //determinado sitio, mesmo que o racio seja inferior a outras jogadas
    
	if(opponent_wins_nextround(root->board) && false){
	    cout<<"HERE"<<'\n';
	}else{
	    for(int i=0; i<root->moves.size(); ++i){
		if(root->children[i]->ratio >= winRatio){
		    selected_id=i;
		    winRatio=root->children[i]->ratio;
		}
	    }

	   root=root->children[selected_id];
	}

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

    //Jogada do player 
	playerMove(&root);
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

	cout<<"========================="<<'\n';
	
    }

    while(root->parent != NULL){
	root=root->parent;
    }
    free_memory(root);
    
    return 0;
}
