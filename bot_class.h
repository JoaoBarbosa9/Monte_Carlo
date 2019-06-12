#ifndef BOT_CLASS_H
#define BOT_CLASS_H
#include <iostream>
#include <cmath>
#include "game_state.h"

typedef unsigned int uint;

struct Bot{
	game_state* tree; //game tree for each bot
	int id; // X->1 O->2

	Bot(game_state* gmt , int id_n){
		tree=gmt;
		id=id_n;

		tree->player=id^3;
	}

	void first_play(){
		int num_iter=500;
		int selected_id=0;
		double winRatio=0;
    	game_state* selected;
    	game_state* expanded;
    	selected=NULL;expanded=NULL;

    	//Expanding tree
		for(;;){
    		selection(tree, &selected);
    		expansion(selected, &expanded);
    		simulation(&expanded, id);
    		backtrace(&expanded, expanded->simu, id);
    		if(num_iter<=0 || expanded->n_empty==0){
    			break;
    		}
    		num_iter--;
    	}

    	for(int i=0; i<tree->moves.size(); ++i){
    			if(tree->children[i]->ratio >= winRatio){
    				selected_id=i;
    				winRatio=tree->children[i]->ratio;
    			}
    	}
    	tree=tree->children[selected_id];

    	free_memory_decision_tree(tree->parent,tree,true);
    	delete tree->parent->children;
    	tree->parent->children=NULL;
    	delete tree->parent;
    	tree->parent=NULL;
	}

	void update(game_state* opp_root){
		//Steps:
		//1-Delete old tree
		//2-Set new root
		//3-Create new tree and choose final node
		int num_iter=1000;
		double winRatio=0;
    	game_state* selected;
    	game_state* expanded;
    	selected=NULL;expanded=NULL;
    	int x_next=0;
    	int y_next=0;
    	int selected_id=0;

    	//Set new root
    	delete tree;
    	tree = new game_state();
    	tree->copy_state(opp_root);

    	//Choosing new node
    	std::cout<<opponent_wins_nextround(tree->board, id, x_next, y_next)<<
    						' '<<x_next<<' '<<y_next<<'\n';
    	if(opponent_wins_nextround(tree->board, id, x_next, y_next)){
    		tree=new game_state(tree,2,x_next,y_next);
    		delete tree->parent;
    		tree->parent=NULL;
    	}else{
	    	//Expanding tree
				for(;;){
	    		selection(tree, &selected);
	    		expansion(selected, &expanded);
	    		simulation(&expanded, id);
	    		backtrace(&expanded, expanded->simu, id);
	    		if(num_iter<=0){
	    			break;
	    		}
	    		if(expanded->n_empty==0){
	    			for(int i=0; i<tree->moves.size();++i){
	    				free_memory_decision_tree(tree->children[i],tree->children[i], true);
	    			}
	    			//print_tree(tree);
	    			selected=NULL;expanded=NULL;
	    		}
	    		num_iter--;
	    	}


	    	//print_tree(tree);

	    	for(int i=0; i<tree->moves.size(); ++i){
    			if(tree->children[i]->ratio >= winRatio){
    				selected_id=i;
    				winRatio=tree->children[i]->ratio;
    			}
    		}
    		tree=tree->children[selected_id];
    		free_memory_decision_tree(tree->parent,tree, true);
    		delete tree->parent->children;
    		tree->parent->children=NULL;
    		delete tree->parent;
    		tree->parent=NULL;
    	}
	}

	bool opponent_wins_nextround(uint (&board)[3][3],int player, int &x, int &y){
		bool win_OP=0;
		bool win_PL=0;
		uint buf_board[3][3];
		memcpy(buf_board,board,sizeof(board));
		for(int i=0;i<3;++i){
			for(int j=0;j<3;++j){
				if(buf_board[i][j]==0){
					//std::cout<<i<<" "<<j<<" "<<player<<" "<<(player^3)<<" \n";
					buf_board[i][j] = (player^3);
					//std::cout<<final_state(board)<<" "<<board[i][j]<<" \n";
					if (final_state(buf_board) == (player^3) ){
						win_OP=true;
						x=i;y=j;
					}

					buf_board[i][j] = player;
					//std::cout<<final_state(board)<<" "<<board[i][j]<<" \n";
					if (final_state(buf_board) == player ){
						win_PL=true;
					}
					
					buf_board[i][j] = 0;
				}
			}
		}
		if(win_OP == true && win_PL == false) return true;
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

	void free_memory_path(game_state* root){
		if(root->parent != NULL){
			free_memory_path(root->parent);
			//print_board(root->board);
			delete root;
		}else{
			//print_board(root->board);
			delete root;
		}
	}

};

#endif