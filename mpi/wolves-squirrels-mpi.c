/*
 ============================================================================
 Name        : wolves-squirrels-mpi.c
 Author      : Group-40 (Gayana, Sri and Stefan)
 Version     : 1.0
 Copyright   : CPD
 Description : OpenMPI  World in C
 ============================================================================
 */


#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>

#define WOLF "w"
#define SQUIRREL "s"
#define TREE "t"
#define ICE "i"

#define SPAWN_THREAD 2
#define ROOT 0
// maximum of two numbers , defined  here
#define MAX(a,b) (((a)>(b))?(a):(b))

// matrix element structure

typedef struct world {
	int type;
	int breeding_period;
	int starvation_period;
	int isUpdated;
	int i_RelativePosX;
	int i_RelativePosY;
} world;

// define global variables
int g_iNumberofGen;
int g_GridSize;
int g_ComMatrixSize;
int l_iWolfBP;
int l_iSquirrelBP;
int l_iWolfSP;

/* define the root source file , this will be initially called by the program to populate the world*/
world **pt_SourceRoot;

/* Every other slaves including master will hold this */
world **pt_SourceWorld;
world **pt_DestinationWorld;


int *pGlobalLoadArray;
int *pAggArray;
int *pActualLoadArray;
FILE *gf_DumpFile;

/* once finished processing , this function will be called by master to dumb the output */
void fnDumpTheOutput(world **ptPrintWorld, int *pGridSize,
		int *pNumberofProcess) {
	char zFilebuf[256];
	snprintf(zFilebuf, sizeof(zFilebuf), "/tmp/World_np_%d_Gen_%d_WSz_%d.out",
			*pNumberofProcess,g_iNumberofGen,*pGridSize);
	gf_DumpFile = fopen(zFilebuf, "a+");
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
				fprintf(gf_DumpFile, "%d %d w\n", row, columns);

			}
				break;
			case 2: {
				fprintf(gf_DumpFile, "%d %d s\n", row, columns);
			}
				break;
			case 3: {
				fprintf(gf_DumpFile, "%d %d t\n", row, columns);
			}
				break;
			case 4: {
				fprintf(gf_DumpFile, "%d %d i\n", row, columns);
			}
				break;
			case 5: {
				fprintf(gf_DumpFile, "%d %d $\n", row, columns);
			}
				break;

			}
		}

	}

	fclose(gf_DumpFile);

}

/* this function gets actual row ,actual column and adjacent row ,adjacent column and map as input .
 * it will give the bool as result ,saying , whether any of the adjacent cell has chance to move to actual cell one by one
 * this is result then will be used to determine cell movements
 */
bool fnPossibleMovementToSameCell(int *i_Row, int *i_Column, int *_iActualRow,
		int *_iActualColumn, world **pt_Whichworld, int rank, int num_proc) {

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
	int l_iThresholdPos = 0;
	if (rank == 0) {
		l_iThresholdPos = pGlobalLoadArray[rank] + 2;
	} else if (rank == num_proc - 1) {
		l_iThresholdPos = pGlobalLoadArray[rank] + 2;
	} else {
		l_iThresholdPos = pGlobalLoadArray[rank] + 4;
	}
	if ((*i_Row >= 0 && *i_Row <= l_iThresholdPos - 1)
			&& (*i_Column >= 0 && *i_Column <= g_GridSize - 1)) {

		world *l_ptWorld = *(pt_Whichworld + *i_Row) + *i_Column;
		int *ptTempRow = i_Row;
		int *ptTempColumn = i_Column;

		switch (l_ptWorld->type) {
		case 0:
		case 4:
			return false;

		case 1: // if it is wolf , process is different, because we have to consider only more than one squirrel
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

			if (*ptTempRow + 1 <= l_iThresholdPos - 1) {
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

			if (*ptTempRow + 1 <= l_iThresholdPos - 1) {

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
		}
			break;
		case 2:
		case 5: {
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

			if (*ptTempRow + 1 <= l_iThresholdPos - 1) {

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
			break;
		default:
			break;
		}

		if (l_iNumberofPossiblemovement != 0) {
			int l_iCValue = (pt_Whichworld[*i_Row][*i_Column].i_RelativePosX)
					* g_GridSize + *i_Column;
			int l_imodval = l_iCValue % l_iNumberofPossiblemovement;
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
/* in order to avoid the complexity in the main update function, this function would really helpful to separate
 * the complex cases in the caller function, basically this function updates mirror world according to previous function
 * result.
 */
void fnupdateCell(int *_iBreedingPeriod, int *_iStarvationPeriod, int *_iType,
		int *i_Row, int *i_Column, int *_iUpdateRow, int *_iUpdateColumn,
		world **pt_srcWorld, world **pt_DstWorld) {
	world *l_ptSrcWorld = *(pt_srcWorld + *i_Row) + *i_Column;
	world *l_ptDstWorldCell = *(pt_DstWorld + *_iUpdateRow) + *_iUpdateColumn;
	switch (*_iType) {
	case 1: {
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
		break;
	case 2: {
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
		break;
	case 5: {
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
		break;
	default:
		break;
	}

}
/* if we are processing empty cell, then wolf can come to that cell, there is maximum four possible chance to come
 * each of the possible chance will be analyzed by caller function and result will be passed here to update.
 * if any of the wolf breeds before come to an empty cell, then we have to update corresponding cell as well.*/
void fnSingleUpdatetoCell(world **ptSource, world **ptDst, int *i_Row,
		int *i_Column) {

	world *pt_TempWorld;
	world *pt_TempDstWorld;
	pt_TempWorld = *(ptSource + *i_Row) + *i_Column;
	pt_TempDstWorld = *(ptDst + *i_Row) + *i_Column;

	if (pt_TempWorld->breeding_period == 0) {

		pt_TempDstWorld->type = 1;
		pt_TempDstWorld->starvation_period = l_iWolfSP;
		pt_TempDstWorld->breeding_period = l_iWolfBP;
		pt_TempDstWorld->isUpdated = true;
	}
	pt_TempDstWorld->isUpdated = true;

}

/* Tree processing is slightly different than other process, even though basic processing concepts are same, the tpye we updating
 * is differ from wolf processing, that is why we have created separate function to handle this .
 */
void fnTreeCellUpdate(world **ptSource, world **ptDst, int *i_Row,
		int *i_Column, int *iActualRow, int *iActualColumn) {

	world *ptTempWorld = *(ptSource + *i_Row) + *i_Column;
	world *ptDestWorld = *(ptDst + *i_Row) + *i_Column;
	if (ptTempWorld->breeding_period == 0) {
		if (ptTempWorld->type == 5)
			ptDestWorld->type = 5;
		else
			ptDestWorld->type = 2;
		ptDestWorld->starvation_period = 0;
		ptDestWorld->breeding_period = l_iSquirrelBP;
		ptDestWorld->isUpdated = true;
	} else {
		(*(ptDst + *iActualRow) + *iActualColumn)->type = 5;

		(*(ptDst + *iActualRow) + *iActualColumn)->starvation_period = 0;
		(*(ptDst + *iActualRow) + *iActualColumn)->breeding_period =
				l_iSquirrelBP;
		(*(ptDst + *iActualRow) + *iActualColumn)->isUpdated = true;

	}
	ptDestWorld->isUpdated = true;

}

/* This is the heart function of whole program, basically this function will be receiving the cell and columns it want to process
 * and check neighboring cells possibilities , according to neighboring cells result, then every updates will be happened to mirror world
 */
void fnUpdateCell(int *i_Row, int *i_Column, world **ptSource, world **ptDst,
		int rank, int num_proc) {

	world *l_ptWorld = *(ptSource + *i_Row) + *i_Column;
	int l_nextCell = 0;

	switch (l_ptWorld->type) {

	case 0: {
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
		l_nextCell = *i_Column + 1;
		if (fnPossibleMovementToSameCell(i_Row, &l_nextCell, i_Row, i_Column,
				ptSource, rank, num_proc)) {
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
				ptSource, rank, num_proc)) {
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
				ptSource, rank, num_proc)) {

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
				ptSource, rank, num_proc)) {

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

				(*(ptDst + *i_Row) + *i_Column)->type = 1;
				(*(ptDst + *i_Row) + *i_Column)->isUpdated = true;
				if (l_iSqCounter > 0) // so there are squirrel , so we need to set final wolf's starvation period as initial starvation period
						{
					(*(ptDst + *i_Row) + *i_Column)->starvation_period =
							l_iWolfSP;
				} else
					(*(ptDst + *i_Row) + *i_Column)->starvation_period =
							l_iMaxStarvation;
				if (bIsStarvationEqual) {

					(*(ptDst + *i_Row) + *i_Column)->breeding_period =
							l_iMaxBreading;
				} else {
					if (l_iMaxStarvation == l_iLeftStarvationPeriod) {
						(*(ptDst + *i_Row) + *i_Column)->breeding_period =
								(*(ptSource + *i_Row) + *i_Column - 1)->breeding_period;
					} else if (l_iMaxStarvation == l_iRightStarvationPeriod) {
						(*(ptDst + *i_Row) + *i_Column)->breeding_period =
								(*(ptSource + *i_Row) + *i_Column + 1)->breeding_period;

					} else if (l_iMaxStarvation == l_iDownStartvationPeriod) {
						(*(ptDst + *i_Row) + *i_Column)->breeding_period =
								(*(ptSource + *i_Row + 1) + *i_Column)->breeding_period;

					} else {
						(*(ptDst + *i_Row) + *i_Column)->breeding_period =
								(*(ptSource + *i_Row - 1) + *i_Column)->breeding_period;
					}

				}

				if (l_bUperWolfchance) {
					l_nextCell = *i_Row - 1;
					fnSingleUpdatetoCell(ptSource, ptDst, &l_nextCell,
							i_Column);

				}
				if (l_bRightWolfchance) {
					l_nextCell = *i_Column + 1;
					fnSingleUpdatetoCell(ptSource, ptDst, i_Row, &l_nextCell);

				}

				if (l_bDownWolfchance) {
					l_nextCell = *i_Row + 1;
					fnSingleUpdatetoCell(ptSource, ptDst, &l_nextCell,
							i_Column);

				}

				if (l_bLeftWolfchance) {
					l_nextCell = *i_Column - 1;
					fnSingleUpdatetoCell(ptSource, ptDst, i_Row, &l_nextCell);

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

				if (l_bUperSqchance) {

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

	}
		break;
	case 3: { // if that cell has a tree    /*there is no conflict , and check other movements*/

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
				ptSource, rank, num_proc))) {
			l_bLeftSqchance = true;
			l_iLeftBreadingPeriod =
					(*(ptSource + *i_Row) + *i_Column - 1)->breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Column + 1;
		if ((fnPossibleMovementToSameCell(i_Row, &l_nextCell, i_Row, i_Column,
				ptSource, rank, num_proc))) {
			l_bRightSqchance = true;

			l_iRightBreadingPeriod =
					(*(ptSource + *i_Row) + *i_Column + 1)->breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Row + 1;
		if ((fnPossibleMovementToSameCell(&l_nextCell, i_Column, i_Row,
				i_Column, ptSource, rank, num_proc))) {
			l_bDownSqchance = true;
			l_iDownBreadingPeriod =
					(*(ptSource + *i_Row + 1) + *i_Column)->breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Row - 1;
		if ((fnPossibleMovementToSameCell(&l_nextCell, i_Column, i_Row,
				i_Column, ptSource, rank, num_proc))) {
			l_bUperSqchance = true;
			l_iUpBreadingPeriod =
					(*(ptSource + *i_Row - 1) + *i_Column)->breeding_period;
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
				(*(ptDst + *i_Row) + *i_Column)->type = 5;
				(*(ptDst + *i_Row) + *i_Column)->starvation_period = 0;
				(*(ptDst + *i_Row) + *i_Column)->breeding_period =
						l_iMaxBreading;
				(*(ptDst + *i_Row) + *i_Column)->isUpdated = true;
			}

			if (l_bUperSqchance) {
				l_nextCell = *i_Row - 1;
				fnTreeCellUpdate(ptSource, ptDst, &l_nextCell, i_Column, i_Row,
						i_Column);

			}
			if (l_bRightSqchance) {
				l_nextCell = *i_Column + 1;
				fnTreeCellUpdate(ptSource, ptDst, i_Row, &l_nextCell, i_Row,
						i_Column);

			}

			if (l_bDownSqchance) {
				l_nextCell = *i_Row + 1;
				fnTreeCellUpdate(ptSource, ptDst, &l_nextCell, i_Column, i_Row,
						i_Column);
			}

			if (l_bLeftSqchance) {
				l_nextCell = *i_Column - 1;
				fnTreeCellUpdate(ptSource, ptDst, i_Row, &l_nextCell, i_Row,
						i_Column);

			}
		}
	}
		break;
	case 2: { // if that cell has a squirrel  , other wolf come and eat*/

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
				ptSource, rank, num_proc))) {

			l_bLeftWolfchance = true;
			l_iLeftStarvationPeriod =
					(*(ptSource + *i_Row) + *i_Column - 1)->starvation_period;
			l_iTotalStarvationPeriod += l_iLeftStarvationPeriod;
			l_iLeftBreadingPeriod =
					(*(ptSource + *i_Row) + *i_Column - 1)->breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Column + 1;

		if ((fnPossibleMovementToSameCell(i_Row, &l_nextCell, i_Row, i_Column,
				ptSource, rank, num_proc))) {

			l_bRightWolfchance = true;
			l_iRightStarvationPeriod =
					(*(ptSource + *i_Row) + *i_Column + 1)->starvation_period;
			l_iTotalStarvationPeriod += l_iRightStarvationPeriod;
			l_iRightBreadingPeriod =
					(*(ptSource + *i_Row) + *i_Column + 1)->breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Row + 1;

		if ((fnPossibleMovementToSameCell(&l_nextCell, i_Column, i_Row,
				i_Column, ptSource, rank, num_proc))) {

			l_bDownWolfchance = true;
			l_iDownStartvationPeriod =
					(*(ptSource + *i_Row + 1) + *i_Column)->starvation_period;
			l_iTotalStarvationPeriod += l_iDownStartvationPeriod;
			l_iDownBreadingPeriod =
					(*(ptSource + *i_Row + 1) + *i_Column)->breeding_period;
			++l_iCounter;
		}
		l_nextCell = *i_Row - 1;

		if ((fnPossibleMovementToSameCell(&l_nextCell, i_Column, i_Row,
				i_Column, ptSource, rank, num_proc))) {

			l_bUperWolfchance = true;
			l_iUpStarvationPeriod =
					(*(ptSource + *i_Row - 1) + *i_Column)->starvation_period;
			l_iTotalStarvationPeriod += l_iUpStarvationPeriod;
			l_iUpBreadingPeriod =
					(*(ptSource + *i_Row - 1) + *i_Column)->breeding_period;
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
				(*(ptDst + *i_Row) + *i_Column)->type = 1;
				(*(ptDst + *i_Row) + *i_Column)->starvation_period =
						l_iMaxStarvation;
				(*(ptDst + *i_Row) + *i_Column)->breeding_period =
						l_iMaxBreading;
				(*(ptDst + *i_Row) + *i_Column)->isUpdated = true;
			} else {

				if (l_iMaxStarvation == l_iLeftStarvationPeriod) {
					(*(ptDst + *i_Row) + *i_Column)->breeding_period =
							(*(ptSource + *i_Row) + *i_Column - 1)->breeding_period;
				} else if (l_iMaxStarvation == l_iRightStarvationPeriod) {
					(*(ptDst + *i_Row) + *i_Column)->breeding_period =
							(*(ptSource + *i_Row) + *i_Column + 1)->breeding_period;

				} else if (l_iMaxStarvation == l_iDownStartvationPeriod) {
					(*(ptDst + *i_Row) + *i_Column)->breeding_period =
							(*(ptSource + *i_Row + 1) + *i_Column)->breeding_period;

				} else {
					(*(ptDst + *i_Row) + *i_Column)->breeding_period =
							(*(ptSource + *i_Row - 1) + *i_Column)->breeding_period;
				}

			}

			if (l_bUperWolfchance) {

				l_nextCell = *i_Row - 1;
				fnSingleUpdatetoCell(ptSource, ptDst, &l_nextCell, i_Column);

			}
			if (l_bRightWolfchance) {
				l_nextCell = *i_Column + 1;
				fnSingleUpdatetoCell(ptSource, ptDst, i_Row, &l_nextCell);

			}

			if (l_bDownWolfchance) {
				l_nextCell = *i_Row + 1;
				fnSingleUpdatetoCell(ptSource, ptDst, &l_nextCell, i_Column);

			}

			if (l_bLeftWolfchance) {
				l_nextCell = *i_Column - 1;
				fnSingleUpdatetoCell(ptSource, ptDst, i_Row, &l_nextCell);

			}
		}
	}
		break;
	default:
		break;
	}
}
/*  Main memory allocation for this program, this function would allocate series of memory chunk and
 *  this is the  most optimized way to allocate matrix rather than conventional way */
world **alloc_2d_world(int rows, int cols) {
	int i;
	world *ptContiniousMem = (world *) malloc(rows * cols * sizeof(world));
	world **ptSrc = (world **) malloc(rows * sizeof(world*));
	for (i = 0; i < rows; i++)
		ptSrc[i] = &(ptContiniousMem[cols * i]);

	return ptSrc;
}
/* Once read all the coordinates from input file, it will create new world according to input size */
void fnInitiateWorld() {

	pt_SourceRoot = alloc_2d_world(g_GridSize, g_GridSize);
	int iRow, iCol;
	for (iRow = 0; iRow < g_GridSize; ++iRow) {
		for (iCol = 0; iCol < g_GridSize; ++iCol) {
			pt_SourceRoot[iRow][iCol].breeding_period = 0;
			pt_SourceRoot[iRow][iCol].starvation_period = 0;
			pt_SourceRoot[iRow][iCol].i_RelativePosX = iRow;
			pt_SourceRoot[iRow][iCol].i_RelativePosY = iCol;
			pt_SourceRoot[iRow][iCol].type = 0;
			pt_SourceRoot[iRow][iCol].isUpdated = 0;

		}
	}

}
void fnInitDstWorld(world **pDstWord, int *pRow) {
	int iRow, iCol;
	for (iRow = 0; iRow < *pRow; ++iRow) {
		for (iCol = 0; iCol < g_GridSize; ++iCol) {
			pDstWord[iRow][iCol].breeding_period = 0;
			pDstWord[iRow][iCol].starvation_period = 0;
			pDstWord[iRow][iCol].i_RelativePosX = 0;
			pDstWord[iRow][iCol].i_RelativePosY = 0;
			pDstWord[iRow][iCol].type = 0;
			pDstWord[iRow][iCol].isUpdated = 0;

		}
	}

}

void fnLoadtheGeneration(char *_pFileName, int _iWolfBP, int _iSquirrelBP,
		int _iWolfSP, int *pGridSize) {

	FILE *l_fp;

	l_fp = fopen(_pFileName, "r");
	int l_xCor = 0;
	int l_yCor = 0;
	int l_Type = 0;
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
				*pGridSize = atoi(l_sTem);
				g_GridSize = *pGridSize;
				fnInitiateWorld();
			}
			tok = strtok(l_sTem, " ");

			while (tok) {

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
						printf("/* Unrecognized character in the file */ \n");
				}
				tok = strtok(NULL, " ");
				++l_iCounter;
			}
			if (l_xCor <= g_GridSize - 1 && l_yCor <= g_GridSize - 1) {

				pt_SourceRoot[l_yCor][l_xCor].type = l_Type;
				switch (l_Type) {
				case 1: /*wolf*/
				{
					pt_SourceRoot[l_yCor][l_xCor].breeding_period = _iWolfBP;
					pt_SourceRoot[l_yCor][l_xCor].starvation_period = _iWolfSP;
				}
					break;
				case 2: /*Squirrel*/
				{
					pt_SourceRoot[l_yCor][l_xCor].breeding_period =
							_iSquirrelBP;
					pt_SourceRoot[l_yCor][l_xCor].starvation_period = 0;
				}
					break;
				case 3:
				case 4: {
					pt_SourceRoot[l_yCor][l_xCor].breeding_period = 0;
					pt_SourceRoot[l_yCor][l_xCor].starvation_period = 0;
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

/* The name of the function clearly states copy the world , indeed , it will copy from mirror world and update the source world
 * then flush the mirror world , basically , preparing mirror world for next round */

void fnCopyTopAndLastWorld(world **src_World, world **dst_World,
		world *pupdatedworld, int *pSizeofthearray, int iProcessRank,
		int num_proc) {

	int m, n;
	int l_iEndPos = 0;

	l_iEndPos = pGlobalLoadArray[iProcessRank] + 2;
	int iCommonConter = 0;
	for (m = 0; m < l_iEndPos; ++m) {
		for (n = 0; n < g_GridSize; ++n) {
			world *ptDstTemp = *(dst_World + m) + n;
			if (ptDstTemp->isUpdated == 1) {

				world *ptSrcTemp = *(src_World + m) + n;
				ptDstTemp->i_RelativePosX = ptSrcTemp->i_RelativePosX;
				ptDstTemp->i_RelativePosY = ptSrcTemp->i_RelativePosY;

				__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
				memcpy(ptSrcTemp, ptDstTemp, sizeof(world));
				memset(ptDstTemp, 0, sizeof(world));
				if (iProcessRank == 0) {
					if (m >= pGlobalLoadArray[iProcessRank] - 2
							&& m <= pGlobalLoadArray[iProcessRank] + 1) {
						world *ptSrc = *(src_World + m) + n;
						world *ptDst = pupdatedworld + iCommonConter;
						__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
						memcpy(ptDst, ptSrc, sizeof(world));
						++iCommonConter;
						ptSrc->isUpdated = 0;
					}
				}
				if (iProcessRank == num_proc - 1) {
					if (m >= 0 && m <= 3) {
						world *ptSrc = *(src_World + m) + n;
						world *ptDst = pupdatedworld + iCommonConter;
						__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
						memcpy(ptDst, ptSrc, sizeof(world));
						++iCommonConter;
						ptSrc->isUpdated = 0;
					}
				}
			}

		}
	}
	*pSizeofthearray = iCommonConter;

}
/* This function will be used to copy the middle part of the matrix's both two sides (upper side and bottom side ) , because top matrix and last bottom matrix
 * extra row will be copied by above function
 */
void fnCopyMiddleWorld(world **src_World, world **dst_World, world *pUpArray,
		int *pUpArraySize, world *pDownArray, int *pDownArraySize, int rank,
		int num_proc) {
	int m, n;
	int l_iEndPos = 0;
	if (rank == ROOT) {
		l_iEndPos = pGlobalLoadArray[rank] + 2;
	} else if (rank == num_proc - 1) {
		l_iEndPos = pGlobalLoadArray[rank] + 2;
	} else {
		l_iEndPos = pGlobalLoadArray[rank] + 4;
	}
	int iUpCounter = 0;
	int iDownCounter = 0;
	for (m = 0; m < l_iEndPos; ++m) {
		for (n = 0; n < g_GridSize; ++n) {
			world *ptDstTemp = *(dst_World + m) + n;
			if (ptDstTemp->isUpdated == 1) {

				world *ptSrcTemp = *(src_World + m) + n;

				ptDstTemp->i_RelativePosX = ptSrcTemp->i_RelativePosX;
				ptDstTemp->i_RelativePosY = ptSrcTemp->i_RelativePosY;
				__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
				memcpy(ptSrcTemp, ptDstTemp, sizeof(world));
				memset(ptDstTemp, 0, sizeof(world));
				if (m >= 0 && m <= 3) {
					world *ptSrc = *(src_World + m) + n;
					world *ptDst = pUpArray + iUpCounter;
					__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
					memcpy(ptDst, ptSrc, sizeof(world));
					++iUpCounter;
					ptSrc->isUpdated = 0;
				}
				if (m >= pGlobalLoadArray[rank]
						&& m <= pGlobalLoadArray[rank] + 3) {
					world *ptSrc = *(src_World + m) + n;
					world *ptDst = pDownArray + iDownCounter;
					__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
					memcpy(ptDst, ptSrc, sizeof(world));
					++iDownCounter;
					ptSrc->isUpdated = 0;
				}

			}

		}
	}

	*pDownArraySize = iDownCounter;
	*pUpArraySize = iUpCounter;

}
/* This function will be called last to populate the final matrix. every proecess will be sendting their part, so we have to update
 * them accordingly
 */
void fnFinalUpdate(world *pUpdateWorld, int *pSize) {
	int i;
	world *ptUpdateWorld1 = NULL;
	for (i = 0; i < *pSize; ++i) {
		ptUpdateWorld1 = (pUpdateWorld + i);
		int x = ptUpdateWorld1->i_RelativePosX;
		int y = ptUpdateWorld1->i_RelativePosY;
		__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
		memcpy((*(pt_SourceRoot + x) + y), ptUpdateWorld1, sizeof(world));

	}
}

/* Every time , this function will be called by fnMPICommunication , because , every sub generation matrixes are passing their common area with others ,
 * So , when ever we receive the updated array from the neighbors , we should update our host matrx with neighbors updated values */
void fnUpdateWorldFromOtherProcess(world * pUpdateWorld, int *pSize, int rank,
bool bIsUpSide) {

	int i;
	world *ptUpdateWorld1 = NULL;
	int x;
	for (i = 0; i < *pSize; ++i) {
		ptUpdateWorld1 = (pUpdateWorld + i);
		if (bIsUpSide)  //upper paart of the matrix
		{
			x = ptUpdateWorld1->i_RelativePosX - (pAggArray[rank] - 2);
		} else {
			if (rank == ROOT)
				x = ptUpdateWorld1->i_RelativePosX - pAggArray[rank];
			else
				x = ptUpdateWorld1->i_RelativePosX - pAggArray[rank] + 2;
		}
		int y = ptUpdateWorld1->i_RelativePosY;
		if (ptUpdateWorld1->isUpdated == 1) {
			pt_SourceWorld[x][y].breeding_period =
					ptUpdateWorld1->breeding_period;
			pt_SourceWorld[x][y].starvation_period =
					ptUpdateWorld1->starvation_period;
			pt_SourceWorld[x][y].type = ptUpdateWorld1->type;
			pt_SourceWorld[x][y].isUpdated = 0;

		}
	}
}


/* this is the heart part of the mpi communication,  here we are using asyn mpi send and rcv , we are not sure which process is going to finish first,
 * as soon as they finish they come here and send ,and wait for data*/
void fnMPICommunication(int rank, int iProcessRank, world *pUpArray,
		int *pUparraysize, world *pDownarray, int *pDownarraysize) {

	MPI_Datatype worldtype, oldworldtypes[2];
	int blockcounts[2];

	MPI_Aint offsets[1];
	offsets[0] = 0;
	oldworldtypes[0] = MPI_INT;
	blockcounts[0] = 6;
	MPI_Type_struct(1, blockcounts, offsets, oldworldtypes, &worldtype);
	MPI_Type_commit(&worldtype);

	int iMPIError;
	int iUpArraySize = 0;
	int iDownArraySize = 0;
	MPI_Request send_request_first, recv_request_first;
	MPI_Request send_request_second, recv_request_send;
	world PrimaryArray[g_ComMatrixSize];
	world SecondaryArray[g_ComMatrixSize];
	memset(PrimaryArray, 0, g_ComMatrixSize);
	memset(SecondaryArray, 0, g_ComMatrixSize);
	MPI_Status status;
	bool isMiddleMatrix = false;
	if (rank == ROOT) {
		iMPIError = MPI_Isend(pDownarraysize, 1, MPI_INT, rank + 1, 0,
		MPI_COMM_WORLD, &send_request_first);
		iMPIError = MPI_Irecv(&iUpArraySize, 1, MPI_INT, rank + 1, MPI_ANY_TAG,
		MPI_COMM_WORLD, &recv_request_first);

	} else if (rank == iProcessRank - 1) {
		iMPIError = MPI_Isend(pUparraysize, 1, MPI_INT, rank - 1, 0,
		MPI_COMM_WORLD, &send_request_first);
		iMPIError = MPI_Irecv(&iUpArraySize, 1, MPI_INT, rank - 1, MPI_ANY_TAG,
		MPI_COMM_WORLD, &recv_request_first);

	} else {
		iMPIError = MPI_Isend(pDownarraysize, 1, MPI_INT, rank + 1, 0,
		MPI_COMM_WORLD, &send_request_first);
		iMPIError = MPI_Isend(pUparraysize, 1, MPI_INT, rank - 1, 0,
		MPI_COMM_WORLD, &send_request_second);

		iMPIError = MPI_Irecv(&iUpArraySize, 1, MPI_INT, rank - 1, MPI_ANY_TAG,
		MPI_COMM_WORLD, &recv_request_send);
		iMPIError = MPI_Irecv(&iDownArraySize, 1, MPI_INT, rank + 1,
		MPI_ANY_TAG,
		MPI_COMM_WORLD, &recv_request_first);

	}
	// data has been sent to remote process, we have to wait , until data received by other ends .
	if (rank == ROOT || rank == iProcessRank - 1) {
		iMPIError = MPI_Wait(&send_request_first, &status);
		iMPIError = MPI_Wait(&recv_request_first, &status);
	} else {
		iMPIError = MPI_Wait(&send_request_first, &status);
		iMPIError = MPI_Wait(&recv_request_first, &status);
		iMPIError = MPI_Wait(&send_request_second, &status);
		iMPIError = MPI_Wait(&recv_request_send, &status);
	}

	if (rank == ROOT) {

		iMPIError = MPI_Isend(pDownarray, *pDownarraysize, worldtype, rank + 1,
				0,
				MPI_COMM_WORLD, &send_request_first);
		iMPIError = MPI_Irecv(PrimaryArray, iUpArraySize, worldtype, rank + 1,
		MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request_first);

	} else if (rank == iProcessRank - 1) {

		iMPIError = MPI_Isend(pUpArray, *pUparraysize, worldtype, rank - 1, 0,
		MPI_COMM_WORLD, &send_request_first);
		iMPIError = MPI_Irecv(PrimaryArray, iUpArraySize, worldtype, rank - 1,
		MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request_first);

	} else {

		iMPIError = MPI_Isend(pDownarray, *pDownarraysize, worldtype, rank + 1,
				0,
				MPI_COMM_WORLD, &send_request_first);
		iMPIError = MPI_Isend(pUpArray, *pUparraysize, worldtype, rank - 1, 0,
		MPI_COMM_WORLD, &send_request_second);

		iMPIError = MPI_Irecv(PrimaryArray, iUpArraySize, worldtype, rank - 1,
		MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request_send);
		iMPIError = MPI_Irecv(SecondaryArray, iDownArraySize, worldtype,
				rank + 1,
				MPI_ANY_TAG, MPI_COMM_WORLD, &recv_request_first);

		isMiddleMatrix = true;

	}
	if (rank == ROOT || rank == iProcessRank - 1) {
		iMPIError = MPI_Wait(&send_request_first, &status);
		iMPIError = MPI_Wait(&recv_request_first, &status);
	} else {
		iMPIError = MPI_Wait(&send_request_first, &status);
		iMPIError = MPI_Wait(&recv_request_first, &status);
		iMPIError = MPI_Wait(&send_request_second, &status);
		iMPIError = MPI_Wait(&recv_request_send, &status);
	}

	if (isMiddleMatrix) {
		fnUpdateWorldFromOtherProcess(PrimaryArray, &iUpArraySize, rank,
		true);
		fnUpdateWorldFromOtherProcess(SecondaryArray, &iDownArraySize, rank,
		false);
	} else {
		if (rank == ROOT)
			fnUpdateWorldFromOtherProcess(PrimaryArray, &iUpArraySize, rank,
			false);
		else
			fnUpdateWorldFromOtherProcess(PrimaryArray, &iUpArraySize, rank,
			true);
	}
	if (iMPIError != MPI_SUCCESS) {
		printf("Error occurred in fnmpiprocessing section- rank -%d \n", rank);
	}

	MPI_Type_free(&worldtype);

}
// prepare the root matrix for final processing , once data sent to slaves master process clear his matrix and prepare for incomming
// information from master itself and slaves.
void fnClearAndPrepareRootMatrix() {
	free(pt_SourceRoot[0]);
	free(pt_SourceRoot);
	pt_SourceRoot = alloc_2d_world(g_GridSize, g_GridSize);
}

// Calculate how many rows each process have to process and create a global array
int * fnLoadDistributeArray(int iNumofProcess, int rank) {
	int l_iStripSize = g_GridSize / iNumofProcess;
	int *pLoadArray = NULL;
	pLoadArray = (int*) malloc(iNumofProcess * sizeof(int));

	int iLoad;
	for (iLoad = 0; iLoad < iNumofProcess; ++iLoad)
		pLoadArray[iLoad] = l_iStripSize;
	if (g_GridSize != l_iStripSize * iNumofProcess) // we can not distribute the load equally
			{
		int l_iRemainder = g_GridSize - (l_iStripSize * iNumofProcess);

		for (iLoad = 0; iLoad < l_iRemainder; ++iLoad) {
			pLoadArray[iLoad] = pLoadArray[iLoad] + 1;
		}

	}

	return pLoadArray;
}
/* This the place where master distribute the data to slaves, all the size will be calculated according to global load array*/
void fnDistributeDataToSlaves(int iNumPro, int rank) {

	MPI_Datatype worldtype, oldworldtypes[2];
	int blockcounts[2];

	MPI_Aint offsets[1];
	offsets[0] = 0;
	oldworldtypes[0] = MPI_INT;
	blockcounts[0] = 6;

	MPI_Type_struct(1, blockcounts, offsets, oldworldtypes, &worldtype);
	MPI_Type_commit(&worldtype);

	if (rank != ROOT) {
		pt_SourceRoot = (world **) malloc(sizeof(world *) * 1);
		pt_SourceRoot[0] = (world *) malloc(sizeof(world) * 1);

	}
	pGlobalLoadArray = fnLoadDistributeArray(iNumPro, rank);
	pAggArray = (int*) malloc(sizeof(int) * iNumPro);
	int iAgg = 0;
	pAggArray[0] = 0;
	for (iAgg = 1; iAgg < iNumPro; ++iAgg) {
		pAggArray[iAgg] = pGlobalLoadArray[iAgg - 1] + pAggArray[iAgg - 1];
	}
	if (pGlobalLoadArray != NULL) {
		int iSize = 0;
		if (rank == 0 || rank == iNumPro - 1) {
			iSize = pGlobalLoadArray[rank] + 2;
		} else
			iSize = pGlobalLoadArray[rank] + 4;
		pt_DestinationWorld = alloc_2d_world(iSize, g_GridSize);
		fnInitDstWorld(pt_DestinationWorld, &iSize);
		pt_SourceWorld = alloc_2d_world(iSize, g_GridSize);
	}
	int PaddingArray;
	int DataSizeCount[iNumPro]; /* how many pieces of data everyone has */
	int iDataSize;
	int DisplaceArray[iNumPro]; /* the starting point of everyone's data */
	for (iDataSize = 0; iDataSize < iNumPro; ++iDataSize) {

		if (iDataSize == 0) {
			DataSizeCount[iDataSize] = (pGlobalLoadArray[iDataSize] + 2)
					* g_GridSize;
			DisplaceArray[iDataSize] = 0;
		} else if (iDataSize == (iNumPro - 1)) {
			DataSizeCount[iDataSize] = (pGlobalLoadArray[iDataSize] + 2)
					* g_GridSize;
			DisplaceArray[iDataSize] = (pAggArray[iDataSize] - 2) * g_GridSize;
		} else {
			DataSizeCount[iDataSize] = (pGlobalLoadArray[iDataSize] + 4)
					* g_GridSize;
			DisplaceArray[iDataSize] = (pAggArray[iDataSize] - 2) * g_GridSize;

		}
	}
	PaddingArray = DataSizeCount[rank];
	//Send the data to slaves
	MPI_Scatterv(&pt_SourceRoot[0][0], DataSizeCount, DisplaceArray, worldtype,
			&pt_SourceWorld[0][0], PaddingArray, worldtype, ROOT,
			MPI_COMM_WORLD);
	fnClearAndPrepareRootMatrix();
}
/* Process the world , it includes the communication part also , once processed they will call mpicommunication function share information among
 * process*/
void fnProcessWorld(int iProcessRank, int num_procs) {
	MPI_Datatype mpi_world_struct, mpi_old_world_type[2];

	int blockcounts[2];

	MPI_Aint offsets[1];
	offsets[0] = 0;
	mpi_old_world_type[0] = MPI_INT;
	blockcounts[0] = 6;
	double d_TotalProcessingTime;
	double l_clkStartTime;
	double inittime, totaltime;

	totaltime = 0.0;

	/* Create the user defined struct and commit that , so we can use mpi_world_struct as a normal struct*/
	MPI_Type_struct(1, blockcounts, offsets, mpi_old_world_type,
			&mpi_world_struct);
	MPI_Type_commit(&mpi_world_struct);
	int i;
	int m;
	int n;
	int l_iStart = 0;
	int l_iGenCounter = 0;

	int iLoopStartPos = 0;
	int iLoopEndPos = 0;
/* choose the start pos and end pos of each process , root process will be starting from 0 and others will start from 2, because , there are sharing row among others*/
	if (iProcessRank == ROOT) {
		iLoopStartPos = 0;
		iLoopEndPos = pGlobalLoadArray[iProcessRank];
	} else if (iProcessRank == num_procs - 1) {
		iLoopStartPos = 2;
		iLoopEndPos = pGlobalLoadArray[iProcessRank] + 2;
	} else {
		iLoopStartPos = 2;
		iLoopEndPos = pGlobalLoadArray[iProcessRank] + 2;
	}
	l_clkStartTime = omp_get_wtime();

/* Here we are using relative pos to skip the column, because , actual matrix's row and each process's row are not equal , for example , actual matrix row number 45 would match
 * with 14 in newer small matrx, */
	for (i = 1; i <= g_iNumberofGen; i++) {
		++l_iGenCounter;
#pragma omp parallel for private (n,l_iStart) schedule(static,1)
		for (m = iLoopStartPos; m < iLoopEndPos; ++m) {

			if (pt_SourceWorld[m][0].i_RelativePosX % 2 == 0)
				l_iStart = 1;
			else
				l_iStart = 0;

			for (n = l_iStart; n < g_GridSize; n = n + 2) {
				fnUpdateCell(&m, &n, pt_SourceWorld, pt_DestinationWorld,
						iProcessRank, num_procs);
			}
		}
/*Red generation processed finished , going to share the common ares with other process*/
		world downarray[4 * g_GridSize];
		world uparray[4 * g_GridSize];

		memset(downarray, 0, sizeof(downarray));
		memset(uparray, 0, sizeof(downarray));

		int downarraysize = 0;
		int uparraysize = 0;
		inittime = MPI_Wtime();
		if (iProcessRank == ROOT) {
       /*I am root process , so I have to copy last 4 rows and send to my adjacent process*/
			fnCopyTopAndLastWorld(pt_SourceWorld, pt_DestinationWorld,
					downarray, &downarraysize, iProcessRank, num_procs);
			fnMPICommunication(iProcessRank, num_procs, uparray, &uparraysize,
					downarray, &downarraysize);

		} else if (iProcessRank == num_procs - 1) {
			fnCopyTopAndLastWorld(pt_SourceWorld, pt_DestinationWorld, uparray,
					&uparraysize, iProcessRank, num_procs);

			fnMPICommunication(iProcessRank, num_procs, uparray, &uparraysize,
					downarray, &downarraysize);

		} else {
			/*I am middle matrix , so I need to copy my top 4 row and bottom 4 rows */
			fnCopyMiddleWorld(pt_SourceWorld, pt_DestinationWorld, uparray,
					&uparraysize, downarray, &downarraysize, iProcessRank,
					num_procs);

			fnMPICommunication(iProcessRank, num_procs, uparray, &uparraysize,
					downarray, &downarraysize);

		}
/* Data have been successfully exchanged  , so I can start process black generation*/
		totaltime += (MPI_Wtime() - inittime);
#pragma omp parallel for private (n) schedule(dynamic,1)
		for (m = iLoopStartPos; m < iLoopEndPos; ++m) {
			if (pt_SourceWorld[m][0].i_RelativePosX % 2 == 0)
				l_iStart = 0;
			else
				l_iStart = 1;
			for (n = l_iStart; n < g_GridSize; n = n + 2) {

				fnUpdateCell(&m, &n, pt_SourceWorld, pt_DestinationWorld,
						iProcessRank, num_procs);
			}
		}
		downarraysize = 0;
		uparraysize = 0;
		memset(downarray, 0, sizeof(downarray));
		memset(uparray, 0, sizeof(downarray));
		inittime = MPI_Wtime();
		if (iProcessRank == ROOT) {

			fnCopyTopAndLastWorld(pt_SourceWorld, pt_DestinationWorld,
					downarray, &downarraysize, iProcessRank, num_procs);
			fnMPICommunication(iProcessRank, num_procs, uparray, &uparraysize,
					downarray, &downarraysize);

		} else if (iProcessRank == num_procs - 1) {
			fnCopyTopAndLastWorld(pt_SourceWorld, pt_DestinationWorld, uparray,
					&uparraysize, iProcessRank, num_procs);

			fnMPICommunication(iProcessRank, num_procs, uparray, &uparraysize,
					downarray, &downarraysize);

		} else {
			fnCopyMiddleWorld(pt_SourceWorld, pt_DestinationWorld, uparray,
					&uparraysize, downarray, &downarraysize, iProcessRank,
					num_procs);

			fnMPICommunication(iProcessRank, num_procs, uparray, &uparraysize,
					downarray, &downarraysize);

		}
		totaltime += (MPI_Wtime() - inittime);
// every generation reduce the starvation period and breeding period
		if (l_iGenCounter < g_iNumberofGen) {
			int l_iRow;
			int l_iColumn;
			int l_iStartPos = 0;
			int l_iEndPos = 0;
			if (iProcessRank == ROOT) {
				l_iEndPos = pGlobalLoadArray[iProcessRank] + 2;
			} else if (iProcessRank == num_procs - 1) {
				l_iEndPos = pGlobalLoadArray[iProcessRank] + 2;
			} else {
				l_iEndPos = pGlobalLoadArray[iProcessRank] + 4;
			}
#pragma omp parallel for private(l_iColumn)
			for (l_iRow = l_iStartPos; l_iRow < l_iEndPos; ++l_iRow) {
				for (l_iColumn = 0; l_iColumn < g_GridSize; ++l_iColumn) {
					world *l_ptWorld = *(pt_SourceWorld + l_iRow) + l_iColumn;
					switch (l_ptWorld->type) {
					case 1: {
						if (l_ptWorld->breeding_period != 0)
							--l_ptWorld->breeding_period;
						--l_ptWorld->starvation_period;
						if (l_ptWorld->starvation_period == -1) // we are going to reduce the starvation period and it is going to die here
								{
							l_ptWorld->type = 0;
							l_ptWorld->starvation_period = 0;
							l_ptWorld->breeding_period = 0;

						}
					}
						break;
					case 2:
					case 5: {
						if (l_ptWorld->breeding_period != 0)
							--l_ptWorld->breeding_period;
					}
						break;
					default:
						break;

					}
				}
			}
		}
	}

	/* Total generation finished here, now every process have to send their own data to master , before that
	 * we are going to calculate the time which have taken to process*/

	d_TotalProcessingTime += (omp_get_wtime() - l_clkStartTime);

	printf("Average comm time - rank - %d  | time - %f \n",iProcessRank, totaltime );
	double l_dGlobalComTime = 0.0;
	double l_dGlobalProcessingTime = 0.0;
	/*We used mpi reduce to sum the total taken for communication*/
	MPI_Reduce(&totaltime, &l_dGlobalComTime, 1, MPI_DOUBLE, MPI_SUM, 0,
	MPI_COMM_WORLD);

	/* Calculate total time to process whole generation, so each process have to send their own value, we will sum here*/
	MPI_Reduce(&d_TotalProcessingTime, &l_dGlobalProcessingTime, 1, MPI_DOUBLE,
	MPI_SUM, 0,
	MPI_COMM_WORLD);
	if (iProcessRank == ROOT) {
		double d_ompprocessingTime =
				(l_dGlobalProcessingTime - l_dGlobalComTime) / num_procs;
		double d_avg_com_time = l_dGlobalComTime / num_procs;
		printf(
				"###########       DETAIL     ###############################################\n");
		printf(
				"Number of Gen - %d | Grid size- %d |  Communication Time(overlap) : %f Sec |  processing time - %f\n",
				g_iNumberofGen, g_GridSize, d_avg_com_time,
				d_ompprocessingTime);
		printf(
				"#############################################################################\n\n");
	}
	int *ptCountRemoteArray;
	world *ptLocalProcessedMatrix = 0;
	world *ptRecvArray = 0;
	int *displacements = 0;
	int *counts = 0;
	if (iProcessRank == 0) {
		counts = (int*) malloc(num_procs * sizeof(int));
		displacements = (int*) malloc(num_procs * sizeof(int));
	}
	int iSpos = 0;
	int iendpos = 0;
	if (iProcessRank == ROOT) {
		iSpos = 0;
		iendpos = pGlobalLoadArray[iProcessRank];
	} else {
		iSpos = 2;
		iendpos = pGlobalLoadArray[iProcessRank] + 2;
	}
	int iColumn = 0;
	int iStartpos;
	ptLocalProcessedMatrix = (world*) malloc(
			pGlobalLoadArray[iProcessRank] * g_GridSize * sizeof(world));
	int iLcounter = 0;
	for (iStartpos = iSpos; iStartpos < iendpos; ++iStartpos) {
		for (iColumn = 0; iColumn < g_GridSize; ++iColumn) {
			world *ptSrcTemp = *(pt_SourceWorld + iStartpos) + iColumn;
			__asm__(".symver memcpy,memcpy@GLIBC_2.2.5");
			//* some systems do not have newest memcpy@@GLIBC_2.14 - stay with old good one */
			memcpy((ptLocalProcessedMatrix + iLcounter), ptSrcTemp,
					sizeof(world));
			++iLcounter;

		}
	}
	ptCountRemoteArray = (int*) malloc(1 * sizeof(int));
	ptCountRemoteArray[0] = pGlobalLoadArray[iProcessRank] * g_GridSize;
	MPI_Gather((void*) ptCountRemoteArray, 1, MPI_INT, (void*) counts, 1,
	MPI_INT, 0,
	MPI_COMM_WORLD);
	int size;
	if (iProcessRank == 0) {
		displacements[0] = 0;
		for (i = 1; i < num_procs; i++) {
			displacements[i] = counts[i - 1] + displacements[i - 1];
		}
		size = 0;
		for (i = 0; i < num_procs; i++)
			size = size + counts[i];
		ptRecvArray = (world*) malloc(size * sizeof(world));
	}

	MPI_Gatherv(ptLocalProcessedMatrix,
			pGlobalLoadArray[iProcessRank] * g_GridSize, mpi_world_struct,
			ptRecvArray, counts, displacements, mpi_world_struct, 0,
			MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
	/* Each process have sent the data to master process, so we can populate the root matrix*/
	if (iProcessRank == ROOT) {
		fnFinalUpdate(ptRecvArray, &size);
		free(ptRecvArray);
	}

	free(ptLocalProcessedMatrix);
	if (iProcessRank == ROOT) {  //only master process will print the output
		fnDumpTheOutput(pt_SourceRoot, &g_GridSize, &num_procs);
	}
}

int main(int argc, char **argv) {
	int numofProcess, rank, len, rc;

	rc = MPI_Init(&argc, &argv);
	if (rc != MPI_SUCCESS) {
		printf("Error starting MPI program. Terminating.\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	MPI_Comm_size(MPI_COMM_WORLD, &numofProcess); // get the number of the process/hosts
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // get the current process id

	if (numofProcess == 1) {
		printf(
				"Number of process is 1 , going to terminate,please specify more that one.\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
	char hostname[MPI_MAX_PROCESSOR_NAME];
	MPI_Get_processor_name(hostname, &len);

	int iLen;
	int iHostTotalLen = 0;
	for (iLen = 0; iLen < len; ++iLen) {
		iHostTotalLen += (int) hostname[iLen];
	}

	char *l_pFileName;

	if (argc == 6) {

		l_pFileName = argv[1];
		l_iWolfBP = atoi(argv[2]);
		l_iSquirrelBP = atoi(argv[3]);
		l_iWolfSP = atoi(argv[4]);
		g_iNumberofGen = atoi(argv[5]);

	} else {
		printf(
				"Some arguments are missing from input , Please check the argument list \n");
		printf(
				"<Program name><file name><wolf BP><Squirrel BP><Number of Gen>");
		MPI_Finalize();
		exit(EXIT_FAILURE);

	}
	printf("Number of tasks= %d My rank= %d Running on %s\n", numofProcess,
			rank, hostname);
	int l_iGridSize = 0;
	if (rank == ROOT) {

		fnLoadtheGeneration(l_pFileName, l_iWolfBP, l_iSquirrelBP, l_iWolfSP,
				&l_iGridSize);

	}
	MPI_Bcast(&l_iGridSize, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

	g_GridSize = l_iGridSize;

	g_ComMatrixSize = 4 * g_GridSize;
	fnDistributeDataToSlaves(numofProcess, rank);
	MPI_Barrier(MPI_COMM_WORLD);
	int *ptRcvHostName = (int *) malloc(numofProcess * sizeof(int));

	MPI_Allgather(&iHostTotalLen, 1, MPI_INT, ptRcvHostName, 1, MPI_INT, MPI_COMM_WORLD);

	int iMes = 0;
	for (iLen = 0; iLen < numofProcess; ++iLen) {
		if (ptRcvHostName[iLen] == iHostTotalLen)
			++iMes;
	}
	if (iMes == 1) {
		printf(
				"I am running alone, so I am going to spawn two threads -my rank -%d \n",
				rank);
		omp_set_num_threads(SPAWN_THREAD);  //currently this value is defined as two , because all the machine in the cluster have maximum 4 cores.
	} else if (iMes > 1) {
		printf(
				"Some other process are running on my host, I am going to perform without spawning any thread -my rank -%d \n",
				rank);
		omp_set_num_threads(1);
	}

	/* Everything is settled , we are going to start process the matrix, so all the slaves will be processing this matrix */
	fnProcessWorld(rank, numofProcess);

	/* We should clean up the memory that we have allocated for every matrix  */
	free(pt_SourceWorld[0]);
	free(pt_SourceWorld);

	free(pt_DestinationWorld[0]);
	free(pt_DestinationWorld);

	free(pt_SourceRoot[0]);
	free(pt_SourceRoot);

	/* MPI cleanup*/
	MPI_Finalize();

	return EXIT_SUCCESS;
}

