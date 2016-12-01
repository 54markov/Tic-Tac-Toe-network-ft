#ifndef MESSAGE_H
#define MESSAGE_H

enum MSG { AUTH, GAME, CHAT, RESR, EVN };	// enumeration of messages
enum ACT { YES, NO };                   	// enumeration of actions
enum MRK { ZERO, CROSS, EMPTY, FULL, DRAW };// enumeration of marks: 
                                        	//   - valid of players and game_filed
enum SST { MAIN, RESERVE };					// enumeration of server state
enum EVN { TECH_WIN, LOST_CONN, UPD };			// enumeration of events
 
typedef struct {
	int room_state;
    int room_number;         // number of game room
    int room_players[2];     // store players fd:
                             //   - indexation [0] equal [ZERO] - players;
                             //   - indexation [1] equal [CROSS] - players. 
    int room_field[3][3];    // game field
    int room_last;
} room_info;

typedef struct {
    int request;
    /*
     * case NO  - nothing
     * case YES - main server send actual information about all game rooms
     */
    room_info game_rooms[4];    
} resr_msg;

typedef struct {
	// to server
	int new_game; 		// YES - new, NO - continue

	/*
	 * case: new_game = NO -> 'client' fill game_room and player field
	 */

	// from server
	int number;		    // number of game room
	int player; 		// type of player ZERO or CROSS
	int start; 	        // game state YES or NO
} auth_msg;

typedef struct {
	int number;		    // number of game room
	int player;		    // type of players ZERO or CROSS
	int row;			// y
	int col;			// x
} game_msg;

typedef struct {
	int number;			// room game number
	int player;		    // type of players ZERO or CROSS
	char string[256];	// chat message
} chat_msg;

typedef struct {
	int event;
} evnt_msg;

/*
 * Encapsulation of all type messages, between client and server and reserve
 */
 
typedef struct {
	int type;           // type of message
	/*
	 * AUTHOR - for authorization 
	 * GAME   - for game process
	 * CHAT   - for chat communication
	 * RESR   - for main-reserve communication
 	 */
	union
	{
		evnt_msg evnt;
		auth_msg auth;  // authorization message
		game_msg game;  // game message
		chat_msg chat;  // chat message
		resr_msg resr;  // reserve message
	};
} common_msg;

/*
 * Regular situation: Each message will be broadcasted to all reserver servers.
 * If main server is down: Use resr_msg with field request = 1.
 */

#endif