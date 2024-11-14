/* 
   IZP:   proj3.c 
   autor: Klára Jánová
   login: xjanov10 
   datum: 8.12. 2014
*/

//#define _CRT_SECURE_NO_DEPRECATE	//visual studiu se nelibi funkce fscanf:)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define STRING_MATCH 0 
#define RIGHT 1 //hledam RPATH
#define LEFT 0  //hledam LPATH

enum Hranice
{
	LEFT_BORDER = 1,    //bin. 001
	RIGHT_BORDER = 2,	// 010
	TOPBOT_BORDER = 4	// 100
};

typedef struct mapa
{
	int radku;
	int sloupcu;
	unsigned char *bunky;
} Mapa;

enum Operace			  //vycet provadenych operaci v programu
{
	Help,
	Test,
	Rpath,
	Lpath,
	Spath
};

typedef struct parametry    //arg. programu
{
	char *soubor;
	enum Operace Operace;
	int startRadekIdx;
	int startSloupecIdx;
} Parametry;

enum Chyby
{
	Ok,			 //zadna chyba => 0
	EParametry,  //chyba parametru => 1
	EAlokace,    //chyba alokace   => 2
	EMapa,       //chyba mapy      => 3
	ESoubor,	 //chyba souboru   => 4
	EUnknown	 //jina chyba      => 5
};

const char *chyboveHlasky[] =
{
	"ok\n",
	"chybne parametry\n",
	"nedostatek pameti\n",
	"chybna mapa\n",
	"nelze otevrit soubor\n",
	"neznama chyba\n"
};



int vypisChybu(int chyba)
{
	if (chyba <= Ok || chyba > ESoubor)      //enum prideluje konstantam cisla, tudiz muzu porovnavat
		chyba = EUnknown;
	fprintf(stderr, "%s", chyboveHlasky[chyba]);

	return chyba;
}

bool StringCompare(char *prvniString, char *druhyString)
{
	return(strcmp(prvniString, druhyString) == STRING_MATCH);
}

int nactiParametry(int argc, char **argv, Parametry *parametry)
{
	parametry->startRadekIdx = -1; //chceme matici indexovat od nuly
	parametry->startSloupecIdx = -1;

	if (argc == 2 && StringCompare(argv[1], "--help"))
	{
		parametry->Operace = Help;
	}
	else if (argc == 3 && StringCompare(argv[1], "--test"))
	{
		parametry->soubor = argv[2];
		parametry->Operace = Test;
	}

	else if (argc == 5)
	{
		parametry->startRadekIdx = atoi(argv[2]);
		parametry->startSloupecIdx = atoi(argv[3]);
		--parametry->startRadekIdx; //chceme matici indexovat od nuly
		--parametry->startSloupecIdx;
		parametry->soubor = argv[4];

		if (StringCompare(argv[1], "--rpath")) //--rpath  R C <Tab.txt
			parametry->Operace = Rpath;
		else if (StringCompare(argv[1], "--lpath")) //--lpath R C <tab.txt
			parametry->Operace = Lpath;
		else if (StringCompare(argv[1], "--shortest"))
			parametry->Operace = Spath;
		else
			return EParametry;
	}
	else
		return EParametry;

	return Ok;
}

void vypisNapovedu(void)
{
	printf("Autor: xjanov10\n\n"
		"Popis programu:\n"
		"Program v bludisti zadanem textovym souborem hleda cestu ven dle libovolnych souradnic.\n"
		"Vypise po sobe nasledujici souradnice policek bludiste, kterymi vede cesta z bludiste ven.\n\n"
		"argumenty programu:\n"
		"--help: vytiskne napovedu a skonci program\n"
		"--test nazevsouboru.txt: zkontroluje, zda je mapa zadana spravne\n"
		"--rpath nazevsouboru.txt: vyhleda a vypise cestu od bodu zadanych souradnic podle pravidla prave ruky\n"
		"--lpath nazevsouboru.txt: vyhleda a vypise cestu od bodu zadanych souradnic podle pravidla leve ruky\n"
		"--shortest nazevsouboru.txt: vyhleda a vypise nejkratsi moznou cestu od bodu zadanych souradnic\n");
}


bool jeBunkaValidni(int c)  //prvek mapy je validni pokud obsahuje cislo v rozsahu 0-7
{
	return c >= 0 && c <= 7;
}

unsigned char getItem(Mapa *mapa, int radek, int sloupec) //ziskej prvek na souradnicich R C
{

	int index = ((mapa->sloupcu * (radek)) + sloupec);
	unsigned char hodnota = mapa->bunky[index];
	return hodnota;
}

bool isBorder(Mapa *mapa, int radek, int sloupec, int hranice) //funkce na hledani co je na kterym indexu za cislo na urceni hranice
{
	return getItem(mapa, radek, sloupec) & hranice;
}

int nactiMapu(FILE *soubor, Mapa *mapa)    // pomocna funkce pro zjisteni validity mapy; mapa je validni pokud ma pozadovane rozmezi prvku
{										   // a zaroven prvky splnuji rozsah 0-7, a pokud sedi navzajem hrany policek
	int cislo;
	int i = 0;
	int ctiZeSouboru;

	while ((ctiZeSouboru = fscanf(soubor, "%d", &cislo)) != EOF)	//zaciname nacitat prvky do struktury
	{
		if (i >= mapa->radku*mapa->sloupcu)	 //pokud je zadano vic prvku nez bylo definovano na 1.radku souboru->chyba
			return EMapa;

		if (!ctiZeSouboru)
			return EMapa;

		mapa->bunky[i++] = cislo;
	}

	if (i != mapa->radku*mapa->sloupcu)		//pokud je zadano mene prvku nez bylo definovano na 1.radku souboru ->chyba
		return EMapa;

	return Ok;
}

bool jeLichy(int index)		//zjisteni typu policka podle toho, jestli je radek/sloupec sudy/lichy
{
	return index % 2;
}

bool jeSudy(int index)
{
	return !(index % 2);
}

int jeMapaValidni(Mapa *mapa, Parametry *parametry)	//funkce kontroluje hrany bludiste-mapa je validni pokud sedi navzajem hrany policek
{
	for (int i = 0; i < mapa->radku; i++)			//radky zaciname od nuly do maxima radku
		for (int j = 0; j < mapa->sloupcu; j++)		//sloupce taky
		{
		if (!jeBunkaValidni(getItem(mapa, i, j)))  //kontrola prvku 0-7, pokud nevalidni, vypisu kde presne
			return EMapa;

		if (j != mapa->sloupcu - 1 &&		//kontrola prave steny s levou stenou policka napravo
			isBorder(mapa, i, j, RIGHT_BORDER) != isBorder(mapa, i, j + 1, LEFT_BORDER))  //pokud j neni rovno
			return EMapa;

		if (j != 0 &&				// kontrola leve steny s pravou stenou policka nalevo
			isBorder(mapa, i, j, LEFT_BORDER) != isBorder(mapa, i, j - 1, RIGHT_BORDER))
			return EMapa;

		if ((i != mapa->radku - 1)   // kontrola spodni steny s horni stenou policka pode mnou
			&& ((jeSudy(i) && jeLichy(j)) || (jeLichy(i) && jeSudy(j)))
			&& isBorder(mapa, i, j, TOPBOT_BORDER) != isBorder(mapa, i + 1, j, TOPBOT_BORDER))
			return EMapa;
		}
	if (parametry->Operace != Test) //vstup je mimo bludiste
	{
		if (parametry->startRadekIdx >= mapa->radku ||
			parametry->startSloupecIdx >= mapa->sloupcu ||
			parametry->startRadekIdx < 0 ||
			parametry->startSloupecIdx < 0)
			return EMapa;
	}

	return Ok;
}

int vytvorMapu(Mapa *mapa, Parametry *parametry)
{
	int chyba;
	FILE *fp = fopen(parametry->soubor, "r");                                  //otevri soubor pro cteni

	if (fp == NULL)												   //nespravne provedene otevreni souboru vraci null
		return ESoubor;

	chyba = fscanf(fp, " %d %d ", &mapa->radku, &mapa->sloupcu);   //nacti ze souboru pocet prvku a sloupcu abych vedela kolik mista alokovat

	if (chyba != 2)												   //nenacteny prave 2 prvky->chyba mapy, zavri soubor
	{
		fclose(fp);
		return EMapa;
	}

	if (mapa->radku <= 0 || mapa->sloupcu <= 0) 
	{
		fclose(fp);
		return EMapa;
	}

	mapa->bunky = malloc(sizeof(char)*mapa->radku*mapa->sloupcu);	//alokujeme!
	if (mapa->bunky == NULL)
	{
		fclose(fp);
		return EAlokace;
	}

	chyba = nactiMapu(fp, mapa);	//zjistime validitu mapy, zda sedi sloupce a radky

	if (chyba != Ok)				//pokud neni validni, uvolnime alokovanou pamet a zavrem soubor
	{
		fclose(fp);
		free(mapa->bunky);
		return chyba;
	}

	fclose(fp);

	chyba = jeMapaValidni(mapa, parametry);	//opet zjisteni validity, tentokrat zda sedi hrany policek
	if (chyba != Ok)				//nesedi->dealokace, zavrit soubor
	{
		free(mapa->bunky);
		return chyba;
	}

	return Ok;
}

int start_border(Mapa *mapa, int row, int col, int leftright) //funkce podle zvoleneho parametru rpath/lpath vrátí, která hranice se má po
															  //vstupu do bludiste následovat
{
	if (leftright == RIGHT)				//byl zvolen parametr rpath
	{
		if (jeSudy(row) && col == 0)	//pokud je radek sudy a sloupec 0 -> vrat pravou stenu
			return RIGHT_BORDER;
		else if (jeLichy(row) && col == 0)	//lichy radek a sloupec 0-> vrat horni/dolni stenu
			return TOPBOT_BORDER;
		else if (col == mapa->sloupcu - 1 && jeSudy(row + col))	//sudy radek
			return TOPBOT_BORDER;
		else if (col == mapa->sloupcu - 1 && jeLichy(row + col))  
			return LEFT_BORDER;
		else if (row == 0)				//pokud je radek nulty, zaciname od leva stena
			return LEFT_BORDER;
		else if (row == mapa->radku - 1)	//pokud je radek
			return RIGHT_BORDER;
		else
			return EMapa;
	}

	else //zvolen par. lpath
	{
		if (jeSudy(row) && col == 0)	//pokud je radek sudy a sloupec 0 -> vrat pravou stenu
			return RIGHT_BORDER;
		else if (jeLichy(row) && col == 0)	//lichy radek a sloupec 0-> vrat pravou, ...
			return RIGHT_BORDER;
		else if (col == mapa->sloupcu - 1 && jeSudy(row + col))
			return TOPBOT_BORDER;
		else if (col == mapa->sloupcu - 1 && jeLichy(row + col))
			return LEFT_BORDER;
		else if (row == 0)				
			return RIGHT_BORDER;
		else if (row == mapa->radku - 1)
			return LEFT_BORDER;
		else
			return EMapa;

	}
}

int getNextRpathBorder(int row, int col, int border)   //hledam dalsi hranici od minuleho policka a pokracuju v hledani cesty
{
	if (border == TOPBOT_BORDER && jeLichy(row) && jeSudy(col)) //pokud je hranice horni/dolni a radek je lichy,sloupec sudy,hranice je vpravo
		return RIGHT_BORDER;

	else if (border == TOPBOT_BORDER && jeLichy(row) && jeLichy(col))
		return LEFT_BORDER;

	else if (border == TOPBOT_BORDER && jeSudy(row) && jeSudy(col))
		return LEFT_BORDER;

	else if (border == TOPBOT_BORDER && jeSudy(row) && jeLichy(col))
		return RIGHT_BORDER;

	else if (border == LEFT_BORDER && jeLichy(row) && jeSudy(col)) //pokud je puv. hranice leva a radek je lichy, sloupec sudy->
		return TOPBOT_BORDER;										//hranice je dole nebo nahore

	else if (border == LEFT_BORDER && jeLichy(row) && jeLichy(col))
		return RIGHT_BORDER;

	else if (border == LEFT_BORDER && jeSudy(row) && jeSudy(col))
		return RIGHT_BORDER;

	else if (border == LEFT_BORDER && jeSudy(row) && jeLichy(col))
		return TOPBOT_BORDER;

	else if (border == RIGHT_BORDER && jeLichy(row) && jeSudy(col)) //pokud je puv. hranice prava a radek je lichy, sloupec sudy->
		return LEFT_BORDER;											//hranice je vlevo

	else if (border == RIGHT_BORDER && jeLichy(row) && jeLichy(col))
		return TOPBOT_BORDER;

	else if (border == RIGHT_BORDER && jeSudy(row) && jeSudy(col))
		return TOPBOT_BORDER;

	else if (border == RIGHT_BORDER && jeSudy(row) && jeLichy(col))
		return LEFT_BORDER;

	return border;
}



void rpath(Mapa *mapa, int startRow, int startCol)  //hleda cestu podle prave ruky
{
	int row = startRow;
	int col = startCol;

	int startBorder = start_border(mapa, startRow, startCol, RIGHT); //hledam pocatecni hranici, kde budu zacinat hledat cestu
	int border = startBorder;

	while (true)
	{
		printf("%d,%d\n", row + 1, col + 1);

		if (isBorder(mapa, row, col, border)) //mohu projit aktualni hranou
			border = getNextRpathBorder(row, col, border); //nemuzu, hledam dalsi pruchozi
		if (isBorder(mapa, row, col, border))
			border = getNextRpathBorder(row, col, border); //porad nemuzu, vystupuju hranou, kterou jsem vstoupila

		if (border == RIGHT_BORDER) //vystupuji pravou hranou, jdu o sloupec doprava
		{
			col++;
			border = getNextRpathBorder(row, col, LEFT_BORDER); //vstoupila jsem levou hranou, hledam dalsi hranu pro pruchod
		}
		else if (border == LEFT_BORDER) //vystupuji levou hranou, jdu o sloupec doleva
		{
			col--;
			border = getNextRpathBorder(row, col, RIGHT_BORDER); //vstoupila jsem pravou hranou, hledam dalsi hranu pro pruchod
		}
		else if (border == TOPBOT_BORDER && jeLichy(col) && jeLichy(row)) //vystupuji horem, jdu o radek nahoru
		{
			row--;
			border = getNextRpathBorder(row, col, TOPBOT_BORDER); //vstoupila jsem spodem, hledam dalsi hranu pro pruchod
		}
		else if (border == TOPBOT_BORDER && jeLichy(col) && jeSudy(row)) //vystupuji spodem, jdu o radek dolu
		{
			row++;
			border = getNextRpathBorder(row, col, TOPBOT_BORDER); //vstoupila jsem horem, hledam dalsi hranu pro pruchod
		}
		else if (border == TOPBOT_BORDER && jeSudy(col) && jeLichy(row)) //vystupuji horem, jdu o radek nahoru
		{
			row++;
			border = getNextRpathBorder(row, col, TOPBOT_BORDER); //vstoupila jsem horem, hledam dalsi hranu pro pruchod
		}
		else if (border == TOPBOT_BORDER && jeSudy(col) && jeSudy(row)) //vystupuji spodem, jdu o radek dolu
		{
			row--;
			border = getNextRpathBorder(row, col, TOPBOT_BORDER); //vstoupila jsem spodem, hledam dalsi hranu pro pruchod
		}
		if (row < 0 || col < 0 || row >= mapa->radku || col >= mapa->sloupcu) //dostala jsem se ven z bludiste? jo,konec cyklu, ne, opakuju
			break;
	}
}

int getNextLpathBorder(int row, int col, int border)   //hledam dalsi hranici od minuleho policka a pokracuju v hledani cesty
{
	if (border == TOPBOT_BORDER && jeLichy(row) && jeSudy(col)) //pokud je hranice horni/dolni a radek je lichy,sloupec sudy,hranice je vlevo
		return LEFT_BORDER;

	else if (border == TOPBOT_BORDER && jeLichy(row) && jeLichy(col))
		return RIGHT_BORDER;

	else if (border == TOPBOT_BORDER && jeSudy(row) && jeSudy(col))
		return RIGHT_BORDER;

	else if (border == TOPBOT_BORDER && jeSudy(row) && jeLichy(col))
		return LEFT_BORDER;

	else if (border == LEFT_BORDER && jeLichy(row) && jeSudy(col)) //pokud je puv. hranice leva a radek je lichy, sloupec sudy->
		return RIGHT_BORDER;										//hranice je vpravo

	else if (border == LEFT_BORDER && jeLichy(row) && jeLichy(col))
		return TOPBOT_BORDER;

	else if (border == LEFT_BORDER && jeSudy(row) && jeSudy(col))
		return TOPBOT_BORDER;

	else if (border == LEFT_BORDER && jeSudy(row) && jeLichy(col))
		return RIGHT_BORDER;

	else if (border == RIGHT_BORDER && jeLichy(row) && jeSudy(col)) //pokud je puv. hranice prava a radek je lichy, sloupec sudy->
		return TOPBOT_BORDER;										//hranice je nahore/dole

	else if (border == RIGHT_BORDER && jeLichy(row) && jeLichy(col))
		return LEFT_BORDER;

	else if (border == RIGHT_BORDER && jeSudy(row) && jeSudy(col))
		return LEFT_BORDER;

	else if (border == RIGHT_BORDER && jeSudy(row) && jeLichy(col))
		return TOPBOT_BORDER;

	return border;
}

void lpath(Mapa *mapa, int startRow, int startCol)  //hleda cestu podle leve ruky
{
	int row = startRow;
	int col = startCol;

	int startBorder = start_border(mapa, startRow, startCol, LEFT); //hledam pocatecni hranici, kde budu zacinat hledat cestu

	int border = startBorder;

	while (true)
	{
		printf("%d,%d\n", row + 1, col + 1);

		if (isBorder(mapa, row, col, border))
			border = getNextLpathBorder(row, col, border); 

		if (isBorder(mapa, row, col, border))
			border = getNextLpathBorder(row, col, border); 

		if (border == RIGHT_BORDER) //vystupuji pravou hranou, jdu o sloupec doprava
		{
			col++;
			border = getNextLpathBorder(row, col, LEFT_BORDER); //vstoupila jsem levou hranou, hledam dalsi hranu pro pruchod
		}
		else if (border == LEFT_BORDER) //vstupuji levou hranou, jdu o sloupec doleva
		{
			col--;
			border = getNextLpathBorder(row, col, RIGHT_BORDER); 
		}
		else if (border == TOPBOT_BORDER && jeLichy(col) && jeLichy(row)) 
		{
			row--;
			border = getNextLpathBorder(row, col, TOPBOT_BORDER); 
		}
		else if (border == TOPBOT_BORDER && jeLichy(col) && jeSudy(row)) 
		{
			row++;
			border = getNextLpathBorder(row, col, TOPBOT_BORDER); 
		}
		else if (border == TOPBOT_BORDER && jeSudy(col) && jeLichy(row)) 
		{
			row++;
			border = getNextLpathBorder(row, col, TOPBOT_BORDER); 
		}
		else if (border == TOPBOT_BORDER && jeSudy(col) && jeSudy(row))
		{
			row--;
			border = getNextLpathBorder(row, col, TOPBOT_BORDER);
		}
		if (row < 0 || col < 0 || row >= mapa->radku || col >= mapa->sloupcu) //dostala jsem se ven z bludiste? jo,konec cyklu, ne, opakuju
			break;
	}
}

typedef struct pozice //ukladani aktualni pozice trojuhelniku do fronty 
{
	int row; //index radku
	int col; //index sloupce
} Pozice;

typedef struct queue  //fronta trojuhelniku pro BFS algoritmus hledani nejkratsi cesty
{
	Pozice *fronta;           //samotna fronta
	int velikost;             //jeji velikost
	Pozice *predchudce;       //pamatovani odkud jsme prisli na danou pozici
	Pozice *naslednik;        //pamatovani kam mame jit (pole predchudce v opacnem smeru)
	int prvniVeFronte;        //index prvku, ktery ma ted prijit na zpracovani
	int posledniVeFronte;     //index posledniho prvku vlozeneho do fronty
	bool *prvekZpracovanFlag; //zadny prvek nechceme dat do fronty vicekrat
} Queue;

void vlozDoFronty(Queue *q, int row, int col, int colsTotal) //vlozeni prvku na konec fronty 
{
	if (!q->prvekZpracovanFlag[row*colsTotal + col]) //pokud ve fronte jeste nikdy nebyl, vlozime
	{
		q->fronta[q->posledniVeFronte].row = row;
		q->fronta[q->posledniVeFronte].col = col;
		q->posledniVeFronte = (q->posledniVeFronte + 1) % q->velikost;

		q->prvekZpracovanFlag[row*colsTotal + col] = true;
	}
}

Pozice *vyberZFronty(Queue *q) //odebrani prvku z vrcholu fronty 
{
	Pozice *aktualni = &(q->fronta[q->prvniVeFronte]);
	q->prvniVeFronte = (q->prvniVeFronte + 1) % q->velikost;
	return aktualni;
}

/* ulozeni indexu predchudce aktualniho prvku, abychom pak mohli
zrekonstruovat cestu nazpet */
void ulozPredchudce(Queue *q, int predchudceRow, int predchudceCol,
	int aktualniRow, int aktualniCol, int colsTotal)
{
	int index2Dto1D = aktualniRow*colsTotal + aktualniCol;
	if (q->predchudce[index2Dto1D].row < 0 &&  //vkladame pouze pokud aktualni prvek jeste neni mimo bludiste
		q->predchudce[index2Dto1D].col < 0)
	{
		q->predchudce[index2Dto1D].row = predchudceRow;
		q->predchudce[index2Dto1D].col = predchudceCol;
	}
}

int alokujFrontu(Queue *q, int velikost) // alokace dynamicke pameti pro frontu 
{
	q->velikost = velikost;
	q->fronta = calloc(q->velikost, sizeof(Pozice));
	q->predchudce = calloc(q->velikost, sizeof(Pozice));
	q->naslednik = calloc(q->velikost, sizeof(Pozice));
	q->prvekZpracovanFlag = calloc(q->velikost, sizeof(bool));

	if (!q->fronta || !q->predchudce || !q->prvekZpracovanFlag || !q->naslednik)
		return EAlokace;
	return Ok;
}

void dealokujFrontu(Queue *q) //uvolneni pameti pro frontu 
{
	free(q->fronta);
	free(q->predchudce);
	free(q->naslednik);
	free(q->prvekZpracovanFlag);
	q->fronta = 0;
	q->predchudce = 0;
	q->naslednik = 0;
	q->prvekZpracovanFlag = 0;
}

void inicializujFrontu(Queue *q)   //inicializace fronty 
{
	q->prvniVeFronte = 0;
	q->posledniVeFronte = 0;
	for (int i = 0; i < q->velikost; ++i) //-1 znaci, ze nema predchudce
	{
		q->predchudce[i].row = -1;
		q->predchudce[i].col = -1;
	}
}

int jeFrontaPrazdna(Queue *q) //prazdna fronta nam rekne, ze z bludiste neni cesty ven 
{
	return q->prvniVeFronte == q->posledniVeFronte;
}

/* Parametr --shortest, hledani nejkratsi cesty z bludiste
 Indexy row a col mohou byt kdekoli uvnitr bludiste.
 Vyuziva algoritmus breadth first search za pomoci fronty.
 Z daneho aktualniho trojuhelniku se do fronty nahraji vsichni jeho sousede,
 ke kterym se da dostat pres pruchozi stenu, nasledne se aktualni trojuhelnik
 oznaci jako zpracovany, z fronty postupne vybirame jeho sousedy a opet vkladame
 do fronty sousedy techto sousedu atd.
 Jakmile je soused mimo bludiste, algoritmus konci a vypise se cesta pomoci pole
 predchudcu, resp. nasledniku */

int spath(Mapa *mapa, int row, int col)
{
	Queue q;
	int chyba = alokujFrontu(&q, mapa->radku*mapa->sloupcu);
	if (chyba != Ok)
		return chyba;

	inicializujFrontu(&q);

	vlozDoFronty(&q, row, col, mapa->sloupcu); //vlozeni pocatecniho trojuhelniku do fronty

	Pozice *aktualni;  //aktualni zpracovavany trojuhelnik

	while (true) //bfs:
	{
		aktualni = vyberZFronty(&q); //vyber prvek z vrcholu fronty

		/*nasledne do fronty vlozime vsechny jeho sousedy, ke kterym je pruchozi stena
		a ulozime jejich predchudce, coz je nyni aktualni prvek
		pokud je soused mimo bludiste, nasli jsme cestu a ven a BFS konci*/

		if (!isBorder(mapa, aktualni->row, aktualni->col, LEFT_BORDER)) //soused nalevo
		{
			if (aktualni->col > 0)
			{
				vlozDoFronty(&q, aktualni->row, aktualni->col - 1, mapa->sloupcu);
				ulozPredchudce(&q, aktualni->row, aktualni->col, aktualni->row, aktualni->col - 1, mapa->sloupcu);
			}
			else break;
		}

		if (!isBorder(mapa, aktualni->row, aktualni->col, RIGHT_BORDER)) //soused napravo
		{
			if (aktualni->col + 1 < mapa->sloupcu)
			{
				vlozDoFronty(&q, aktualni->row, aktualni->col + 1, mapa->sloupcu);
				ulozPredchudce(&q, aktualni->row, aktualni->col, aktualni->row, aktualni->col + 1, mapa->sloupcu);
			}
			else break;
		}

		if (!isBorder(mapa, aktualni->row, aktualni->col, TOPBOT_BORDER))
		{
			if (jeSudy(aktualni->row + aktualni->col)) //soused nahore
			{ 
				if (aktualni->row > 0)
				{
					vlozDoFronty(&q, aktualni->row - 1, aktualni->col, mapa->sloupcu);
					ulozPredchudce(&q, aktualni->row, aktualni->col, aktualni->row - 1, aktualni->col, mapa->sloupcu);
				}
				else break;
			}
			else
			{
				if (aktualni->row + 1 < mapa->radku) //soused dole
				{
					vlozDoFronty(&q, aktualni->row + 1, aktualni->col, mapa->sloupcu);
					ulozPredchudce(&q, aktualni->row, aktualni->col, aktualni->row + 1, aktualni->col, mapa->sloupcu);
				}
				else break;
			}
		}

		if (jeFrontaPrazdna(&q))  //prazdna fronta indikuje neexistujici cestu ven z bludiste
		{
			dealokujFrontu(&q);
			return EMapa;
		}
	}

	/*nyni cestu vypiseme, ale jelikoz podle pole predchudcu zjistime pouze
	cestu od vychodu z bludiste do pocatecniho prvku, musime predchudce
	preklopit do pole nasledniku, abychom mohli vypisovat od pocatecniho
	k vychodu*/
	int tmpRow, tmpCol;
	q.naslednik[aktualni->row*mapa->sloupcu + aktualni->col].row = -1;
	q.naslednik[aktualni->row*mapa->sloupcu + aktualni->col].col = -1;

	while (aktualni->row != row || aktualni->col != col)   //preklopime predchudce do nasledniku
	{
		tmpRow = aktualni->row;
		tmpCol = aktualni->col;
		aktualni->row = q.predchudce[tmpRow*mapa->sloupcu + tmpCol].row;
		aktualni->col = q.predchudce[tmpRow*mapa->sloupcu + tmpCol].col;
		q.naslednik[aktualni->row*mapa->sloupcu + aktualni->col].row = tmpRow;
		q.naslednik[aktualni->row*mapa->sloupcu + aktualni->col].col = tmpCol;
	}

	while (q.naslednik[aktualni->row*mapa->sloupcu + aktualni->col].row != -1 &&  //vypiseme cestu od pocatecniho bodu k vychodu pomoci nasledniku
		q.naslednik[aktualni->row*mapa->sloupcu + aktualni->col].col != -1)
	{
		printf("%d,%d\n", aktualni->row + 1, aktualni->col + 1);
		tmpRow = aktualni->row;
		tmpCol = aktualni->col;
		aktualni->row = q.naslednik[tmpRow*mapa->sloupcu + tmpCol].row;
		aktualni->col = q.naslednik[tmpRow*mapa->sloupcu + tmpCol].col;

	}

	printf("%d,%d\n", aktualni->row + 1, aktualni->col + 1);   //vychod dotiskneme extra, protoze nema naslednika

	dealokujFrontu(&q); //uvolneni pameti

	return Ok;
}

int main(int argc, char **argv)
{
	Parametry parametry;
	Mapa mapa;
	int chyba;

	chyba = nactiParametry(argc, argv, &parametry);

	if (chyba != Ok)
		return vypisChybu(chyba);

	if (parametry.Operace == Help) //pro par. help vytiskne napovedu
	{
		vypisNapovedu();
		return EXIT_SUCCESS;
	}

	chyba = vytvorMapu(&mapa, &parametry);  //dem kontrolovat validitu, pokud se nerovna vysledek 0, chyba
	if (chyba != Ok)
	{
		if (chyba == EMapa && parametry.Operace == Test) //pro par. --test vypise invaliditu pokud je chyba nekde v mape
		{
			printf("Invalid\n");
			free(mapa.bunky);
			return chyba;
		}
		else
			return vypisChybu(chyba);
	}

	switch (parametry.Operace)
	{
	case Test:			//pokud byl zvolen parametr test, vypise validni|| nevalidni
		printf("Valid\n");
		break;

	case Lpath: //hledani cesty pravidlem leve ruky
		lpath(&mapa, parametry.startRadekIdx, parametry.startSloupecIdx);
		break;

	case Rpath: //hledani cesty pravidlem prave ruky
		rpath(&mapa, parametry.startRadekIdx, parametry.startSloupecIdx);
		break;

	case Spath: //hledani nejrkatsi cesty --shortest
		chyba = spath(&mapa, parametry.startRadekIdx, parametry.startSloupecIdx);
		break;

	default:
		chyba = EUnknown;
	}

	free(mapa.bunky); //uvolneni pameti
	if (chyba != Ok)
		return vypisChybu(chyba);

	return EXIT_SUCCESS;
}

