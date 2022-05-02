#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>
double conv(double x){
	double a = pow(-M_E, -4);
	double b = pow(a, x);
	double c = a+1;
	double d = b+1;
	return c/d;
}
unsigned char newTable[4];
/*In general, this games stores cards (and lack thereof) as integers from 0 to 8191. Displaying any number in this range
in binary demonstrates 13 slots which are either 1 or 0, making it ideal for the purposes of showing which cards are present
and which are not. The suits are numbered 0...3 as spades, diamonds, clubs, and hearts. The valuation is completely arbitrary.*/
/*This method is used in debugging to indicate which cards of a given suit are in a hand.*/
void printSuitBinary(unsigned short suit){
	int i = 0;
	for(i = 0; i < 13; i++){
		char c = 0x30+((char)((suit>>i)%2));
		printf("%c",c);
	}
}
/*This prints an entire hand based on the above method, along with a legend.*/
void printHandBinary(unsigned short hand[4]){
	printf("	A23456789XJQK\nS	");
	printSuitBinary(hand[0]);
	printf("\nD	");
	printSuitBinary(hand[1]);
	printf("\nC	");
	printSuitBinary(hand[2]);
	printf("\nH	");
	printSuitBinary(hand[3]);
	printf("\n");
}
/*This considers all the cards that have been played (collectively, a "table") and determines which cards are still playable.*/
int playableSuit(char table[4][2], char suitNum){
	/*This determines which spades are still playable.*/
	if(suitNum == 0){
		unsigned short r = 0;
		/*By default, all the values in the table array are 14. If the following statement is true, that means no
		spades (and by extension, no other cards) have been played. It then returns spade-7 as the only playable card.*/
		if(table[0][0] > 13){
			return 1<<6;
		}
		else{
			r|=(1<<(table[0][0]-1));
			r|=(1<<(table[0][1]+1));
		}
		/*Ace playability is still being coded.*/
		if(table[0][0] == 1 || table[0][1] == 12){
			r|=1;
		}
		return r;
	}
	/*
	 * A non-spade below 7 is playable if the corresponding spade has already been played.
	 * A non-spade above 7 is playable if the correpsonding spade has already been played.
	 * A non-spade with value 7 is playable once the 7 of spades has already been played.
	 */
	else if(table[0][0] < 14){
		unsigned short r = 0;
		if(table[suitNum][0] == 14){
			r|=(1<<6);
		}
		/*Since the default value of the table slots is 14, the value of table[0][0] after the 7 of spades has been played
		will be lower than the slot in an unplayed suit.*/
		else{
			if(table[0][0] < table[suitNum][0]){
				r|=(1<<(table[suitNum][0]-1));
			}
			if(table[0][1] > table[suitNum][1]){
				r|=(1<<(table[suitNum][1]+1));
			}
			/*Ace playability is still being coded.*/
			if(
					(table[suitNum][0] == 1 && table[0][0] == 0) 
					||
					(table[suitNum][1] == 12 && table[0][1] == 13) 
					){
				r|=1;
			}
		}
		return r;
	}
	/*There is no reason this line should execute, but it exists as a failsafe.*/
	return 0;
}
/*This simply prints the suit.*/
void printSuit(char s){
	switch(s){
		case 0:
			printf("Spades");
			break;
		case 1:
			printf("Diamonds");
			break;
		case 2:
			printf("Clubs");
			break;
		case 3:
			printf("Hearts");
			break;
		default:
			printf("NOSUIT");
	}
}
/*This returns a character for a given card value. Since computers begin counting at zero for most purposes, 0 is treated as ace,
1 as two, and so on. The face cards return the first letter of their names, 10 returns 'X' (Roman numeral), and the other cards
return their corresponding digit.*/
char valueChar(char v){
	switch(v){
		case 13:
			return 'A';
		case 0:
			return 'A';
		case 9:
			return 'X';
		case 10:
			return 'J';
		case 11:
			return 'Q';
		case 12:
			return 'K';
		default:
			return 0x31+v;
	}
	return 0x31+v;
}
/*This prints the entire table, see definition at playableSuit.*/
void printTable(char table[4][2]){
	char i;
	for(i = 0; i < 4; i++){
		if(table[i][0] < 13){
			printSuit(i);
			printf("	%c	", valueChar((char)table[i][0]));
			if(table[i][0] != table[i][1]){
				printf("%c", valueChar((char)table[i][1]));
			}
			printf("\n");
		}
	}
}
/*At this point, computer players play cards at random. More intelligent computer players
can be programmed later.*/
int compPlay(unsigned short hand[4], char table[4][2], unsigned char level){
	unsigned short p[4];
	char i, j;
	unsigned char pl = 0;
	short playable[52][2];
	for(i = 0; i < 52; i++){
		playable[i][0] = 53;
		playable[i][1] = 0;
	}
	for(i = 0; i < 4; i++){
		/*The array p contains the cards which are playable on a turn AND that are in the computer player's hand.*/
		p[i] = hand[i]&playableSuit(table, i);
		/*The value p1 contains the total number of playabe cards. The following loop terminates if there are no
		playable cards in the player's hand of the suit currently being examined, if there are no higher playable
		cards in this suit, or when all 13 slots have already been examined.*/
		for(j = 0; p[i] > 0 && p[i] >= 1<<j && j < 13; j++){
			if((p[i]>>j)%2 == 1){
				playable[pl][0] = (i*13)+j;
				playable[pl][1] = 0;
				pl++;
			}
		}
	}
	/*If the computer player has at least one playable card, it will choose one at random.*/
	/*More intelligent algorithm:
	 Add 10 for every point it would cost in the final tally.
	 If there is a higher (or lower) card in the same suit, add 10 for every card in the gap
	 between them.
	 Remove 5 for every card in the gap between the median card in the top "half" of the table
	 and a king of the same suit.
	 Remove 10 for every card in the gap between the median card in the bottom "half" of the
	 table and two of the same suit.*/
	if(pl > 0){
		for(i = 0; pl > 1 && i < pl; i++){
			playable[i][1] = 0;
			if(playable[i][0]%13 == 0){
				playable[i][1]+=200;
			}
			else if(playable[i][0]%13 == 11 && playable[i][0]/13 == 0){
				playable[i][1]+=500;
			}
			else if(playable[i][0]%13 >= 9){
				playable[i][1]+=100;
			}
			else{
				playable[i][1]+=50;
			}
			if(playable[i][0]%13 >= 6){
				if((hand[playable[i][0]/13]-(1<<(playable[i][0]%13)))>>6 > 0){
					j=0;
					while(((hand[playable[i][0]/13]-(1<<(playable[i][0]%13)))>>(6+j))%2 == 0){
						j++;
					}
					playable[i][1]+=(j*10);
				}
				unsigned char top[4] = {9,9,9,9};
				unsigned char topSize = 0;
				for(j = 0; j < 4; j++){
					if(table[j][1] < 14){
						top[j] = table[j][1];
						topSize++;
					}
				}
				unsigned char medTop = 9;
				if(topSize > 2){
					for(j = 0; j < topSize-1; j++){
						for(unsigned char j1 = j+1; j1 < topSize-1; j1++){
							if(top[j1] < top[j]){
								unsigned char temp = top[j1];
								top[j1] = top[j];
								top[j] = temp;
							}
						}
					}
				}
				if(topSize%2==1){
					medTop = top[topSize/2];
				}
				else{
					medTop = top[(topSize/2)-1]+top[topSize/2];
					medTop>>=1;
				}
				playable[i][1]-=(10*(12-medTop));
			}
			if(playable[i][0]%13 <= 6){
				if((hand[playable[i][0]/13]-(1<<(playable[i][0]%13)))%(1<<6) > 0){
					j=0;
					while(((hand[playable[i][0]/13]-(1<<(playable[i][0]%13)))<<j)%(1<<6) < 1<<5){
						j++;
					}
					playable[i][1]+=(j*10);
				}
				unsigned char bottom[4] = {4,4,4,4};
				unsigned char bottomSize = 0;
				for(j = 0; j < 4; j++){
					if(table[j][1] < 14){
						bottom[j] = table[j][0];
						bottomSize++;
					}
				}
				unsigned char medBottom = 4;
				if(bottomSize > 2){
					for(j = 0; j < bottomSize-1; j++){
						for(unsigned char j1 = j+1; j1 < bottomSize; j1++){
							if(bottom[j1] < bottom[j]){
								unsigned char temp = bottom[j1];
								bottom[j1] = bottom[j];
								bottom[j] = temp;
							}
						}
					}
				}
				if(bottomSize%2==1){
					medBottom = bottom[bottomSize/2];
				}
				else{
					medBottom = bottom[(bottomSize/2)-1]+bottom[bottomSize/2];
					medBottom>>=1;
				}
				playable[i][1]-=(10*(medBottom-1));
			}

		}
		if(pl > 1){
			for(i = 0; i < pl-1; i++){
				for(j = i+1; j < pl; j++){
					if(playable[i][1] > playable[j][1]){
						short tempA = playable[i][0];
						short tempB = playable[i][1];
						playable[i][0] = playable[j][0];
						playable[i][1] = playable[j][1];
						playable[j][0] = tempA;
						playable[j][1] = tempB;
					}
				}
			}
			double r = (1.0+((double)(rand()%1000)))/1000.0;
			double c;
			if(level > 1){
				double h = conv(r);
				double d = h-r;
				double f = d*((double)(level-1))/9.0;
				double g = r+f;
				c = g;
			}
			else{
				c=r;
			}
			return playable[(int)(c*pl)][0];
		}
		else{
			return playable[0][0];
		}
	}
	/*There is no reason the following statement should ever be executed, but it's present as a failsafe.*/
	return 53;
}
/*This prints a player's hand.*/
void printHand(unsigned short hand[4], unsigned short p[4], char table[4][2]){
	char s, v;
	char i = 0;
	char pl = 0;
	/*This loop goes through all 52 cards.*/
	for(i = 0; i < 52; i++){
		s = i/13;
		v = i%13;
		/*If a card is found to be in the player's hand, it prints the card.*/
		if((hand[s]>>v)%2==1){
			pl++;
			printf("%d: %c	", (int)pl, valueChar((char)v));
			printSuit(s);
			/*If the card is playable, that is also indicated.*/
			if((p[s]>>v)%2==1){
				printf("	PLAYABLE");
			}
			printf("\n");
		}
	}
}
/*This method treads many of the same paths as the computer player method, but does obviously include input.*/
char humanPlay(unsigned short hand[4], char table[4][2]){
	unsigned short p[4] = {0,0,0,0};
	char pl = 0;
	char h = 0;
	char i = 0;
	char j = 0;
	/*The playable cards in a user's hand are stored in p. The number of playable cards is stored in pl.
	The total number of cards in one's hand is stored in h.*/
	for(i = 0; i < 4; i++){
		p[i] = hand[i]&playableSuit(table, i);
		for(j = 0; j < 13; j++){
			pl+=((p[i]>>j)%2);
			h+=((hand[i]>>j)%2);
		}
	}
	if(pl > 0){
		printHand(hand, p, table);
	}
	int o;
	int g = -1;
	char optStr[256];
	/*The user is allowed to expose the table of cards so-far played, reminding themselves of the cards
	in their own hand, and deciding which card to play. While the inner input options are in do...while
	loops, the outer input option is in a while loop, so that a lack of playable cards will prevent the
	loop from starting.*/
	while(pl > 0 && (g < 0 || g > h)){
		do{
			printf("1: Print table\n2: Print hand\n3: Decide card\nOption: ");
			scanf("%s", optStr);
			o = optStr[0]-'0';
			if(o < 1 || o > 3){
				printf("Invalid option.\n");
			}
		} while(o < 1 || o > 3);
		if(o == 1){
			printTable(table);
		}
		else if(o == 2){
			printHand(hand, p, table);
		}
		else if(o == 3){
			g = -1;
			while(o == 3 && (g < 0 || g > h)){ 
				printf("Choose a card number based on the above list, or zero to cancel: ");
				scanf("%s", optStr);
				optStr[2] = 0;
				if(optStr[0] >= '0' && optStr[0] <= '9' && (optStr[1] == 0 || optStr[1] == '\n' || (optStr[1] >= '0' && optStr[1] <= '9'))){
					g = (int)(atol(optStr));
					if(g < 0 || g > h){
						printf("Invalid option.\n");
					}
					else if(g > 0){
						char s = 0;
						char v = 0;
						for(i = 0; g > 0 && i < 52; i++){
							s = i/13;
							v = i%13;
							g-=((hand[s]>>v)%2);
						}
						if((p[s]>>v)%2 > 0){
							return (s*13)+v;
						}
						else{
							printf("This card is not playable.\n");
							g = h+1;
						}
					}
					else if(g == 0){
						o = -1;
					}
				}
				else{
					printf("Invalid option.\n");
					o = -1;
				}
			}
		}
		o = -1;
	}
	printf("You have no playable cards.\nGame will resume in ");
	j = 5;
	while(j > 0){
        sleep(1);
        printf("%d\n", j);
        j--;
	}
	printf("\n");
	return 53;
}
/*This loop determines which cards a computer will dispose if the next player has no playable cards. The computer
simply decides not to dispose playable cards (if possible) and otherwise acts randomly. More intelligent algorithms
can be designed later.*/
unsigned char compDispose(unsigned short hand[4], char table[4][2], unsigned char level){
	unsigned short p[4] = {0,0,0,0};
	char i, j;
	char pl = 0;
	char t = 0;
	short cards[52][3];
	for(i = 0; i < 4; i++){
		/*The array p contains the cards in the computer player's suit which AREN'T playable.*/
		p[i] = hand[i]&(playableSuit(table, i));
		for(j = 0; hand[i] > 0 && hand[i] >= 1<<j && j < 13; j++){
			if((hand[i]>>j)%2 == 1){
				cards[t][0] = (i*13)+j;
				cards[t][1] = 0;
				if((p[i]>>j)%2 == 1){
					cards[t][2] = 1;
					pl++;
				}
				else{
					cards[t][2] = 0;
				}
				t++;
			}
		}
	}
	for(i = 0; t > 1 && i < t; i++){
		if(cards[i][0]%13 == 0){
			cards[i][1]+=200;
		}
		else if(cards[i][0]%13 == 11 && cards[i][0]/13 == 0){
			cards[i][1]+=500;
		}
		else if(cards[i][0]%13 >= 9){
			cards[i][1]+=100;
		}
		else{
			cards[i][1]+=50;
		}
		if(cards[i][0]%13 != 0){
			if(cards[i][0]%13 < 6){
				for(j = (cards[i][0]%13)+1; j <= 6 && j < table[cards[i][0]/13][0]; j++){
					cards[i][1]+=10;
					if((hand[cards[i][0]/13]>>j)%2 == 1){
						cards[i][1]+=20;
					}
				}
				for(j = cards[i][0]%13; cards[i][0]/13 != 0 && j <= 6 && j < table[0][0]; j++){
					cards[i][1]+=10;
					if((hand[0]>>j)%2 == 1){
						cards[i][1]+=20;
					}
				}
			}
			if(cards[i][0]%13 > 6){
				for(j = (cards[i][0]%13)-1; j > 6 || (table[cards[i][0]/13][0] == 14 && j == 6); j--){
					cards[i][1]+=10;
					if((hand[cards[i][0]/13]>>j)%2 == 1){
						cards[i][1]+=20;
					}
				}
				for(j = cards[i][0]%13; cards[i][0]/13 != 0 && j >= 6; j--){
					cards[i][1]+=10;
					if((hand[0]>>j)%2 == 1){
						cards[i][1]+=20;
					}
				}
			}
		}
		else{
			if(table[0][0] == 0 || table[cards[i][0]/13][0] < 13-table[cards[i][0]/13][1]){
				for(j = 1; j < table[cards[i][0]/13][0] && j <= 6; j++){
					cards[i][1]+=10;
					if((hand[cards[i][0]/13]>>j)%2 == 1){
						cards[i][1]+=20;
					}
				}
				for(j = 0; cards[i][0]/13 != 0 && j < table[0][0] && j <= 6; j++){
					cards[i][1]+=10;
					if((hand[0]>>j)%2 == 1){
						cards[i][1]+=20;
					}
				}
			}
			if(table[0][1] == 13 || table[cards[i][0]/13][0] > 13-table[cards[i][0]/13][1]){
				for(j = 12; j > 6 || (table[cards[i][0]/13][0] == 14 && j == 6); j--){
					cards[i][1]+=10;
					if((hand[cards[i][0]/13]>>j)%2 == 1){
						cards[i][1]+=20;
					}
				}
				for(j = 13; cards[i][0] != 0 && j > table[0][1] && j >= 6; j--){
					cards[i][1]+=10;
					if((hand[0]>>j)%2 == 1){
						cards[i][1]+=20;
					}
				}
			}
		}
	}
	/*There are playable cards, but not all cards are playable:
	 * 	Sort playable cards towards the beginning, sort playable and non-playable cards separately
	 *All cards are immediately playable OR no cards are immediately playable:
	 	Sort entire array once*/
	if(t > 1){
		if(t != pl && pl != 0){
			for(j = 0; j < t-1; j++){
				for(unsigned char j1 = j+1; j1 < t; j1++){
					if(cards[j1][2] > cards[j][2]){
						short temp = cards[j1][0];
						cards[j1][0] = cards[j][0];
						cards[j][0] = temp;
						temp = cards[j1][1];
						cards[j1][1] = cards[j][1];
						cards[j][1] = temp;
						temp = cards[j1][2];
						cards[j1][2] = cards[j][2];
						cards[j][2] = temp;
					}
				}
			}
			for(j = 0; j < pl-1; j++){
				for(unsigned char j1 = j+1; j1 < pl; j1++){
					if(cards[j1][1] < cards[j][1]){
						short temp = cards[j1][0];
						cards[j1][0] = cards[j][0];
						cards[j][0] = temp;
						temp = cards[j1][1];
						cards[j1][1] = cards[j][1];
						cards[j][1] = temp;
						temp = cards[j1][2];
						cards[j1][2] = cards[j][2];
						cards[j][2] = temp;
					}
				}
			}
		}
		for(j = pl; j < t-1; j++){
			for(unsigned char j1 = j+1; j1 < t; j1++){
				if(cards[j1][1] < cards[j][1]){
					short temp = cards[j1][0];
					cards[j1][0] = cards[j][0];
					cards[j][0] = temp;
					temp = cards[j1][1];
					cards[j1][1] = cards[j][1];
					cards[j][1] = temp;
					temp = cards[j1][2];
					cards[j1][2] = cards[j][2];
					cards[j][2] = temp;
				}
			}
		}
	}
	/*If there are playable cards, only allow a 5% chance that they will be chosen.
		  If a playable card is chosen
			If there is only one playable card, play it.
			Otherwise if the player is level 1
				Choose a random number among the playable cards
			Otherwise
				Choose a random number among the playable cards, and process it to choose a more advantageous card
		 Otherwise
			If there is only one non-playable card, play it
			Otherwise if the player is level 1
				Choose a random number among the non-playable cards
			Otherwise
				Choose a random number among the non-playable cards, and process it to choose amore advantageous card
	 Otherwise
		If there is only one card, play it.
		Otherwise if the player is level 1
			Choose a random number among the cards
		Otherwise
			Choose a random number among the playable cards, and process it to choose a more advantageous card
	*/
	if(pl > 0){
		int r = rand();
		if(r < RAND_MAX/20){
			if(pl == 1){
                if(cards[0][0] > 52){
                    printf("Garbage card\n");
                }
				return cards[0][0];
			}
			if(level > 1){
				double rd = ((double)rand())/((double)RAND_MAX);
				double c = conv(rd);
				double g = ((c-rd)*((double)(level-1))/9.0)+rd;
				return cards[(int)(g*pl)][0];
			}
            r = rand()%pl;
            if(cards[r][0] > 52){
                printf("Garbage card\n");
            }
			return cards[r][0];
		}
		if(t-pl == 1){
			return cards[pl][0];
		}
		if(level > 1){
			double rd = ((double)rand())/((double)RAND_MAX);
			double c = conv(rd);
			double g = ((c-rd)*((double)(level-1))/9.0)+rd;
			if(t != pl){
                if(cards[pl+((int)(g*(t-pl)))][0] > 52){
                    printf("Garbage card\n");
                }
				return cards[pl+((int)(g*(t-pl)))][0];
			}
            if(cards[((int)(g*t))][0] > 51){
                printf("Garbage card\n");
            }
            return cards[((int)(g*t))][0];
		}
		if(t != pl){
            r = (rand()%(t-pl))+pl;
            if(cards[(rand()%(t-pl))+pl][0] > 52){
                printf("Garbage card\n");
            }
			return cards[(rand()%(t-pl))+pl][0];
		}
        r =(rand()%t);
        if(cards[(rand()%t)][0] > 52){
            printf("Garbage card\n");
        }
		return cards[(rand()%t)][0];
	}
	if(t > 1){
		if(level > 1){
			double rd = ((double)rand())/((double)RAND_MAX);
			double c = conv(rd);
			double g = ((c-rd)*((double)(level-1))/9.0)+rd;
            if(cards[(int)(g*t)][0] > 52){
                printf("Garbage card\n");
            }
			return cards[(int)(g*t)][0];
		}
        int r = rand()%t;
        if(cards[r][0] > 51){
            printf("Garbage card\n");
        }
		return cards[r][0];
	}
    if(cards[0][0] > 51){
        printf("Garbage card\n");
    }
	return cards[0][0];
}
unsigned char humanDispose(unsigned short hand[4], char table[4][2]){
	/*This array will contain the cards that are playable.*/
	unsigned short p[4] = {0,0,0,0};
	char pl = 0;
	char i = 0;
	char j = 0;
	/*The loop goes suit by suit and finds the playable cards and tallies up the total
	 * number of cards in the user's hand.*/
	for(i = 0; i < 4; i++){
		p[i] = playableSuit(table, i);
		for(j = 0; j < 13; j++){
			pl+=((hand[i]>>j)%2);
		}
	}
	printHand(hand, p, table);
	printf("The next player needs a card from you.\n");
	int o = -1;
	int g = -1;
	unsigned char optStr[256];
	/*This loop continues only as long as o is -1, or as long as g is an invalid value. The variable
	 * o contains the option the player chooses and g holds the card the player chooses to give the
	 * previous player.*/
	while(o == -1 || g < 0 || g > pl){
		/*This is the inner loop for the option. It continues until some number between 1 and 3
		 * inclusive is chosen. It currently can only handle integer input.*/
		while (o < 1 || o > 3){
			printf("1: Print table\n2: Print hand\n3: Decide card\nOption: ");
			scanf("%s", optStr);
			o = optStr[0]-'0';
			if(o < 1 || o > 3){
				printf("Invalid option.\n");
			}
		}
		/*If the user chooses 1, the table is printed.*/
		if(o == 1){
			printTable(table);
		}
		/*If the user chooses 2, their own hand is printed.*/
		else if(o == 2){
			printHand(hand, p, table);
		}
		/*If the user chooses 3, they are prompted to choose
		 * a card to dispose.*/
		else if(o == 3){
			/*This is the inner loop for choosing a card to dispose. It continues until a number between 0 and
			 * the total number of cards in the player's hand (inclusive) is chosen.*/
			g = -1;
			while(
                  g < 0 ||
                  g > pl){
				printf("Choose a card number based on the above list, or zero to cancel: ");
				scanf("%s", optStr);
				optStr[2] = 0;
				if(optStr[0] >= '0' && optStr[0] <= '9' && (optStr[1] == 0 || optStr[1] == '\n' || (optStr[1] >= '0' && optStr[1] <= '9'))){
					g = (int)(atol((char*)optStr));
				}
				/*This statement handles the invalid options.*/
				if(g < 0 || g > pl){
					printf("Invalid option.\n");
				}
				/*If the user chose a card in their hand, the following routine will check if it is playable, discourage
				 * the player from disposing it if so, and send it out for disposal if the user agrees to knowingly dispose
				 * a playable card or if it was not playable to begin with.*/
				else if(g > 0){
					char s = 0;
					char v = 0;
					/*In the following loop, g is decrmented for every card in the deck that the player actually has.
					 * It hits 0 upon finding the user-selected card.*/
					for(i = 0; g > 0 && i < 52; i++){
						s = i/13;
						v = i%13;
						g-=((hand[s]>>v)%2);
					}
					/*This handles the user choosing a playable card.*/
					if((p[s]>>v)%2 > 0){
						/*This is a string that will contain the user's ultimate choice. Though only the first 
						 * character is important, using a string handles multi-character input, accidental or
						 * deliberate.*/
						char u[256] = {'a'};
						while(u[0] != 'n' && u[0] != 'N' && u[0] != 'y' && u[0] != 'Y'){
							printf("This is a playable card. Are you sure you want to dispose of it? y/n ");
							scanf("%s", u);
							/*The loop repeats until the first character of the string is N or Y.*/
							if(u[0] != 'n' && u[0] != 'N' && u[0] != 'y' && u[0] != 'Y'){
								printf("Invalid option.\n");
							}
							/*If the user types Y, the method returns the card in question.*/
							else if(u[0] == 'y' || u[0] == 'Y'){
								return (s*13)+v;
							}
							/*If the user types N, the loop resets to where the user is prompted to choose a card.*/
							else if(u[0] == 'n' || u[0] == 'N'){
								g = -1;
							}
						}
					}
					/*In this case, the card was not playable to begin with, and it is disposed without
					 * further question.*/
					else{
                        if((s*13)+v > 52){
                            printf("Garbage card\n");
                        }
						return (s*13)+v;
					}
				}
				else{
					o = -1;
				}
			}
		}
		/*This resets o to -1 to make sure the loop repeats until a card is disposed.*/
		o = -1;
	}
	/*This return statement should never be called, but exists to prevent compilation errors.*/
	return 53;
}
unsigned char dispose(char pNum, char curP, unsigned short hand[pNum][4], char table[4][2], unsigned short humans, unsigned char levels[pNum]){
	if((humans>>curP)%2 == 1){
		return humanDispose(hand[curP], table);
	}
	return compDispose(hand[curP], table, levels[curP]);
}
unsigned char play(char pNum, char curP, unsigned short hand[pNum][4], char table[4][2], unsigned short humans, unsigned char levels[pNum]){
	char r;
	if((humans>>curP)%2 == 1){
		r = humanPlay(hand[curP], table);
		if(r%13 == 0){
			/*Handles aces*/
			if(r/13 == 0 && table[0][0] == 1 && table[0][1] == 12){
				printf("Where would you like to play the ace? Press 2 for two, press K for king: ");
				char s[256];
				do{
					scanf("%s", s);
					if(s[0] != '2' && s[0] != 'k' && s[0] != 'K'){
						printf("Invalid option\n");
					}
				} while(s[0] != '2' && s[0] != 'k' && s[0] != 'K');
				if(s[0] != '2'){
					r+=104;
				}
			}
			else if(table[0][1] > 11){
				r+=104;
			}
		}
	}
	else{
		r = compPlay(hand[curP], table, levels[curP]);
		if(r%13 == 0){
			/*Handles aces*/
			if(r/13 == 0 && table[0][0] == 1 && table[0][1] == 12){
				r+=((rand()%2)*104);
			}
			else if((r/13 == 0 && table[0][1] == 12) || (r/13 != 0 && table[0][1] == 13)){
				r+=104;
			}
		}
	}
    if(r > 53 && !(r >= 104 && r%13==0)){
        printf("Garbage card\n");
    }
	return r;
}
char compTurn(char pNum, char curP, unsigned short hand[pNum][4], char table[4][2], unsigned short humans, unsigned char levels[pNum]){
    if(humans!=0){sleep((!((humans>>curP)%2))*(1+(rand()%5)));}
	unsigned char d = play(pNum, curP, hand, table, humans, levels);
	if(d>=52 && d < 104){
		d = dispose(pNum, (curP+pNum-1)%pNum, hand, table, humans, levels);
		hand[(curP+pNum-1)%pNum][d/13]^=(1<<(d%13));
		hand[curP][d/13]|=(1<<(d%13));
        if(humans != 0){
            printf("Player %d has taken a card from player %d\n", curP+1, ((curP+pNum-1)%pNum)+1);}
	}
	else{
        if((d < 0 || d > 51) && d != 104 && d != 104+13 && d != 104+26 && d!= 104+39){
            printf("Garbage card played.\n");
            return 1;
        }
		if(d%13 == 0){
			/*Handles aces*/
			if(d >= 104){
				d-=104;
				table[d/13][1] = 13;
			}
			else{
				table[d/13][0] = 0;
			}
			hand[curP][(d%104)/13]^=1;
		}
		else if(d%13 == 6){
			table[d/13][0] = 6;
			table[d/13][1] = 6;
			hand[curP][d/13]^=(1<<(d%13));
		}
		else if(d%13 > 6){
			table[d/13][1]++;
			hand[curP][d/13]^=(1<<(d%13));
		}
		else if(d%13 < 6){
			table[d/13][0]--;
			hand[curP][d/13]^=(1<<(d%13));
		}
		else{
			printf("ERROR - CARD PLAYED WHEN NOT PLAYABLE\n");
			return 1;
		}
        if(humans != 0){
		printf("Player %d has played the %c of ", curP+1, valueChar(d%13));
		printSuit(d/13);
            printf("\n");}
	}
	return 0;
}
char handCount(unsigned short hand[4]){
	char i;
	char h = 0;
	for(i = 0; i < 52; i++){
		h+=((hand[i/13]>>(i%13))%2);
	}
	return h;
}
int points(unsigned short hand[4]){
	char i = 0;
	char j = 0;
	unsigned short t = 0;
	/*50 points is added to whomever holds the queen of spades at the end of the
	game.*/
	t+=(50*((hand[0]>>11)%2));
	/*The queen of hearts, if present, is removed from the hand. NOT 2^11 in binary becomes 1111111111101. Since 1&X=X and
	0&X=0, the Q's place will be zero after the following statement no matter its original value, while every other place
	will remain the same no matter their original value.*/
	hand[0]&=(~(1<<11));
	for(i = 0; i < 4; i++){
		/*This adds 20 to the count for every ace in a player's hand.*/
		t+=(20*((hand[i])%2));
		/*This adds 5 points for every card from 2 to 9, inclusive.*/
		for(j = 1; j < 8; j++){
			t+=(5*((hand[i]>>j)%2));
		}
		/*This adds 10 points for every card from 10 to king, inclusive. This
		is why the queen of spades had to be removed.*/
		for(j = 8; j < 13; j++){
			t+=(10*((hand[i]>>j)%2));
		}
	}
	return t;
}
/*This is the primary function of the program. The parameters are the first player, the number of players, the table, and
a variable the encodes the human players in binary.*/
int turn(char f, char pNum, unsigned short hand[pNum][4], char table[4][2], unsigned short humans, unsigned char levels[pNum]){
	unsigned char hu = pNum+1;
	for(unsigned char i = 0; i < pNum && hu > pNum; i++){
		if((humans>>i)%2 == 1){
			hu = i;
		}
	}
	unsigned short humanHist[2][4];
	for(unsigned char i = 0; i < 4; i++){
		humanHist[0][i] = hand[hu][i]&playableSuit(table, i);
	}
	/*This sets the variable that will contain the size of the smallest hand at 53. Obviously, this is impossible, so it
	will definitely be lowered to a more reasonable number during the first iteration of the while loop below.*/
	char minHand = 53;
	/*The following two values are used for various for loops.*/
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned short k = 0;
	/*The following variable is a player cursor. It starts at whomever has the 7 of spades, as determined by the deal
	function below.*/
	char p = f;
	/*This variable will exceed zero if two or more players are somehow sharing a card, or a card is played out of turn. 
	It is technically the return value, which feeds into the return value of main, so it can bring down the entire program
	if it exceeds zero.*/
	char y = 0;
	/*This loop ultimately calls most of the other functions in this program, and continues as long as every card only exists
	on the table or in one player's hand, while only playable cards are put on the table, and while every player has at least
	one card.*/
	do{
        if((humans>>p)%2 == 1){
            for(i = 0; humans != 0 && i < pNum; i++){
                printf("Player %d has %d cards.\n", i+1, handCount(hand[i]));
            }
        }
		char t = compTurn(pNum, p, hand, table, humans, levels);
		y = t*(y+t);
        if(humans!=0){
            printTable(table);}
        
		k = 0;
		/*This is a routine to make sure no two players somehow get the same card.*/
		for(i = 0; i < 52 && k < 2; i++){
			k = 0;
			char s = i/13;
			char v = i%13;
			for(j = 0; j < pNum && k < 2; j++){
				k+=((hand[j][s]>>v)%2);
			}
			if(k >= 2){
				printf("%d players have the %c of ", k, valueChar(v));
				printSuit(s);
				printf("\n");
				y = pNum+1;
			}
		}
		/*This determines if anyone has fewer than one card.*/
		for(i = 0; i < pNum && minHand > 0; i++){
			char h=handCount(hand[i]);
			if(h<minHand){
				minHand = h;
			}
		}
		for(i = 0; i < 4 && y == 0; i++){
			/*This array contains the current list of playable cards and what the list was last turn.*/
			humanHist[1][i] = humanHist[0][i];
			humanHist[0][i] = hand[hu][i]&playableSuit(table, i);
			for(j = 0; j < 13 && y == 0; j++){
				/*If a playable card from last round is now not considered playable for this round and it was not disposed
				 * to another player*/
				if((humanHist[1][i]>>j)%2>(humanHist[0][i]>>j)%2){
					char t = 0;
					for(k = 0; t == 0 && k < pNum; k++){
						if(k != hu && (hand[k][i]>>j)%2 == 1){
							t++;
						}
					}
					/*If it is nowhere on the table*/
					if(t == 0 && ((j == 0 && table[i][0] != 0 && table[i][1] != 13) || (j < 6 && j > 0 && table[i][0] > j) || (j >= 6 && table[i][1] < j))){
						/*Pork and beans*/
						playableSuit(table, i);
						printf("Card is somehow no longer playable despite formerly being playable.");
						y = pNum+1;
					}
				}
			}
		}
		/*This starts the next player's turn.*/
		p++;
		p%=pNum;
	} while(minHand > 0 && y < pNum);
	if(minHand < 1 && y < pNum){
		/*This ranks the players. If someone were to be holding every card in the deck at the end of a game, they
		would have 460 points. This is obviously impossible, so adding the actual point numbers plus a multiple
		of 460 is a convenient way to store both rank and points, using division and modulo arithmetic to split them.*/
		unsigned short rank[pNum];
		for(j = 0; j < pNum; j++){
			rank[j]=(j*460)+points(hand[j]);
		}
		for(i = 0; i < pNum-1; i++){
			for(j = i+1; j < pNum; j++){
				if(rank[j]%460 < rank[i]%460){
					k = rank[j];
					rank[j] = rank[i];
					rank[i] = k;
				}
			}
		}
		y = 0;
        if(humans != 0){
            printf("Player %d has won.\n", (rank[0]/460)+1);
            for(j = 1; j < pNum; j++){
                printf("Player %d has %d points.\n", (rank[j]/460)+1, rank[j]%460);
            }
        }
	}
	if(y != 0){
		for(unsigned char i = 0; i < 4; i++){
			playableSuit(table, i);
		}
	}
	return (int)y;
}
/*This randomly deals every card to the players and returns the starting player, who has the 7 of spades.*/
char deal(char pNum, unsigned short hands[pNum][4]){
	unsigned short deck[4] = {8191,8191,8191,8191};
	char i, j, s = 0, v = 0;
	char f = pNum+1;
	for(i = 0; i < 52; i++){
		char r = 1+(rand()%(52-i));
		for(j = 0; r > 0 && j < 52; j++){
			s = j/13;
			v = j%13;
			r-=((deck[j/13]>>(j%13))%2);
		}
		hands[i%pNum][s]|=(1<<v);
		deck[s]^=(1<<v);
		if(s == 0 && v == 6){
			f = i%pNum;
            
        }
	}
	return f;
}
int main1(void){
    srand((unsigned int)(time(0)));
    int y = 0;
    long time = clock();
    printf("Output\n");
    for(unsigned int x = 0; y == 0 && x < UINT_MAX; x++){
        if(clock()-time >= 1000000){
            printf("%f%%\n", 100*((double)x)/((double)UINT_MAX));
            time = clock();
        }
        unsigned char i, j;
        unsigned char pNum = 3+(rand()%6);
        unsigned short hand[pNum][4];
        unsigned char levels[pNum];
        for(i = 0; i < pNum; i++){
            levels[i] = 1+(rand()%9);
            for(j = 0; j < 4; j++){
                hand[i][j] = 0;
            }
        }
        char table[4][2] = {
            {14, 14},
            {14, 14},
            {14, 14},
            {14, 14}
        };
        y = turn(deal(pNum, hand), pNum, hand, table, 0, levels);
    }
    return y;
}
int main(void){
	srand((unsigned int)(time(0)));
	int y = 0;
	printf("Spades Rule.\n(c) Jake Raymond, 2021\n");
	unsigned char optStr[256] = {'a', 0};
	while(y == 0 && optStr[0] != '3'){
		while(optStr[0] < '1' || optStr[0] > '3'){
			printf("1: Start\n2: Rules\n3: Quit\nChoose: ");
			scanf("%s", optStr);
			if(optStr[0] < '1' || optStr[0] > '3'){
				printf("Invalid option.\n");
			}
		}
		if(optStr[0] == '1'){
			while(optStr[0] < '3' || optStr[0] > '8'){
				printf("Please insert number of players, between 3 and 8: ");
				scanf("%s", optStr);
				while(optStr[0] < '3' || optStr[0] > '8'){
					printf("Invalid option.\n");
				}
			}
			char pNum = optStr[0]-'0';
			/*None of the numbers hand should ever exceed 8191, so it would likely be most practical to store them as shorts.*/
			unsigned short hand[pNum][4];
			char i = 0;
			char j = 0;
			unsigned char levels[pNum];
			optStr[0] = 'a';
			for(i = 0; i < pNum; i++){
				unsigned char l = 11;
				while(i != 0 && (l < 1 || l > 10) && (optStr[0] < '0' || optStr[0] > '9')){
					printf("Difficulty for Player %d, 1-10: ", i+1);
					scanf("%s", optStr);
					if(optStr[0] < '0' || optStr[0] > '9'){
						printf("Invalid option.\n");
					}
					else{
						optStr[2] = 0;
						if(optStr[1] == 0 || optStr[1] == '\n' || (optStr[1] >= '0' && optStr[1] <= '9')){
							l = (int)(atol((char*)optStr));
							if(l < 1 || l > 10){
								printf("Invalid option.\n");
								optStr[0] = 'a';
							}
							else{
								levels[i] = l;
							}
						}
					}
				}
				optStr[0] = 'a';
				for(j = 0; j < 4; j++){
					hand[i][j] = 0;
				}
			}
			char table[4][2] = {
				{14, 14},
				{14, 14},
				{14, 14},
				{14, 14}
			};
			newTable[0] = 238;
			newTable[1] = 238;
			newTable[2] = 238;
			newTable[3] = 238;
			y = turn(deal(pNum, hand), pNum, hand, table, 1, levels);
			optStr[0] = '0';
		}
		else if(optStr[0] == '2'){
			printf("The first person without any cards wins. The 7 of spades is the only\ncard that can be put on the table immediately at the start of the game.\nAfterwards, another spade can be played as long as all the other spades\nbetween it and 7 have already been played. The ace of spades can be played\non the 2 or king of spades. A non-spade can be played if the spade of the\nsame value has been played, and all the cards of the same suit between it\nand 7 have also been played. An ace of suits other than spades can only be\nplayed at the same end as the ace of spades (e.g. if the ace of spades has\nbeen played on the 2, even if the king of diamonds has been played, the ace\nof diamonds is still only playable if the 2 of diamonds has already been\nplayed).\nAt the end of the game, points are tallied up. The queen of spades is worth 50\npoints. All other face cards and all 10s are worth 10. All aces are worth 20.\nAll other cards are worth 5. Higher point counts are worse than lower counts.\nA - Ace\nX - 10\nJ - Jack\nQ - Queen\nK - King\nNumber - Face value\n");
			optStr[0] = '0';
		}
	}
	return y;
}
