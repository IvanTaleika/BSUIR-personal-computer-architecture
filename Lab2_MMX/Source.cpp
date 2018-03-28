//Taleika 650504 - 7)matrix multiplication
#include <iostream>
#include <ctime>

#define ROWS 4 //8
#define COLUMS 4 //8
#define MAX_ELEMENT_VALUE 32
#define REPEAT_COUNT 1000000

#define CELL_SIZE 2
#define ROW_SIZE 8 //16

#define MMX_SINGLE_STEP 4
#define MMX_STEP_SIZE 8
#define RESULT_CELL_SIZE 4

void initMatrix(short int(&matrix)[ROWS][COLUMS]);
void cMultiplyMatrix(short int(&matrix1)[ROWS][COLUMS], short int(&matrix2)[ROWS][COLUMS]);
void transponMatrix(short int(&matrix)[ROWS][COLUMS]);

template<class T>
void showMatrix(T(&matrix)[ROWS][COLUMS]);

int main(void) {
	//srand(time(0));

	short int matrix1[ROWS][COLUMS]; //Создание первой матрицы
	initMatrix(matrix1);
	std::cout << "Matrix 1:" << std::endl;
	showMatrix(matrix1);
	std::cout << std::endl;

	short int matrix2[ROWS][COLUMS]; //Создание второй матрицы
	initMatrix(matrix2);
	std::cout << "Matrix 2:" << std::endl;
	showMatrix(matrix2);
	std::cout << std::endl;

	cMultiplyMatrix(matrix1, matrix2);
	
	short int result[ROWS][COLUMS]; //Создание матрицы для результата
	int repeatCounter;
	int i, j, k; 
	clock_t start = clock();
	//Вычисления с использованием ассемблера
	_asm {
		mov repeatCounter, 0
	multiplyAgain: 
			mov i, 0
			lea ebx, result		//Загрузка адреса матриц
			lea ecx, matrix1
			push ecx
			lea esi, matrix2
			push esi
			
		loopI:
				mov j, 0
			loopJ:
					mov k, 0
					xor ax,ax	//Обнуление регистра ax
					mov [ebx], ax
				loopK:
						mov ax, [ecx]
						mov dx, [esi]
						mul dx
						add [ebx], ax
						add ecx, CELL_SIZE  //Перемещение по матрице
						add esi, ROW_SIZE
						inc k
						cmp k, ROWS
						jne loopK

					add ebx, CELL_SIZE
					pop esi
					pop ecx
					add esi, CELL_SIZE
					push ecx
					push esi
					inc j
					cmp j, COLUMS
					jne loopJ

				pop esi
				pop ecx
				add ecx, ROW_SIZE
				lea esi, matrix2
				push ecx
				push esi
				inc i
				cmp i, ROWS
				jne loopI

			pop esi
			pop ecx
			inc repeatCounter
			cmp repeatCounter, REPEAT_COUNT
			jne multiplyAgain

	}
	clock_t finish = clock();
	std::cout << std::endl;
	std::cout << "Result of multiplication using Assembler:" << std::endl;
	showMatrix(result);
	std::cout << "time:" << finish - start << "ms" << std::endl;

	int resultMMX[ROWS][COLUMS];
	transponMatrix(matrix2);
	start = clock();
	//Вычисления с использованием MMX
	_asm {
		mov repeatCounter, 0
	multiplyAgainMMX: 
			mov i, 0
			lea ebx, resultMMX		
			lea ecx, matrix1
			push ecx
			lea esi, matrix2
			push esi
			
		iLoopMMX:
				mov j, 0
			jLoopMMX:
					mov k, 0
					pxor MM7,MM7
					movd [ebx], MM7
				kLoopMMX:
						movq MM0, [ecx]		
						movq MM1, [esi]
						pmaddwd MM0, MM1
						paddd MM7, MM0
						add ecx, MMX_STEP_SIZE
						add esi, MMX_STEP_SIZE
						add k, 4
						cmp k, ROWS
						jne kLoopMMX

					movq MM0,MM7
					psrlq MM7, 32
					paddd MM7, MM0
					movd [ebx],MM7
					add ebx, RESULT_CELL_SIZE
					pop esi
					pop ecx
					add esi, ROW_SIZE
					push ecx
					push esi
					inc j
					cmp j, COLUMS
					jne jLoopMMX

				pop esi
				pop ecx
				add ecx, ROW_SIZE
				lea esi, matrix2
				push ecx
				push esi
				inc i
				cmp i, ROWS
				jne iLoopMMX

			pop esi
			pop ecx
			inc repeatCounter
			cmp repeatCounter, REPEAT_COUNT
			jne multiplyAgainMMX
		emms
	}
	finish = clock();
	std::cout << std::endl;
	std::cout << "Result of multiplication using MMX:" << std::endl;
	showMatrix(resultMMX);
	std::cout << "time:" << finish - start << "ms" << std::endl;
	system("pause");
	return 0;
}

//Инициализация матрицы
void initMatrix(short int(&matrix)[ROWS][COLUMS]) {
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMS; j++) {
			matrix[i][j] = rand() % MAX_ELEMENT_VALUE;
		}
	}
}

//Вывод матрицы на экран
template<class T>
void showMatrix(T(&matrix)[ROWS][COLUMS]) {
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLUMS; j++) {
			std::cout << matrix[i][j] << "\t";
		}
		std::cout << std::endl;
	}
}

//Функция умножения матриц используя язык C++
void cMultiplyMatrix(short int(&matrix1)[ROWS][COLUMS], short int(&matrix2)[ROWS][COLUMS]) {
	short int result[ROWS][COLUMS];
	int repeatCounter = 0;
	clock_t start = clock();
	while (repeatCounter < REPEAT_COUNT) {
		for (int i = 0; i < ROWS; i++) {
			for (int j = 0; j < COLUMS; j++) {
				result[i][j] = 0;
				for (int k = 0; k < ROWS; k++) {
					result[i][j] += matrix1[i][k] * matrix2[k][j];
				}
			}
		}
		repeatCounter++;
	}
	clock_t finish = clock();
	std::cout << "Result of multiplication using C:" << std::endl;
	showMatrix(result);
	std::cout << "time:" << finish - start << "ms" << std::endl;
}

//Функция транспонирования матрицы
void transponMatrix(short int(&matrix)[ROWS][COLUMS]) { 
	short int temp;
	for (int i = 0; i < ROWS; i++) {
		for (int j = i + 1; j < COLUMS; j++) {
			temp = matrix[i][j];
			matrix[i][j] = matrix[j][i];
			matrix[j][i] = temp;
		}
	}
}