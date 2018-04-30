#ifndef GAME_STATE_H
#define GAME_STATE_H
#include <iostream>
#include <utility>
#include <unordered_map>
#include <random>
#include <cmath>
#include <memory>
#include <chrono>
#include <queue>

#define fr first
#define sc second
#define PLAYER_1 1 //1st BOT
#define PLAYER_2 2 //PLAYER/PERSON || 2nd BOT
struct pair_hash{
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
	auto h1=std::hash<T1>{}(p.first);
	auto h2=std::hash<T2>{}(p.second);

	return h1^h2;
    }
};

typedef unsigned int uint;
typedef std::pair<int,int> pii;
typedef std::unordered_map<pii, bool, pair_hash> play_map;
extern uint seed;

const double n_exp=1.5;
//Node representing a certain game state
struct game_state {
    game_state *parent;
    game_state **children;

    int visits,wins,n_empty;
    int player; //Current player to move
    int simu; //Result of the simulation for the current node
    double ratio,selection_ratio;
    uint board[3][3];
    play_map moves;

    //Initiate root (if AI first to play)
    game_state(int mode=1): visits(0), wins(0), simu(0)
    {
	init_board(&board);
	n_empty=9;
	ratio=0;
	selection_ratio=0;
	parent=NULL;
	std::minstd_rand0 gen2(seed);
	if(mode==1){
	    moves.emplace(std::make_pair((int) floor((double)gen2()/(gen2.max()+1) *3),(int) floor((double)gen2()/(gen2.max()+1) *3)),true);
	    player=PLAYER_2;
	}else if(mode==2){
	    player=PLAYER_1;
	}
	/*moves.emplace(std::make_pair(0,0),true);*/
	//moves.emplace(std::make_pair(1,1),true);
	/*moves.emplace(std::make_pair(2,2),true);*/
	children=NULL;
	
    }

    //Initiate node from parent
    game_state(game_state* gmt, int mode=1, int xx=-1, int yy=-1): visits(1), wins(0), simu(0), ratio(0.0)
    {
	player=gmt->player^3;
	n_empty=gmt->n_empty-1;
	parent=gmt;
	children=NULL;
	memcpy(board, gmt->board, sizeof(gmt->board));
	//mode 1 = jogada do bot, mode 2 = jogada do player
	if(mode==1) make_play(gmt, &board);
	else board[xx][yy]=player;
	create_new_playmap(board, &moves);
    
    }
    void init_board(uint (*bb)[3][3]){
	for(int i=0;i<3;++i){
	    for(int j=0;j<3;++j){
		(*bb)[i][j]=0;
	    }
	}
    }
    void make_play(game_state* gmt, uint (*bb)[3][3]){
	for(play_map::iterator it=gmt->moves.begin(); it!= gmt->moves.end();++it){
	    if(it->sc){
		(*bb)[it->fr.fr][it->fr.sc] = gmt->player^3;
		it->sc=false;
		break;
	    }
	}
    }

    void create_new_playmap(uint bb[3][3], play_map* moves){
	for(int i=0;i<3;++i){
	    for(int j=0;j<3;++j){
		if(bb[i][j]==0){
		    moves->emplace(std::make_pair(i,j),true);
		}
	    }
	}
    }

};


int final_state(uint bb[3][3]){
    //Checks final game state
    //Return 1: player 1 won
    //Return 2: player 2 won
    //Return 0: draw
    
    //Testing columns/rows
    for(int i=0;i<3;++i){ 
	for(int j=0;j<3;++j){
	    if((bb[i][j]==bb[(i+1)%3][j] && bb[(i+1)%3][j]==bb[(i+2)%3][j]) || (bb[i][j]==bb[i][(j+1)%3] && bb[i][(j+1)%3]==bb[i][(j+2)%3])){
		return bb[i][j];
	    }
	}
    }
    //Testing diagonals
    if((bb[0][0]==bb[1][1] && bb[1][1]==bb[2][2]) || (bb[0][2]==bb[1][1] && bb[1][1]==bb[2][0])){
	return bb[1][1];
    }

    return 0;
}

bool child_left_toadd(game_state* gmt){
    if(gmt->children == NULL){
	return true;
    }
    for(int i=0;i<gmt->moves.size();++i){
	if(gmt->children[i]==NULL){
	    return true;
	}
    }
    return false;
}

void selection(game_state* root, game_state** final_node){
    *final_node=root;
    std::queue<game_state*> queue, queue_selected;
    queue.push(root);

    while(!queue.empty()){
	if(!child_left_toadd(queue.front())){
	    for(int i=0;i<queue.front()->moves.size();++i){
		if(queue.front()->children[i] != NULL){
		    queue.push(queue.front()->children[i]);
		}
	    }
	}else{
	    if(queue.front() != root){
		queue.front()->selection_ratio =(double)(queue.front())->wins/(queue.front())->visits + pow(log((double) (queue.front())->parent->visits)/(queue.front())->visits , 1.0/n_exp);
	    }
	    queue_selected.push(queue.front());
	}
	queue.pop();
    }

    double win_ratio=0;
    while(!queue_selected.empty()){
	if(queue_selected.front()->selection_ratio >= win_ratio){
	    *final_node=queue_selected.front();
	    win_ratio=queue_selected.front()->selection_ratio;
	}
	queue_selected.pop();
    }
    
}

void expansion(game_state* gmt, game_state** final_node){
    if(gmt->children == NULL){
	game_state** children=new game_state*[gmt->moves.size()];
	gmt->children=children;
	for(int i=0;i<gmt->moves.size();++i){
	    gmt->children[i]=NULL;
	}
    }
    for(int i=0;i<gmt->moves.size();++i){
	if(gmt->children[i] == NULL){
	    game_state* child_i= new game_state(gmt);
	    gmt->children[i]=child_i;
	    *final_node=child_i;
	    break;
	}
    }
}

void simulation(game_state** gmt){
    uint bb[3][3];
    int it=(*gmt)->n_empty;
    int player=(*gmt)->player;
    int ir,jr;
    memcpy(bb, (*gmt)->board, sizeof((*gmt)->board));

    std::minstd_rand0 gen(seed);
    
    player=player^3; //next play is from opponent
    while(it>0 && final_state(bb)==0){
	ir=(int) floor((double)gen()/(gen.max()+1) *3);
	jr=(int) floor((double)gen()/(gen.max()+1) *3);
	if(bb[ir][jr]==0){ //TODO: adicionar preferencia em jogar nos cantos
	    bb[ir][jr]=player;
	    player=player^3;
	    it--;
	}
    }
    (*gmt)->simu = final_state(bb);
    if((*gmt)->simu==1) (*gmt)->wins++; // Win ratio Policy
    (*gmt)->ratio=(double)(*gmt)->wins/(*gmt)->visits;
    (*gmt)->selection_ratio=(double)(*gmt)->wins/(*gmt)->visits +
				    pow(log((double) (*gmt)->parent->visits)/(*gmt)->visits , 1.0/n_exp);
}

void backtrace(game_state** gm_l, int sim){
    if((*gm_l)->parent->parent == NULL){ 
	(*gm_l)->parent->visits++;
	if(sim==1){
	    (*gm_l)->parent->wins++;
	}
	
    }else{
	(*gm_l)->parent->visits++;
	if(sim==1){
	    (*gm_l)->parent->wins++;
	}
	(*gm_l)->parent->ratio=(double)(*gm_l)->parent->wins/(*gm_l)->parent->visits;
	(*gm_l)->parent->selection_ratio=(double)(*gm_l)->parent->wins/(*gm_l)->parent->visits +
				pow(log((double) (*gm_l)->parent->parent->visits)/(*gm_l)->parent->visits ,1.0/n_exp);
	backtrace(&((*gm_l)->parent), sim);
    }
}

#endif
