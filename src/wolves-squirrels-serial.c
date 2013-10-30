/*
 ============================================================================
 Name        : wolves-squirrels-serial.c
 Author      : Group-40 (Gayana, Sri and Stefan)
 Version     : 1.0
 Copyright   : CPD
 Description : OpenMP serial World in C
 ============================================================================
 */
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define PRINT_NORMAL
//#define PRINT_CORDINATE
//#define PRINT_DEATIL
///#define PRINT_DEBUG

#define WOLF "w"
#define SQUIRREL "s"
#define TREE "t"
#define ICE "i"

#define MAX(a,b) (((a)>(b))?(a):(b))
int g_iNumberofGen;
int g_GridSize;

int l_iWolfBP;
int l_iSquirrelBP;
int l_iWolfSP;
typedef struct world {
	int type;
	int breeding_period;
	int starvation_period;
	bool isUpdated;
} world;

world **pt_SourceWorld;
world **pt_DestinationWorld;
FILE *gf_DumpFile;
void fnDumpTheOutput(int *iGen, world **ptPrintWorld) {
	gf_DumpFile = fopen("World.txt", "a+");
	if (gf_DumpFile == NULL) {
		printf("Unable to open the file.\n");
		exit(1);
	}

	int row;
	int columns;
	world *l_ptWorld;

	for (row = 0; row < g_GridSize; row++) {

		for (columns = 0; columns < g_GridSize; columns++) {
			l_ptWorld = *(ptPrintWorld + row) + columns;
			switch (l_ptWorld->type) {
			case 1: {
				fprintf(gf_DumpFile, "g%d - %d %d w \n", *iGen, row, columns);

			}
				break;
			case 2: {
				fprintf(gf_DumpFile, "g%d - %d %d s \n", *iGen, row, columns);
			}
				break;
			case 3: {
				fprintf(gf_DumpFile, "g%d - %d %d t \n", *iGen, row, columns);
			}
				break;
			case 4: {
				fprintf(gf_DumpFile, "g%d - %d %d x\n", *iGen, row, columns);
			}
				break;
			case 5: {
				fprintf(gf_DumpFile, "g%d - %d %d $ \n", *iGen, row, columns);
			}
				break;

			}
		}

	}

	fclose(gf_DumpFile);

}
void fnPrintWorld(world **ptPrintWorld) {
	int row;
	int columns;
	world *l_ptWorld;
#ifdef PRINT_DEATIL
	for (row = 0; row < g_GridSize; row++) {

		printf("%d", row);
		for (columns = 0; columns < g_GridSize; columns++) {
			l_ptWorld = *(ptPrintWorld + row) + columns;
			switch (l_ptWorld->type) {
				case 1: {
					printf("(%d,%d,w)<%d,%d>", l_ptWorld->breeding_period,
							l_ptWorld->starvation_period, row, columns);
				}
				break;
				case 2: {
					printf("(%d,%d,s)<%d,%d>", l_ptWorld->breeding_period,
							l_ptWorld->starvation_period, row, columns);
				}
				break;
				case 3: {
					printf("(%d,%d,t)<%d,%d>", l_ptWorld->breeding_period,
							l_ptWorld->starvation_period, row, columns);
				}
				break;
				case 4: {
					printf("(%d,%d,x)<%d,%d>", l_ptWorld->breeding_period,
							l_ptWorld->starvation_period, row, columns);
				}
				break;
				case 5: {
					printf("(%d,%d,$)<%d,%d>", l_ptWorld->breeding_period,
							l_ptWorld->starvation_period, row, columns);
				}
				break;
				default: {
					printf("   ----     ");
				}

			}
		}
		printf("\n");
	}
#endif
#ifdef PRINT_CORDINATE
	for (row = 0; row < g_GridSize; row++) {

		for (columns = 0; columns < g_GridSize; columns++) {
			l_ptWorld = *(ptPrintWorld + row) + columns;
			switch (l_ptWorld->type) {
			case 1: {
				printf("(%d,%d) - w \n", row, columns);
			}
				break;
			case 2: {
				printf("(%d,%d) - s \n", row, columns);
			}
				break;
			case 3: {
				printf("(%d,%d) - t \n", row, columns);
			}
				break;
			case 4: {
				printf("(%d,%d) - i \n", row, columns);
			}
				break;
			case 5: {
				printf("(%d,%d) -$i \n", row, columns);
			}
				break;

			}
		}

	}
#endif
#ifdef PRINT_NORMAL
	for (row = 0; row < g_GridSize; row++) {
		for (columns = 0; columns < g_GridSize; columns++) {
			l_ptWorld = *(ptPrintWorld + row) + columns;
			switch (l_ptWorld->type) {
				case 1: {
					printf(" <w> ");
				}
				break;
				case 2: {
					printf(" <s> ");
				}
				break;
				case 3: {
					printf(" <t> ");
				}
				break;
				case 4: {
					printf(" <x> ");
				}
				break;
				case 5: {
					printf(" <$> ");
				}
				break;
				default: {
					printf(" ___ ");
				}

			}
		}
		printf("\n");
	}

#endif
}

bool fnPossibleMovementToSameCell(int *i_Row, int *i_Column, int *_iActualRow,
		int *_iActualColumn, world **pt_Whichworld) {

	int l_iNumberofPossiblemovement = 0;
	bool up = false, right = false, down = false;
	int l_iupval = -1;
	int l_rightval = -1;
	int l_downval = -1;
	int l_leftval = -1;
	int iCountSquirrel = 0;
	bool bupmove = false;
	bool brightmove = false;
	bool bdownmove = false;
	bool bleftmove = false;
	if ((*i_Row >= 0 && *i_Row <= g_GridSize - 1)
			&& (*i_Column >= 0 && *i_Column <= g_GridSize - 1)) {

		world *l_ptWorld = *(pt_Whichworld + *i_Row) + *i_Column;
		int *ptTempRow = i_Row;
		int *ptTempColumn = i_Column;

		if (l_ptWorld->type == 0 || l_ptWorld->type == 4) // if next cell is return false, nothing is gonna happen
				{
			return false;
		}
		if (l_ptWorld->type == 1) // if it is wolf , process is different, because we have to consider only more than one squirrel
				{
			if (*ptTempRow - 1 >= 0) {
				if ((*(pt_Whichworld + *ptTempRow - 1) + *ptTempColumn)->type
						== 2) {
					++iCountSquirrel;
					bupmove = true;
				}

			}

			if (*ptTempColumn + 1 <= g_GridSize - 1) {
				if ((*(pt_Whichworld + *i_Row) + *ptTempColumn + 1)->type
						== 2) {
					++iCountSquirrel;
					brightmove = true;
				}

			}

			if (*ptTempRow + 1 <= g_GridSize - 1) {
				if ((*(pt_Whichworld + *ptTempRow + 1) + *i_Column)->type
						== 2) {
					++iCountSquirrel;
					bdownmove = true;
				}

			}

			if (*ptTempColumn - 1 >= 0) {
				if ((*(pt_Whichworld + *i_Row) + *ptTempColumn - 1)->type
						== 2) {
					++iCountSquirrel;
					bleftmove = true;
				}

			}

			if (iCountSquirrel == 1) {
				if (bupmove) {
					if (*_iActualRow == *ptTempRow - 1
							&& *_iActualColumn == *i_Column) {

						return true;
					} else
						return false;
				} else if (brightmove) {
					if (*_iActualRow == *i_Row
							&& *_iActualColumn == *ptTempColumn + 1) {

						return true;
					} else
						return false;
				} else if (bdownmove) {
					if (*_iActualRow == *ptTempRow + 1
							&& *_iActualColumn == *i_Column) {

						return true;
					} else
						return false;
				} else if (bleftmove) {
					if (*_iActualRow == *i_Row
							&& *_iActualColumn == *ptTempColumn - 1) {
						return true;
					} else
						return false;

				}
			}
			world *ptTemp;

			if (*ptTempRow - 1 >= 0) {

				ptTemp = *(pt_Whichworld + *ptTempRow - 1) + *ptTempColumn;
				if ((ptTemp->type == 2 || ptTemp->type == 0)) {
					++l_iNumberofPossiblemovement;
					up = true;
					l_iupval = 0;

				}
			}

			if (*ptTempColumn + 1 <= g_GridSize - 1) {
				ptTemp = *(pt_Whichworld + *ptTempRow) + *ptTempColumn + 1;
				if ((ptTemp->type == 2 || ptTemp->type == 0)) {
					++l_iNumberofPossiblemovement;

					right = true;
					if (up)
						l_rightval = 1;
					else
						l_rightval = 0;

				}
			}

			if (*ptTempRow + 1 <= g_GridSize - 1) {

				ptTemp = *(pt_Whichworld + *ptTempRow + 1) + *ptTempColumn;
				if ((ptTemp->type == 2 || ptTemp->type == 0)) {
					++l_iNumberofPossiblemovement;
					down = true;
					if (up && right) {
						l_downval = 2;

					} else if ((up && !right) || (!up && right)) {
						l_downval = 1;
					} else if (!up && !right) {
						l_downval = 0;
					}

				}
			}

			if (*ptTempColumn - 1 >= 0) {

				ptTemp = *(pt_Whichworld + *ptTempRow) + *ptTempColumn - 1;
				if ((ptTemp->type == 2 || ptTemp->type == 0)) {
					++l_iNumberofPossiblemovement;

				}
				if (up && right && down) {
					l_leftval = 3;

				} else if ((up && !right && down) || (!up && right && down)
						|| (up && right && !down)) {
					l_leftval = 2;
				} else if ((!up && !right && down) || (!up && right && !down)
						|| (up && !right && !down)) {
					l_leftval = 1;
				} else if ((!up && !right && !down)) {
					l_leftval = 0;
				}
			}
			ptTempColumn = i_Column;
		} else if (l_ptWorld->type == 2 || l_ptWorld->type == 5) // if it is squ , process is different
				{
			world * ptTemp;

			if (*ptTempRow - 1 >= 0) {

				ptTemp = *(pt_Whichworld + *ptTempRow - 1) + *ptTempColumn;
				if ((ptTemp->type == 0 || ptTemp->type == 3)) {

					++l_iNumberofPossiblemovement;
					up = true;
					l_iupval = 0;

				}

			}

			if (*ptTempColumn + 1 <= g_GridSize - 1) {

				ptTemp = *(pt_Whichworld + *ptTempRow) + *ptTempColumn + 1;
				if ((ptTemp->type == 0 || ptTemp->type == 3)) {

					++l_iNumberofPossiblemovement;
					right = true;
					if (up)
						l_rightval = 1;
					else
						l_rightval = 0;

				}
			}

			if (*ptTempRow + 1 <= g_GridSize - 1) {

				ptTemp = *(pt_Whichworld + *ptTempRow + 1) + *ptTempColumn;
				if ((ptTemp->type == 0 || ptTemp->type == 3)) {

					++l_iNumberofPossiblemovement;
					down = true;
					if (up && right) {
						l_downval = 2;
					} else if ((up && !right) || (!up && right)) {
						l_downval = 1;
					} else if (!up && !right) {
						l_downval = 0;
					}

				}
			}

			if (*ptTempColumn - 1 >= 0) {

				ptTemp = *(pt_Whichworld + *ptTempRow) + *ptTempColumn - 1;
				if ((ptTemp->type == 0 || ptTemp->type == 3)) {

					++l_iNumberofPossiblemovement;

					if (up && right && down) {
						l_leftval = 3;

					} else if ((up && !right && down) || (!up && right && down)
							|| (up && right && !down)) {
						l_leftval = 2;
					} else if ((!up && !right && down)
							|| (!up && right && !down)
							|| (up && !right && !down)) {
						l_leftval = 1;
					} else if ((!up && !right && !down)) {
						l_leftval = 0;
					}

				}
			}

		}

		if (l_iNumberofPossiblemovement != 0) {
			int l_iCValue = *i_Row * g_GridSize + *i_Column;
			int l_imodval = l_iCValue % l_iNumberofPossiblemovement;
#ifdef DEBUG
			printf(" possible movements : %d-- mod value- %d -row %d|col =%d\n",
					l_iNumberofPossiblemovement, l_imodval, *i_Row, *i_Column);
#endif
			if (l_iupval == l_imodval) {

				if (*_iActualRow == *ptTempRow - 1
						&& *_iActualColumn == *i_Column) {
					return true;
				} else
					return false;
			}

			if (l_rightval == l_imodval) {
				if (*_iActualRow == *i_Row
						&& *_iActualColumn == *ptTempColumn + 1) {
					return true;
				} else
					return false;

			}
			if (l_downval == l_imodval) {
				if (*_iActualRow == *ptTempRow + 1
						&& *_iActualColumn == *i_Column) {
					return true;
				} else
					return false;
			}
			if (l_leftval == l_imodval) {
				if (*_iActualRow == *i_Row
						&& *_iActualColumn == *ptTempColumn - 1) {
					return true;
				} else
					return false;
			}
			return false;
		} else {
			return false;
		}
	} else
		return false;

}

void fnupdateCell(int *_iBreedingPeriod, int *_iStarvationPeriod, int *_iType,
		int *i_Row, int *i_Column, int *_iUpdateRow, int *_iUpdateColumn,
		world **pt_srcWorld, world **pt_DstWorld) {
	world *l_ptSrcWorld = *(pt_srcWorld + *i_Row) + *i_Column;
	world *l_ptDstWorldCell = *(pt_DstWorld + *_iUpdateRow) + *_iUpdateColumn;
	if (*_iType == 1) {
		bool _isBreed = false;
		if (l_ptSrcWorld->breeding_period == 0) {
			(*(pt_DstWorld + *i_Row) + *i_Column)->type = *_iType;
			(*(pt_DstWorld + *i_Row) + *i_Column)->breeding_period = l_iWolfBP;
			(*(pt_DstWorld + *i_Row) + *i_Column)->starvation_period =
					l_iWolfSP;
			(*(pt_DstWorld + *i_Row) + *i_Column)->isUpdated = true;
			_isBreed = true;

		}
		(*(pt_DstWorld + *i_Row) + *i_Column)->isUpdated = true;
		l_ptDstWorldCell->isUpdated = true;
		l_ptDstWorldCell->type = 1;
		l_ptDstWorldCell->starvation_period = *_iStarvationPeriod;
		if (_isBreed)
			l_ptDstWorldCell->breeding_period = l_iWolfBP;
		else
			l_ptDstWorldCell->breeding_period = *_iBreedingPeriod;

	}
	if (*_iType == 2) {
		bool _isBreed = false;
		if (l_ptSrcWorld->breeding_period == 0) {

			(*(pt_DstWorld + *i_Row) + *i_Column)->type = *_iType;
			(*(pt_DstWorld + *i_Row) + *i_Column)->breeding_period =
					l_iSquirrelBP;
			(*(pt_DstWorld + *i_Row) + *i_Column)->starvation_period = 0;
			(*(pt_DstWorld + *i_Row) + *i_Column)->isUpdated = true;
			_isBreed = true;

		}
		(*(pt_DstWorld + *i_Row) + *i_Column)->isUpdated = true;
		l_ptDstWorldCell->isUpdated = true;
		l_ptDstWorldCell->type = 2;
		l_ptDstWorldCell->starvation_period = *_iStarvationPeriod;
		if (_isBreed)
			l_ptDstWorldCell->breeding_period = l_iSquirrelBP;
		else
			l_ptDstWorldCell->breeding_period = *_iBreedingPeriod;
	}
	if (*_iType == 5) {
		bool _isBreed = false;
		if (l_ptSrcWorld->breeding_period == 0) {
			(*(pt_DstWorld + *i_Row) + *i_Column)->type = *_iType;
			(*(pt_DstWorld + *i_Row) + *i_Column)->breeding_period =
					l_iSquirrelBP;
			(*(pt_DstWorld + *i_Row) + *i_Column)->starvation_period = 0;
			(*(pt_DstWorld + *i_Row) + *i_Column)->isUpdated = true;

			_isBreed = true;
		} else {
			(*(pt_DstWorld + *i_Row) + *i_Column)->type = 3;
		}
		(*(pt_DstWorld + *i_Row) + *i_Column)->isUpdated = true;
		l_ptDstWorldCell->isUpdated = true;
		l_ptDstWorldCell->type = 2;
		l_ptDstWorldCell->starvation_period = *_iStarvationPeriod;
		if (_isBreed)
			l_ptDstWorldCell->breeding_period = l_iSquirrelBP;
		else
			l_ptDstWorldCell->breeding_period = *_iBreedingPeriod;
	}

}
void fnUpdateCell(int *i_Row, int *i_Column, world **ptSource, world **ptDst) {

	world *l_ptWorld = *(ptSource + *i_Row) + *i_Column;
	bool isConflict = false;
	int l_nextCell = 0;
	if (l_ptWorld->type == 0)
		isConflict = true;
	/*-----------------------------------------------------------------------------------------------------*/
	if (isConflict) {

		int l_iWolfType = 1;
		int l_SquirrelType = 0;
		bool l_bUperWolfchance = false;
		bool l_bRightWolfchance = false;
		bool l_bDownWolfchance = false;
		bool l_bLeftWolfchance = false;

		bool l_bUperSqchance = false;
		bool l_bRightSqchance = false;
		bool l_bDownSqchance = false;
		bool l_bLeftSqchance = false;

		int l_iUpStarvationPeriod = 0;
		int l_iRightStarvationPeriod = 0;
		int l_iDownStartvationPeriod = 0;
		int l_iLeftStarvationPeriod = 0;

		int l_iTotalStarvationPeriod = 0;

		int l_iUpBreadingPeriod = 0;
		int l_iRightBreadingPeriod = 0;
		int l_iDownBreadingPeriod = 0;
		int l_iLeftBreadingPeriod = 0;

		int l_iUpBreadingPeriodS = 0;
		int l_iRightBreadingPeriodS = 0;
		int l_iDownBreadingPeriodS = 0;
		int l_iLeftBreadingPeriodS = 0;
		int l_iSqCounter = 0;
		int l_iwolfCounter = 0;
		int l_iCounter = 0;

		int *ptTempRow = i_Row;
		int *ptTempColumn = i_Column;
		world *ptTemp;
#ifdef DEBUG
		printf("Processing conflict cell : row - %d | column = %d \n", *i_Row,
				*i_Column);
#endif
		l_nextCell = *i_Column + 1;
		if (fnPossibleMovementToSameCell(i_Row, &l_nextCell, i_Row, i_Column,
				ptSource)) {
#ifdef DEBUG
			printf("Processing i_Column+1 section \n");
#endif
			ptTemp = *(ptSource + *ptTempRow) + *ptTempColumn + 1;
			if (ptTemp->type == 1) {
				l_iRightStarvationPeriod = ptTemp->starvation_period;
				l_iRightBreadingPeriod = ptTemp->breeding_period;
				l_bRightWolfchance = true;
				l_iTotalStarvationPeriod += l_iRightStarvationPeriod;
				++l_iwolfCounter;
			}
			if (ptTemp->type == 2 || ptTemp->type == 5) {
				l_iRightBreadingPeriodS = ptTemp->breeding_period;
				l_bRightSqchance = true;
				++l_iSqCounter;

			}
			++l_iCounter;

		}
		l_nextCell = *i_Column - 1;
		if (fnPossibleMovementToSameCell(i_Row, &l_nextCell, i_Row, i_Column,
				ptSource)) {
#ifdef DEBUG
			printf("Processing Column -1 section \n");
#endif
			ptTemp = *(ptSource + *ptTempRow) + *ptTempColumn - 1;
			if (ptTemp->type == 1) {
				l_iLeftStarvationPeriod = ptTemp->starvation_period;
				l_iLeftBreadingPeriod = ptTemp->breeding_period;
				l_bLeftWolfchance = true;
				++l_iwolfCounter;
				l_iTotalStarvationPeriod += l_iLeftStarvationPeriod;
			}
			if (ptTemp->type == 2 || ptTemp->type == 5) {
				l_iLeftBreadingPeriodS = ptTemp->breeding_period;
				l_bLeftSqchance = true;
				++l_iSqCounter;

			}
			++l_iCounter;

		}
		l_nextCell = *i_Row + 1;
		if (fnPossibleMovementToSameCell(&l_nextCell, i_Column, i_Row, i_Column,
				ptSource)) {
#ifdef DEBUG
			printf("Processing i_Row+1 section \n");
#endif
			ptTemp = *(ptSource + *ptTempRow + 1) + *ptTempColumn;
			if (ptTemp->type == 1) {
				l_iDownStartvationPeriod = ptTemp->starvation_period;
				l_iDownBreadingPeriod = ptTemp->breeding_period;
				l_bDownWolfchance = true;
				++l_iwolfCounter;
				l_iTotalStarvationPeriod += l_iDownStartvationPeriod;
			}
			if (ptTemp->type == 2 || ptTemp->type == 5) {
				l_iDownBreadingPeriodS = ptTemp->breeding_period;
				l_bDownSqchance = true;
				++l_iSqCounter;

			}
			++l_iCounter;

		}
		l_nextCell = *i_Row - 1;
		if (fnPossibleMovementToSameCell(&l_nextCell, i_Column, i_Row, i_Column,
				ptSource)) {
#ifdef DEBUG
			printf("Processing i_Row-1 section \n");
#endif
			ptTemp = *(ptSource + *ptTempRow - 1) + *ptTempColumn;
			if (ptTemp->type == 1) {
				l_iUpStarvationPeriod = ptTemp->starvation_period;
				l_iUpBreadingPeriod = ptTemp->breeding_period;
				l_bUperWolfchance = true;
				++l_iwolfCounter;
				l_iTotalStarvationPeriod += l_iUpStarvationPeriod;
			}
			if (ptTemp->type == 2 || ptTemp->type == 5) {
				l_iUpBreadingPeriodS = ptTemp->breeding_period;
				l_bUperSqchance = true;
				++l_iSqCounter;

			}
			++l_iCounter;

		}

		if (l_iCounter == 1) //one of the element is going to come
				{
			// lets take wolf
			if (l_bUperWolfchance) {
				l_nextCell = *i_Row - 1;
				fnupdateCell(&l_iUpBreadingPeriod, &l_iUpStarvationPeriod,
						&l_iWolfType, &l_nextCell, i_Column, i_Row, i_Column,
						ptSource, ptDst);
			}
			if (l_bRightWolfchance) {
				l_nextCell = *i_Column + 1;
				fnupdateCell(&l_iRightBreadingPeriod, &l_iRightStarvationPeriod,
						&l_iWolfType, i_Row, &l_nextCell, i_Row, i_Column,
						ptSource, ptDst);
			}
			if (l_bDownWolfchance) {
				l_nextCell = *i_Row + 1;
				fnupdateCell(&l_iDownBreadingPeriod, &l_iDownStartvationPeriod,
						&l_iWolfType, &l_nextCell, i_Column, i_Row, i_Column,
						ptSource, ptDst);
			}
			if (l_bLeftWolfchance) {
				l_nextCell = *i_Column - 1;
				fnupdateCell(&l_iLeftBreadingPeriod, &l_iLeftStarvationPeriod,
						&l_iWolfType, i_Row, &l_nextCell, i_Row, i_Column,
						ptSource, ptDst);
			}
			// lets update squirrel
			if (l_bUperSqchance) {
				l_nextCell = *i_Row - 1;
				ptTemp = *(pt_SourceWorld + *ptTempRow - 1) + *ptTempColumn;
				fnupdateCell(&l_iUpBreadingPeriodS, &l_SquirrelType,
						&ptTemp->type, &l_nextCell, i_Column, i_Row, i_Column,
						ptSource, ptDst);
			}
			ptTempRow = i_Row;
			if (l_bRightSqchance) {

				l_nextCell = *i_Column + 1;
				ptTemp = *(pt_SourceWorld + *ptTempRow) + *ptTempColumn + 1;
				fnupdateCell(&l_iRightBreadingPeriodS, &l_SquirrelType,
						&ptTemp->type, i_Row, &l_nextCell, i_Row, i_Column,
						ptSource, ptDst);
			}

			if (l_bDownSqchance) {
				l_nextCell = *i_Row + 1;
				ptTemp = *(pt_SourceWorld + *ptTempRow + 1) + *ptTempColumn;
				fnupdateCell(&l_iDownBreadingPeriodS, &l_SquirrelType,
						&ptTemp->type, &l_nextCell, i_Column, i_Row, i_Column,
						ptSource, ptDst);
			}

			if (l_bLeftSqchance) {
				l_nextCell = *i_Column - 1;
				ptTemp = *(pt_SourceWorld + *ptTempRow) + *ptTempColumn - 1;
				fnupdateCell(&l_iLeftBreadingPeriodS, &l_SquirrelType,
						&ptTemp->type, i_Row, &l_nextCell, i_Row, i_Column,
						ptSource, ptDst);
			}
			ptTempColumn = i_Column;

		}

		if (l_iCounter > 1) {
			if (l_iwolfCounter > 0
					&& ((l_iSqCounter > 0) || l_iSqCounter == 0)) {
				// there are only 2 or more wolf
				bool bIsStarvationEqual = false;
				int l_iMaxBreading = 0;
				int l_interMax1 =
				MAX(l_iLeftStarvationPeriod,l_iRightStarvationPeriod);
				int l_interMax2 =
				MAX(l_iDownStartvationPeriod,l_iUpStarvationPeriod);
				int l_iMaxStarvation = MAX(l_interMax1,l_interMax2);
				if (l_iMaxStarvation
						== (l_iTotalStarvationPeriod / l_iwolfCounter)) //starvation is equal for every wolf
						{
					bIsStarvationEqual = true;
					//lets calculate the maximum breading
					int l_interMaxBread1 =
					MAX(l_iLeftBreadingPeriod,l_iRightBreadingPeriod);
					int l_interMaxBread2 =
					MAX(l_iDownBreadingPeriod,l_iUpBreadingPeriod);
					l_iMaxBreading = MAX(l_interMaxBread1,l_interMaxBread2);
				}

				ptDst[*i_Row][*i_Column].type = 1;
				ptDst[*i_Row][*i_Column].isUpdated = true;
				if (l_iSqCounter > 0) // so there are squirrel , so we need to set final wolf's starvation period as initial starvation period
						{
					ptDst[*i_Row][*i_Column].starvation_period = l_iWolfSP;
				} else
					ptDst[*i_Row][*i_Column].starvation_period =
							l_iMaxStarvation;
				if (bIsStarvationEqual) {

					ptDst[*i_Row][*i_Column].breeding_period = l_iMaxBreading;
				} else {
					if (l_iMaxStarvation == l_iLeftStarvationPeriod) {
						ptDst[*i_Row][*i_Column].breeding_period =
								ptSource[*i_Row][*i_Column - 1].breeding_period;
					} else if (l_iMaxStarvation == l_iRightStarvationPeriod) {
						ptDst[*i_Row][*i_Column].breeding_period =
								ptSource[*i_Row][*i_Column + 1].breeding_period;

					} else if (l_iMaxStarvation == l_iDownStartvationPeriod) {
						ptDst[*i_Row][*i_Column].breeding_period =
								ptSource[*i_Row + 1][*i_Column].breeding_period;

					} else {
						ptDst[*i_Row][*i_Column].breeding_period =
								ptSource[*i_Row - 1][*i_Column].breeding_period;
					}

				}
				world *pt_TempWorld;
				world *pt_TempDstWorld;
				if (l_bUperWolfchance) {
					pt_TempWorld = *(ptSource + *i_Row - 1) + *i_Column;
					pt_TempDstWorld = *(ptDst + *i_Row - 1) + *i_Column;

					if (pt_TempWorld->breeding_period == 0) {

						pt_TempDstWorld->type = 1;
						pt_TempDstWorld->starvation_period = l_iWolfSP;
						pt_TempDstWorld->breeding_period = l_iWolfBP;
						pt_TempDstWorld->isUpdated = true;
					}
					pt_TempDstWorld->isUpdated = true;
				}
				if (l_bRightWolfchance) {
					pt_TempWorld = *(pt_SourceWorld + *i_Row) + *i_Column + 1;
					pt_TempDstWorld = *(ptDst + *i_Row) + *i_Column + 1;
					if (pt_TempWorld->breeding_period == 0) {

						pt_TempDstWorld->type = 1;
						pt_TempDstWorld->starvation_period = l_iWolfSP;
						pt_TempDstWorld->breeding_period = l_iWolfBP;
						pt_TempDstWorld->isUpdated = true;
					}
					pt_TempDstWorld->isUpdated = true;
				}

				if (l_bDownWolfchance) {
					pt_TempWorld = *(pt_SourceWorld + *i_Row + 1) + *i_Column;
					pt_TempDstWorld = *(ptDst + *i_Row + 1) + *i_Column;
					if (pt_TempWorld->breeding_period == 0) {

						pt_TempDstWorld->type = 1;
						pt_TempDstWorld->starvation_period = l_iWolfSP;
						pt_TempDstWorld->breeding_period = l_iWolfBP;
						pt_TempDstWorld->isUpdated = true;
					}
					pt_TempDstWorld->isUpdated = true;
				}

				if (l_bLeftWolfchance) {
					pt_TempWorld = *(pt_SourceWorld + *i_Row) + *i_Column - 1;
					pt_TempDstWorld = *(ptDst + *i_Row) + *i_Column - 1;
					if (pt_TempWorld->breeding_period == 0) {

						pt_TempDstWorld->type = 1;
						pt_TempDstWorld->starvation_period = l_iWolfSP;
						pt_TempDstWorld->breeding_period = l_iWolfBP;
						pt_TempDstWorld->isUpdated = true;
					}
					pt_TempDstWorld->isUpdated = true;
				}

			} else if ((l_iwolfCounter == 0) && l_iSqCounter > 0) {
				// there are 2 or more squirrel

				int l_iMaxBreading;

				//lets calculate the maximum breading

				int l_interMaxBread1 =
				MAX(l_iLeftBreadingPeriodS,l_iRightBreadingPeriodS);
				int l_interMaxBread2 =
				MAX(l_iDownBreadingPeriodS,l_iUpBreadingPeriodS);
				l_iMaxBreading = MAX(l_interMaxBread1,l_interMaxBread2);

				/*pt_SourceWord[*i_Row][*i_Column].type = 2;
				 pt_SourceWord[*i_Row][*i_Column].starvation_period = 0;

				 pt_SourceWord[*i_Row][*i_Column].breeding_period =
				 l_iMaxBreading;*/

				if (l_bUperSqchance) {

					/*pt_SourceWorld[*i_Row][*i_Column].type = 2;
					 pt_SourceWorld[*i_Row][*i_Column].starvation_period = 0;

					 pt_SourceWorld[*i_Row][*i_Column].breeding_period =
					 l_iMaxBreading;*/
					l_nextCell = *i_Row - 1;
					fnupdateCell(&l_iMaxBreading, &l_SquirrelType,
							&pt_SourceWorld[*i_Row - 1][*i_Column].type,
							&l_nextCell, i_Column, i_Row, i_Column, ptSource,
							ptDst);

				}
				if (l_bRightSqchance) {

					l_nextCell = *i_Column + 1;
					fnupdateCell(&l_iMaxBreading, &l_SquirrelType,
							&pt_SourceWorld[*i_Row][*i_Column + 1].type, i_Row,
							&l_nextCell, i_Row, i_Column, ptSource, ptDst);
				}

				if (l_bDownSqchance) {

					l_nextCell = *i_Row + 1;
					fnupdateCell(&l_iMaxBreading, &l_SquirrelType,
							&pt_SourceWorld[*i_Row + 1][*i_Column].type,
							&l_nextCell, i_Column, i_Row, i_Column, ptSource,
							ptDst);
				}

				if (l_bLeftSqchance) {

					l_nextCell = *i_Column - 1;
					fnupdateCell(&l_iMaxBreading, &l_SquirrelType,
							&pt_SourceWorld[*i_Row][*i_Column - 1].type, i_Row,
							&l_nextCell, i_Row, i_Column, ptSource, ptDst);
				}

			}

		}

	} else if (ptSource[*i_Row][*i_Column].type == 3) { // if that cell has a tree    /*there is no conflict , and check other movements*/

#ifdef DEBUG
		printf("Processing tree  cell : row - %d | column = %d \n", *i_Row,
				*i_Column);
#endif
		bool l_bUperSqchance = false;
		bool l_bRightSqchance = false;
		bool l_bDownSqchance = false;
		bool l_bLeftSqchance = false;

		int l_iUpBreadingPeriod = 0;
		int l_iRightBreadingPeriod = 0;
		int l_iDownBreadingPeriod = 0;
		int l_iLeftBreadingPeriod = 0;
		int l_iCounter = 0;

		int l_nextCell = 0;
		l_nextCell = *i_Column - 1;
		if ((fnPossibleMovementToSameCell(i_Row, &l_nextCell, i_Row, i_Column,
				ptSource))) {
			l_bLeftSqchance = true;
			l_iLeftBreadingPeriod =
					ptSource[*i_Row][*i_Column - 1].breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Column + 1;
		if ((fnPossibleMovementToSameCell(i_Row, &l_nextCell, i_Row, i_Column,
				ptSource))) {
			l_bRightSqchance = true;

			l_iRightBreadingPeriod =
					ptSource[*i_Row][*i_Column + 1].breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Row + 1;
		if ((fnPossibleMovementToSameCell(&l_nextCell, i_Column, i_Row,
				i_Column, ptSource))) {
			l_bDownSqchance = true;
			l_iDownBreadingPeriod =
					ptSource[*i_Row + 1][*i_Column].breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Row - 1;
		if ((fnPossibleMovementToSameCell(&l_nextCell, i_Column, i_Row,
				i_Column, ptSource))) {
			l_bUperSqchance = true;
			l_iUpBreadingPeriod =
					ptSource[*i_Row - 1][*i_Column].breeding_period;
			++l_iCounter;
		}
		if (l_iCounter >= 1) {
			//lets calculate the maxl_ptWorldimum breading
			if (l_iCounter > 1) {
				int l_interMaxBread1 =
				MAX(l_iLeftBreadingPeriod,l_iRightBreadingPeriod);
				int l_interMaxBread2 =
				MAX(l_iDownBreadingPeriod,l_iUpBreadingPeriod);
				int l_iMaxBreading = MAX(l_interMaxBread1,l_interMaxBread2);
				//update the cell with these parameters
				ptDst[*i_Row][*i_Column].type = 5;
				ptDst[*i_Row][*i_Column].starvation_period = 0;
				ptDst[*i_Row][*i_Column].breeding_period = l_iMaxBreading;
				ptDst[*i_Row][*i_Column].isUpdated = true;
			}
			if (l_bUperSqchance) {
				world *ptTempWorld = *(ptSource + *i_Row - 1) + *i_Column;
				world *ptDestWorld = *(ptDst + *i_Row - 1) + *i_Column;
				if (ptTempWorld->breeding_period == 0) {
					if (ptTempWorld->type == 5)
						ptDestWorld->type = 5;
					else
						ptDestWorld->type = 2;
					ptDestWorld->starvation_period = 0;
					ptDestWorld->breeding_period = l_iSquirrelBP;
					ptDestWorld->isUpdated = true;
				} else {
					ptDst[*i_Row][*i_Column].type = 5;
					ptDst[*i_Row][*i_Column].starvation_period = 0;
					ptDst[*i_Row][*i_Column].breeding_period = l_iSquirrelBP;
					ptDst[*i_Row][*i_Column].isUpdated = true;

				}
				ptDestWorld->isUpdated = true;
			}
			if (l_bRightSqchance) {
				world *ptTempWorld = *(ptSource + *i_Row) + *i_Column + 1;
				world *ptDestWorld = *(ptDst + *i_Row) + *i_Column + 1;
				if (ptTempWorld->breeding_period == 0) {
					if (ptTempWorld->type == 5)
						ptDestWorld->type = 5;
					else
						ptDestWorld->type = 2;
					ptDestWorld->starvation_period = 0;
					ptDestWorld->breeding_period = l_iSquirrelBP;
					ptDestWorld->isUpdated = true;
				} else {
					ptDst[*i_Row][*i_Column].type = 5;
					ptDst[*i_Row][*i_Column].starvation_period = 0;
					ptDst[*i_Row][*i_Column].breeding_period = l_iSquirrelBP;
					ptDst[*i_Row][*i_Column].isUpdated = true;

				}
				ptDestWorld->isUpdated = true;
			}

			if (l_bDownSqchance) {
				world *ptTempWorld = *(ptSource + *i_Row + 1) + *i_Column;
				world *ptDestWorld = *(ptDst + *i_Row + 1) + *i_Column;
				if (ptTempWorld->breeding_period == 0) {
					if (ptTempWorld->type == 5)
						ptDestWorld->type = 5;
					else
						ptDestWorld->type = 2;
					ptDestWorld->starvation_period = 0;
					ptDestWorld->breeding_period = l_iSquirrelBP;
					ptDestWorld->isUpdated = true;
				} else {
					ptDst[*i_Row][*i_Column].type = 5;
					ptDst[*i_Row][*i_Column].starvation_period = 0;
					ptDst[*i_Row][*i_Column].breeding_period = l_iSquirrelBP;
					ptDst[*i_Row][*i_Column].isUpdated = true;

				}
				ptDestWorld->isUpdated = true;
			}

			if (l_bLeftSqchance) {
				world *ptTempWorld = *(ptSource + *i_Row) + *i_Column - 1;
				world *ptDestWorld = *(ptDst + *i_Row) + *i_Column - 1;
				if (ptTempWorld->breeding_period == 0) {
					if (ptTempWorld->type == 5)
						ptDestWorld->type = 5;
					else
						ptDestWorld->type = 2;
					ptDestWorld->starvation_period = 0;
					ptDestWorld->breeding_period = l_iSquirrelBP;
					ptDestWorld->isUpdated = true;
				} else {
					ptDst[*i_Row][*i_Column].type = 5;
					ptDst[*i_Row][*i_Column].starvation_period = 0;
					ptDst[*i_Row][*i_Column].breeding_period = l_iSquirrelBP;
					ptDst[*i_Row][*i_Column].isUpdated = true;
				}
				ptDestWorld->isUpdated = true;
			}
		}
	} else if (ptSource[*i_Row][*i_Column].type == 2) { // if that cell has a squirrel  , other wolf come and eat*/
#ifdef DEBUG
			printf("Processing squirrel  cell : row - %d | column = %d \n", *i_Row,
					*i_Column);
#endif
		bool l_bUperWolfchance = false;
		bool l_bRightWolfchance = false;
		bool l_bDownWolfchance = false;
		bool l_bLeftWolfchance = false;
		int l_iUpStarvationPeriod = 0;
		int l_iRightStarvationPeriod = 0;
		int l_iDownStartvationPeriod = 0;
		int l_iLeftStarvationPeriod = 0;

		int l_iUpBreadingPeriod = 0;
		int l_iRightBreadingPeriod = 0;
		int l_iDownBreadingPeriod = 0;
		int l_iLeftBreadingPeriod = 0;
		int l_iCounter = 0;

		int l_iTotalStarvationPeriod = 0;
		int l_nextCell = 0;
		l_nextCell = *i_Column - 1;

		if ((fnPossibleMovementToSameCell(i_Row, &l_nextCell, i_Row, i_Column,
				ptSource))) {

			l_bLeftWolfchance = true;
			l_iLeftStarvationPeriod =
					ptSource[*i_Row][*i_Column - 1].starvation_period;
			l_iTotalStarvationPeriod += l_iLeftStarvationPeriod;
			l_iLeftBreadingPeriod =
					ptSource[*i_Row][*i_Column - 1].breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Column + 1;

		if ((fnPossibleMovementToSameCell(i_Row, &l_nextCell, i_Row, i_Column,
				ptSource))) {

			l_bRightWolfchance = true;
			l_iRightStarvationPeriod =
					ptSource[*i_Row][*i_Column + 1].starvation_period;
			l_iTotalStarvationPeriod += l_iRightStarvationPeriod;
			l_iRightBreadingPeriod =
					ptSource[*i_Row][*i_Column + 1].breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Row + 1;

		if ((fnPossibleMovementToSameCell(&l_nextCell, i_Column, i_Row,
				i_Column, ptSource))) {

			l_bDownWolfchance = true;
			l_iDownStartvationPeriod =
					ptSource[*i_Row + 1][*i_Column].starvation_period;
			l_iTotalStarvationPeriod += l_iDownStartvationPeriod;
			l_iDownBreadingPeriod =
					ptSource[*i_Row + 1][*i_Column].breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Row - 1;

		if ((fnPossibleMovementToSameCell(&l_nextCell, i_Column, i_Row,
				i_Column, ptSource))) {

			l_bUperWolfchance = true;
			l_iUpStarvationPeriod =
					ptSource[*i_Row - 1][*i_Column].starvation_period;
			l_iTotalStarvationPeriod += l_iUpStarvationPeriod;
			l_iUpBreadingPeriod =
					ptSource[*i_Row - 1][*i_Column].breeding_period;
			++l_iCounter;
		}

		if (l_iCounter != 0) {

			int l_interMax1 =
			MAX(l_iLeftStarvationPeriod, l_iRightStarvationPeriod);
			int l_interMax2 =
			MAX(
					l_iDownStartvationPeriod, l_iUpStarvationPeriod);
			int l_iMaxStarvation = MAX(l_interMax1,
					l_interMax2);
			if (l_iMaxStarvation == (l_iTotalStarvationPeriod / l_iCounter)) //starvation is equal for every wolf
					{
				//lets calculate the maximum breading
				int l_interMaxBread1 =
				MAX(l_iLeftBreadingPeriod, l_iRightBreadingPeriod);
				int l_interMaxBread2 =
				MAX(
						l_iDownBreadingPeriod, l_iUpBreadingPeriod);
				int l_iMaxBreading = MAX(l_interMaxBread1,
						l_interMaxBread2);
//update the cell with these parameters
				ptDst[*i_Row][*i_Column].type = 1;
				ptDst[*i_Row][*i_Column].starvation_period = l_iMaxStarvation;
				ptDst[*i_Row][*i_Column].breeding_period = l_iMaxBreading;
				ptDst[*i_Row][*i_Column].isUpdated = true;
			} else {

				if (l_iMaxStarvation == l_iLeftStarvationPeriod) {
					ptDst[*i_Row][*i_Column].breeding_period =
							ptSource[*i_Row][*i_Column - 1].breeding_period;
				} else if (l_iMaxStarvation == l_iRightStarvationPeriod) {
					ptDst[*i_Row][*i_Column].breeding_period =
							ptSource[*i_Row][*i_Column + 1].breeding_period;

				} else if (l_iMaxStarvation == l_iDownStartvationPeriod) {
					ptDst[*i_Row][*i_Column].breeding_period = ptSource[*i_Row
							+ 1][*i_Column].breeding_period;

				} else {
					ptDst[*i_Row][*i_Column].breeding_period = ptSource[*i_Row
							- 1][*i_Column].breeding_period;
				}

			}
			world *pt_TempWorld;
			world *ptDestWorld;
			if (l_bUperWolfchance) {
				pt_TempWorld = *(ptSource + *i_Row - 1) + *i_Column;
				ptDestWorld = *(ptDst + *i_Row - 1) + *i_Column;
				if (pt_TempWorld->breeding_period == 0) {
					ptDestWorld->type = 1;
					ptDestWorld->starvation_period = l_iWolfSP;
					ptDestWorld->breeding_period = l_iWolfBP;
					ptDestWorld->isUpdated = true;
				}
				ptDestWorld->isUpdated = true;
			}
			if (l_bRightWolfchance) {
				pt_TempWorld = *(ptSource + *i_Row) + *i_Column + 1;
				ptDestWorld = *(ptDst + *i_Row) + *i_Column + 1;
				if (pt_TempWorld->breeding_period == 0) {

					ptDestWorld->type = 1;
					ptDestWorld->starvation_period = l_iWolfSP;
					ptDestWorld->breeding_period = l_iWolfBP;
					ptDestWorld->isUpdated = true;
				}
				ptDestWorld->isUpdated = true;
			}

			if (l_bDownWolfchance) {
				pt_TempWorld = *(ptSource + *i_Row + 1) + *i_Column;
				ptDestWorld = *(ptDst + *i_Row + 1) + *i_Column;
				if (pt_TempWorld->breeding_period == 0) {

					ptDestWorld->type = 1;
					ptDestWorld->starvation_period = l_iWolfSP;
					ptDestWorld->breeding_period = l_iWolfBP;
					ptDestWorld->isUpdated = true;
				}
				ptDestWorld->isUpdated = true;
			}

			if (l_bLeftWolfchance) {
				pt_TempWorld = *(ptSource + *i_Row) + *i_Column - 1;
				ptDestWorld = *(ptDst + *i_Row) + *i_Column - 1;
				if (pt_TempWorld->breeding_period == 0) {

					ptDestWorld->type = 1;
					ptDestWorld->starvation_period = l_iWolfSP;
					ptDestWorld->breeding_period = l_iWolfBP;
					ptDestWorld->isUpdated = true;
				}
				ptDestWorld->isUpdated = true;
			}
		}
	}
}

/*-----------------------------------------------------------------------------------------------------*/
void fnCopyWorld(world **src_World, world **dst_World) {
	int m, n;

	for (m = 0; m < g_GridSize; ++m) {
		for (n = 0; n < g_GridSize; ++n) {
			if (dst_World[m][n].isUpdated) {
				src_World[m][n].type = dst_World[m][n].type;
				src_World[m][n].breeding_period =
						dst_World[m][n].breeding_period;
				src_World[m][n].starvation_period =
						dst_World[m][n].starvation_period;
				src_World[m][n].isUpdated = false;

			}

		}
	}
	for (m = 0; m < g_GridSize; ++m)
		memset(dst_World[m], 0, sizeof(world) * g_GridSize);

}

void fnInitiateWorld() {
	pt_SourceWorld = (world **) malloc(sizeof(world) * g_GridSize);

	//Allocating memory for the columns of three matrices.
	int i;
	for (i = 0; i < g_GridSize; i++) {
		pt_SourceWorld[i] = (world *) malloc(sizeof(world) * g_GridSize);
	}

	int m;
	for (m = 0; m < g_GridSize; ++m)
		memset(pt_SourceWorld[m], 0, sizeof(world) * g_GridSize);

	pt_DestinationWorld = (world **) malloc(sizeof(world) * g_GridSize);
	for (i = 0; i < g_GridSize; i++) {
		pt_DestinationWorld[i] = (world *) malloc(sizeof(world) * g_GridSize);
	}

	for (m = 0; m < g_GridSize; ++m)
		memset(pt_DestinationWorld[m], 0, sizeof(world) * g_GridSize);

}
void fnLoadtheGeneration(char *_pFileName, int _iWolfBP, int _iSquirrelBP,
		int _iWolfSP) {

	FILE *l_fp;

	l_fp = fopen(_pFileName, "r");
	int l_xCor;
	int l_yCor;
	int l_Type;
	int l_iCounter = 0;
	int l_iLineCounter = 0;

	if (l_fp != NULL) {

		char line[128]; /* or other suitable maximum line size */
		while (fgets(line, sizeof line, l_fp) != NULL) /* read a line */
		{
			char *l_sTem = (char*) malloc(20);

			char *tok = NULL;
			strcpy(l_sTem, line);
			l_sTem[strlen(line) - 1] = '\0';
			if (l_iLineCounter == 0) {
				g_GridSize = atoi(l_sTem);
				fnInitiateWorld();
			}
			tok = strtok(l_sTem, " ");

			while (tok) {
#ifdef DEBUG
				printf("Token: %s--counter - %d\n", tok, l_iCounter);
#endif
				if (l_iCounter == 0)
					l_yCor = atoi(tok);
				if (l_iCounter == 1)
					l_xCor = atoi(tok);
				if (l_iCounter == 2) {
					if (strcmp(tok, WOLF) == 0)
						l_Type = 1;
					else if (strcmp(tok, SQUIRREL) == 0)
						l_Type = 2;
					else if (strcmp(tok, TREE) == 0)
						l_Type = 3;
					else if (strcmp(tok, ICE) == 0)
						l_Type = 4;
					else
						printf("Unrecognized character in the file \n");
				}
				tok = strtok(NULL, " ");
				++l_iCounter;
			}
			if (l_xCor <= g_GridSize - 1 && l_yCor <= g_GridSize - 1) {
#ifdef DEBUG
				printf("x cor = %d | y cor = %d \n", l_xCor, l_yCor);
#endif
				pt_SourceWorld[l_yCor][l_xCor].type = l_Type;
				switch (l_Type) {
				case 1: /*wolf*/
				{
					pt_SourceWorld[l_yCor][l_xCor].breeding_period = _iWolfBP;
					pt_SourceWorld[l_yCor][l_xCor].starvation_period = _iWolfSP;
				}
					break;
				case 2: /*Squirrel*/
				{
					pt_SourceWorld[l_yCor][l_xCor].breeding_period =
							_iSquirrelBP;
					pt_SourceWorld[l_yCor][l_xCor].starvation_period = 0;
				}
					break;
				case 3:
				case 4: {
					pt_SourceWorld[l_yCor][l_xCor].breeding_period = 0;
					pt_SourceWorld[l_yCor][l_xCor].starvation_period = 0;
				}
					break;

				}
			}
			l_iCounter = 0;
			++l_iLineCounter;
			free(l_sTem);
		}
		fclose(l_fp);
	} else {
		perror(_pFileName); /* why didn't the file open? */
	}
	/* Load the generation in to world*/
}
void fnProcessWorld() {
	int i;

	int m;
	int n;
	int l_iStart = 0;
	int l_iGenCounter = 0;

	double l_clkStartTime = omp_get_wtime();

	for (i = 1; i <= g_iNumberofGen; i++) {
		++l_iGenCounter;

		for (m = 0; m < g_GridSize; ++m) {
			if (m % 2 == 0)
				l_iStart = 1;
			else
				l_iStart = 0;

			for (n = l_iStart; n < g_GridSize; n = n + 2) {

				fnUpdateCell(&m, &n, pt_SourceWorld, pt_DestinationWorld);
			}
		}

		fnCopyWorld(pt_SourceWorld, pt_DestinationWorld);

		for (m = 0; m < g_GridSize; ++m) {
			for (n = (m % 2); n < g_GridSize; n = n + 2) {

				fnUpdateCell(&m, &n, pt_SourceWorld, pt_DestinationWorld);
			}
		}

		fnCopyWorld(pt_SourceWorld, pt_DestinationWorld);

		//fnDumpTheOutput(&l_iGenCounter, pt_SourceWorld);
		if (l_iGenCounter < g_iNumberofGen) {
			int l_iRow;
			int l_iColumn;
			for (l_iRow = 0; l_iRow < g_GridSize; ++l_iRow) {
				for (l_iColumn = 0; l_iColumn < g_GridSize; ++l_iColumn) {
					world *l_ptWorld = *(pt_SourceWorld + l_iRow) + l_iColumn;
					if (l_ptWorld->type == 1) {
						if (l_ptWorld->breeding_period != 0)
							--l_ptWorld->breeding_period;
						--l_ptWorld->starvation_period;
						if (l_ptWorld->starvation_period == -1) // we are going to reduce the starvation period and it is going to die here
								{
							l_ptWorld->type = 0;
							l_ptWorld->starvation_period = 0;
							l_ptWorld->breeding_period = 0;

						}
#ifdef DEBUG
						printf(
								" %d Generation finished wolf breeding period = %d | st period =%d |row =%d | Column =%d\n",
								l_iGenCounter, l_ptWorld->breeding_period,
								l_ptWorld->starvation_period, l_iRow,
								l_iColumn);
#endif
					} else if ((l_ptWorld->type == 2)
							|| (l_ptWorld->type == 5)) {
						if (l_ptWorld->breeding_period != 0)
							--l_ptWorld->breeding_period;
					}

				}
			}
		}
	}

	fnCopyWorld(pt_SourceWorld, pt_DestinationWorld);

	double l_clkEndtime = omp_get_wtime();

	printf("Total serial code processing time - %.16g \n",
			(l_clkEndtime - l_clkStartTime));
	fnPrintWorld(pt_SourceWorld);
//	/fnPrintWorld();

}

int main(int argc, char **argv) {

	char *l_pFileName;
	double d_ProgramStartTime = omp_get_wtime();
	if (argc > 1) {

		l_pFileName = argv[1];
		l_iWolfBP = atoi(argv[2]);
		l_iSquirrelBP = atoi(argv[3]);
		l_iWolfSP = atoi(argv[4]);
		g_iNumberofGen = atoi(argv[5]);
		fnLoadtheGeneration(l_pFileName, l_iWolfBP, l_iSquirrelBP, l_iWolfSP);
	} else {
		printf("Please check the argument list \n");
	}

	printf("\n");
	fnProcessWorld();

	/*Free the memory that we have allocated */
	int j;
	for (j = 0; j < g_GridSize; j++) {
		world * pt_WorldPointer = pt_SourceWorld[j];
		free(pt_WorldPointer);
	}
	for (j = 0; j < g_GridSize; j++) {
		world * pt_WorldPointer = pt_DestinationWorld[j];
		free(pt_WorldPointer);
	}
	double d_ProgramEndTime = omp_get_wtime();

	printf("Total processing time of serial code - %.16g \n",
			(d_ProgramEndTime - d_ProgramStartTime));
	puts(""); /* prints  */
	return EXIT_SUCCESS;
}

